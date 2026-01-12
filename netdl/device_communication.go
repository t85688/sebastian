package netdl

type CommunicationStatus int

const (
	CommunicationStatusUnknown CommunicationStatus = iota
	CommunicationStatusReachable
	CommunicationStatusUnreachable
)

var communicationStatusMap = map[CommunicationStatus]string{
	CommunicationStatusUnknown:     "unknown",
	CommunicationStatusReachable:   "reachable",
	CommunicationStatusUnreachable: "unreachable",
}

func (cs CommunicationStatus) String() string {
	if str, ok := communicationStatusMap[cs]; ok {
		return str
	}

	return "unknown"
}

// Communication is the mamagement interfaces' status of the device
type Communication struct {
	ICMP struct {
		Status string `json:"status"`
	} `json:"icmp,omitempty"`
	SNMP struct {
		Status string `json:"status"`
	} `json:"snmp,omitempty"`
}

func (c *Communication) IsNormal() bool {
	if c.ICMP.Status == "" && c.SNMP.Status == "reachable" {
		return true
	}
	if c.ICMP.Status == "reachable" && c.SNMP.Status == "" {
		return true
	}
	if c.ICMP.Status == "reachable" && c.SNMP.Status == "reachable" {
		return true
	}
	return false
}
