package wsv2

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations/importconfig"
)

func StartImportConfig(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.OperationsCommandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StartImportConfig: %T", data)
		return
	}

	iImportConfig, err := dipool.GetInstance[importconfig.IImportConfig]()
	if err != nil {
		logger.Errorf("Failed to get importconfig instance: %v", err)
		return
	}

	logger.Info(fmt.Sprintf("StartImportConfig wsCmd:%+v\n", wsCmd))
	iImportConfig.StartImportConfig(ctx.ConnId, wsCmd.ProjectId, wsCmd.Id)
}

func StopImportConfig(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.BaseWsCommmandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StopImportConfig: %T", data)
		return
	}

	iImportConfig, err := dipool.GetInstance[importconfig.IImportConfig]()
	if err != nil {
		logger.Errorf("Failed to get importconfig instance: %v", err)
		return
	}

	logger.Info(fmt.Sprintf("StopImportConfig wsCmd:%+v\n", wsCmd))
	iImportConfig.Stop()
}
