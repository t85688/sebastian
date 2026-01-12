package wsv2

import (
	"encoding/json"
	"errors"
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/featuremanager"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/monitor"
)

func StartMonitor(ctx *wscontext.Context, data any) {
	//  kFailed = 1006,
	//  kSuccess = 200,

	pushWsResp := func(err error) {
		wsResp := wscommand.WsCommandErrorResponse{
			BaseWsCommandResponse: *wscommand.NewBaseWsCommandResponse(
				wscommand.ActWSCommandStartMonitor.Int64(),
				200,
				"",
				nil),
		}

		if err != nil {
			wsResp.ErrorMessage = err.Error()
			wsResp.StatusCode = 1006
		}

		jsonBytes, err := json.Marshal(wsResp)

		if err != nil {
			logger.Errorf("Failed to marshal StartMonitor response: %v", err)
			return
		}

		ws.Unicast(jsonBytes, ctx.ConnId)
	}

	if !featuremanager.GetInstance().IsProjectOperationStageSupported() {
		err := fmt.Errorf("The monitor feature is not supported by current license")
		logger.Error(err.Error())
		pushWsResp(fmt.Errorf("failed to start monitor: the license is not supported"))
		return
	}

	wsCmd, ok := data.(wscommand.StartMonitorCommandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StartMonitor: %T", data)
		pushWsResp(fmt.Errorf("failed to start monitor: internal error"))
		return
	}

	monitorInstance, err := dipool.GetInstance[monitor.IMonitor]()
	if err != nil {
		logger.Errorf("Failed to get IMonitor instance: %v", err)
		pushWsResp(fmt.Errorf("failed to start monitor: internal error"))
		return
	}

	err = monitorInstance.Start(wsCmd.ProjectId)
	if err != nil {
		pushWsResp(fmt.Errorf("failed to start monitor: %v", err))
		return
	}

	pushWsResp(nil)
}

func StopMonitor(ctx *wscontext.Context, data any) {
	pushWsResp := func(err error) {
		wsResp := wscommand.WsCommandErrorResponse{
			BaseWsCommandResponse: *wscommand.NewBaseWsCommandResponse(
				wscommand.ActWSCommandStopMonitor.Int64(),
				200,
				"",
				nil),
		}

		if err != nil {
			wsResp.ErrorMessage = err.Error()
			wsResp.StatusCode = 1006
		}

		jsonBytes, err := json.Marshal(wsResp)

		if err != nil {
			logger.Errorf("Failed to marshal StartMonitor response: %v", err)
			return
		}

		ws.Unicast(jsonBytes, ctx.ConnId)
	}

	wsCmd, ok := data.(wscommand.StopMonitorCommandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StopMonitor: %T", data)
		pushWsResp(fmt.Errorf("failed to stop monitor: internal error"))
		return
	}

	projectId := wsCmd.ProjectId
	if projectId <= 0 {
		// invalid project id
		logger.Errorf("Invalid project ID for StopMonitor: %d", projectId)
		pushWsResp(fmt.Errorf("failed to stop monitor: invalid project ID"))
		return
	}

	monitorInstance, err := dipool.GetInstance[monitor.IMonitor]()
	if err != nil {
		logger.Errorf("Failed to get IMonitor instance: %v", err)
		pushWsResp(fmt.Errorf("failed to stop monitor: internal error"))
		return
	}

	err = monitorInstance.Stop(projectId)
	if err != nil {
		logger.Errorf("Failed to stop monitor: %v", err)
		if errors.Is(err, monitor.ErrStopMonitorNotRunning) {
			// todo: check project status, if it is in monitoring, change it to idle
			pushWsResp(nil)
			return
		}

		pushWsResp(fmt.Errorf("failed to stop monitor: %v", err))
		return
	}

	pushWsResp(nil)
}
