package importconfig

import (
	"fmt"
	"sync"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/logging"
)

type ImportConfig struct {
	stopFlag bool
	wsConnId string
	wsOpCode int64
	mutex    sync.Mutex
	// mafImportConfigTimeout         int
	onFailedResponse               func(connId string, opcode int64, statusCode int64, errMessage string)
	onProgressResponse             func(connId string, opcode int64, progress int64)
	onDeviceResultProgressResponse func(connId string, opcode int64, deviceResult wscommand.OperationsDeviceResult)
	onCompletedResponse            func(connId string, opcode int64)
}

func NewImportConfig(
	onFailedResponse func(connId string, opcode int64, statusCode int64, errMessage string),
	onProgressResponse func(connId string, opcode int64, progress int64),
	onDeviceResultProgressResponse func(connId string, opcode int64, deviceResult wscommand.OperationsDeviceResult),
	onCompletedResponse func(connId string, opcode int64),
) IImportConfig {
	return &ImportConfig{
		stopFlag: false,
		// mafImportConfigTimeout:         3,
		wsOpCode:                       wscommand.ActWSCommandStartImportDeviceConfig.Int64(),
		onFailedResponse:               onFailedResponse,
		onProgressResponse:             onProgressResponse,
		onDeviceResultProgressResponse: onDeviceResultProgressResponse,
		onCompletedResponse:            onCompletedResponse,
	}
}

var logger = logging.NewWithField("module", "importconfig")

func (importconfig *ImportConfig) Stop() {
	logger.Info("Stop ImportConfig module")
	importconfig.stopFlag = true
	return
}

// "Id": int64, // only support a Device
func (importconfig *ImportConfig) StartImportConfig(connId string, projectId int64, deviceIds []int64) {
	logger.Info("Start ImportConfig module")
	importconfig.wsConnId = connId
	importconfig.stopFlag = false

	var project domain.Project
	var status statuscode.Response
	var err error

	// Update Project Status to DeviceConfiguring
	configuringStatus := domain.ProjectStatusEnum_DeviceConfiguring
	status = core.UpdateProjectStatus(projectId, configuringStatus)
	if !status.IsSuccess() {
		logger.Error("Error UpdateProjectStatus:", status.ErrorMessage)
		importconfig.onFailedResponse(importconfig.wsConnId, importconfig.wsOpCode, int64(status.StatusCode), status.ErrorMessage)
		return
	}

	// Use defer to handle Stop response
	defer func() {
		if importconfig.stopFlag {
			msg := "ImportConfig has been stopped by user request"
			logger.Info(msg)
			importconfig.onFailedResponse(importconfig.wsConnId, importconfig.wsOpCode, wscommand.ActWSResponseStatusStop.Int64(), msg)
		}

		idle_status := domain.ProjectStatusEnum_Idle
		status = core.UpdateProjectStatus(projectId, idle_status)
		if !status.IsSuccess() {
			logger.Error("Error UpdateProjectStatus:", status.ErrorMessage)
		}
	}()

	// Check Only one device ID. Currently supported a device.
	if len(deviceIds) > 1 {
		err := fmt.Errorf("Invalid input: device count exceeds one")
		importconfig.onFailedResponse(importconfig.wsConnId, importconfig.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}
	if len(deviceIds) == 0 {
		err := fmt.Errorf("Invalid input: no devices were provided")
		importconfig.onFailedResponse(importconfig.wsConnId, importconfig.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}

	// Get device ID
	deviceId := deviceIds[0]

	// Get Project(Monitor Project)
	isOperation := true
	showPassword := true
	project, status = core.GetProject(projectId, isOperation, showPassword)
	if !status.IsSuccess() {
		logger.Error("Error GetProject:", status.ErrorMessage)
		importconfig.onFailedResponse(importconfig.wsConnId, importconfig.wsOpCode, int64(status.StatusCode), status.ErrorMessage)
		return
	}
	if importconfig.stopFlag {
		return
	}

	// // Create target Device
	// targetDevices, err := operations.CreateDeviceList(project, deviceIds)
	// if err != nil {
	// 	logger.Error("Error CreateDeviceList():", err)
	// 	importconfig.onFailedResponse(importconfig.wsConnId, importconfig.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
	// 	return
	// }

	// Clear MAF Device
	err = common.ClearMAFCache()
	if err != nil {
		logger.Error("Error ClearMAFCache():", err)
		importconfig.onFailedResponse(importconfig.wsConnId, importconfig.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}

	importconfig.onProgressResponse(importconfig.wsConnId, importconfig.wsOpCode, 20)

	// Search All project devices by IP list
	// projectDevicesIpList := operations.CreateIpList(project.Devices)
	device, err := operations.GetProjectDeviceById(project, deviceId)
	if err != nil {
		logger.Errorf("Device not found at Project. err: %v", err)
		importconfig.onFailedResponse(importconfig.wsConnId, importconfig.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}

	deviceIp := device.Ipv4.IpAddress
	ipList := []string{deviceIp}
	mafDeviceIpIdMap, err := common.MafIpListSearch(ipList)
	if importconfig.stopFlag {
		return
	}
	if err != nil {
		logger.Error("Error MafIpListSearch():", err)
		importconfig.onFailedResponse(importconfig.wsConnId, importconfig.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}

	// Set MAF Secret(All project Devices)
	deviceList := []domain.Device{device}
	err = operations.SetMAFDevicesSecret(deviceList, mafDeviceIpIdMap)
	if err != nil {
		logger.Error("Error SetMAFDevicesSecret():", err)
		importconfig.onFailedResponse(importconfig.wsConnId, importconfig.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}
	importconfig.onProgressResponse(importconfig.wsConnId, importconfig.wsOpCode, 40)

	// Create Job Worker
	mafJobWorker := common.NewMAFJobWorker()

	mafDevicesIdList := operations.GetMAFDevicesId(mafDeviceIpIdMap)
	arpEnable := bool(false)
	err = mafJobWorker.EnableMAFDevicesSNMPByJob(mafDevicesIdList, arpEnable)
	if err != nil {
		logger.Error("Error EnableMAFDevicesSNMPByJob():", err)
		importconfig.onFailedResponse(importconfig.wsConnId, importconfig.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}
	importconfig.onProgressResponse(importconfig.wsConnId, importconfig.wsOpCode, 60)

	//	Get MAF device ID
	var mafDeviceId string
	if mafDevId, ok := mafDeviceIpIdMap[deviceIp]; ok {
		mafDeviceId = mafDevId
	} else { // not found the key
		err := fmt.Errorf("Device IP(%s) not found at MAF device map.", deviceIp)
		importconfig.onFailedResponse(importconfig.wsConnId, importconfig.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}

	err = importconfig.executeDeviceImportConfig(device, mafDeviceId, project)
	if importconfig.stopFlag {
		return
	}
	if err != nil {
		logger.Error("Error executeDevicesImportConfig():", err)
		importconfig.onFailedResponse(importconfig.wsConnId, importconfig.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}
	// Finished
	importconfig.onCompletedResponse(importconfig.wsConnId, importconfig.wsOpCode)

}
