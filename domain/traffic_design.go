package domain

type TrafficDesign struct {
	ApplicationSetting              []ApplicationSetting       `json:"ApplicationSetting"`
	Mode                            string                     `json:"Mode"`
	PerStreamPrioritySetting        []PerStreamPrioritySetting `json:"PerStreamPrioritySetting"`
	StreamSetting                   []StreamSetting            `json:"StreamSetting"`
	TimeSlotSetting                 []TimeSlotSetting          `json:"TimeSlotSetting"`
	TrafficTypeConfigurationSetting []TrafficTypeConfiguration `json:"TrafficTypeConfigurationSetting"`
}

type ApplicationSetting struct {
	ApplicationName string          `json:"ApplicationName"`
	Id              int             `json:"Id"`
	StreamParameter StreamParameter `json:"StreamParameter"`
	TSN             TSN             `json:"TSN"`
	TrafficSetting  TrafficSetting  `json:"TrafficSetting"`
	VlanSetting     VlanSetting     `json:"VlanSetting"`
}

type StreamParameter struct {
	EarliestTransmitOffset float64 `json:"EarliestTransmitOffset"`
	Interval               float64 `json:"Interval"`
	Jitter                 float64 `json:"Jitter"`
	LatestTransmitOffset   float64 `json:"LatestTransmitOffset"`
	MaxBytesPerInterval    int     `json:"MaxBytesPerInterval"`
	MaxFrameSize           int     `json:"MaxFrameSize"`
	MaxFramesPerInterval   int     `json:"MaxFramesPerInterval"`
}

type TSN struct {
	Active bool `json:"Active"`
	FRER   bool `json:"FRER"`
}

type TrafficSetting struct {
	MaxReceiveOffset float64 `json:"MaxReceiveOffset"`
	MinReceiveOffset float64 `json:"MinReceiveOffset"`
	QosType          string  `json:"QosType"`
	TrafficTypeClass int     `json:"TrafficTypeClass"`
}

type VlanSetting struct {
	EnableSubType     bool     `json:"EnableSubType"`
	EtherType         int      `json:"EtherType"`
	PriorityCodePoint int      `json:"PriorityCodePoint"`
	SubType           int      `json:"SubType"`
	Tagged            bool     `json:"Tagged"`
	TcpPort           int      `json:"TcpPort"`
	Type              []string `json:"Type"`
	UdpPort           int      `json:"UdpPort"`
	UntaggedMode      string   `json:"UntaggedMode"`
	UserDefinedVlan   bool     `json:"UserDefinedVlan"`
	VlanId            int      `json:"VlanId"`
}

type PerStreamPrioritySetting struct {
	EnableSubType     bool       `json:"EnableSubType"`
	EtherType         int        `json:"EtherType"`
	Id                int        `json:"Id"`
	PortList          []PortList `json:"PortList"`
	PriorityCodePoint int        `json:"PriorityCodePoint"`
	SubType           int        `json:"SubType"`
	TcpPort           int        `json:"TcpPort"`
	Type              []string   `json:"Type"`
	UdpPort           int        `json:"UdpPort"`
	VlanId            int        `json:"VlanId"`
}

type PortList struct {
	DeviceId int   `json:"DeviceId"`
	Ports    []int `json:"Ports"`
}

type StreamSetting struct {
	Active         bool       `json:"Active"`
	ApplicationId  int        `json:"ApplicationId"`
	DestinationMac string     `json:"DestinationMac"`
	Id             int        `json:"Id"`
	Listeners      []Listener `json:"Listeners"`
	Multicast      bool       `json:"Multicast"`
	StreamName     string     `json:"StreamName"`
	Talker         Talker     `json:"Talker"`
}

type Listener struct {
	DeviceId    int `json:"DeviceId"`
	InterfaceId int `json:"InterfaceId"`
}

type Talker struct {
	DeviceId    int `json:"DeviceId"`
	InterfaceId int `json:"InterfaceId"`
}

type TimeSlotSetting struct {
	GateControl []GateControl `json:"GateControl"`
	Id          int           `json:"Id"`
	PortList    []PortList    `json:"PortList"`
}

type GateControl struct {
	Interval int   `json:"Interval"`
	QueueSet []int `json:"QueueSet"`
}

type TrafficTypeConfiguration struct {
	PriorityCodePointSet []int   `json:"PriorityCodePointSet"`
	ReservedTime         float64 `json:"ReservedTime"`
	TrafficClass         int     `json:"TrafficClass"`
	TrafficType          string  `json:"TrafficType"`
}
