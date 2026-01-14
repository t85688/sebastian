package core

import (
	"encoding/json"
	"fmt"
	"net/http"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

func GetSFPModules() (*domain.SFPModules, statuscode.Response) {
	// 設定目標 API URL
	url := fmt.Sprintf("%s/%s/sfp-modules", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	data, status := httputils.HttpClient(http.MethodGet, url, nil)
	if !status.IsSuccess() {
		// 若解析失敗，回傳 400 BadRequest
		return nil, status
	}

	var modules domain.SFPModules
	err := json.Unmarshal(data, &modules)
	if err != nil {
		// 若解析失敗，回傳 400 BadRequest
		return nil, statuscode.StatusBadRequest(err.Error(), statuscode.MessageCode_HttpRequestSuccess)
	}

	return &modules, statuscode.StatusOK(nil)
}

func GetSFPModuleNameIdMap() (map[string]int64, statuscode.Response) {
	resultMap := make(map[string]int64) // ModelName, Id

	modules, status := GetSFPModules()
	if !status.IsSuccess() {
		return resultMap, status
	}

	for _, module := range modules.SFPModuleMap {
		resultMap[module.ModuleName] = module.Id
	}
	return resultMap, statuscode.StatusOK(nil)
}
