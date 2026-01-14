package core

import (
	"fmt"
	"net/http"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

func UpdateProjectSetting(project_id int64, project_setting domain.ProjectSetting) statuscode.Response {
	url := fmt.Sprintf("%s/%s/project/%d/setting", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id)

	_, res := httputils.HttpClient(http.MethodPut, url, []byte(project_setting.String()))
	if !res.IsSuccess() {
		return res
	}

	return statuscode.StatusOK(nil)
}

func GetProjectSetting(project_id int64) (project_setting domain.ProjectSetting, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%d/setting", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id)

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return project_setting, res
	}

	res = project_setting.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return project_setting, res
	}

	return project_setting, statuscode.StatusOK(project_setting)
}
