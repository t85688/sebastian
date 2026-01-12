package domain

type EthernetModules struct {
	EthernetModuleMap map[int64]EthernetModule `json:"EthernetModuleMap"`
}

type EthernetModuleInterface struct {
	InterfaceId   int64    `json:"InterfaceId"`
	InterfaceName string   `json:"InterfaceName"`
	SupportSpeeds []int64  `json:"SupportSpeeds"`
	CableTypes    []string `json:"CableTypes"`
}

type EthernetModule struct {
	Id                 int64                     `json:"Id"`
	DataVersion        string                    `json:"DataVersion"`
	ModuleName         string                    `json:"ModuleName"`
	ModuleType         string                    `json:"ModuleType"`
	Description        string                    `json:"Description"`
	Profiles           []string                  `json:"Profiles"`
	NumberOfInterfaces int                       `json:"NumberOfInterfaces"`
	Interfaces         []EthernetModuleInterface `json:"Interfaces"`
}
