package wsdispatcher

import (
	"encoding/json"
	"fmt"
	"sync"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wshandler"
)

type Dispatcher struct {
	handlers map[string]wshandler.IHandler
	mutex    sync.RWMutex // May not be needed if only one goroutine registers handlers at startup.
}

func NewDispatcher() *Dispatcher {
	return &Dispatcher{
		handlers: make(map[string]wshandler.IHandler),
	}
}

func (dispatcher *Dispatcher) RegisterHandler(messageType string, handler wshandler.IHandler) {
	if messageType == "" {
		panic("messageType cannot be empty")
	}

	if handler == nil {
		panic("handler cannot be nil")
	}

	dispatcher.mutex.Lock()
	defer dispatcher.mutex.Unlock()

	// Register the handler in the dispatcher
	dispatcher.handlers[messageType] = handler
}

func (dispatcher *Dispatcher) RegisterHandleFunc(messageType string, handlerFunc wshandler.HandleFunc) {
	if messageType == "" {
		panic("messageType cannot be empty")
	}

	if handlerFunc == nil {
		panic("handlerFunc cannot be nil")
	}

	dispatcher.mutex.Lock()
	defer dispatcher.mutex.Unlock()

	// Register the handler in the dispatcher
	dispatcher.handlers[messageType] = handlerFunc
}

func (dispatcher *Dispatcher) Dispatch(ctx *wscontext.Context, baseCmdData wscommand.BaseWsCommmandSchema) error {
	dispatcher.mutex.RLock()
	defer dispatcher.mutex.RUnlock()

	var commandData any
	commandType := ctx.GetCmdType()
	handler, exists := dispatcher.handlers[commandType.String()]
	if !exists {
		return fmt.Errorf("no handler registered for message type: %s", commandType.String())
	}

	commandData, hasCommandData, err := dispatcher.ParseMessage(ctx)
	if err != nil {
		return err
	}

	if !hasCommandData {
		commandData = baseCmdData
	}

	handler.HandleMessage(ctx, commandData)
	return nil
}

func (dispatcher *Dispatcher) ParseMessage(ctx *wscontext.Context) (any, bool, error) {
	// This should never happen.
	if len(ctx.Msg) == 0 {
		logger.Warnf("[WS_ID: %s] Received empty message, cannot parse", ctx.GetConnId())
		return nil, false, fmt.Errorf("message cannot be empty")
	}

	var commandData any

	switch ctx.GetCmdType() {
	case wscommand.ActWSCommandStartMonitor:
		// Parse start monitor command
		startCmd := wscommand.StartMonitorCommandSchema{}
		err := json.Unmarshal(ctx.Msg, &startCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling StartMonitorCommandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}

		commandData = startCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStopMonitor:
		stopCmd := wscommand.StopMonitorCommandSchema{}
		err := json.Unmarshal(ctx.Msg, &stopCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling StopMonitorCommandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}

		commandData = stopCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStartScanTopology:
		scanCmd := wscommand.ScanTopologyCommandSchema{}
		err := json.Unmarshal(ctx.Msg, &scanCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling ScanTopologyCommandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}

		commandData = scanCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStopScanTopology:
		// Parse stop scan topology command
	case wscommand.ActWSCommandStartDeviceDiscovery:
		// Parse start device discovery command
	case wscommand.ActWSCommandStopDeviceDiscovery:
		// Parse stop device discovery command

	case wscommand.ActWSCommandStartDeploy:
		deployCmd := wscommand.DeployCommandSchema{}
		err := json.Unmarshal(ctx.Msg, &deployCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling DeployCommandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}

		commandData = deployCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStopDeploy:
		baseCmd := wscommand.BaseWsCommmandSchema{}
		err := json.Unmarshal(ctx.Msg, &baseCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling BaseWsCommmandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}

		commandData = baseCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStartTopologyMapping:
		deployCmd := wscommand.TopologyMappingCommandSchema{}
		err := json.Unmarshal(ctx.Msg, &deployCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling TopologyMappingCommandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}

		commandData = deployCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStopTopologyMapping:
		baseCmd := wscommand.BaseWsCommmandSchema{}
		err := json.Unmarshal(ctx.Msg, &baseCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling BaseWsCommmandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}

		commandData = baseCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStartReboot:
		operationsCmd := wscommand.OperationsCommandSchema{}
		err := json.Unmarshal(ctx.Msg, &operationsCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling OperationsCommandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}
		commandData = operationsCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStopReboot:
		baseCmd := wscommand.BaseWsCommmandSchema{}
		err := json.Unmarshal(ctx.Msg, &baseCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling BaseWsCommmandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}
		commandData = baseCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStartFactoryDefault:
		operationsCmd := wscommand.OperationsCommandSchema{}
		err := json.Unmarshal(ctx.Msg, &operationsCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling OperationsCommandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}
		commandData = operationsCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStopFactoryDefault:
		baseCmd := wscommand.BaseWsCommmandSchema{}
		err := json.Unmarshal(ctx.Msg, &baseCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling BaseWsCommmandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}
		commandData = baseCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStartLocator:
		locatorCmd := wscommand.LocatorCommandSchema{}
		err := json.Unmarshal(ctx.Msg, &locatorCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling LocatorCommandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}
		commandData = locatorCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStopLocator:
		baseCmd := wscommand.BaseWsCommmandSchema{}
		err := json.Unmarshal(ctx.Msg, &baseCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling BaseWsCommmandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}
		commandData = baseCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStartExportDeviceConfig:
		operationsCmd := wscommand.OperationsCommandSchema{}
		err := json.Unmarshal(ctx.Msg, &operationsCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling OperationsCommandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}
		commandData = operationsCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStopExportDeviceConfig:
		baseCmd := wscommand.BaseWsCommmandSchema{}
		err := json.Unmarshal(ctx.Msg, &baseCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling BaseWsCommmandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}
		commandData = baseCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStartImportDeviceConfig:
		operationsCmd := wscommand.OperationsCommandSchema{}
		err := json.Unmarshal(ctx.Msg, &operationsCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling OperationsCommandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}
		commandData = operationsCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStopImportDeviceConfig:
		baseCmd := wscommand.BaseWsCommmandSchema{}
		err := json.Unmarshal(ctx.Msg, &baseCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling BaseWsCommmandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}
		commandData = baseCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStartFirmwareUpgrade:
		firmwareupgradeCmd := wscommand.FirmwareUpgradeCommandSchema{}
		err := json.Unmarshal(ctx.Msg, &firmwareupgradeCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling FirmwareUpgradeCommandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}
		commandData = firmwareupgradeCmd
		return commandData, true, nil
	case wscommand.ActWSCommandStopFirmwareUpgrade:
		baseCmd := wscommand.BaseWsCommmandSchema{}
		err := json.Unmarshal(ctx.Msg, &baseCmd)
		if err != nil {
			logger.Warnf("[WS_ID: %s] Error unmarshalling BaseWsCommmandSchema: %w", ctx.GetConnId(), err)
			return nil, false, err
		}
		commandData = baseCmd
		return commandData, true, nil

	default:
		// use base command schema
		return nil, false, nil
	}

	return nil, false, nil
}
