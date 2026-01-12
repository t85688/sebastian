package restfulAPI

import (
	"fmt"
	"net/http"
	"strconv"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/restfulAPI/dto"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/monitor"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/api"
)

func StartMonitor(c *api.Context) {
	monitorInstance, err := dipool.GetInstance[monitor.IMonitor]()
	if err != nil {
		logger.Errorf("[StartMonitor] get monitor instance failed: %v", err)
		httputils.Respond(c, statuscode.StatusInternalError("internal server error"))
		return
	}

	var req dto.MonitorStartRequest
	bindRes := httputils.Bind(c, &req)
	if !bindRes.IsSuccess() {
		httputils.Respond(c, bindRes)
		return
	}

	if req.ProjectId <= 0 {
		httputils.Respond(c, statuscode.StatusBadRequest("invalid project id", statuscode.MessageCode_HttpRequestFailed))
		return
	}

	err = monitorInstance.Start(req.ProjectId)

	if err == nil {
		httputils.Respond(c, statuscode.StatusOK(nil))
	} else {
		logger.Errorf("[StartMonitor] failed to start monitor: %v", err)
		httputils.Respond(c, statuscode.StatusBadRequest("failed to start monitor", statuscode.MessageCode_HttpRequestFailed))
	}
}

func StopMonitor(c *api.Context) {
	monitorInstance, err := dipool.GetInstance[monitor.IMonitor]()
	if err != nil {
		logger.Errorf("[StopMonitor] failed to get monitor instance : %v", err)
		httputils.Respond(c, statuscode.StatusInternalError("internal server error"))
		return
	}

	var req dto.MonitorStopRequest
	bindRes := httputils.Bind(c, &req)
	if !bindRes.IsSuccess() {
		httputils.Respond(c, bindRes)
		return
	}

	err = monitorInstance.Stop(req.ProjectId)
	if err == nil {
		httputils.Respond(c, statuscode.StatusOK(nil))
	} else {
		logger.Errorf("[StopMonitor] failed to stop monitor: %v", err)
		httputils.Respond(c, statuscode.StatusBadRequest("failed to stop monitor", statuscode.MessageCode_HttpRequestFailed))
	}
}

func GetSFPList(c *api.Context) {
	monitorInstance, err := dipool.GetInstance[monitor.IMonitor]()
	if err != nil {
		logger.Errorf("[GetSFPList] failed to get monitor instance : %v", err)
		httputils.Respond(c, statuscode.StatusInternalError("internal server error"))
		return
	}

	projectIdStr := c.Param("projectId")
	if projectIdStr == "" {
		httputils.Respond(c, statuscode.StatusBadRequest(
			"projectId is required", statuscode.MessageCode_HttpRequestFailed))
		return
	}

	projectId, err := strconv.ParseInt(projectIdStr, 10, 64)
	if err != nil {
		logger.Warnf("[GetSFPList] invalid projectId: %v, err: %v", projectIdStr, err)
		httputils.Respond(c, statuscode.StatusBadRequest(
			"invalid projectId", statuscode.MessageCode_HttpRequestFailed))
		return
	}

	monitorStatus, monitorStatusExists := monitorInstance.GetMonitorStatus(projectId)

	if monitorStatusExists && monitorStatus != monitor.MonitorStatusRunning {
		httputils.Respond(c, statuscode.StatusBadRequest(
			"monitor is not running", statuscode.MessageCode_HttpRequestFailed))
		return
	}

	sfpLinks, err := monitorInstance.GetSFPLinks(projectId)
	if err != nil {
		httputils.Respond(c, statuscode.StatusBadRequest(
			fmt.Sprintf("failed to get SFP links: %v", err),
			statuscode.MessageCode_HttpRequestFailed))
		return
	}

	result := &dto.MonitorSFPListResponse{}
	resultSFPLinks := make([]*dto.MonitorSFPLink, 0, len(sfpLinks))

	for _, link := range sfpLinks {
		resultSFPLinks = append(resultSFPLinks, &dto.MonitorSFPLink{
			Source: dto.MonitorSFPPort{
				DeviceId:      link.Source.DeviceId,
				DeviceIp:      link.Source.DeviceIp,
				InterfaceId:   link.Source.InterfaceId,
				InterfaceName: link.Source.InterfaceName,
				ModelName:     link.Source.ModelName,
				RxPower:       link.Source.RxPower,
				TxPower:       link.Source.TxPower,
				TemperatureC:  link.Source.TemperatureC,
				TemperatureF:  link.Source.TemperatureF,
				Voltage:       link.Source.Voltage,
			},
			Target: dto.MonitorSFPPort{
				DeviceId:      link.Target.DeviceId,
				DeviceIp:      link.Target.DeviceIp,
				InterfaceId:   link.Target.InterfaceId,
				InterfaceName: link.Target.InterfaceName,
				ModelName:     link.Target.ModelName,
				RxPower:       link.Target.RxPower,
				TxPower:       link.Target.TxPower,
				TemperatureC:  link.Target.TemperatureC,
				TemperatureF:  link.Target.TemperatureF,
				Voltage:       link.Target.Voltage,
			},
		})
	}

	result.SFPList = resultSFPLinks
	c.JSON(http.StatusOK, result)
}
