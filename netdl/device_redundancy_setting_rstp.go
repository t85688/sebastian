package netdl

const (
	// Spanning Tree Port Roles
	EdgeAuto = "auto"
	EdgeYes  = "yes"
	EdgeNo   = "no"

	// Spanning Tree Port Link Types
	LinkTypePointToPoint = "pointToPoint"
	LinkTypeShared       = "shared"
	LinkTypeAuto         = "auto"
)

type RedundancySpanningTreeSetting struct {
	RedundancySpanningTreeCommon

	BridgePriority     int64                     `json:"bridgePriority"`
	ErrorRecoveryTime  *int64                    `json:"errorRecoveryTime,omitempty"`
	Swift              *bool                     `json:"swift,omitempty"`
	Revert             *bool                     `json:"revert,omitempty"`
	StpRstpPortEntries []SpanningTreePortSetting `json:"rstpPortEntries"`
}

type SpanningTreePortSetting struct {
	PortId    int    `json:"portId"`
	AliasName string `json:"aliasName"`
	Enable    *bool  `json:"enable,omitempty"`
	Edge      string `json:"edge" validate:"oneof=auto yes no"`
	Priority  int64  `json:"priority"`
	PathCost  int64  `json:"pathCost"`
	LinkType  string `json:"linkType" validate:"oneof=pointToPoint shared auto"`

	// guard & filter
	BpduFilter *bool `json:"bpduFilter,omitempty"`
	BpduGuard  *bool `json:"bpduGuard,omitempty"`
	RootGuard  *bool `json:"rootGuard,omitempty"`
	LoopGuard  *bool `json:"loopGuard,omitempty"`
}
