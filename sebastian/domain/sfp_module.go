package domain

type SFPModules struct {
	SFPModuleMap map[int64]SFPModule `json:"SFPModuleMap"`
}

type WavelengthNM struct {
	RX int64 `json:"RX"`
	TX int64 `json:"TX"`
}

type SFPModule struct {
	ConnectorType          string               `json:"ConnectorType"`
	DataVersion            string               `json:"DataVersion"`
	Description            string               `json:"Description"`
	IconName               string               `json:"IconName"`
	Id                     int64                `json:"Id"`
	Mode                   string               `json:"Mode"`
	ModuleName             string               `json:"ModuleName"`
	ModuleType             string               `json:"ModuleType"`
	OperatingTemperatureC  OperatingTemperature `json:"OperatingTemperatureC"`
	Profiles               []string             `json:"Profiles"`
	Purchasable            bool                 `json:"Purchasable"`
	Speed                  int64                `json:"Speed"`
	TransmissionDistanceKM int64                `json:"TransmissionDistanceKM"`
	WDM                    bool                 `json:"WDM"`
	WavelengthNM           WavelengthNM         `json:"WavelengthNM"`
}
