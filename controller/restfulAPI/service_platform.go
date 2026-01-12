package restfulAPI

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/api"
)

func GetServicePlatformContracts(c *api.Context) {
	var device_code string
	res := httputils.GetQuery(c, "device_code", &device_code, true)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	res = core.GetServicePlatformContracts(device_code)
	httputils.Respond(c, res)
}

func GetServicePlatformPrices(c *api.Context) {
	var device_code string
	res := httputils.GetQuery(c, "device_code", &device_code, true)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var param domain.ServicePlatformPricesParam
	res = httputils.Bind(c, &param)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	res = core.GetServicePlatformPrices(device_code, param)
	httputils.Respond(c, res)
}

func RegisterServicePlatformDesignHistory(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var device_code string
	res = httputils.GetQuery(c, "device_code", &device_code, true)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	res = core.CreateServicePlatformProject(device_code, project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var param domain.ServicePlatformDesignHistoryParam
	res = httputils.Bind(c, &param)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	res = core.RegisterServicePlatformDesignHistory(device_code, project_id, param)
	httputils.Respond(c, res)
}
