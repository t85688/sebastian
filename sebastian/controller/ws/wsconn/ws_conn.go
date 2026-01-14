package wsconn

import (
	"context"
	"encoding/json"
	"fmt"
	"sync"

	"github.com/google/uuid"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wsdispatcher"
	afhttp "gitlab.com/moxa/sw/maf/moxa-app-framework/pkg/protocol/http"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/logging"
)

var logger = logging.NewWithField("source", "ws")

const defaultWriteChannelSize = 16 // Default size for the write channel buffer
type WsConn struct {
	ID        string
	ProjectId int64
	conn      afhttp.WebSocketConnection
	writeChan chan []byte
	Wg        sync.WaitGroup
	ctx       context.Context
	cancel    context.CancelFunc
	writeLock sync.Mutex // Mutex to protect write operations
	isClosed  bool
	closeOnce sync.Once
}

func NewWsConn(conn afhttp.WebSocketConnection, projectId int64) *WsConn {
	ctx, cancel := context.WithCancel(context.Background())

	instance := &WsConn{
		ID:        uuid.NewString(),
		ProjectId: projectId,
		conn:      conn,
		writeChan: make(chan []byte, defaultWriteChannelSize), // Buffered channel to handle writes
		ctx:       ctx,
		cancel:    cancel,
		isClosed:  false,
	}

	return instance
}

func (conn *WsConn) Write(msg []byte) error {
	conn.writeLock.Lock()
	defer conn.writeLock.Unlock()

	if conn.isClosed {
		return fmt.Errorf("connection %s is closed", conn.ID)
	}

	conn.writeChan <- msg
	return nil
}

func (conn *WsConn) WriteWorker() error {
	defer conn.Wg.Done()
	defer conn.cancel()

	for {
		select {
		case <-conn.ctx.Done():
			logger.Debugf("[WS_ID: %s] Write Worker stopped due to context done", conn.ID)
			return nil
		case msg, ok := <-conn.writeChan:
			if !ok {
				logger.Debugf("[WS_ID: %s] Write channel had been closed, stopping write worker", conn.ID)
				return nil
			}

			if len(msg) == 0 {
				logger.Debugf("[WS_ID: %s] Received empty message, skipping write", conn.ID)
				continue
			}

			logger.Debugf("[WS_ID: %s] Writing message: %s", conn.ID, msg)
			if err := conn.conn.WriteMessage(msg); err != nil {
				logger.Debugf("[WS_ID: %s] Error writing message: %v", conn.ID, err)
				return err
			} else {
				logger.Debugf("[WS_ID: %s] Message written successfully", conn.ID)
			}
		}
	}
}

func (conn *WsConn) ReadWorker() error {
	defer conn.Wg.Done()
	defer conn.cancel()

	for {
		select {
		case <-conn.ctx.Done():
			logger.Infof("[WS_ID: %s] Receive Worker stopped due to context done\n", conn.ID)
			return nil
		default:
			msg, err := conn.conn.ReadMessage()
			if err != nil {
				return err
			}

			go conn.onMessage(msg) // refactor as worker pattern
		}
	}
}

func (conn *WsConn) onMessage(msg []byte) {
	logger.Infof("[WS_ID: %s] Received message: %s\n", conn.ID, msg)

	wsCmd := wscommand.BaseWsCommmandSchema{}
	err := json.Unmarshal(msg, &wsCmd)
	if err != nil {
		logger.Warnf("[WS_ID: %s] Error unmarshalling message: %v\n", conn.ID, err)
		return
	}

	commandType := wscommand.ParseActWSCommand(wsCmd.OpCode)

	if commandType == wscommand.ActWSCommandUnknown {
		logger.Infof("Received unknown command:", wsCmd.OpCode) // to hex
		return                                                  // or handle unknown command
	}

	wsCtx := wscontext.NewContext()
	wsCtx.ConnId = conn.ID
	wsCtx.Msg = msg
	wsCtx.ProjectId = conn.ProjectId
	wsCtx.CmdType = commandType

	dispatchErr := wsdispatcher.Dispatch(wsCtx, wsCmd)
	if dispatchErr != nil {
		logger.Warnf("[WS_ID: %s] Error dispatching message: %v\n", conn.ID, dispatchErr)
	}
}

func (conn *WsConn) Close() error {
	conn.closeOnce.Do(func() {
		conn.writeLock.Lock()
		defer conn.writeLock.Unlock()

		conn.isClosed = true
		defaultConnHub.removeConnection(conn.ID)
		close(conn.writeChan)
	})

	err := conn.conn.Close()

	return err
}
