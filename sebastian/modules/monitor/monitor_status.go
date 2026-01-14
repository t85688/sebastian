package monitor

type MonitorStatus int

const (
	MonitorStatusUnknown MonitorStatus = iota
	MonitorStatusStopped
	MonitorStatusRunning
)

var monitorStatusStrMap = map[MonitorStatus]string{
	MonitorStatusUnknown: "Unknown",
	MonitorStatusStopped: "Stopped",
	MonitorStatusRunning: "Running",
}

func (ms MonitorStatus) String() string {
	if str, ok := monitorStatusStrMap[ms]; ok {
		return str
	}

	return "Unknown"
}
