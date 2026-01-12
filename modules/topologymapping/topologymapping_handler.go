package topologymapping

import (
	"encoding/json"
	"fmt"
	"sort"
	"time"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/configmapper"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	devicemanager "gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/manager"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/manager/job/model"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/lib/topology"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
	dmschema "gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

func (topologyMapper *TopologyMapper) clearMAFCache() error {
	netCache, err := dipool.GetNetCache()
	if err != nil {
		return err
	}

	netCache.DeleteAllDevices()
	netCache.DeleteAllLinks()

	return nil
}

func (topologyMapper *TopologyMapper) waitForJobCompletion(jobId string, intervalSeconds int, timeoutSeconds int) error {
	logger.Info("Start waitForJobCompletion")

	timeout := time.After(time.Duration(timeoutSeconds) * time.Second)
	ticker := time.NewTicker(time.Duration(intervalSeconds) * time.Second)
	defer ticker.Stop()
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	// local funct (topologyMapper *TopologyMapper)ion: check if all actions are completed, return stats
	allActionsCompleted := func(job *model.Job) (bool, int, int) {
		total := 0
		done := 0

		for _, tasks := range job.Pipeline {
			for _, task := range tasks {
				for _, action := range task.Actions {
					total++
					if action.CompletedAt != nil {
						done++

						// jsonBytes, _ := json.MarshalIndent(action, "", "  ")
						logger.Info(fmt.Sprintf("MAF Complete Action: %+v\n", action))
					}
				}
			}
		}
		return done == total, done, total - done
	}
	for {
		select {
		case <-timeout:
			err := fmt.Errorf("Job(%s) did not complete within timeout(%d sec)", jobId, timeoutSeconds)
			logger.Error(err)
			return err
		case <-ticker.C:
			job, code, err := dmManager.GetJobById(jobId)
			if code >= mafGenericErrorBase || err != nil {
				err := fmt.Errorf("Failed to get job: %w", err)
				logger.Error(err)
				return err
			}
			allDone, done, pending := allActionsCompleted(job)
			logger.Info(fmt.Sprintf("Job(%s) Checking: %d/%d actions completed (%d pending)", jobId, done, done+pending, pending))
			if allDone {
				logger.Info(fmt.Sprintf("Job(%s) All actions completed", jobId))
				return nil
			}
		}
		if topologyMapper.stopFlag {
			return nil
		}
	}
}

func (topologyMapper *TopologyMapper) enableMAFDevicesSNMPByJob(mafBaseDevices []schema.DeviceBase) error {
	// deviceMappingMap(ActDeviceId(int64), MafDeviceId(string))
	logger.Info("Start enableMAFDevicesSNMPByJob")

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	// Create the config payload
	enable := true
	cfg := netdl.MgmtInterfaceSetting{
		Snmp: &netdl.Snmp{
			Enable: &enable,
		}}
	payload, _ := json.Marshal(cfg)

	// Create Tasks
	var tasks []devicemanager.TaskInput
	for _, mafBaseDevice := range mafBaseDevices {
		task := devicemanager.TaskInput{
			DeviceId: mafBaseDevice.DeviceId,
			Actions: []devicemanager.ActionInput{
				{
					Action:  "setMgmtInterface",
					Payload: payload,
					RetryPolicy: &model.RetryPolicy{
						MaxAttempt: 2,
						Interval:   1,
					},
				},
			},
		}
		tasks = append(tasks, task)
	}

	jobInput := devicemanager.JobInput{
		AutoParallel: true,
		ArpEnable:    true,
		Tasks:        tasks,
	}
	jsonBytes, _ := json.MarshalIndent(jobInput, "", "  ")
	logger.Info(fmt.Sprintf("jobInput: %+v\n", string(jsonBytes)))

	// Create Job
	job, code, err := dmManager.CreateJob(jobInput)
	if err != nil {
		logger.Error(fmt.Sprintf("CreateJob failed: %v (code=%d)\n", err, code))
		return err
	}
	logger.Info(fmt.Sprintf("Job created: %+v\n", job))

	// Wait Job Complete
	err = topologyMapper.waitForJobCompletion(job.JobId, 1, 180) // interval(1 sec), timeout(180 sec)
	if err != nil {
		return err
	}

	return nil
}

func (topologyMapper *TopologyMapper) updateProjectDeviceIpSettingConfigByResult(project domain.Project, mappingResult domain.TopologyMappingResult) error {

	// Clear Project DeviceIpSettingTables
	project.DeviceConfig.DeviceIpSettingTables = make(map[int64]domain.DeviceIpSettingTable)

	if mappingResult.Deploy {
		for _, deviceResult := range mappingResult.MappingReport {
			// Check device mapping status is Success or Warning
			if deviceResult.Status != DeviceMappingResultStatusSuccess.String() &&
				deviceResult.Status != DeviceMappingResultStatusWarning.String() {
				continue
			}

			// Check Online IP address not empty
			if deviceResult.OnlineIpAddress == "" {
				continue
			}

			// Check IP are different
			if deviceResult.OfflineIpAddress == deviceResult.OnlineIpAddress {
				continue
			}

			// Get Project Device by OfflineDeviceId
			projectDevice, err := getProjectDeviceById(project, deviceResult.OfflineDeviceId)
			if err != nil {
				logger.Errorf("Offline device not found at Project. err: %v", err)
				return err
			}

			// Build mapping IP setting table
			ipSetting := domain.DeviceIpSettingTable{
				DeviceId:   deviceResult.OfflineDeviceId,
				OfflineIP:  deviceResult.OfflineIpAddress,
				OnlineIP:   deviceResult.OnlineIpAddress,
				MacAddress: deviceResult.OnlineMacAddress,
				Gateway:    projectDevice.Ipv4.Gateway,
				DNS1:       projectDevice.Ipv4.DNS1,
				DNS2:       projectDevice.Ipv4.DNS2,
			}
			// Subnet mask
			if projectDevice.Ipv4.SubnetMask == "" {
				ipSetting.SubnetMask = "255.255.255.0"
			} else {
				ipSetting.SubnetMask = projectDevice.Ipv4.SubnetMask
			}

			project.DeviceConfig.DeviceIpSettingTables[deviceResult.OfflineDeviceId] = ipSetting
		}
	}

	// Update Core Project DeviceConfig
	isOperation := false
	projectWithDeviceConfig := domain.ProjectWithDeviceConfig{
		Id:           project.Id,
		DeviceConfig: project.DeviceConfig,
	}
	res := core.UpdateProjectDeviceConfig(&projectWithDeviceConfig, isOperation)
	if !res.IsSuccess() {
		err := fmt.Errorf("Failed to update project device config:%s", res.ErrorMessage)
		logger.Error(err)
		return err
	} else {
		logger.Info("Project device config updated successfully")
	}

	return nil
}

func (topologyMapper *TopologyMapper) createMappingTopology(mafDevList []netdl.Device, mafTopology []topology.Connection) (mappingTopology, error) {
	topology := mappingTopology{
		Devices:        []mappingDevice{},
		Links:          []mappingLink{},
		SourceDeviceId: -1,
	}

	// Create Device By mafDevList
	devMacIdMappingMap := make(map[string]int64)
	deviceId := int64(0)
	for _, mafDevice := range mafDevList {
		deviceId = deviceId + 1

		// Get MAC and transfer format(0090e896964a -> 00-90-E8-96-96-4A)
		macAddress, err := transferMACToDashesFormat(mafDevice.MAC)
		if err != nil {
			logger.Warnf("Invalid MAC address format: %v, device: %v, mac: %v", err, mafDevice.IP, mafDevice.MAC)
		}

		// Find DeviceProfile
		profile := common.FindDeviceProfileByModelNameAndModules(topologyMapper.simpleProfiles, mafDevice.ModelName, mafDevice.Modules)
		if profile == nil {
			profile = common.GetUnknownDeviceProfile(topologyMapper.simpleProfiles)
			if profile == nil {
				logger.Errorf("Failed to find device profile for unknown model")
				return topology, fmt.Errorf("failed to find device profile for unknown model")
			}
		}

		device := mappingDevice{
			Id:              deviceId,
			IpAddress:       mafDevice.IP,
			MacAddress:      macAddress,
			ModelName:       profile.ModelName,
			Vendor:          profile.Vendor,
			DeviceType:      profile.DeviceType,
			FirmwareVersion: mafDevice.FirmwareVersion,
			SerialNumber:    mafDevice.SerialNumber,
			DeviceProfileId: int64(profile.Id),
			BuiltInPower:    profile.BuiltInPower,
		}

		// Generate ModularConfiguration
		var modularConfiguration *domain.ModularConfiguration
		if mafDevice.Modules != nil {
			device.ModularInfo = *mafDevice.Modules
			var err error
			modularConfiguration, err = configmapper.MappingDeviceModules(mafDevice.Modules)
			if err != nil {
				logger.Warnf("Failed to map device modules for maf device %s(%s): %v", mafDevice.DeviceId, mafDevice.IP, err)
			}
		}
		if modularConfiguration != nil {
			device.ModularConfiguration = *modularConfiguration
		} else {
			device.ModularConfiguration = domain.ModularConfiguration{
				Ethernet: map[string]int64{},
				Power:    map[string]int64{},
			}
		}

		// Append device
		topology.Devices = append(topology.Devices, device)

		devMacIdMappingMap[macAddress] = deviceId
	}

	// Create Link & Find Management Endpoint(SourceDeviceId)
	linkId := int64(0)
	for _, mafConn := range mafTopology {
		// Check the fromPort exist
		fromDevMac, err := transferMACToDashesFormat(mafConn.FromDevice.MAC) // (00:90:e8:96:95:cd -> 00-90-E8-96-95-CD)
		if err != nil {
			logger.Warnf("Invalid MAC address format: %v, device: %v, mac: %v", err, mafConn.FromDevice.IP, mafConn.FromDevice.MAC)
			continue
		}

		if fromDevId, ok := devMacIdMappingMap[fromDevMac]; ok {
			formDevPort, err := internal.GetPortFromString(mafConn.FromPort)
			if err != nil {
				jsonBytes, _ := json.MarshalIndent(mafConn, "", "  ")
				logger.Warnf("Failed to get the FromPort value: %v,\n At the MAF connection: %v", err, string(jsonBytes))
				continue
			}

			// Check the toDevice is Management Endpoint
			if mafConn.ToDevice.IsSelf != nil && *mafConn.ToDevice.IsSelf {
				topology.SourceDeviceId = int64(fromDevId)
			}

			// Check the toPort exist
			toDevMac, err := transferMACToDashesFormat(mafConn.ToDevice.MAC) // (00:90:e8:96:95:cd -> 00-90-E8-96-95-CD)
			if err != nil {
				logger.Warnf("Invalid MAC address format: %v, device: %v, mac: %v", err, mafConn.ToDevice.IP, mafConn.ToDevice.MAC)
				continue
			}

			if toDevId, ok := devMacIdMappingMap[toDevMac]; ok {
				toDevPort, err := internal.GetPortFromString(mafConn.ToPort)
				if err != nil {
					jsonBytes, _ := json.MarshalIndent(mafConn, "", "  ")
					logger.Warnf("Failed to get the ToPort value: %v,\n At the MAF connection: %v", err, string(jsonBytes))
					continue
				}
				// Append link
				linkId = linkId + 1
				link := mappingLink{
					Id:                     linkId,
					SourceDeviceId:         fromDevId,
					SourceInterfaceId:      int64(formDevPort),
					DestinationDeviceId:    toDevId,
					DestinationInterfaceId: int64(toDevPort),
				}
				topology.Links = append(topology.Links, link)
			}
		}

	}
	return topology, nil
}

func (topologyMapper *TopologyMapper) getPhysicalTopology(project domain.Project) (mappingTopology, error) {
	logger.Info("Start getPhysicalTopology")
	var topology mappingTopology

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return topology, err
	}

	// Enable Set Arp table
	dmManager.AutoSetArp(true)
	defer dmManager.AutoSetArp(false)

	// Search Device by BroadcastDiscovery
	var code int
	code, err = dmManager.Search(dmschema.DeviceSearchInput{
		Method:  devicemanager.BroadcastDiscovery,
		Timeout: broadcastSearchTimeout,
	})
	if code >= mafGenericErrorBase || err != nil {
		return topology, err
	}

	if topologyMapper.stopFlag {
		return topology, nil
	}

	topologyMapper.onProgressResponse(topologyMapper.wsConnId, 50)

	mafBaseDevices, err := dmManager.GetDevices(dmschema.GetDeviceListInput{})
	if err != nil {
		logger.Error("Error getting devices:", err)
		return topology, err
	}

	// Update MAF Device Secret
	for _, mafBaseDevice := range mafBaseDevices {
		prjDev, err := getProjectDeviceByIp(project, mafBaseDevice.IP)
		if err != nil {
			continue
		}
		// Update Design Device Secret
		err = updateMAFDeviceSecret(mafBaseDevice, prjDev)
		if err != nil {
			logger.Warnf("UpdateMAFDeviceSecret failed. Device: %s(%s), Error: %v", mafBaseDevice.IP, mafBaseDevice.MAC, err)
			continue
		}
		// if err != nil {
		// 	// Update Design Device Secret
		// 	err = updateMAFDeviceSecret(mafBaseDevice, prjDev)
		// 	if err != nil {
		// 		continue
		// 	}
		// } else {
		// 	// Update Project Setting Secret
		// 	err = updateMAFDeviceSecretByProjectSetting(mafBaseDevice, project.ProjectSetting)
		// 	if err != nil {
		// 		continue
		// 	}
		// }

	}

	// Force enable SNMP service
	_ = topologyMapper.enableMAFDevicesSNMPByJob(mafBaseDevices)
	if topologyMapper.stopFlag {
		return topology, nil
	}

	time.Sleep(2 * time.Second) // sleep 2 second

	// Fetch MAF device info
	var mafDevList []netdl.Device
	var mafDevIdList []string
	for _, mafBaseDevice := range mafBaseDevices {
		if topologyMapper.stopFlag {
			return topology, nil
		}

		// Fetch DeviceInfo
		deviceBase, _, err := dmManager.FetchDeviceInfo(mafBaseDevice.DeviceId)
		if err == nil {
			mafBaseDevice = deviceBase
		}

		mafDevice := netdl.Device{
			DeviceBasic: netdl.DeviceBasic(mafBaseDevice),
		}
		mafDevIdList = append(mafDevIdList, mafBaseDevice.DeviceId)
		deviceInfo := fmt.Sprintf("ID: %v, IP: %v, ModelName: %v, SerialNumber: %v, FirmwareVersion: %v, Location: %v, MAC: %v", mafBaseDevice.DeviceId, mafBaseDevice.IP, mafBaseDevice.ModelName, mafBaseDevice.SerialNumber, mafBaseDevice.FirmwareVersion, mafBaseDevice.Location, mafBaseDevice.MAC)
		logger.Info(deviceInfo)

		// Get Device Modules
		modules, _, err := dmManager.GetModules(mafBaseDevice.DeviceId)
		if err == nil {
			mafDevice.Modules = &modules
		}

		mafDevList = append(mafDevList, mafDevice)
	}
	topologyMapper.onProgressResponse(topologyMapper.wsConnId, 60)

	// TODO: use job
	// err = topologyMapper.assignMafDevsModularByJob(mafDevList)
	// if err != nil {
	// 	logger.Error("AssignDevicesModularByJob() failed:", err)
	// 	return topology, err
	// }
	// Get Physical topology by MAF DM
	mafTopology, code, err := dmManager.GetTopology(model.SortRequest{
		Devices:   mafDevIdList,
		EnableArp: true,
	})
	if code >= mafGenericErrorBase || err != nil {
		return topology, err
	}
	jsonBytes, _ := json.MarshalIndent(mafTopology, "", "  ")
	logger.Info("MAF Topology Result:\n" + string(jsonBytes))
	for _, mafDevice := range mafDevList {
		jsonBytes, _ := json.MarshalIndent(mafDevice, "", "  ")
		logger.Info("MafDevice:\n" + string(jsonBytes))

	}
	if topologyMapper.stopFlag {
		return topology, nil
	}

	// Create the Mapping Topology
	topology, err = topologyMapper.createMappingTopology(mafDevList, mafTopology)
	if err != nil {
		logger.Error("Create MappingTopology failed. err:", err)
		return topology, err
	}

	return topology, nil
}

func (topologyMapper *TopologyMapper) createMappingTopologyByProject(project domain.Project) mappingTopology {
	topology := mappingTopology{
		Devices:        []mappingDevice{},
		Links:          []mappingLink{},
		SourceDeviceId: -1,
	}

	// Device
	for _, prjDev := range project.Devices {

		// Get DeviceProfile
		profile := common.GetDeviceProfileById(topologyMapper.simpleProfiles, prjDev.DeviceProfileId)
		if profile == nil {
			logger.Errorf("Failed to find device profile. ID: %v", prjDev.DeviceProfileId)
			continue
		}

		device := mappingDevice{
			Id:                   prjDev.Id,
			IpAddress:            prjDev.Ipv4.IpAddress,
			MacAddress:           prjDev.MacAddress,
			ModelName:            prjDev.DeviceProperty.ModelName,
			Vendor:               prjDev.DeviceProperty.Vendor,
			DeviceType:           prjDev.DeviceType,
			FirmwareVersion:      prjDev.FirmwareVersion,
			SerialNumber:         "",
			DeviceProfileId:      int64(prjDev.DeviceProfileId),
			ModularConfiguration: prjDev.ModularConfiguration,
			BuiltInPower:         profile.BuiltInPower,
		}
		// Append device
		topology.Devices = append(topology.Devices, device)
	}

	// Link
	for _, prjLink := range project.Links {
		link := mappingLink{
			Id:                     int64(prjLink.LinkInfo.Id),
			SourceDeviceId:         int64(prjLink.LinkInfo.LinkConf.SourceDeviceId),
			SourceInterfaceId:      int64(prjLink.LinkInfo.LinkConf.SourceInterfaceId),
			DestinationDeviceId:    int64(prjLink.LinkInfo.LinkConf.DestinationDeviceId),
			DestinationInterfaceId: int64(prjLink.LinkInfo.LinkConf.DestinationInterfaceId),
		}
		// Append link
		topology.Links = append(topology.Links, link)
	}
	return topology
}

func (topologyMapper *TopologyMapper) findOfflineSourceCandidates(offlineTopology mappingTopology, onlineTopology mappingTopology) ([]int64, error) {
	offlineSourceCandidates := make([]int64, 0)
	jsonBytes, _ := json.MarshalIndent(offlineTopology, "", "  ")
	logger.Info("OfflineTopology:\n" + string(jsonBytes))

	// Get the online Management Endpoint(source device)
	onlineSrcDevId := onlineTopology.SourceDeviceId
	onlineSrcDev, err := findDeviceById(onlineTopology.Devices, onlineSrcDevId)
	if err != nil {
		logger.Errorf("Online source device not found. err: %v", err)
		return nil, err
	}

	// jsonBytes, _ = json.MarshalIndent(onlineSrcDev, "", "  ")
	// logger.Info("OnlineSrcDev:\n" + string(jsonBytes))

	// Find the Offline source device(design Management Endpoint) candidates
	var sourceDevCandidates []sourceDeviceCandidate
	for _, offlineDev := range offlineTopology.Devices {
		// logger.Infof("offlineDev.ModelName: %s", offlineDev.ModelName)

		// Check same model
		if offlineDev.ModelName == onlineSrcDev.ModelName {
			ipNum := mustIPv4ToU32(offlineDev.IpAddress)
			if ipNum == 0 {
				logger.Warnf("Skip the Offline device whose IP address cannot be parsed. DeviceID: %v, IP:", offlineDev.Id, offlineDev.IpAddress)
				continue
			}
			sourceDevCandidates = append(sourceDevCandidates, sourceDeviceCandidate{
				DeviceId: offlineDev.Id,
				IpNum:    ipNum,
			})
		}
	}
	logger.Info(fmt.Sprintf("sourceDevCandidates size:%d", len(sourceDevCandidates)))

	// Sort sourceDevCandidates
	// Priority:
	// - 1. Same IP address
	// - 2. IP address number from small to large
	onlineSrcDevIpNum := mustIPv4ToU32(onlineSrcDev.IpAddress)
	sort.Slice(sourceDevCandidates, func(i, j int) bool {
		x, y := sourceDevCandidates[i], sourceDevCandidates[j]

		xSame := x.IpNum == onlineSrcDevIpNum
		ySame := y.IpNum == onlineSrcDevIpNum
		if xSame != ySame {
			return xSame // true -> first
		}

		return x.IpNum < y.IpNum // smaller first
	})

	// Append to result
	logger.Info("Offline Source device candidates:")
	for _, candidate := range sourceDevCandidates {
		offlineSourceCandidates = append(offlineSourceCandidates, candidate.DeviceId)
		logger.Infof("%v,", candidate.DeviceId)
	}

	return offlineSourceCandidates, nil
}

func (topologyMapper *TopologyMapper) checkDevice(offlineDevice mappingDevice,
	onlineDevice mappingDevice,
	builtInPower bool,
	offlineDevicesResult map[int64]*domain.MappingDeviceResultItem) error {
	offlineId := offlineDevice.Id

	// Avoid duplicated check
	if _, exists := offlineDevicesResult[offlineId]; exists {
		return nil
	}

	// 1. Check Model Name
	if offlineDevice.ModelName != onlineDevice.ModelName {
		errorMsg := "Check Model Name failed"

		item := createMappingDeviceResultItem(
			offlineDevice,
			onlineDevice,
			builtInPower,
			DeviceMappingResultStatusFailed.String(),
			errorMsg,
		)
		offlineDevicesResult[offlineId] = &item
		return fmt.Errorf(errorMsg)
	}

	// 2. Check Power Module (only when not built-in power)
	if !builtInPower {
		if !equalMapStringInt64(
			offlineDevice.ModularConfiguration.Power,
			onlineDevice.ModularConfiguration.Power,
		) {
			errorMsg := "Check Power Module failed"

			item := createMappingDeviceResultItem(
				offlineDevice,
				onlineDevice,
				builtInPower,
				DeviceMappingResultStatusFailed.String(),
				errorMsg,
			)
			offlineDevicesResult[offlineId] = &item

			return fmt.Errorf(errorMsg)
		}
	}

	// 3. Check Ethernet Module (Skip built-in ethernet)
	if !equalMapStringInt64(
		offlineDevice.ModularConfiguration.Ethernet,
		onlineDevice.ModularConfiguration.Ethernet,
	) {
		errorMsg := "Check Line Module failed"
		item := createMappingDeviceResultItem(
			offlineDevice,
			onlineDevice,
			builtInPower,
			DeviceMappingResultStatusFailed.String(),
			errorMsg,
		)
		offlineDevicesResult[offlineId] = &item

		return fmt.Errorf(errorMsg)
	}

	// All checks passed -> Success
	item := createMappingDeviceResultItem(
		offlineDevice,
		onlineDevice,
		builtInPower,
		DeviceMappingResultStatusSuccess.String(),
		"",
	)
	offlineDevicesResult[offlineId] = &item

	return nil
}

func (topologyMapper *TopologyMapper) mappingTopology(offlineTopology mappingTopology, onlineTopology mappingTopology) (domain.TopologyMappingResult, error) {
	logger.Info("Start mappingTopology()")

	var result domain.TopologyMappingResult

	// Result maps and sets
	offlineDevicesResult := make(map[int64]*domain.MappingDeviceResultItem) // <offlineDeviceId, resultItem>
	foundOnlineDeviceIdMap := make(map[int64]struct{})
	onlineMappedDeviceIdMap := make(map[int64]struct{})
	offlineNotFoundPortDeviceIdMap := make(map[int64]struct{})
	// offlineDeviceCheckFailedIdMap := make(map[int64]struct{})

	// Prepare data
	// offlineDeviceLinksMap / onlineDeviceLinksMap: map[DeviceID][]Link
	offlineDeviceLinksMap := generateDeviceLinksMap(offlineTopology)
	onlineDeviceLinksMap := generateDeviceLinksMap(onlineTopology)

	// offline_mapping_device_id_set: the set of offline devices which should be mapped (MOXA devices)
	offlineMappingDeviceIdMap := generateOfflineMappingDeviceMap(offlineTopology)

	// BFS queue for offline devices
	var offlineDeviceIdQueue []int64
	onlineDeviceId := onlineTopology.SourceDeviceId

	// Check source device is a MOXA device
	if _, ok := offlineMappingDeviceIdMap[offlineTopology.SourceDeviceId]; ok {
		offlineDeviceIdQueue = append(offlineDeviceIdQueue, offlineTopology.SourceDeviceId)
	}

	// BFS over offline devices while there are devices in queue and online topology is not empty
	for len(offlineDeviceIdQueue) > 0 && len(onlineTopology.Devices) > 0 {
		// dequeue
		offlineDeviceId := offlineDeviceIdQueue[0]
		offlineDeviceIdQueue = offlineDeviceIdQueue[1:]

		if offlineDeviceId != offlineTopology.SourceDeviceId { // not first (not source device)
			// Get online_device_id from previous mapping result
			onlineDeviceId = -1

			offlineResult, ok := offlineDevicesResult[offlineDeviceId]
			if !ok { // next offline device not found in offlineDevicesResult
				logger.Infof("Offline device not in offlineDevicesResult, skip it. DeviceID=%d", offlineDeviceId)
				continue
			}

			onlineDeviceId = offlineResult.OnlineDeviceId
			if onlineDeviceId == -1 {
				continue
			}

			// [bugfix:2885] Topology mapping - mapping result show 1 online IP twice
			if _, ok := onlineMappedDeviceIdMap[onlineDeviceId]; ok {
				continue
			}
		}

		// Find offline device
		offlineDevice, err := findDeviceById(offlineTopology.Devices, offlineDeviceId)
		if err != nil {
			logger.Error("Offline device(ID:%d) not found in the Offline Topology. err: %v", offlineDeviceId, err)
			return result, err
		}

		// Find online device
		onlineDevice, err := findDeviceById(onlineTopology.Devices, onlineDeviceId)
		if err != nil {
			logger.Error("Online device(ID:%d) not found in the Online Topology. err: %v", onlineDeviceId, err)
			return result, err
		}

		// Mark online device as found & mapped
		foundOnlineDeviceIdMap[onlineDeviceId] = struct{}{}
		onlineMappedDeviceIdMap[onlineDeviceId] = struct{}{}

		// Avoid duplicated check
		if _, exists := offlineDevicesResult[offlineDeviceId]; !exists {
			// CheckDevice will populate offlineDevicesResult and offlineDeviceCheckFailedIdMap
			topologyMapper.checkDevice(
				offlineDevice,
				onlineDevice,
				onlineDevice.BuiltInPower,
				offlineDevicesResult,
			)
		}

		// Enqueue neighbor devices by link
		for _, neighborLink := range offlineDeviceLinksMap[offlineDeviceId] {
			// Destination
			nextDeviceID := neighborLink.DestinationDeviceId
			if _, checked := offlineDevicesResult[nextDeviceID]; !checked &&
				!containsInt64(offlineDeviceIdQueue, nextDeviceID) {
				if _, isMoxa := offlineMappingDeviceIdMap[nextDeviceID]; isMoxa {
					offlineDeviceIdQueue = append(offlineDeviceIdQueue, nextDeviceID)
				}
			}

			// Source
			nextDeviceID = neighborLink.SourceDeviceId
			if _, checked := offlineDevicesResult[nextDeviceID]; !checked &&
				!containsInt64(offlineDeviceIdQueue, nextDeviceID) {
				if _, isMoxa := offlineMappingDeviceIdMap[nextDeviceID]; isMoxa {
					offlineDeviceIdQueue = append(offlineDeviceIdQueue, nextDeviceID)
				}
			}
		}

		// Get offline leave_if_id -> opposite_dev map: <leave_if_id, opposite_device_id>
		offlineLeaveIfOppositeDevMap := generateLeaveInterfaceOppositeDeviceMap(
			true,
			offlineDeviceId,
			offlineDeviceLinksMap[offlineDeviceId],
			offlineMappingDeviceIdMap,
		)

		// Get online leave_if_id -> opposite_dev map: <leave_if_id, opposite_device_id>
		onlineLeaveIfOppositeDevMap := generateLeaveInterfaceOppositeDeviceMap(
			false,
			onlineDeviceId,
			onlineDeviceLinksMap[onlineDeviceId],
			offlineMappingDeviceIdMap,
		)

		// Get offline device's interface name map: <interface_id, interface_name>
		// offlineDeviceInterfacesNameMap := offlineDevice.GetInterfacesIdNameMap()

		// Check offline one-hop device & link
		for leaveIfId, offlineOppositeDeviceID := range offlineLeaveIfOppositeDevMap {
			// // Get leave_if_name
			// leaveIfName, ok := offlineDeviceInterfacesNameMap[leaveIfId]
			// if !ok {
			// 	leaveIfName = fmt.Sprintf("%d", leaveIfId)
			// }

			// If online leave interface not found -> mark as Failed, record "Port (...) not found"
			if _, found := onlineLeaveIfOppositeDevMap[leaveIfId]; !found {
				item, exists := offlineDevicesResult[offlineDeviceId]
				// msg := fmt.Sprintf("Port (%s) not found", leaveIfName)
				msg := fmt.Sprintf("Port (%d) not found", leaveIfId)

				if !exists {
					// First time set failed result for this offline device
					newItem := createMappingDeviceResultItem(
						offlineDevice,
						onlineDevice,
						onlineDevice.BuiltInPower,
						DeviceMappingResultStatusFailed.String(),
						msg,
					)
					offlineDevicesResult[offlineDeviceId] = &newItem
					offlineNotFoundPortDeviceIdMap[offlineDeviceId] = struct{}{}
				} else {
					// Already has a result
					if item.ErrorMessage == "" {
						// First error message
						newItem := createMappingDeviceResultItem(
							offlineDevice,
							onlineDevice,
							onlineDevice.BuiltInPower,
							DeviceMappingResultStatusFailed.String(),
							msg,
						)
						offlineDevicesResult[offlineDeviceId] = &newItem
						offlineNotFoundPortDeviceIdMap[offlineDeviceId] = struct{}{}
					} else {
						// Append error message
						item.ErrorMessage = fmt.Sprintf("%s, %s", item.ErrorMessage, msg)
					}
				}
				continue
			}

			// Has leave_if on online side -> check opposite device mapping

			// Avoid duplicated check: if offline opposite device already in result, skip
			if _, alreadyChecked := offlineDevicesResult[offlineOppositeDeviceID]; alreadyChecked {
				continue
			}

			// Find offline opposite device
			offlineOppositeDevice, err := findDeviceById(offlineTopology.Devices, offlineOppositeDeviceID)
			if err != nil {
				err := fmt.Errorf(
					"offline opposite device not found in offline topology. deviceID=%d",
					offlineOppositeDeviceID,
				)
				logger.Error(err)
				return result, err
			}

			// Find online opposite device
			onlineOppositeDeviceID := onlineLeaveIfOppositeDevMap[leaveIfId]
			onlineOppositeDevice, err := findDeviceById(onlineTopology.Devices, onlineOppositeDeviceID)
			if err != nil {
				err := fmt.Errorf(
					"online opposite device not found in online topology. deviceID=%d",
					onlineOppositeDeviceID,
				)
				logger.Error(err)
				return result, err
			}

			// Mark online opposite device as found
			foundOnlineDeviceIdMap[onlineOppositeDeviceID] = struct{}{}

			// [bugfix:2885] Topology mapping - mapping result show 1 online IP twice
			if _, mapped := onlineMappedDeviceIdMap[onlineOppositeDeviceID]; mapped {
				continue
			}

			// Device check for opposite device
			// CheckDevice may fail; if so, just continue (non-fatal for whole mapping)
			if err := topologyMapper.checkDevice(
				offlineOppositeDevice,
				onlineOppositeDevice,
				onlineOppositeDevice.BuiltInPower,
				offlineDevicesResult,
			); err != nil {
				continue
			}
		}
	}

	// Update warning status
	for _, offlineDeviceResult := range offlineDevicesResult {
		if offlineDeviceResult.Status == DeviceMappingResultStatusFailed.String() {
			offlineDeviceId := offlineDeviceResult.OfflineDeviceId

			// Only check devices that have "port not found" error
			if _, hasNotFoundPort := offlineNotFoundPortDeviceIdMap[offlineDeviceId]; hasNotFoundPort {
				// Check its links
				for _, deviceLink := range offlineDeviceLinksMap[offlineDeviceId] {
					// Get opposite device id
					var oppositeOfflineDeviceId int64
					if deviceLink.SourceDeviceId == offlineDeviceId {
						oppositeOfflineDeviceId = deviceLink.DestinationDeviceId
					} else {
						oppositeOfflineDeviceId = deviceLink.SourceDeviceId
					}

					// If opposite device also has "port not found", downgrade to Warning
					if _, alsoNotFound := offlineNotFoundPortDeviceIdMap[oppositeOfflineDeviceId]; alsoNotFound {
						offlineDeviceResult.Status = DeviceMappingResultStatusWarning.String()
					}
				}
			}
		}
	}

	// Append other devices to result (Offline)
	offlineEndStationIdMap := make(map[int64]struct{})
	for _, offlineDevice := range offlineTopology.Devices {
		if _, exists := offlineDevicesResult[offlineDevice.Id]; !exists {
			// If this is not a MOXA device, mark as SKIP.
			newItem := domain.MappingDeviceResultItem{
				OfflineDeviceId:  offlineDevice.Id,
				OfflineIpAddress: offlineDevice.IpAddress,
				OfflineModelName: offlineDevice.ModelName,
			}

			if _, isMoxa := offlineMappingDeviceIdMap[offlineDevice.Id]; !isMoxa {
				newItem.Status = DeviceMappingResultStatusSkip.String()
			} else {
				newItem.Status = DeviceMappingResultStatusNotFound.String()
				newItem.ErrorMessage = "Online device not found"
			}

			offlineDevicesResult[offlineDevice.Id] = &newItem
		}
	}

	// Generate final topology mapping result from collected data
	result = generateTopologyMappingResult(
		offlineDevicesResult,
		foundOnlineDeviceIdMap,
		offlineEndStationIdMap,
		onlineTopology,
	)

	return result, nil
}

func (topologyMapper *TopologyMapper) compareTopologyByCandidates(offlineTopology mappingTopology, onlineTopology mappingTopology, offlineSourceCandidates []int64) (domain.TopologyMappingResult, error) {
	var mappingResult domain.TopologyMappingResult

	// Check the source candidate not empty
	if len(offlineSourceCandidates) == 0 {
		mappingResult, err := topologyMapper.mappingTopology(offlineTopology, onlineTopology)
		if err != nil {
			return mappingResult, err
		}
		return mappingResult, nil
	}

	// For each candidate offline source device, try mapping topology
	var mappingResultCandidates []topologyMappingResultCandidate

	for _, offlineSrcDevId := range offlineSourceCandidates {
		if topologyMapper.stopFlag {
			return mappingResult, nil
		}

		candidateOfflineTopology := offlineTopology
		candidateOfflineTopology.SourceDeviceId = offlineSrcDevId

		// Execute the topology mapping algorithm
		topologyMappingResult, err := topologyMapper.mappingTopology(candidateOfflineTopology, onlineTopology)
		if err != nil {
			continue
		}
		candidate := createTopologyMappingResultCandidate(topologyMappingResult, candidateOfflineTopology)

		// If: (deployable) & (no warning item) & (no failed item)
		// Directly use this result
		if topologyMappingResult.Deploy &&
			candidate.WarningItemNum == 0 &&
			candidate.FailedItemNum == 0 {
			return topologyMappingResult, nil
		}

		// Otherwise, store it for later evaluation
		mappingResultCandidates = append(mappingResultCandidates, candidate)
	}

	// If no perfect result was found (no direct deploy success), we select the best result among all candidates.
	if len(mappingResultCandidates) > 0 {
		// Sort candidates based on:
		// 1. Deploy success: deployable candidates come first.
		//    - Among deployable candidates:
		//        a. fewer warning items (ascending)
		//        b. smaller IP number (ascending)
		//
		// 2. Deploy failed:
		//    - Among failed candidates:
		//        a. fewer failed items (ascending)
		//        b. smaller IP number (ascending)
		sort.Slice(mappingResultCandidates, func(i, j int) bool {
			x := mappingResultCandidates[i]
			y := mappingResultCandidates[j]

			xResult := x.TopologyMappingResult
			yResult := y.TopologyMappingResult

			xDeploy := xResult.Deploy
			yDeploy := yResult.Deploy

			// Case 1: both candidates can deploy
			if xDeploy && yDeploy {
				if x.WarningItemNum != y.WarningItemNum {
					return x.WarningItemNum < y.WarningItemNum
				}
				return x.OfflineSourceIpNum < y.OfflineSourceIpNum
			}

			// Case 2: both candidates cannot deploy
			if !xDeploy && !yDeploy {
				if x.FailedItemNum != y.FailedItemNum {
					return x.FailedItemNum < y.FailedItemNum
				}
				return x.OfflineSourceIpNum < y.OfflineSourceIpNum
			}

			// Case 3: one deploys and one does not — deployable candidate comes first
			if xDeploy && !yDeploy {
				return true
			}

			// x fails, y succeeds
			return false
		})

		// Select the best candidate as the final mapping result
		best := mappingResultCandidates[0]
		mappingResult = best.TopologyMappingResult
	}

	return mappingResult, nil
}

// func (topologyMapper *TopologyMapper) waitForJobCompletion(jobId string, intervalSeconds int, timeoutSeconds int) error {
// 	logger.Info("Start waitForJobCompletion")

// 	timeout := time.After(time.Duration(timeoutSeconds) * time.Second)
// 	ticker := time.NewTicker(time.Duration(intervalSeconds) * time.Second)
// 	defer ticker.Stop()
// 	dmManager, err := dipool.GetDMManager()
// 	if err != nil {
// 		return err
// 	}

// 	// local funct (topologyMapper *TopologyMapper)ion: check if all actions are completed, return stats
// 	allActionsCompleted := func(job *model.Job) (bool, int, int) {
// 		total := 0
// 		done := 0

// 		for _, tasks := range job.Pipeline {
// 			for _, task := range tasks {
// 				for _, action := range task.Actions {
// 					total++
// 					if action.CompletedAt != nil {
// 						done++
// 						logger.Info(fmt.Sprintf("action.Result: %v", action.Result))
// 					}
// 				}
// 			}
// 		}
// 		return done == total, done, total - done
// 	}
// 	for {
// 		select {
// 		case <-timeout:
// 			err := fmt.Errorf("Job(%s) did not complete within timeout(%d sec)", jobId, timeoutSeconds)
// 			logger.Error(err)
// 			return err
// 		case <-ticker.C:
// 			job, code, err := dmManager.GetJobById(jobId)
// 			if code >= MafGenericErrorBase || err != nil {
// 				err := fmt.Errorf("Failed to get job: %w", err)
// 				logger.Error(err)
// 				return err
// 			}
// 			allDone, done, pending := allActionsCompleted(job)
// 			logger.Info(fmt.Sprintf("Job(%s) Checking: %d/%d actions completed (%d pending)", jobId, done, done+pending, pending))
// 			if allDone {
// 				logger.Info(fmt.Sprintf("Job(%s) All actions completed", jobId))
// 				return nil
// 			}
// 		}
// 		if topologyMapper.stopFlag {
// 			return nil
// 		}
// 	}
// }
// func (topologyMapper *TopologyMapper) assignMafDevsModularByJob(mafDevList []netdl.Device) error {
// 	logger.Info("Start assignMafDevsModularByJob")

// 	dmManager, err := dipool.GetDMManager()
// 	if err != nil {
// 		return err
// 	}

// 	// Create the config payload
// 	enable := true
// 	cfg := netdl.MgmtInterfaceSetting{
// 		Snmp: &netdl.Snmp{
// 			Enable: &enable,
// 		}}
// 	payload, _ := json.Marshal(cfg)

// 	// Create Tasks
// 	var tasks []devicemanager.TaskInput
// 	var index int
// 	for _, mafDev := range mafDevList {
// 		task := devicemanager.TaskInput{
// 			DeviceId: mafDev.DeviceId,
// 			Actions: []devicemanager.ActionInput{
// 				{
// 					Action:  "setMgmtInterface",
// 					Payload: payload,
// 					RetryPolicy: &model.RetryPolicy{
// 						MaxAttempt: 2,
// 						Interval:   1,
// 					},
// 				},
// 			},
// 		}
// 		tasks = append(tasks, task)
// 		index = index + 1
// 	}

// 	jobInput := devicemanager.JobInput{
// 		AutoParallel: true,
// 		ArpEnable:    true,
// 		Tasks:        tasks,
// 	}
// 	logger.Info(fmt.Sprintf("jobInput: %+v\n", jobInput))

// 	// Create Job
// 	job, code, err := dmManager.CreateJob(jobInput)
// 	if err != nil {
// 		logger.Error(fmt.Sprintf("CreateJob failed: %v (code=%d)\n", err, code))
// 		return err
// 	}
// 	logger.Info(fmt.Sprintf("Job created: %+v\n", job))

// 	// Wait Job Complete
// 	err = topologyMapper.waitForJobCompletion(job.JobId, 1, 180) // interval(1 sec), timeout(180 sec)
// 	if err != nil {
// 		return err
// 	}

// 	return nil
// }
