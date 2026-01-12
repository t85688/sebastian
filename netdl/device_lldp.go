package netdl

import (
	"encoding/json"
	"fmt"
)

type LldpStatus struct {
	LocalInformation LldpLocalInformation  `json:"localInformation"`
	LocalTimer       *LldpLocalTimer       `json:"localTimer,omitempty"`
	RemoteStatistics *LldpRemoteStatistics `json:"remoteStatistics,omitempty"`
	PortList         []LldpPort            `json:"portList"`
}

// ChassisIDSubtypeEnum represents the source of a chassis Identifier.
type ChassisIDSubtypeEnum string

const (
	ChassisIDSubtypeChassisComponent ChassisIDSubtypeEnum = "chassis-component" // entPhysicalAlias for chassis(3)
	ChassisIDSubtypeInterfaceAlias   ChassisIDSubtypeEnum = "interface-alias"   // ifAlias for an interface
	ChassisIDSubtypePortComponent    ChassisIDSubtypeEnum = "port-component"    // entPhysicalAlias for port/backplane
	ChassisIDSubtypeMacAddress       ChassisIDSubtypeEnum = "mac-address"       // unicast MAC address
	ChassisIDSubtypeNetworkAddress   ChassisIDSubtypeEnum = "network-address"   // network address
	ChassisIDSubtypeInterfaceName    ChassisIDSubtypeEnum = "interface-name"    // ifName for an interface
	ChassisIDSubtypeLocal            ChassisIDSubtypeEnum = "local"             // locally defined value
)

type PortIDSubtypeEnum string

const (
	PortIDSubtypeInterfaceAlias PortIDSubtypeEnum = "interface-alias" // ifAlias for an interface
	PortIDSubtypePortComponent  PortIDSubtypeEnum = "port-component"  // entPhysicalAlias for port/backplane
	PortIDSubtypeMacAddress     PortIDSubtypeEnum = "mac-address"     // unicast MAC address
	PortIDSubtypeNetworkAddress PortIDSubtypeEnum = "network-address" // network address
	PortIDSubtypeInterfaceName  PortIDSubtypeEnum = "interface-name"  // ifName for an interface
	PortIDSubtypeAgentCircuitID PortIDSubtypeEnum = "agent-circuit-id"
	PortIDSubtypeLocal          PortIDSubtypeEnum = "local" // locally defined value
)

// Local device info
type LldpLocalInformation struct {
	ChassisIDSubtype ChassisIDSubtypeEnum `json:"chassisIDSubtype"`
	ChassisID        string               `json:"chassisID"`
	NumberOfPorts    int                  `json:"numberOfPorts"`
	SysObjID         string               `json:"sysObjID"`
}

// Timer settings for LLDP
type LldpLocalTimer struct {
	TransmitInterval      uint32 `json:"transmitInterval"`      // seconds
	NotificationInterval  uint32 `json:"notificationInterval"`  // seconds
	ReinitializationDelay uint32 `json:"reinitializationDelay"` // seconds
	HoldtimeMultiplier    uint32 `json:"holdtimeMultiplier"`    //times
}

// Remote LLDP neighbor statistics
type LldpRemoteStatistics struct {
	LastChangeTime uint32 `json:"lastChangeTime"` // timestamp
	Inserts        uint32 `json:"inserts"`
	Drops          uint32 `json:"drops"`
	Deletes        uint32 `json:"deletes"`
	Ageouts        uint32 `json:"ageouts"`
}

// ENUM: enable / disable
type LldpStatusTypeEnum string

const (
	LldpStatusEnable  LldpStatusTypeEnum = "enable"
	LldpStatusDisable LldpStatusTypeEnum = "disable"
)

func (lldpStatusTypeEnum *LldpStatusTypeEnum) UnmarshalJSON(b []byte) error {
	var status string
	if err := json.Unmarshal(b, &status); err != nil {
		return err
	}
	switch status {
	case "enable":
		*lldpStatusTypeEnum = LldpStatusEnable
	case "disable":
		*lldpStatusTypeEnum = LldpStatusDisable
	default:
		return fmt.Errorf("invalid LLDP status type: %s. Should be \"enable\" or \"disable\"", status)
	}
	return nil
}
func (lldpStatusTypeEnum LldpStatusTypeEnum) MarshalJSON() ([]byte, error) {
	var status string
	switch lldpStatusTypeEnum {
	case LldpStatusEnable:
		status = "enabled"
	case LldpStatusDisable:
		status = "disabled"
	default:
		return nil, fmt.Errorf("invalid LLDP status type: %s", lldpStatusTypeEnum)
	}
	return json.Marshal(status)
}

// LLDP port entry
type LldpPort struct {
	ID                   int                    `json:"id"` // device defined port ID
	PortID               string                 `json:"portID"`
	PortIDSubtype        PortIDSubtypeEnum      `json:"portIDSubtype"`
	PortType             string                 `json:"portType"` // TBD with yumi
	PortDescription      string                 `json:"portDescription"`
	InterfaceDescription string                 `json:"interfaceDescription"`
	TxStatus             LldpStatusTypeEnum     `json:"txStatus"` // enum: enable / disable
	RxStatus             LldpStatusTypeEnum     `json:"rxStatus"` // enum: enable / disable
	Neighbors            []LldpNeighbor         `json:"neighbors"`
	TrafficStatistics    *LldpTrafficStatistics `json:"trafficStatistics,omitempty"`
}

// Neighbor entry
type LldpNeighbor struct {
	PortID           string               `json:"portID"`          // LLDP learned, not device defined port ID
	PortIDSubtype    PortIDSubtypeEnum    `json:"portIDSubtype"`   // LLDP learned, not device defined port ID subtype
	PortDescription  string               `json:"portDescription"` // TBD with yumi
	ChassisIDSubtype ChassisIDSubtypeEnum `json:"ChassisIDSubtype"`
	ChassisID        string               `json:"chassisID"`
}

// LLDP per-port traffic counters
type LldpTrafficStatistics struct {
	TxTotalFrame            uint32 `json:"txTotalFrame"`
	RxTotalEntriesAged      uint32 `json:"rxTotalEntriesAged"`
	RxTotalFrames           uint32 `json:"rxTotalFrames"`
	RxErrorFrames           uint32 `json:"rxErrorFrames"`
	RxTotalDiscardsFrames   uint32 `json:"rxTotalDiscardsFrames"`
	RxTotalUnrecognizedTlvs uint32 `json:"rxTotalUnrecognizedTlvs"`
	RxTotalDiscardsTlvs     uint32 `json:"rxTotalDiscardsTlvs"`
}
