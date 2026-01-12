package netdl

import (
	"encoding/json"
	"fmt"
)

type LinkAggregationSetting struct {
	PortChannelList []PortChannelConfig `json:"portChannelList"`
	PortList        []LaPortConfig      `json:"portList"`
}

type LAGroupStatusEnum string

const (
	LAGroupEnable  LAGroupStatusEnum = "enabled"
	LAGroupDisable LAGroupStatusEnum = "disabled"
)

func (laGroupStatusEnum *LAGroupStatusEnum) UnmarshalJSON(b []byte) error {
	var status string
	if err := json.Unmarshal(b, &status); err != nil {
		return err
	}
	switch status {
	case "enabled":
		*laGroupStatusEnum = LAGroupEnable
	case "disabled":
		*laGroupStatusEnum = LAGroupDisable
	default:
		return fmt.Errorf("invalid link aggregation group status: %s. Should be \"enabled\" or \"disabled\"", status)
	}
	return nil
}
func (laGroupStatusEnum LAGroupStatusEnum) MarshalJSON() ([]byte, error) {
	var status string
	switch laGroupStatusEnum {
	case LAGroupEnable:
		status = "enabled"
	case LAGroupDisable:
		status = "disabled"
	default:
		return nil, fmt.Errorf("invalid link aggregation group status: %s", laGroupStatusEnum)
	}
	return json.Marshal(status)
}

type LAGroupTypeEnum string

const (
	TypeManual LAGroupTypeEnum = "manual"
	TypeLACP   LAGroupTypeEnum = "LACP"
)

func (laGroupTypeEnum *LAGroupTypeEnum) UnmarshalJSON(b []byte) error {
	var groupType string
	if err := json.Unmarshal(b, &groupType); err != nil {
		return err
	}
	switch groupType {
	case "manual":
		*laGroupTypeEnum = TypeManual
	case "LACP":
		*laGroupTypeEnum = TypeLACP
	default:
		return fmt.Errorf("invalid link aggregation group type: %s. Should be \"manual\" or \"LACP\"", groupType)
	}
	return nil
}
func (laGroupTypeEnum LAGroupTypeEnum) MarshalJSON() ([]byte, error) {
	var groupType string
	switch laGroupTypeEnum {
	case TypeManual:
		groupType = "manual"
	case TypeLACP:
		groupType = "LACP"
	default:
		return nil, fmt.Errorf("invalid link aggregation group type: %s", laGroupTypeEnum)
	}
	return json.Marshal(groupType)
}

type HashAlgorithmEnum string

const (
	AlgorithmSMAC                    HashAlgorithmEnum = "SMAC"
	AlgorithmDMAC                    HashAlgorithmEnum = "DMAC"
	AlgorithmSMACDMAC                HashAlgorithmEnum = "SMAC + DMAC"
	AlgorithmSMACDMACSIPDIP          HashAlgorithmEnum = "SMAC+DMAC+SIP+DIP"
	AlgorithmSMACDMACSIPDIPSLDL4Port HashAlgorithmEnum = "SMAC+DMAC+SIP+DIP+SL4Port+DL4Port"
)

func (algorithmEnum *HashAlgorithmEnum) UnmarshalJSON(b []byte) error {
	var algorithm string
	if err := json.Unmarshal(b, &algorithm); err != nil {
		return err
	}
	switch algorithm {
	case "SMAC":
		*algorithmEnum = AlgorithmSMAC
	case "DMAC":
		*algorithmEnum = AlgorithmDMAC
	case "SMAC + DMAC":
		*algorithmEnum = AlgorithmSMACDMAC
	case "SMAC+DMAC+SIP+DIP":
		*algorithmEnum = AlgorithmSMACDMACSIPDIP
	case "SMAC+DMAC+SIP+DIP+SL4Port+DL4Port":
		*algorithmEnum = AlgorithmSMACDMACSIPDIPSLDL4Port
	default:
		return fmt.Errorf("invalid link aggregation hash algorithm: %s. Should be \"SMAC\", \"DMAC\", \"SMAC + DMAC\", \"SMAC+DMAC+SIP+DIP\" or \"SMAC+DMAC+SIP+DIP+SL4Port+DL4Port\"", algorithm)
	}
	return nil
}
func (algorithmEnum HashAlgorithmEnum) MarshalJSON() ([]byte, error) {
	var algorithm string
	switch algorithmEnum {
	case AlgorithmSMAC:
		algorithm = "SMAC"
	case AlgorithmDMAC:
		algorithm = "DMAC"
	case AlgorithmSMACDMAC:
		algorithm = "SMAC + DMAC"
	case AlgorithmSMACDMACSIPDIP:
		algorithm = "SMAC+DMAC+SIP+DIP"
	case AlgorithmSMACDMACSIPDIPSLDL4Port:
		algorithm = "SMAC+DMAC+SIP+DIP+SL4Port+DL4Port"
	default:
		return nil, fmt.Errorf("invalid link aggregation hash algorithm: %s", algorithmEnum)
	}
	return json.Marshal(algorithm)
}

type PortChannelConfig struct {
	PortChannel         int               `json:"portChannel"`
	LAGroupStatus       LAGroupStatusEnum `json:"laGroupStatus"`
	Type                LAGroupTypeEnum   `json:"type"`
	ConfigureMemberPort []int             `json:"configureMemberPort"`
	Algorithm           HashAlgorithmEnum `json:"algorithm"`
}

type LACPModesEnum string

const (
	ModeActive  LACPModesEnum = "active"
	ModePassive LACPModesEnum = "passive"
)

func (modeEnum *LACPModesEnum) UnmarshalJSON(b []byte) error {
	var mode string
	if err := json.Unmarshal(b, &mode); err != nil {
		return err
	}
	switch mode {
	case "active":
		*modeEnum = ModeActive
	case "passive":
		*modeEnum = ModePassive
	default:
		return fmt.Errorf("invalid LACP mode: %s. Should be \"active\" or \"passive\"", mode)
	}
	return nil
}
func (modeEnum LACPModesEnum) MarshalJSON() ([]byte, error) {
	var mode string
	switch modeEnum {
	case ModeActive:
		mode = "active"
	case ModePassive:
		mode = "passive"
	default:
		return nil, fmt.Errorf("invalid LACP mode: %s", modeEnum)
	}
	return json.Marshal(mode)
}

type LACPTimeoutEnum int

const (
	TimeoutShort LACPTimeoutEnum = 3
	TimeoutLong  LACPTimeoutEnum = 90
)

func (timeoutEnum *LACPTimeoutEnum) UnmarshalJSON(b []byte) error {
	var timeout int
	if err := json.Unmarshal(b, &timeout); err != nil {
		return err
	}
	switch timeout {
	case 3:
		*timeoutEnum = TimeoutShort
	case 90:
		*timeoutEnum = TimeoutLong
	default:
		return fmt.Errorf("invalid timeout value: %d. Should be \"3\" or \"90\"", timeout)
	}
	return nil
}
func (timeoutEnum LACPTimeoutEnum) MarshalJSON() ([]byte, error) {
	var timeout int
	switch timeoutEnum {
	case TimeoutShort:
		timeout = 3
	case TimeoutLong:
		timeout = 90
	default:
		return nil, fmt.Errorf("invalid timeout value: %d", timeoutEnum)
	}
	return json.Marshal(timeout)
}

type LaPortConfig struct {
	PortID    int             `json:"portID"`
	AliasName string          `json:"aliasName"`
	Mode      LACPModesEnum   `json:"mode"`     // "active" or "passive"
	Timeout   LACPTimeoutEnum `json:"timeout"`  // 3 or 90
	WaitTime  int             `json:"waitTime"` // Range: 0–10
}
