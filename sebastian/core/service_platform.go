package core

import (
	"context"
	"encoding/json"
	"fmt"
	"strconv"
	"sync"
	"time"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

const (
	clientID     = "chamberlain-client-v1"
	clientSecret = "chamberlain@admin@123"
	scope        = "openid profile email user:auth contract:read project"
)

var gServicePlatformTokenMap = make(map[string]domain.ServicePlatformTokenHandler)
var gMutex sync.Mutex

func RequestServicePlatformDeviceCode() (res statuscode.Response) {
	path, err := internal.GetAccountServiceEndpointFromEnviron()
	if err != nil {
		logger.Infoln("Get Account Service Platform Endpoint failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}
	url := path + "/realms/moxa-crm/protocol/openid-connect/auth/device"
	logger.Infoln("Service Platform: POST %s", url)

	client := httputils.GetClient()
	resp, err := client.R().
		SetHeaders(map[string]string{
			"Content-Type": "application/x-www-form-urlencoded",
		}).
		SetFormData(map[string]string{
			"client_id":     clientID,
			"client_secret": clientSecret,
			"scope":         scope,
		}).
		Post(url)

	if err != nil {
		logger.Infoln("HTTP request failed: %v", err)
		return statuscode.StatusRequestFailed(fmt.Sprintf("HTTP request failed: %v", err))
	} else if resp.IsError() {
		// HTTP 4xx / 5xx
		response := domain.ServicePlatformAuthErrorResp{}
		err = json.Unmarshal(resp.Body(), &response)
		if err != nil {
			logger.Infoln("Unmarshal Auth error response failed: %s", err.Error())
			return statuscode.StatusRequestFailed(err.Error())
		}
		logger.Infoln("Auth error response: %s", response)
		return statuscode.StatusRequestFailed(fmt.Sprintf("Service Platform Auth failed: %s", response))
	}

	response := domain.ServicePlatformAuthResp{}
	err = json.Unmarshal(resp.Body(), &response)
	if err != nil {
		logger.Infoln("Unmarshal Auth response failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}

	return statuscode.StatusOK(response)
}

func LogoutServicePlatform(ctx context.Context, refreshToken string) (res statuscode.Response) {
	ctx, cancel := context.WithTimeout(ctx, 30*time.Second)
	defer cancel()

	path, err := internal.GetAccountServiceEndpointFromEnviron()
	if err != nil {
		logger.Infoln("Get Account Service Platform Endpoint failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}
	url := path + "/realms/moxa-crm/protocol/openid-connect/logout"
	logger.Infoln("Service Platform: POST %s", url)

	client := httputils.GetClient()
	resp, err := client.R().
		SetContext(ctx).
		SetHeaders(map[string]string{
			"Content-Type": "application/x-www-form-urlencoded",
		}).
		SetFormData(map[string]string{
			"client_id":     clientID,
			"client_secret": clientSecret,
			"refresh_token": refreshToken,
		}).
		Post(url)

	if err != nil {
		logger.Infoln("HTTP request failed: %v", err)
		return statuscode.StatusRequestFailed(fmt.Sprintf("HTTP request failed: %v", err))
	} else if resp.IsError() {
		// HTTP 4xx / 5xx
		response := domain.ServicePlatformAuthErrorResp{}
		err = json.Unmarshal(resp.Body(), &response)
		if err != nil {
			logger.Infoln("Unmarshal Auth error response failed: %s", err.Error())
			return statuscode.StatusRequestFailed(err.Error())
		}
		logger.Infoln("Auth error response: %s", response)
		return statuscode.StatusRequestFailed(fmt.Sprintf("Service Platform Auth failed: %s", response))
	}

	return statuscode.StatusNoContent()
}

func PostServicePlatformToken(ctx context.Context, deviceCode string) (res statuscode.Response) {
	ctx, cancel := context.WithTimeout(ctx, 30*time.Second)
	defer cancel()

	path, err := internal.GetAccountServiceEndpointFromEnviron()
	if err != nil {
		logger.Infoln("Get Account Service Platform Endpoint failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}
	url := path + "/realms/moxa-crm/protocol/openid-connect/token"
	logger.Infoln("Service Platform: POST %s", url)

	client := httputils.GetClient()
	resp, err := client.R().
		SetContext(ctx).
		SetHeaders(map[string]string{
			"Content-Type": "application/x-www-form-urlencoded",
		}).
		SetFormData(map[string]string{
			"grant_type":    "urn:ietf:params:oauth:grant-type:device_code",
			"device_code":   deviceCode,
			"client_id":     clientID,
			"client_secret": clientSecret,
		}).
		Post(url)

	if err != nil {
		logger.Infoln("HTTP request failed: %v", err)
		return statuscode.StatusRequestFailed(fmt.Sprintf("HTTP request failed: %v", err))
	} else if resp.IsError() {
		// HTTP 4xx / 5xx
		response := domain.ServicePlatformAuthErrorResp{}
		err = json.Unmarshal(resp.Body(), &response)
		if err != nil {
			logger.Infoln("Unmarshal Auth error response failed: %s", err.Error())
			return statuscode.StatusRequestFailed(err.Error())
		}
		logger.Infoln("Auth error response: %s", response)
		return statuscode.StatusRequestFailed(fmt.Sprintf("Service Platform Auth failed: %s", response))
	}

	result := domain.ServicePlatformTokenResp{}
	err = json.Unmarshal(resp.Body(), &result)
	if err != nil {
		logger.Infoln("Unmarshal token response failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}

	token := domain.ServicePlatformTokenHandler{
		DeviceCode:   deviceCode,
		AccessToken:  result.AccessToken,
		RefreshToken: result.RefreshToken,
		ExpireAt:     time.Now().Add(time.Duration(result.ExpiresIn-60) * time.Second),
	}

	SetServicePlatformToken(token)

	return statuscode.StatusOK(token)
}

func RefreshServicePlatformToken(ctx context.Context, token domain.ServicePlatformTokenHandler) (res statuscode.Response) {
	ctx, cancel := context.WithTimeout(ctx, 30*time.Second)
	defer cancel()

	path, err := internal.GetAccountServiceEndpointFromEnviron()
	if err != nil {
		logger.Infoln("Get Account Service Platform Endpoint failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}

	url := path + "/realms/moxa-crm/protocol/openid-connect/token"
	logger.Infoln("Service Platform: POST %s", url)

	client := httputils.GetClient()
	resp, err := client.R().
		SetContext(ctx).
		SetHeaders(map[string]string{
			"Content-Type": "application/x-www-form-urlencoded",
		}).
		SetFormData(map[string]string{
			"grant_type":    "refresh_token",
			"refresh_token": token.RefreshToken,
			"client_id":     clientID,
			"client_secret": clientSecret,
		}).
		Post(url)

	if err != nil {
		logger.Infoln("HTTP request failed: %v", err)
		return statuscode.StatusRequestFailed(fmt.Sprintf("HTTP request failed: %v", err))
	} else if resp.IsError() {
		// HTTP 4xx / 5xx
		response := domain.ServicePlatformAuthErrorResp{}
		err = json.Unmarshal(resp.Body(), &response)
		if err != nil {
			logger.Infoln("Unmarshal Auth error response failed: %s", err.Error())
			return statuscode.StatusRequestFailed(err.Error())
		}
		logger.Infoln("Auth error response: %s", response)
		return statuscode.StatusRequestFailed(fmt.Sprintf("Service Platform Auth failed: %s", response))
	}

	result := domain.ServicePlatformTokenResp{}
	err = json.Unmarshal(resp.Body(), &result)
	if err != nil {
		logger.Infoln("Unmarshal token response failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}

	token = domain.ServicePlatformTokenHandler{
		DeviceCode:   token.DeviceCode,
		AccessToken:  result.AccessToken,
		RefreshToken: result.RefreshToken,
		ExpireAt:     time.Now().Add(time.Duration(result.RefreshExpiresIn-60) * time.Second),
	}

	SetServicePlatformToken(token)

	return statuscode.StatusOK(token)
}

func GetServicePlatformEndpoint() (res statuscode.Response) {
	path, err := internal.GetServicePlatformEndpointFromEnviron()
	if err != nil {
		logger.Infoln("Get Service Platform Endpoint failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}

	return statuscode.StatusOK(path)
}

func GetServicePlatformContracts(deviceCode string) (res statuscode.Response) {
	path, err := internal.GetServicePlatformEndpointFromEnviron()
	if err != nil {
		logger.Infoln("Get Service Platform Endpoint failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}

	url := path + "/api/v1/crm/contracts"
	logger.Infoln("Service Platform: GET %s", url)

	token, ok := GetServicePlatformToken(deviceCode)
	if !ok {
		err := fmt.Sprintf("Unauthorized device code: %s", deviceCode)
		logger.Infoln(err)
		return statuscode.StatusRequestFailed(err)
	}

	client := httputils.GetClient()
	resp, err := client.R().
		SetAuthToken(token.AccessToken).
		Get(url)

	if err != nil {
		return statuscode.StatusRequestFailed(fmt.Sprintf("HTTP request failed: %v", err))
	} else if resp.IsError() {
		// HTTP 4xx / 5xx
		response := domain.ServicePlatformCRMErrorResp{}
		err = json.Unmarshal(resp.Body(), &response)
		if err != nil {
			logger.Infoln("Unmarshal CRM error response failed: %s", err.Error())
			return statuscode.StatusRequestFailed(err.Error())
		}
		logger.Infoln("CRM error response: %s", response.String())
		return statuscode.StatusRequestFailed(response.Error.Message)
	}

	response := domain.ServicePlatformCRMResp{}
	err = json.Unmarshal(resp.Body(), &response)
	if err != nil {
		logger.Infoln("Unmarshal CRM response failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}

	return statuscode.StatusOK(response.Data)
}

func GetServicePlatformPrices(deviceCode string, param domain.ServicePlatformPricesParam) (res statuscode.Response) {
	path, err := internal.GetServicePlatformEndpointFromEnviron()
	if err != nil {
		logger.Infoln("Get Service Platform Endpoint failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}

	url := path + "/api/v1/crm/prices"
	logger.Infoln("Service Platform: POST %s", url)

	token, ok := GetServicePlatformToken(deviceCode)
	if !ok {
		err := fmt.Sprintf("Unauthorized device code: %s", deviceCode)
		logger.Infoln(err)
		return statuscode.StatusRequestFailed(err)
	}

	client := httputils.GetClient()
	resp, err := client.R().
		SetAuthToken(token.AccessToken).
		SetHeaders(map[string]string{
			"Content-Type": "application/json",
		}).
		SetBody(param).
		Post(url)

	if err != nil {
		return statuscode.StatusRequestFailed(fmt.Sprintf("HTTP request failed: %v", err))
	} else if resp.IsError() {
		// HTTP 4xx / 5xx
		response := domain.ServicePlatformCRMErrorResp{}
		err = json.Unmarshal(resp.Body(), &response)
		if err != nil {
			logger.Infoln("Unmarshal CRM error response failed: %s", err.Error())
			return statuscode.StatusRequestFailed(err.Error())
		}
		logger.Infoln("CRM error response: %s", response.String())
		return statuscode.StatusRequestFailed(response.Error.Message)
	}

	response := domain.ServicePlatformCRMResp{}
	err = json.Unmarshal(resp.Body(), &response)
	if err != nil {
		logger.Infoln("Unmarshal CRM response failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}

	return statuscode.StatusOK(response.Data)
}

func CreateServicePlatformProject(deviceCode string, projectId int64) (res statuscode.Response) {
	path, err := internal.GetServicePlatformEndpointFromEnviron()
	if err != nil {
		logger.Infoln("Get Service Platform Endpoint failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}

	url := path + "/api/v1/chamberlain/projects"
	logger.Infoln("Service Platform: POST %s", url)

	project, res := GetProject(projectId, false, false)
	if !res.IsSuccess() {
		return res
	}

	token, ok := GetServicePlatformToken(deviceCode)
	if !ok {
		err := fmt.Sprintf("Unauthorized device code: %s", deviceCode)
		logger.Infoln(err)
		return statuscode.StatusRequestFailed(err)
	}

	client := httputils.GetClient()
	resp, err := client.R().
		SetAuthToken(token.AccessToken).
		SetHeaders(map[string]string{
			"Content-Type": "application/json",
		}).
		SetBody(map[string]interface{}{
			"clientProjectId": projectId,
			"name":            project.ProjectSetting.ProjectName,
		}).
		Post(url)

	if err != nil {
		return statuscode.StatusRequestFailed(fmt.Sprintf("HTTP request failed: %v", err))
	} else if resp.IsError() {
		// HTTP 4xx / 5xx
		response := domain.ServicePlatformCRMErrorResp{}
		err = json.Unmarshal(resp.Body(), &response)
		if err != nil {
			logger.Infoln("Unmarshal CRM error response failed: %s", err.Error())
			return statuscode.StatusRequestFailed(err.Error())
		}
		logger.Infoln("CRM error response: %s", response.String())
		return statuscode.StatusRequestFailed(response.Error.Message)
	}

	response := domain.ServicePlatformCRMResp{}
	err = json.Unmarshal(resp.Body(), &response)
	if err != nil {
		logger.Infoln("Unmarshal CRM response failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}

	logger.Infoln("Create project response: %s", response.String())

	return statuscode.StatusOK(response.Data)
}

func RegisterServicePlatformDesignHistory(deviceCode string, projectId int64, param domain.ServicePlatformDesignHistoryParam) (res statuscode.Response) {
	path, err := internal.GetServicePlatformEndpointFromEnviron()
	if err != nil {
		logger.Infoln("Get Service Platform Endpoint failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}

	url := path + "/api/v1/chamberlain/design-history-records"
	logger.Infoln("Service Platform: POST %s", url)

	token, ok := GetServicePlatformToken(deviceCode)
	if !ok {
		err := fmt.Sprintf("Unauthorized device code: %s", deviceCode)
		logger.Infoln(err)
		return statuscode.StatusRequestFailed(err)
	}

	client := httputils.GetClient()
	resp, err := client.R().
		SetAuthToken(token.AccessToken).
		SetHeaders(map[string]string{
			"Content-Type": "application/json",
		}).
		SetQueryParam("clientProjectId", strconv.FormatInt(projectId, 10)).
		SetBody(param).
		Post(url)

	if err != nil {
		return statuscode.StatusRequestFailed(fmt.Sprintf("HTTP request failed: %v", err))
	} else if resp.IsError() {
		// HTTP 4xx / 5xx
		response := domain.ServicePlatformCRMErrorResp{}
		err = json.Unmarshal(resp.Body(), &response)
		if err != nil {
			logger.Infoln("Unmarshal CRM error response failed: %s", err.Error())
			return statuscode.StatusRequestFailed(err.Error())
		}
		logger.Infoln("CRM error response: %s", response.String())
		return statuscode.StatusRequestFailed(response.Error.Message)
	}

	response := domain.ServicePlatformCRMResp{}
	err = json.Unmarshal(resp.Body(), &response)
	if err != nil {
		logger.Infoln("Unmarshal CRM response failed: %s", err.Error())
		return statuscode.StatusRequestFailed(err.Error())
	}

	return statuscode.StatusOK(response.Data)
}

func GetServicePlatformToken(deviceCode string) (token domain.ServicePlatformTokenHandler, ok bool) {
	gMutex.Lock()
	token, ok = gServicePlatformTokenMap[deviceCode]
	gMutex.Unlock()
	return token, ok
}

func SetServicePlatformToken(token domain.ServicePlatformTokenHandler) {
	gMutex.Lock()
	gServicePlatformTokenMap[token.DeviceCode] = token
	gMutex.Unlock()
}

func DeleteServicePlatformToken(deviceCode string) {
	gMutex.Lock()
	delete(gServicePlatformTokenMap, deviceCode)
	gMutex.Unlock()
}
