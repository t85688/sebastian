package netdl

type TimeAwareShaperSetting struct {
	TimeAwareShaperSetting []PortTimeAwareShaper `json:"timeAwareShaperSetting"`
}

type PortTimeAwareShaper struct {
	PortID          int                `json:"portId"`
	AliasName       string             `json:"aliasName"`
	Enable          bool               `json:"enable"`
	GateControlList []GateControlEntry `json:"gateControlList"`
}

type GateControlEntry struct {
	Slot       int     `json:"slot"`
	Interval   float64 `json:"interval"`
	OpenQueues []int   `json:"openQueues"`
}
