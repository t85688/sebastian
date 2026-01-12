package restfulAPI

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/api"
)

func GetProjectSetting(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	_, res = core.GetProjectSetting(project_id)
	httputils.Respond(c, res)
}

func UpdateProjectSetting(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var project_setting domain.ProjectSetting
	res = httputils.Bind(c, &project_setting)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	res = core.UpdateProjectSetting(project_id, project_setting)
	httputils.Respond(c, res)
}

func PatchProjectSetting(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	project_setting, res := core.GetProjectSetting(project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	res = httputils.Bind(c, &project_setting)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	res = core.UpdateProjectSetting(project_id, project_setting)
	httputils.Respond(c, res)
}
