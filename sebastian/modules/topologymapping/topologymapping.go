package topologymapping

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

type ITopologyMapper interface {
	TopologyMapping(connId string, projectId int64, baselineId int64)
	Stop()
}

type DeviceMappingResultStatus int

const (
	DeviceMappingResultStatusSuccess  DeviceMappingResultStatus = 1 // Success (Found Mapping device & ModelName correctly)
	DeviceMappingResultStatusWarning  DeviceMappingResultStatus = 2 // Warning
	DeviceMappingResultStatusChecked  DeviceMappingResultStatus = 3 // EndStation Checked
	DeviceMappingResultStatusFailed   DeviceMappingResultStatus = 4 // Found Mapping Device but ModelName incorrect
	DeviceMappingResultStatusNotFound DeviceMappingResultStatus = 5 // Mapping Device not found
	DeviceMappingResultStatusSkip     DeviceMappingResultStatus = 6 // Skip (Not MOXA device)
)

var DeviceMappingResultStatusMap = map[DeviceMappingResultStatus]string{
	DeviceMappingResultStatusSuccess:  "Success",
	DeviceMappingResultStatusWarning:  "Warning",
	DeviceMappingResultStatusChecked:  "Checked",
	DeviceMappingResultStatusFailed:   "Failed",
	DeviceMappingResultStatusNotFound: "NotFound",
	DeviceMappingResultStatusSkip:     "Skip",
}

func (cmd DeviceMappingResultStatus) String() string {
	if name, ok := DeviceMappingResultStatusMap[cmd]; ok {
		return name
	}
	return DeviceMappingResultStatusMap[DeviceMappingResultStatusSkip]
}

type mappingDevice struct {
	Id                   int64                       `json:"Id"`
	IpAddress            string                      `json:"IpAddress"`
	MacAddress           string                      `json:"MacAddress"`
	ModelName            string                      `json:"ModelName"`
	Vendor               string                      `json:"Vendor"`
	DeviceType           domain.DeviceType           `json:"DeviceType"`
	FirmwareVersion      string                      `json:"FirmwareVersion"`
	SerialNumber         string                      `json:"SerialNumber"`
	DeviceProfileId      int64                       `json:"DeviceProfileId"`
	ModularConfiguration domain.ModularConfiguration `json:"ModularConfiguration, omitempty"`
	ModularInfo          netdl.Modules               `json:"ModularInfo"`
	BuiltInPower         bool                        `json:"BuiltInPower"`
}

type mappingLink struct {
	Id                     int64 `json:"Id"`
	SourceDeviceId         int64 `json:"SourceDeviceId"`
	SourceInterfaceId      int64 `json:"SourceInterfaceId"`
	DestinationDeviceId    int64 `json:"DestinationDeviceId"`
	DestinationInterfaceId int64 `json:"DestinationInterfaceId"`
}

type mappingTopology struct {
	Devices        []mappingDevice
	Links          []mappingLink
	SourceDeviceId int64
}

type topologyMappingResultCandidate struct {
	OfflineSourceIpNum    uint32                       `json:"OfflineSourceIpNum"`
	TopologyMappingResult domain.TopologyMappingResult `json:"TopologyMappingResult"`
	OfflineTopology       mappingTopology              `json:"OfflineTopology"`
	WarningItemNum        uint32                       `json:"WarningItemNum"`
	FailedItemNum         uint32                       `json:"FailedItemNum"`
}

type sourceDeviceCandidate struct {
	DeviceId int64
	IpNum    uint32
}
