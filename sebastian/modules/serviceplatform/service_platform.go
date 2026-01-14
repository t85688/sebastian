package serviceplatform

import (
	"context"
	"os"
	"os/signal"
	"sync"
	"time"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
)

type IServicePlatform interface {
	StartServicePlatform(connId string, deviceCode string)
	StopServicePlatform(connId string)
}

type ServicePlatform struct {
	mutex            sync.Mutex
	parentCtx        context.Context
	ctx              map[string]context.Context    // map[connId]Context
	cancel           map[string]context.CancelFunc // map[connId]CancelFunc
	connMap          map[string]string             // map[connId]deviceCode
	onStatusResponse func(connId string, loginStatus bool)
	onFailedResponse func(connId string, errMessage string)
}

func NewServicePlatform(
	onStatusResponse func(connId string, loginStatus bool),
	onFailedResponse func(connId string, errMessage string),
) IServicePlatform {
	parentCtx, _ := signal.NotifyContext(context.Background(), os.Interrupt)
	return &ServicePlatform{
		parentCtx:        parentCtx,
		ctx:              make(map[string]context.Context),
		cancel:           make(map[string]context.CancelFunc),
		connMap:          make(map[string]string),
		onStatusResponse: onStatusResponse,
		onFailedResponse: onFailedResponse,
	}
}

func (s *ServicePlatform) StartServicePlatform(connId string, deviceCode string) {
	s.cancelTask(connId)

	ctx := s.addTask(connId)

	s.addDeviceCode(connId, deviceCode)

	go s.PollingServicePlatformToken(connId, ctx, deviceCode)
}

func (s *ServicePlatform) StopServicePlatform(connId string) {
	defer s.cancelTask(connId)

	deviceCode, exists := s.getDeviceCode(connId)
	if !exists {
		logger.Infoln("Service platform doens't run")
		return
	}

	token, exists := core.GetServicePlatformToken(deviceCode)
	if !exists {
		logger.Infoln("Unauthorized service platform")
		return
	}

	if ctx, ok := s.ctx[connId]; ok {
		res := core.LogoutServicePlatform(ctx, token.RefreshToken)
		if !res.IsSuccess() {
			logger.Infoln("Logout service platform failed: %s", res.ErrorMessage)
			return
		}
	}
}

func (s *ServicePlatform) PollingServicePlatformToken(connId string, ctx context.Context, deviceCode string) {
	ticker := time.NewTicker(10 * time.Second)
	defer ticker.Stop()

	timeout := 10 * time.Minute
	startTime := time.Now()

	for {
		select {
		case <-ctx.Done():
			var errMessage string
			if s.parentCtx.Err() != nil {
				errMessage = "Stop polling: System Shutdown"
			} else if ctx.Err() == context.Canceled {
				errMessage = "Stop polling: Logout"
				s.onStatusResponse(connId, false)
			}
			logger.Infoln(errMessage, time.Now().Format("15:04:05"))
			return

		case t := <-ticker.C:
			if time.Since(startTime) > timeout {
				logger.Infoln("Stop polling: Timeout", time.Now().Format("15:04:05"))
				s.onFailedResponse(connId, "Stop polling: Timeout")
				s.cancelTask(connId)
				return
			}
			logger.Infoln("Tick service platform...", t.Format("15:04:05"))
			token, exists := core.GetServicePlatformToken(deviceCode)
			if exists {
				if time.Now().After(token.ExpireAt) {
					res := core.RefreshServicePlatformToken(ctx, token)
					if !res.IsSuccess() {
						s.onStatusResponse(connId, false)
					} else {
						ticker.Reset(1 * time.Minute)
						s.onStatusResponse(connId, true)
						startTime = time.Now()
					}
				}
			} else {
				res := core.PostServicePlatformToken(ctx, deviceCode)
				if !res.IsSuccess() {
					s.onStatusResponse(connId, false)
				} else {
					ticker.Reset(1 * time.Minute)
					s.onStatusResponse(connId, true)
					startTime = time.Now()
				}
			}
		}
	}
}

func (s *ServicePlatform) addTask(connId string) (ctx context.Context) {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	ctx, cancel := context.WithCancel(s.parentCtx)
	s.ctx[connId] = ctx
	s.cancel[connId] = cancel

	return ctx
}

func (s *ServicePlatform) addDeviceCode(connId string, deviceCode string) {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	s.connMap[connId] = deviceCode
}

func (s *ServicePlatform) cancelTask(connId string) {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	if cancel, ok := s.cancel[connId]; ok {
		cancel()
		delete(s.ctx, connId)
		delete(s.cancel, connId)
	}

	if deviceCode, ok := s.connMap[connId]; ok {
		core.DeleteServicePlatformToken(deviceCode)
		delete(s.connMap, connId)
	}

}

func (s *ServicePlatform) getDeviceCode(connId string) (deviceCode string, exists bool) {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	deviceCode, exists = s.connMap[connId]
	return deviceCode, exists
}
