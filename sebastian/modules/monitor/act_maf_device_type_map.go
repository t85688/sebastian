package monitor

func getActDeviceType(mafDeviceType string) ActDeviceType {
	actDeviceType, ok := mafActDeviceTypeMap[mafDeviceType]
	if ok {
		return actDeviceType
	}

	return ActDeviceTypeUnknown
}

var mafActDeviceTypeMap = map[string]ActDeviceType{
	MafDeviceTypeEmpty:    ActDeviceTypeUnknown,
	MafDeviceTypeNotFound: ActDeviceTypeUnknown,
	MafDeviceTypeICMP:     ActDeviceTypeICMP,
	MafDeviceTypeUnknown:  ActDeviceTypeUnknown,
	MafDeviceTypeMOXA:     ActDeviceTypeMoxa,
	MafDeviceTypeCompat:   ActDeviceTypeCompat,
}
