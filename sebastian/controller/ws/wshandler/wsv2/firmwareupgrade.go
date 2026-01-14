package wsv2

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations/firmwareupgrade"
)

func StartFirmwareUpgrade(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.FirmwareUpgradeCommandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StartFirmwareUpgrade: %T", data)
		return
	}

	iFirmwareUpgrade, err := dipool.GetInstance[firmwareupgrade.IFirmwareUpgrade]()
	if err != nil {
		logger.Errorf("Failed to get firmwareupgrade instance: %v", err)
		return
	}

	logger.Info(fmt.Sprintf("StartFirmwareUpgrade wsCmd:%+v\n", wsCmd))
	iFirmwareUpgrade.StartFirmwareUpgrade(ctx.ConnId, wsCmd.ProjectId, wsCmd.Id, wsCmd.FirmwareName)
}

func StopFirmwareUpgrade(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.BaseWsCommmandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StopFirmwareUpgrade: %T", data)
		return
	}

	iFirmwareUpgrade, err := dipool.GetInstance[firmwareupgrade.IFirmwareUpgrade]()
	if err != nil {
		logger.Errorf("Failed to get firmwareupgrade instance: %v", err)
		return
	}

	logger.Info(fmt.Sprintf("StopFirmwareUpgrade wsCmd:%+v\n", wsCmd))
	iFirmwareUpgrade.Stop()
}
