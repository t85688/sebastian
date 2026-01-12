package wscommand

type MonitorManagementEndpoint struct {
	DeviceId    int64 `json:"DeviceId,omitempty"`
	InterfaceId int64 `json:"InterfaceId,omitempty"`
}

type MonitorAliveDevices []*MonitorAliveDevice

type MonitorAliveDevice struct {
	Alive     bool   `json:"Alive"`
	Id        int64  `json:"Id"`
	IpAddress string `json:"IpAddress"`
}

type MonitorAliveLinks []*MonitorAliveLink

type MonitorAliveLink struct {
	Alive                  bool   `json:"Alive"`
	DestinationDeviceId    int64  `json:"DestinationDeviceId"`
	DestinationDeviceIp    string `json:"DestinationDeviceIp"`
	DestinationInterfaceId int64  `json:"DestinationInterfaceId"`
	Id                     int64  `json:"Id"`
	Redundancy             bool   `json:"Redundancy"`
	SourceDeviceId         int64  `json:"SourceDeviceId"`
	SourceDeviceIp         string `json:"SourceDeviceIp"`
	SourceInterfaceId      int64  `json:"SourceInterfaceId"`
}

type MonitorSystemStatusUpdateDevices []*MonitorSystemStatusUpdateDevice

type MonitorSystemStatusUpdateDevice struct {
	DeviceId   int64  `json:"DeviceId"`
	DeviceIp   string `json:"DeviceIp"`
	Alias      string `json:"Alias"`
	MacAddress string `json:"MacAddress"`
	ModelName  string `json:"ModelName"`

	ModularInfo       *DeviceModularInfo      `json:"ModularInfo"`
	FirmwareVersion   string                  `json:"FirmwareVersion"`
	ProductRevision   string                  `json:"ProductRevision"`
	SerialNumber      string                  `json:"SerialNumber"`
	DeviceName        string                  `json:"DeviceName"`
	Role              string                  `json:"Role"`
	RedundantProtocol []string                `json:"RedundantProtocol"`
	CPUUsage          string                  `json:"CPUUsage"`
	MemoryUsage       string                  `json:"MemoryUsage"`
	SystemUptime      string                  `json:"SystemUptime"`
	Interfaces        []*DeviceInterfaceEntry `json:"Interfaces"`
}

type DeviceInterfaceEntry struct {
	// DeviceId      int64  `json:"DeviceId"`
	InterfaceId   int64  `json:"InterfaceId"`
	InterfaceName string `json:"InterfaceName"`
	Description   string `json:"Description"`
	MacAddress    string `json:"MacAddress"`
	// IpAddress     string `json:"IpAddress"`
	Active bool `json:"Active"`
}

type DeviceModularInfo struct {
	Ethernet map[int]EthernetModule `json:"Ethernet"`
	Power    map[int]PowerModule    `json:"Power"`
}

type EthernetModule struct {
	Exist           bool   `json:"Exist"`
	ModuleName      string `json:"ModuleName"`
	SerialNumber    string `json:"SerialNumber"`
	ProductRevision string `json:"ProductRevision"`
	Status          string `json:"Status"`
	ModuleId        int64  `json:"ModuleId"`
}

type PowerModule struct {
	Exist           bool   `json:"Exist"`
	ModuleName      string `json:"ModuleName"`
	SerialNumber    string `json:"SerialNumber"`
	ProductRevision string `json:"ProductRevision"`
	Status          string `json:"Status"`
}

type MonitorTraffiicUpdateLinks []*MonitorTraffiicUpdateLink

type MonitorTraffiicUpdateLink struct {
	DestinationDeviceId           int64  `json:"DestinationDeviceId"`
	DestinationDeviceIp           string `json:"DestinationDeviceIp"`
	DestinationInterfaceId        int64  `json:"DestinationInterfaceId"`
	SourceDeviceId                int64  `json:"SourceDeviceId"`
	SourceDeviceIp                string `json:"SourceDeviceIp"`
	SourceInterfaceId             int64  `json:"SourceInterfaceId"`
	LinkId                        int64  `json:"LinkId"`
	Speed                         int64  `json:"Speed"`
	Timestamp                     int64  `json:"Timestamp"`
	SourceTrafficUtilization      int64  `json:"SourceTrafficUtilization"`
	DestinationTrafficUtilization int64  `json:"DestinationTrafficUtilization"`
}

type MonitorSwiftDevices []*MonitorSwiftDevice

type MonitorSwiftDevice struct {
	DeviceId int64  `json:"DeviceId"`
	DeviceIp string `json:"DeviceIp"`
	Offline  bool   `json:"Offline"`
	Online   bool   `json:"Online"`
}
