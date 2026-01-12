package netdl

import (
	"encoding/json"
	"fmt"
)

type TimeSyncProfile string

const (
	ProfileIEEE1588Default2008 TimeSyncProfile = "IEEE 1588 Default-2008"
	ProfileIEEE802Dot1AS2011   TimeSyncProfile = "IEEE 802.1AS-2011"
	MultipleProfiles           TimeSyncProfile = "multipleProfiles"
	NotSupportProfile          TimeSyncProfile = "notSupport"
)

func (timeSyncProfile *TimeSyncProfile) UnmarshalJSON(b []byte) error {
	var profile string
	if err := json.Unmarshal(b, &profile); err != nil {
		return err
	}
	switch profile {
	case "IEEE 1588 Default-2008":
		*timeSyncProfile = ProfileIEEE1588Default2008
	case "IEEE 802.1AS-2011":
		*timeSyncProfile = ProfileIEEE802Dot1AS2011
	case "multipleProfiles":
		*timeSyncProfile = MultipleProfiles
	case "notSupport":
		*timeSyncProfile = NotSupportProfile
	default:
		return fmt.Errorf("invalid time sync profile: %s. Should be \"IEEE 1588 Default-2008\", \"IEEE 802.1AS-2011\"or \"multipleProfiles\"", profile)
	}
	return nil
}

func (timeSyncProfile TimeSyncProfile) MarshalJSON() ([]byte, error) {
	var profile string
	switch timeSyncProfile {
	case ProfileIEEE1588Default2008:
		profile = "IEEE 1588 Default-2008"
	case ProfileIEEE802Dot1AS2011:
		profile = "IEEE 802.1AS-2011"
	case MultipleProfiles:
		profile = "multipleProfiles"
	case NotSupportProfile:
		profile = "notSupport"
	default:
		return nil, fmt.Errorf("invalid time sync profile: %s", timeSyncProfile)
	}
	return json.Marshal(profile)
}

type ClockType string

const (
	ClockTypeBoundary    ClockType = "boundary clock"
	ClockTypeTransparent ClockType = "transparent clock"
)

func (clockType *ClockType) UnmarshalJSON(b []byte) error {
	var clockTypeStr string
	if err := json.Unmarshal(b, &clockTypeStr); err != nil {
		return err
	}
	switch clockTypeStr {
	case "boundary clock":
		*clockType = ClockTypeBoundary
	case "transparent clock":
		*clockType = ClockTypeTransparent
	default:
		return fmt.Errorf("invalid clock type: %s. Should be \"boundary clock\" or \"transparent clock\"", clockTypeStr)
	}
	return nil
}

func (clockType ClockType) MarshalJSON() ([]byte, error) {
	var clockTypeStr string
	switch clockType {
	case ClockTypeBoundary:
		clockTypeStr = "boundary clock"
	case ClockTypeTransparent:
		clockTypeStr = "transparent clock"
	default:
		return nil, fmt.Errorf("invalid clock type: %s", clockType)
	}
	return json.Marshal(clockTypeStr)
}

type DelayMechanism string

const (
	DelayMechEndToEnd   DelayMechanism = "end-to-end"
	DelayMechPeerToPeer DelayMechanism = "peer-to-peer"
)

func (delayMechanism *DelayMechanism) UnmarshalJSON(b []byte) error {
	var delayMechanismStr string
	if err := json.Unmarshal(b, &delayMechanismStr); err != nil {
		return err
	}
	switch delayMechanismStr {
	case "end-to-end":
		*delayMechanism = DelayMechEndToEnd
	case "peer-to-peer":
		*delayMechanism = DelayMechPeerToPeer
	default:
		return fmt.Errorf("invalid delay mechanism: %s. Should be \"end-to-end\" or \"peer-to-peer\"", delayMechanismStr)
	}
	return nil
}

func (delayMechanism DelayMechanism) MarshalJSON() ([]byte, error) {
	var delayMechanismStr string
	switch delayMechanism {
	case DelayMechEndToEnd:
		delayMechanismStr = "end-to-end"
	case DelayMechPeerToPeer:
		delayMechanismStr = "peer-to-peer"
	default:
		return nil, fmt.Errorf("invalid delay mechanism: %s", delayMechanism)
	}
	return json.Marshal(delayMechanismStr)
}

type TransportMode string

const (
	TransportModeUDPIPv4             TransportMode = "UDP IPv4"
	TransportModeIEEE802Dot3Ethernet TransportMode = "IEEE 802.3 Ethernet"
)

func (transportMode *TransportMode) UnmarshalJSON(b []byte) error {
	var transportModeStr string
	if err := json.Unmarshal(b, &transportModeStr); err != nil {
		return err
	}
	switch transportModeStr {
	case "UDP IPv4":
		*transportMode = TransportModeUDPIPv4
	case "IEEE 802.3 Ethernet":
		*transportMode = TransportModeIEEE802Dot3Ethernet
	default:
		return fmt.Errorf("invalid transport mode: %s. Should be \"UDP IPv4\" or \"IEEE 802.3 Ethernet\"", transportModeStr)
	}
	return nil
}

func (transportMode TransportMode) MarshalJSON() ([]byte, error) {
	var transportModeStr string
	switch transportMode {
	case TransportModeUDPIPv4:
		transportModeStr = "UDP IPv4"
	case TransportModeIEEE802Dot3Ethernet:
		transportModeStr = "IEEE 802.3 Ethernet"
	default:
		return nil, fmt.Errorf("invalid transport mode: %s", transportMode)
	}
	return json.Marshal(transportModeStr)
}

type ClockMode string

const (
	ClockModeOneStep ClockMode = "one-step"
	ClockModeTwoStep ClockMode = "two-step"
)

func (clockMode *ClockMode) UnmarshalJSON(b []byte) error {
	var clockModeStr string
	if err := json.Unmarshal(b, &clockModeStr); err != nil {
		return err
	}
	switch clockModeStr {
	case "one-step":
		*clockMode = ClockModeOneStep
	case "two-step":
		*clockMode = ClockModeTwoStep
	default:
		return fmt.Errorf("invalid clock mode: %s. Should be \"one-step\" or \"two-step\"", clockModeStr)
	}
	return nil
}

func (clockMode ClockMode) MarshalJSON() ([]byte, error) {
	var clockModeStr string
	switch clockMode {
	case ClockModeOneStep:
		clockModeStr = "one-step"
	case ClockModeTwoStep:
		clockModeStr = "two-step"
	default:
		return nil, fmt.Errorf("invalid clock mode: %s", clockMode)
	}
	return json.Marshal(clockModeStr)
}

type TimeSyncSetting struct {
	Enable              bool                `json:"enable"`
	Profile             TimeSyncProfile     `json:"profile"`
	As2011Setting       *As2011Setting      `json:"ieee802Dot1As2011Setting,omitempty"`
	Ptp1588Default2008  *Ptp1588Default2008 `json:"ieee1588Default2008Setting,omitempty"`
	TimeSyncPortEntries []TimeSyncPortEntry `json:"timeSyncPortEntries"`
}

type As2011Setting struct {
	Priority1     int `json:"priority1"`
	Priority2     int `json:"priority2"`
	AccuracyAlert int `json:"accuracyAlert"`
}

type Ptp1588Default2008 struct {
	ClockType           ClockType      `json:"clockType"`
	DelayMechanism      DelayMechanism `json:"delayMechanism"`
	TransportMode       TransportMode  `json:"transportMode"`
	Priority1           int            `json:"priority1"`
	Priority2           int            `json:"priority2"`
	DomainNumber        int            `json:"domainNumber"`
	ClockMode           ClockMode      `json:"clockMode"`
	AccuracyAlert       int            `json:"accuracyAlert"`
	MaximumStepsRemoved *int           `json:"maximumStepsRemoved,omitempty"`
}

type TimeSyncPortEntry struct {
	PortID                 int             `json:"portId"`
	AliasName              string          `json:"aliasName"`
	Enable                 bool            `json:"enable"`
	Profile                TimeSyncProfile `json:"profile"`
	AnnounceInterval       int             `json:"announceInterval"`
	AnnounceReceiptTimeout int             `json:"announceReceiptTimeout"`
	SyncInterval           int             `json:"syncInterval"`
	DelayRequestInterval   *int            `json:"delayRequestInterval,omitempty"`
	PdelayRequestInterval  *int            `json:"pdelayRequestInterval,omitempty"`

	// AS only
	SyncReceiptTimeout                *int `json:"syncReceiptTimeout,omitempty"`                // AS only
	NeighborPropagationDelayThreshold *int `json:"neighborPropagationDelayThreshold,omitempty"` // AS only
}
