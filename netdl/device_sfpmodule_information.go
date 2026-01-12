package netdl

type SFPModuleInfo struct {
	TxPower     string `json:"tx,omitempty"`
	RxPower     string `json:"rx,omitempty"`
	Temperature string `json:"temp,omitempty"`
	Volt        string `json:"volt,omitempty"`
	Port        int    `json:"port,omitempty"`
	ModuleName  string `json:"name,omitempty"`
}
