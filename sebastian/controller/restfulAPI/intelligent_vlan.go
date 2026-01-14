package restfulAPI

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/api"
)

func GetIntelligentVlan(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	logger.Infof("Received GetIntelligentVlan request: project_id=%d", project_id)

	res = core.GetIntelligentVLAN(project_id)
	httputils.Respond(c, res)
}

func CreateIntelligentVlan(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var intelligent_vlan domain.IntelligentVlan
	res = httputils.Bind(c, &intelligent_vlan)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	logger.Infof("Received CreateIntelligentVlan request: project_id=%d, intelligent_vlan=%+v", project_id, intelligent_vlan)

	res = core.CreateIntelligentVLAN(project_id, intelligent_vlan)
	httputils.Respond(c, res)
}

func UpdateIntelligentVlan(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var vlan_id uint16
	res = httputils.ParseInt(c, "vlanId", &vlan_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var intelligent_vlan domain.IntelligentVlan
	res = httputils.Bind(c, &intelligent_vlan)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	logger.Infof("Received UpdateIntelligentVlan request: project_id=%d, vlan_id=%d, intelligent_vlan=%+v", project_id, vlan_id, intelligent_vlan)

	res = core.UpdateIntelligentVLAN(project_id, vlan_id, intelligent_vlan)
	httputils.Respond(c, res)
}

func DeleteIntelligentVlan(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var vlan_id uint16
	res = httputils.ParseInt(c, "vlanId", &vlan_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	logger.Infof("Received DeleteIntelligentVlan request: project_id=%d, vlan_id=%d", project_id, vlan_id)

	res = core.DeleteIntelligentVLAN(project_id, vlan_id)
	httputils.Respond(c, res)
}
