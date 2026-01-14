package configmapper

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

func MapTimeSyncProfile(profile netdl.TimeSyncProfile) (domain.TimeSyncProfile, bool) {
	switch profile {
	case netdl.ProfileIEEE1588Default2008:
		return domain.TimeSyncProfileIEEE1588_2008, true
	case netdl.ProfileIEEE802Dot1AS2011:
		return domain.TimeSyncProfileIEEE8021AS_2011, true
	default:
		return 0, false
	}
}

func Map1588ClockType(clockType netdl.ClockType) (domain.IEEE1588ClockType, bool) {
	switch clockType {
	case netdl.ClockTypeBoundary:
		return domain.IEEE1588ClockTypeBoundaryClock, true
	case netdl.ClockTypeTransparent:
		return domain.IEEE1588ClockTypeTransparentClock, true
	default:
		return 0, false
	}
}

func Map1588DelayMechanism(delayMechanism netdl.DelayMechanism) (domain.IEEE1588CDelayMechanism, bool) {
	switch delayMechanism {
	case netdl.DelayMechEndToEnd:
		return domain.IEEE1588DelayMechanismEndToEnd, true
	case netdl.DelayMechPeerToPeer:
		return domain.IEEE1588DelayMechanismPeerToPeer, true
	default:
		return 0, false
	}
}

func Map1588TransportMode(transportMode netdl.TransportMode) (domain.IEEE1588TransportType, bool) {
	switch transportMode {
	case netdl.TransportModeUDPIPv4:
		return domain.IEEE1588TransportTypeUDPIPv4, true
	case netdl.TransportModeIEEE802Dot3Ethernet:
		return domain.IEEE1588TransportTypeIEEE802Dot3Ethernet, true
	default:
		return 0, false
	}
}

func Map1588ClockMode(clockMode netdl.ClockMode) (domain.IEEE1588ClockMode, bool) {
	switch clockMode {
	case netdl.ClockModeOneStep:
		return domain.IEEE1588ClockModeOneStep, true
	case netdl.ClockModeTwoStep:
		return domain.IEEE1588ClockModeTwoStep, true
	default:
		return 0, false
	}
}
