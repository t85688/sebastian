package domain

import (
	"encoding/json"
	
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

type SystemInfo struct {
}

type SystemConf struct {
}

type System struct {
	SystemInfo
	SystemConf

	Projects map[int64]Project `json:"Projects"`
}

func (system System) String() string {
	jsonBytes, _ := json.MarshalIndent(system, "", "  ")
	return string(jsonBytes)
}

func (system *System) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &system)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

