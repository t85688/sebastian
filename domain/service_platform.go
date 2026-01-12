package domain

import (
	"encoding/json"
	"time"
)

type ServicePlatformAuthResp struct {
	DeviceCode              string `json:"device_code"`
	UserCode                string `json:"user_code"`
	VerificationUri         string `json:"verification_uri"`
	VerificationUriComplete string `json:"verification_uri_complete"`
	ExpiresIn               int    `json:"expires_in"`
	Interval                int    `json:"interval"`
}

func (servicePlatformAuthResp ServicePlatformAuthResp) String() string {
	jsonBytes, _ := json.MarshalIndent(servicePlatformAuthResp, "", "  ")
	return string(jsonBytes)
}

type ServicePlatformLoginResp struct {
	IsLoggedIn bool `json:"isLoggedIn"`
}

type ServicePlatformTokenResp struct {
	AccessToken      string `json:"access_token"`
	ExpiresIn        int    `json:"expires_in"`
	RefreshExpiresIn int    `json:"refresh_expires_in"`
	RefreshToken     string `json:"refresh_token"`
	TokenType        string `json:"token_type"`
	IdToken          string `json:"id_token"`
	NotBeforePolicy  int    `json:"not-before-policy"`
	SessionState     string `json:"session_state"`
	Scope            string `json:"scope"`
}

func (servicePlatformTokenResp ServicePlatformTokenResp) String() string {
	jsonBytes, _ := json.MarshalIndent(servicePlatformTokenResp, "", "  ")
	return string(jsonBytes)
}

type ServicePlatformTokenHandler struct {
	DeviceCode   string    `json:"deviceCode"`
	AccessToken  string    `json:"accessToken"`
	RefreshToken string    `json:"refreshToken"`
	ExpireAt     time.Time `json:"expireAt"`
}

type ServicePlatformCRMResp struct {
	Data   any `json:"data"`
	Count  int `json:"count"`
	Limit  int `json:"limit"`
	Offset int `json:"offset"`
}

func (servicePlatformCRMResp ServicePlatformCRMResp) String() string {
	jsonBytes, _ := json.MarshalIndent(servicePlatformCRMResp, "", "  ")
	return string(jsonBytes)
}

type ServicePlatformCRMContract struct {
	Id           string   `json:"id"`
	Name         string   `json:"name"`
	Type         string   `json:"type"`
	CurrencyCode []string `json:"currencyCode"`
}

type ServicePlatformPricesParam struct {
	ContractId   string   `json:"contractId"`
	CurrencyCode string   `json:"currencyCode"`
	Type         string   `json:"type"`
	ModelNames   []string `json:"modelNames"`
	Limit        int      `json:"limit"`
	Offset       int      `json:"offset"`
}

type ServicePlatformCRMPrice struct {
	ModelName        string  `json:"modelName"`
	Description      string  `json:"description"`
	Price            float64 `json:"price"`
	CurrencyCode     string  `json:"currencyCode"`
	ContractName     string  `json:"contractName"`
	StandardLeadTime string  `json:"standardLeadTime"`
}

type ServicePlatformProjectStatusEnum int

const (
	ServicePlatformProjectStatusEnum_NotReview ServicePlatformProjectStatusEnum = iota
	ServicePlatformProjectStatusEnum_UnderReviewing
	ServicePlatformProjectStatusEnum_ReviewCompleted
)

var ServicePlatformProjectStatusEnumToString = map[ServicePlatformProjectStatusEnum]string{
	ServicePlatformProjectStatusEnum_NotReview:       "NotReview",
	ServicePlatformProjectStatusEnum_UnderReviewing:  "UnderReviewing",
	ServicePlatformProjectStatusEnum_ReviewCompleted: "ReviewCompleted",
}

var StringToServicePlatformProjectStatusEnum = map[string]ServicePlatformProjectStatusEnum{
	"NotReview":       ServicePlatformProjectStatusEnum_NotReview,
	"UnderReviewing":  ServicePlatformProjectStatusEnum_UnderReviewing,
	"ReviewCompleted": ServicePlatformProjectStatusEnum_ReviewCompleted,
}

func (status ServicePlatformProjectStatusEnum) String() string {
	if s, ok := ServicePlatformProjectStatusEnumToString[status]; ok {
		return s
	}
	return "Unknown"
}

type ServicePlatformCRMProject struct {
	Id              string `json:"id"`
	ClientProjectId int64  `json:"clientProjectId"`
	Name            string `json:"name"`
	Status          string `json:"status"`
	CreatedBy       string `json:"createdBy"`
	CreatedAt       string `json:"createdAt"`
	UpdatedAt       string `json:"updatedAt"`
	LatestEditAt    string `json:"latestEditAt"`
}

type ServicePlatformBomItem struct {
	ModelName string  `json:"modelName"`
	Quantity  int     `json:"quantity"`
	UnitPrice float64 `json:"unitPrice"`
	Amount    float64 `json:"amount"`
	LeadTime  string  `json:"leadTime"`
}

type ServicePlatformDesignHistoryParam struct {
	Name         string                   `json:"name"`
	CurrencyCode string                   `json:"currencyCode"`
	ContractType string                   `json:"contractType"`
	ContractId   string                   `json:"contractId"`
	ContractName string                   `json:"contractName"`
	EditedAt     string                   `json:"editedAt"`
	BomItems     []ServicePlatformBomItem `json:"bomItems"`
}

func (servicePlatformDesignHistoryParam ServicePlatformDesignHistoryParam) String() string {
	jsonBytes, _ := json.MarshalIndent(servicePlatformDesignHistoryParam, "", "  ")
	return string(jsonBytes)
}

type ServicePlatformCRMDesignHistory struct {
	ContractId   string  `json:"contractId"`
	ContractName string  `json:"contractName"`
	ContractType string  `json:"contractType"`
	CreatedAt    string  `json:"createdAt"`
	CreatedBy    string  `json:"createdBy"`
	CurrencyCode string  `json:"currencyCode"`
	EditedAt     string  `json:"editedAt"`
	Id           string  `json:"id"`
	Name         string  `json:"name"`
	ProjectId    string  `json:"projectId"`
	TotalPrice   float64 `json:"totalPrice"`
	UpdatedAt    string  `json:"updatedAt"`
}

type ServicePlatformAuthErrorResp struct {
	Error            string `json:"error"`
	ErrorDescription string `json:"error_description"`
}

func (servicePlatformAuthErrorResp ServicePlatformAuthErrorResp) String() string {
	jsonBytes, _ := json.MarshalIndent(servicePlatformAuthErrorResp, "", "  ")
	return string(jsonBytes)
}

type ServicePlatformCRMErrorResp struct {
	Error ServicePlatformCRMError `json:"error"`
}

func (servicePlatformCRMErrorResp ServicePlatformCRMErrorResp) String() string {
	jsonBytes, _ := json.MarshalIndent(servicePlatformCRMErrorResp, "", "  ")
	return string(jsonBytes)
}

type ServicePlatformCRMError struct {
	Code    int    `json:"code"`
	Error   string `json:"error"`
	Message string `json:"message"`
}
