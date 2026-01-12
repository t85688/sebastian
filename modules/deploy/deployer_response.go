package deploy

import (
	"encoding/json"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
)

func DeployerFailedResponse(connId string, statusCode int64, errMessage string) {

	wsResp := wscommand.NewWsCommandErrorResponse(wscommand.BaseWsCommandResponse{
		OpCode:     wscommand.ActWSCommandStartDeploy.Int64(),
		StatusCode: statusCode,
	}, errMessage, "")

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}

func DeployerProgressResponse(connId string, progress int64) {
	runningStatusCode := int64(1003)
	wsResp := wscommand.NewBaseWsCommandResponse(wscommand.ActWSCommandStartDeploy.Int64(), runningStatusCode, "", wscommand.DeployResult{
		Progress: progress,
	})

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}

func DeployerDeviceResultProgressResponse(connId string, progress int64, deployDeviceResult DeployDeviceResult) {
	runningStatusCode := int64(1003)
	wsResp := wscommand.NewBaseWsCommandResponse(wscommand.ActWSCommandStartDeploy.Int64(), runningStatusCode, "", wscommand.DeployResult{
		Progress:     progress,
		Id:           deployDeviceResult.DeviceId,
		Status:       deployDeviceResult.Status,
		ErrorMessage: deployDeviceResult.ErrorMessage,
	})

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}

func DeployerCompletedResponse(connId string) {
	completedStatusCode := int64(1005)
	wsResp := wscommand.NewBaseWsCommandResponse(wscommand.ActWSCommandStartDeploy.Int64(), completedStatusCode, "", wscommand.DeployResult{
		Progress: 100,
	})

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}
