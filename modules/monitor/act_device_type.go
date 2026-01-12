package monitor

type ActDeviceType int

const (
	ActDeviceTypeUnknown ActDeviceType = iota
	ActDeviceTypeSwitch
	ActDeviceTypeEndStation
	ActDeviceTypeICMP
	ActDeviceTypeMoxa
	ActDeviceTypeCompat
)

var actDeviceTypeModelNameMap = map[ActDeviceType]string{
	ActDeviceTypeUnknown:    "Unknown",
	ActDeviceTypeSwitch:     "Switch",
	ActDeviceTypeEndStation: "End-Station",
	ActDeviceTypeICMP:       "ICMP",
	ActDeviceTypeMoxa:       "Moxa",
	ActDeviceTypeCompat:     "Compatible",
}

func (deviceType ActDeviceType) ModelName() string {
	if modelName, exists := actDeviceTypeModelNameMap[deviceType]; exists {
		return modelName
	}

	return "Unknown"
}
