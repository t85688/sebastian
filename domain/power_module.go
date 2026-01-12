package domain

type PowerModules struct {
	PowerModuleMap map[int64]PowerModule `json:"PowerModuleMap"`
}

type OperatingVoltage struct {
	Supported bool    `json:"Supported"`
	Min       float64 `json:"Min"`
	Max       float64 `json:"Max"`
}

type VoltageSpec struct {
	Supported        bool             `json:"Supported"`
	Input            []int32          `json:"Input"`
	OperatingVoltage OperatingVoltage `json:"OperatingVoltage"`
}

type PowerModule struct {
	Id          int64    `json:"Id"`
	DataVersion string   `json:"DataVersion"`
	ModuleName  string   `json:"ModuleName"`
	ModuleType  string   `json:"ModuleType"`
	Description string   `json:"Description"`
	Profiles    []string `json:"Profiles"`

	BuiltIn bool   `json:"BuiltIn"`
	Series  string `json:"Series"`

	RedundantInput bool        `json:"RedundantInput"`
	PoE            bool        `json:"PoE"`
	DyingGasp      bool        `json:"DyingGasp"`
	VDC            VoltageSpec `json:"VDC"`
	VAC            VoltageSpec `json:"VAC"`
}
