package wscontext

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommon"
)

type Context struct {
	ConnId    string
	Msg       []byte
	ProjectId int64
	CmdType   wscommand.ActWSCommandType
	Role      string
}

func NewContext() *Context {
	// default constructor for Context
	return &Context{
		ConnId:    "",
		Msg:       []byte{},
		ProjectId: wscommon.ProjectIdNone,
		CmdType:   wscommand.ActWSCommandUnknown,
		Role:      "",
	}
}

func (c *Context) GetCmdType() wscommand.ActWSCommandType {
	return c.CmdType
}

func (c *Context) GetRawMessage() []byte {
	return c.Msg
}

func (c *Context) GetConnId() string {
	return c.ConnId
}

func (c *Context) GetProjectId() int64 {
	return c.ProjectId
}
