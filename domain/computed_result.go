package domain

type ComputedResult struct {
	CycleTime         int                `json:"CycleTime"`
	DeviceViewResults []DeviceViewResult `json:"DeviceViewResults"`
	Devices           []Device           `json:"Devices"`
	GclResults        []GclResult        `json:"GclResults"`
	Links             []Link             `json:"Links"`
	RoutingResults    []RoutingResult    `json:"RoutingResults"`
	StreamViewResults []StreamViewResult `json:"StreamViewResults"`
	Streams           []interface{}      `json:"Streams"` // 目前空
	TrafficDesign     TrafficDesign      `json:"TrafficDesign"`
}

// --- 以下為各子結構 ---

type DeviceViewResult struct {
	DeviceId         int               `json:"DeviceId"`
	DeviceIp         string            `json:"DeviceIp"`
	InterfaceResults []InterfaceResult `json:"InterfaceResults"`
}

type InterfaceResult struct {
	InterfaceId   int            `json:"InterfaceId"`
	InterfaceName string         `json:"InterfaceName"`
	StreamResults []StreamResult `json:"StreamResults"`
}

type StreamResult struct {
	FrameOffsetList   []interface{} `json:"FrameOffsetList"`
	Listeners         []string      `json:"Listeners"`
	PriorityCodePoint int           `json:"PriorityCodePoint"`
	QueueId           string        `json:"QueueId"`
	StartTime         int           `json:"StartTime"`
	StopTime          int           `json:"StopTime"`
	StreamId          int           `json:"StreamId"`
	StreamName        string        `json:"StreamName"`
	StreamType        string        `json:"StreamType"`
	Talker            string        `json:"Talker"`
	VlanId            int           `json:"VlanId"`
}

type AutoScan struct {
	BroadcastSearch   bool              `json:"BroadcastSearch"`
	DeviceInformation DeviceInformation `json:"DeviceInformation"`
	Identify          Identify          `json:"Identify"`
	LLDP              bool              `json:"LLDP"`
}

type DeviceInformation struct {
	DeviceName        bool `json:"DeviceName"`
	IPConfiguration   bool `json:"IPConfiguration"`
	InterfaceMAC      bool `json:"InterfaceMAC"`
	InterfaceName     bool `json:"InterfaceName"`
	Location          bool `json:"Location"`
	MACTable          bool `json:"MACTable"`
	ModularInfo       bool `json:"ModularInfo"`
	PortInfo          bool `json:"PortInfo"`
	PortSpeed         bool `json:"PortSpeed"`
	ProductRevision   bool `json:"ProductRevision"`
	RedundantProtocol bool `json:"RedundantProtocol"`
	SerialNumber      bool `json:"SerialNumber"`
	SystemUptime      bool `json:"SystemUptime"`
}

type Identify struct {
	FirmwareVersion bool `json:"FirmwareVersion"`
	ModelName       bool `json:"ModelName"`
	VendorID        bool `json:"VendorID"`
}

type Configuration struct {
	CheckConfigSynchronization bool                 `json:"CheckConfigSynchronization"`
	InformationSetting         bool                 `json:"InformationSetting"`
	LoginPolicy                bool                 `json:"LoginPolicy"`
	LoopProtection             bool                 `json:"LoopProtection"`
	ManagementInterface        bool                 `json:"ManagementInterface"`
	NetworkSetting             bool                 `json:"NetworkSetting"`
	PortSetting                PortSetting          `json:"PortSetting"`
	SNMPTrapSetting            bool                 `json:"SNMPTrapSetting"`
	STPRSTP                    STPRSTP              `json:"STPRSTP"`
	StaticForwardSetting       StaticForwardSetting `json:"StaticForwardSetting"`
	SyslogSetting              bool                 `json:"SyslogSetting"`
	TSN                        TSNSetting           `json:"TSN"`
	TimeSetting                TimeSetting          `json:"TimeSetting"`
	TimeSyncSetting            TimeSyncSetting      `json:"TimeSyncSetting"`
	UserAccount                bool                 `json:"UserAccount"`
	VLANSetting                VLANSetting          `json:"VLANSetting"`
}

type PortSetting struct {
	AdminStatus bool `json:"AdminStatus"`
}

type STPRSTP struct {
	BPDUFilter        bool `json:"BPDUFilter"`
	BPDUGuard         bool `json:"BPDUGuard"`
	ErrorRecoveryTime bool `json:"ErrorRecoveryTime"`
	LinkType          bool `json:"LinkType"`
	LoopGuard         bool `json:"LoopGuard"`
	PortRSTPEnable    bool `json:"PortRSTPEnable"`
	RSTP              bool `json:"RSTP"`
	RootGuard         bool `json:"RootGuard"`
	Swift             bool `json:"Swift"`
}

type StaticForwardSetting struct {
	Multicast bool `json:"Multicast"`
	Unicast   bool `json:"Unicast"`
}

type TSNSetting struct {
	IEEE802Dot1CB  bool `json:"IEEE802Dot1CB"`
	IEEE802Dot1Qbv bool `json:"IEEE802Dot1Qbv"`
}

type TimeSetting struct {
	PTP        bool `json:"PTP"`
	SystemTime bool `json:"SystemTime"`
}

type TimeSyncSetting struct {
	IEC61850_2016                     bool `json:"IEC61850_2016"`
	IEEE1588_2008                     bool `json:"IEEE1588_2008"`
	IEEE1588_2008_ClockMode           bool `json:"IEEE1588_2008_ClockMode"`
	IEEE1588_2008_ClockType           bool `json:"IEEE1588_2008_ClockType"`
	IEEE1588_2008_MaximumStepsRemoved bool `json:"IEEE1588_2008_MaximumStepsRemoved"`
	IEEE802Dot1AS_2011                bool `json:"IEEE802Dot1AS_2011"`
	IEEEC37Dot238_2017                bool `json:"IEEEC37Dot238_2017"`
}

type VLANSetting struct {
	AccessTrunkMode     bool `json:"AccessTrunkMode"`
	DefaultPCP          bool `json:"DefaultPCP"`
	DefaultPVID         bool `json:"DefaultPVID"`
	HybridMode          bool `json:"HybridMode"`
	ManagementVLAN      bool `json:"ManagementVLAN"`
	PerStreamPriority   bool `json:"PerStreamPriority"`
	PerStreamPriorityV2 bool `json:"PerStreamPriorityV2"`
	TEMSTID             bool `json:"TEMSTID"`
}

type Monitor struct {
	BasicStatus         BasicStatus         `json:"BasicStatus"`
	Redundancy          Redundancy          `json:"Redundancy"`
	TimeSynchronization TimeSynchronization `json:"TimeSynchronization"`
	Traffic             Traffic             `json:"Traffic"`
}

type BasicStatus struct {
	FiberCheck        bool `json:"FiberCheck"`
	PortStatus        bool `json:"PortStatus"`
	SystemUtilization bool `json:"SystemUtilization"`
}

type Redundancy struct {
	RSTP bool `json:"RSTP"`
}

type TimeSynchronization struct {
	IEEE1588_2008      bool `json:"IEEE1588_2008"`
	IEEE802Dot1AS_2011 bool `json:"IEEE802Dot1AS_2011"`
}

type Traffic struct {
	TrafficUtilization bool `json:"TrafficUtilization"`
	TxTotalOctets      bool `json:"TxTotalOctets"`
	TxTotalPackets     bool `json:"TxTotalPackets"`
}

type Operation struct {
	CLI               bool `json:"CLI"`
	EnableSNMPService bool `json:"EnableSNMPService"`
	EventLog          bool `json:"EventLog"`
	FactoryDefault    bool `json:"FactoryDefault"`
	FirmwareUpgrade   bool `json:"FirmwareUpgrade"`
	ImportExport      bool `json:"ImportExport"`
	Locator           bool `json:"Locator"`
	Reboot            bool `json:"Reboot"`
}

type SequenceRange struct {
	Max int `json:"Max"`
	Min int `json:"Min"`
}

type Interval struct {
	Denominator int `json:"Denominator"`
	Numerator   int `json:"Numerator"`
}

type TimeAware struct {
	EarliestTransmitOffset int `json:"EarliestTransmitOffset"`
	Jitter                 int `json:"Jitter"`
	LatestTransmitOffset   int `json:"LatestTransmitOffset"`
}

type GclResult struct {
	DeviceId              int                    `json:"DeviceId"`
	DeviceIp              string                 `json:"DeviceIp"`
	InterfaceGateControls []InterfaceGateControl `json:"InterfaceGateControls"`
}

type InterfaceGateControl struct {
	GateControls  []GateControl `json:"GateControls"`
	InterfaceId   int           `json:"InterfaceId"`
	InterfaceName string        `json:"InterfaceName"`
}

type RoutingResult struct {
	CB                bool          `json:"CB"`
	Multicast         bool          `json:"Multicast"`
	PriorityCodePoint int           `json:"PriorityCodePoint"`
	QueueId           int           `json:"QueueId"`
	RoutingPaths      []RoutingPath `json:"RoutingPaths"`
	StreamId          int           `json:"StreamId"`
	StreamName        string        `json:"StreamName"`
	VlanId            int           `json:"VlanId"`
}

type RoutingPath struct {
	ListenerIpAddress string          `json:"ListenerIpAddress"`
	RedundantPaths    []RedundantPath `json:"RedundantPaths"`
	TalkerIpAddress   string          `json:"TalkerIpAddress"`
}

type RedundantPath struct {
	DeviceIds []int `json:"DeviceIds"`
	LinkIds   []int `json:"LinkIds"`
	VlanId    int   `json:"VlanId"`
}

type StreamViewResult struct {
	FrameType         string             `json:"FrameType"`
	Interval          Interval           `json:"Interval"`
	StreamId          int                `json:"StreamId"`
	StreamName        string             `json:"StreamName"`
	StreamPathResults []StreamPathResult `json:"StreamPathResults"`
	StreamTrafficType string             `json:"StreamTrafficType"`
	StreamType        string             `json:"StreamType"`
	TimeSlotIndex     int                `json:"TimeSlotIndex"`
}

type StreamPathResult struct {
	StreamRedundantPathResults []StreamRedundantPathResult `json:"StreamRedundantPathResults"`
}

type StreamRedundantPathResult struct {
	DeviceInterfaceResults []DeviceInterfaceResult `json:"DeviceInterfaceResults"`
}

type DeviceInterfaceResult struct {
	AccumulatedLatency   int    `json:"AccumulatedLatency"`
	DeviceId             int    `json:"DeviceId"`
	DeviceIp             string `json:"DeviceIp"`
	EgressInterfaceId    int    `json:"EgressInterfaceId"`
	EgressInterfaceName  string `json:"EgressInterfaceName"`
	FrameDuration        int    `json:"FrameDuration"`
	FrerType             string `json:"FrerType"`
	IngressInterfaceId   int    `json:"IngressInterfaceId"`
	IngressInterfaceName string `json:"IngressInterfaceName"`
	PriorityCodePoint    int    `json:"PriorityCodePoint"`
	QueueId              string `json:"QueueId"`
	StartTime            int    `json:"StartTime"`
	StopTime             int    `json:"StopTime"`
	StreamDuration       int    `json:"StreamDuration"`
	VlanId               int    `json:"VlanId"`
}
