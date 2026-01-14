package domain

import (
	"encoding/json"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

type Login struct {
	Username string `json:"Username"`
	Password string `json:"Password"`
}

func (login Login) String() string {
	jsonBytes, _ := json.MarshalIndent(login, "", "  ")
	return string(jsonBytes)
}

func (login *Login) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &login)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

type Token struct {
	Token string `json:"Token"`
	Role  string `json:"Role"`
}

func (token Token) String() string {
	jsonBytes, _ := json.MarshalIndent(token, "", "  ")
	return string(jsonBytes)
}

func (token *Token) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &token)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}
