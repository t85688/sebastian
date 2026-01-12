package core

import (
	"encoding/json"
	"fmt"
	"net/http"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

func GetSimpleDeviceProfiles() (*domain.SimpleDeviceProfiles, statuscode.Response) {
	// 設定目標 API URL
	url := fmt.Sprintf("%s/%s/simple-device-profiles", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	data, status := httputils.HttpClient(http.MethodGet, url, nil)
	if !status.IsSuccess() {
		// 若解析失敗，回傳 400 BadRequest
		return nil, status
	}

	var profiles domain.SimpleDeviceProfiles
	err := json.Unmarshal(data, &profiles)
	if err != nil {
		// 若解析失敗，回傳 400 BadRequest
		return nil, statuscode.StatusBadRequest(err.Error(), statuscode.MessageCode_HttpRequestSuccess)
	}

	return &profiles, statuscode.StatusOK(nil)
}

func GetDeviceProfilesWithDefaultDeviceConfig() (*domain.DeviceProfilesWithDefaultDeviceConfig, statuscode.Response) {
	// 設定目標 API URL
	url := fmt.Sprintf("%s/%s/device-profiles-with-default-device-config", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	data, status := httputils.HttpClient(http.MethodGet, url, nil)
	if !status.IsSuccess() {
		// 若解析失敗，回傳 400 BadRequest
		return nil, status
	}

	var profiles domain.DeviceProfilesWithDefaultDeviceConfig
	err := json.Unmarshal(data, &profiles)
	if err != nil {
		// 若解析失敗，回傳 400 BadRequest
		return nil, statuscode.StatusBadRequest(err.Error(), statuscode.MessageCode_HttpRequestSuccess)
	}

	return &profiles, statuscode.StatusOK(nil)
}

func GetPowerDeviceProfiles() (*domain.PowerDeviceProfiles, statuscode.Response) {
	// 設定目標 API URL
	url := fmt.Sprintf("%s/api/v1/power-devices", GetCogsworthHttpEndpoint())

	data, status := httputils.HttpClient(http.MethodGet, url, nil)
	if !status.IsSuccess() {
		// 若解析失敗，回傳 400 BadRequest
		return nil, status
	}

	var profiles domain.PowerDeviceProfiles
	err := json.Unmarshal(data, &profiles)
	if err != nil {
		// 若解析失敗，回傳 400 BadRequest
		return nil, statuscode.StatusBadRequest(err.Error(), statuscode.MessageCode_HttpRequestSuccess)
	}

	return &profiles, statuscode.StatusOK(nil)
}
