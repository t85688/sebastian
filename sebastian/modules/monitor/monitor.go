package monitor

import (
	"context"
	"fmt"
	"reflect"
	"sync"
	"time"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/workerpool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/configmapper"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/topomanager"
	dmschema "gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/network/netmgr/api/parsV1"
)

const (
	projectIdNone          int64 = -1
	defaultPollingInterval       = 15
)

const (
	indexNameActLinkId = "ActLinkId"
)

var (
	ErrStopMonitorNotRunning = fmt.Errorf("monitor is not running")
	ErrStopMonitorStopping   = fmt.Errorf("monitor is stopping, please wait")
)

type IMonitor interface {
	GetMonitorStatus(projectId int64) (MonitorStatus, bool)
	Start(projectId int64) error
	Stop(projectId int64) error

	GetSFPLinks(projectId int64) (SFPLinks, error)
}

func (m *DefaultMonitor) GetSFPLinks(projectId int64) (SFPLinks, error) {
	if !m.isMonitoringOrReloading(projectId) {
		return nil, fmt.Errorf("monitor is not running, projectId: %d", projectId)
	}

	sfpLinksCache := m.sfpCache.GetAll()

	sfpLinks := make(SFPLinks, 0, len(sfpLinksCache))
	for _, link := range sfpLinksCache {
		sfpLinks = append(sfpLinks, link)
	}

	return sfpLinks, nil
}

func (m *DefaultMonitor) GetMonitorStatus(projectId int64) (MonitorStatus, bool) {
	currentState, exists := m.getInternalState(projectId)
	if !exists {
		return MonitorStatusUnknown, false
	}

	// mapping internal state to monitor status
	switch currentState {
	case InternalStateStopped,
		InternalStateStarting,
		InternalStateStopping,
		InternalStatePaused:
		return MonitorStatusStopped, true
	case InternalStateRunning, InternalStateReloading:
		return MonitorStatusRunning, true
	default:
		return MonitorStatusUnknown, true
	}
}

func (m *DefaultMonitor) getInternalState(projectId int64) (InternalState, bool) {
	state, exists := m.internalStateMap.Get(projectId)

	return state, exists
}

func (m *DefaultMonitor) setInternalState(projectId int64, state InternalState) {
	currentState, exists := m.internalStateMap.Get(projectId)

	// internal state change log
	if exists {
		logger.Infof("Monitor internal state changed from %s to %s, projectId: %d", currentState.String(), state.String(), projectId)
	} else {
		logger.Infof("Monitor internal state set to %s, projectId: %d", state.String(), projectId)
	}

	m.internalStateMap.Set(projectId, state)
}

type DefaultMonitor struct {
	context              context.Context
	cancelFunc           context.CancelFunc
	sessionId            int64
	mutex                sync.Mutex
	internalMutex        sync.Mutex
	internalStateMap     *LocalCache[int64, InternalState]
	mafDevicesCache      *LocalCache[string, *netdl.Device]
	mafLinksCache        *LocalCache[string, *netdl.Link]
	baselineCache        *domain.Project
	baselineDeviceMap    *LocalCache[int64, *domain.Device]
	baselineLinkMap      *MultiIndexCache[string, *domain.Link]
	currentDeviceMap     *LocalCache[int64, *domain.Device]
	currentLinkMap       *MultiIndexCache[string, *domain.Link]
	currentProject       *domain.Project
	currentProjectId     int64
	actDeviceManager     topomanager.IActDeviceManager
	simpleProfiles       *domain.SimpleDeviceProfiles
	pollingIntervalNow   int
	scanRangeSettingsNow []*domain.ScanIpRange
	dispatcher           *monitorTaskDispatcher
	sfpCache             *LocalCache[int64, *SFPLink]
	linkMutexMap         sync.Map
}

func NewMonitor() IMonitor {
	return &DefaultMonitor{
		actDeviceManager: topomanager.NewActDeviceManager(),
		currentProjectId: projectIdNone,
		internalStateMap: NewLocalCache[int64, InternalState](),
	}
}

func (m *DefaultMonitor) isMonitoring(projectId int64) bool {
	currentState, exists := m.getInternalState(projectId)
	return exists && currentState == InternalStateRunning
}

func (m *DefaultMonitor) isMonitoringOrReloading(projectId int64) bool {
	currentState, exists := m.getInternalState(projectId)
	return exists && (currentState == InternalStateRunning || currentState == InternalStateReloading)
}

func (m *DefaultMonitor) Start(projectId int64) error {
	if projectId <= 0 {
		return fmt.Errorf("invalid project ID: %d", projectId)
	}

	retryTimes := 9
	retryInterval := 200 * time.Millisecond

	tryLockResult := m.mutex.TryLock()

	if !tryLockResult {
		for i := 0; i < retryTimes; i++ {
			time.Sleep(retryInterval)
			logger.Info("Retrying to acquire lock to start monitor, attempt:", i+1)
			tryLockResult = m.mutex.TryLock()
			if tryLockResult {
				break
			}
		}
	}

	if !tryLockResult {
		logger.Warn("Failed to acquire lock to start monitor, please try again later")

		internalState, exists := m.getInternalState(projectId)

		if exists {
			if internalState == InternalStateStarting {
				logger.Warn("Monitor is starting, please wait")
				return fmt.Errorf("Monitor is starting, please wait")
			}

			if internalState == InternalStateStopping {
				logger.Warn("Monitor is stopping, please try again later")
				return fmt.Errorf("Monitor is stopping, please try again later")
			}
		}

		return fmt.Errorf("Failed to start monitor, please try again later")
	}

	defer m.mutex.Unlock()

	m.internalMutex.Lock()
	defer m.internalMutex.Unlock()

	returnErr := m.onStartMonitor(projectId)
	return returnErr
}

func (m *DefaultMonitor) onStartMonitor(projectId int64) (returnErr error) {
	internalStateMap := m.internalStateMap.GetAll()
	for projectIdInMap, state := range internalStateMap {
		var errMsg string
		if state != InternalStateStopped {
			if projectIdInMap == projectId {
				logger.Warnf("this project has been already in monitor state, projectId: %d", projectIdInMap)
				return nil
			} else {
				errMsg = fmt.Sprintf("Another project is in monitor state, projectId: %d", projectIdInMap)
				logger.Warnf(errMsg)
				return fmt.Errorf("%s", errMsg)
			}
		}
	}

	_, res := core.GetProjectInfo(projectId)

	if !res.IsSuccess() {
		return fmt.Errorf("failed to get project(%v) info: %v", projectId, res)
	}

	res = core.UpdateProjectStatus(projectId, domain.ProjectStatusEnum_Monitoring)
	if !res.IsSuccess() {
		return fmt.Errorf("failed to update project(%v) status to Monitoring: %v", projectId, res)
	}

	logger.Info("Starting monitor...")
	m.setInternalState(projectId, InternalStateStarting)

	defer func() {
		if returnErr != nil {
			cancelFunc := m.cancelFunc
			if cancelFunc != nil {
				cancelFunc()
			}

			m.setInternalState(projectId, InternalStateStopped)

			logger.Errorf("Failed to start monitor: %v, try to update project(%v) status to Idle", returnErr, projectId)
			res = core.UpdateProjectStatus(projectId, domain.ProjectStatusEnum_Idle)
			if !res.IsSuccess() {
				logger.Errorf("Failed to update project(%v) status to Idle: %v", projectId, res)
			} else {
				logger.Infof("Project(%v) status updated to Idle", projectId)
			}

		} else {
			m.setInternalState(projectId, InternalStateRunning)
		}
	}()

	defer func() {
		if r := recover(); r != nil {
			if returnErr == nil {
				returnErr = fmt.Errorf("panic occurred during starting monitor: %v", r)
			} else {
				returnErr = fmt.Errorf("panic occurred during starting monitor: %v; execution error: %v", r, returnErr)
			}

			logger.Errorf("panic occurred during starting monitor: %v", returnErr)
		}
	}()

	// Get Activated Baseline
	project, status := core.GetActivatedBaseline(projectId)
	if !status.IsSuccess() {
		logger.Errorf("Failed to get activated baseline for project %d: %v", projectId, status)
		return fmt.Errorf("Failed to get activated baseline for project %d: %v", projectId, status)
	}

	res = core.UpdateFullProject(project, true)

	if !res.IsSuccess() {
		logger.Errorf("Failed to update operation project(%d): %v", projectId, res)
		return fmt.Errorf("Failed to update operation project(%d): %v", projectId, res)
	}

	operProject, res := core.GetProject(projectId, true, true)
	if !res.IsSuccess() {
		logger.Errorf("Failed to get operation project(%d): %v", projectId, res)
		return fmt.Errorf("Failed to get operation project(%d): %v", projectId, res)
	}

	m.currentProject = &operProject

	// Get Simple Device Profiles
	if m.simpleProfiles == nil {
		simpleProfiles, status := core.GetSimpleDeviceProfiles()
		if !status.IsSuccess() {
			logger.Error("Failed to get simple device profile:", status.ErrorMessage)
			return fmt.Errorf("Failed to get simple device profile: %v", status)
		}
		m.simpleProfiles = simpleProfiles
	}

	nmManager, err := dipool.GetNMManager()
	if err != nil {
		logger.Errorf("Failed to get NM Manager: %v", err)
		return fmt.Errorf("Failed to start monitor")
	}

	// Initialize maps
	logger.Info("Initialize monitor caches...")
	m.pollingIntervalNow = defaultPollingInterval
	m.mafDevicesCache = NewLocalCache[string, *netdl.Device]()
	m.mafLinksCache = NewLocalCache[string, *netdl.Link]()
	m.baselineDeviceMap = NewLocalCache[int64, *domain.Device]()
	baselineLinkMap := NewMultiIndexCache[string, *domain.Link]()
	baselineLinkMap.AddIndex(indexNameActLinkId, func(link *domain.Link) string {
		return fmt.Sprintf("%d", link.Id)
	})
	m.baselineLinkMap = baselineLinkMap

	m.currentDeviceMap = NewLocalCache[int64, *domain.Device]()
	currentLinkMap := NewMultiIndexCache[string, *domain.Link]()
	currentLinkMap.AddIndex(indexNameActLinkId, func(link *domain.Link) string {
		return fmt.Sprintf("%d", link.Id)
	})
	m.currentLinkMap = currentLinkMap
	m.sfpCache = NewLocalCache[int64, *SFPLink]()
	m.baselineCache = project
	m.currentProjectId = projectId
	m.linkMutexMap.Clear()
	m.actDeviceManager.ClearDeviceMappings()

	for i, actDevice := range project.Devices {
		device := &project.Devices[i]

		m.baselineDeviceMap.Set(device.Id, device)
		m.currentDeviceMap.Set(actDevice.Id, &actDevice)
	}

	for i := range project.Links {
		link := &project.Links[i]

		linkStr, err := m.getActLinkStrInBaseline(link)
		if err != nil {
			logger.Warnf("Failed to get link identifier string in baseline: %v", err)
			continue
		}

		m.baselineLinkMap.Set(linkStr, link)
		cloneLink := *link
		m.currentLinkMap.Set(linkStr, &cloneLink)
	}

	logger.Info("Initialize monitor caches... Done")

	logger.Info("Clear MAF Device and Link cache...")
	nmManager.Reset()
	logger.Info("Clear MAF Device and Link cache... Done")

	if len(project.Devices) == 0 && len(project.ProjectSetting.ScanIpRanges) == 0 {
		logger.Warnf("No devices or scan ranges in the activated baseline for project %d", projectId)
	}

	logger.Infof("polling interval in project setting: %v seconds", project.ProjectSetting.MonitorConfiguration.PollingInterval)
	pollingInterval := getValidPollingInterval(project.ProjectSetting.MonitorConfiguration.PollingInterval)
	m.pollingIntervalNow = pollingInterval
	logger.Infof("Monitor polling interval: %v seconds", pollingInterval)

	scanIpRanges := project.ProjectSetting.ScanIpRanges

	deviceSettings := map[string]parsV1.DeviceMonitorConf{}
	scanRangeSettings := make([]parsV1.ScanRangeWithSetting, 0, len(project.ProjectSetting.ScanIpRanges))
	ipSettings := make([]parsV1.IndividualIp, 0, len(project.Devices))

	logger.Infof("[StartMonitor] Total device count in baseline: %d", len(project.Devices))
	for i, actDevice := range project.Devices {
		logger.Infof("baseline device %d: %v", i, actDevice.Ipv4.IpAddress)
	}

	for i, actDevice := range project.Devices {
		mafSNMPVersion, canParseSNMPVersion := configmapper.MapActSNMPVersionToMafSNMPVersion(actDevice.SnmpConfiguration.Version)
		if !canParseSNMPVersion {
			logger.Warnf("invalid SNMP version in device %v: %q", actDevice.Ipv4.IpAddress, actDevice.SnmpConfiguration.Version)
			continue
		}

		mafSNMPAuthType, canParseAuthType := configmapper.MapActSNMPAuthenticationTypeToMafSNMPAuthenticationType(actDevice.SnmpConfiguration.AuthenticationType)
		if !canParseAuthType {
			logger.Warnf("invalid SNMP auth type in device %v: %q", actDevice.Ipv4.IpAddress, actDevice.SnmpConfiguration.AuthenticationType)
			continue
		}

		mafEncType, canParseEncType := configmapper.MapActSNMPEncryptionTypeToMafSNMPEncryptionType(actDevice.SnmpConfiguration.DataEncryptionType)
		if !canParseEncType {
			logger.Warnf("invalid SNMP encryption type in device %v: %q", actDevice.Ipv4.IpAddress, actDevice.SnmpConfiguration.DataEncryptionType)
			continue
		}

		settingKey := fmt.Sprintf("d%d", i)
		deviceSettings[settingKey] = parsV1.DeviceMonitorConf{
			PollingInterval: pollingInterval,
			DeviceSecrets: dmschema.DeviceSecrets{
				GlobalUsername: actDevice.Account.Username,
				GlobalPassword: actDevice.Account.Password,
				Snmp: &dmschema.SNMP{
					Version:         mafSNMPVersion,
					ReadCommunity:   actDevice.SnmpConfiguration.ReadCommunity,
					WriteCommunity:  actDevice.SnmpConfiguration.WriteCommunity,
					Username:        actDevice.SnmpConfiguration.Username,
					AuthType:        mafSNMPAuthType,
					AuthPassword:    actDevice.SnmpConfiguration.AuthenticationPassword,
					DataEncryptType: mafEncType,
					DataEncryptKey:  actDevice.SnmpConfiguration.DataEncryptionKey,
					Port:            actDevice.SnmpConfiguration.Port,
				},
			},
		}

		ipSettings = append(ipSettings, parsV1.IndividualIp{
			IP: actDevice.Ipv4.IpAddress,
			ScanSetting: parsV1.ScanSetting{
				DeviceSetting:  settingKey,
				AutoEnableSnmp: actDevice.EnableSnmpSetting,
			},
		})
	}

	logger.Infof("[StartMonitor] Total scan range count in baseline: %d", len(project.ProjectSetting.ScanIpRanges))
	for i, scanRange := range project.ProjectSetting.ScanIpRanges {
		logger.Infof("baseline scan range %d: %v-%v", i, scanRange.StartIp, scanRange.EndIp)
	}

	for i, scanRange := range project.ProjectSetting.ScanIpRanges {
		mafSNMPVersion, canParseSNMPVersion := configmapper.MapActSNMPVersionToMafSNMPVersion(scanRange.SnmpConfiguration.Version)
		if !canParseSNMPVersion {
			logger.Warnf("invalid SNMP version in IP range : %v", scanRange.SnmpConfiguration.Version)
			continue
		}

		mafSNMPAuthType, canParseAuthType := configmapper.MapActSNMPAuthenticationTypeToMafSNMPAuthenticationType(scanRange.SnmpConfiguration.AuthenticationType)
		if !canParseAuthType {
			logger.Warnf("invalid SNMP auth type in IP range : %v", scanRange.SnmpConfiguration.AuthenticationType)
			continue
		}

		mafEncType, canParseEncType := configmapper.MapActSNMPEncryptionTypeToMafSNMPEncryptionType(scanRange.SnmpConfiguration.DataEncryptionType)
		if !canParseEncType {
			logger.Warnf("invalid SNMP encryption type in IP range : %v", scanRange.SnmpConfiguration.DataEncryptionType)
			continue
		}

		deviceSettingName := fmt.Sprintf("r%d", i)
		deviceSettings[deviceSettingName] = parsV1.DeviceMonitorConf{
			PollingInterval: pollingInterval,
			DeviceSecrets: dmschema.DeviceSecrets{
				GlobalUsername: scanRange.Account.Username,
				GlobalPassword: scanRange.Account.Password,
				Snmp: &dmschema.SNMP{
					Version:         mafSNMPVersion,
					ReadCommunity:   scanRange.SnmpConfiguration.ReadCommunity,
					WriteCommunity:  scanRange.SnmpConfiguration.WriteCommunity,
					Username:        scanRange.SnmpConfiguration.Username,
					AuthType:        mafSNMPAuthType,
					AuthPassword:    scanRange.SnmpConfiguration.AuthenticationPassword,
					DataEncryptType: mafEncType,
					DataEncryptKey:  scanRange.SnmpConfiguration.DataEncryptionKey,
					Port:            scanRange.SnmpConfiguration.Port,
				},
			},
		}

		scanRangeSettings = append(scanRangeSettings, parsV1.ScanRangeWithSetting{
			ScanRange: parsV1.ScanRange{
				StartIP: scanRange.StartIp,
				EndIP:   scanRange.EndIp,
			},
			ScanSetting: parsV1.ScanSetting{
				DeviceSetting:  deviceSettingName,
				AutoEnableSnmp: scanRange.EnableSnmpSetting,
			},
		})
	}

	sessionId := time.Now().UnixNano()
	ctx, cancelFunc := context.WithCancel(context.Background())
	ctx = context.WithValue(ctx, ContextKeyProjectId, projectId)
	ctx = context.WithValue(ctx, ContextKeySessionId, sessionId)
	m.context = ctx
	m.cancelFunc = cancelFunc
	m.sessionId = sessionId

	err = m.SubscribeTopologyUpdateData(ctx)
	if err != nil {
		logger.Error(err.Error())
		return err
	}

	// debug log
	// reqBytes, err := json.Marshal(&parsV1.ListRequest{
	// 	DeviceSettings: deviceSettings,
	// 	ScanList: parsV1.ScanList{
	// 		Ranges:        scanRangeSettings,
	// 		IndividualIps: ipSettings,
	// 	},
	// })

	// if err == nil {
	// 	logger.Infof("StartMonitor request: %s", string(reqBytes))
	// }

	ok, err := nmManager.StartMonitor(true, &parsV1.ListRequest{
		DeviceSettings: deviceSettings,
		ScanList: parsV1.ScanList{
			Ranges:        scanRangeSettings,
			IndividualIps: ipSettings,
		},
	})

	logger.Info("Start NM monitor... ")
	if !ok {
		m.UnSubscribeTopologyUpdateData()
		m.sessionId = -1
		logger.Errorf("Failed to start monitor: %v", err)
		return fmt.Errorf("Failed to start monitor: %v", err)
	}

	logger.Info("Start NM monitor... Done")

	// Initialize monitor dispatcher
	workerPool := workerpool.NewWorkerPoolWaitable(4, 16)
	dispatcher := NewMonitorTaskDispatcher(ctx, workerPool)
	m.dispatcher = dispatcher

	// Register monitor tasks
	m.dispatcher.RegisterTask(
		MonitorTaskTypeDeviceAliveStatus.String(),
		MonitorTaskDispatchStrategyScheduled,
		m.makeMonitorDeviceAliveStatusTask(),
	)

	m.dispatcher.RegisterTask(
		MonitorTaskTypeLinkAliveStatus.String(),
		MonitorTaskDispatchStrategyScheduled,
		m.makeMonitorLinkAliveStatusTask(),
	)

	m.dispatcher.RegisterTask(
		MonitorTaskTypeDeviceSystemStatus.String(),
		MonitorTaskDispatchStrategyScheduled,
		m.makeMonitorDeviceSystemStatusTask(),
	)

	m.dispatcher.RegisterTask(
		MonitorTaskTypeTraffic.String(),
		MonitorTaskDispatchStrategyScheduled,
		m.makeMonitorTrafficUpdateLinksTask(),
	)

	m.dispatcher.RegisterTask(
		MonitorTaskTypeManagementLink.String(),
		MonitorTaskDispatchStrategyScheduled,
		m.makeMonitorManagementLinkTask(),
	)

	m.dispatcher.RegisterTask(
		MonitorTaskTypeSFP.String(),
		MonitorTaskDispatchStrategyScheduled,
		m.makeMonitorSFPTask(),
	)

	m.dispatcher.RegisterTask(
		MonitorTaskTypeSwift.String(),
		MonitorTaskDispatchStrategyScheduled,
		m.makeMonitorSwiftTask(m.currentProject),
	)

	// start dispatcher
	m.dispatcher.Start()

	// store scan range settings
	m.scanRangeSettingsNow = scanIpRanges

	// Start polling scan range settings
	needHotReload := true
	if needHotReload {
		go m.pollingScanRangeSettings(ctx)
	}

	go m.periodicallyUpdateTopology(ctx, 7*time.Second)

	logger.Infof("Monitor started successfully with session ID %d", m.sessionId)

	return nil
}

type hotReloadStatus int

const (
	hotReloadStatusUnknown hotReloadStatus = iota
	hotReloadStatusSuccess
	hotReloadStatusNeedToStop
)

func (m *DefaultMonitor) hotReloadMonitorSettings(ctx context.Context, newScanRangeSettings []*domain.ScanIpRange, newPollingInterval int) error {
	projectId, projectIdExists := getProjectId(ctx)
	if !projectIdExists {
		return fmt.Errorf("Cannot get project ID from context, cannot hot reload monitor settings")
	}

	if !m.internalMutex.TryLock() {
		logger.Warn("cannot get internal lock to hot reload monitor settings")
		return fmt.Errorf("cannot get internal lock to hot reload monitor settings")
	}
	defer m.internalMutex.Unlock()

	if ctx.Err() != nil {
		logger.Warnf("Context is done, cannot hot reload monitor settings")
		return fmt.Errorf("Context is done, cannot hot reload monitor settings")
	}

	reloadStatus := hotReloadStatusUnknown
	originInternalState, stateExist := m.getInternalState(projectId)
	if !stateExist {
		return fmt.Errorf("cannot get original internal state, cannot hot reload monitor settings")
	}

	baseline := m.baselineCache
	if baseline == nil {
		logger.Warn("baseline cache is nil, cannot hot reload monitor settings")
		return fmt.Errorf("baseline cache is nil, cannot hot reload monitor settings")
	}

	defer func() {
		if reloadStatus == hotReloadStatusUnknown {
			m.setInternalState(projectId, originInternalState)
		}

		if reloadStatus == hotReloadStatusNeedToStop {
			logger.Info("Stopping monitor due to hot reload failure...")
			m.setInternalState(projectId, originInternalState)
			err := m.onStopMonitor(projectId)
			if err != nil {
				logger.Errorf("Failed to stop monitor after hot reload failure: %v", err)
			} else {
				logger.Info("Monitor stopped successfully after hot reload failure")
			}
		}
	}()

	defer func() {
		if r := recover(); r != nil {
			logger.Errorf("panic occurred during hot reload monitor settings: %v", r)
		}
	}()

	if newScanRangeSettings == nil {
		return fmt.Errorf("new scan range settings is nil")
	}

	if baseline == nil {
		return fmt.Errorf("baseline cache is nil")
	}

	if !m.isMonitoring(projectId) {
		logger.Warn("Monitor is not running, cannot hot reload settings")
		return fmt.Errorf("Monitor is not running, cannot hot reload settings")
	}

	logger.Info("Hot reloading monitor settings...")

	// set internal state to reloading
	m.setInternalState(projectId, InternalStateReloading)

	nmManager, err := dipool.GetNMManager()
	if err != nil {
		logger.Errorf("Failed to get NM Manager: %v", err)
		return fmt.Errorf("Failed to hot reload monitor settings")
	}

	// update operation project
	logger.Info("Updating operation project...")
	baselineClone := *m.baselineCache
	baselineClone.ProjectSetting.MonitorConfiguration.PollingInterval = newPollingInterval
	baselineClone.ProjectSetting.ScanIpRanges = newScanRangeSettings
	res := core.UpdateFullProject(&baselineClone, true)

	if !res.IsSuccess() {
		reloadStatus = hotReloadStatusNeedToStop
		logger.Errorf("Failed to update operation project for project %d: %v", projectId, res)
		return fmt.Errorf("Failed to update operation project info for project %d: %v", projectId, res)
	} else {
		logger.Info("Update operation project... Done")
	}

	operProject, res := core.GetProject(projectId, true, true)
	if !res.IsSuccess() {
		reloadStatus = hotReloadStatusNeedToStop
		logger.Errorf("Failed to get operation project for project %d: %v", projectId, res)
		return fmt.Errorf("Failed to get operation project info for project %d: %v", projectId, res)
	}

	m.currentProject = &operProject

	// stop NM monitor
	logger.Info("Stop NM monitor...")
	stopResult := nmManager.StopMonitor()
	if !stopResult {
		// reloadStatus = hotReloadStatusNeedToStop
		logger.Error("Failed to stop NM monitor during hot reload")
		// return fmt.Errorf("Failed to stop NM monitor during hot reload")
	} else {
		logger.Info("Stop NM monitor... Done")
	}

	// start monitor
	deviceSettings := map[string]parsV1.DeviceMonitorConf{}
	scanRangeSettings := make([]parsV1.ScanRangeWithSetting, 0, len(newScanRangeSettings))
	ipSettings := make([]parsV1.IndividualIp, 0, len(baseline.Devices))

	for i, actDevice := range baseline.Devices {
		mafSNMPVersion, canParseSNMPVersion := configmapper.MapActSNMPVersionToMafSNMPVersion(actDevice.SnmpConfiguration.Version)
		if !canParseSNMPVersion {
			logger.Warnf("invalid SNMP version in device %+v: %v", actDevice, actDevice.SnmpConfiguration.Version)
			continue
		}

		mafSNMPAuthType, canParseAuthType := configmapper.MapActSNMPAuthenticationTypeToMafSNMPAuthenticationType(actDevice.SnmpConfiguration.AuthenticationType)
		if !canParseAuthType {
			logger.Warnf("invalid SNMP auth type in device %+v: %v", actDevice, actDevice.SnmpConfiguration.AuthenticationType)
			continue
		}

		mafEncType, canParseEncType := configmapper.MapActSNMPEncryptionTypeToMafSNMPEncryptionType(actDevice.SnmpConfiguration.DataEncryptionType)
		if !canParseEncType {
			logger.Warnf("invalid SNMP encryption type in device %+v: %v", actDevice, actDevice.SnmpConfiguration.DataEncryptionType)
			continue
		}

		settingKey := fmt.Sprintf("d%d", i)
		deviceSettings[settingKey] = parsV1.DeviceMonitorConf{
			PollingInterval: newPollingInterval,
			DeviceSecrets: dmschema.DeviceSecrets{
				GlobalUsername: actDevice.Account.Username,
				GlobalPassword: actDevice.Account.Password,
				Snmp: &dmschema.SNMP{
					Version:         mafSNMPVersion,
					ReadCommunity:   actDevice.SnmpConfiguration.ReadCommunity,
					WriteCommunity:  actDevice.SnmpConfiguration.WriteCommunity,
					Username:        actDevice.SnmpConfiguration.Username,
					AuthType:        mafSNMPAuthType,
					AuthPassword:    actDevice.SnmpConfiguration.AuthenticationPassword,
					DataEncryptType: mafEncType,
					DataEncryptKey:  actDevice.SnmpConfiguration.DataEncryptionKey,
					Port:            actDevice.SnmpConfiguration.Port,
				},
			},
		}

		ipSettings = append(ipSettings, parsV1.IndividualIp{
			IP: actDevice.Ipv4.IpAddress,
			ScanSetting: parsV1.ScanSetting{
				DeviceSetting:  settingKey,
				AutoEnableSnmp: actDevice.EnableSnmpSetting,
			},
		})
	}

	for i, scanRange := range newScanRangeSettings {
		mafSNMPVersion, canParseSNMPVersion := configmapper.MapActSNMPVersionToMafSNMPVersion(scanRange.SnmpConfiguration.Version)
		if !canParseSNMPVersion {
			logger.Warnf("invalid SNMP version in IP range : %v", scanRange.SnmpConfiguration.Version)
			continue
		}

		mafSNMPAuthType, canParseAuthType := configmapper.MapActSNMPAuthenticationTypeToMafSNMPAuthenticationType(scanRange.SnmpConfiguration.AuthenticationType)
		if !canParseAuthType {
			logger.Warnf("invalid SNMP auth type in IP range : %v", scanRange.SnmpConfiguration.AuthenticationType)
			continue
		}
		mafEncType, canParseEncType := configmapper.MapActSNMPEncryptionTypeToMafSNMPEncryptionType(scanRange.SnmpConfiguration.DataEncryptionType)
		if !canParseEncType {
			logger.Warnf("invalid SNMP encryption type in IP range : %v", scanRange.SnmpConfiguration.DataEncryptionType)
			continue
		}

		deviceSettingName := fmt.Sprintf("r%d", i)
		deviceSettings[deviceSettingName] = parsV1.DeviceMonitorConf{
			PollingInterval: newPollingInterval,
			DeviceSecrets: dmschema.DeviceSecrets{
				GlobalUsername: scanRange.Account.Username,
				GlobalPassword: scanRange.Account.Password,
				Snmp: &dmschema.SNMP{
					Version:         mafSNMPVersion,
					ReadCommunity:   scanRange.SnmpConfiguration.ReadCommunity,
					WriteCommunity:  scanRange.SnmpConfiguration.WriteCommunity,
					Username:        scanRange.SnmpConfiguration.Username,
					AuthType:        mafSNMPAuthType,
					AuthPassword:    scanRange.SnmpConfiguration.AuthenticationPassword,
					DataEncryptType: mafEncType,
					DataEncryptKey:  scanRange.SnmpConfiguration.DataEncryptionKey,
					Port:            scanRange.SnmpConfiguration.Port,
				},
			},
		}

		scanRangeSettings = append(scanRangeSettings, parsV1.ScanRangeWithSetting{
			ScanRange: parsV1.ScanRange{
				StartIP: scanRange.StartIp,
				EndIP:   scanRange.EndIp,
			},
			ScanSetting: parsV1.ScanSetting{
				DeviceSetting:  deviceSettingName,
				AutoEnableSnmp: scanRange.EnableSnmpSetting,
			},
		})
	}

	// clear cache
	m.linkMutexMap.Clear()
	m.mafDevicesCache.Clear()
	m.mafLinksCache.Clear()
	m.currentDeviceMap.Clear()
	m.currentLinkMap.Clear()
	m.sfpCache.Clear()
	m.actDeviceManager.ClearDeviceMappings()
	nmManager.Reset()

	// load baseline devices and links into current cache
	for _, actDevice := range baseline.Devices {
		m.currentDeviceMap.Set(actDevice.Id, &actDevice)
	}

	for _, link := range baseline.Links {
		linkStr, err := m.getActLinkStrInBaseline(&link)
		if err != nil {
			logger.Warnf("Failed to get link identifier string in baseline: %v", err)
			continue
		}

		m.currentLinkMap.Set(linkStr, &link)
	}

	logger.Info("Start NM monitor ...")
	ok, err := nmManager.StartMonitor(true, &parsV1.ListRequest{
		DeviceSettings: deviceSettings,
		ScanList: parsV1.ScanList{
			Ranges:        scanRangeSettings,
			IndividualIps: ipSettings,
		},
	})

	if !ok {
		reloadStatus = hotReloadStatusNeedToStop
		logger.Errorf("Failed to start monitor during hot reload: %v", err)
		return fmt.Errorf("Failed to start monitor during hot reload: %v", err)
		// if failed to start monitor, teardown everything and return error
	} else {
		logger.Info("Start NM monitor... Done")
	}

	logger.Info("Hot reloading monitor settings... Done")
	reloadStatus = hotReloadStatusSuccess
	m.setInternalState(projectId, InternalStateRunning)

	// update polling interval
	m.pollingIntervalNow = newPollingInterval

	// update current scan range settings
	m.scanRangeSettingsNow = newScanRangeSettings

	return nil

}

func (m *DefaultMonitor) pollingScanRangeSettings(ctx context.Context) {
	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warn("Failed to get project ID from context when polling scan range settings")
		return
	}

	ticker := time.NewTicker(5 * time.Second)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			isOperation := true
			showPassword := true
			operationProject, res := core.GetProject(projectId, isOperation, showPassword)
			if !res.IsSuccess() {
				logger.Errorf("Failed to get operation project(%d): %v", projectId, res)
				continue
			}

			originPollingInterval := m.pollingIntervalNow
			newPollingInterval := getValidPollingInterval(operationProject.ProjectSetting.MonitorConfiguration.PollingInterval)

			isPollingIntervalChanged := originPollingInterval != newPollingInterval

			originIPRanges := m.scanRangeSettingsNow
			if originIPRanges == nil {
				continue
			}

			newIPRanges := operationProject.ProjectSetting.ScanIpRanges
			if newIPRanges == nil {
				logger.Warn("polling scan range settings is nil, cannot compare")
				continue
			}

			isIpRangeChanged := !reflect.DeepEqual(originIPRanges, newIPRanges)

			if isPollingIntervalChanged {
				logger.Infof("Polling interval in operation project: %d seconds", operationProject.ProjectSetting.MonitorConfiguration.PollingInterval)
				logger.Infof("Detected polling interval change: origin %d seconds, new %d seconds", originPollingInterval, newPollingInterval)
			}

			if isIpRangeChanged {
				// originJsonBytes, _ := json.MarshalIndent(originIPRanges, "", "  ")
				// newJsonBytes, _ := json.MarshalIndent(newIPRanges, "", "  ")

				// logger.Debugf("Detected scan range settings change:\nOrigin: %s\nNew: %s", string(originJsonBytes), string(newJsonBytes))

				logger.Info("Detected scan range settings change")

				// log origin IP ranges
				logger.Info("Origin scan range settings:")
				logger.Infof("Total %d ranges:", len(originIPRanges))
				for i, ipRange := range originIPRanges {
					logger.Infof("Origin scan range[%d] %s - %s", i, ipRange.StartIp, ipRange.EndIp)
				}

				// log new IP ranges
				logger.Info("New scan range settings:")
				logger.Infof("Total %d ranges:", len(newIPRanges))
				for i, ipRange := range newIPRanges {
					logger.Infof("New scan range[%d] %s - %s", i, ipRange.StartIp, ipRange.EndIp)
				}
			}

			if isPollingIntervalChanged || isIpRangeChanged {
				err := m.hotReloadMonitorSettings(ctx, newIPRanges, newPollingInterval)

				if err != nil {
					logger.Errorf("Failed to hot reload monitor settings: %v", err)
				} else {
					logger.Info("Hot reload monitor settings... Done")
				}
			}

		case <-ctx.Done():
			logger.Info("Stopping scan range settings polling due to context cancellation")
			return
		}
	}
}

func getValidPollingInterval(inputInterval int) int {
	if inputInterval >= 10 && inputInterval <= 600 {
		return inputInterval
	}

	return defaultPollingInterval
}

func (m *DefaultMonitor) Stop(projectId int64) error {
	if projectId <= 0 {
		return fmt.Errorf("invalid project ID: %d", projectId)
	}

	retryTimes := 9
	retryInterval := 200 * time.Millisecond
	tryLockResult := m.mutex.TryLock()

	if !tryLockResult {
		for i := 0; i < retryTimes; i++ {
			time.Sleep(retryInterval)
			logger.Info("Retrying to acquire lock to stop monitor, attempt:", i+1)
			tryLockResult = m.mutex.TryLock()
			if tryLockResult {
				break
			}
		}
	}

	if !tryLockResult {
		internalState, exists := m.getInternalState(projectId)

		if exists {
			if internalState == InternalStateStopping {
				logger.Warn("Monitor is stopping, please wait")
				return ErrStopMonitorStopping
			}
		}

		logger.Warnf("failed to stop monitor, projectId: %v, internalState: %v, stateExists: %v", projectId, internalState, exists)
		return fmt.Errorf("failed to stop monitor")
	}

	defer m.mutex.Unlock()

	m.internalMutex.Lock()
	defer m.internalMutex.Unlock()
	err := m.onStopMonitor(projectId)
	return err
}

func (m *DefaultMonitor) onStopMonitor(projectId int64) (returnErr error) {
	projectInfo, res := core.GetProjectInfo(projectId)
	if !res.IsSuccess() {
		return fmt.Errorf("failed to get project(%v) info: %v", projectId, res)
	}

	defer func() {
		if returnErr == nil || returnErr == ErrStopMonitorNotRunning {
			if projectInfo.ProjectStatus == domain.ProjectStatusEnum_Monitoring.String() {
				res = core.UpdateProjectStatus(projectId, domain.ProjectStatusEnum_Idle)
				if !res.IsSuccess() {
					logger.Errorf("Failed to update project(%v) status to Idle: %v", projectId, res)
				} else {
					logger.Infof("Project(%v) status updated to Idle", projectId)
				}
			}
		}
	}()

	originInternalState, exists := m.getInternalState(projectId)
	if !exists {
		logger.Warnf("Failed to get internal state, projectId: %v", projectId)
		returnErr = ErrStopMonitorNotRunning
		return
	}

	if !m.isMonitoringOrReloading(projectId) {
		logger.Warn("Monitor is not running")
		returnErr = ErrStopMonitorNotRunning
		return
	}

	logger.Info("Stop monitor...")
	m.setInternalState(projectId, InternalStateStopping)

	ctx, cancel := m.context, m.cancelFunc
	if ctx == nil {
		logger.Warn("[onStopMonitor] Monitor context is nil")
	}

	if cancel == nil {
		logger.Warn("[onStopMonitor] Monitor cancel function is nil")
	}

	nmManager, err := dipool.GetNMManager()
	if err != nil {
		m.setInternalState(projectId, originInternalState)
		logger.Errorf("Failed to get NM Manager: %v", err)
		returnErr = fmt.Errorf("Failed to stop monitor")
		return
	}

	valid := nmManager.StopMonitor()
	if !valid {
		// m.setInternalState(originInternalState)

		// 9/23 與 MAF 確認過，即使 StopMonitor 回傳 false，monitor 依舊會是停止的狀態
		logger.Warnf("failed to stop NM monitor")
		// return fmt.Errorf("failed to stop monitor: NM Monitor is not running")
	}

	m.mafDevicesCache.Clear()
	m.mafLinksCache.Clear()
	m.baselineDeviceMap.Clear()
	m.baselineLinkMap.Clear()
	m.currentDeviceMap.Clear()
	m.currentLinkMap.Clear()
	m.sfpCache.Clear()
	m.scanRangeSettingsNow = nil
	m.currentProjectId = projectIdNone
	m.actDeviceManager.ClearDeviceMappings()
	m.UnSubscribeTopologyUpdateData()

	if cancel != nil {
		cancel()
	}

	m.dispatcher.WaitAndClose()

	m.setInternalState(projectId, InternalStateStopped)
	logger.Info("Stop monitor... Done")
	return nil
}

func (m *DefaultMonitor) GetLinkMutex(linkStr string) *sync.Mutex {
	newMutex := &sync.Mutex{}
	linkMutexAny, _ := m.linkMutexMap.LoadOrStore(linkStr, newMutex)
	return linkMutexAny.(*sync.Mutex)
}

func (m *DefaultMonitor) DeleteLinkMutex(linkStr string) {
	m.linkMutexMap.Delete(linkStr)
}
