package restfulAPI

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/api"
)

func Login(c *api.Context) {
	var login domain.Login
	res := httputils.Bind(c, &login)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	_, res = core.Login(login)
	httputils.Respond(c, res)
}

func CheckTokenExist(c *api.Context) {
	_, res := core.CheckTokenExist()
	httputils.Respond(c, res)
}

func RenewToken(c *api.Context) {
	_, res := core.RenewToken()

	httputils.Respond(c, res)
}

func Logout(c *api.Context) {
	res := core.Logout()
	httputils.Respond(c, res)
}
