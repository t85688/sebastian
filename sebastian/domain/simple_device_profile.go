package domain

type SimpleDeviceProfiles struct {
	Profiles []SimpleDeviceProfile `json:"SimpleDeviceProfileSet"`
}

type SimpleDeviceProfile struct {
	Id          int64    `json:"Id"`
	IconName    string   `json:"IconName"`
	Purchasable bool     `json:"Purchasable"`
	Profiles    []string `json:"Profiles"`

	// Identify
	L2Family                string         `json:"L2Family"`
	L3Series                string         `json:"L3Series"`
	L4Series                string         `json:"L4Series"`
	Category                DeviceCategory `json:"Category"`
	MountType               MountTypeList  `json:"MountType"`
	ModelName               string         `json:"ModelName"`
	PhysicalModelName       string         `json:"PhysicalModelName"`
	LatestFirmwareVersion   string         `json:"LatestFirmwareVersion"`
	SupportFirmwareVersions []string       `json:"SupportFirmwareVersions"`

	Description           string               `json:"Description"`
	OperatingTemperatureC OperatingTemperature `json:"OperatingTemperatureC"`
	DeviceCluster         string               `json:"DeviceCluster"`

	BuiltInPower        bool             `json:"BuiltInPower"`
	SupportPowerModules map[int64]string `json:"SupportPowerModules"`
	SupportPowerSlots   uint8            `json:"SupportPowerSlots"`

	SupportEthernetModules           map[int64]string `json:"SupportEthernetModules"`
	SupportEthernetSlots             uint8            `json:"SupportEthernetSlots"`
	DefaultEthernetSlotOccupiedIntfs uint8            `json:"DefaultEthernetSlotOccupiedIntfs"`

	DeviceType DeviceType `json:"DeviceType"`

	StandardsAndCertifications []string           `json:"StandardsAndCertifications"`
	SupportedInterfaces        []string           `json:"SupportedInterfaces"`
	MaxPortSpeed               int64              `json:"MaxPortSpeed"`
	Interfaces                 []ProfileInterface `json:"Interfaces"`
	Vendor                     string             `json:"Vendor"`
	DeviceName                 string             `json:"DeviceName"`

	// Others Infos
	BuiltIn      bool    `json:"BuiltIn"`
	Hide         bool    `json:"Hide"`
	Certificate  bool    `json:"Certificate"`
	ReservedVlan []int32 `json:"ReservedVlan"`

	// FeatureGroup
	FeatureGroup FeatureGroup `json:"FeatureGroup"`
}

type OperatingTemperature struct {
	Max int `json:"Max"`
	Min int `json:"Min"`
}

type ProfileInterface struct {
	CableTypes    []string `json:"CableTypes"`
	InterfaceId   int      `json:"InterfaceId"`
	InterfaceName string   `json:"InterfaceName"`
	SupportSpeeds []int    `json:"SupportSpeeds"`
}

type DeviceProfilesWithDefaultDeviceConfig struct {
	Profiles []DeviceProfileWithDefaultDeviceConfig `json:"DeviceProfileWithDefaultDeviceConfigSet"`
}
type DeviceProfileWithDefaultDeviceConfig struct {
	Id                  int64        `json:"Id"`
	DefaultDeviceConfig DeviceConfig `json:"DefaultDeviceConfig,omitempty"`
}
