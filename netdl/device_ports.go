package netdl

type PortSetting struct {
	PortId   int    `json:"portId"`
	PortName string `json:"portName,omitempty"`
	Enable   bool   `json:"enable"`
}

type Port struct {
	ID               int               `json:"id,omitempty"`
	Speed            int               `json:"speed,omitempty"`
	AliasName        string            `json:"aliasName,omitempty"`
	Description      *string           `json:"description,omitempty"`
	Enabled          string            `json:"enabled,omitempty" validate:"oneof=enable disable"`
	MediaType        string            `json:"mediaType,omitempty"`
	PortSFP          *PortSFP          `json:"sfp,omitempty"`
	PortPOE          *PortPOE          `json:"poe,omitempty"`
	PortRedundancy   *PortRedundancy   `json:"redundancy,omitempty"`
	PortTraffic      *PortTraffic      `json:"traffic,omitempty"`
	PortPktErrorRate *PortPktErrorRate `json:"pktErrorRate,omitempty"`
}

type PortSFP struct {
	Model       string   `json:"model,omitempty"`
	RxPower     string   `json:"rxPower,omitempty"`
	RxPowerWarn []string `json:"rxPowerWarn,omitempty"`
	SN          string   `json:"sn,omitempty"`
	TempWarn    []string `json:"tempWarn,omitempty"`
	Temperature string   `json:"temperature,omitempty"`
	TxPower     string   `json:"txPower,omitempty"`
	TxPowerWarn []string `json:"txPowerWarn,omitempty"`
	Voltage     string   `json:"voltage,omitempty"`
}

type PortTraffic struct {
	RecordTime     uint64 `json:"recordTime"`
	OutUtilization uint64 `json:"outUtilization"` // unit is 1/10000 usage, 10000 means 100%
	InUtilization  uint64 `json:"inUtilization"`  // unit is 1/10000 usage, 10000 means 100%
}

type PortPktErrorRate struct {
	RecordTime   uint64 `json:"recordTime"`
	OutErrorRate uint64 `json:"outErrorRate"` // unit is 1/10000 usage, 10000 means 100%
	InErrorRate  uint64 `json:"inErrorRate"`  // unit is 1/10000 usage, 10000 means 100%
}

type PortPOE struct {
	Consumption     string `json:"consumption,omitempty"`
	Voltage         string `json:"voltage,omitempty"`
	Current         string `json:"current,omitempty"`
	PowerOutputMode string `json:"powerOutputMode,omitempty"`
	LegacyPdDetact  string `json:"legacyPdDetact,omitempty"`
	Class           string `json:"class,omitempty"`
}

type PortRedundancy struct {
	Type              string `json:"type,omitempty"`
	PortRole          string `json:"portRole,omitempty"`
	PortStatus        string `json:"portStatus,omitempty"`
	*PortSpanningTree `json:"spanningTree,omitempty"`
}

type PortSpanningTree struct {
	PortId             int    `json:"portId"`
	AliasName          string `json:"aliasName"`
	PortState          int    `json:"portState"`
	PathCost           int64  `json:"pathCost"`
	DesignatedCost     int64  `json:"designatedCost"`
	EdgePort           bool   `json:"edgePort"`
	RstpPortRole       int    `json:"rstpPortRole"`
	BpduInconsistency  bool   `json:"bpduInconsistency"`
	RootInconsistency  bool   `json:"rootInconsistency"`
	LoopInconsistency  bool   `json:"loopInconsistency"`
	OperBridgeLinkType bool   `json:"operBridgeLinkType"`
}
