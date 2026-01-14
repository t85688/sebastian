package dto

type MonitorStartRequest struct {
	ProjectId int64 `json:"ProjectId"`
}

type MonitorStopRequest struct {
	ProjectId int64 `json:"ProjectId"`
}

type MonitorSFPPort struct {
	DeviceId      int64  `json:"DeviceId"`
	DeviceIp      string `json:"DeviceIp"`
	InterfaceId   int    `json:"InterfaceId"`
	InterfaceName string `json:"InterfaceName"`
	ModelName     string `json:"ModelName"`
	RxPower       string `json:"RxPower"`
	TxPower       string `json:"TxPower"`
	TemperatureC  string `json:"TemperatureC"`
	TemperatureF  string `json:"TemperatureF"`
	Voltage       string `json:"Voltage"`
}

type MonitorSFPLink struct {
	Source MonitorSFPPort `json:"Source"`
	Target MonitorSFPPort `json:"Target"`
}

type MonitorSFPListResponse struct {
	SFPList []*MonitorSFPLink `json:"SFPList"`
}
