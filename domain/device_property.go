package domain

type DeviceProperty struct {
	Certificate                         bool
	Description                         string
	DeviceCluster                       string
	FeatureGroup                        FeatureGroup
	GCLOffsetMaxDuration                int
	GCLOffsetMinDuration                int
	GateControlListLength               int
	Ieee802Dot1cb                       Ieee802Dot1cb
	Ieee802VlanTag                      Ieee802VlanTag
	L2Family                            string
	L3Series                            string
	L4Series                            string
	MaxVlanCfgSize                      int
	ModelName                           string
	MountType                           MountTypeList
	NumberOfQueue                       int
	OperatingTemperatureC               TemperatureRange
	PerQueueSize                        int
	PhysicalModelName                   string
	ProcessingDelayMap                  map[string]ProcessingDelay
	PtpQueueId                          int
	ReservedVlan                        []int
	StreamPriorityConfigIngressIndexMax int
	TickGranularity                     int
	TrafficSpecification                TrafficSpecification
	Vendor                              string
}

type TemperatureRange struct {
	Max int
	Min int
}

type ProcessingDelay struct {
	DependentDelayRatio int
	IndependentDelay    int
}

type TrafficSpecification struct {
	EnableMaxBytesPerInterval bool
	Interval                  TrafficInterval
	MaxBytesPerInterval       int
	MaxFrameSize              int
	MaxFramesPerInterval      int
	TimeAware                 TimeAwareSpec
}

type TrafficInterval struct {
	Denominator int
	Numerator   int
}

type TimeAwareSpec struct {
	EarliestTransmitOffset int
	Jitter                 int
	LatestTransmitOffset   int
}

type Ieee802Dot1cb struct {
	SequenceGeneration     MinMax
	SequenceIdentification MinMax
	SequenceRecovery       SequenceRecovery
	StreamIdentity         StreamIdentity
}

type MinMax struct {
	Min int
	Max int
}

type SequenceRecovery struct {
	Min              int
	Max              int
	MinHistoryLength int
	MaxHistoryLength int
	ResetTimeout     int
}

type StreamIdentity struct {
	Min       int
	Max       int
	MinHandle int
	MaxHandle int
}

type Ieee802VlanTag struct {
	PriorityCodePoint int
	VlanId            int
}
