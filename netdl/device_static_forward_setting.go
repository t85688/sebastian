package netdl

type UnicastStaticForwardingStruct struct {
	UnicastStaticForwardingSetting []UnicastStaticForwardingEntry `json:"unicastStaticForwardingSetting"`
}

type UnicastStaticForwardingEntry struct {
	VlanId       int    `json:"vlanId"`
	MacAddress   string `json:"macAddress"`
	EgressPortId int    `json:"egressPortId"`
}

type MulticastStaticForwardingStruct struct {
	MulticastStaticForwardingSetting []MulticastStaticForwardingEntry `json:"multicastStaticForwardingSetting"`
}

type MulticastStaticForwardingEntry struct {
	VlanId           int    `json:"vlanId"`
	MacAddress       string `json:"macAddress"`
	EgressPortIds    []int  `json:"egressPortIds"`
	ForbiddenPortIds *[]int `json:"forbiddenPortIds,omitempty"`
}
