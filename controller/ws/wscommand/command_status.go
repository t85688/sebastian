package wscommand

type ActWSResponseStatus int64

const (
	ActWSResponseStatusUnknown ActWSResponseStatus = 0
	ActWSResponseStatusStop    ActWSResponseStatus = 1005
	ActWSResponseStatusFailed  ActWSResponseStatus = 1006
)

var ActWSResponseStatusMap = map[ActWSResponseStatus]string{
	ActWSResponseStatusUnknown: "Unknown",
	ActWSResponseStatusStop:    "Stop",
	ActWSResponseStatusFailed:  "Failed",
}

func ParseActWSResponseStatus(opCode int64) ActWSResponseStatus {
	if _, ok := ActWSResponseStatusMap[ActWSResponseStatus(opCode)]; ok {
		return ActWSResponseStatus(opCode)
	}
	return ActWSResponseStatusUnknown
}

func (cmd ActWSResponseStatus) Int64() int64 {
	return int64(cmd)
}

func (cmd ActWSResponseStatus) String() string {
	if name, ok := ActWSResponseStatusMap[cmd]; ok {
		return name
	}
	return ActWSResponseStatusMap[ActWSResponseStatusUnknown]
}
