package wsdispatcher

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wshandler"
)

func RegisterHandler(messageType string, handler wshandler.IHandler) {
	defaultDispatcher.RegisterHandler(messageType, handler)
}

func RegisterHandleFunc(messageType string, handlerFunc wshandler.HandleFunc) {
	defaultDispatcher.RegisterHandleFunc(messageType, handlerFunc)
}

func Dispatch(ctx *wscontext.Context, baseCmdData wscommand.BaseWsCommmandSchema) error {
	return defaultDispatcher.Dispatch(ctx, baseCmdData)
}
