package monitor

type MonitorTaskType int

const (
	MonitorTaskTypeDeviceAliveStatus MonitorTaskType = iota + 1
	MonitorTaskTypeLinkAliveStatus
	MonitorTaskTypeManagementLink
	MonitorTaskTypeDeviceSystemStatus
	MonitorTaskTypeDeviceCreated
	MonitorTaskTypeDeviceDeleted
	MonitorTaskTypeUpdateDeviceConfiguration
	MonitorTaskTypeSFP
	MonitorTaskTypeTraffic
	MonitorTaskTypeVLAN
	MonitorTaskTypeSwift
)

var monitorTaskTypeToString = map[MonitorTaskType]string{
	MonitorTaskTypeDeviceAliveStatus:         "DeviceAliveStatus",
	MonitorTaskTypeLinkAliveStatus:           "LinkAliveStatus",
	MonitorTaskTypeManagementLink:            "ManagermentLink",
	MonitorTaskTypeDeviceSystemStatus:        "DeviceSystemStatus",
	MonitorTaskTypeDeviceCreated:             "DeviceCreated",
	MonitorTaskTypeDeviceDeleted:             "DeviceDeleted",
	MonitorTaskTypeUpdateDeviceConfiguration: "UpdateDeviceConfiguration",
	MonitorTaskTypeSFP:                       "SFP",
	MonitorTaskTypeTraffic:                   "Traffic",
	MonitorTaskTypeVLAN:                      "VLAN",
	MonitorTaskTypeSwift:                     "Swift",
}

func (mtt MonitorTaskType) String() string {
	if str, ok := monitorTaskTypeToString[mtt]; ok {
		return str
	}

	return "Unknown"
}
