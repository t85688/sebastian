package netdl

type LinkStatus int

const (
	LinkStatusUnknown LinkStatus = iota
	LinkStatusUp
	LinkStatusDown
)

var (
	linkStatusMap = map[LinkStatus]string{
		LinkStatusUnknown: "unknown",
		LinkStatusUp:      "up",
		LinkStatusDown:    "down",
	}
)

func (linkStatus LinkStatus) String() string {
	if linkStatusStr, ok := linkStatusMap[linkStatus]; ok {
		return linkStatusStr
	}
	return "unknown"
}
