package topologymapping

import (
	"bytes"
	"fmt"
	"net"
	"sort"
	"strconv"
	"strings"

	"encoding/binary"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/macutility"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/configmapper"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
	dmschema "gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
)

func containsInt64(list []int64, v int64) bool {
	for _, x := range list {
		if x == v {
			return true
		}
	}
	return false
}

func equalMapStringInt64(a, b map[string]int64) bool {
	if a == nil && b == nil {
		return true
	}
	if a == nil || b == nil {
		return false
	}
	if len(a) != len(b) {
		return false
	}

	for k, v := range a {
		if bv, ok := b[k]; !ok || bv != v {
			return false
		}
	}
	return true
}

func generateTopologyMappingResult(
	offlineDevicesResult map[int64]*domain.MappingDeviceResultItem,
	foundOnlineDevices map[int64]struct{},
	offlineEndStationDevices map[int64]struct{},
	onlineTopology mappingTopology,
) domain.TopologyMappingResult {

	var result domain.TopologyMappingResult

	// mapping_offline_ip_set: IPs of offline devices that are mapped (Success/Warning, and not end stations)
	mappingOfflineIPSet := make(map[string]struct{})
	// offline_end_station_checked_ip_set: IPs of offline end stations that are already checked
	offlineEndStationCheckedIpMap := make(map[string]struct{})
	offlineDevicesCanDeploy := true

	// 1) Generate result from offline devices
	for _, item := range offlineDevicesResult {
		itemID := int64(len(result.MappingReport) + 1)
		item.Id = itemID

		// Collect IPs of mapped offline devices (excluding EndStation)
		if item.Status == DeviceMappingResultStatusSuccess.String() || item.Status == DeviceMappingResultStatusWarning.String() {
			// Excluding EndStation
			if _, isEndStation := offlineEndStationDevices[item.OfflineDeviceId]; !isEndStation {
				mappingOfflineIPSet[item.OfflineIpAddress] = struct{}{}
			}
		}

		// Collect checked EndStation IPs
		if item.Status == DeviceMappingResultStatusChecked.String() {
			offlineEndStationCheckedIpMap[item.OfflineIpAddress] = struct{}{}
		}

		result.MappingReport = append(result.MappingReport, *item)

		// Check whether offline devices as a whole can be deployed
		if item.Status == DeviceMappingResultStatusFailed.String() || item.Status == DeviceMappingResultStatusNotFound.String() {
			offlineDevicesCanDeploy = false
		}
	}

	// 2) Append online devices that were not mapped
	for _, onlineDev := range onlineTopology.Devices {
		if _, found := foundOnlineDevices[onlineDev.Id]; found {
			continue
		}

		itemID := int64(len(result.MappingReport) + 1)
		deviceItem := domain.MappingDeviceResultItem{
			Id:               itemID,
			OnlineDeviceId:   onlineDev.Id,
			OnlineIpAddress:  onlineDev.IpAddress,
			OnlineModelName:  onlineDev.ModelName,
			OnlineMacAddress: onlineDev.MacAddress,
		}

		// When offline devices can deploy, check if IP duplicates with any mapped offline device
		if offlineDevicesCanDeploy {
			if _, dup := mappingOfflineIPSet[deviceItem.OnlineIpAddress]; dup {
				deviceItem.Status = DeviceMappingResultStatusFailed.String()
				deviceItem.ErrorMessage = "The IP duplicated with the offline design device"
			} else {
				deviceItem.Status = DeviceMappingResultStatusSkip.String()
			}
		} else {
			deviceItem.Status = DeviceMappingResultStatusSkip.String()
		}

		// Skip Checked EndStation
		if deviceItem.Status == DeviceMappingResultStatusSkip.String() {
			if _, checked := offlineEndStationCheckedIpMap[deviceItem.OnlineIpAddress]; checked {
				continue
			}
		}

		result.MappingReport = append(result.MappingReport, deviceItem)
	}

	// 3) Sort result by Id
	sort.Slice(result.MappingReport, func(i, j int) bool {
		return result.MappingReport[i].Id < result.MappingReport[j].Id
	})

	// 4) Check can deploy
	canDeploy := true
	for _, item := range result.MappingReport {
		if item.Status == DeviceMappingResultStatusFailed.String() || item.Status == DeviceMappingResultStatusNotFound.String() {
			canDeploy = false
			break
		}
	}
	result.Deploy = canDeploy

	return result
}

func createTopologyMappingResultCandidate(topologyMappingResult domain.TopologyMappingResult, offlineTopology mappingTopology) topologyMappingResultCandidate {
	offlineSrcIpNum := uint32(0)
	offlineSrcDev, err := findDeviceById(offlineTopology.Devices, offlineTopology.SourceDeviceId)
	if err != nil {
		logger.Warnf("Offline device(ID:%d) not found in the Offline Topology. err: %v", offlineTopology.SourceDeviceId, err)
	}
	offlineSrcIpNum = mustIPv4ToU32(offlineSrcDev.IpAddress)

	warningItemNum := uint32(0)
	failedItemNum := uint32(0)

	for _, item := range topologyMappingResult.MappingReport {
		// Warning
		if item.Status == DeviceMappingResultStatusWarning.String() {
			warningItemNum = warningItemNum + 1
			continue
		}
		// Failed (or NotFound)
		if item.Status == DeviceMappingResultStatusFailed.String() {
			failedItemNum = failedItemNum + 1
			continue
		}
		if item.Status == DeviceMappingResultStatusNotFound.String() {
			failedItemNum = failedItemNum + 1
			continue
		}
	}

	return topologyMappingResultCandidate{
		OfflineSourceIpNum:    offlineSrcIpNum,
		TopologyMappingResult: topologyMappingResult,
		OfflineTopology:       offlineTopology,
		WarningItemNum:        warningItemNum,
		FailedItemNum:         failedItemNum,
	}
}

func createMappingDeviceResultItem(
	offlineDev mappingDevice,
	onlineDev mappingDevice,
	builtInPower bool,
	status string,
	errorMsg string,
) domain.MappingDeviceResultItem {
	// jsonBytes, _ := json.MarshalIndent(onlineDev, "", "  ")
	// logger.Info("createMappingDeviceResultItem() onlineDev:\n" + string(jsonBytes))

	// Create Simple Ethernet module
	onlineEthernetModule := make(map[int64]domain.SimpleEthernetModule)
	for _, eth := range onlineDev.ModularInfo.Ethernet {
		if eth == nil {
			continue
		}
		slotId := eth.SlotID

		// Not in modular configuration (Chamberlain not support module)
		if _, ok := onlineDev.ModularConfiguration.Ethernet[strconv.Itoa(slotId)]; !ok {
			logger.Warnf("Unsupported line module found (module: %s, slot: %d)", eth.ModuleName, slotId)
		}

		onlineEthernetModule[int64(slotId)] = domain.SimpleEthernetModule{
			ModuleName:   eth.ModuleName,
			SerialNumber: eth.SerialNumber,
		}

	}

	// Create Simple Power module
	onlinePowerModule := make(map[int64]domain.SimplePowerModule)
	for _, pow := range onlineDev.ModularInfo.Power {
		if pow == nil {
			continue
		}

		slotId := pow.SlotID

		// Not in modular configuration (Chamberlain not support module)
		if _, ok := onlineDev.ModularConfiguration.Power[strconv.Itoa(slotId)]; !ok {
			logger.Warnf("Unsupported power module found (module: %s, slot: %d)", pow.ModuleName, slotId)
		}

		onlinePowerModule[int64(slotId)] = domain.SimplePowerModule{
			ModuleName:   pow.ModuleName,
			SerialNumber: pow.SerialNumber,
		}

	}
	return domain.MappingDeviceResultItem{
		Id: offlineDev.Id,

		OfflineDeviceId:  offlineDev.Id,
		OfflineIpAddress: offlineDev.IpAddress,
		OfflineModelName: offlineDev.ModelName,

		OnlineDeviceId:     onlineDev.Id,
		OnlineIpAddress:    onlineDev.IpAddress,
		OnlineModelName:    onlineDev.ModelName,
		OnlineMacAddress:   onlineDev.MacAddress,
		OnlineSerialNumber: onlineDev.SerialNumber,

		OnlineEthernetModule: onlineEthernetModule,
		OnlinePowerModule:    onlinePowerModule,

		Status:       status,
		ErrorMessage: errorMsg,
	}
}

func generateLeaveInterfaceOppositeDeviceMap(
	checkMoxaVendor bool,
	deviceId int64,
	deviceLinks []mappingLink,
	moxaVendorDeviceIds map[int64]struct{},
) map[int64]int64 {

	result := make(map[int64]int64)

	for _, neighborLink := range deviceLinks {

		dstDevId := int64(neighborLink.DestinationDeviceId)
		srcDevId := int64(neighborLink.SourceDeviceId)
		dstIfId := int64(neighborLink.DestinationInterfaceId)
		srcIfId := int64(neighborLink.SourceInterfaceId)

		// Case 1: this device is the destination side
		if dstDevId == deviceId {
			if checkMoxaVendor {
				// Must be MOXA vendor
				if _, isMoxa := moxaVendorDeviceIds[srcDevId]; isMoxa {
					result[dstIfId] = srcDevId
				}
			} else {
				result[dstIfId] = srcDevId
			}
		}

		// Case 2: this device is the source side
		if srcDevId == deviceId {
			if checkMoxaVendor {
				if _, isMoxa := moxaVendorDeviceIds[dstDevId]; isMoxa {
					result[srcIfId] = dstDevId
				}
			} else {
				result[srcIfId] = dstDevId
			}
		}
	}

	return result
}

func linkExists(links []mappingLink, id int64) bool {
	for _, l := range links {
		if l.Id == id {
			return true
		}
	}
	return false
}

func generateDeviceLinksMap(mappingTopology mappingTopology) map[int64][]mappingLink {
	result := make(map[int64][]mappingLink)

	for _, link := range mappingTopology.Links {
		// Destination device
		dstId := int64(link.DestinationDeviceId)

		// Initialize if not exists
		if _, exists := result[dstId]; !exists {
			result[dstId] = []mappingLink{}
		}
		// Prevent duplicates by Link.Id
		if !linkExists(result[dstId], link.Id) {
			result[dstId] = append(result[dstId], link)
		}

		// Source device
		srcId := int64(link.SourceDeviceId)

		// Initialize if not exists
		if _, exists := result[srcId]; !exists {
			result[srcId] = []mappingLink{}
		}
		// Prevent duplicates by Link.Id
		if !linkExists(result[srcId], link.Id) {
			result[srcId] = append(result[srcId], link)
		}
	}

	return result
}

func generateOfflineMappingDeviceMap(mappingTopology mappingTopology) map[int64]struct{} {
	result := make(map[int64]struct{})

	for _, dev := range mappingTopology.Devices {
		// [bugfix:2854] Topology mapping - Moxa end station should be skipped
		// Skip end-station devices
		if dev.DeviceType == domain.DeviceTypeEndStation {
			continue
		}
		vendor := dev.Vendor

		// Moxa devices only
		if vendor == moxaVendorId || vendor == moxaVendorString {
			result[dev.Id] = struct{}{}
		}
	}
	return result
}

func findDeviceById(devices []mappingDevice, deviceId int64) (mappingDevice, error) {
	for _, device := range devices {
		if device.Id == deviceId {
			return device, nil
		}
	}
	err := fmt.Errorf("The device with ID %d not found", deviceId)
	logger.Error(err)
	return mappingDevice{}, err
}

func getProjectDeviceById(project domain.Project, deviceId int64) (domain.Device, error) {
	for _, device := range project.Devices {
		if device.Id == deviceId {
			return device, nil
		}
	}
	err := fmt.Errorf("The device with ID %d not found", deviceId)
	logger.Error(err)
	return domain.Device{}, err
}

func getProjectDeviceByIp(project domain.Project, ip string) (domain.Device, error) {
	for _, device := range project.Devices {
		if device.Ipv4.IpAddress == ip {
			return device, nil
		}
	}
	err := fmt.Errorf("The device with IP(%s) not found", ip)
	logger.Error(err)
	return domain.Device{}, err
}

func mustIPv4ToU32(s string) uint32 {
	ip := net.ParseIP(strings.TrimSpace(s))
	if ip == nil {
		return 0
	}
	ip4 := ip.To4()
	if ip4 == nil {
		return 0
	}
	return binary.BigEndian.Uint32(ip4)
}

func normalizeMAC(mac string) string {
	mac = strings.ReplaceAll(mac, "-", "")
	mac = strings.ReplaceAll(mac, ":", "")
	return strings.ToLower(mac)
}

func transferMACToDashesFormat(mac string) (string, error) {
	newMacAddress := ""
	if mac != "" {
		parsedMacAddress, err := macutility.ParseMACAddress(mac)
		if err == nil {
			newMacAddress = parsedMacAddress.Dashes(macutility.UpperCase)
		} else {
			return newMacAddress, err

		}
	}
	return newMacAddress, nil
}

func bytesCompare(a, b net.IP) int {
	a4 := a.To4()
	b4 := b.To4()
	if a4 == nil || b4 == nil {
		// fallback: comapre original 16 bytes, avoid nil panic
		return bytes.Compare(a, b)
	}
	return bytes.Compare(a4, b4)
}

// func updateMAFDeviceSecretByProjectSetting(mafBaseDev dmschema.DeviceBase, projectSetting domain.Project.ProjectSetting) error {
// 	dmManager, err := dipool.GetDMManager()
// 	if err != nil {
// 		return err
// 	}

// 	mafSNMPVersion, canParseSNMPVersion := configmapper.MapActSNMPVersionToMafSNMPVersion(projectSetting.SnmpConfiguration.Version)
// 	if !canParseSNMPVersion {
// 		err := fmt.Errorf("invalid SNMP version in ProjectSetting's SNMP configuration: %v", projectSetting.SnmpConfiguration.Version)
// 		logger.Error(err)
// 		return err
// 	}

// 	mafSNMPAuthType, canParseAuthType := configmapper.MapActSNMPAuthenticationTypeToMafSNMPAuthenticationType(projprojectSettingectDev.SnmpConfiguration.AuthenticationType)
// 	if !canParseAuthType {
// 		err := fmt.Errorf("invalid SNMP auth type in ProjectSetting's configuration: %v", projectSetting.SnmpConfiguration.AuthenticationType)
// 		logger.Error(err)
// 		return err
// 	}

// 	mafEncType, canParseEncType := configmapper.MapActSNMPEncryptionTypeToMafSNMPEncryptionType(projectSetting.SnmpConfiguration.DataEncryptionType)
// 	if !canParseEncType {
// 		err := fmt.Errorf("invalid SNMP encryption type in ProjectSetting's configuration: %v", projectSetting.SnmpConfiguration.DataEncryptionType)
// 		logger.Error(err)
// 		return err
// 	}

// 	restfulPort := uint16(projectSetting.RestfulConfiguration.Port)
// 	username := projectSetting.Account.Username
// 	password := projectSetting.Account.Password
// 	err = dmManager.UpdateDeviceSecret(mafBaseDev.DeviceId, schema.DeviceSecrets{
// 		GlobalUsername: username,
// 		GlobalPassword: password,
// 		Snmp: &schema.SNMP{
// 			Version:         mafSNMPVersion,
// 			ReadCommunity:   projectSetting.SnmpConfiguration.ReadCommunity,
// 			WriteCommunity:  projectSetting.SnmpConfiguration.WriteCommunity,
// 			Username:        projectSetting.SnmpConfiguration.Username,
// 			AuthType:        mafSNMPAuthType,
// 			AuthPassword:    projectSetting.SnmpConfiguration.DataEncryptionKey,
// 			DataEncryptType: mafEncType,
// 			DataEncryptKey:  projectSetting.SnmpConfiguration.DataEncryptionKey,
// 			Port:            projectSetting.SnmpConfiguration.Port,
// 			// TransportProtocol: "TCP",
// 		},
// 		Http: &schema.HTTP{
// 			Port:     &restfulPort,
// 			Username: &username,
// 			Password: &password,
// 		},
// 		Https: &schema.HTTPS{
// 			Port:     &restfulPort,
// 			Username: &username,
// 			Password: &password,
// 		},
// 		MoxaCmd: &schema.MOXACMD{
// 			Port:     &restfulPort,
// 			Username: &username,
// 			Password: &password,
// 		},
// 	})
// 	if err != nil {
// 		return err
// 	}

// 	return nil
// }

func updateMAFDeviceSecret(mafBaseDev dmschema.DeviceBase, projectDev domain.Device) error {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	mafSNMPVersion, canParseSNMPVersion := configmapper.MapActSNMPVersionToMafSNMPVersion(projectDev.SnmpConfiguration.Version)
	if !canParseSNMPVersion {
		err := fmt.Errorf("Invalid SNMP version in Device(%s) SNMP configuration: %v", projectDev.Ipv4.IpAddress, projectDev.SnmpConfiguration.Version)
		logger.Error(err)
		return err
	}

	mafSNMPAuthType, canParseAuthType := configmapper.MapActSNMPAuthenticationTypeToMafSNMPAuthenticationType(projectDev.SnmpConfiguration.AuthenticationType)
	if !canParseAuthType {
		err := fmt.Errorf("Invalid SNMP auth type in Device(%s) SNMP configuration: %v", projectDev.Ipv4.IpAddress, projectDev.SnmpConfiguration.AuthenticationType)
		logger.Error(err)
		return err
	}

	mafEncType, canParseEncType := configmapper.MapActSNMPEncryptionTypeToMafSNMPEncryptionType(projectDev.SnmpConfiguration.DataEncryptionType)
	if !canParseEncType {
		err := fmt.Errorf("Invalid SNMP encryption type in Device(%s) SNMP configuration: %v", projectDev.Ipv4.IpAddress, projectDev.SnmpConfiguration.DataEncryptionType)
		logger.Error(err)
		return err
	}

	restfulPort := uint16(projectDev.RestfulConfiguration.Port)
	username := projectDev.Account.Username
	password := projectDev.Account.Password
	err = dmManager.UpdateDeviceSecret(mafBaseDev.DeviceId, schema.DeviceSecrets{
		GlobalUsername: username,
		GlobalPassword: password,
		Snmp: &schema.SNMP{
			Version:         mafSNMPVersion,
			ReadCommunity:   projectDev.SnmpConfiguration.ReadCommunity,
			WriteCommunity:  projectDev.SnmpConfiguration.WriteCommunity,
			Username:        projectDev.SnmpConfiguration.Username,
			AuthType:        mafSNMPAuthType,
			AuthPassword:    projectDev.SnmpConfiguration.DataEncryptionKey,
			DataEncryptType: mafEncType,
			DataEncryptKey:  projectDev.SnmpConfiguration.DataEncryptionKey,
			Port:            projectDev.SnmpConfiguration.Port,
			// TransportProtocol: "TCP",
		},
		Http: &schema.HTTP{
			Port:     &restfulPort,
			Username: &username,
			Password: &password,
		},
		Https: &schema.HTTPS{
			Port:     &restfulPort,
			Username: &username,
			Password: &password,
		},
		MoxaCmd: &schema.MOXACMD{
			Port:     &restfulPort,
			Username: &username,
			Password: &password,
		},
	})
	if err != nil {
		return err
	}

	return nil
}
