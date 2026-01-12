package netdl

// Redundancy Protocols String
const (
	RedundancyProtocolDualHoming     = "dualHoming"
	RedundancyProtocolTurboChain     = "turboChain"
	RedundancyProtocolTurboRing      = "turboRing"
	RedundancyProtocolTurboRingV2    = "turboRingV2"
	RedundancyProtocolMRP            = "mrp"
	RedundancyProtocolSTP            = "stp"
	RedundancyProtocolRSTP           = "rstp"
	RedundancyProtocolSTPRSTP        = "stprstp"
	RedundancyProtocolMSTP           = "mstp"
	RedundancyProtocolPRP            = "prp"
	RedundancyProtocolHSR            = "hsr"
	RedundancyProtocolPRPHSRCoupling = "prpHsrCoupling"
)

const (
	// Spanning Tree Protocol (STP) versions
	SpanningTreeVersionSTP  = "stp"
	SpanningTreeVersionRSTP = "rstp"
	SpanningTreeVersionMSTP = "mstp"
)

type RedundancyStatus struct {
	SpanningTree *RedundancySpanningTreeStatus `json:"spanningTree,omitempty"`

	*RedundancyDualHoming  `json:"dualHoming,omitempty"`
	*RedundancyTurboChain  `json:"turboChain,omitempty"`
	*RedundancyTurboRing   `json:"turboRing,omitempty"`
	*RedundancyTurboRingV2 `json:"turboRingV2,omitempty"`
	*RedundancyMRP         `json:"mbr,omitempty"`
	*RedundancyPRP         `json:"prp,omitempty"`
	*RedundancyHSR         `json:"hsr,omitempty"`
}

type RedundancySpanningTreeCommon struct {
	Enable           bool   `json:"enable"`
	StpMode          string `json:"stpMode" validate:"oneof=stpRstp notSupport"`
	Compatibility    string `json:"compatibility" validate:"oneof=stp rstp notSupport"`
	ForwardDelayTime int64  `json:"forwardDelayTime"`
	HelloTime        int64  `json:"helloTime"`
	MaxAge           int64  `json:"maxAge"`
}

type RedundancySpanningTreeRootInformation struct {
	ForwardDelayTime int64  `json:"forwardDelayTime"`
	HelloTime        int64  `json:"helloTime"`
	MaxAge           int64  `json:"maxAge"`
	BridgeId         string `json:"rootBridgeId,omitempty"`
	RootPathCost     int64  `json:"rootPathCost"`
}

type RedundancySpanningTreeStatus struct {
	Enable bool                                  `json:"enable"`
	Root   RedundancySpanningTreeRootInformation `json:"root"`
	Role   string                                `json:"role,omitempty"`
}

type RedundancySpanningTreeStatusComplete struct {
	Root   RedundancySpanningTreeRootInformation    `json:"root"`
	Role   string                                   `json:"role,omitempty"`
	Enable bool                                     `json:"enable"`
	Ports  []*RedundancySpanningTreePortStatusEntry `json:"ports,omitempty"`
}

type RedundancySpanningTreePortStatusEntry struct {
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

type RedundancyDualHoming struct {
	Enable bool `json:"enable"`
}
type RedundancyTurboChain struct {
	Enable bool   `json:"enable"`
	Role   string `json:"role"`
}
type RedundancyTurboRing struct {
	Enable bool   `json:"enable"`
	Role   string `json:"role"`
}
type RedundancyTurboRingV2 struct {
	Enable bool   `json:"enable"`
	Role   string `json:"role"`
}
type RedundancyMRP struct {
	Enable bool   `json:"enable"`
	Role   string `json:"role"`
}
type RedundancyPRP struct {
	Enable bool `json:"enable"`
}

type RedundancyHSR struct {
	Enable bool `json:"enable"`
}
