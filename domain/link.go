package domain

import (
	"encoding/json"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

type LinkInfo struct {
	Id int64 `json:"Id"`
	LinkConf
}

func (linkInfo LinkInfo) String() string {
	jsonBytes, _ := json.MarshalIndent(linkInfo, "", "  ")
	return string(jsonBytes)
}

func (linkInfo *LinkInfo) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &linkInfo)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

type LinkConf struct {
	Speed                  int `json:"Speed"`
	SourceDeviceId         int `json:"SourceDeviceId"`
	SourceInterfaceId      int `json:"SourceInterfaceId"`
	DestinationDeviceId    int `json:"DestinationDeviceId"`
	DestinationInterfaceId int `json:"DestinationInterfaceId"`
}

func (linkConf LinkConf) String() string {
	jsonBytes, _ := json.MarshalIndent(linkConf, "", "  ")
	return string(jsonBytes)
}

func (linkConf *LinkConf) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &linkConf)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

func (linkConf *LinkConf) CheckFeasibility() statuscode.Response {
	return statuscode.StatusOK(nil)
}

type Link struct {
	netdl.Link
	PropagationDelay int `json:"PropagationDelay"`
	// LinkInfo

	// Alive            bool   `json:"Alive"`
	// CableLength      int    `json:"CableLength"`
	// CableType        string `json:"CableType"`
	// Redundant        bool   `json:"Redundant"`
	// Speed            int    `json:"Speed"`
}

func (link *Link) String() string {
	jsonBytes, _ := json.MarshalIndent(link, "", "  ")
	return string(jsonBytes)
}

func (link *Link) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &link)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

type Links struct {
	Links []Link `json:"Links"`
}
