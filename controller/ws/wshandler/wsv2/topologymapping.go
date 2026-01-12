package wsv2

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/topologymapping"
)

func StartTopologyMapping(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.TopologyMappingCommandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StartTopologyMapping: %T", data)
		return
	}

	topologyMapper, err := dipool.GetInstance[topologymapping.ITopologyMapper]()
	if err != nil {
		logger.Errorf("Failed to get topologyMapper instance: %v", err)
		return
	}

	// logger.Info("StartTopologyMapping wsCmd: %v", wsCmd)
	logger.Info(fmt.Sprintf("StartTopologyMapping wsCmd:%+v\n", wsCmd))
	topologyMapper.TopologyMapping(ctx.ConnId, wsCmd.ProjectId, wsCmd.DesignBaselineId)
}

func StopTopologyMapping(ctx *wscontext.Context, data any) {
	wsCmd, ok := data.(wscommand.BaseWsCommmandSchema)
	if !ok {
		logger.Errorf("Invalid data type for StopTopologyMapping: %T", data)
		return
	}

	topologyMapper, err := dipool.GetInstance[topologymapping.ITopologyMapper]()
	if err != nil {
		logger.Errorf("Failed to get topologyMapper instance: %v", err)
		return
	}

	// logger.Info("StartTopologyMapping wsCmd: %v", wsCmd)
	logger.Info(fmt.Sprintf("StopTopologyMapping wsCmd:%+v\n", wsCmd))
	topologyMapper.Stop()
}
