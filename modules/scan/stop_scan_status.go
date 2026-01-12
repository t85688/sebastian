package scan

type StopScanResultStatus int

const (
	StopScanResultStatusUnknown StopScanResultStatus = iota
	StopScanResultStatusCompleted
	StopScanResultStatusNotScanning
)

var StopScanResultStatusStrMap = map[StopScanResultStatus]string{
	StopScanResultStatusUnknown:     "Unknown",
	StopScanResultStatusCompleted:   "Completed",
	StopScanResultStatusNotScanning: "NotScanning",
}

func (status StopScanResultStatus) String() string {
	if str, ok := StopScanResultStatusStrMap[status]; ok {
		return str
	}
	return "Unknown"
}
