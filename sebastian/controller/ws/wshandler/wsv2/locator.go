package wsv2

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations/locator"
)

func StartLocator(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.LocatorCommandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StartLocator: %T", data)
		return
	}

	iLocator, err := dipool.GetInstance[locator.ILocator]()
	if err != nil {
		logger.Errorf("Failed to get locator instance: %v", err)
		return
	}

	if wsCmd.Duration == 0 { // default as 30 sec
		wsCmd.Duration = 30
	}

	logger.Info(fmt.Sprintf("StartLocator wsCmd:%+v\n", wsCmd))
	iLocator.StartLocator(ctx.ConnId, wsCmd.ProjectId, wsCmd.Id, wsCmd.Duration)
}

func StopLocator(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.BaseWsCommmandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StopLocator: %T", data)
		return
	}

	iLocator, err := dipool.GetInstance[locator.ILocator]()
	if err != nil {
		logger.Errorf("Failed to get locator instance: %v", err)
		return
	}

	logger.Info(fmt.Sprintf("StopLocator wsCmd:%+v\n", wsCmd))
	iLocator.Stop()
}
