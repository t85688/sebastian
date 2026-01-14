package operations

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/manager/job/model"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/logging"
)

var logger = logging.NewWithField("module", "operations")

func SplitExistingAndMissingDevices(deviceList []domain.Device, mafDeviceIpIdMap map[string]string) ([]domain.Device, []domain.Device) { // return existing & missing devices
	existingDevices := []domain.Device{}
	missinDevices := []domain.Device{}

	for _, device := range deviceList {
		ip := device.Ipv4.IpAddress
		if _, ok := mafDeviceIpIdMap[ip]; ok {
			existingDevices = append(existingDevices, device)
		} else {
			missinDevices = append(missinDevices, device)
		}
	}
	return existingDevices, missinDevices
}

func GetProjectDeviceById(project domain.Project, deviceId int64) (domain.Device, error) {
	for _, device := range project.Devices {
		if device.Id == deviceId {
			return device, nil
		}
	}
	err := fmt.Errorf("The device with ID %d not found", deviceId)
	logger.Error(err)

	return domain.Device{}, err
}

func CreateDeviceList(project domain.Project, deviceIds []int64) ([]domain.Device, error) {
	deviceList := []domain.Device{}

	for _, id := range deviceIds {
		device, err := GetProjectDeviceById(project, id)
		if err != nil {
			logger.Errorf("Device not found at Project. err: %v", err)
			return deviceList, err
		}
		deviceList = append(deviceList, device)

	}

	return deviceList, nil
}

func CreateIpList(deviceList []domain.Device) []string {
	ipList := []string{}

	for _, device := range deviceList {
		ipList = append(ipList, device.Ipv4.IpAddress)
	}

	return ipList
}

func SetMAFDevicesSecret(deviceList []domain.Device, mafDeviceIpIdMap map[string]string) error {
	for _, device := range deviceList {
		ip := device.Ipv4.IpAddress
		if mafDeviceId, ok := mafDeviceIpIdMap[ip]; ok {
			err := common.UpdateMAFDeviceSecret(mafDeviceId, device)
			if err != nil {
				logger.Errorf("UpdateMAFDeviceSecret(). err: %v", err)
				return err
			}
		} else { // not found the key
			logger.Warnf("Device IP(%s) not found at MAF device map.", ip)
			continue
		}
	}

	return nil
}

func GetTargetMAFDevicesId(deviceList []domain.Device, mafDeviceIpIdMap map[string]string) ([]string, error) {
	mafDeviceIdList := []string{}

	for _, device := range deviceList {
		ip := device.Ipv4.IpAddress
		if mafDeviceId, ok := mafDeviceIpIdMap[ip]; ok {
			mafDeviceIdList = append(mafDeviceIdList, mafDeviceId)
		} else { // not found the key
			err := fmt.Errorf("Device IP(%s) not found at MAF device map.", ip)
			logger.Error(err)
			return mafDeviceIdList, err
		}
	}

	return mafDeviceIdList, nil
}

func GetMAFDevicesId(mafDeviceIpIdMap map[string]string) []string {
	mafDeviceIdList := []string{}

	for _, mafDeviceId := range mafDeviceIpIdMap {
		mafDeviceIdList = append(mafDeviceIdList, mafDeviceId)
	}

	return mafDeviceIdList
}

func CreateDeviceIdMap(deviceList []domain.Device, mafDeviceIpIdMap map[string]string) (map[int64]string, error) {
	deviceIdMap := make(map[int64]string) // map<ProjectDeviceID, MAFDeviceID>

	for _, device := range deviceList {
		ip := device.Ipv4.IpAddress
		if mafDeviceId, ok := mafDeviceIpIdMap[ip]; ok {
			deviceIdMap[device.Id] = mafDeviceId
		} else { // not found the key
			err := fmt.Errorf("Device IP(%s) not found at MAF device map.", ip)
			logger.Error(err)
			return deviceIdMap, err
		}
	}

	return deviceIdMap, nil
}

func layerMergeFarToNear(sortResult model.SortResult) ([]string, error) {
	// Merges multiple far-to-near paths into a single path.
	if len(sortResult.Paths) == 0 {
		return nil, nil
	}
	// Find maximum depth
	maxDepth := 0
	for _, p := range sortResult.Paths {
		depth := len(p.Devices) - 1
		if depth > maxDepth {
			maxDepth = depth
		}
	}

	seen := make(map[string]bool)
	var merged []string

	// Traverse from the farthest layer to the origin
	for d := maxDepth; d >= 0; d-- {
		for _, p := range sortResult.Paths {
			if len(p.Devices)-1 >= d {
				idx := len(p.Devices) - 1 - d
				node := p.Devices[idx]
				if !seen[node] {
					seen[node] = true
					merged = append(merged, node)
				}
			}
		}
	}
	return merged, nil
}

func SortDevicesByDistance(deviceIdMap map[int64]string) ([]int64, error) {
	logger.Info("Start SortDevicesByDistance")

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}

	var mafDevIdList []string
	for _, mafId := range deviceIdMap {
		mafDevIdList = append(mafDevIdList, mafId)
	}
	reqDevices := model.SortRequest{
		Devices:   mafDevIdList,
		EnableArp: false,
	}

	// Sort by MAF DM
	sortResult, code, err := dmManager.GetSortDistance(reqDevices)
	if code >= common.MafGenericErrorBase || err != nil {
		return nil, err
	}
	logger.Info(fmt.Sprintf("MAF Sort Result: %v", sortResult))

	mergedMafDevicePath, err := layerMergeFarToNear(sortResult)
	if err != nil {
		return nil, err
	}

	// Reverse lookup from mafDevice to deviceId
	valToKey := make(map[string]int64)
	for k, v := range deviceIdMap {
		valToKey[v] = k
	}
	var resultDeviceIdPath []int64
	for _, node := range mergedMafDevicePath {
		if deviceId, ok := valToKey[node]; ok {
			resultDeviceIdPath = append(resultDeviceIdPath, deviceId)
		}
	}

	return resultDeviceIdPath, err
}

func GetTargetDevicesIdSequence(targetDevices []domain.Device, projectDevicesIdSequence []int64) []int64 {
	targetDevicesIdMap := make(map[int64]struct{}) // Map(TargetDeviceId, null)
	for _, device := range targetDevices {
		targetDevicesIdMap[device.Id] = struct{}{}
	}

	targetDevicesIdSequence := []int64{}
	for _, id := range projectDevicesIdSequence {
		if _, ok := targetDevicesIdMap[id]; ok {
			targetDevicesIdSequence = append(targetDevicesIdSequence, id)
		}
	}
	return targetDevicesIdSequence
}

// func  SortDevicesByIp(deviceList []domain.Device) (deviceList domain.Device) {

// 	type kv struct {
// 		deviceId int64
// 		ip       net.IP
// 	}
// 	var pairs []kv

// 	for _, device := range deviceList {

// 		pairs = append(pairs, kv{deviceId, net.ParseIP(device.Ipv4.IpAddress)})
// 	}

// 	// Sort slice by IP (convert string to net.IP for correct numeric comparison)
// 	sort.Slice(pairs, func(i, j int) bool {
// 		return bytesCompare(pairs[i].ip, pairs[j].ip) < 0
// 	})

// 	// Extract sorted deviceIds into a list
// 	var sorted []int64
// 	for _, p := range pairs {
// 		sorted = append(sorted, p.deviceId)
// 	}

// 	return sorted, nil
// }

// func EnableMAFDevicesSNMP(mafDeviceIdList []string) error {
// 	mafJobWorker := common.NewMAFJobWorker()
// 	err := mafJobWorker.EnableMAFDevicesSNMPByJob(mafDeviceIdList)
// 	if err != nil {
// 		logger.Error("Error EnableMAFDevicesSNMPByJob():", err)
// 		return err
// 	}

// 	return nil
// }

// func (locator *Locator) TriggerMAFDeviceLocator(mafDeviceIdList []string, duration uint32) error {
// 	mafJobWorker := common.NewMAFJobWorker()
// 	err := mafJobWorker.TriggerMAFDeviceLocatorByJob(mafDeviceIdList, int(duration))
// 	if err != nil {
// 		logger.Error("Error TriggerMAFDeviceLocatorByJob():", err)
// 		return err
// 	}

// 	return nil
// }
