package deploy

import (
	"bytes"
	"fmt"
	"net"
	"strings"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/manager/job/model"
)

func getNewDeployDeviceIdsByMappingMap(deviceMappingMap map[int64]string) []int64 {
	var newDeviceIds []int64
	for deviceId, _ := range deviceMappingMap {
		newDeviceIds = append(newDeviceIds, deviceId)
	}
	return newDeviceIds
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

func updateProjectDevice(project *domain.Project, device domain.Device) error {
	for i := range project.Devices {
		if project.Devices[i].Id == device.Id {
			project.Devices[i] = device
			return nil
		}
	}
	err := fmt.Errorf("The device with ID %d not found", device.Id)
	logger.Error(err)
	return err
}

func normalizeMAC(mac string) string {
	mac = strings.ReplaceAll(mac, "-", "")
	mac = strings.ReplaceAll(mac, ":", "")
	return strings.ToLower(mac)
}

func layerMergeFarToNear(sortResult model.SortResult) ([]string, error) {
	// Merges multiple far-to-near paths into a single path.
	if len(sortResult.Paths) == 0 {
		return nil, nil
	}

	// // Check common origin. MAF already skip the same node
	// origin := sortResult.Paths[0].Devices[len(sortResult.Paths[0].Devices)-1]
	// for i, p := range sortResult.Paths {
	// 	if len(p.Devices) == 0 {
	// 		return nil, fmt.Errorf("path %d is empty", i)
	// 	}
	// 	if p.Devices[len(p.Devices)-1] != origin {
	// 		return nil, fmt.Errorf("all paths must end at the same origin (%s), but path %d ends at %s",
	// 			origin, i, p.Devices[len(p.Devices)-1])
	// 	}
	// }

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

func bytesCompare(a, b net.IP) int {
	a4 := a.To4()
	b4 := b.To4()
	if a4 == nil || b4 == nil {
		// fallback: comapre original 16 bytes, avoid nil panic
		return bytes.Compare(a, b)
	}
	return bytes.Compare(a4, b4)
}

func getDeviceDefaultAccount(profiles_with_default_device_config *domain.DeviceProfilesWithDefaultDeviceConfig, device domain.Device) (domain.UserAccount, error) {
	system_default_account := domain.UserAccount{Username: "admin", Password: "moxa"}
	logger.Info(fmt.Sprintf("DeviceProfileId:%d", device.DeviceProfileId))

	for _, profile := range profiles_with_default_device_config.Profiles {
		if profile.Id == device.DeviceProfileId {
			if len(profile.DefaultDeviceConfig.UserAccountTables) != 0 {
				userAccountTable := profile.DefaultDeviceConfig.UserAccountTables[0]
				if account, ok := userAccountTable.Accounts["admin"]; ok {
					logger.Info("Find admin account")
					return account, nil
				} else {
					// return the system default account
					logger.Info("Not find admin account")
					return system_default_account, nil
				}
			} else {
				// return the system default account
				logger.Info("UserAccountTables is Empty")
				return system_default_account, nil
			}
		}
	}

	err := fmt.Errorf("The DeviceProfileId(%d) not exists at the DeviceProfilesWithDefaultDeviceConfig", device.DeviceProfileId)
	logger.Error(err)
	return system_default_account, err
}
