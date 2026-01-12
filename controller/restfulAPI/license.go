package restfulAPI

import (
	"os"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/api"
)

func GetLicenseInformation(c *api.Context) {
	res := core.GetLicenseInformation()
	httputils.Respond(c, res)
}

func GetLicenseUserCode(c *api.Context) {
	machineId := internal.GetMachineId()
	userCode := &struct {
		UserCode string `json:"UserCode"`
	}{
		UserCode: machineId,
	}
	httputils.Respond(c, statuscode.StatusOK(userCode))
}

func ImportLicenseActivationCode(c *api.Context) {
	var activationCode domain.ActivationCode
	res := httputils.ReadAll(c, &activationCode)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	res = core.ImportLicenseActivationCode(activationCode)
	httputils.Respond(c, res)
}

func UploadLicenseActivationCode(c *api.Context) {
	source, err := c.FormFile("activationCodeFile")
	if err != nil {
		httputils.Respond(c, statuscode.StatusBadRequest(err.Error(), 0))
		return
	}

	targetFile, err := os.CreateTemp("", "f-*")
	if err != nil {
		httputils.Respond(c, statuscode.StatusInternalError(err.Error()))
		return
	}
	targetFile.Close()
	defer os.Remove(targetFile.Name())

	if err = c.SaveUploadedFile(source, targetFile.Name()); err != nil {
		httputils.Respond(c, statuscode.StatusInternalError(err.Error()))
		return
	}

	rawActivationCode, err := os.ReadFile(targetFile.Name())
	if err != nil {
		httputils.Respond(c, statuscode.StatusInternalError(err.Error()))
		return
	}

	activationCode := domain.ActivationCode{
		RawString: string(rawActivationCode),
	}
	res := core.ImportLicenseActivationCode(activationCode)
	httputils.Respond(c, res)
}
