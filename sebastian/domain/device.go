package domain

import (
	"encoding/json"
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

type MountType int

const (
	MountTypeUnknown MountType = iota
	MountTypeDinRail
	MountTypeWallMount
	MountTypeRackMount
)

var mountTypeMap = map[MountType]string{
	MountTypeUnknown:   "Unknown",
	MountTypeDinRail:   "DinRail",
	MountTypeWallMount: "WallMount",
	MountTypeRackMount: "RackMount",
}

var mountTypeStrToTypeMap = map[string]MountType{
	"Unknown":   MountTypeUnknown,
	"DinRail":   MountTypeDinRail,
	"WallMount": MountTypeWallMount,
	"RackMount": MountTypeRackMount,
}

func (mountType MountType) String() string {
	if str, exists := mountTypeMap[mountType]; exists {
		return str
	}
	return mountTypeMap[MountTypeUnknown]
}

func ParseMountType(str string) MountType {
	if mountType, exists := mountTypeStrToTypeMap[str]; exists {
		return mountType
	}
	return MountTypeUnknown
}

func (mountType MountType) MarshalJSON() ([]byte, error) {
	str, exists := mountTypeMap[mountType]
	if exists {
		return json.Marshal(str)
	}
	return nil, fmt.Errorf("invalid MountType: %d", mountType)
}

func (mountType *MountType) UnmarshalJSON(data []byte) error {
	var str string
	if err := json.Unmarshal(data, &str); err != nil {
		return err
	}
	mappedMountType, exists := mountTypeStrToTypeMap[str]
	if exists {
		*mountType = mappedMountType
		return nil
	}
	*mountType = MountTypeUnknown
	return fmt.Errorf("invalid MountType: %s", str)
}

// MountTypeList is the slice enum for multi-mount (used as field type)
type MountTypeList []MountType

func (m MountTypeList) MarshalJSON() ([]byte, error) {
	strList := make([]string, len(m))
	for i, typ := range m {
		strList[i] = typ.String()
	}
	return json.Marshal(strList)
}

func (m *MountTypeList) UnmarshalJSON(data []byte) error {
	var strList []string
	if err := json.Unmarshal(data, &strList); err != nil {
		return err
	}
	types := make(MountTypeList, len(strList))
	for i, s := range strList {
		types[i] = ParseMountType(s)
	}
	*m = types
	return nil
}

type DeviceType int

const (
	DeviceTypeUnknown DeviceType = iota
	DeviceTypeTSNSwitch
	DeviceTypeEndStation
	DeviceTypeBridgedEndStation
	DeviceTypeSwitch
	DeviceTypePoeAccessory
	DeviceTypeNetworkMgmtSoftware
	DeviceTypeICMP
	DeviceTypeMoxa
)

var deviceTypeMap = map[DeviceType]string{
	DeviceTypeUnknown:             "Unknown",
	DeviceTypeTSNSwitch:           "TSNSwitch",
	DeviceTypeEndStation:          "EndStation",
	DeviceTypeBridgedEndStation:   "BridgedEndStation",
	DeviceTypeSwitch:              "Switch",
	DeviceTypePoeAccessory:        "PoeAccessory",
	DeviceTypeNetworkMgmtSoftware: "NetworkMgmtSoftware",
	DeviceTypeICMP:                "ICMP",
	DeviceTypeMoxa:                "Moxa",
}

var deviceTypeStrToTypeMap = map[string]DeviceType{
	"Unknown":             DeviceTypeUnknown,
	"TSNSwitch":           DeviceTypeTSNSwitch,
	"EndStation":          DeviceTypeEndStation,
	"BridgedEndStation":   DeviceTypeBridgedEndStation,
	"Switch":              DeviceTypeSwitch,
	"PoeAccessory":        DeviceTypePoeAccessory,
	"NetworkMgmtSoftware": DeviceTypeNetworkMgmtSoftware,
	"ICMP":                DeviceTypeICMP,
	"Moxa":                DeviceTypeMoxa,
}

func (deviceType DeviceType) String() string {
	if str, exists := deviceTypeMap[deviceType]; exists {
		return str
	}

	return deviceTypeMap[DeviceTypeUnknown]
}

func ParseDeviceType(str string) DeviceType {
	if deviceType, exists := deviceTypeStrToTypeMap[str]; exists {
		return deviceType
	}

	return DeviceTypeUnknown
}

func (deviceType DeviceType) MarshalJSON() ([]byte, error) {
	str, exists := deviceTypeMap[deviceType]
	if exists {
		return json.Marshal(str)
	}

	return nil, fmt.Errorf("invalid DeviceType: %d", deviceType)
}

func (deviceType *DeviceType) UnmarshalJSON(data []byte) error {
	var str string
	if err := json.Unmarshal(data, &str); err != nil {
		return err
	}

	mappedDeviceType, exists := deviceTypeStrToTypeMap[str]
	if exists {
		*deviceType = mappedDeviceType
		return nil
	}

	*deviceType = DeviceTypeUnknown
	return fmt.Errorf("invalid DeviceType: %s", str)
}

type DeviceCategory int

const (
	DeviceCategoryNone DeviceCategory = iota
	DeviceCategoryUnmanagedSwitches
	DeviceCategoryRackmountSwitches
	DeviceCategoryLayer2SmartSwitches
	DeviceCategoryLayer2ManagedSwitches
	DeviceCategoryLayer3ManagedSwitches
)

var deviceCategoryMap = map[DeviceCategory]string{
	DeviceCategoryNone:                  "None",
	DeviceCategoryUnmanagedSwitches:     "UnmanagedSwitches",
	DeviceCategoryRackmountSwitches:     "RackmountSwitches",
	DeviceCategoryLayer2SmartSwitches:   "Layer2SmartSwitches",
	DeviceCategoryLayer2ManagedSwitches: "Layer2ManagedSwitches",
	DeviceCategoryLayer3ManagedSwitches: "Layer3ManagedSwitches",
}

var deviceCategoryStrToTypeMap = map[string]DeviceCategory{
	"None":                  DeviceCategoryNone,
	"UnmanagedSwitches":     DeviceCategoryUnmanagedSwitches,
	"RackmountSwitches":     DeviceCategoryRackmountSwitches,
	"Layer2SmartSwitches":   DeviceCategoryLayer2SmartSwitches,
	"Layer2ManagedSwitches": DeviceCategoryLayer2ManagedSwitches,
	"Layer3ManagedSwitches": DeviceCategoryLayer3ManagedSwitches,
}

func (category DeviceCategory) String() string {
	if str, ok := deviceCategoryMap[category]; ok {
		return str
	}
	return deviceCategoryMap[DeviceCategoryNone]
}

func ParseDeviceCategory(str string) DeviceCategory {
	if category, ok := deviceCategoryStrToTypeMap[str]; ok {
		return category
	}
	return DeviceCategoryNone
}

func (category DeviceCategory) MarshalJSON() ([]byte, error) {
	str, exists := deviceCategoryMap[category]
	if exists {
		return json.Marshal(str)
	}
	return nil, fmt.Errorf("invalid DeviceCategory: %d", category)
}

func (category *DeviceCategory) UnmarshalJSON(data []byte) error {
	var str string
	if err := json.Unmarshal(data, &str); err != nil {
		return err
	}
	mapped, exists := deviceCategoryStrToTypeMap[str]
	if exists {
		*category = mapped
		return nil
	}
	*category = DeviceCategoryNone
	return fmt.Errorf("invalid DeviceCategory: %s", str)
}

type CreateDeviceRequest struct {
	DeviceConf DeviceConf `json:"Device"`
	FromBag    bool       `json:"FromBag"`
}

func (createDeviceRequest CreateDeviceRequest) String() string {
	jsonBytes, _ := json.MarshalIndent(createDeviceRequest, "", "  ")
	return string(jsonBytes)
}

func (createDeviceRequest *CreateDeviceRequest) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &createDeviceRequest)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

type DeviceConf struct {
	ModelName            string               `json:"ModelName,omitempty"`
	DeviceAlias          string               `json:"DeviceAlias,omitempty"`
	DeviceName           string               `json:"deviceName,omitempty" gorm:"column:device_name;type:varchar(255)"`
	DeviceProfileId      int64                `json:"DeviceProfileId,omitempty"`
	IP                   string               `json:"ip,omitempty" gorm:"column:ip;type:varchar(15)"`
	MAC                  string               `json:"mac,omitempty" gorm:"column:mac;type:varchar(12)"`
	SubnetMask           string               `json:"subnetMask,omitempty" gorm:"-"`
	DefaultGateway       string               `json:"defaultGateway,omitempty" gorm:"-"`
	DnsServers           []string             `json:"dnsServers,omitempty" gorm:"-"`
	FirmwareVersion      string               `json:"firmwareVersion,omitempty" gorm:"column:firmware_version;type:varchar(50)"`
	Interfaces           []*netdl.Interface   `json:"interfaces,omitempty"`
	ModularConfiguration ModularConfiguration `json:"ModularConfiguration,omitempty"`
}

func (deviceConf DeviceConf) String() string {
	jsonBytes, _ := json.MarshalIndent(deviceConf, "", "  ")
	return string(jsonBytes)
}

func (deviceConf *DeviceConf) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &deviceConf)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

func (deviceConf *DeviceConf) CheckFeasibility() statuscode.Response {
}

type Device struct {
	netdl.Device

	DeviceSecrets            schema.DeviceSecrets `json:"DeviceSecrets"`
	DeviceProfileId          int64                `json:"DeviceProfileId"`
	FirmwareFeatureProfileId int                  `json:"FirmwareFeatureProfileId"`
	EthernetProfileId        map[int]int64        `json:"EthernetProfileId"` // map[slotId]EthernetProfileId
	PowerProfileId           map[int]int64        `json:"PowerProfileId"`    // map[slotId]powerProfileId
	Coordinate               Coordinate           `json:"Coordinate"`
	DeviceAlias              string               `json:"DeviceAlias"`
	DeviceProperty           DeviceProperty       `json:"DeviceProperty"`
	Distance                 int                  `json:"Distance"`
	ModularConfiguration     ModularConfiguration `json:"ModularConfiguration"`

	// DeviceInterfaces map[int64]DeviceInterface `json:"DeviceInterfaces"`
	// Account                  Account              `json:"Account"`
	// Coordinate               Coordinate           `json:"Coordinate"`
	// DeviceAlias              string               `json:"DeviceAlias"`
	// DeviceName               string               `json:"DeviceName"`
	// DeviceProfileId          int64                `json:"DeviceProfileId"`
	// DeviceProperty           DeviceProperty       `json:"DeviceProperty"`
	// DeviceRole               string               `json:"DeviceRole"`
	// DeviceStatus             DeviceStatus         `json:"DeviceStatus"`
	// DeviceType               DeviceType           `json:"DeviceType"`
	// Distance                 int                  `json:"Distance"`
	// EnableSnmpSetting        bool                 `json:"EnableSnmpSetting"`
	// FirmwareFeatureProfileId int                  `json:"FirmwareFeatureProfileId"`
	// FirmwareVersion          string               `json:"FirmwareVersion"`
	// Id                       int64                `json:"Id"`
	// Interfaces               []Interface          `json:"Interfaces"`
	// Ipv4                     Ipv4                 `json:"Ipv4"`
	// MacAddress               string               `json:"MacAddress"`
	// ModularConfiguration ModularConfiguration `json:"ModularConfiguration"`
	// NetconfConfiguration     NetconfConfiguration `json:"NetconfConfiguration"`
	// RestfulConfiguration     RestfulConfiguration `json:"RestfulConfiguration"`
	// SSHPort                  int                  `json:"SSHPort"`
	// SnmpConfiguration        SnmpConfiguration    `json:"SnmpConfiguration"`
	// Tier                     int                  `json:"Tier"`
}

func (device *Device) String() string {
	jsonBytes, _ := json.MarshalIndent(device, "", "  ")
	return string(jsonBytes)
}

func (device *Device) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &device)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

func (device *Device) Convert() (deviceConf *DeviceConf) {
	return &DeviceConf{
		ModelName:            device.ModelName,
		DeviceName:           device.DeviceName,
		IP:                   device.IP,
		MAC:                  device.MAC,
		SubnetMask:           device.SubnetMask,
		DefaultGateway:       device.DefaultGateway,
		DnsServers:           device.DnsServers,
		FirmwareVersion:      device.FirmwareVersion,
		DeviceProfileId:      device.DeviceProfileId,
		Interfaces:           device.Interfaces,
		ModularConfiguration: device.ModularConfiguration,
	}
}

// type Account struct {
// 	DefaultSetting bool   `json:"DefaultSetting"`
// 	Password string `json:"Password"`
// 	Username string `json:"Username"`
// }

type Coordinate struct {
	X int `json:"X"`
	Y int `json:"Y"`
}

// type DeviceStatus struct {
// 	ICMPStatus           bool `json:"ICMPStatus"`
// 	NETCONFStatus        bool `json:"NETCONFStatus"`
// 	NewMOXACommandStatus bool `json:"NewMOXACommandStatus"`
// 	RESTfulStatus        bool `json:"RESTfulStatus"`
// 	SNMPStatus           bool `json:"SNMPStatus"`
// }

// type Interface struct {
// 	Active        bool     `json:"Active"`
// 	CableTypes    []string `json:"CableTypes"`
// 	Description   string   `json:"Description"`
// 	DeviceId      int64    `json:"DeviceId"`
// 	InterfaceId   int      `json:"InterfaceId"`
// 	InterfaceName string   `json:"InterfaceName"`
// 	IpAddress     string   `json:"IpAddress"`
// 	MacAddress    string   `json:"MacAddress"`
// 	Management    bool     `json:"Management"`
// 	RootGuard     bool     `json:"RootGuard"`
// 	SupportSpeeds []int    `json:"SupportSpeeds"`
// 	Used          bool     `json:"Used"`
// }

// type Ipv4 struct {
// 	DNS1       string `json:"DNS1"`
// 	DNS2       string `json:"DNS2"`
// 	Gateway    string `json:"Gateway"`
// 	IpAddress  string `json:"IpAddress"`
// 	SubnetMask string `json:"SubnetMask"`
// }

type ModularConfiguration struct {
	Ethernet map[int]int64 `json:"Ethernet"`
	Power    map[int]int64 `json:"Power"`
}

// type SimpleDevices struct {
// 	Devices map[string]*Device `json:"Devices"`
// }

// type DeviceIds struct {
// 	DeviceIds []string `json:"DeviceIds"`
// }

// func (devIds DeviceIds) String() string {
// 	jsonBytes, _ := json.MarshalIndent(devIds, "", "  ")
// 	return string(jsonBytes)
// }

// func (devIds *DeviceIds) UnmarshalJSONData(data []byte) error {
// 	return json.Unmarshal(data, devIds)
// }

type DeviceOfflineConfigFileMap struct {
	DeviceOfflineConfigFileMap map[string]string `json:"DeviceOfflineConfigFileMap"`
}

func (deviceOfflineConfigFileMap DeviceOfflineConfigFileMap) String() string {
	jsonBytes, _ := json.MarshalIndent(deviceOfflineConfigFileMap, "", "  ")
	return string(jsonBytes)
}

func (deviceOfflineConfigFileMap *DeviceOfflineConfigFileMap) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &deviceOfflineConfigFileMap)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

type DeployDevice struct {
	DeviceId        int64  `json:"DeviceId"`
	DeviceIp        string `json:"DeviceIp"`
	DeviceAlias     string `json:"DeviceAlias"`
	ModelName       string `json:"ModelName"`
	FirmwareVersion string `json:"FirmwareVersion"`
	MacAddress      string `json:"MacAddress"`
	SerialNumber    string `json:"SerialNumber"`
	Location        string `json:"Location"`
	Support         bool   `json:"Support"`
}

func (deployDevice DeployDevice) String() string {
	jsonBytes, _ := json.MarshalIndent(deployDevice, "", "  ")
	return string(jsonBytes)
}

func (deployDevice *DeployDevice) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &deployDevice)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

type DeployDeviceList struct {
	DeviceList []DeployDevice `json:"DeviceList"`
}

func (deployDeviceList DeployDeviceList) String() string {
	jsonBytes, _ := json.MarshalIndent(deployDeviceList, "", "  ")
	return string(jsonBytes)
}

func (deployDeviceList *DeployDeviceList) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &deployDeviceList)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

type DevicePartial struct {
	Id                   string                 `json:"Id"`
	MacAddress           *string                `json:"MacAddress,omitempty"`
	DeviceProfileId      *int64                 `json:"DeviceProfileId,omitempty"`
	DeviceProperty       *DevicePropertyPartial `json:"DeviceProperty,omitempty"`
	FirmwareVersion      *string                `json:"FirmwareVersion,omitempty"`
	DeviceName           *string                `json:"DeviceName,omitempty"`
	ModularConfiguration *ModularConfiguration  `json:"ModularConfiguration,omitempty"`
}

type DevicePropertyPartial struct {
	ModelName *string `json:"ModelName,omitempty"`
}
