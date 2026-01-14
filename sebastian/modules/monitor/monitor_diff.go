package monitor

import (
	"context"
	"encoding/json"
	"fmt"
	"reflect"
	"time"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/macutility"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/configmapper"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/network/nettwin/topic"
)

const localhostIP = "127.0.0.1"

var (
	errContextDone = fmt.Errorf("context done")
)

func (m *DefaultMonitor) SubscribeTopologyUpdateData(ctx context.Context) error {
	projectId, projectExists := getProjectId(ctx)
	if !projectExists {
		return fmt.Errorf("No project ID found in context, cannot subscribe topology update data")
	}

	if projectId == projectIdNone {
		return fmt.Errorf("Invalid project ID in context, cannot subscribe topology update data")
	}

	ipcClient, _ := dipool.GetMAFIPCClient()
	ipcClient.Subscribe(topic.DiffTopic, func(topic string, payload []byte) {
		if !m.isMonitoringOrReloading(projectId) {
			logger.Warnf("Not in monitoring state, ignore the diff data, %s", string(payload))
			return
		}

		if ctx.Err() != nil {
			logger.Warnf("Context is done, ignore the diff data, %s", string(payload))
			return
		}

		netdiffOutput := &netdl.DiffOutput{}
		err := json.Unmarshal(payload, netdiffOutput)
		if err != nil {
			logger.Errorf("Failed to unmarshal netdiff output: %v", err)
			return
		}

		if netdiffOutput == nil {
			return
		}

		if netdiffOutput.Devices != nil {
			m.onAddMafDevices(ctx, netdiffOutput.Devices.Added)
			m.onUpdateMafDevice(ctx, netdiffOutput.Devices.Updated)
			m.onDeleteMafDevice(ctx, netdiffOutput.Devices.Deleted)
		}

		if netdiffOutput.Links != nil {
			m.onAddMafLink(ctx, netdiffOutput.Links.Added)
			m.onUpdateMafLink(ctx, netdiffOutput.Links.Updated)
			m.onDeleteMafLink(ctx, netdiffOutput.Links.Deleted)
		}
	})

	return nil
}

func (m *DefaultMonitor) UnSubscribeTopologyUpdateData() {
	ipcClient, _ := dipool.GetMAFIPCClient()
	ipcClient.UnSubscribe(topic.DiffTopic)
}

func (m *DefaultMonitor) onAddMafDevices(ctx context.Context, addedDevices []*netdl.Device) {
	if ctx.Err() != nil {
		return
	}

	if len(addedDevices) == 0 {
		return
	}

	for _, addedDevice := range addedDevices {
		m.mafDevicesCache.Set(addedDevice.DeviceId, addedDevice)
		logger.Infof("Added device: %v", addedDevice.IP)
	}
}

func (m *DefaultMonitor) onUpdateMafDevice(ctx context.Context, updatedDevices []*netdl.Device) {
	if ctx.Err() != nil {
		return
	}

	if len(updatedDevices) == 0 {
		return
	}

	for _, updatedDevice := range updatedDevices {
		logger.Infof("Updated device: %v", updatedDevice.IP)

		_, mafDeviceExists := m.mafDevicesCache.Get(updatedDevice.DeviceId)
		if !mafDeviceExists {
			logger.Warnf("No MAF device found for updated device: %v", updatedDevice.DeviceId)
			continue
		}

		m.mafDevicesCache.Set(updatedDevice.DeviceId, updatedDevice)
	}

}

func (m *DefaultMonitor) onDeleteMafDevice(ctx context.Context, deletedDevices []*netdl.Device) {
	if ctx.Err() != nil {
		return
	}

	if len(deletedDevices) == 0 {
		return
	}

	for _, deletedDevice := range deletedDevices {
		logger.Infof("Deleted device: %v", deletedDevice.IP)
		m.mafDevicesCache.Delete(deletedDevice.DeviceId)
	}
}

func (m *DefaultMonitor) onAddMafLink(ctx context.Context, addedLinks []*netdl.Link) {
	if ctx.Err() != nil {
		return
	}

	if len(addedLinks) == 0 {
		return
	}

	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warnf("[onAddMafLink] No project ID found in context")
		return
	}

	baseline := m.baselineCache

	if baseline == nil {
		return
	}

	for _, addedLink := range addedLinks {
		// add to maf link cache
		mafLinkStr, err := getMafLinkIdentifier(addedLink)
		if err != nil {
			logger.Errorf("Failed to get MAF link identifier: %v", err)
			continue
		}

		m.mafLinksCache.Set(mafLinkStr, addedLink)

		logger.Infof("Added link: %v", mafLinkStr)

		if addedLink.FromDevice.IP == localhostIP || addedLink.ToDevice.IP == localhostIP {
			logger.Warnf("Ignore create link with localhost device: %v", addedLink)
			continue
		}

		isAlive := addedLink.Status == netdl.LinkStatusUp.String()
		hasSpeed := addedLink.Speed > 0
		_, currentLinkExists := m.findActLinkInCurrentByMafLink(addedLink)

		if !currentLinkExists && isAlive && hasSpeed {
			linkMutex := m.GetLinkMutex(mafLinkStr)
			linkMutex.Lock()

			// create act link
			actLink, err := m.createActLinkByMafLink(projectId, addedLink)
			if err != nil {
				logger.Errorf("Failed to create act link by maf link: %v", err)
				linkMutex.Unlock()
				continue
			}

			// add to current link map
			m.currentLinkMap.Set(mafLinkStr, actLink)

			// onAddLink
			m.onCreateLink(ctx, actLink)

			linkMutex.Unlock()
		}
	}

}

func (m *DefaultMonitor) onUpdateMafLink(ctx context.Context, updatedLinks []*netdl.Link) {
	if ctx.Err() != nil {
		return
	}

	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warnf("[onUpdateMafLink] No project ID found in context")
		return
	}

	if len(updatedLinks) == 0 {
		return
	}

	for _, updatedLink := range updatedLinks {
		mafLinkStr, err := getMafLinkIdentifier(updatedLink)
		if err != nil {
			logger.Errorf("Failed to get MAF link identifier: %v", err)
			continue
		}

		logger.Infof("Updated link: %v", mafLinkStr)

		_, mafLinkExist := m.mafLinksCache.Get(mafLinkStr)
		if !mafLinkExist {
			logger.Warnf("No MAF link found for updated link: %v", mafLinkStr)
			continue
		}

		m.mafLinksCache.Set(mafLinkStr, updatedLink)

		if updatedLink.FromDevice.IP == localhostIP || updatedLink.ToDevice.IP == localhostIP {
			logger.Warnf("Ignore update link with localhost device: %v", updatedLink)
			continue
		}

		hasSpeed := updatedLink.Speed > 0
		isAlive := updatedLink.Status == netdl.LinkStatusUp.String()
		_, existsInBaseline := m.baselineLinkMap.Get(mafLinkStr)
		currentLink, currentLinkExists := m.findActLinkInCurrentByMafLink(updatedLink)

		if !currentLinkExists && isAlive && hasSpeed {
			linkMutex := m.GetLinkMutex(mafLinkStr)
			linkMutex.Lock()

			// create link
			actLink, err := m.createActLinkByMafLink(projectId, updatedLink)

			if err != nil {
				logger.Errorf("Failed to create act link by maf link(%v): %v", mafLinkStr, err)
				linkMutex.Unlock()
				continue
			}

			m.currentLinkMap.Set(mafLinkStr, actLink)
			m.onCreateLink(ctx, actLink)
			linkMutex.Unlock()
		}

		if currentLinkExists && !existsInBaseline && !isAlive {
			linkMutex := m.GetLinkMutex(mafLinkStr)
			linkMutex.Lock()

			// delete link
			res := core.DeleteLink(projectId, int64(currentLink.Id), true)
			if !res.IsSuccess() {
				logger.Errorf("Failed to delete link(%v): %v", mafLinkStr, res)
				linkMutex.Unlock()
				continue
			}

			m.currentLinkMap.Delete(mafLinkStr)
			m.onDeleteLink(ctx, currentLink)
			// m.DeleteLinkMutex(mafLinkStr)
			linkMutex.Unlock()
		}

	}
}

func (m *DefaultMonitor) onDeleteMafLink(ctx context.Context, deletedLinks []*netdl.Link) {
	if ctx.Err() != nil {
		return
	}

	if len(deletedLinks) == 0 {
		return
	}

	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warnf("[onDeleteMafLink] No project ID found in context")
		return
	}

	baseline := m.baselineCache
	if baseline == nil {
		return
	}

	for _, deletedLink := range deletedLinks {
		mafLinkStr, err := getMafLinkIdentifier(deletedLink)
		if err != nil {
			logger.Errorf("Failed to get MAF link identifier: %v", err)
			continue
		}

		logger.Infof("Deleted link: %v", mafLinkStr)

		m.mafLinksCache.Delete(mafLinkStr)

		currentLink, currentLinkExists := m.findActLinkInCurrentByMafLink(deletedLink)
		if !currentLinkExists {
			logger.Warnf("No active link found for deleted link: %v", deletedLink)
			continue
		}

		if currentLinkExists {
			// find link in baseline
			_, existsInBaseline := m.baselineLinkMap.Get(mafLinkStr)
			if !existsInBaseline {
				linkMutex := m.GetLinkMutex(mafLinkStr)
				linkMutex.Lock()

				// delete link
				res := core.DeleteLink(projectId, int64(currentLink.Id), true)
				if !res.IsSuccess() {
					logger.Errorf("Failed to delete link: %v", res)
					linkMutex.Unlock()
					continue
				}
				m.currentLinkMap.Delete(mafLinkStr)

				// onDeleteLink
				m.onDeleteLink(ctx, currentLink)
				// m.DeleteLinkMutex(mafLinkStr)
				linkMutex.Unlock()
				logger.Infof("Deleted act link: %v(%v)", currentLink.Id, mafLinkStr)
			}
		}
	}

}

func (m *DefaultMonitor) createActDeviceByMafDevice(projectId int64, mafDevice *netdl.Device) (*domain.Device, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		logger.Errorf("Failed to get DMManager: %v", err)
		return nil, err
	}

	// fetch device info
	deviceInfo, _, err := dmManager.FetchDeviceInfo(mafDevice.DeviceId)
	if err != nil {
		logger.Warnf("Failed to fetch device info: %v, mafDeviceID: %v", err, mafDevice.DeviceId)
	} else {
		if mafDevice.Location == nil {
			mafDevice.Location = deviceInfo.Location
		}

		if mafDevice.FirmwareVersion == "" {
			mafDevice.FirmwareVersion = deviceInfo.FirmwareVersion
		}

		if mafDevice.DeviceName == nil {
			mafDevice.DeviceName = deviceInfo.DeviceName
		}

		if mafDevice.ProductRevision == "" {
			mafDevice.ProductRevision = deviceInfo.ProductRevision
		}

		if mafDevice.MAC == "" {
			mafDevice.MAC = deviceInfo.MAC
		}
	}

	actDeviceType := getActDeviceType(mafDevice.DeviceType)
	modelName := mafDevice.ModelName
	if actDeviceType == ActDeviceTypeICMP || actDeviceType == ActDeviceTypeMoxa {
		modelName = actDeviceType.ModelName()
	}

	// find device profile by model name
	profile := common.FindDeviceProfileByModelNameAndModules(m.simpleProfiles, modelName, mafDevice.Modules)
	if profile == nil {
		profile = getUnknownDeviceProfile(m.simpleProfiles)
		if profile == nil {
			logger.Errorf("Failed to find device profile for unknown model")
			return nil, fmt.Errorf("failed to find device profile for unknown model")
		}
	}

	macAddress := ""
	if mafDevice.MAC != "" {
		parsedMacAddress, err := macutility.ParseMACAddress(mafDevice.MAC)
		if err == nil {
			macAddress = parsedMacAddress.Dashes(macutility.UpperCase)
		} else {
			logger.Warnf("Invalid MAC address format: %v, device: %v, mac: %v", err, mafDevice.IP, mafDevice.MAC)
		}
	}

	// create device
	deviceConf := domain.DeviceConf{
		MacAddress:      macAddress,
		DeviceAlias:     "", // mafDevice.DeviceName,
		ModelName:       profile.ModelName,
		FirmwareVersion: mafDevice.FirmwareVersion,
		DeviceProfileId: int64(profile.Id),
		Ipv4: domain.ActIpv4{
			IpAddress: mafDevice.IP,
		},
	}
	if mafDevice.DeviceName != nil {
		deviceConf.DeviceName = *mafDevice.DeviceName
	}

	fromBag := false
	isOperation := true
	newActDevice, res := core.CreateDevice(projectId, deviceConf, fromBag, isOperation)
	if !res.IsSuccess() {
		logger.Errorf("Failed to create device: %v", res)
		return nil, fmt.Errorf("failed to create device: %v", res)
	}

	return &newActDevice, nil
}

func (m *DefaultMonitor) createActLink(projectId int64, linkConf *domain.LinkConf) (*domain.Link, error) {
	linkInfo, res := core.CreateLink(projectId, *linkConf, true)

	linkstr := fmt.Sprintf("%v:%v-%v:%v", linkConf.SourceDeviceId, linkConf.SourceInterfaceId, linkConf.DestinationDeviceId, linkConf.DestinationInterfaceId)

	if !res.IsSuccess() {
		logger.Errorf("Failed to create link(%v): %v", linkstr, res)
		return nil, fmt.Errorf("failed to create link(%v): %v", linkstr, res)
	}

	// get created link
	actLink, res := core.GetLink(projectId, int64(linkInfo.Id), true)
	if !res.IsSuccess() {
		logger.Errorf("Failed to get created link(%v): %v", linkstr, res)
		return nil, fmt.Errorf("failed to get created link(%v): %v", linkstr, res)
	}

	return &actLink, nil
}

func (m *DefaultMonitor) createActLinkByMafLink(projectId int64, mafLink *netdl.Link) (*domain.Link, error) {
	mafLinkStr, err := getMafLinkIdentifier(mafLink)
	if err != nil {
		return nil, fmt.Errorf("failed to get MAF link identifier: %v", err)
	}
	srcActDeviceId, fromExists := m.findActDeviceInCurrentByIP(mafLink.FromDevice.IP)
	if !fromExists {
		logger.Warnf("No source device found for added link: %v", mafLinkStr)
		return nil, fmt.Errorf("no source act device found for added link: %v", mafLinkStr)
	}

	dstActDeviceId, toExists := m.findActDeviceInCurrentByIP(mafLink.ToDevice.IP)
	if !toExists {
		logger.Warnf("No destination device found for added link: %v", mafLinkStr)
		return nil, fmt.Errorf("no destination act device found for added link: %v", mafLinkStr)
	}

	if mafLink.Speed <= 0 {
		return nil, fmt.Errorf("invalid link (%v) speed: %v", mafLinkStr, mafLink.Speed)
	}

	// Create ActLink
	linkConf := domain.LinkConf{
		SourceDeviceId:         int(srcActDeviceId),
		SourceInterfaceId:      mafLink.FromPort,
		DestinationDeviceId:    int(dstActDeviceId),
		DestinationInterfaceId: mafLink.ToPort,
		Speed:                  int(mafLink.Speed / 1000000), // convert to Mbps
	}

	linkInfo, res := core.CreateLink(projectId, linkConf, true)
	if !res.IsSuccess() {
		logger.Errorf("Failed to create link(%v): %v", mafLinkStr, res)
		return nil, fmt.Errorf("failed to create link(%v): %v", mafLinkStr, res)
	}

	// get created link
	actLink, res := core.GetLink(projectId, int64(linkInfo.Id), true)
	if !res.IsSuccess() {
		logger.Errorf("Failed to get created link(%v): %v", mafLinkStr, res)
		return nil, fmt.Errorf("failed to get created link(%v): %v", mafLinkStr, res)
	}

	return &actLink, nil
}

func (m *DefaultMonitor) periodicallyUpdateTopology(ctx context.Context, interval time.Duration) {
	if interval < 3*time.Second {
		interval = 10 * time.Second
	}

	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warnf("No project found in context, cannot start periodically update topology, projectId: %v", projectId)
		return
	}

	ticker := time.NewTicker(interval)
	defer ticker.Stop()

	logger.Infof("Start periodically update topology, projectId: %v", projectId)

	defer func() {
		logger.Infof("Context is done, Stop periodically update topology, projectId: %v", projectId)
	}()

	for {
		select {
		case <-ticker.C:
			if ctx.Err() != nil {
				return
			}
			m.syncLatestMafDevicesAndLinks(ctx)

			if ctx.Err() != nil {
				return
			}
			m.updateTopologyDevice(ctx)

			if ctx.Err() != nil {
				return
			}
			m.updateDeviceInfo(ctx)

			if ctx.Err() != nil {
				return
			}
			m.updateDeviceConfig(ctx)

			if ctx.Err() != nil {
				return
			}
			m.updateTopologyLink(ctx)

			if ctx.Err() != nil {
				return
			}
			m.updateLinkInfo(ctx)
		case <-ctx.Done():
			return
		}
	}
}

func (m *DefaultMonitor) syncLatestMafDevicesAndLinks(ctx context.Context) {
	if ctx.Err() != nil {
		return
	}

	if !m.internalMutex.TryLock() {
		logger.Warnf("failed to acquire lock for syncing latest maf devices and links, skip this round")
		return
	}

	defer m.internalMutex.Unlock()

	if ctx.Err() != nil {
		return
	}
	logger.Info("Sync latest MAF devices and links...")

	nmManager, err := dipool.GetNMManager()
	if err != nil {
		logger.Errorf("[syncLatestMafDevicesAndLinks] Failed to get NMManager: %v", err)
		return
	}

	mafDevices := nmManager.GetAllDevices()
	mafLinks := nmManager.GetAllLinks()

	if ctx.Err() != nil {
		logger.Warnf("context error occurred, skip getting latest maf devices and links: %v", ctx.Err())
		return
	}

	logger.Info("Sync latest MAF devices...")

	logger.Info("Total MAF devices:", len(mafDevices))
	for _, d := range mafDevices {
		logger.Infof("MAF Device: ID=%s, IP=%s, Model=%s", d.DeviceId, d.IP, d.ModelName)
	}

	mafDevicesMap := make(map[string]*netdl.Device, len(mafDevices))
	for _, mafDevice := range mafDevices {
		mafDevicesMap[mafDevice.DeviceId] = mafDevice
	}
	m.mafDevicesCache.SetAll(mafDevicesMap)

	for _, mafDevice := range mafDevices {
		actDeviceID, actDeviceIDExists := m.findActDeviceIDByMafDevice(mafDevice)

		var actDevice *domain.Device
		actDeviceExists := false

		if actDeviceIDExists {
			actDevice, actDeviceExists = m.findActDeviceInCurrent(actDeviceID)
		} else {
			actDevice, actDeviceExists = m.findActDeviceInCurrentByMafIP(mafDevice)
		}

		if !actDeviceIDExists && actDeviceExists {
			m.actDeviceManager.AddDeviceMapping(actDevice.Id, mafDevice.DeviceId)
		} else if actDeviceIDExists && !actDeviceExists {
			m.actDeviceManager.DeleteDeviceMappingByActDeviceID(actDeviceID)
		}
	}

	logger.Info("Sync latest MAF devices... done")

	if ctx.Err() != nil {
		logger.Warnf("context error occurred, skip getting latest maf devices and links: %v", ctx.Err())
		return
	}

	logger.Info("Sync latest MAF links...")

	logger.Info("Total MAF links:", len(mafLinks))
	for _, l := range mafLinks {
		mafLinkStr, err := getMafLinkIdentifier(l)
		if err != nil {
			logger.Errorf("Failed to get MAF link identifier: %v", err)
			continue
		}
		logger.Infof("MAF Link: %s, ID: %v", mafLinkStr, l.ID)
	}

	mafLinksMap := make(map[string]*netdl.Link, len(mafLinks))
	for _, mafLink := range mafLinks {
		mafLinkStr, err := getMafLinkIdentifier(mafLink)
		if err != nil {
			logger.Errorf("Failed to get MAF link identifier: %v", err)
			continue
		}

		mafLinksMap[mafLinkStr] = mafLink
	}
	m.mafLinksCache.SetAll(mafLinksMap)

	logger.Info("Sync latest MAF links... done")

	logger.Info("Sync latest MAF devices and links... done")
}

func (m *DefaultMonitor) updateTopologyDevice(ctx context.Context) {
	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warnf("[updateTopologyDevice] No project ID found in context")
		return
	}

	nmManager, err := dipool.GetNMManager()
	if err != nil {
		logger.Errorf("Failed to get NMManager: %v", err)
		return
	}

	baseline := m.baselineCache
	if baseline == nil {
		logger.Warnf("No baseline found, cannot update topology device")
		return
	}

	addDeviceInCurrentCache := func(ctx context.Context, actDevice *domain.Device, mafDeviceId string) {
		m.currentDeviceMap.Set(actDevice.Id, actDevice)
		m.actDeviceManager.AddDeviceMapping(actDevice.Id, mafDeviceId)
		m.onCreateDevice(ctx, actDevice)
	}

	deleteDeviceInCurrentCache := func(ctx context.Context, actDevice *domain.Device) {
		m.actDeviceManager.DeleteDeviceMappingByActDeviceID(actDevice.Id)
		m.currentDeviceMap.Delete(actDevice.Id)
		m.onDeleteDevice(ctx, actDevice)
	}

	mafDevices := nmManager.GetAllDevices()
	mafDeviceIpMap := make(map[string]*netdl.Device, len(mafDevices))
	for _, mafDevice := range mafDevices {
		mafDeviceIpMap[mafDevice.IP] = mafDevice
	}

	actDevices := m.currentDeviceMap.GetAll()
	actDeviceIpMap := make(map[string]*domain.Device, len(actDevices))
	for _, actDevice := range actDevices {
		actDeviceIpMap[actDevice.Ipv4.IpAddress] = actDevice
	}

	baselineDevices := m.baselineDeviceMap.GetAll()
	baselineDeviceMap := make(map[string]*domain.Device, len(baselineDevices))
	for _, baselineDevice := range baselineDevices {
		baselineDeviceMap[baselineDevice.Ipv4.IpAddress] = baselineDevice
	}

	for _, actDevice := range actDevices {
		_, mafDeviceExists := mafDeviceIpMap[actDevice.Ipv4.IpAddress]
		isInBaseline := baselineDeviceMap[actDevice.Ipv4.IpAddress] != nil

		if !mafDeviceExists && !isInBaseline {
			res := core.DeleteDevice(projectId, actDevice.Id, true)
			if !res.IsSuccess() {
				logger.Errorf("Failed to delete device: %v", res)
				continue
			} else {
				deleteDeviceInCurrentCache(ctx, actDevice)
				logger.Infof("Deleted act device: %v(%v)", actDevice.Id, actDevice.Ipv4.IpAddress)
			}
		}
	}

	for _, mafDevice := range mafDevices {
		hasModelName := mafDevice.ModelName != ""
		isAlive := mafDevice.Communication != nil && mafDevice.Communication.ICMP.Status == netdl.CommunicationStatusReachable.String()
		isInBaseline := baselineDeviceMap[mafDevice.IP] != nil
		actDevice, actDeviceExists := actDeviceIpMap[mafDevice.IP]

		if !isInBaseline && !isAlive && actDeviceExists {
			// delete device
			res := core.DeleteDevice(projectId, actDevice.Id, true)
			if !res.IsSuccess() {
				logger.Errorf("Failed to delete device: %v", res)
				continue
			} else {
				deleteDeviceInCurrentCache(ctx, actDevice)
				logger.Infof("Deleted act device: %v(%v)", actDevice.Id, actDevice.Ipv4.IpAddress)
			}

		}

		// // skip device with empty model name
		if isAlive && !actDeviceExists && hasModelName {
			// create device
			newActDevice, err := m.createActDeviceByMafDevice(projectId, mafDevice)
			if err != nil {
				logger.Errorf("Failed to create act device by maf device: %v", err)
				continue
			}

			addDeviceInCurrentCache(ctx, newActDevice, mafDevice.DeviceId)
		}
	}

}

func (m *DefaultMonitor) updateDeviceInfo(ctx context.Context) {
	if ctx.Err() != nil {
		return
	}

	updateDeviceInCurrentCache := func(actDevice *domain.Device, mafDeviceId string) {
		m.currentDeviceMap.Set(actDevice.Id, actDevice)
		m.actDeviceManager.AddDeviceMapping(actDevice.Id, mafDeviceId)
	}

	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warnf("[updateDevice] No project ID found in context")
		return
	}

	actDevices := m.currentDeviceMap.GetAll()
	for _, actDevice := range actDevices {
		if ctx.Err() != nil {
			return
		}

		mafDevice, mafDeviceExists := m.findMafDeviceByActDeviceID(actDevice.Id)
		if !mafDeviceExists {
			continue
		}

		devicePartial := &domain.DevicePartial{
			Id: actDevice.Id,
		}
		var newProfile *domain.SimpleDeviceProfile
		isProfileChanged := false
		needUpdate := false

		// device profile
		actDeviceType := getActDeviceType(mafDevice.DeviceType)
		modelName := mafDevice.ModelName
		if actDeviceType == ActDeviceTypeICMP || actDeviceType == ActDeviceTypeMoxa {
			modelName = actDeviceType.ModelName()
		}

		newProfile = common.FindDeviceProfileByModelNameAndModules(m.simpleProfiles, modelName, mafDevice.Modules)
		if newProfile == nil {
			newProfile = getUnknownDeviceProfile(m.simpleProfiles)
			if newProfile == nil {
				logger.Errorf("Failed to find device profile for unknown model")
				continue
			}
		}

		isProfileChanged = actDevice.DeviceProfileId != newProfile.Id
		if isProfileChanged {
			mafModelName := mafDevice.ModelName

			oldProfileModelName := actDevice.DeviceProperty.ModelName
			newProfileModelName := newProfile.ModelName

			logger.Infof("maf device%v(%v) model name: %v", mafModelName, mafDevice.DeviceId, mafDevice.IP, mafModelName)
			logger.Infof("Detect act device profile changed from %v to %v for device: %v(%v)", oldProfileModelName, newProfileModelName, actDevice.Id, actDevice.Ipv4.IpAddress)

			devicePartial.DeviceProfileId = &newProfile.Id

			needUpdate = true
		}

		if mafDevice.MAC != "" {
			parsedMafMacAddress, mafMacErr := macutility.ParseMACAddress(mafDevice.MAC)
			parsedActMacAddress, actMacErr := macutility.ParseMACAddress(actDevice.MacAddress)

			if mafMacErr != nil {
				logger.Errorf("Failed to parse MAF device(%v) MAC address: %v", mafDevice.IP, mafMacErr)
			} else if actMacErr == nil {
				if parsedMafMacAddress.Dashes(macutility.UpperCase) != parsedActMacAddress.Dashes(macutility.UpperCase) {
					logger.Infof("Detect MAC address changed from %v to %v for device: %v(%v)", actDevice.MacAddress, mafDevice.MAC, actDevice.Id, actDevice.Ipv4.IpAddress)
					macAddress := parsedMafMacAddress.Dashes(macutility.UpperCase)
					devicePartial.MacAddress = &macAddress

					needUpdate = true
				} else {
					macAddress := parsedMafMacAddress.Dashes(macutility.UpperCase)
					devicePartial.MacAddress = &macAddress

					needUpdate = true
				}
			}
		}

		if mafDevice.FirmwareVersion != "" {
			if actDevice.FirmwareVersion != mafDevice.FirmwareVersion {
				logger.Infof("Detect firmware version changed from %v to %v for device: %v(%v)", actDevice.FirmwareVersion, mafDevice.FirmwareVersion, actDevice.Id, actDevice.Ipv4.IpAddress)
				firmwareVersion := mafDevice.FirmwareVersion
				devicePartial.FirmwareVersion = &firmwareVersion

				needUpdate = true
			}
		}

		if mafDevice.DeviceName != nil && *mafDevice.DeviceName != "" {
			if actDevice.DeviceName != *mafDevice.DeviceName {
				logger.Infof("Detect device name changed from %v to %v for device: %v(%v)", actDevice.DeviceName, mafDevice.DeviceName, actDevice.Id, actDevice.Ipv4.IpAddress)
				deviceName := *mafDevice.DeviceName
				devicePartial.DeviceName = &deviceName

				needUpdate = true
			}
		}

		// device module
		if mafDevice.Modules != nil {
			actDeviceModules, err := configmapper.MappingDeviceModules(mafDevice.Modules)
			if err != nil {
				logger.Errorf("Failed to map device modules: %v", err)
			} else {
				if !reflect.DeepEqual(actDevice.ModularConfiguration, *actDeviceModules) {
					devicePartial.ModularConfiguration = actDeviceModules

					needUpdate = true
				}
			}
		}

		if needUpdate {
			// update device
			res := core.PartialUpdateDevice(
				projectId,
				devicePartial,
				true,
			)

			if !res.IsSuccess() {
				logger.Errorf("Failed to update device: %v", res.ErrorMessage)
			} else {
				// Get updated device
				updatedActDevice, res := core.GetDevice(projectId, actDevice.Id, true)
				if !res.IsSuccess() {
					logger.Errorf("Failed to get updated device after update: %v", res.ErrorMessage)
					continue
				}

				// Update cache
				updateDeviceInCurrentCache(&updatedActDevice, mafDevice.DeviceId)

				m.onUpdateDevice(ctx, &updatedActDevice)
			}
		}
	}
}

func (m *DefaultMonitor) updateDeviceConfig(ctx context.Context) {
	isCanceled := false

	defer func() {
		if isCanceled {
			logger.Warnf("Context is done, Stop updating device config")
		}
	}()

	if ctx.Err() != nil {
		isCanceled = true
		return
	}

	if !m.internalMutex.TryLock() {
		logger.Warnf("failed to acquire lock for updating device config, skip this round")
		return
	}

	defer m.internalMutex.Unlock()

	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warnf("[updateDeviceConfig] No project ID found in context")
		return
	}

	isOperation := true
	showPassword := true
	operationProject, res := core.GetProject(projectId, isOperation, showPassword)
	if !res.IsSuccess() {
		logger.Errorf("Failed to get project for update device config: %v", res)
		return
	}

	currentDeviceConfig := operationProject.DeviceConfig

	actDevices := m.currentDeviceMap.GetAll()
	for _, actDevice := range actDevices {
		if ctx.Err() != nil {
			isCanceled = true
			return
		}

		mafDevice, mafDeviceExists := m.findMafDeviceByActDeviceID(actDevice.Id)
		if !mafDeviceExists {
			continue
		}

		// InformationSetting
		if currentDeviceConfig.InformationSettingTables == nil {
			currentDeviceConfig.InformationSettingTables = make(map[int64]domain.InformationSettingTable)
		}

		actInfoSetting := currentDeviceConfig.InformationSettingTables[actDevice.Id]
		actInfoSetting.DeviceId = actDevice.Id

		if mafDevice.Location != nil {
			actInfoSetting.Location = *mafDevice.Location
		}

		if mafDevice.Description != nil {
			actInfoSetting.Description = *mafDevice.Description
		}

		if mafDevice.DeviceName != nil {
			actInfoSetting.DeviceName = *mafDevice.DeviceName
		}

		if mafDevice.ContactInfo != nil {
			actInfoSetting.ContactInformation = *mafDevice.ContactInfo
		}

		currentDeviceConfig.InformationSettingTables[actDevice.Id] = actInfoSetting

		if mafDevice.Configuration != nil {
			// NetworkSetting
			if mafDevice.Configuration.EthInterfaceSettings != nil && len(*mafDevice.Configuration.EthInterfaceSettings) > 0 {
				if currentDeviceConfig.NetworkSettingTables == nil {
					currentDeviceConfig.NetworkSettingTables = make(map[int64]domain.NetworkSettingTable)
				}

				mafEtherIfaceSetting := (*mafDevice.Configuration.EthInterfaceSettings)[0]

				actNetworkSetting := currentDeviceConfig.NetworkSettingTables[actDevice.Id]
				actNetworkSetting.DeviceId = actDevice.Id

				networkSettingMode := actNetworkSetting.NetworkSettingMode

				networkMode := configmapper.MapNetworkSettingMode(mafEtherIfaceSetting.Ipv4NSettings.Mode)
				if networkMode != domain.NetworkSettingModeUnknown {
					networkSettingMode = networkMode.String()
				}

				actNetworkSetting.NetworkSettingMode = networkSettingMode
				actNetworkSetting.DNS1 = mafEtherIfaceSetting.Ipv4NSettings.Dns1
				actNetworkSetting.DNS2 = mafEtherIfaceSetting.Ipv4NSettings.Dns2
				actNetworkSetting.Gateway = mafEtherIfaceSetting.Ipv4NSettings.Gateway
				actNetworkSetting.SubnetMask = mafEtherIfaceSetting.Ipv4NSettings.Netmask

				currentDeviceConfig.NetworkSettingTables[actDevice.Id] = actNetworkSetting
			}

			// RSTPSetting
			if mafDevice.Configuration.RstpSetting != nil {
				actRSTPSetting, err := configmapper.MappingDeviceConfigRSTPSetting(mafDevice.Configuration.RstpSetting, actDevice.Id)
				if err != nil {
					logger.Errorf("Failed to map RSTP setting: %v", err)
				} else {
					if currentDeviceConfig.RstpTables == nil {
						currentDeviceConfig.RstpTables = make(map[int64]domain.RstpTable)
					}
					currentDeviceConfig.RstpTables[actDevice.Id] = *actRSTPSetting
				}
			}

			// VLANSetting
			if mafDevice.Configuration.VlanSetting != nil {
				actVLANSetting, err := configmapper.MappingDeviceConfigVLANSetting(mafDevice.Configuration.VlanSetting, actDevice.Id)
				if err != nil {
					logger.Errorf("Failed to map VLAN setting: %v", err)
				} else {
					if currentDeviceConfig.VlanTables == nil {
						currentDeviceConfig.VlanTables = make(map[int64]domain.VlanTable)
					}

					currentDeviceConfig.VlanTables[actDevice.Id] = *actVLANSetting
				}
			}

			// workaround begin
			portIdMap := map[int]bool{}
			for _, iface := range actDevice.Interfaces {
				portIdMap[iface.InterfaceId] = true
			}

			// workaround end

			if mafDevice.Configuration.PcpSetting != nil {
				actPcpSetting, err := configmapper.MappingPcpSettingTables(actDevice.Id, mafDevice.Configuration.PcpSetting, portIdMap)
				if err != nil {
					logger.Errorf("Failed to map PCP setting: %v", err)
				} else {
					if currentDeviceConfig.PortDefaultPCPTables == nil {
						currentDeviceConfig.PortDefaultPCPTables = make(map[int64]domain.PortDefaultPCPTable)
					}

					currentDeviceConfig.PortDefaultPCPTables[actDevice.Id] = *actPcpSetting
				}
			}

			if mafDevice.Configuration.LoopProtectionSetting != nil {
				actLoopProtectionSetting := configmapper.MappingDeviceConfigLoopProtectionSetting(mafDevice.Configuration.LoopProtectionSetting, actDevice.Id)

				if currentDeviceConfig.LoopProtectionTables == nil {
					currentDeviceConfig.LoopProtectionTables = make(map[int64]domain.LoopProtection)
				}

				currentDeviceConfig.LoopProtectionTables[actDevice.Id] = *actLoopProtectionSetting
			}

			if mafDevice.Configuration.LoginPolicySetting != nil {
				actLoginPolicySetting, err := configmapper.MappingLoginPolicy(actDevice.Id, mafDevice.Configuration.LoginPolicySetting)
				if err != nil {
					logger.Errorf("Failed to map Login Policy setting: %v", err)
				} else {
					if currentDeviceConfig.LoginPolicyTables == nil {
						currentDeviceConfig.LoginPolicyTables = make(map[int64]domain.LoginPolicyTable)
					}

					currentDeviceConfig.LoginPolicyTables[actDevice.Id] = *actLoginPolicySetting
				}
			}

			if mafDevice.Configuration.PortSetting != nil {
				actPortSettings := configmapper.MappingDeviceConfigPortSetting(*mafDevice.Configuration.PortSetting, actDevice.Id)
				if currentDeviceConfig.PortSettingTables == nil {
					currentDeviceConfig.PortSettingTables = make(map[int64]domain.PortSettingTable)
				}

				currentDeviceConfig.PortSettingTables[actDevice.Id] = *actPortSettings
			}

			if mafDevice.Configuration.SystemTimeConfig != nil && mafDevice.Configuration.DaylightSavingSetting != nil {
				actSystemTimeConfig, err := configmapper.MappingTimeSettingTables(
					actDevice.Id,
					mafDevice.Configuration.SystemTimeConfig,
					mafDevice.Configuration.DaylightSavingSetting,
				)
				if err != nil {
					logger.Errorf("Failed to map System Time config: %v", err)
				} else {
					if currentDeviceConfig.TimeSettingTables == nil {
						currentDeviceConfig.TimeSettingTables = make(map[int64]domain.TimeSettingTable)
					}
					currentDeviceConfig.TimeSettingTables[actDevice.Id] = *actSystemTimeConfig
				}
			}

			if mafDevice.Configuration.SNMPTrapSetting != nil {
				actSNMPTrapSetting, err := configmapper.MappingSnmpTrapSettingTables(actDevice.Id, *mafDevice.Configuration.SNMPTrapSetting)
				if err != nil {
					logger.Errorf("Failed to map SNMP Trap setting: %v", err)
				} else {
					if currentDeviceConfig.SnmpTrapSettingTables == nil {
						currentDeviceConfig.SnmpTrapSettingTables = make(map[int64]domain.SnmpTrapSettingTable)
					}
					currentDeviceConfig.SnmpTrapSettingTables[actDevice.Id] = *actSNMPTrapSetting
				}
			}

			if mafDevice.Configuration.SyslogSetting != nil {
				actSyslogSetting, err := configmapper.MappingSyslogSetting(actDevice.Id, mafDevice.Configuration.SyslogSetting)
				if err != nil {
					logger.Errorf("Failed to map Syslog setting: %v", err)
				} else {
					if currentDeviceConfig.SyslogSettingTables == nil {
						currentDeviceConfig.SyslogSettingTables = make(map[int64]domain.SyslogSettingTable)
					}
					currentDeviceConfig.SyslogSettingTables[actDevice.Id] = *actSyslogSetting
				}
			}

			if mafDevice.Configuration.PerStreamPriority != nil {
				actIngress, ingressErr := configmapper.MappingStreamPriorityIngressTables(actDevice.Id, mafDevice.Configuration.PerStreamPriority)
				actEgress, egressErr := configmapper.MappingStreamPriorityEgressTable(actDevice.Id, mafDevice.Configuration.PerStreamPriority)

				validIngress := ingressErr == nil
				validEgress := egressErr == nil

				if ingressErr != nil {
					logger.Errorf("Failed to map per stream priority ingress setting: %v", ingressErr)
				}

				if egressErr != nil {
					logger.Errorf("Failed to map per stream priority egress setting: %v", egressErr)
				}

				if validIngress {
					if currentDeviceConfig.StreamPriorityIngressTables == nil {
						currentDeviceConfig.StreamPriorityIngressTables = make(map[int64]domain.StreamPriorityIngressTable)
					}

					currentDeviceConfig.StreamPriorityIngressTables[actDevice.Id] = *actIngress
				}

				if validEgress {
					if currentDeviceConfig.StreamPriorityEgressTables == nil {
						currentDeviceConfig.StreamPriorityEgressTables = make(map[int64]domain.StreamPriorityEgressTable)
					}

					currentDeviceConfig.StreamPriorityEgressTables[actDevice.Id] = *actEgress
				}
			}

			if mafDevice.Configuration.TimeAwareShaper != nil {
				actGCL, err := configmapper.MappingGCLSetting(actDevice.Id, mafDevice.Configuration.TimeAwareShaper)
				if err != nil {
					logger.Errorf("Failed to map GCL setting: %v", err)
				} else {
					if currentDeviceConfig.GCLTables == nil {
						currentDeviceConfig.GCLTables = make(map[int64]domain.GCLTable)
					}
					currentDeviceConfig.GCLTables[actDevice.Id] = *actGCL
				}
			}
		}

		projectWithDeviceConfig := &domain.ProjectWithDeviceConfig{
			Id:           projectId,
			DeviceConfig: currentDeviceConfig,
		}

		if ctx.Err() != nil {
			isCanceled = true
			return
		}

		isOperation := true
		res := core.UpdateProjectDeviceConfig(projectWithDeviceConfig, isOperation)
		if !res.IsSuccess() {
			logger.Errorf("Failed to update project device config: %v", res)
		} else {
			logger.Infof("Updated device config successfully for project: %v", projectId)
			m.currentProject.DeviceConfig = currentDeviceConfig
		}
	}

}

func (m *DefaultMonitor) updateTopologyLink(ctx context.Context) {
	if ctx.Err() != nil {
		return
	}

	projectId, projectExists := getProjectId(ctx)
	if !projectExists {
		logger.Errorf("No project ID found in context, cannot detect link change")
		return
	}

	if !m.isMonitoring(projectId) {
		return
	}

	mafLinks := m.mafLinksCache.GetAll()
	baselineDeviceMap := m.baselineDeviceMap.GetAll()
	baselineLinks := m.baselineLinkMap.GetAll()
	currentDeviceMap := m.currentDeviceMap.GetAll()
	currentDeviceIpMap := make(map[string]*domain.Device, len(currentDeviceMap))
	for _, device := range currentDeviceMap {
		currentDeviceIpMap[device.Ipv4.IpAddress] = device
	}

	currentLinkMap := m.currentLinkMap.GetAll()

	baselineDiffLinkMap := make(map[string]*DiffLink, len(baselineLinks))
	inputBaselineDiffLinkMap := make(map[string]*DiffLink, len(baselineLinks))
	inputActualDiffLinkMap := make(map[string]*DiffLink, len(mafLinks))

	for _, baselineLink := range baselineLinks {
		srcActDevice, srcActDeviceExists := m.findActDeviceInBaseline(int64(baselineLink.SourceDeviceId))
		dstActDevice, dstActDeviceExists := m.findActDeviceInBaseline(int64(baselineLink.DestinationDeviceId))

		if !srcActDeviceExists {
			logger.Warnf("No source device found for baseline link: %v", baselineLink.Id)
			continue
		}

		if !dstActDeviceExists {
			logger.Warnf("No destination device found for baseline link: %v", baselineLink.Id)
			continue
		}

		diffLink := NewDiffLink(
			srcActDevice.Ipv4.IpAddress,
			baselineLink.SourceInterfaceId,
			dstActDevice.Ipv4.IpAddress,
			baselineLink.DestinationInterfaceId,
		)

		baselineDiffLinkMap[diffLink.String()] = diffLink
		inputBaselineDiffLinkMap[diffLink.String()] = diffLink
	}

	for _, link := range mafLinks {
		// skip link with localhost endpoint
		if link.FromDevice.IP == localhostIP || link.ToDevice.IP == localhostIP {
			continue
		}

		if link.Speed <= 0 {
			linkStr := fmt.Sprintf("%v:%v-%v:%v", link.FromDevice.IP, link.FromPort, link.ToDevice.IP, link.ToPort)
			logger.Warnf("skip link(%v) with invalid speed: %v", linkStr, link.Speed)
			continue
		}

		if link.Status != netdl.LinkStatusUp.String() {
			continue
		}

		diffLink := NewDiffLink(
			link.FromDevice.IP,
			link.FromPort,
			link.ToDevice.IP,
			link.ToPort,
		)

		inputActualDiffLinkMap[diffLink.String()] = diffLink
	}

	diffResult, err := ComputeLinkDiff(inputBaselineDiffLinkMap, inputActualDiffLinkMap)
	if err != nil {
		logger.Errorf("Failed to compute link diff: %v", err)
		return
	} else {
		logger.Debugf("Link Diff Result:\n %v", diffResult.String())
	}

	createLink := func(linkStr string, ip1 string, port1 int, ip2 string, port2 int) (*domain.Link, error) {
		srcActDevice, srcActDeviceExists := currentDeviceIpMap[ip1]
		dstActDevice, dstActDeviceExists := currentDeviceIpMap[ip2]

		if !srcActDeviceExists {
			return nil, fmt.Errorf("no source device found for link endpoint: %v", ip1)
		}

		if !dstActDeviceExists {
			return nil, fmt.Errorf("no destination device found for link endpoint: %v", ip2)
		}

		mafLink, mafLinkExists := mafLinks[linkStr]
		if !mafLinkExists {
			return nil, fmt.Errorf("no maf link found for link: %v", linkStr)
		}

		newActLink, err := m.createActLink(projectId, &domain.LinkConf{
			SourceDeviceId:         int(srcActDevice.Id),
			SourceInterfaceId:      port1,
			DestinationDeviceId:    int(dstActDevice.Id),
			DestinationInterfaceId: port2,
			Speed:                  int(mafLink.Speed / 1000000),
		})

		return newActLink, err
	}

	onCreateLink := func(linkStr string, link *domain.Link) {
		m.currentLinkMap.Set(linkStr, link)
		m.onCreateLink(ctx, link)
	}

	onDeleteLink := func(linkStr string, link *domain.Link) {
		m.currentLinkMap.Delete(linkStr)
		m.onDeleteLink(ctx, link)
		// m.DeleteLinkMutex(linkStr)
	}

	// build current link endpoint map
	currentLinkEndpointMap := make(map[string]*domain.Link, 0)
	for _, currentLink := range currentLinkMap {
		linkEndpoints, err := m.getActLinkEndpointsStr(currentLink, currentDeviceMap)
		if err != nil {
			logger.Errorf("Failed to get act link endpoints string: %v", err)
			continue
		}

		for _, endpointStr := range linkEndpoints {
			currentLinkEndpointMap[endpointStr] = currentLink
		}
	}

	// 刪除不屬於 Unchanged Links && 不屬於 New Links & 不在 baseline 的 link
	for _, currentLink := range currentLinkMap {
		currentLinkStr, err := m.getActLinkStr(currentLink, currentDeviceMap)
		if err != nil {
			logger.Errorf("Failed to get act link string: %v", err)
			continue
		}

		currentLinkEndpointStr, err := m.getActLinkEndpointsStr(currentLink, currentDeviceMap)
		if err != nil {
			logger.Errorf("Failed to get act link endpoints string: %v", err)
			continue
		}

		fromEndpoint := currentLinkEndpointStr[0]
		toEndpoint := currentLinkEndpointStr[1]

		_, isInBaseline := baselineDiffLinkMap[currentLinkStr]
		_, isInUnchangedLinks := diffResult.UnchangedLinks[currentLinkStr]
		_, isInNewLinks := diffResult.NewLinks[currentLinkStr]

		if !isInBaseline && !isInUnchangedLinks && !isInNewLinks {
			linkMutex := m.GetLinkMutex(currentLinkStr)
			linkMutex.Lock()

			res := core.DeleteLink(projectId, int64(currentLink.Id), true)
			if !res.IsSuccess() {
				logger.Errorf("Failed to delete act link: %v", res)
				linkMutex.Unlock()
				continue
			}

			delete(currentLinkEndpointMap, fromEndpoint)
			delete(currentLinkEndpointMap, toEndpoint)
			delete(currentLinkMap, currentLinkStr)
			onDeleteLink(currentLinkStr, currentLink)
			// m.DeleteLinkMutex(currentLinkStr)
			linkMutex.Unlock()
		}
	}

	// create unchanaged link if not exist in operation project
	for _, unchangedLink := range diffResult.UnchangedLinks {
		_, exists := currentLinkMap[unchangedLink.String()]
		if !exists {
			// check port availability
			fromEndpoint := unchangedLink.FromEndpoint()
			toEndpoint := unchangedLink.ToEndpoint()
			l1, fromExists := currentLinkEndpointMap[fromEndpoint]
			l2, toExists := currentLinkEndpointMap[toEndpoint]

			if fromExists || toExists {
				if fromExists && toExists {
					if l1.Id == l2.Id {
						continue
					}
				}

				if fromExists {
					fromLinkStr, err := m.getActLinkStr(l1, currentDeviceMap)
					if err != nil {
						logger.Errorf("Failed to get act link string: %v", err)
						continue
					}

					fromLinkEndpoints, err := m.getActLinkEndpointsStr(l1, currentDeviceMap)
					if err != nil {
						logger.Errorf("Failed to get act link endpoints string: %v", err)
						continue
					}

					fromEndpointStr := fromLinkEndpoints[0]
					toEndpointStr := fromLinkEndpoints[1]

					linkMutex := m.GetLinkMutex(fromLinkStr)
					linkMutex.Lock()

					res := core.DeleteLink(projectId, int64(l1.Id), true)
					if !res.IsSuccess() {
						logger.Errorf("Failed to delete act link: %v", res)
						linkMutex.Unlock()
						continue
					}

					delete(currentLinkEndpointMap, fromEndpointStr)
					delete(currentLinkEndpointMap, toEndpointStr)
					delete(currentLinkMap, fromLinkStr)
					onDeleteLink(fromLinkStr, l1)
					// m.DeleteLinkMutex(fromLinkStr)
					linkMutex.Unlock()
				}

				if toExists {
					toLinkStr, err := m.getActCurrentLinkStr(l2)
					if err != nil {
						logger.Errorf("Failed to get act link string: %v", err)
						continue
					}

					toLinkEndpoints, err := m.getActLinkEndpointsStr(l2, currentDeviceMap)
					if err != nil {
						logger.Errorf("Failed to get act link endpoints string: %v", err)
						continue
					}

					fromEndpointStr := toLinkEndpoints[0]
					toEndpointStr := toLinkEndpoints[1]

					linkMutex := m.GetLinkMutex(toLinkStr)
					linkMutex.Lock()

					res := core.DeleteLink(projectId, int64(l2.Id), true)
					if !res.IsSuccess() {
						logger.Errorf("Failed to delete act link: %v", res)
						linkMutex.Unlock()
						continue
					}

					delete(currentLinkEndpointMap, fromEndpointStr)
					delete(currentLinkEndpointMap, toEndpointStr)
					delete(currentLinkMap, toLinkStr)
					onDeleteLink(toLinkStr, l2)
					// m.DeleteLinkMutex(toLinkStr)
					linkMutex.Unlock()
				}
			}

			_, mafLinkExists := mafLinks[unchangedLink.String()]
			if !mafLinkExists {
				logger.Errorf("No maf link found for unchanged link: %v", unchangedLink.String())
				continue
			} else {
				linkMutex := m.GetLinkMutex(unchangedLink.String())
				linkMutex.Lock()

				createdLink, err := createLink(
					unchangedLink.String(),
					unchangedLink.FromIP(),
					unchangedLink.FromPort(),
					unchangedLink.ToIP(),
					unchangedLink.ToPort(),
				)

				if err != nil {
					logger.Errorf("Failed to create unchanged link(%v): %v", unchangedLink.String(), err)
					linkMutex.Unlock()
					continue
				}

				currentLinkEndpointMap[fromEndpoint] = createdLink
				currentLinkEndpointMap[toEndpoint] = createdLink
				currentLinkMap[unchangedLink.String()] = createdLink
				onCreateLink(unchangedLink.String(), createdLink)
				linkMutex.Unlock()
			}

		}
	}

	// create new link if not exist in project and not rewired link
	for newLinkStr, newLink := range diffResult.NewLinks {
		_, isRewiredLink := diffResult.RewiredLinks[newLinkStr]

		if !isRewiredLink {
			_, exists := currentLinkMap[newLinkStr]
			if !exists {
				// check port availability
				fromEndpoint := newLink.FromEndpoint()
				toEndpoint := newLink.ToEndpoint()

				fromLink, fromExists := currentLinkEndpointMap[fromEndpoint]
				if fromExists {
					fromLinkStr, err := m.getActLinkStr(fromLink, currentDeviceMap)
					if err != nil {
						logger.Errorf("Failed to get act link string: %v", err)
						continue
					}

					fromLinkEndpoints, err := m.getActLinkEndpointsStr(fromLink, currentDeviceMap)
					if err != nil {
						logger.Errorf("Failed to get act link endpoints string: %v", err)
						continue
					}

					// delete occupying link

					linkMutex := m.GetLinkMutex(fromLinkStr)
					linkMutex.Lock()

					res := core.DeleteLink(projectId, int64(fromLink.Id), true)
					if !res.IsSuccess() {
						logger.Errorf("Failed to delete act link: %v", res)
						linkMutex.Unlock()
						continue
					}

					fromEndpointStr := fromLinkEndpoints[0]
					toEndpointStr := fromLinkEndpoints[1]

					delete(currentLinkEndpointMap, fromEndpointStr)
					delete(currentLinkEndpointMap, toEndpointStr)
					delete(currentLinkMap, fromLinkStr)
					onDeleteLink(fromLinkStr, fromLink)
					// m.DeleteLinkMutex(fromLinkStr)
					linkMutex.Unlock()
				}

				toLink, toExists := currentLinkEndpointMap[toEndpoint]

				if toExists {
					toLinkStr, err := m.getActLinkStr(toLink, currentDeviceMap)
					if err != nil {
						logger.Errorf("Failed to get act link string: %v", err)
						continue
					}

					toLinkEndpoints, err := m.getActLinkEndpointsStr(toLink, currentDeviceMap)
					if err != nil {
						logger.Errorf("Failed to get act link endpoints string: %v", err)
						continue
					}

					// delete occupying link
					linkMutex := m.GetLinkMutex(toLinkStr)
					linkMutex.Lock()

					res := core.DeleteLink(projectId, int64(toLink.Id), true)
					if !res.IsSuccess() {
						logger.Errorf("Failed to delete act link: %v", res)
						linkMutex.Unlock()
						continue
					}

					fromEndpointStr := toLinkEndpoints[0]
					toEndpointStr := toLinkEndpoints[1]

					delete(currentLinkEndpointMap, fromEndpointStr)
					delete(currentLinkEndpointMap, toEndpointStr)
					delete(currentLinkMap, toLinkStr)
					onDeleteLink(toLinkStr, toLink)
					// m.DeleteLinkMutex(toLinkStr)
					linkMutex.Unlock()
				}

				linkMutex := m.GetLinkMutex(newLink.String())
				linkMutex.Lock()

				createdLink, err := createLink(
					newLink.String(),
					newLink.FromIP(),
					newLink.FromPort(),
					newLink.ToIP(),
					newLink.ToPort(),
				)

				if err != nil {
					logger.Errorf("Failed to create new link(%v): %v", newLink.String(), err)
					linkMutex.Unlock()
					continue
				}

				currentLinkEndpointMap[fromEndpoint] = createdLink
				currentLinkEndpointMap[toEndpoint] = createdLink
				currentLinkMap[newLink.String()] = createdLink
				onCreateLink(newLink.String(), createdLink)
				linkMutex.Unlock()
			}

		}
	}

	// Rewired Links
	for rewiredLinkStr, rewiredLink := range diffResult.RewiredLinks {
		_, exists := currentLinkMap[rewiredLinkStr]
		if !exists {
			// check port availability
			fromEndpoint := rewiredLink.FromEndpoint()
			toEndpoint := rewiredLink.ToEndpoint()

			fromLink, fromExists := currentLinkEndpointMap[fromEndpoint]
			toLink, toExists := currentLinkEndpointMap[toEndpoint]

			if fromExists && toExists {
				if fromLink.Id == toLink.Id {
					continue
				}
			}

			if fromExists {
				fromLinkStr, err := m.getActCurrentLinkStr(fromLink)
				if err != nil {
					logger.Errorf("Failed to get act link string: %v", err)
					continue
				}

				fromLinkEndpoints, err := m.getActLinkEndpointsStr(fromLink, currentDeviceMap)
				if err != nil {
					logger.Errorf("Failed to get act link endpoints string: %v", err)
					continue
				}
				// delete occupying link
				linkMutex := m.GetLinkMutex(fromLinkStr)
				linkMutex.Lock()

				res := core.DeleteLink(projectId, int64(fromLink.Id), true)
				if !res.IsSuccess() {
					logger.Errorf("Failed to delete act link: %v", res)
					linkMutex.Unlock()
					continue
				}

				fromEndpointStr := fromLinkEndpoints[0]
				toEndpointStr := fromLinkEndpoints[1]

				delete(currentLinkEndpointMap, fromEndpointStr)
				delete(currentLinkEndpointMap, toEndpointStr)
				delete(currentLinkMap, fromLinkStr)
				onDeleteLink(fromLinkStr, fromLink)
				// m.DeleteLinkMutex(fromLinkStr)
				linkMutex.Unlock()
			}

			if toExists {
				toLinkStr, err := m.getActCurrentLinkStr(toLink)
				if err != nil {
					logger.Errorf("Failed to get act link string: %v", err)
					continue
				}

				toLinkEndpoints, err := m.getActLinkEndpointsStr(toLink, currentDeviceMap)
				if err != nil {
					logger.Errorf("Failed to get act link endpoints string: %v", err)
					continue
				}

				// delete occupying link
				linkMutex := m.GetLinkMutex(toLinkStr)
				linkMutex.Lock()

				res := core.DeleteLink(projectId, int64(toLink.Id), true)
				if !res.IsSuccess() {
					logger.Errorf("Failed to delete act link: %v", res)
					linkMutex.Unlock()
					continue
				}

				fromEndpointStr := toLinkEndpoints[0]
				toEndpointStr := toLinkEndpoints[1]

				delete(currentLinkEndpointMap, fromEndpointStr)
				delete(currentLinkEndpointMap, toEndpointStr)
				delete(currentLinkMap, toLinkStr)
				onDeleteLink(toLinkStr, toLink)
				// m.DeleteLinkMutex(toLinkStr)
				linkMutex.Unlock()
			}

			linkMutex := m.GetLinkMutex(rewiredLink.String())
			linkMutex.Lock()

			createdLink, err := createLink(
				rewiredLink.String(),
				rewiredLink.FromIP(),
				rewiredLink.FromPort(),
				rewiredLink.ToIP(),
				rewiredLink.ToPort(),
			)

			if err != nil {
				logger.Errorf("Failed to create rewired link(%v): %v", rewiredLink.String(), err)
				linkMutex.Unlock()
				continue
			}

			currentLinkEndpointMap[fromEndpoint] = createdLink
			currentLinkEndpointMap[toEndpoint] = createdLink
			currentLinkMap[rewiredLink.String()] = createdLink
			onCreateLink(rewiredLink.String(), createdLink)
			linkMutex.Unlock()
		}
	}

	// 補上 baseline link 但是之前被刪除的，且 port 是可用的
	for baselineLinkStr, baselineLink := range baselineLinks {
		// check if baseline link exists in current
		_, exists := currentLinkMap[baselineLinkStr]
		if exists {
			continue
		}

		endpoints, err := m.getActLinkEndpointsStr(baselineLink, baselineDeviceMap)
		if err != nil {
			logger.Errorf("Failed to get baseline link endpoints string: %v", err)
			continue
		}

		fromEndpoint := endpoints[0]
		toEndpoint := endpoints[1]

		// check port availability
		_, hasFromLink := currentLinkEndpointMap[fromEndpoint]
		_, hasToLink := currentLinkEndpointMap[toEndpoint]

		// create link if not exist in project
		if !hasFromLink && !hasToLink {
			srcDeviceId := baselineLink.SourceDeviceId
			dstDeviceId := baselineLink.DestinationDeviceId

			srcActDevice, srcInBaseline := m.findActDeviceInBaseline(int64(srcDeviceId))
			dstActDevice, dstInBaseline := m.findActDeviceInBaseline(int64(dstDeviceId))

			if !srcInBaseline {
				logger.Warnf("No source device found for baseline link: %v", baselineLinkStr)
				continue
			}

			if !dstInBaseline {
				logger.Warnf("No destination device found for baseline link: %v", baselineLinkStr)
				continue
			}

			srcIP := srcActDevice.Ipv4.IpAddress
			dstIP := dstActDevice.Ipv4.IpAddress

			var currentSrcDeviceId int64 = -1
			var currentDstDeviceId int64 = -1

			hasSrcDevice := false
			hasDstDevice := false

			for deviceId, device := range currentDeviceMap {
				if device.Ipv4.IpAddress == srcIP {
					currentSrcDeviceId = int64(deviceId)
					hasSrcDevice = true
					break
				}
			}

			if !hasSrcDevice {
				logger.Warnf("No source device found in current for baseline link: %v", baselineLinkStr)
				continue
			}

			for deviceId, device := range currentDeviceMap {
				if device.Ipv4.IpAddress == dstIP {
					currentDstDeviceId = int64(deviceId)
					hasDstDevice = true
					break
				}
			}

			if !hasDstDevice {
				logger.Warnf("No destination device found in current for baseline link: %v", baselineLinkStr)
				continue
			}

			srcPort := baselineLink.SourceInterfaceId
			dstPort := baselineLink.DestinationInterfaceId
			linkSpeed := baselineLink.Speed

			linkConf := &domain.LinkConf{
				SourceDeviceId:         int(currentSrcDeviceId),
				SourceInterfaceId:      srcPort,
				DestinationDeviceId:    int(currentDstDeviceId),
				DestinationInterfaceId: dstPort,
				Speed:                  linkSpeed,
			}

			linkStr := fmt.Sprintf("%v-%v", fromEndpoint, toEndpoint)
			linkMutex := m.GetLinkMutex(linkStr)
			linkMutex.Lock()

			createdLink, err := m.createActLink(projectId, linkConf)

			if err != nil {
				logger.Errorf("Failed to create act link for baseline link: %v", err)
				linkMutex.Unlock()
				continue
			}

			currentLinkEndpointMap[fromEndpoint] = createdLink
			currentLinkEndpointMap[toEndpoint] = createdLink
			currentLinkMap[baselineLinkStr] = createdLink
			onCreateLink(baselineLinkStr, createdLink)
			linkMutex.Unlock()
		}
	}
}

func (m *DefaultMonitor) updateLinkInfo(ctx context.Context) {
	if ctx.Err() != nil {
		return
	}

	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warnf("[updateLinkInfo] No project ID found in context")
		return
	}

	mafLinks := m.mafLinksCache.GetAll()
	actLinks := m.currentLinkMap.GetAll()

	for actLinkStr, actLink := range actLinks {
		if ctx.Err() != nil {
			return
		}

		linkMutex := m.GetLinkMutex(actLinkStr)
		linkMutex.Lock()

		mafLink, exists := mafLinks[actLinkStr]
		if exists && mafLink.Speed > 0 {
			oldSpeed := actLink.Speed
			newSpeed := int(mafLink.Speed / 1000000)
			if actLink.Speed != newSpeed {
				linkInfo, res := core.UpdateLink(projectId, int64(actLink.Id), domain.LinkConf{
					Speed:                  newSpeed,
					SourceDeviceId:         actLink.SourceDeviceId,
					SourceInterfaceId:      actLink.SourceInterfaceId,
					DestinationDeviceId:    actLink.DestinationDeviceId,
					DestinationInterfaceId: actLink.DestinationInterfaceId,
				}, true)

				if !res.IsSuccess() {
					logger.Errorf("Failed to update link speed: %v", res)
					linkMutex.Unlock()
					continue
				}

				newLink, res := core.GetLink(projectId, int64(linkInfo.Id), true)
				if !res.IsSuccess() {
					logger.Errorf("Failed to get updated link: %v", res)
					linkMutex.Unlock()
					continue
				}

				m.currentLinkMap.Set(actLinkStr, &newLink)
				linkMutex.Unlock()
				logger.Infof("Updated link speed: %v, from %v Mbps to %v Mbps", actLinkStr, oldSpeed, newSpeed)
			} else {
				linkMutex.Unlock()
			}
		} else {
			linkMutex.Unlock()
		}
	}
}
