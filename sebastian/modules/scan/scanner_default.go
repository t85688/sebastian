package scan

import (
	"context"
	"encoding/json"
	"fmt"
	"math"
	"strconv"
	"sync"

	dmschema "gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/network/netmgr/api/parsV1"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/macutility"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/configmapper"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	// devicemanager "gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/manager"
)

// maxConcurrencyTotalQuery int, maxConcurrencyPerDeviceQuery
const defaultMaxConcurrencyTotalQuery = 16
const defaultMaxConcurrencyPerDeviceQuery = 2

type Scanner struct {
	mutex      sync.Mutex
	onProgress func(ctx context.Context, projectId int64, progress int)
	ctx        context.Context
	cancel     context.CancelFunc
}

func (scanner *Scanner) StopScanTopology() StopScanResultStatus {
	ctx := scanner.ctx
	cancelFunc := scanner.cancel
	if ctx == nil {
		return StopScanResultStatusNotScanning
	}

	if cancelFunc == nil {
		return StopScanResultStatusNotScanning
	}

	if ctx.Err() != nil {
		return StopScanResultStatusNotScanning
	}

	cancelFunc()
	logger.Info("ScanTopology cancelled...")
	return StopScanResultStatusCompleted
}

func calculateOverallProgress(partialProgress int, lowerBound int, upperBound int) int {
	if partialProgress < 0 {
		return lowerBound
	}

	if partialProgress > 100 {
		return upperBound
	}

	progress := (float64(partialProgress) / float64(100)) * (float64(upperBound) - float64(lowerBound))
	progressInt := math.Floor(progress)

	overallProgress := lowerBound + int(progressInt)
	if overallProgress > upperBound {
		overallProgress = upperBound
	}

	return overallProgress
}

func OnScanTopologyProgressWithWebsocket(ctx context.Context, projectId int64, progress int) {
	connIdVal := ctx.Value(wscontext.ContextKeyConnId)
	connId, ok := connIdVal.(string)
	if !ok {
		logger.Errorf("[OnScanTopologyProgress] ConnId not found in context")
		return
	}

	if connId == "" {
		logger.Errorf("[OnScanTopologyProgress] ConnId is empty")
		return
	}

	scanResult := wscommand.ScanTopologyResult{
		Progress: progress,
	}

	wsResp := wscommand.NewBaseWsCommandResponse(wscommand.ActWSCommandStartScanTopology.Int64(), 1003, "", scanResult)

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}

func NewScanner(
	onProgress func(ctx context.Context, projectId int64, progress int)) IScanner {
	return &Scanner{
		onProgress: onProgress,
		ctx:        context.Background(),
		cancel:     func() {},
	}
}

func (scanner *Scanner) StartScanTopology(ctxIn context.Context, projectId int64, newTopology bool) (resultItems []*ScanDeviceItem, statusOut ScanResultStatus, errOut error) {
	isCancelled := false
	isBusy := false
	resultItems = []*ScanDeviceItem{}
	actDevices := map[int64]*domain.Device{}
	lastProgress := -1

	defer func() {
		if r := recover(); r != nil {
			resultItems = nil
			err := fmt.Errorf("recovered from panic when Scanning topology: %v", r)
			if errOut != nil {
				errOut = fmt.Errorf("%v; execution error: %v", err, errOut)
			}
			statusOut = ScanResultStatusUnknown
		} else if errOut != nil {
			resultItems = nil
			statusOut = ScanResultStatusUnknown
		} else if isCancelled {
			errOut = errScanCanceled
			resultItems = nil
			statusOut = ScanResultStatusCancel
			logger.Info("ScanTopology cancelled... done")
		} else if isBusy {
			errOut = errScanBusy
			resultItems = nil
			statusOut = ScanResultStatusBusy
			logger.Info("another ScanTopology is in progress")
		} else {
			statusOut = ScanResultStatusCompleted
		}
	}()

	if !scanner.mutex.TryLock() {
		isBusy = true
		return
	}
	defer scanner.mutex.Unlock()

	var ctx context.Context
	var cancel context.CancelFunc
	if ctxIn != nil {
		ctx, cancel = context.WithCancel(ctxIn)
	} else {
		ctx, cancel = context.WithCancel(context.Background())
	}

	scanner.ctx, scanner.cancel = ctx, cancel

	ipRanges, res := core.GetProjectIPScanRanges(projectId)
	if !res.IsSuccess() {
		errOut = fmt.Errorf("failed to get project IP scan ranges: %v", res.ErrorMessage)
		return
	}

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		errOut = fmt.Errorf("failed to get DeviceManager instance: %v", err)
		return
	}

	nmManager, err := dipool.GetNMManager()
	if err != nil {
		errOut = fmt.Errorf("failed to get NetManager instance: %v", err)
		return
	}

	actDeviceManager, err := getActDeviceManager()
	if err != nil {
		errOut = fmt.Errorf("failed to get ActDeviceManager instance: %v", err)
		return
	}

	nmManager.Reset()

	if len(ipRanges.ScanIpRanges) == 0 {
		errOut = fmt.Errorf("no IP ranges found for scanning, projectId: %v", projectId)
		return
	}

	if ctx.Err() != nil {
		isCancelled = true
		return
	}

	logger.Info("delete existing devices and links")
	resDeleteDevices := deleteProjectDevices(projectId)
	if !resDeleteDevices.IsSuccess() {
		logger.Warnf("Failed to delete existing devices in project %d: %s", projectId, resDeleteDevices.ErrorMessage)
		errOut = fmt.Errorf("failed to clear existing devices before scan topology")
		return
	}

	resDeleteLinks := deleteProjectLinks(projectId)
	if !resDeleteLinks.IsSuccess() {
		logger.Warnf("Failed to delete existing links in project %d: %s", projectId, resDeleteLinks.ErrorMessage)
		errOut = fmt.Errorf("failed to clear existing links before scan topology")
		return
	}
	actDeviceManager.ClearDeviceMappings()

	deviceSettings := make(map[string]parsV1.DeviceMonitorConf)
	scanRangeSettings := make([]parsV1.ScanRangeWithSetting, 0, len(ipRanges.ScanIpRanges))

	for idx, ipRange := range ipRanges.ScanIpRanges {
		deviceSettingName := strconv.Itoa(idx)
		mafSNMPVersion, canParseSNMPVersion := configmapper.MapActSNMPVersionToMafSNMPVersion(ipRange.SnmpConfiguration.Version)
		if !canParseSNMPVersion {
			logger.Warnf("invalid SNMP version in IP range: %v", ipRange.SnmpConfiguration.Version)
			continue
		}

		mafSNMPAuthType, canParseAuthType := configmapper.MapActSNMPAuthenticationTypeToMafSNMPAuthenticationType(ipRange.SnmpConfiguration.AuthenticationType)
		if !canParseAuthType {
			logger.Warnf("invalid SNMP auth type in IP range: %v", ipRange.SnmpConfiguration.AuthenticationType)
			continue
		}

		mafEncType, canParseEncType := configmapper.MapActSNMPEncryptionTypeToMafSNMPEncryptionType(ipRange.SnmpConfiguration.DataEncryptionType)
		if !canParseEncType {
			logger.Warnf("invalid SNMP encryption type in IP range: %v", ipRange.SnmpConfiguration.DataEncryptionType)
			continue
		}

		deviceSettings[deviceSettingName] = parsV1.DeviceMonitorConf{
			DeviceSecrets: dmschema.DeviceSecrets{
				GlobalUsername: ipRange.Account.Username,
				GlobalPassword: ipRange.Account.Password,
				Snmp: &dmschema.SNMP{
					Version:         mafSNMPVersion,
					ReadCommunity:   ipRange.SnmpConfiguration.ReadCommunity,
					WriteCommunity:  ipRange.SnmpConfiguration.WriteCommunity,
					Username:        ipRange.SnmpConfiguration.Username,
					AuthType:        mafSNMPAuthType,
					AuthPassword:    ipRange.SnmpConfiguration.AuthenticationPassword,
					DataEncryptType: mafEncType,
					DataEncryptKey:  ipRange.SnmpConfiguration.DataEncryptionKey,
					Port:            ipRange.SnmpConfiguration.Port,
				},
			},
		}

		scanRangeSettings = append(scanRangeSettings, parsV1.ScanRangeWithSetting{
			ScanRange: parsV1.ScanRange{
				StartIP: ipRange.StartIp,
				EndIP:   ipRange.EndIp,
			},
			ScanSetting: parsV1.ScanSetting{
				DeviceSetting:  deviceSettingName,
				AutoEnableSnmp: ipRange.EnableSnmpSetting,
			},
		})
	}

	listReq := parsV1.ListRequest{
		DeviceSettings: deviceSettings,
		ScanList: parsV1.ScanList{
			Ranges: scanRangeSettings,
		},
	}

	if ctx.Err() != nil {
		isCancelled = true
		return
	}

	deviceDisCoveryTask, err := nmManager.DeviceDiscovery( /* isSelfWawre */ false, &listReq)
	if err != nil {
		errOut = fmt.Errorf("failed to start device discovery: %v", err)
		return
	}

	deviceDiscoveryProgress := 0
deviceDiscoveryLoop:
	for {
		select {
		case deviceDiscoveryProgress = <-deviceDisCoveryTask.ProgressChan():
			logger.Infof("Device Discovery Progress: %d%%", deviceDiscoveryProgress)

			currentProgress := calculateOverallProgress(deviceDiscoveryProgress, 0, 30)
			if currentProgress > lastProgress {
				scanner.onProgress(ctx, projectId, currentProgress)
				lastProgress = currentProgress
			}
		case <-deviceDisCoveryTask.Done():
			logger.Info("Device Discovery completed")
			break deviceDiscoveryLoop
		}
	}

	logger.Infof("final device discovery progress: %d%%, total progress: %d%%", deviceDiscoveryProgress, calculateOverallProgress(deviceDiscoveryProgress, 0, 30))

	mafDevices := nmManager.GetAllDevices()

	if len(mafDevices) == 0 {
		logger.Info("No devices found")
		return
	}

	if ctx.Err() != nil {
		isCancelled = true
		return
	}

	linkDiscoveryTask, err := nmManager.LinkDiscovery(false)
	if err != nil {
		logger.Error("Error starting link discovery:", err)
		errOut = fmt.Errorf("failed to start link discovery: %v", err)
		return
	}

	linkDiscoveryProgress := 0
linkDiscoveryLoop:
	for {
		select {
		case linkDiscoveryProgress = <-linkDiscoveryTask.ProgressChan():
			currentProgress := calculateOverallProgress(linkDiscoveryProgress, 30, 70)
			if currentProgress > lastProgress {
				scanner.onProgress(ctx, projectId, currentProgress)
				lastProgress = currentProgress
			}

			logger.Infof("Link Discovery Progress: %d%%\n", linkDiscoveryProgress)
		case <-linkDiscoveryTask.Done():
			logger.Info("Link Discovery completed")
			break linkDiscoveryLoop
		}
	}

	logger.Infof("final link discovery progress: %d%%, total progress: %d%%", linkDiscoveryProgress, calculateOverallProgress(linkDiscoveryProgress, 30, 70))

	if ctx.Err() != nil {
		isCancelled = true
		return
	}

	// Get Device Simple Profile
	// todo: 抽到初始化的地方，取一次就好
	simpleProfiles, status := core.GetSimpleDeviceProfiles()
	if !status.IsSuccess() {
		logger.Error("Failed to get simple device profile:", status.ErrorMessage)
		errOut = fmt.Errorf("failed to get simple device profile")
		return
	}

	unknownProfile := getUnknownDeviceProfile(simpleProfiles)
	if unknownProfile == nil {
		logger.Error("Unknown device profile not found")
		errOut = fmt.Errorf("unknown device profile not found")
		return
	}

	// fetch device basic info
	for _, mafDevice := range mafDevices {
		fetchDevice, _, err := dmManager.FetchDeviceInfo(mafDevice.DeviceId)
		if err != nil {
			logger.Errorf("Failed to fetch device info for device %s: %v, model: %s", mafDevice.DeviceId, err, mafDevice.ModelName)
		} else {
			if mafDevice.Location == nil {
				mafDevice.Location = fetchDevice.Location
			}

			if mafDevice.FirmwareVersion == "" {
				mafDevice.FirmwareVersion = fetchDevice.FirmwareVersion
			}

			if mafDevice.DeviceName == nil {
				mafDevice.DeviceName = fetchDevice.DeviceName
			}

			if mafDevice.ProductRevision == "" {
				mafDevice.ProductRevision = fetchDevice.ProductRevision
			}

			if mafDevice.MAC == "" {
				mafDevice.MAC = fetchDevice.MAC
			}
		}

		modules, err := GetModules(mafDevice.DeviceId)
		if err != nil {
			logger.Errorf("Failed to get device modules for device %s(%s): %v, model: %s", mafDevice.DeviceId, mafDevice.IP, err, mafDevice.ModelName)
		} else {
			mafDevice.Modules = modules
		}

	}

	logger.Info("Device List: ")
	// Add Device to the project
	for _, mafDevice := range mafDevices {
		// call Qt api to create device
		deviceInfo := fmt.Sprintf("IP: %v, ModelName: %v, DeviceName: %v, SerialNumber: %v, FirmwareVersion: %v, Location: %v, MAC: %v",
			mafDevice.IP,
			mafDevice.ModelName,
			mafDevice.DeviceName,
			mafDevice.SerialNumber,
			mafDevice.FirmwareVersion,
			mafDevice.Location,
			mafDevice.MAC)

		logger.Info(deviceInfo)

		profile := common.FindDeviceProfileByModelNameAndModules(simpleProfiles, mafDevice.ModelName, mafDevice.Modules)
		if profile == nil {
			logger.Warnf("Device %s (%s) does not have a specific profile, using Unknown profile", mafDevice.ModelName, mafDevice.IP)
			profile = unknownProfile
		}
		logger.Infof("Using profile: %s, ProjectId: %v", profile.ModelName, projectId)

		// Generate ModularConfiguration
		var modularConfiguration *domain.ModularConfiguration
		if mafDevice.Modules != nil {
			var err error
			modularConfiguration, err = configmapper.MappingDeviceModules(mafDevice.Modules)
			if err != nil {
				logger.Warnf("Failed to map device modules for maf device %s(%s): %v", mafDevice.DeviceId, mafDevice.IP, err)
			}
		}

		// parse mac address
		macAddress := ""
		if mafDevice.MAC != "" {
			parsedMacAddress, err := macutility.ParseMACAddress(mafDevice.MAC)
			if err == nil {
				macAddress = parsedMacAddress.Dashes(macutility.UpperCase)
			} else {
				logger.Warnf("Invalid MAC address format: %v, device: %v, mac: %v", err, mafDevice.IP, mafDevice.MAC)
			}
		}

		is_operation := false
		from_bag := false
		actDeviceConf := domain.DeviceConf{
			MacAddress:      macAddress,
			DeviceAlias:     "", // mafDevice.DeviceName,
			ModelName:       profile.ModelName,
			FirmwareVersion: mafDevice.FirmwareVersion,
			DeviceProfileId: int64(profile.Id),
			Ipv4: domain.ActIpv4{
				IpAddress: mafDevice.IP,
			},
		}

		if modularConfiguration != nil {
			actDeviceConf.ModularConfiguration = *modularConfiguration
		}

		if mafDevice.DeviceName != nil {
			actDeviceConf.DeviceName = *mafDevice.DeviceName
		}
		actDevice, actStatus := core.CreateDevice(projectId, actDeviceConf, from_bag, is_operation)

		if !actStatus.IsSuccess() {
			logger.Error("Failed to create device:", mafDevice.IP, actStatus.StatusCode, actStatus.ErrorMessage)
			continue
		} else {
			logger.Infof("ACT Device created successfully: %s", actDevice.Ipv4.IpAddress)
		}

		// workaround begin: 目前 CreateDevice 回傳值沒有接 Device.DeviceProperty.ModelName
		actDevice.ModelName = profile.ModelName
		// workaround end
		actDevices[actDevice.Id] = &actDevice

		// mapping maf device id to act device id
		actDeviceManager.AddDeviceMapping(actDevice.Id, mafDevice.DeviceId)
	}

	if ctx.Err() != nil {
		isCancelled = true
		return
	}

	mafLinks := nmManager.GetAllLinks()
	logger.Info("Link List: ")
	for _, link := range mafLinks {
		logger.Infof("%+v", link.GetExactIdentity())
	}

	// Add Link to the project
	localhostIP := "127.0.0.1"
	for _, mafLink := range mafLinks {

		if mafLink.FromDevice.IP == localhostIP || mafLink.ToDevice.IP == localhostIP {
			logger.Warnf("Skipping link with localhost IP: %s - %s", mafLink.FromDevice.IP, mafLink.ToDevice.IP)
			continue
		}

		ds := nmManager.GetDevicesByIP(mafLink.FromDevice.IP)
		if len(ds) == 0 {
			logger.Warnf("MAF Source device IP %s not found in mapping", mafLink.FromDevice.IP)
			continue
		}

		sourceMafDeviceId := ds[0].DeviceId
		sourceActDeviceId, ok := actDeviceManager.GetActDeviceID(sourceMafDeviceId)
		if !ok {
			logger.Warnf("MAF Source device ID %s not found in mapping", mafLink.FromDevice.DeviceId)
			continue
		}

		ds = nmManager.GetDevicesByIP(mafLink.ToDevice.IP)
		if len(ds) == 0 {
			logger.Warnf("MAF Target device IP %s not found in mapping", mafLink.ToDevice.IP)
			continue
		}

		targetMafDeviceId := ds[0].DeviceId
		targetActDeviceId, ok := actDeviceManager.GetActDeviceID(targetMafDeviceId)
		if !ok {
			logger.Warnf("MAF Target device ID %s not found in mapping", mafLink.ToDevice.DeviceId)
			continue
		}

		speed := int(mafLink.Speed / 1000000) // convert to Mbps

		// workaround begin: 目前 mafLink speed 都是 0，先用 support speed
		if speed <= 0 {
			sourceActDevice, ok := actDevices[sourceActDeviceId]
			if !ok {
				logger.Warnf("ACT Source device ID %d not found in actDevices when creating link", sourceActDeviceId)
				continue
			}

			targetActDevice, ok := actDevices[targetActDeviceId]
			if !ok {
				logger.Warnf("ACT Target device ID %d not found in actDevices when creating link", targetActDeviceId)
				continue
			}

			// 取兩台設備的支援速度共同較高的當作 link speed
			hasCommonSpeed := false
			commonSpeed := 0
			sourceSupportSpeeds := map[int]bool{}
			for _, iface := range sourceActDevice.Interfaces {
				if iface.InterfaceId == mafLink.FromPort {
					for _, supportSpeed := range iface.SupportSpeeds {
						sourceSupportSpeeds[supportSpeed] = true
					}
				}
			}

			for _, iface := range targetActDevice.Interfaces {
				if iface.InterfaceId == mafLink.ToPort {
					for _, supportSpeed := range iface.SupportSpeeds {
						if sourceSupportSpeeds[supportSpeed] {
							hasCommonSpeed = true
							if supportSpeed > commonSpeed {
								commonSpeed = supportSpeed
							}
						}
					}
				}
			}

			if !hasCommonSpeed {
				logger.Warnf("No common speed found for link (%v:%v - %v:%v), skip creating this link", sourceActDeviceId, mafLink.FromPort, targetActDeviceId, mafLink.ToPort)
				continue
			}

			speed = commonSpeed
		}
		// workaround end

		if speed <= 0 {
			logger.Warnf("Invalid link speed (%d Mbps) for link (%v:%v - %v:%v), skip creating this link", speed, sourceActDeviceId, mafLink.FromPort, targetActDeviceId, mafLink.ToPort)
			continue
		}

		actLinkConf := domain.LinkConf{
			SourceDeviceId:         int(sourceActDeviceId),
			SourceInterfaceId:      mafLink.FromPort,
			DestinationDeviceId:    int(targetActDeviceId),
			DestinationInterfaceId: mafLink.ToPort,
			Speed:                  speed,
		}

		is_operation := false

		createdLink, res := core.CreateLink(projectId, actLinkConf, is_operation)

		if !res.IsSuccess() {
			logger.Warnf("Failed to create link (%v:%v - %v:%v): %v",
				actLinkConf.SourceDeviceId,
				actLinkConf.SourceInterfaceId,
				actLinkConf.DestinationDeviceId,
				actLinkConf.DestinationInterfaceId,
				res.ErrorMessage)
		} else {
			logger.Infof("ACT Link created successfully: (%v:%v - %v:%v)", createdLink.SourceDeviceId, createdLink.SourceInterfaceId, createdLink.DestinationDeviceId, createdLink.DestinationInterfaceId)
		}
	}

	if ctx.Err() != nil {
		isCancelled = true
		return
	}

	deviceConfig, err := scanner.batchQueryDeviceConfiguration(ctx, projectId, actDevices)

	if err != nil {
		logger.Error("Failed to batch query device configuration:", err)
		errOut = fmt.Errorf("failed to batch query device configuration: %v", err)
		return
	}

	scanner.updateDeviceConfiguration(ctx, projectId, deviceConfig)

	deviceIpv4Map := make(map[int64]map[string]any, len(deviceConfig.NetworkSettingTables))

	// update device IPv4
	if deviceConfig.NetworkSettingTables != nil {
		for deviceId, table := range deviceConfig.NetworkSettingTables {
			device, exists := actDevices[deviceId]
			if !exists {
				logger.Warnf("Device ID %d not found in actDevices when updating IPv4", deviceId)
				continue
			}

			ipv4 := domain.ActIpv4{
				IpAddress:  device.Ipv4.IpAddress,
				SubnetMask: table.SubnetMask,
				Gateway:    table.Gateway,
				DNS1:       table.DNS1,
				DNS2:       table.DNS2,
			}

			deviceIpv4Map[deviceId] = map[string]any{
				"Ipv4": ipv4,
			}
		}
	}

	if len(deviceIpv4Map) > 0 {
		res := core.PartialUpdateDevices(projectId, deviceIpv4Map, false)
		if !res.IsSuccess() {
			logger.Error("Failed to update device IPv4:", res.ErrorMessage)
			errOut = fmt.Errorf("failed to update device IPv4: %v", res.ErrorMessage)
			return
		}
	}

	scanner.onProgress(ctx, projectId, 95)

	if ctx.Err() != nil {
		isCancelled = true
		return
	}

	// workaround begin: 目前 delete devices 可能會失敗，導致 create device 失敗，先用 mafDevice 顯示
	// resultItems = make([]*ScanDeviceItem, 0, len(mafDevices))
	// for _, mafDevice := range mafDevices {
	// 	resultItems = append(resultItems, &ScanDeviceItem{
	// 		FirmwareVersion: mafDevice.FirmwareVersion,
	// 		IP:              mafDevice.IP,
	// 		MacAddress:      mafDevice.MAC,
	// 		ModelName:       mafDevice.ModelName,
	// 	})
	// }

	// workaround end

	resultItems = make([]*ScanDeviceItem, 0, len(actDevices))
	for _, actDevice := range actDevices {
		resultItems = append(resultItems, &ScanDeviceItem{
			FirmwareVersion: actDevice.FirmwareVersion,
			IP:              actDevice.Ipv4.IpAddress,
			MacAddress:      actDevice.MacAddress,
			ModelName:       actDevice.ModelName,
		})
	}

	return
}

func (scanner *Scanner) batchQueryDeviceConfiguration(ctx context.Context, projectId int64, actDeviceMap map[int64]*domain.Device) (*domain.DeviceConfig, error) {
	// query Device Settings
	queryScheduler := NewDefaultScannerBatchJobRunner(defaultMaxConcurrencyTotalQuery, defaultMaxConcurrencyPerDeviceQuery, actDeviceMap)
	deviceConfig, err := queryScheduler.BatchQuery(ctx, projectId, scanner.onProgress)
	if err != nil {
		logger.Error("Failed to query device config:", err)
		return nil, err
	}

	return deviceConfig, nil
}

func (scanner *Scanner) updateDeviceConfiguration(ctx context.Context, projectId int64, deviceConfig *domain.DeviceConfig) {
	// query Device Settings
	projectWithDeviceConfig := domain.ProjectWithDeviceConfig{
		Id: projectId,
	}

	projectWithDeviceConfig.DeviceConfig = *deviceConfig
	scanner.onProgress(ctx, projectId, 90)

	isOperation := false
	res := core.UpdateProjectDeviceConfig(&projectWithDeviceConfig, isOperation)
	if !res.IsSuccess() {
		logger.Error("Failed to update project device config:", res.ErrorMessage)
		return
	} else {
		logger.Info("Project device config updated successfully")
	}
}

func getUnknownDeviceProfile(simpleProfiles *domain.SimpleDeviceProfiles) *domain.SimpleDeviceProfile {
	for _, profile := range simpleProfiles.Profiles {
		if profile.ModelName == "Unknown" {
			return &profile
		}
	}

	return nil
}

func deleteProjectLinks(projectId int64) statuscode.Response {
	is_operation := false
	res := core.DeleteAllLinks(projectId, is_operation)
	return res
}

func deleteProjectDevices(projectId int64) statuscode.Response {
	is_operation := false
	res := core.DeleteAllDevices(projectId, is_operation)
	return res
}
