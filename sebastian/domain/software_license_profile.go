package domain

type SoftwareLicenseProfiles struct {
	Profiles []SoftwareLicenseProfile `json:"SoftwareLicenseProfileSet"`
}

type SoftwareLicenseProfile struct {
	Id          int64    `json:"Id"`
	IconName    string   `json:"IconName"`
	DataVersion string   `json:"DataVersion"`
	Purchasable bool     `json:"Purchasable"`
	Profiles    []string `json:"Profiles"`
	ModelName   string   `json:"ModelName"`
	Description string   `json:"Description"`

	BuiltIn     bool       `json:"BuiltIn"`
	Hide        bool       `json:"Hide"`
	Certificate bool       `json:"Certificate"`
	Vendor      string     `json:"Vendor"`
	DeviceType  DeviceType `json:"DeviceType"`
}
