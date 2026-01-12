package restfulAPI

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/api"
)

func CreateProject(c *api.Context) {
	var project_conf domain.ProjectConf
	res := httputils.Bind(c, &project_conf)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	_, res = core.CreateProject(project_conf)
	httputils.Respond(c, res)
}

func GetProjectInfos(c *api.Context) {
	_, res := core.GetProjectInfos()

	logger.Debugf("Received GetProjectInfos request")
	httputils.Respond(c, res)
}

func GetProject(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var mode []string
	res = httputils.GetQueryArray(c, "mode", &mode, false)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	is_operation := len(mode) > 0 && mode[0] == "operation"
	show_password := false
	_, res = core.GetProject(project_id, is_operation, show_password)
	httputils.Respond(c, res)
}

func UpdateProject(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var project_conf domain.ProjectConf
	res = httputils.Bind(c, &project_conf)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var mode []string
	res = httputils.GetQueryArray(c, "mode", &mode, false)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	is_operation := len(mode) > 0 && mode[0] == "operation"
	_, res = core.UpdateProject(project_id, project_conf, is_operation)
	httputils.Respond(c, res)
}

func PatchUpdateProject(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var mode []string
	res = httputils.GetQueryArray(c, "mode", &mode, false)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	is_operation := len(mode) > 0 && mode[0] == "operation"
	show_password := false
	project, res := core.GetProject(project_id, is_operation, show_password)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var project_conf domain.ProjectConf
	project_conf.Description = project.ProjectInfo.Description
	project_conf.ProjectName = project.ProjectSetting.ProjectName
	project_conf.UserId = project.ProjectInfo.UserId
	project_conf.Profile = project.ProjectInfo.Profile

	res = httputils.Bind(c, &project_conf)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	_, res = core.UpdateProject(project_id, project_conf, is_operation)
	httputils.Respond(c, res)
}

func DeleteProjects(c *api.Context) {
	var project_ids []int64
	res := httputils.GetQueryArray(c, "ids", &project_ids, true)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	res = core.DeleteProjects(project_ids)
	httputils.Respond(c, res)
}

func CopyProject(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	_, res = core.CopyProject(project_id)
	httputils.Respond(c, res)
}

func ExportProject(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	_, res = core.ExportProject(project_id)
	httputils.Respond(c, res)
}

func ImportProject(c *api.Context) {
	var project domain.Project
	res := httputils.ReadAll(c, &project)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	res = core.ImportProject(project)
	httputils.Respond(c, res)
}

func UpdateProjectStatus(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var value string
	res = httputils.ReadAll(c, &value)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	project_status, res := domain.ParseProjectStatus(value)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	res = core.UpdateProjectStatus(project_id, project_status)
	httputils.Respond(c, res)
}
