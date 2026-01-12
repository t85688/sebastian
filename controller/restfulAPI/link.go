package restfulAPI

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/api"
)

func CreateLink(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var link_conf domain.LinkConf
	res = httputils.Bind(c, &link_conf)
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
	_, res = core.CreateLink(project_id, link_conf, is_operation)
	httputils.Respond(c, res)
}

func UpdateLink(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var link_id int64
	res = httputils.ParseInt(c, "linkId", &link_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var link_conf domain.LinkConf
	res = httputils.Bind(c, &link_conf)
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
	_, res = core.UpdateLink(project_id, link_id, link_conf, is_operation)
	httputils.Respond(c, res)
}

func PatchLink(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var link_id int64
	res = httputils.ParseInt(c, "linkId", &link_id)
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
	link, res := core.GetLink(project_id, link_id, is_operation)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	res = httputils.Bind(c, &link.LinkConf)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	_, res = core.UpdateLink(project_id, link_id, link.LinkConf, is_operation)
	httputils.Respond(c, res)
}

func GetLink(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var link_id int64
	res = httputils.ParseInt(c, "linkId", &link_id)
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
	_, res = core.GetLink(project_id, link_id, is_operation)
	httputils.Respond(c, res)
}

func DeleteLinks(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var link_ids []int64
	res = httputils.GetQueryArray(c, "ids", &link_ids, true)
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
	res = core.DeleteLinks(project_id, link_ids, is_operation)
	httputils.Respond(c, res)
}
