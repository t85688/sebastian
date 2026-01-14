package common

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	devicemanager "gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/manager"
	dmschema "gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
)

const (
	MafGenericErrorBase = 100000000
	dmSearchTimeout     = 5 // 5 seconds
)

func ClearMAFCache() error {
	netCache, err := dipool.GetNetCache()
	if err != nil {
		return err
	}

	netCache.DeleteAllDevices()
	netCache.DeleteAllLinks()

	return nil
}

func MafIpListSearch(ipList []string) (map[string]string, error) {
	deviceIpIdMap := make(map[string]string) // Map(IP(string), MafDeviceId(string))

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return deviceIpIdMap, err
	}

	// IpListDiscovery
	productLines := []string{"switch"}
	_, err = dmManager.Search(dmschema.DeviceSearchInput{
		Method:      devicemanager.IpListDiscovery,
		Timeout:     dmSearchTimeout,
		ProductLine: &productLines,
		IPListOption: dmschema.IPListOption{
			IPList: &ipList,
		},
	})

	// Generate the deviceIpIdMap
	mafDevices, err := dmManager.GetDevices(dmschema.GetDeviceListInput{})
	if err != nil {
		Logger.Error("Error getting devices:", err)
		return deviceIpIdMap, err
	}

	Logger.Info("Device List:")
	for _, mafDevice := range mafDevices {
		deviceInfo := fmt.Sprintf("ID: %v, IP: %v, ModelName: %v, SerialNumber: %v, FirmwareVersion: %v, Location: %v, MAC: %v", mafDevice.DeviceId, mafDevice.IP, mafDevice.ModelName, mafDevice.SerialNumber, mafDevice.FirmwareVersion, mafDevice.Location, mafDevice.MAC)
		Logger.Info(deviceInfo)
		deviceIpIdMap[mafDevice.IP] = mafDevice.DeviceId
	}

	return deviceIpIdMap, err
}

func UpdateMAFDeviceSecret(mafDeviceId string, projectDev domain.Device) error {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	err = dmManager.UpdateDeviceSecret(mafDeviceId, projectDev.DeviceSecrets)
	if err != nil {
		return err
	}

	return nil
}
