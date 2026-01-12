package domain

import (
	"encoding/json"
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

type TopologySetting struct {
	ManagementInterfaces []ManagementInterface `json:"ManagementInterfaces"`
	RedundantGroup       RedundantGroup        `json:"RedundantGroup"`
	IntelligentVlanGroup []IntelligentVlan     `json:"IntelligentVlanGroup"`
}

type ManagementInterface struct {
	HttpActive                        bool   `json:"HttpActive"`
	HttpTcpPort                       int    `json:"HttpTcpPort"`
	HttpsActive                       bool   `json:"HttpsActive"`
	HttpsTcpPort                      int    `json:"HttpsTcpPort"`
	NumberOfHttpAndHttpsLoginSessions int    `json:"NumberOfHttpAndHttpsLoginSessions"`
	NumberOfTelnetAndSshLoginSessions int    `json:"NumberOfTelnetAndSshLoginSessions"`
	SnmpActive                        string `json:"SnmpActive"` // "enabled" 字串
	SnmpPort                          int    `json:"SnmpPort"`
	SnmpTransportProtocol             string `json:"SnmpTransportProtocol"` // e.g. "tCP"
	SshActive                         bool   `json:"SshActive"`
	SshTcpPort                        int    `json:"SshTcpPort"`
	TelnetActive                      bool   `json:"TelnetActive"`
	TelnetTcpPort                     int    `json:"TelnetTcpPort"`
}

type RedundantGroup struct {
	RSTP  []RSTPEntry `json:"RSTP"`
	Swift Swift       `json:"Swift"`
}

type RSTPEntry struct {
	Id               int64   `json:"Id"`
	HelloTime        int64   `json:"HelloTime"`
	RootDevice       int64   `json:"RootDevice"`
	BackupRootDevice int64   `json:"BackupRootDevice"`
	Devices          []int64 `json:"Devices"`
}

type Swift struct {
	Active           bool          `json:"Active"`
	BackupRootDevice int64         `json:"BackupRootDevice"`
	DeviceTierMap    map[int64]int `json:"DeviceTierMap"` // deviceID(string) -> tier(int)
	Links            []int         `json:"Links"`         // 以 int slice 存放連結裝置 ID
	RootDevice       int64         `json:"RootDevice"`
}

type StreamTypeEnum int

const (
	StreamTypeEnum_Unknown StreamTypeEnum = iota
	StreamTypeEnum_Tagged
	StreamTypeEnum_Untagged
)

var StreamTypeEnumToString = map[StreamTypeEnum]string{
	StreamTypeEnum_Unknown:  "Unknown",
	StreamTypeEnum_Tagged:   "Tagged",
	StreamTypeEnum_Untagged: "Untagged",
}

var StringToStreamTypeEnum = map[string]StreamTypeEnum{
	"Unknown":  StreamTypeEnum_Unknown,
	"Tagged":   StreamTypeEnum_Tagged,
	"Untagged": StreamTypeEnum_Untagged,
}

func (e *StreamTypeEnum) UnmarshalJSON(b []byte) error {
	var s string
	if err := json.Unmarshal(b, &s); err == nil {
		if v, ok := StringToStreamTypeEnum[s]; ok {
			*e = v
			return nil
		}
		return fmt.Errorf("invalid StreamType: %s", s)
	}

	// fallback: allow numeric
	var i int
	if err := json.Unmarshal(b, &i); err == nil {
		*e = StreamTypeEnum(i)
		return nil
	}

	return fmt.Errorf("invalid StreamType value")
}

func (stream_type StreamTypeEnum) String() string {
	if s, ok := StreamTypeEnumToString[stream_type]; ok {
		return s
	}
	return "Unknown"
}

func ParseStreamType(s string) (StreamTypeEnum, statuscode.Response) {
	if stream_type, ok := StringToStreamTypeEnum[s]; ok {
		return stream_type, statuscode.StatusOK(nil)
	}
	return StreamTypeEnum_Unknown, statuscode.StatusBadRequest("Invalid stream type: "+s, 0)
}

type IntelligentVlan struct {
	VlanId         uint16         `json:"VlanId"` // The unique id of the vlan
	StreamType     StreamTypeEnum `json:"StreamType"`
	EndStationList []string       `json:"EndStationList"` // The vlan id list
}

func (intelligentVlan IntelligentVlan) String() string {
	jsonBytes, _ := json.MarshalIndent(intelligentVlan, "", "  ")
	return string(jsonBytes)
}

func (intelligentVlan *IntelligentVlan) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &intelligentVlan)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}
