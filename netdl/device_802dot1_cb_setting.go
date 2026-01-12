package netdl

type StreamIdentificationSetting struct {
	StreamIdentificationSetting []StreamIdentificationEntry `json:"streamIdentificationSetting"`
}

type StreamIdentificationEntry struct {
	HandleId   int    `json:"handleId"`
	VlanId     int    `json:"vlanId"`
	MacAddress string `json:"macAddress"`
}
type FrerSetting struct {
	FrerSetting []FrerEntry `json:"frerSetting"`
}

type FrerType string

const (
	FrerTypeSplit   FrerType = "split"
	FrerTypeForward FrerType = "forward"
	FrerTypeMerge   FrerType = "merge"
)

type FrerEntry struct {
	Type           FrerType        `json:"type"`
	IngressStreams []IngressStream `json:"ingressStreams"`
	EgressStreams  []EgressStream  `json:"egressStreams"`
}

type IngressStream struct {
	StreamID  string `json:"streamId"` // format: "1/11:22:33:44:55:66"
	InputPort int    `json:"inputPort"`
}

type EgressStream struct {
	PortID      int            `json:"portId"`
	ToEndDevice *bool          `json:"toEndDevice,omitempty"` // only for forward/merge
	Overwrite   *FrerOverwrite `json:"overwrite,omitempty"`
}

type FrerOverwrite struct {
	VlanOverwrite       *int    `json:"vlanOverwrite,omitempty"`
	MacAddressOverwrite *string `json:"macAddressOverwrite,omitempty"`
	PriorityOverwrite   *int    `json:"priorityOverwrite,omitempty"`
}
