package wscommand

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommon"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
)

type BaseWsCommmandSchema struct {
	OpCode    int64 `json:"OpCode"`
	ProjectId int64 `json:"ProjectId"`
	// Data      any `json:"data"`
}

func NewBaseWsCommandSchema() *BaseWsCommmandSchema {
	return &BaseWsCommmandSchema{
		OpCode:    ActWSCommandUnknown.Int64(),
		ProjectId: wscommon.ProjectIdNone,
	}
}

type ScanTopologyCommandSchema struct {
	BaseWsCommmandSchema
	NewTopology bool `json:"NewTopology"`
}

type DeviceDiscoveryCommandSchema struct {
	BaseWsCommmandSchema
	domain.Account                `json:"Account"`
	domain.NetconfConfiguration   `json:"NetconfConfiguration"`
	domain.RestfulConfiguration   `json:"RestfulConfiguration"`
	domain.SnmpConfiguration      `json:"SnmpConfiguration"`
	domain.DefineDeviceType       `json:"DefineDeviceType"`
	domain.DefineNetworkInterface `json:"DefineNetworkInterface"`
	EnableSnmpSetting             bool `json:"EnableSnmpSetting"`
}

type DeployCommandSchema struct {
	BaseWsCommmandSchema
	DesignBaselineId  int64   `json:"designBaselineId"`
	Id                []int64 `json:"id"`
	SkipMappingDevice bool    `json:"skipMappingDevice"`
}

type TopologyMappingCommandSchema struct {
	BaseWsCommmandSchema
	DesignBaselineId int64 `json:"DesignBaselineId"`
}

type StartMonitorCommandSchema struct {
	BaseWsCommmandSchema
}

type StopMonitorCommandSchema struct {
	BaseWsCommmandSchema
	SaveToProject bool `json:"SaveToProject"`
}

type LocatorCommandSchema struct {
	BaseWsCommmandSchema
	Id       []int64 `json:"id"`
	Duration uint32  `json:"Duration"`
}

type FirmwareUpgradeCommandSchema struct {
	BaseWsCommmandSchema
	Id           []int64 `json:"id"`
	FirmwareName string  `json:"FirmwareName"`
}

type OperationsCommandSchema struct {
	BaseWsCommmandSchema
	Id []int64 `json:"id"`
}

type ServicePlatformCommandSchema struct {
	BaseWsCommmandSchema
	DeviceCode string `json:"DeviceCode"`
}
