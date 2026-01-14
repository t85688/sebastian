package core

import (
	"fmt"
	"net/http"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

func UpdateSystem(system_conf domain.SystemConf) (system domain.System, res statuscode.Response) {
	system, res = GetSystem()
	if !res.IsSuccess() {
		return system, res
	}

	system.SystemConf = system_conf
	url := fmt.Sprintf("%s/%s/system", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	data, res := httputils.HttpClient(http.MethodPut, url, []byte(system.String()))
	if !res.IsSuccess() {
		return system, res
	}

	res = system.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return system, res
	}

	return system, statuscode.StatusOK(system)
}

func GetSystem() (system domain.System, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/system", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return system, res
	}

	res = system.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return system, res
	}

	return system, statuscode.StatusOK(system)
}
