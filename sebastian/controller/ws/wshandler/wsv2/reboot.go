package wsv2

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations/reboot"
)

func StartReboot(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.OperationsCommandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StartReboot: %T", data)
		return
	}

	iReboot, err := dipool.GetInstance[reboot.IReboot]()
	if err != nil {
		logger.Errorf("Failed to get reboot instance: %v", err)
		return
	}

	logger.Info(fmt.Sprintf("StartReboot wsCmd:%+v\n", wsCmd))
	iReboot.StartReboot(ctx.ConnId, wsCmd.ProjectId, wsCmd.Id)
}

func StopReboot(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.BaseWsCommmandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StopReboot: %T", data)
		return
	}

	iReboot, err := dipool.GetInstance[reboot.IReboot]()
	if err != nil {
		logger.Errorf("Failed to get reboot instance: %v", err)
		return
	}

	logger.Info(fmt.Sprintf("StopReboot wsCmd:%+v\n", wsCmd))
	iReboot.Stop()
}
