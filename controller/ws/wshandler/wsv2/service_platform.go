package wsv2

import (
	"encoding/json"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/serviceplatform"
)

func StartServicePlatform(ctx *wscontext.Context, data any) {
	pushFailedResp := func(errMsg string) {
		baseResp := wscommand.NewBaseWsCommandResponse(wscommand.ActWSCommandStartServicePlatform.Int64(), 1006, "", nil)
		errResp := wscommand.NewWsCommandErrorResponse(*baseResp, errMsg, "")

		errRespByte, err := json.Marshal(errResp)
		if err != nil {
			logger.Errorf("failed to marshal ws error response: %v", err)
			return
		}

		ws.Unicast(errRespByte, ctx.ConnId)
	}

	servicePlatform, err := dipool.GetInstance[serviceplatform.IServicePlatform]()
	if err != nil {
		logger.Errorf("Failed to get servicePlatform instance: %v", err)
		pushFailedResp("Internal server error")
		return
	}

	logger.Info("StartServicePlatform")
	servicePlatform.StartServicePlatform(ctx.ConnId)
}

func StopServicePlatform(ctx *wscontext.Context, data any) {
	pushFailedResp := func(errMsg string) {
		baseResp := wscommand.NewBaseWsCommandResponse(wscommand.ActWSCommandStopServicePlatform.Int64(), 1006, "", nil)
		errResp := wscommand.NewWsCommandErrorResponse(*baseResp, errMsg, "")

		errRespByte, err := json.Marshal(errResp)
		if err != nil {
			logger.Errorf("failed to marshal ws error response: %v", err)
			return
		}

		ws.Unicast(errRespByte, ctx.ConnId)
	}

	servicePlatform, err := dipool.GetInstance[serviceplatform.IServicePlatform]()
	if err != nil {
		logger.Errorf("Failed to get servicePlatform instance: %v", err)
		pushFailedResp("Internal server error")
		return
	}

	logger.Info("StopServicePlatform wsCmd")
	servicePlatform.StopServicePlatform(ctx.ConnId)
}
