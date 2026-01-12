package core

import (
	"fmt"
	"net/http"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

func Login(login domain.Login) (token domain.Token, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/login", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	data, res := httputils.HttpClient(http.MethodPost, url, []byte(login.String()))
	if !res.IsSuccess() {
		return token, res
	}

	res = token.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return token, res
	}

	return token, statuscode.StatusOK(token)
}

func CheckTokenExist() (token domain.Token, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/login/check", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return token, res
	}

	res = token.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return token, res
	}

	return token, statuscode.StatusOK(token)
}

func RenewToken() (token domain.Token, res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/renew", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return token, res
	}

	res = token.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return token, res
	}

	return token, statuscode.StatusOK(token)
}

func Logout() (res statuscode.Response) {
	url := fmt.Sprintf("%s/%s/logout", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix())

	_, res = httputils.HttpClient(http.MethodPost, url, nil)
	return res
}
