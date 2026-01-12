package netdl

type LinkRedundancyStatus int

const (
	LinkRedundancyStatusNone LinkRedundancyStatus = iota
	LinkRedundancyStatusForwarding
	LinkRedundancyStatusBlocking
)

var (
	linkRedundancyStatusMap = map[LinkRedundancyStatus]string{
		LinkRedundancyStatusNone:       "",
		LinkRedundancyStatusForwarding: "forwarding",
		LinkRedundancyStatusBlocking:   "blocking",
	}
)

func (lrs LinkRedundancyStatus) String() string {
	if redundancyStr, ok := linkRedundancyStatusMap[lrs]; ok {
		return redundancyStr
	}
	return ""
}
