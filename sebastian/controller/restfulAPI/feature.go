package restfulAPI

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/featuremanager"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/api"
)

func GetFeatureActivationStatus(c *api.Context) {
	status := featuremanager.GetInstance().GetActivationStatus()
	activationStatus := &struct {
		Status string `json:"Status"`
	}{
		Status: status.String(),
	}
	httputils.Respond(c, statuscode.StatusOK(activationStatus))
}
