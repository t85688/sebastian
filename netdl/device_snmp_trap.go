package netdl

type SNMPTrapSetting struct {
	HostName       string `json:"hostName"`
	Mode           string `json:"mode,omitempty"`
	V1v2cCommunity string `json:"v1v2cCommunity,omitempty"`
}
