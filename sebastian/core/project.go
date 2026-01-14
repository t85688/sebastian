package core

import (
	"encoding/json"
	"fmt"
	"net/http"
	"os"
	"path/filepath"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/licensemanager"
)

func CreateProject(project_conf domain.ProjectConf) (project_info domain.ProjectInfo, res statuscode.Response) {
	res = project_conf.CheckFeasibility()
	if !res.IsSuccess() {
		return project_info, res
	}

	url := fmt.Sprintf("%s/%s/project", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	data, res := httputils.HttpClient(http.MethodPost, url, []byte(project_conf.String()))
	if !res.IsSuccess() {
		return project_info, res
	}

	res = project_info.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return project_info, res
	}

	return project_info, statuscode.StatusCreated(project_info)
}

func GetProjectInfo(projectId int64) (project_info domain.ProjectInfo, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/simple-project/%d", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), projectId)

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return project_info, res
	}

	res = project_info.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return project_info, res
	}

	return project_info, statuscode.StatusOK(project_info)
}

func GetProjectInfos() (project_infos []domain.ProjectInfo, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/simple-projects", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return project_infos, res
	}

	var simple_projects domain.ActSimpleProjects
	res = simple_projects.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return project_infos, res
	}

	project_infos = simple_projects.SimpleProjectSet
	return project_infos, statuscode.StatusOK(project_infos)
}

func GetProject(project_id int64, is_operation bool, show_password bool) (project domain.Project, res statuscode.Response) {
	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation], "showPassword": show_password}
	url := fmt.Sprintf("%s/%s/project/%d?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, httputils.QueryParams(params))

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return project, res
	}

	res = project.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return project, res
	}

	return project, statuscode.StatusOK(project)
}

func UpdateFullProject(project *domain.Project, is_operation bool) (res statuscode.Response) {
	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), httputils.QueryParams(params))

	jsonBytes, err := json.Marshal(project)
	if err != nil {
		return statuscode.StatusRequestFailed(err.Error())
	}

	_, res = httputils.HttpClient(http.MethodPut, url, jsonBytes)
	return res
}

func UpdateProject(project_id int64, project_conf domain.ProjectConf, is_operation bool) (project_info domain.ProjectInfo, res statuscode.Response) {
	res = project_conf.CheckFeasibility()
	if !res.IsSuccess() {
		return project_info, res
	}

	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), httputils.QueryParams(params))

	data, res := httputils.HttpClient(http.MethodPut, url, []byte(project_conf.String()))
	if !res.IsSuccess() {
		return project_info, res
	}

	res = project_info.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return project_info, res
	}

	return project_info, statuscode.StatusOK(project_info)
}

func UpdateProjectStatus(project_id int64, status domain.ProjectStatusEnum) (res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%d/status", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id)

	_, res = httputils.HttpClient(http.MethodPut, url, []byte(status.String()))
	if !res.IsSuccess() {
		return res
	}

	return statuscode.StatusNoContent()
}

func GetProjectDeviceConfig(project_id int64) (deviceConfig domain.ProjectWithDeviceConfig, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%d", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id)

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return deviceConfig, res
	}

	res = deviceConfig.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return deviceConfig, res
	}

	return deviceConfig, statuscode.StatusOK(deviceConfig)
}

func UpdateProjectDeviceConfig(deviceConfig *domain.ProjectWithDeviceConfig, isOperation bool) (res statuscode.Response) {
	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[isOperation]}
	url := fmt.Sprintf("%s/%s/project?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), httputils.QueryParams(params))

	_, res = httputils.HttpClient(http.MethodPatch, url, []byte(deviceConfig.String()))
	if !res.IsSuccess() {
		return res
	}

	return statuscode.StatusOK(nil)
}

func UpdateProjectDevices(projectDevices domain.ProjectWithDevices) (res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	_, res = httputils.HttpClient(http.MethodPatch, url, []byte(projectDevices.String()))
	if !res.IsSuccess() {
		return res
	}

	return statuscode.StatusOK(nil)
}

func DeleteProject(project_id int64) (res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%d", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id)

	_, res = httputils.HttpClient(http.MethodDelete, url, nil)
	if !res.IsSuccess() {
		return res
	}

	return statuscode.StatusNoContent()
}

func DeleteProjects(project_ids []int64) (res statuscode.Response) {
	for _, project_id := range project_ids {
		res = DeleteProject(project_id)
		if !res.IsSuccess() {
			return res
		}
	}

	return statuscode.StatusNoContent()
}

func CopyProject(project_id int64) (project_info domain.ProjectInfo, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%d/copy", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id)

	data, res := httputils.HttpClient(http.MethodPost, url, nil)
	if !res.IsSuccess() {
		return project_info, res
	}

	res = project_info.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return project_info, res
	}

	return project_info, statuscode.StatusCreated(project_info)
}

func ExportProject(project_id int64) (project domain.Project, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%d/export", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id)

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return project, res
	}

	res = project.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return project, res
	}

	return project, statuscode.StatusOK(project)
}

func ImportProject(project domain.Project) (res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/import", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	_, res = httputils.HttpClient(http.MethodPost, url, []byte(project.String()))
	if !res.IsSuccess() {
		return res
	}

	return statuscode.StatusCreated(nil)
}

func GetProjectIPScanRanges(projectId int64) (*domain.ScanIpRanges, statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%d/setting/scan-ip-ranges", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), projectId)

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return nil, res
	}

	ranges := domain.ScanIpRanges{}
	err := json.Unmarshal(data, &ranges)
	if err != nil {
		return nil, statuscode.StatusBadRequest(err.Error(), 0)
	}

	return &ranges, statuscode.StatusOK(&ranges)
}

func GetDesignBaselineProject(project_id int64, baselineId int64) (project domain.Project, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%d/design/baseline/%d/project", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, baselineId)

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return project, res
	}

	res = project.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return project, res
	}

	return project, statuscode.StatusOK(project)
}

func GetDesignBaselineProjectDeviceConfig(project_id int64, baselineId int64) (deviceConfig domain.ProjectWithDeviceConfig, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%d/design/baseline/%d/project", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, baselineId)

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return deviceConfig, res
	}

	res = deviceConfig.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return deviceConfig, res
	}

	return deviceConfig, statuscode.StatusOK(deviceConfig)
}

func GetActivatedBaseline(projectId int64) (*domain.Project, statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%v/operation/baseline/activate/project", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), projectId)
	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return nil, res
	}

	project := &domain.Project{}
	res = project.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return nil, res
	}

	return project, statuscode.StatusOK(project)
}

func UpdateDesignBaselineProjectDevices(baselineId int64, projectDevices domain.ProjectWithDevices) (res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%d/design/baseline/%d/project", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), projectDevices.Id, baselineId)

	_, res = httputils.HttpClient(http.MethodPatch, url, []byte(projectDevices.String()))
	if !res.IsSuccess() {
		return res
	}

	return statuscode.StatusOK(nil)
}

func GetLicenseInformation() (res statuscode.Response) {
	licenseV2Path, err := internal.GetLicenseV2PathfromEnviron()
	if err != nil {
		return statuscode.StatusInternalError(err.Error())
	}

	if _, err := os.Stat(licenseV2Path); os.IsNotExist(err) {
		return statuscode.StatusBadRequest("can not get existing license information", 0)
	}

	licenseManager := licensemanager.GetInstance()
	licenseV2, err := licenseManager.ReadLicenseV2FromFile(licenseV2Path)
	if err != nil {
		return statuscode.StatusInternalError(err.Error())
	}

	licenseV2Info := &struct {
		EffectiveDate      *licensemanager.LicenseV2Date                   `json:"EffectiveDate,omitempty"`
		ExpiryDate         *licensemanager.LicenseV2Date                   `json:"ExpiryDate,omitempty"`
		GrantType          licensemanager.LicenseV2GrantType               `json:"GrantType"`
		HostDeploymentType licensemanager.LicenseV2HostDeploymentType      `json:"HostDeploymentType"`
		NodeCount          int                                             `json:"NodeCount"`
		ProjectProfiles    licensemanager.LicenseV2ProjectProfileTypeSlice `json:"ProjectProfiles"`
		ProjectStages      licensemanager.LicenseV2ProjectStageTypeSlice   `json:"ProjectStages"`
		FeatureProfiles    []licensemanager.LicenseV2FeatureProfileType    `json:"FeatureProfiles"`
	}{
		EffectiveDate:      licenseV2.EffectiveDate,
		ExpiryDate:         licenseV2.ExpiryDate,
		GrantType:          licenseV2.GrantType,
		HostDeploymentType: licenseV2.HostDeploymentType,
		NodeCount:          licenseV2.NodeCount,
		ProjectProfiles:    licenseV2.ProjectProfiles,
		ProjectStages:      licenseV2.ProjectStages,
		FeatureProfiles:    licenseV2.FeatureProfiles,
	}
	return statuscode.StatusOK(licenseV2Info)
}

func ImportLicenseActivationCode(activationCode domain.ActivationCode) (res statuscode.Response) {
	rawString := activationCode.RawString
	licenseManager := licensemanager.GetInstance()
	licenseV2, err := licenseManager.ReadLicenseV2FromString(rawString)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}

	licenseV2Path, err := internal.GetLicenseV2PathfromEnviron()
	if err != nil {
		return statuscode.StatusInternalError(err.Error())
	}

	if _, err := os.Stat(licenseV2Path); os.IsNotExist(err) {
		licenseV2Dir := filepath.Dir(licenseV2Path)
		_, err = os.Stat(licenseV2Dir)
		if err != nil {
			err = os.MkdirAll(licenseV2Dir, 0755)
			if err != nil {
				return statuscode.StatusInternalError(err.Error())
			}
		}

		err = licenseManager.WriteLicenseV2ToFile(*licenseV2, licenseV2Path)
		if err != nil {
			return statuscode.StatusInternalError(err.Error())
		}
		return statuscode.StatusOK(nil)
	}

	if licenseV2.GrantType == licensemanager.LicenseV2GrantTypeTrial {
		return statuscode.StatusBadRequest("can not import trial license over existing license", 0)
	}

	err = licenseManager.WriteLicenseV2ToFile(*licenseV2, licenseV2Path)
	if err != nil {
		return statuscode.StatusInternalError(err.Error())
	}
	return statuscode.StatusOK(nil)
}
