package domain

// 使用者無法更改的資訊
type DeviceInterfaceInfo struct {
	ProjectId   int64 `json:"ProjectId"`
	DeviceId    int64 `json:"DeviceId"`
	InterfaceId int64 `json:"InterfaceId"`
}

// 使用者可更改的資訊
type DeviceInterfaceConf struct {
	InterfaceName string `json:"InterfaceName"`
}

// 實際設備的資訊
type DeviceInterfaceStat struct {
}

type DeviceInterface struct {
	DeviceInterfaceInfo
	DeviceInterfaceConf
	DeviceInterfaceStat
}
