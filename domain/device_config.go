package domain

type DeviceConfig struct {
	CBTables                     map[int64]CbTable                    `json:"CbTables"`
	GCLTables                    map[int64]GCLTable                   `json:"GCLTables"`
	InformationSettingTables     map[int64]InformationSettingTable    `json:"InformationSettingTables"`
	LoginPolicyTables            map[int64]LoginPolicyTable           `json:"LoginPolicyTables"`
	LoopProtectionTables         map[int64]LoopProtection             `json:"LoopProtectionTables"`
	ManagementInterfaceTables    map[int64]ManagementInterfaceTable   `json:"ManagementInterfaceTables"`
	DeviceIpSettingTables        map[int64]DeviceIpSettingTable       `json:"MappingDeviceIpSettingTables"`
	MulticastStaticForwardTables map[int64]StaticForwardTable         `json:"MulticastStaticForwardTables"`
	NetworkSettingTables         map[int64]NetworkSettingTable        `json:"NetworkSettingTables"`
	PortDefaultPCPTables         map[int64]PortDefaultPCPTable        `json:"PortDefaultPCPTables"`
	PortSettingTables            map[int64]PortSettingTable           `json:"PortSettingTables"`
	RstpTables                   map[int64]RstpTable                  `json:"RstpTables"`
	SnmpTrapSettingTables        map[int64]SnmpTrapSettingTable       `json:"SnmpTrapSettingTables"`
	StreamPriorityEgressTables   map[int64]StreamPriorityEgressTable  `json:"StreamPriorityEgressTables"`
	StreamPriorityIngressTables  map[int64]StreamPriorityIngressTable `json:"StreamPriorityIngressTables"`
	SyslogSettingTables          map[int64]SyslogSettingTable         `json:"SyslogSettingTables"`
	TimeSettingTables            map[int64]TimeSettingTable           `json:"TimeSettingTables"`
	TimeSyncTables               map[int64]TimeSyncTable              `json:"TimeSyncTables"`
	UnicastStaticForwardTables   map[int64]StaticForwardTable         `json:"UnicastStaticForwardTables"`
	UserAccountTables            map[int64]UserAccountTable           `json:"UserAccountTables"`
	VlanTables                   map[int64]VlanTable                  `json:"VlanTables"`
}

type SyslogSettingTable struct {
	DeviceId      int64  `json:"DeviceId"`
	Enabled       bool   `json:"Enabled"`
	SyslogServer1 bool   `json:"SyslogServer1"`
	Address1      string `json:"Address1"`
	Port1         int    `json:"Port1"`
	SyslogServer2 bool   `json:"SyslogServer2"`
	Address2      string `json:"Address2"`
	Port2         int    `json:"Port2"`
	SyslogServer3 bool   `json:"SyslogServer3"`
	Address3      string `json:"Address3"`
	Port3         int    `json:"Port3"`
}

type InformationSettingTable struct {
	DeviceId           int64  `json:"DeviceId"`
	DeviceName         string `json:"DeviceName"`
	Location           string `json:"Location"`
	Description        string `json:"Description"`
	ContactInformation string `json:"ContactInformation"`
}

type LoginPolicyTable struct {
	DeviceId                          int64  `json:"DeviceId"`
	LoginMessage                      string `json:"LoginMessage"`
	LoginAuthenticationFailureMessage string `json:"LoginAuthenticationFailureMessage"`
	LoginFailureLockout               bool   `json:"LoginFailureLockout"`
	RetryFailureThreshold             int    `json:"RetryFailureThreshold"`
	LockoutDuration                   int    `json:"LockoutDuration"`
	AutoLogoutAfter                   int    `json:"AutoLogoutAfter"`
}

type SnmpTrapSettingTable struct {
	DeviceId int64                  `json:"DeviceId"`
	HostList []SnmpTrapSettingEntry `json:"HostList"`
}

type SnmpTrapSettingEntry struct {
	HostName      string `json:"HostName"`
	Mode          string `json:"Mode"`
	TrapCommunity string `json:"TrapCommunity"`
}

type TimeSettingTable struct {
	DeviceId           int64          `json:"DeviceId"`
	ClockSource        string         `json:"ClockSource" validate:"omitempty,oneof=Local SNTP NTP PTP"`
	DaylightSavingTime bool           `json:"DaylightSavingTime"`
	NTPTimeServer1     string         `json:"NTPTimeServer1"`
	NTPTimeServer2     string         `json:"NTPTimeServer2"`
	Offset             string         `json:"Offset"`
	SNTPTimeServer1    string         `json:"SNTPTimeServer1"`
	SNTPTimeServer2    string         `json:"SNTPTimeServer2"`
	TimeZone           string         `json:"TimeZone"`
	Start              TransitionTime `json:"Start"`
	End                TransitionTime `json:"End"`
}

type TransitionTime struct {
	Day    int `json:"Day"`
	Hour   int `json:"Hour"`
	Minute int `json:"Minute"`
	Month  int `json:"Month"`
	Week   int `json:"Week"`
}

type ManagementInterfaceTable struct {
	DeviceId             int64 `json:"DeviceId"`
	EncryptedMoxaService struct {
		Enable bool `json:"Enable"`
	} `json:"EncryptedMoxaService"`
	HttpMaxLoginSessions int `json:"HttpMaxLoginSessions"`
	HttpService          struct {
		Enable bool `json:"Enable"`
		Port   int  `json:"Port"`
	} `json:"HttpService"`
	HttpsService struct {
		Enable bool `json:"Enable"`
		Port   int  `json:"Port"`
	} `json:"HttpsService"`
	SSHService struct {
		Enable bool `json:"Enable"`
		Port   int  `json:"Port"`
	} `json:"SSHService"`
	SnmpService struct {
		Mode                   string `json:"Mode"` // "Enabled"
		Port                   int    `json:"Port"`
		TransportLayerProtocol string `json:"TransportLayerProtocol"`
	} `json:"SnmpService"`
	TelnetService struct {
		Enable bool `json:"Enable"`
		Port   int  `json:"Port"`
	} `json:"TelnetService"`
	TerminalMaxLoginSessions int `json:"TerminalMaxLoginSessions"`
}

type TransportLayerProtocol int

const (
	TransportLayerProtocolUDP TransportLayerProtocol = 1
	TransportLayerProtocolTCP TransportLayerProtocol = 2
)

var transportLayerProtocolStrMap = map[TransportLayerProtocol]string{
	TransportLayerProtocolUDP: "UDP",
	TransportLayerProtocolTCP: "TCP",
}

func (p TransportLayerProtocol) String() string {
	if str, ok := transportLayerProtocolStrMap[p]; ok {
		return str
	}

	return ""
}

type LoopProtection struct {
	DetectInterval        int   `json:"DetectInterval"`
	DeviceId              int64 `json:"DeviceId"`
	NetworkLoopProtection bool  `json:"NetworkLoopProtection"`
}

// CbTable holds FRER and StreamIdentityList
type CbTable struct {
	DeviceId           int64                 `json:"DeviceId"`
	FRER               FRERTable             `json:"FRER"`
	StreamIdentityList []StreamIdentityEntry `json:"StreamIdentityList"`
}

type StreamIdentityEntry struct {
	Index                 int                    `json:"Index"`
	TsnStreamIdEntryGroup *TsnStreamIdEntryGroup `json:"TsnStreamIdEntryGroup"`
}

type TsnStreamIdEntryGroup struct {
	Handle                               int                                  `json:"Handle"`
	ActInFacing                          ActInFacing                          `json:"ActInFacing"`
	ActOutFacing                         ActOutFacing                         `json:"ActOutFacing"`
	ActIdentificationType                ActIdentificationType                `json:"ActIdentificationType"`
	ActNullStreamIdentificationGroup     ActNullStreamIdentificationGroup     `json:"ActNullStreamIdentificationGroup"`
	ActSmacVlanStreamIdentificationGroup ActSmacVlanStreamIdentificationGroup `json:"ActSmacVlanStreamIdentificationGroup"`
	ActDmacVlanStreamIdentificationGroup ActDmacVlanStreamIdentificationGroup `json:"ActDmacVlanStreamIdentificationGroup"`
	ActIpStreamIdentificationGroup       ActIpStreamIdentificationGroup       `json:"ActIpStreamIdentificationGroup"`
}

type ActInFacing struct {
	InputPortList  []string `json:"InputPortList"`
	OutputPortList []string `json:"OutputPortList"`
}

type ActOutFacing struct {
	InputPortList  []string `json:"InputPortList"`
	OutputPortList []string `json:"OutputPortList"`
}

type ActIdentificationType int

const (
	ActIdentificationTypeNullStreamIdentification     ActIdentificationType = 1
	ActIdentificationTypeSmacVlanStreamIdentification ActIdentificationType = 2
	ActIdentificationTypeDmacVlanStreamIdentification ActIdentificationType = 3
	ActIdentificationTypeIpStreamIdentification       ActIdentificationType = 4
)

var actIdentificationTypeStrMap = map[ActIdentificationType]string{
	ActIdentificationTypeNullStreamIdentification:     "dot1cb-stream-identification-types:null-stream-identification",
	ActIdentificationTypeSmacVlanStreamIdentification: "dot1cb-stream-identification-types:smac-vlan-stream-identification",
	ActIdentificationTypeDmacVlanStreamIdentification: "dot1cb-stream-identification-types:dmac-vlan-stream-identification",
	ActIdentificationTypeIpStreamIdentification:       "dot1cb-stream-identification-types:ip-stream-identification",
}

func (t ActIdentificationType) String() string {
	if str, ok := actIdentificationTypeStrMap[t]; ok {
		return str
	}

	return ""
}

type ActNullStreamIdentificationGroup struct {
	DestinationMac string                    `json:"DestinationMac"`
	Tagged         VLANTagIdentificationType `json:"Tagged"`
	VlanId         int                       `json:"Vlan"`
}

type VLANTagIdentificationType int

const (
	VLANTagIdentificationTypeNone     VLANTagIdentificationType = 0
	VLANTagIdentificationTypeTagged   VLANTagIdentificationType = 1
	VLANTagIdentificationTypePriority VLANTagIdentificationType = 2
	VLANTagIdentificationTypeAll      VLANTagIdentificationType = 3
)

var vlanTagIdentificationTypeStrMap = map[VLANTagIdentificationType]string{
	VLANTagIdentificationTypeNone:     "None",
	VLANTagIdentificationTypeTagged:   "Tagged",
	VLANTagIdentificationTypePriority: "Priority",
	VLANTagIdentificationTypeAll:      "All",
}

func (t VLANTagIdentificationType) String() string {
	if str, ok := vlanTagIdentificationTypeStrMap[t]; ok {
		return str
	}

	return ""
}

type ActSmacVlanStreamIdentificationGroup struct {
	SourceMac string                    `json:"SourceMac"`
	Tagged    VLANTagIdentificationType `json:"Tagged"`
	VlanId    int                       `json:"Vlan"`
}

type ActDmacVlanStreamIdentificationGroup struct {
}

type Down struct {
	DestinationMac string                    `json:"DestinationMac"`
	Tagged         VLANTagIdentificationType `json:"Tagged"`
	VlanId         int                       `json:"Vlan"`
	Priority       int8                      `json:"Priority"`
}

type Up struct {
	DestinationMac string                    `json:"DestinationMac"`
	Tagged         VLANTagIdentificationType `json:"Tagged"`
	VlanId         int                       `json:"Vlan"`
	Priority       int8                      `json:"Priority"`
}

type ActIpStreamIdentificationGroup struct {
	DestinationMac  string                    `json:"DestinationMac"`
	Tagged          VLANTagIdentificationType `json:"Tagged"`
	VlanId          int                       `json:"Vlan"`
	IpSource        string                    `json:"IpSource"`
	IpDestination   string                    `json:"IpDestination"`
	Dscp            uint8                     `json:"Dscp"`
	NextProtocol    NextProtocol              `json:"NextProtocol"`
	SourcePort      int                       `json:"SourcePort"`
	DestinationPort int                       `json:"DestinationPort"`
}

type NextProtocol int

const (
	NextProtocolNone NextProtocol = 0
	NextProtocolUdp  NextProtocol = 1
	NextProtocolTcp  NextProtocol = 2
	NextProtocolSctp NextProtocol = 3
)

var nextProtocolStrMap = map[NextProtocol]string{
	NextProtocolNone: "none",
	NextProtocolUdp:  "udp",
	NextProtocolTcp:  "tcp",
	NextProtocolSctp: "sctp",
}

func (p NextProtocol) String() string {
	if str, ok := nextProtocolStrMap[p]; ok {
		return str
	}
	return ""
}

type FRERTable struct {
	SequenceGenerationLists     []SequenceGenerationList     `json:"SequenceGenerationLists"`
	SequenceIdentificationLists []SequenceIdentificationList `json:"SequenceIdentificationLists"`
	SequenceRecoveryLists       []SequenceRecoveryList       `json:"SequenceRecoveryLists"`
}

type SequenceIdentificationList struct {
	SequenceIdentificationEntry SequenceIdentificationEntry `json:"SequenceIdentificationEntry"`
}

type SequenceIdentificationEntry struct {
	StreamList    []int  `json:"StreamList"`
	Port          string `json:"Port"` // interface name
	Direction     bool   `json:"Direction"`
	Active        bool   `json:"Active"`
	Encapsulation string `json:"Encapsulation"`
	PathIdLanId   int8   `json:"PathIdLanId"`
}

type SequenceEncodeDecodeType int

const (
	SequenceEncodeDecodeTypeRTag               SequenceEncodeDecodeType = 1
	SequenceEncodeDecodeTypeHSRSequenceTag     SequenceEncodeDecodeType = 2
	SequenceEncodeDecodeTypePRPSequenceTrailer SequenceEncodeDecodeType = 3
)

var sequenceEncodeDecodeTypeStrMap = map[SequenceEncodeDecodeType]string{
	SequenceEncodeDecodeTypeRTag:               "r-tag",
	SequenceEncodeDecodeTypeHSRSequenceTag:     "hsr-sequence-tag",
	SequenceEncodeDecodeTypePRPSequenceTrailer: "prp-sequence-trailer",
}

func (t SequenceEncodeDecodeType) String() string {
	if str, ok := sequenceEncodeDecodeTypeStrMap[t]; ok {
		return str
	}

	return ""
}

type SequenceRecoveryList struct {
	Index                 int                   `json:"Index"`
	SequenceRecoveryEntry SequenceRecoveryEntry `json:"SequenceRecoveryEntry"`
}

type SequenceRecoveryEntry struct {
	StreamList                     []int                          `json:"StreamList"`
	PortList                       []string                       `json:"PortList"`
	Direction                      bool                           `json:"Direction"`
	Reset                          bool                           `json:"Reset"`
	Algorithm                      string                         `json:"Algorithm"`
	HistoryLength                  int                            `json:"HistoryLength"`
	ResetTimeout                   int64                          `json:"ResetTimeout"`
	InvalidSequenceValue           int                            `json:"InvalidSequenceValue"`
	TakeNoSequence                 bool                           `json:"TakeNoSequence"`
	IndividualRecovery             bool                           `json:"IndividualRecovery"`
	LatentErrorDetection           bool                           `json:"LatentErrorDetection"`
	LatentErrorDetectionParameters LatentErrorDetectionParameters `json:"LatentErrorDetectionParameters"`
}

type LatentErrorDetectionParameters struct {
	Difference  int   `json:"Difference"`
	Period      int   `json:"Period"` // default "2000"
	Paths       int   `json:"Paths"`
	ResetPeriod int64 `json:"ResetPeriod"` // default "30000"
}

type SequenceRecoveryAlgorithm int

const (
	SequenceRecoveryAlgorithmVector SequenceRecoveryAlgorithm = 0
	SequenceRecoveryAlgorithmMatch  SequenceRecoveryAlgorithm = 1
)

var sequenceRecoveryAlgorithmStrMap = map[SequenceRecoveryAlgorithm]string{
	SequenceRecoveryAlgorithmVector: "vector",
	SequenceRecoveryAlgorithmMatch:  "match",
}

func (t SequenceRecoveryAlgorithm) String() string {
	if str, ok := sequenceRecoveryAlgorithmStrMap[t]; ok {
		return str
	}
	return ""
}

type SequenceGenerationList struct {
	Index                   int                     `json:"Index"`
	SequenceGenerationEntry SequenceGenerationEntry `json:"SequenceGenerationEntry"`
}

type SequenceGenerationEntry struct {
	StreamList []int `json:"StreamList"`
	Direction  bool  `json:"Direction"`
	Reset      bool  `json:"Reset"`
}

type GCLTable struct {
	DeviceId                 int64                    `json:"DeviceId"`
	InterfacesGateParameters []InterfaceGateParameter `json:"InterfacesGateParameters"`
}

type InterfaceGateParameter struct {
	InterfaceId    int64          `json:"InterfaceId"`
	GateParameters GateParameters `json:"GateParameters"`
}

type GateParameters struct {
	AdminBaseTime           BaseTime           `json:"AdminBaseTime"`
	AdminControlList        []AdminControlList `json:"AdminControlList"`
	AdminControlListLength  int                `json:"AdminControlListLength"`
	AdminCycleTime          TimeRatio          `json:"AdminCycleTime"`
	AdminCycleTimeExtension int64              `json:"AdminCycleTimeExtension"`
	AdminGateStates         int                `json:"AdminGateStates"`
	ConfigChange            bool               `json:"ConfigChange"`
	GateEnabled             bool               `json:"GateEnabled"`
}

type AdminControlList struct {
	Index         int       `json:"Index"`
	OperationName string    `json:"OperationName"`
	SgsParams     SgsParams `json:"SgsParams"`
}

type SgsParams struct {
	GateStatesValue   int `json:"GateStatesValue"`
	TimeIntervalValue int `json:"TimeIntervalValue"`
}

// Time structures
type BaseTime struct {
	FractionalSeconds int64 `json:"FractionalSeconds"`
	Second            int64 `json:"Second"`
}

type TimeRatio struct {
	Denominator int64 `json:"Denominator"`
	Numerator   int64 `json:"Numerator"`
}

type StaticForwardTable struct {
	DeviceId             int64                `json:"DeviceId"`
	StaticForwardEntries []StaticForwardEntry `json:"StaticForwardEntries"`
}

type StaticForwardEntry struct {
	VlanId               int    `json:"VlanId"`
	MAC                  string `json:"MAC"`
	EgressPorts          []int  `json:"EgressPorts"`
	Dot1qStatus          int    `json:"Dot1qStatus"`
	ForbiddenEgressPorts []int  `json:"ForbiddenEgressPorts"`
}

type NetworkSettingTable struct {
	DeviceId           int64  `json:"DeviceId"`
	IpAddress          string `json:"IpAddress"`
	SubnetMask         string `json:"SubnetMask"`
	Gateway            string `json:"Gateway"`
	DNS1               string `json:"DNS1"`
	DNS2               string `json:"DNS2"`
	NetworkSettingMode string `json:"NetworkSettingMode"`
}

type NetworkSettingMode int

const (
	NetworkSettingModeUnknown NetworkSettingMode = -1
	NetworkSettingModeStatic  NetworkSettingMode = 0
	NetworkSettingModeDHCP    NetworkSettingMode = 1
	NetworkSettingModeBootp   NetworkSettingMode = 2
)

var networkSettingModeStrMap = map[NetworkSettingMode]string{
	NetworkSettingModeStatic: "Static",
	NetworkSettingModeDHCP:   "DHCP",
	NetworkSettingModeBootp:  "Bootp",
}

func (mode NetworkSettingMode) Int() int {
	return int(mode)
}

func (mode NetworkSettingMode) String() string {
	if str, ok := networkSettingModeStrMap[mode]; ok {
		return str
	}

	return ""
}

type DeviceIpSettingTable struct {
	DeviceId   int64  `json:"DeviceId"`
	MacAddress string `json:"MacAddress"`
	OfflineIP  string `json:"OfflineIP"`
	OnlineIP   string `json:"OnlineIP"`
	SubnetMask string `json:"SubnetMask"`
	Gateway    string `json:"Gateway"`
	DNS1       string `json:"DNS1"`
	DNS2       string `json:"DNS2"`
}

type PortDefaultPCPTable struct {
	DeviceId               int64                  `json:"DeviceId"`
	DefaultPriorityEntries []DefaultPriorityEntry `json:"DefaultPriorityEntries"`
}

type DefaultPriorityEntry struct {
	PortId     int64 `json:"PortId"`
	DefaultPCP int   `json:"DefaultPCP"`
}

type PortSettingTable struct {
	DeviceId           int64              `json:"DeviceId"`
	PortSettingEntries []PortSettingEntry `json:"PortSettingEntries"`
}

type PortSettingEntry struct {
	PortId      int64 `json:"PortId"`
	AdminStatus bool  `json:"AdminStatus"`
}

type RstpTable struct {
	DeviceId              int64           `json:"DeviceId"`
	Active                bool            `json:"Active"`
	ForwardDelay          int             `json:"ForwardDelay"`
	HelloTime             int             `json:"HelloTime"`
	MaxAge                int             `json:"MaxAge"`
	Priority              int             `json:"Priority"`
	RstpConfigRevert      bool            `json:"RstpConfigRevert"`
	RstpConfigSwift       bool            `json:"RstpConfigSwift"`
	RstpErrorRecoveryTime int             `json:"RstpErrorRecoveryTime"`
	RstpPortEntries       []RstpPortEntry `json:"RstpPortEntries"`
	SpanningTreeVersion   string          `json:"SpanningTreeVersion"`
}

type RstpPortEntry struct {
	PortId       int64  `json:"PortId"`
	PortPriority int    `json:"PortPriority"`
	PathCost     int    `json:"PathCost"`
	RstpEnable   bool   `json:"RstpEnable"`
	BpduFilter   bool   `json:"BpduFilter"`
	BpduGuard    bool   `json:"BpduGuard"`
	LoopGuard    bool   `json:"LoopGuard"`
	RootGuard    bool   `json:"RootGuard"`
	Edge         string `json:"Edge"`
	LinkType     string `json:"LinkType"`
}

type StreamPriorityEgressTable struct {
	DeviceId          int64             `json:"DeviceId"`
	StadConfigEntries []StadConfigEntry `json:"StadConfigEntries"`
}

type StadConfigEntry struct {
	PortId      int64 `json:"PortId"`
	EgressUntag int   `json:"EgressUntag"`
}

const (
	EgressUntagTrue  = 1
	EgressUntagFalse = 2
)

type StreamPriorityIngressTable struct {
	DeviceId                 int64                    `json:"DeviceId"`
	InterfaceStadPortEntries []InterfaceStadPortEntry `json:"InterfaceStadPortEntries"`
}

type InterfaceStadPortEntry struct {
	InterfaceId     int64           `json:"InterfaceId"`
	StadPortEntries []StadPortEntry `json:"StadPortEntries"`
}

type StadPortEntry struct {
	PortId        int64    `json:"PortId"`         // The Port ID(Port Index)
	IngressIndex  int      `json:"IngressIndex"`   // The Ingress Index (0-9) (config index)
	Enable        int      `json:"IndexEnable"`    // The Index Enable (INTEGER(true(1), false(2))) (enable)
	VlanId        int      `json:"VlanId"`         // The Vlan ID (2-4094) (vid)
	VlanPcp       int      `json:"VlanPcp"`        // The Vlan PCP (0-7) (pcp)
	Ethertype     int      `json:"EthertypeValue"` // The Ethertype Value (ethertype)
	SubtypeEnable int      `json:"SubtypeEnable"`  // The Subtype Enable (frametype) (INTEGER(true(1), false(2))) (enable)
	Subtype       int      `json:"SubtypeValue"`   // The Subtype Value (frametypevalue)
	Type          []string `json:"Type"`           // The Types
	UdpPort       int      `json:"UdpPort"`        // The UdpPort
	TcpPort       int      `json:"TcpPort"`        // The TcpPort
}

func NewStadPortEntry() *StadPortEntry {
	return &StadPortEntry{
		PortId:        -1,
		IngressIndex:  0,
		Enable:        IngressSubTypeDisabled,
		VlanId:        1,
		VlanPcp:       0,
		Ethertype:     0,
		SubtypeEnable: IngressSubTypeDisabled,
		Subtype:       0,
		Type:          []string{StreamPriorityTypeEthertype.String()},
		UdpPort:       1,
		TcpPort:       1,
	}
}

type StreamPriorityType int

const (
	StreamPriorityTypeInactive StreamPriorityType = iota
	StreamPriorityTypeTcp
	StreamPriorityTypeEthertype
	StreamPriorityTypeUdp
)

var streamPriorityTypeStrMap = map[StreamPriorityType]string{
	StreamPriorityTypeInactive:  "inactive",
	StreamPriorityTypeEthertype: "etherType",
	StreamPriorityTypeTcp:       "tcpPort",
	StreamPriorityTypeUdp:       "udpPort",
}

func (t StreamPriorityType) String() string {
	if str, ok := streamPriorityTypeStrMap[t]; ok {
		return str
	}
	return ""
}

const (
	IngressEnabled  = 1
	IngressDisabled = 2
)

const (
	IngressSubTypeEnabled  = 1
	IngressSubTypeDisabled = 2
)

type UserAccountTable struct {
	DeviceId              int64                  `json:"DeviceId"`
	SyncConnectionAccount string                 `json:"SyncConnectionAccount"`
	Accounts              map[string]UserAccount `json:"Accounts"`
}

type UserAccount struct {
	Username string `json:"Username"`
	Password string `json:"Password"`
	Role     string `json:"Role"`
	Email    string `json:"Email"`
	Active   bool   `json:"Active"`
}

type VlanTable struct {
	DeviceId            int64               `json:"DeviceId"`
	ManagementVlan      int                 `json:"ManagementVlan"`
	PortVlanEntries     []PortVlanEntry     `json:"PortVlanEntries"`
	VlanPortTypeEntries []VlanPortTypeEntry `json:"VlanPortTypeEntries"`
	VlanStaticEntries   []VlanStaticEntry   `json:"VlanStaticEntries"`
}

type PortVlanEntry struct {
	PortId       int64  `json:"PortId"`
	PVID         int    `json:"PVID"`
	VlanPriority string `json:"VlanPriority"`
}

type VlanPortTypeEntry struct {
	PortId       int64  `json:"PortId"`
	VlanPortType string `json:"VlanPortType"`
	VlanPriority string `json:"VlanPriority"`
}

type VlanStaticEntry struct {
	VlanId               int    `json:"VlanId"`
	Name                 string `json:"Name"`
	RowStatus            int    `json:"RowStatus"`
	TeMstid              bool   `json:"TeMstid"`
	EgressPorts          []int  `json:"EgressPorts"`
	UntaggedPorts        []int  `json:"UntaggedPorts"`
	ForbiddenEgressPorts []int  `json:"ForbiddenEgressPorts"`
	VlanPriority         string `json:"VlanPriority"`
}

type SpanningTreeVersion int

const (
	SpanningTreeVersionSTP        SpanningTreeVersion = 0
	SpanningTreeVersionRSTP       SpanningTreeVersion = 2
	SpanningTreeVersionNotSupport SpanningTreeVersion = 3
)

var spanningTreeVersionStrMap = map[string]SpanningTreeVersion{
	"STP":        SpanningTreeVersionSTP,
	"RSTP":       SpanningTreeVersionRSTP,
	"NotSupport": SpanningTreeVersionNotSupport,
}

var spanningTreeVersionMap = map[SpanningTreeVersion]string{
	SpanningTreeVersionSTP:        "STP",
	SpanningTreeVersionRSTP:       "RSTP",
	SpanningTreeVersionNotSupport: "NotSupport",
}

func (version SpanningTreeVersion) String() string {
	if str, ok := spanningTreeVersionMap[version]; ok {
		return str
	}

	return "NotSupport"
}

func ParseSpanningTreeVersion(str string) SpanningTreeVersion {
	if version, ok := spanningTreeVersionStrMap[str]; ok {
		return version
	}

	return SpanningTreeVersionNotSupport
}
