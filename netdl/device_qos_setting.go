package netdl

type PcpEntry struct {
	PortID            int   `json:"portId"`
	PriorityCodePoint uint8 `json:"priorityCodePoint"`
}

// PCP = priority code point
type PcpSetting struct {
	PriorityCodePointSetting []PcpEntry `json:"priorityCodePointSetting"`
}
