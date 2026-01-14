package core

import (
	"encoding/json"
	"fmt"
	"net/http"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

func CreateDevice(project_id int64, device_conf domain.DeviceConf, from_bag bool, is_operation bool) (device_info domain.Device, res statuscode.Response) {
	res = device_conf.CheckFeasibility()
	if !res.IsSuccess() {
		return device_info, res
	}

	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project/%d/device?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, httputils.QueryParams(params))

	create_device_req := domain.CreateDeviceRequest{
		DeviceConf: device_conf,
		FromBag:    from_bag,
	}

	data, res := httputils.HttpClient(http.MethodPost, url, []byte(create_device_req.String()))
	if !res.IsSuccess() {
		return device_info, res
	}

	res = device_info.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return device_info, res
	}

	return device_info, statuscode.StatusOK(device_info)
}

func UpdateDevice(project_id int64, device_id int64, device_conf *domain.DeviceConf, is_operation bool) (device *domain.Device, res statuscode.Response) {
	device, res := GetDevice(project_id, device_id, is_operation)
	if !res.IsSuccess() {
		return device_info, res
	}

	device.DeviceConf = device_conf

	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project/%d/device?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, httputils.QueryParams(params))

	data, res := httputils.HttpClient(http.MethodPut, url, []byte(device.String()))
	if !res.IsSuccess() {
		return device_info, res
	}

	res = device_info.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return device_info, res
	}

	return device_info, statuscode.StatusOK(device_info)
}

func PartialUpdateDevices(projectId int64, deviceMap map[int64]map[string]any, isOperation bool) statuscode.Response {
	if deviceMap == nil {
		return statuscode.StatusBadRequest("DeviceMap is nil", 0)
	}

	if projectId < 1 {
		return statuscode.StatusBadRequest("ProjectId is invalid", 0)
	}

	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[isOperation]}
	url := fmt.Sprintf("%s/%s/project/%d/devices?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), projectId, httputils.QueryParams(params))

	patchDevicesMap := make(map[int64]string, len(deviceMap))
	for deviceId, devicePartial := range deviceMap {
		valueJSONBytes, err := json.Marshal(devicePartial)
		if err != nil {
			return statuscode.StatusBadRequest(fmt.Errorf("failed to marshal devicePartial: %w", err).Error(), 0)
		}
		patchDevicesMap[deviceId] = string(valueJSONBytes)
	}

	type patchDevicesBody struct {
		PatchDeviceMap map[int64]string `json:"PatchDeviceMap"`
	}

	patchDeviceBody := patchDevicesBody{
		PatchDeviceMap: patchDevicesMap,
	}

	jsonBytes, err := json.Marshal(patchDeviceBody)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}

	_, res := httputils.HttpClient(http.MethodPatch, url, jsonBytes)

	return res
}

func PartialUpdateDevice(projectId int64, device *domain.DevicePartial, isOperation bool) statuscode.Response {
	if device == nil {
		return statuscode.StatusBadRequest("DevicePartial is nil", 0)
	}

	if device.Id < 1 {
		return statuscode.StatusBadRequest("DeviceId is invalid", 0)
	}

	if projectId < 1 {
		return statuscode.StatusBadRequest("ProjectId is invalid", 0)
	}

	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[isOperation]}
	url := fmt.Sprintf("%s/%s/project/%d/device?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), projectId, httputils.QueryParams(params))

	jsonBytes, err := json.Marshal(device)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}

	_, res := httputils.HttpClient(http.MethodPatch, url, jsonBytes)

	return res
}

func UpdateFullyDevice(project_id int64, device domain.Device, is_operation bool) (res_device domain.Device, res statuscode.Response) {
	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project/%d/device?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, httputils.QueryParams(params))

	data, res := httputils.HttpClient(http.MethodPut, url, []byte(device.String()))
	if !res.IsSuccess() {
		return res_device, res
	}

	res = res_device.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return res_device, res
	}

	return res_device, statuscode.StatusOK(res_device)
}

func GetDevice(project_id int64, device_id int64, is_operation bool) (device domain.Device, res statuscode.Response) {
	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project/%d/device/%d?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, device_id, httputils.QueryParams(params))

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return device, res
	}

	res = device.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return device, res
	}

	return device, statuscode.StatusOK(device)
}

func GetSimpleDevices(project_id int64, is_operation bool) (devices []domain.Device, res statuscode.Response) {
	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project/%d/simple-devices?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, httputils.QueryParams(params))

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return devices, res
	}

	var simpleDevices domain.SimpleDevices

	err := json.Unmarshal(data, &simpleDevices)
	if err != nil {
		return devices, statuscode.StatusBadRequest(err.Error(), 0)
	}

	devices = simpleDevices.Devices
	return devices, statuscode.StatusOK(devices)
}

func GetDevices(project_id int64, is_operation bool) (devices []domain.Device, res statuscode.Response) {
	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project/%d/devices?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, httputils.QueryParams(params))

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return devices, res
	}

	err := json.Unmarshal(data, &devices)
	if err != nil {
		return devices, statuscode.StatusBadRequest(err.Error(), 0)
	}

	return devices, statuscode.StatusOK(devices)
}

func DeleteDevice(project_id, device_id int64, is_operation bool) (res statuscode.Response) {
	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project/%d/device/%d?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, device_id, httputils.QueryParams(params))
	_, res = httputils.HttpClient(http.MethodDelete, url, nil)
	return res
}

func DeleteDevices(project_id int64, device_ids []int64, is_operation bool) (res statuscode.Response) {
	type requestBody struct {
		DeviceIds []int64 `json:"DeviceIds"`
	}

	if len(device_ids) == 0 {
		return statuscode.StatusOK(nil)
	}

	req := requestBody{
		DeviceIds: device_ids,
	}

	bodyBytes, err := json.Marshal(req)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}

	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project/%d/devices?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, httputils.QueryParams(params))
	_, res = httputils.HttpClient(http.MethodDelete, url, bodyBytes)
	return res
}

func DeleteAllDevices(project_id int64, is_operation bool) (res statuscode.Response) {
	devices, res := GetSimpleDevices(project_id, is_operation)
	if !res.IsSuccess() {
		return res
	}

	device_ids := make([]int64, 0, len(devices))
	for _, device := range devices {
		device_ids = append(device_ids, device.Id)
	}

	res = DeleteDevices(project_id, device_ids, is_operation)
	if !res.IsSuccess() {
		return res
	}

	return statuscode.StatusOK(nil)
}

func GetProjectDeployDevices(projectId int64) (deployDeviceList domain.DeployDeviceList, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%d/deploy-device-list", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), projectId)
	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return deployDeviceList, res
	}

	err := json.Unmarshal(data, &deployDeviceList)
	if err != nil {
		// 若解析失敗，回傳 400 BadRequest
		return deployDeviceList, statuscode.StatusBadRequest(err.Error(), statuscode.MessageCode_HttpRequestSuccess)
	}

	return deployDeviceList, statuscode.StatusOK(deployDeviceList)
}

func GetDesignBaselineDeployDevices(projectId int64, designBaselineId int64) (deployDeviceList domain.DeployDeviceList, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/project/%d/design/baseline/%d/deploy-device-list", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), projectId, designBaselineId)

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return deployDeviceList, res
	}

	err := json.Unmarshal(data, &deployDeviceList)
	if err != nil {
		// 若解析失敗，回傳 400 BadRequest
		return deployDeviceList, statuscode.StatusBadRequest(err.Error(), statuscode.MessageCode_HttpRequestSuccess)
	}

	return deployDeviceList, statuscode.StatusOK(deployDeviceList)
}
