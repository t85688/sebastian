package wsv2

import (
	"context"
	"encoding/json"
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/scan"
)

func StartScanTopology(ctx *wscontext.Context, data any) {
	pushFailedResp := func(errMsg string) {
		baseResp := wscommand.NewBaseWsCommandResponse(wscommand.ActWSCommandStartScanTopology.Int64(), 1006, "", nil)
		errResp := wscommand.NewWsCommandErrorResponse(*baseResp, errMsg, "")

		errRespByte, err := json.Marshal(errResp)
		if err != nil {
			logger.Errorf("failed to marshal ws error response: %v", err)
			return
		}

		ws.Unicast(errRespByte, ctx.ConnId)
	}

	pushSuccessfulResp := func(scanResult *wscommand.ScanTopologyResult) {
		baseResp := wscommand.NewBaseWsCommandResponse(wscommand.ActWSCommandStartScanTopology.Int64(), 1005, "", scanResult)

		wsRespByte, err := json.Marshal(baseResp)
		if err != nil {
			logger.Errorf("failed to marshal ws response: %v", err)
			return
		}

		ws.Unicast(wsRespByte, ctx.ConnId)
	}

	wsCmd, ok := data.(wscommand.ScanTopologyCommandSchema)
	if !ok {
		pushFailedResp("Internal server error")
		logger.Errorf("Invalid data type for StartScanTopology: %T", data)
		return
	}

	scanner, err := dipool.GetInstance[scan.IScanner]()
	if err != nil {
		logger.Errorf("Failed to get scanner instance: %v", err)
		pushFailedResp("Internal server error")
		return
	}

	if wsCmd.ProjectId <= 0 {
		logger.Errorf("Invalid ProjectId for StartScanTopology: %d", wsCmd.ProjectId)
		pushFailedResp(fmt.Sprintf("Invalid ProjectId: %d", wsCmd.ProjectId))
		return
	}

	status := core.UpdateProjectStatus(wsCmd.ProjectId, domain.ProjectStatusEnum_Scanning)
	if !status.IsSuccess() {
		logger.Errorf("Failed to start scan, project status cannot be updated: %v", status.ErrorMessage)
		pushFailedResp(fmt.Sprintf("failed to start scan, project status cannot be updated: %v", status.ErrorMessage))
		return
	}

	connId := ctx.ConnId
	reqCtx := context.WithValue(context.Background(), wscontext.ContextKeyConnId, connId)

	scanResultItems, _, err := scanner.StartScanTopology(reqCtx, wsCmd.ProjectId, wsCmd.NewTopology)
	status = core.UpdateProjectStatus(wsCmd.ProjectId, domain.ProjectStatusEnum_Idle)
	if !status.IsSuccess() {
		logger.Errorf("project status cannot be updated: %v", status.ErrorMessage)
	}

	if err != nil {
		logger.Errorf("Failed to scan topology: %v", err)
		pushFailedResp(fmt.Sprintf("Failed to scan topology: %v", err))
		return
	}

	resultItems := make([]*wscommand.ScanTopologyResultItem, 0, len(scanResultItems))
	for _, item := range scanResultItems {
		resultItems = append(resultItems, &wscommand.ScanTopologyResultItem{
			FirmwareVersion: item.FirmwareVersion,
			IP:              item.IP,
			MacAddress:      item.MacAddress,
			ModelName:       item.ModelName,
		})
	}

	pushSuccessfulResp(&wscommand.ScanTopologyResult{
		Progress:   100,
		ScanResult: &resultItems,
	})
}

func StopScanTopology(ctx *wscontext.Context, data any) {
	pushFailedResp := func(errMsg string) {
		baseResp := wscommand.NewBaseWsCommandResponse(wscommand.ActWSCommandStopScanTopology.Int64(), 1006, "", nil)
		errResp := wscommand.NewWsCommandErrorResponse(*baseResp, errMsg, "")

		errRespByte, err := json.Marshal(errResp)
		if err != nil {
			logger.Errorf("failed to marshal ws error response: %v", err)
			return
		}

		ws.Unicast(errRespByte, ctx.ConnId)
	}

	pushSuccessfulResp := func() {
		baseResp := wscommand.NewBaseWsCommandResponse(wscommand.ActWSCommandStopScanTopology.Int64(), 200, "", nil)

		wsRespByte, err := json.Marshal(baseResp)
		if err != nil {
			logger.Errorf("failed to marshal ws response: %v", err)
			return
		}

		ws.Unicast(wsRespByte, ctx.ConnId)
	}

	_, ok := data.(wscommand.BaseWsCommmandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StopScanTopology: %T", data)
		pushFailedResp("Internal server error")
		return
	}

	scanner, err := dipool.GetInstance[scan.IScanner]()
	if err != nil {
		logger.Errorf("Failed to get scanner instance: %v", err)
		pushFailedResp("Internal server error")
		return
	}

	status := scanner.StopScanTopology()
	if status == scan.StopScanResultStatusNotScanning {
		logger.Infof("failed to stop scan topology: No scan topology task is running")
		pushSuccessfulResp()
		return
	}

	if status == scan.StopScanResultStatusCompleted {
		logger.Infof("scan topology task has been triggered")
		pushSuccessfulResp()
		return
	}

	pushFailedResp("Failed to stop scan topology")
}
