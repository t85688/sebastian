package netdl

type LinkupDelaySetting struct {
	Enable               bool                   `json:"enable"`
	LinkupDelayPortTable []LinkupDelayPortEntry `json:"linkupDelayPortTable"`
}

type LinkupDelayPortEntry struct {
	PortId     int    `json:"portId"`
	PortName   string `json:"portName"`
	Enable     bool   `json:"enable"`
	DelayTimer int    `json:"delayTimer"` // 1 - 1000 seconds
}
