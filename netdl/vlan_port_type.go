package netdl

type VlanPortType int

const (
	VlanPortTypeUnknown VlanPortType = iota
	VlanPortTypeAccess
	VlanPortTypeTrunk
	VlanPortTypeHybrid
)
