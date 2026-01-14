package wsv2

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations/factorydefault"
)

func StartFactoryDefault(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.OperationsCommandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StartFactoryDefault: %T", data)
		return
	}

	iFactoryDefault, err := dipool.GetInstance[factorydefault.IFactoryDefault]()
	if err != nil {
		logger.Errorf("Failed to get factorydefault instance: %v", err)
		return
	}

	logger.Info(fmt.Sprintf("StartFactoryDefault wsCmd:%+v\n", wsCmd))
	iFactoryDefault.StartFactoryDefault(ctx.ConnId, wsCmd.ProjectId, wsCmd.Id)
}

func StopFactoryDefault(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.BaseWsCommmandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StopFactoryDefault: %T", data)
		return
	}

	iFactoryDefault, err := dipool.GetInstance[factorydefault.IFactoryDefault]()
	if err != nil {
		logger.Errorf("Failed to get factorydefault instance: %v", err)
		return
	}

	logger.Info(fmt.Sprintf("StopFactoryDefault wsCmd:%+v\n", wsCmd))
	iFactoryDefault.Stop()
}
