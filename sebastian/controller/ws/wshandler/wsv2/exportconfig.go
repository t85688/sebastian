package wsv2

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations/exportconfig"
)

func StartExportConfig(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.OperationsCommandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StartExportConfig: %T", data)
		return
	}

	iExportConfig, err := dipool.GetInstance[exportconfig.IExportConfig]()
	if err != nil {
		logger.Errorf("Failed to get exportconfig instance: %v", err)
		return
	}

	logger.Info(fmt.Sprintf("StartExportConfig wsCmd:%+v\n", wsCmd))
	iExportConfig.StartExportConfig(ctx.ConnId, wsCmd.ProjectId, wsCmd.Id)
}

func StopExportConfig(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.BaseWsCommmandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StopExportConfig: %T", data)
		return
	}

	iExportConfig, err := dipool.GetInstance[exportconfig.IExportConfig]()
	if err != nil {
		logger.Errorf("Failed to get exportconfig instance: %v", err)
		return
	}

	logger.Info(fmt.Sprintf("StopExportConfig wsCmd:%+v\n", wsCmd))
	iExportConfig.Stop()
}
