package wsv2

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/deploy"
)

func StartDeploy(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.DeployCommandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StartDeploy: %T", data)
		return
	}

	deployer, err := dipool.GetInstance[deploy.IDeployer]()
	if err != nil {
		logger.Errorf("Failed to get deployer instance: %v", err)
		return
	}

	// logger.Info("StartDeploy wsCmd: %v", wsCmd)
	logger.Info(fmt.Sprintf("StartDeploy wsCmd:%+v\n", wsCmd))
	deployer.Deploy(ctx.ConnId, wsCmd.ProjectId, wsCmd.DesignBaselineId, wsCmd.Id, wsCmd.SkipMappingDevice)
}

func StopDeploy(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.BaseWsCommmandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StopDeploy: %T", data)
		return
	}

	deployer, err := dipool.GetInstance[deploy.IDeployer]()
	if err != nil {
		logger.Errorf("Failed to get deployer instance: %v", err)
		return
	}

	// logger.Info("StartDeploy wsCmd: %v", wsCmd)
	logger.Info(fmt.Sprintf("StopDeploy wsCmd:%+v\n", wsCmd))
	deployer.Stop()
}
