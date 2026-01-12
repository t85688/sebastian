package domain

type PowerDeviceProfiles struct {
	Profiles []PowerDeviceProfile `json:"PowerDeviceProfileSet"`
}

type PowerDeviceProfile struct {
	Id                    int64                `json:"Id"`
	IconName              string               `json:"IconName"`
	DataVersion           string               `json:"DataVersion"`
	Purchasable           bool                 `json:"Purchasable"`
	Profiles              []string             `json:"Profiles"`
	ModelName             string               `json:"ModelName"`
	MountType             MountTypeList        `json:"MountType"`
	Description           string               `json:"Description"`
	OperatingTemperatureC OperatingTemperature `json:"OperatingTemperatureC"`

	BuiltIn     bool       `json:"BuiltIn"`
	Hide        bool       `json:"Hide"`
	Certificate bool       `json:"Certificate"`
	Vendor      string     `json:"Vendor"`
	DeviceType  DeviceType `json:"DeviceType"`

	MaxPortSpeed int64              `json:"MaxPortSpeed"`
	Interfaces   []ProfileInterface `json:"Interfaces"`
}
