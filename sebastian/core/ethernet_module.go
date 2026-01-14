package core

import (
	"encoding/json"
	"fmt"
	"net/http"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

func GetEthernetModules() (*domain.EthernetModules, statuscode.Response) {
	// 設定目標 API URL
	url := fmt.Sprintf("%s/%s/ethernet-modules", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	data, status := httputils.HttpClient(http.MethodGet, url, nil)
	if !status.IsSuccess() {
		// 若解析失敗，回傳 400 BadRequest
		return nil, status
	}

	var modules domain.EthernetModules
	err := json.Unmarshal(data, &modules)
	if err != nil {
		// 若解析失敗，回傳 400 BadRequest
		return nil, statuscode.StatusBadRequest(err.Error(), statuscode.MessageCode_HttpRequestSuccess)
	}

	return &modules, statuscode.StatusOK(nil)
}

func GetEthernetModuleNameIdMap() (map[string]int64, statuscode.Response) {
	resultMap := make(map[string]int64) // ModelName, Id

	modules, status := GetEthernetModules()
	if !status.IsSuccess() {
		return resultMap, status
	}

	for _, module := range modules.EthernetModuleMap {
		resultMap[module.ModuleName] = module.Id
	}
	return resultMap, statuscode.StatusOK(nil)
}
