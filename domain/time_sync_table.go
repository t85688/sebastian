package domain

type TimeSyncProfile int

const (
	TimeSyncProfileIEEE8021AS_2011     TimeSyncProfile = 1
	TimeSyncProfileIEEE1588_2008       TimeSyncProfile = 3
	TimeSyncProfileIEC_61850_2016      TimeSyncProfile = 4
	TimeSyncProfileIEEE_C37Dot238_2017 TimeSyncProfile = 5
)

var TimeSyncProfileStrMap = map[TimeSyncProfile]string{
	TimeSyncProfileIEEE8021AS_2011:     "IEEE_802Dot1AS_2011",
	TimeSyncProfileIEEE1588_2008:       "IEEE_1588_2008",
	TimeSyncProfileIEC_61850_2016:      "IEC_61850_2016",
	TimeSyncProfileIEEE_C37Dot238_2017: "IEEE_C37Dot238_2017",
}

func (profile TimeSyncProfile) String() string {
	return TimeSyncProfileStrMap[profile]
}

type TimeSyncTable struct {
	DeviceId         int64                     `json:"DeviceId"`
	Enabled          bool                      `json:"Enabled"`
	Profile          string                    `json:"Profile"`
	IEEE1588_2008    *IEEE1588_2008_Profile    `json:"IEEE_1588_2008"`
	IEEE8021AS_2011  *IEEE8021AS_2011_Profile  `json:"IEEE_802Dot1AS_2011"`
	IEC61850_2016    *IEC61850_2016_Profile    `json:"IEC_61850_2016"`
	IEEEC37_238_2017 *IEEEC37_238_2017_Profile `json:"IEEE_C37Dot238_2017"`
}

// --- IEEE 1588-2008 ---
type IEEE1588_2008_Profile struct {
	AccuracyAlert       int                  `json:"AccuracyAlert"`
	ClockAccuracy       int                  `json:"ClockAccuracy"`
	ClockClass          int                  `json:"ClockClass"`
	ClockMode           string               `json:"ClockMode"`
	ClockType           string               `json:"ClockType"`
	DelayMechanism      string               `json:"DelayMechanism"`
	DomainNumber        int                  `json:"DomainNumber"`
	MaximumStepsRemoved int                  `json:"MaximumStepsRemoved"`
	PortEntries         []IEEE1588_PortEntry `json:"PortEntries"`
	Priority1           int                  `json:"Priority1"`
	Priority2           int                  `json:"Priority2"`
	TransportType       string               `json:"TransportType"`
}

type IEEE1588ClockType int

const (
	IEEE1588ClockTypeBoundaryClock    IEEE1588ClockType = 2
	IEEE1588ClockTypeTransparentClock IEEE1588ClockType = 3
)

var ieee1588ClockTypeStrMap = map[IEEE1588ClockType]string{
	IEEE1588ClockTypeBoundaryClock:    "BoundaryClock",
	IEEE1588ClockTypeTransparentClock: "TransparentClock",
}

func (clockType IEEE1588ClockType) String() string {
	return ieee1588ClockTypeStrMap[clockType]
}

type IEEE1588CDelayMechanism int

const (
	IEEE1588DelayMechanismEndToEnd   IEEE1588CDelayMechanism = 1
	IEEE1588DelayMechanismPeerToPeer IEEE1588CDelayMechanism = 2
)

var ieee1588DelayMechanismStrMap = map[IEEE1588CDelayMechanism]string{
	IEEE1588DelayMechanismEndToEnd:   "EndToEnd",
	IEEE1588DelayMechanismPeerToPeer: "PeerToPeer",
}

func (delayMech IEEE1588CDelayMechanism) String() string {
	return ieee1588DelayMechanismStrMap[delayMech]
}

type IEEE1588TransportType int

const (
	IEEE1588TransportTypeUDPIPv4             IEEE1588TransportType = 1
	IEEE1588TransportTypeUDPIPv6             IEEE1588TransportType = 2
	IEEE1588TransportTypeIEEE802Dot3Ethernet IEEE1588TransportType = 3
)

var ieee1588TransportTypeStrMap = map[IEEE1588TransportType]string{
	IEEE1588TransportTypeUDPIPv4:             "UDPIPv4",
	IEEE1588TransportTypeUDPIPv6:             "UDPIPv6",
	IEEE1588TransportTypeIEEE802Dot3Ethernet: "IEEE802Dot3Ethernet",
}

func (transportType IEEE1588TransportType) String() string {
	return ieee1588TransportTypeStrMap[transportType]
}

type IEEE1588ClockMode int

const (
	IEEE1588ClockModeOneStep IEEE1588ClockMode = 1
	IEEE1588ClockModeTwoStep IEEE1588ClockMode = 2
)

var ieee1588ClockModeStrMap = map[IEEE1588ClockMode]string{
	IEEE1588ClockModeOneStep: "OneStep",
	IEEE1588ClockModeTwoStep: "TwoStep",
}

func (clockMode IEEE1588ClockMode) String() string {
	return ieee1588ClockModeStrMap[clockMode]
}

func NewIEEE1588_2008_Profile() *IEEE1588_2008_Profile {
	return &IEEE1588_2008_Profile{
		ClockType:           IEEE1588ClockTypeBoundaryClock.String(),
		DelayMechanism:      IEEE1588DelayMechanismEndToEnd.String(),
		TransportType:       IEEE1588TransportTypeIEEE802Dot3Ethernet.String(),
		ClockMode:           IEEE1588ClockModeTwoStep.String(),
		PortEntries:         []IEEE1588_PortEntry{},
		Priority1:           128,
		Priority2:           128,
		DomainNumber:        0,
		AccuracyAlert:       1000,
		MaximumStepsRemoved: 255,
		ClockClass:          248,
		ClockAccuracy:       254,
	}
}

type IEEE1588_PortEntry struct {
	AnnounceInterval       int  `json:"AnnounceInterval"`
	AnnounceReceiptTimeout int  `json:"AnnounceReceiptTimeout"`
	DelayReqInterval       int  `json:"DelayReqInterval"`
	Enable                 bool `json:"Enable"`
	PdelayReqInterval      int  `json:"PdelayReqInterval"`
	PortId                 int  `json:"PortId"`
	SyncInterval           int  `json:"SyncInterval"`
}

func NewIEEE1588_PortEntry() *IEEE1588_PortEntry {
	return &IEEE1588_PortEntry{
		PortId:                 -1,
		Enable:                 true,
		AnnounceInterval:       1,
		AnnounceReceiptTimeout: 3,
		SyncInterval:           0,
		DelayReqInterval:       0,
		PdelayReqInterval:      0,
	}
}

// --- IEEE 802.1AS-2011 ---
type IEEE8021AS_2011_Profile struct {
	AccuracyAlert int                    `json:"AccuracyAlert"`
	ClockAccuracy int                    `json:"ClockAccuracy"`
	ClockClass    int                    `json:"ClockClass"`
	Priority1     int                    `json:"Priority1"`
	Priority2     int                    `json:"Priority2"`
	PortEntries   []IEEE8021AS_PortEntry `json:"PortEntries"`
}

func NewIEEE8021AS_2011_Profile() *IEEE8021AS_2011_Profile {
	return &IEEE8021AS_2011_Profile{
		Priority1:     246,
		Priority2:     248,
		ClockClass:    248,
		ClockAccuracy: 254,
		AccuracyAlert: 500,
		PortEntries:   []IEEE8021AS_PortEntry{},
	}
}

type IEEE8021AS_PortEntry struct {
	AnnounceInterval        int  `json:"AnnounceInterval"`
	AnnounceReceiptTimeout  int  `json:"AnnounceReceiptTimeout"`
	Enable                  bool `json:"Enable"`
	NeighborPropDelayThresh int  `json:"NeighborPropDelayThresh"`
	PdelayReqInterval       int  `json:"PdelayReqInterval"`
	PortId                  int  `json:"PortId"`
	SyncInterval            int  `json:"SyncInterval"`
	SyncReceiptTimeout      int  `json:"SyncReceiptTimeout"`
}

func NewIEEE8021AS_PortEntry() *IEEE8021AS_PortEntry {
	return &IEEE8021AS_PortEntry{
		PortId:                  -1,
		Enable:                  true,
		AnnounceInterval:        0,
		AnnounceReceiptTimeout:  3,
		SyncInterval:            -3,
		SyncReceiptTimeout:      3,
		PdelayReqInterval:       0,
		NeighborPropDelayThresh: 800,
	}
}

// --- IEC 61850-2016 ---
type IEC61850_2016_Profile struct {
	AccuracyAlert       int    `json:"AccuracyAlert"`
	ClockAccuracy       int    `json:"ClockAccuracy"`
	ClockClass          int    `json:"ClockClass"`
	ClockMode           string `json:"ClockMode"`
	ClockType           string `json:"ClockType"`
	DelayMechanism      string `json:"DelayMechanism"`
	DomainNumber        int    `json:"DomainNumber"`
	MaximumStepsRemoved int    `json:"MaximumStepsRemoved"`
	PortEntries         []any  `json:"PortEntries"` // 若未使用可為空介面
	Priority1           int    `json:"Priority1"`
	Priority2           int    `json:"Priority2"`
	TransportType       string `json:"TransportType"`
}

// --- IEEE C37.238-2017 ---
type IEEEC37_238_2017_Profile struct {
	AccuracyAlert       int    `json:"AccuracyAlert"`
	ClockAccuracy       int    `json:"ClockAccuracy"`
	ClockClass          int    `json:"ClockClass"`
	ClockMode           string `json:"ClockMode"`
	ClockType           string `json:"ClockType"`
	DelayMechanism      string `json:"DelayMechanism"`
	DomainNumber        int    `json:"DomainNumber"`
	GrandmasterId       int    `json:"GrandmasterId"`
	MaximumStepsRemoved int    `json:"MaximumStepsRemoved"`
	PortEntries         []any  `json:"PortEntries"` // 空陣列處理
	Priority1           int    `json:"Priority1"`
	Priority2           int    `json:"Priority2"`
	TransportType       string `json:"TransportType"`
}
