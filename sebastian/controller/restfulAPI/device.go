package restfulAPI

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/api"
)

func CreateDevice(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var create_device_request domain.CreateDeviceRequest
	res = httputils.Bind(c, &create_device_request)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var mode []string
	res = httputils.GetQueryArray(c, "mode", &mode, false)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	is_operation := len(mode) > 0 && mode[0] == "operation"
	device_conf := create_device_request.DeviceConf
	from_bag := create_device_request.FromBag
	_, res = core.CreateDevice(project_id, device_conf, from_bag, is_operation)
	httputils.Respond(c, res)
}

func UpdateDevice(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var device_id int64
	res = httputils.ParseInt(c, "deviceId", &device_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var device_conf domain.DeviceConf
	res = httputils.Bind(c, &device_conf)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var mode []string
	res = httputils.GetQueryArray(c, "mode", &mode, false)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	is_operation := len(mode) > 0 && mode[0] == "operation"
	_, res = core.UpdateDevice(project_id, device_id, device_conf, is_operation)
	httputils.Respond(c, res)
}

func PatchDevice(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var device_id int64
	res = httputils.ParseInt(c, "deviceId", &device_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var mode []string
	res = httputils.GetQueryArray(c, "mode", &mode, false)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	is_operation := len(mode) > 0 && mode[0] == "operation"
	device, res := core.GetDevice(project_id, device_id, is_operation)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	deviceConf := device.Convert()
	res = httputils.Bind(c, deviceConf)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	_, res = core.UpdateDevice(project_id, device_id, deviceConf, is_operation)
	httputils.Respond(c, res)
}

func GetDevice(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var device_id int64
	res = httputils.ParseInt(c, "deviceId", &device_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var mode []string
	res = httputils.GetQueryArray(c, "mode", &mode, false)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	is_operation := len(mode) > 0 && mode[0] == "operation"
	_, res = core.GetDevice(project_id, device_id, is_operation)
	httputils.Respond(c, res)
}

func DeleteDevices(c *api.Context) {
	var project_id int64
	res := httputils.ParseInt(c, "projectId", &project_id)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var device_ids []int64
	res = httputils.GetQueryArray(c, "ids", &device_ids, true)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	var mode []string
	res = httputils.GetQueryArray(c, "mode", &mode, false)
	if !res.IsSuccess() {
		httputils.Respond(c, res)
		return
	}

	is_operation := len(mode) > 0 && mode[0] == "operation"
	res = core.DeleteDevices(project_id, device_ids, is_operation)
	httputils.Respond(c, res)
}
