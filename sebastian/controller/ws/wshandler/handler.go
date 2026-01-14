package wshandler

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscontext"
)

type HandleFunc func(ctx *wscontext.Context, data any)

func (fn HandleFunc) HandleMessage(ctx *wscontext.Context, data any) {
	fn(ctx, data)
}

type IHandler interface {
	HandleMessage(ctx *wscontext.Context, data any)
}
