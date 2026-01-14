package wscommand

type OperationsDeviceResultStatus int

const (
	OperationsDeviceResultStatusSuccess OperationsDeviceResultStatus = 1 // Success
	OperationsDeviceResultStatusFailed  OperationsDeviceResultStatus = 2 // Failed
)

var OperationsDeviceResultStatusMap = map[OperationsDeviceResultStatus]string{
	OperationsDeviceResultStatusSuccess: "Success",
	OperationsDeviceResultStatusFailed:  "Failed",
}

func (cmd OperationsDeviceResultStatus) String() string {
	if name, ok := OperationsDeviceResultStatusMap[cmd]; ok {
		return name
	}
	return OperationsDeviceResultStatusMap[OperationsDeviceResultStatusFailed]
}

type OperationsDeviceResult struct {
	Progress     int64  `json:"Progress"`
	Id           int64  `json:"Id,omitempty"`
	Status       string `json:"Status,omitempty"`
	ErrorMessage string `json:"ErrorMessage,omitempty"`
}
