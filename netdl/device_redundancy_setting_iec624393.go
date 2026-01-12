package netdl

import (
	"encoding/json"
	"fmt"
)

const (
	PRPLanAId int = 0
	PRPLanBId int = 1
)

type IEC624393Protocol int

const (
	IEC624393ProtocolNone IEC624393Protocol = iota
	IEC624393ProtocolPRP
	IEC624393ProtocolHSR
	IEC624393ProtocolCoupling
)

var iec624393ProtocolStrMap = map[IEC624393Protocol]string{
	IEC624393ProtocolNone:     "",
	IEC624393ProtocolPRP:      "prp",
	IEC624393ProtocolHSR:      "hsr",
	IEC624393ProtocolCoupling: "coupling",
}

var iec624393ProtocolStrToTypeMap = map[string]IEC624393Protocol{
	"":         IEC624393ProtocolNone,
	"prp":      IEC624393ProtocolPRP,
	"hsr":      IEC624393ProtocolHSR,
	"coupling": IEC624393ProtocolCoupling,
}

func (protocol IEC624393Protocol) String() string {
	return iec624393ProtocolStrMap[protocol]
}

func ParseIEC624393Protocol(str string) IEC624393Protocol {
	if protocol, exists := iec624393ProtocolStrToTypeMap[str]; exists {
		return protocol
	}

	return IEC624393ProtocolNone
}

func (protocol IEC624393Protocol) MarshalJSON() ([]byte, error) {
	str, exists := iec624393ProtocolStrMap[protocol]
	if exists {
		return json.Marshal(str)
	}

	return nil, fmt.Errorf("invalid IEC624393Protocol type: %d", protocol)
}

func (protocol *IEC624393Protocol) UnmarshalJSON(data []byte) error {
	var str string
	if err := json.Unmarshal(data, &str); err != nil {
		return err
	}

	mappedProtocol, exists := iec624393ProtocolStrToTypeMap[str]
	if exists {
		*protocol = mappedProtocol
		return nil
	}

	*protocol = IEC624393ProtocolNone
	return fmt.Errorf("invalid IEC624393Protocol type: %s", str)
}

type RedundancyPRPHSRSetting struct {
	Enable          bool              `json:"enable"`
	EntryForgetTime int               `json:"entryForgetTime"`
	LanId           int               `json:"lanId"`
	NetId           int               `json:"netId"`
	Protocol        IEC624393Protocol `json:"protocol"`
}

type SupervisionFrameSetting struct {
	Enable              bool   `json:"enable"`
	DestMacAddrLastByte string `json:"destMacAddrLastByte"`
	ForwardToInterlink  bool   `json:"forwardToInterlink"`
	LifeCheckInterval   int    `json:"lifeCheckInterval"` // 1-60 seconds
	NodeForgetTime      int    `json:"nodeForgetTime"`    // 60-120 seconds
}
