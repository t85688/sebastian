package domain

type FeatureGroup struct {
	AutoScan      FeatureAutoScan      `json:"AutoScan"`
	Configuration FeatureConfiguration `json:"Configuration"`
	Monitor       FeatureMonitor       `json:"Monitor"`
	Operation     FeatureOperation     `json:"Operation"`
}

type FeatureAutoScan struct {
	BroadcastSearch   bool                     `json:"BroadcastSearch"`
	DeviceInformation FeatureDeviceInformation `json:"DeviceInformation"`
	Identify          FeatureIdentify          `json:"Identify"`
	LLDP              bool                     `json:"LLDP"`
}

type FeatureDeviceInformation struct {
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

type FeatureIdentify struct {
	FirmwareVersion bool `json:"FirmwareVersion"`
	ModelName       bool `json:"ModelName"`
	VendorID        bool `json:"VendorID"`
}

// Configuration 區塊

type FeatureConfiguration struct {
	CheckConfigSynchronization bool                        `json:"CheckConfigSynchronization"`
	InformationSetting         bool                        `json:"InformationSetting"`
	LoginPolicy                bool                        `json:"LoginPolicy"`
	LoopProtection             bool                        `json:"LoopProtection"`
	ManagementInterface        bool                        `json:"ManagementInterface"`
	NetworkSetting             bool                        `json:"NetworkSetting"`
	PortSetting                FeaturePortSetting          `json:"PortSetting"`
	SNMPTrapSetting            bool                        `json:"SNMPTrapSetting"`
	STPRSTP                    FeatureSTPRSTP              `json:"STPRSTP"`
	StaticForwardSetting       FeatureStaticForwardSetting `json:"StaticForwardSetting"`
	SyslogSetting              bool                        `json:"SyslogSetting"`
	TSN                        FeatureTSN                  `json:"TSN"`
	TimeSetting                FeatureTimeSetting          `json:"TimeSetting"`
	TimeSyncSetting            FeatureTimeSyncSetting      `json:"TimeSyncSetting"`
	UserAccount                bool                        `json:"UserAccount"`
	VLANSetting                FeatureVLANSetting          `json:"VLANSetting"`
}

type FeaturePortSetting struct {
	AdminStatus bool `json:"AdminStatus"`
}

type FeatureSTPRSTP struct {
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

type FeatureStaticForwardSetting struct {
	Multicast bool `json:"Multicast"`
	Unicast   bool `json:"Unicast"`
}

type FeatureTSN struct {
	IEEE802Dot1CB  bool `json:"IEEE802Dot1CB"`
	IEEE802Dot1Qbv bool `json:"IEEE802Dot1Qbv"`
}

type FeatureTimeSetting struct {
	PTP        bool `json:"PTP"`
	SystemTime bool `json:"SystemTime"`
}

type FeatureTimeSyncSetting struct {
	IEC61850_2016                     bool `json:"IEC61850_2016"`
	IEEE1588_2008                     bool `json:"IEEE1588_2008"`
	IEEE1588_2008_ClockMode           bool `json:"IEEE1588_2008_ClockMode"`
	IEEE1588_2008_ClockType           bool `json:"IEEE1588_2008_ClockType"`
	IEEE1588_2008_MaximumStepsRemoved bool `json:"IEEE1588_2008_MaximumStepsRemoved"`
	IEEE802Dot1AS_2011                bool `json:"IEEE802Dot1AS_2011"`
	IEEEC37Dot238_2017                bool `json:"IEEEC37Dot238_2017"`
}

type FeatureVLANSetting struct {
	AccessTrunkMode     bool `json:"AccessTrunkMode"`
	DefaultPCP          bool `json:"DefaultPCP"`
	DefaultPVID         bool `json:"DefaultPVID"`
	HybridMode          bool `json:"HybridMode"`
	ManagementVLAN      bool `json:"ManagementVLAN"`
	PerStreamPriority   bool `json:"PerStreamPriority"`
	PerStreamPriorityV2 bool `json:"PerStreamPriorityV2"`
	TEMSTID             bool `json:"TEMSTID"`
}

// Monitor 區塊

type FeatureMonitor struct {
	BasicStatus         FeatureBasicStatus         `json:"BasicStatus"`
	Redundancy          FeatureRedundancy          `json:"Redundancy"`
	TimeSynchronization FeatureTimeSynchronization `json:"TimeSynchronization"`
	Traffic             FeatureTraffic             `json:"Traffic"`
}

type FeatureBasicStatus struct {
	FiberCheck        bool `json:"FiberCheck"`
	PortStatus        bool `json:"PortStatus"`
	SystemUtilization bool `json:"SystemUtilization"`
}

type FeatureRedundancy struct {
	RSTP bool `json:"RSTP"`
}

type FeatureTimeSynchronization struct {
	IEEE1588_2008      bool `json:"IEEE1588_2008"`
	IEEE802Dot1AS_2011 bool `json:"IEEE802Dot1AS_2011"`
}

type FeatureTraffic struct {
	TrafficUtilization bool `json:"TrafficUtilization"`
	TxTotalOctets      bool `json:"TxTotalOctets"`
	TxTotalPackets     bool `json:"TxTotalPackets"`
}

// Operation 區塊

type FeatureOperation struct {
	CLI               bool `json:"CLI"`
	EnableSNMPService bool `json:"EnableSNMPService"`
	EventLog          bool `json:"EventLog"`
	FactoryDefault    bool `json:"FactoryDefault"`
	FirmwareUpgrade   bool `json:"FirmwareUpgrade"`
	ImportExport      bool `json:"ImportExport"`
	Locator           bool `json:"Locator"`
	Reboot            bool `json:"Reboot"`
}
