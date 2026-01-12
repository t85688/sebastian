package netdl

import (
	"encoding/json"
	"fmt"
)

type PerStreamPrioritySetting struct {
	PerStreamPrioritySetting []PerStreamPriorityEntry `json:"perStreamPrioritySetting"`
}

// PSP = per-stream priority
type PSPTypeEnum string

const (
	PSPTypeL2       = "L2"
	PSPTypeL3Tcp    = "L3TCP"
	PSPTypeL3Udp    = "L3UDP"
	PSPTypeInactive = "inactive"
)

func (typeEnum *PSPTypeEnum) UnmarshalJSON(b []byte) error {
	var typeName string
	if err := json.Unmarshal(b, &typeName); err != nil {
		return err
	}
	switch typeName {
	case "L2":
		*typeEnum = PSPTypeL2
	case "L3TCP":
		*typeEnum = PSPTypeL3Tcp
	case "L3UDP":
		*typeEnum = PSPTypeL3Udp
	case "inactive":
		*typeEnum = PSPTypeInactive
	default:
		return fmt.Errorf("invalid per-stream priority type: %s. Should be \"L2\", \"L3TCP\", \"L3UDP\" or \"inactive\"", typeName)
	}
	return nil
}

func (typeEnum PSPTypeEnum) MarshalJSON() ([]byte, error) {
	var typeName string
	switch typeEnum {
	case PSPTypeL2:
		typeName = "L2"
	case PSPTypeL3Tcp:
		typeName = "L3TCP"
	case PSPTypeL3Udp:
		typeName = "L3UDP"
	case PSPTypeInactive:
		typeName = "inactive"
	default:
		return nil, fmt.Errorf("invalid per-stream priority type: %s", typeEnum)
	}
	return json.Marshal(typeName)
}

type PerStreamPriorityEntry struct {
	Index  int `json:"index"`
	PortId int `json:"portId"`

	Type      PSPTypeEnum `json:"type"`                // "L2", "L3TCP", "L3UDP", "disabled"
	EtherType *string     `json:"etherType,omitempty"` //
	SubType   *string     `json:"subType,omitempty"`
	TCPPort   *int        `json:"tcpPort,omitempty"`
	UDPPort   *int        `json:"udpPort,omitempty"`

	VlanId            int `json:"vlanId"`
	PriorityCodePoint int `json:"priorityCodePoint"`
}
