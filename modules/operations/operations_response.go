package operations

import (
	"encoding/json"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
)

const (
	runningStatusCode   = 1003
	completedStatusCode = 1005
)

func OperationFailedResponse(connId string, opcode int64, statusCode int64, errMessage string) {

	wsResp := wscommand.NewWsCommandErrorResponse(wscommand.BaseWsCommandResponse{
		OpCode:     opcode,
		StatusCode: statusCode,
	}, errMessage, "")

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}

func OperationProgressResponse(connId string, opcode int64, progress int64) {
	wsResp := wscommand.NewBaseWsCommandResponse(opcode, runningStatusCode, "", wscommand.BaseData{
		Progress: progress,
	})

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}

func OperationDeviceResultProgressResponse(connId string, opcode int64, result wscommand.OperationsDeviceResult) {
	wsResp := wscommand.NewBaseWsCommandResponse(opcode, runningStatusCode, "", result)

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}

func OperationCompletedResponse(connId string, opcode int64) {
	wsResp := wscommand.NewBaseWsCommandResponse(opcode, completedStatusCode, "", wscommand.BaseData{
		Progress: 100,
	})

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}
