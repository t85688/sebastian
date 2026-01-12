package serviceplatform

import (
	"encoding/json"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
)

func ServicePlatformStatusResponse(connId string, login bool) {
	runningStatusCode := int64(1003)
	wsResp := &wscommand.BaseWsCommandResponse{
		OpCode:     wscommand.ActWSCommandStartServicePlatform.Int64(),
		StatusCode: runningStatusCode,
		Data: domain.ServicePlatformLoginResp{
			IsLoggedIn: login,
		}}

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}

func ServicePlatformAuthResponse(connId string, authResult domain.ServicePlatformAuthResp) {
	completedStatusCode := int64(1005)
	wsResp := &wscommand.BaseWsCommandResponse{
		OpCode:     wscommand.ActWSCommandStartServicePlatform.Int64(),
		StatusCode: completedStatusCode,
		Data:       authResult}

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}

func ServicePlatformFailedResponse(connId string, errMessage string) {
	failedStatusCode := int64(1006)
	wsResp := &wscommand.WsCommandErrorResponse{
		BaseWsCommandResponse: wscommand.BaseWsCommandResponse{
			OpCode:     wscommand.ActWSCommandStartServicePlatform.Int64(),
			StatusCode: failedStatusCode,
		},
		ErrorMessage: errMessage,
	}

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}
