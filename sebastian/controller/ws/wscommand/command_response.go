package wscommand

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
)

type BaseWsCommandResponse struct {
	OpCode     int64  `json:"OpCode"`
	StatusCode int64  `json:"StatusCode"`
	Action     string `json:"Action,omitempty"`
	Data       any    `json:"Data,omitempty"`
	Path       string `json:"Path,omitempty"`
}

func NewBaseWsCommandResponse(opCode, statusCode int64, action string, data any) *BaseWsCommandResponse {
	return &BaseWsCommandResponse{
		OpCode:     opCode,
		StatusCode: statusCode,
		Action:     action,
		Data:       data,
	}
}

type BaseData struct {
	Progress int64 `json:"Progress"`
}

type DeployResult struct {
	Progress     int64  `json:"Progress"`
	Id           int64  `json:"Id,omitempty"`
	Status       string `json:"Status,omitempty"`
	ErrorMessage string `json:"ErrorMessage,omitempty"`
}

type TopologyMappingResultData struct {
	Progress              int64                        `json:"Progress"`
	TopologyMappingResult domain.TopologyMappingResult `json:"TopologyMappingResult"`
}

type WsCommandErrorResponse struct {
	BaseWsCommandResponse
	ErrorMessage string `json:"ErrorMessage"`
	Parameter    string `json:"Parameter,omitempty"`
}

func NewWsCommandErrorResponse(base BaseWsCommandResponse, errorMessage string, parameter string) *WsCommandErrorResponse {
	return &WsCommandErrorResponse{
		BaseWsCommandResponse: base,
		ErrorMessage:          errorMessage,
		Parameter:             parameter,
	}
}
