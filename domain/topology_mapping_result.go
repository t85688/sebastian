package domain

type SimpleEthernetModule struct {
	ModuleName   string `json:"ModuleName"`
	SerialNumber string `json:"SerialNumber"`
}

type SimplePowerModule struct {
	ModuleName   string `json:"ModuleName"`
	SerialNumber string `json:"SerialNumber"`
}

type MappingDeviceResultItem struct {
	// Unique ID
	Id int64 `json:"Id"`

	// Offline device information
	OfflineDeviceId  int64  `json:"OfflineDeviceId"`
	OfflineIpAddress string `json:"OfflineIpAddress"`
	OfflineModelName string `json:"OfflineModelName"`

	// Online device information
	OnlineDeviceId       int64                          `json:"OnlineDeviceId"`
	OnlineIpAddress      string                         `json:"OnlineIpAddress"`
	OnlineModelName      string                         `json:"OnlineModelName"`
	OnlineMacAddress     string                         `json:"OnlineMacAddress"`
	OnlineSerialNumber   string                         `json:"OnlineSerialNumber"`
	OnlineEthernetModule map[int64]SimpleEthernetModule `json:"OnlineEthernetModule"`
	OnlinePowerModule    map[int64]SimplePowerModule    `json:"OnlinePowerModule"`

	// Result information
	Status       string `json:"Status"`
	ErrorMessage string `json:"ErrorMessage"`
}

type TopologyMappingResult struct {
	MappingReport []MappingDeviceResultItem `json:"MappingReport"`
	Deploy        bool                      `json:"Deploy"`
}
