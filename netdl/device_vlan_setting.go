package netdl

import (
	"encoding/json"
	"fmt"
)

type VlanSetting struct {
	// Management VLAN is only present in the VLAN setting of L2 devices.
	MgmtVlan        *int64          `json:"mgmtVlan,omitempty"`
	VlanEntries     []VlanEntry     `json:"vlanEntries"`
	VlanPortEntries []VlanPortEntry `json:"portVlanEntries"`
}

type VlanEntry struct {
	VlanId      int64  `json:"vlanId"`
	VlanName    string `json:"vlanName"`
	Temstid     *bool  `json:"temstid,omitempty"`
	MemberPorts []int  `json:"memberPort"`
}

// define vlan port type enum
type VlanModeEnum int64

const (
	VlanModeAccess VlanModeEnum = iota + 1
	VlanModeTrunk
	VlanModeHybrid
	VlanModeNone
)

func (vlanModeEnum *VlanModeEnum) UnmarshalJSON(b []byte) error {
	var mode string
	if err := json.Unmarshal(b, &mode); err != nil {
		return err
	}
	switch mode {
	case "access":
		*vlanModeEnum = VlanModeAccess
	case "trunk":
		*vlanModeEnum = VlanModeTrunk
	case "hybrid":
		*vlanModeEnum = VlanModeHybrid
	case "none":
		*vlanModeEnum = VlanModeNone
	default:
		return fmt.Errorf("invalid vlan mode: %s. Should be \"access\", \"trunk\", \"hybrid\" or \"none\"", mode)
	}
	return nil
}

func (vlanModeEnum VlanModeEnum) MarshalJSON() ([]byte, error) {
	var mode string
	switch vlanModeEnum {
	case VlanModeAccess:
		mode = "access"
	case VlanModeTrunk:
		mode = "trunk"
	case VlanModeHybrid:
		mode = "hybrid"
	case VlanModeNone:
		mode = "none"
	default:
		return nil, fmt.Errorf("invalid vlan mode: %d", vlanModeEnum)
	}
	return json.Marshal(mode)
}

type VlanPortEntry struct {
	PortId        int          `json:"portId"`
	AliasName     string       `json:"aliasName"`
	Pvid          int64        `json:"pvid"`
	Mode          VlanModeEnum `json:"mode"`
	UntaggedVlans []int64      `json:"untaggedVlans"`
	TaggedVlans   []int64      `json:"taggedVlans"`
}
