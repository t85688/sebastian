package domain

import (
	"encoding/json"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
)

type ProjectSetting struct {
	// Account                               Account                `json:"Account"`
	AlgorithmConfiguration AlgorithmConfiguration `json:"AlgorithmConfiguration"`
	CfgWizardSetting       CfgWizardSetting       `json:"CfgWizardSetting"`
	MonitorConfiguration   MonitorConfiguration   `json:"MonitorConfiguration"`
	// NetconfConfiguration                  NetconfConfiguration   `json:"NetconfConfiguration"`
	PriorityCodePointToQueueMapping map[string][]int `json:"PriorityCodePointToQueueMapping"`
	ProjectName                     string           `json:"ProjectName"`
	ProjectStartIp                  ProjectStartIp   `json:"ProjectStartIp"`
	// RestfulConfiguration                  RestfulConfiguration   `json:"RestfulConfiguration"`
	ScanIpRanges []*ScanIpRange `json:"ScanIpRanges"`
	// SnmpConfiguration                     SnmpConfiguration      `json:"SnmpConfiguration"`
	// SnmpTrapConfiguration                 SnmpTrapConfiguration  `json:"SnmpTrapConfiguration"`
	TrafficTypeToPriorityCodePointMapping map[string][]int     `json:"TrafficTypeToPriorityCodePointMapping"`
	VlanRange                             VlanRange            `json:"VlanRange"`
	DeviceSecrets                         schema.DeviceSecrets `json:"DeviceSecrets"`
}

func (project_setting ProjectSetting) String() string {
	jsonBytes, _ := json.MarshalIndent(project_setting, "", "  ")
	return string(jsonBytes)
}

func (project_setting *ProjectSetting) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &project_setting)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

type AlgorithmConfiguration struct {
	BestEffortBandwidth        int  `json:"BestEffortBandwidth"`
	KeepPreviousResult         bool `json:"KeepPreviousResult"`
	MediaSpecificOverheadBytes int  `json:"MediaSpecificOverheadBytes"`
	TimeSyncBandwidth          int  `json:"TimeSyncBandwidth"`
	TimeSyncDelay              int  `json:"TimeSyncDelay"`
	Timeout                    int  `json:"Timeout"`
}

type CfgWizardSetting struct {
	DefineDeviceToBeSet         DefineDeviceToBeSet         `json:"DefineDeviceToBeSet"`
	DefineDeviceType            DefineDeviceType            `json:"DefineDeviceType"`
	DefineIpAssigningSequence   DefineIpAssigningSequence   `json:"DefineIpAssigningSequence"`
	DefineIpAssignment          DefineIpAssignment          `json:"DefineIpAssignment"`
	DefineIpConfiguringSequence DefineIpConfiguringSequence `json:"DefineIpConfiguringSequence"`
	DefineNetworkInterface      DefineNetworkInterface      `json:"DefineNetworkInterface"`
	DefineTopologyScan          DefineTopologyScan          `json:"DefineTopologyScan"`
	UnlockDevices               UnlockDevices               `json:"UnlockDevices"`
}

type DefineDeviceToBeSet struct {
	DefineDeviceToBeSetType string   `json:"DefineDeviceToBeSetType"`
	Skip                    bool     `json:"Skip"`
	SpecificIps             []string `json:"SpecificIps"`
}

type DefineDeviceType struct {
	MoxaIndustrialEthernetProduct bool `json:"MoxaIndustrialEthernetProduct"`
	MoxaIndustrialWirelessProduct bool `json:"MoxaIndustrialWirelessProduct"`
	Skip                          bool `json:"Skip"`
}

type DefineIpAssigningSequence struct {
	IpAssigningSequenceType string `json:"IpAssigningSequenceType"`
	Skip                    bool   `json:"Skip"`
}

type DefineIpAssignment struct {
	ActDefineIpAssigningRange ActDefineIpAssigningRange `json:"ActDefineIpAssigningRange"`
	ActDefineIpAssigningRule  ActDefineIpAssigningRule  `json:"ActDefineIpAssigningRule"`
	DefineIpAssignment        string                    `json:"DefineIpAssignment"`
	Skip                      bool                      `json:"Skip"`
}

type ActDefineIpAssigningRange struct {
	StartIp string `json:"StartIp"`
	EndIp   string `json:"EndIp"`
}

type ActDefineIpAssigningRule struct {
	StartIp        string `json:"StartIp"`
	IncrementalGap int    `json:"IncrementalGap"`
}

type DefineIpConfiguringSequence struct {
	IpConfiguringSequenceType string `json:"IpConfiguringSequenceType"`
	Skip                      bool   `json:"Skip"`
}

type DefineNetworkInterface struct {
	DefineNetworkInterfaces []string `json:"DefineNetworkInterfaces"` // or a struct if more detail
	Skip                    bool     `json:"Skip"`
}

type DefineTopologyScan struct {
	Skip bool `json:"Skip"`
}

type UnlockDevices struct {
	Skip bool `json:"Skip"`
}

type MonitorConfiguration struct {
	FromIpScanList     bool `json:"FromIpScanList"`
	FromOfflineProject bool `json:"FromOfflineProject"`
	PollingInterval    int  `json:"PollingInterval"`
}

// type NetconfConfiguration struct {
// 	DefaultSetting bool           `json:"DefaultSetting"`
// 	NetconfOverSSH NetconfOverSSH `json:"NetconfOverSSH"`
// 	NetconfOverTLS NetconfOverTLS `json:"NetconfOverTLS"`
// 	TLS            bool           `json:"TLS"`
// }

// type NetconfOverSSH struct {
// 	SSHPort int `json:"SSHPort"`
// }

// type NetconfOverTLS struct {
// 	CaCerts  string `json:"CaCerts"`
// 	CertFile string `json:"CertFile"`
// 	KeyFile  string `json:"KeyFile"`
// 	TLSPort  int    `json:"TLSPort"`
// }

type ProjectStartIp struct {
	DNS1       string `json:"DNS1"`
	DNS2       string `json:"DNS2"`
	Gateway    string `json:"Gateway"`
	IpAddress  string `json:"IpAddress"`
	SubnetMask string `json:"SubnetMask"`
}

// type RestfulConfiguration struct {
// 	DefaultSetting bool   `json:"DefaultSetting"`
// 	Port           int    `json:"Port"`
// 	Protocol       string `json:"Protocol"`
// }

// type SnmpConfiguration struct {
// 	AuthenticationPassword string `json:"AuthenticationPassword"`
// 	AuthenticationType     string `json:"AuthenticationType"`
// 	DataEncryptionKey      string `json:"DataEncryptionKey"`
// 	DataEncryptionType     string `json:"DataEncryptionType"`
// 	DefaultSetting         bool   `json:"DefaultSetting"`
// 	Port                   int    `json:"Port"`
// 	ReadCommunity          string `json:"ReadCommunity"`
// 	Username               string `json:"Username"`
// 	Version                string `json:"Version"`
// 	WriteCommunity         string `json:"WriteCommunity"`
// }

// type SnmpTrapConfiguration struct {
// 	AuthenticationPassword string `json:"AuthenticationPassword"`
// 	AuthenticationType     string `json:"AuthenticationType"`
// 	DataEncryptionKey      string `json:"DataEncryptionKey"`
// 	DataEncryptionType     string `json:"DataEncryptionType"`
// 	Port                   int    `json:"Port"`
// 	TrapCommunity          string `json:"TrapCommunity"`
// 	Username               string `json:"Username"`
// 	Version                string `json:"Version"`
// }

type TrafficTypeToPriorityCodePointMapping struct {
	BestEffort []int `json:"BestEffort"`
	Cyclic     []int `json:"Cyclic"`
	NA         []int `json:"NA"`
	TimeSync   []int `json:"TimeSync"`
}

type VlanRange struct {
	Min int `json:"Min"`
	Max int `json:"Max"`
}

type ScanIpRanges struct {
	ScanIpRanges []*ScanIpRange `json:"ScanIpRanges"`
}

type ScanIpRange struct {
	Id int64 `json:"Id"`
	// Account              Account              `json:"Account"`
	AutoProbe         bool   `json:"AutoProbe"`
	EnableSnmpSetting bool   `json:"EnableSnmpSetting"`
	StartIp           string `json:"StartIp"`
	EndIp             string `json:"EndIp"`
	// NetconfConfiguration NetconfConfiguration `json:"NetconfConfiguration"`
	// RestfulConfiguration RestfulConfiguration `json:"RestfulConfiguration"`
	// SnmpConfiguration    SnmpConfiguration    `json:"SnmpConfiguration"`
	DeviceSecrets schema.DeviceSecrets `json:"DeviceSecrets"`
}
