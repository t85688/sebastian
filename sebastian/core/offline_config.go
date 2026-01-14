package core

import (
	"fmt"
	"net/http"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

func GenerateDevicesOfflineConfig(projectId int64, designBaselineId int64, deviceIds []int64) (deviceOfflineConfigMap map[int64]string, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%d/design/baseline/%d/generate-deploy-device-config", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), projectId, designBaselineId)

	deviceIdsReuqest := domain.DeviceIds{
		DeviceIds: deviceIds,
	}

	// 將 project_conf 轉成 JSON 字串後送出 POST 請求
	data, res := httputils.HttpClient(http.MethodPost, url, []byte(deviceIdsReuqest.String()))
	if !res.IsSuccess() {
		return nil, res
	}

	// 將回傳 JSON 解析為 Project 結構
	var response_map domain.DeviceOfflineConfigFileMap
	res = response_map.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return nil, res
	}

	deviceOfflineConfigMap = response_map.DeviceOfflineConfigFileMap
	return deviceOfflineConfigMap, statuscode.StatusOK(nil)
}
