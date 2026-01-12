package netdl

type RstpPortState int

const (
	RstpPortStateUnknown RstpPortState = iota
	RstpPortStateDisabled
	RstpPortStateBlocking
	RstpPortStateListening
	RstpPortStateLearning
	RstpPortStateForwarding
	RstpPortStateBroken
)

var RstpPortStateMap = map[RstpPortState]string{
	RstpPortStateUnknown:    "unknown",
	RstpPortStateDisabled:   "disabled",
	RstpPortStateBlocking:   "blocking",
	RstpPortStateListening:  "listening",
	RstpPortStateLearning:   "learning",
	RstpPortStateForwarding: "forwarding",
	RstpPortStateBroken:     "broken",
}

func (state RstpPortState) String() string {
	if str, ok := RstpPortStateMap[state]; ok {
		return str
	}
	return "unknown"
}
