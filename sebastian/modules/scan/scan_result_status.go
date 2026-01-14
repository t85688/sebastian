package scan

type ScanResultStatus int

const (
	ScanResultStatusUnknown ScanResultStatus = iota
	ScanResultStatusCancel
	ScanResultStatusCompleted
	ScanResultStatusBusy
)

var ScanResultStatusStrMap = map[ScanResultStatus]string{
	ScanResultStatusUnknown:   "Unknown",
	ScanResultStatusCancel:    "Cancel",
	ScanResultStatusCompleted: "Completed",
	ScanResultStatusBusy:      "Busy",
}

func (status ScanResultStatus) String() string {
	if str, ok := ScanResultStatusStrMap[status]; ok {
		return str
	}
	return "Unknown"
}
