package restfulAPI

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/api"
)

func UpdateSystem(c *api.Context) {
	var system_conf domain.SystemConf
	res := httputils.Bind(c, &system_conf)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	_, res = core.UpdateSystem(system_conf)
	httputils.Respond(c, res)
}

func GetSystem(c *api.Context) {
	_, res := core.GetSystem()
	httputils.Respond(c, res)
}
