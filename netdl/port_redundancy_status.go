package netdl

type PortRedundancyStatus int

const (
	PortRedundancyStatusNone PortRedundancyStatus = iota
	PortRedundancyStatusForwarding
	PortRedundancyStatusBlocking
)

var (
	portRedundancyStatusMap = map[PortRedundancyStatus]string{
		PortRedundancyStatusNone:       "",
		PortRedundancyStatusForwarding: "forwarding",
		PortRedundancyStatusBlocking:   "blocking",
	}
)

func (prs PortRedundancyStatus) String() string {
	if redundancyStr, ok := portRedundancyStatusMap[prs]; ok {
		return redundancyStr
	}
	return ""
}
