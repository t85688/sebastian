package topologymapping

import (
	"encoding/json"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
)

func TopologyMapperFailedResponse(connId string, statusCode int64, errMessage string) {

	wsResp := wscommand.NewWsCommandErrorResponse(wscommand.BaseWsCommandResponse{
		OpCode:     wscommand.ActWSCommandStartTopologyMapping.Int64(),
		StatusCode: statusCode,
	}, errMessage, "")

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}

func TopologyMapperProgressResponse(connId string, progress int64) {
	runningStatusCode := int64(1003)
	wsResp := wscommand.NewBaseWsCommandResponse(wscommand.ActWSCommandStartTopologyMapping.Int64(), runningStatusCode, "", wscommand.DeployResult{
		Progress: progress,
	})

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}

func TopologyMapperCompletedResponse(connId string, mappingResult domain.TopologyMappingResult) {
	completedStatusCode := int64(1005)
	wsResp := wscommand.NewBaseWsCommandResponse(wscommand.ActWSCommandStartTopologyMapping.Int64(), completedStatusCode, "", wscommand.TopologyMappingResultData{
		Progress:              100,
		TopologyMappingResult: mappingResult,
	})

	wsRespByte, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("failed to marshal ws response: %v", err)
	} else {
		ws.Unicast(wsRespByte, connId)
	}
}
