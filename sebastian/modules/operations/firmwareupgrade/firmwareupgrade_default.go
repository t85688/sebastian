package firmwareupgrade

import (
	"sync"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/logging"
)

type FirmwareUpgrade struct {
	stopFlag                       bool
	wsConnId                       string
	wsOpCode                       int64
	mutex                          sync.Mutex
	mafFirmwareUpgradeTimeout      int
	onFailedResponse               func(connId string, opcode int64, statusCode int64, errMessage string)
	onProgressResponse             func(connId string, opcode int64, progress int64)
	onDeviceResultProgressResponse func(connId string, opcode int64, deviceResult wscommand.OperationsDeviceResult)
	onCompletedResponse            func(connId string, opcode int64)
}

func NewFirmwareUpgrade(
	onFailedResponse func(connId string, opcode int64, statusCode int64, errMessage string),
	onProgressResponse func(connId string, opcode int64, progress int64),
	onDeviceResultProgressResponse func(connId string, opcode int64, deviceResult wscommand.OperationsDeviceResult),
	onCompletedResponse func(connId string, opcode int64),
) IFirmwareUpgrade {
	return &FirmwareUpgrade{
		stopFlag:                       false,
		mafFirmwareUpgradeTimeout:      30,
		wsOpCode:                       wscommand.ActWSCommandStartExportDeviceConfig.Int64(),
		onFailedResponse:               onFailedResponse,
		onProgressResponse:             onProgressResponse,
		onDeviceResultProgressResponse: onDeviceResultProgressResponse,
		onCompletedResponse:            onCompletedResponse,
	}
}

var logger = logging.NewWithField("module", "firmwareupgrade")

func (firmwareupgrade *FirmwareUpgrade) Stop() {
	logger.Info("Stop FirmwareUpgrade module")
	firmwareupgrade.stopFlag = true
	return
}

// "Id": [], // Device ID list
func (firmwareupgrade *FirmwareUpgrade) StartFirmwareUpgrade(connId string, projectId int64, deviceIds []int64, firmwareFileName string) {
	logger.Info("Start FirmwareUpgrade module")
	firmwareupgrade.wsConnId = connId
	firmwareupgrade.stopFlag = false

	var project domain.Project
	var status statuscode.Response
	var err error

	// Update Project Status to DeviceConfiguring
	configuringStatus := domain.ProjectStatusEnum_DeviceConfiguring
	status = core.UpdateProjectStatus(projectId, configuringStatus)
	if !status.IsSuccess() {
		logger.Error("Error UpdateProjectStatus:", status.ErrorMessage)
		firmwareupgrade.onFailedResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, int64(status.StatusCode), status.ErrorMessage)
		return
	}

	// Use defer to handle Stop response
	defer func() {
		if firmwareupgrade.stopFlag {
			msg := "FirmwareUpgrade has been stopped by user request"
			logger.Info(msg)
			firmwareupgrade.onFailedResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, wscommand.ActWSResponseStatusStop.Int64(), msg)
		}

		idle_status := domain.ProjectStatusEnum_Idle
		status = core.UpdateProjectStatus(projectId, idle_status)
		if !status.IsSuccess() {
			logger.Error("Error UpdateProjectStatus:", status.ErrorMessage)
		}
	}()

	// Get Project(Monitor Project)
	isOperation := true
	showPassword := true
	project, status = core.GetProject(projectId, isOperation, showPassword)
	if !status.IsSuccess() {
		logger.Error("Error GetProject:", status.ErrorMessage)
		firmwareupgrade.onFailedResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, int64(status.StatusCode), status.ErrorMessage)
		return
	}
	if firmwareupgrade.stopFlag {
		return
	}

	// Create target Device
	targetDevices, err := operations.CreateDeviceList(project, deviceIds)
	if err != nil {
		logger.Error("Error CreateDeviceList():", err)
		firmwareupgrade.onFailedResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}

	// Clear MAF Device
	err = common.ClearMAFCache()
	if err != nil {
		logger.Error("Error ClearMAFCache():", err)
		firmwareupgrade.onFailedResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}

	firmwareupgrade.onProgressResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, 20)

	// Search All project devices by IP list
	projectDevicesIpList := operations.CreateIpList(project.Devices)
	mafDeviceIpIdMap, err := common.MafIpListSearch(projectDevicesIpList)
	if firmwareupgrade.stopFlag {
		return
	}
	if err != nil {
		logger.Error("Error MafIpListSearch():", err)
		firmwareupgrade.onFailedResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}

	// Check target Device exists (Update target devices)
	targetDevices, missingDevices := operations.SplitExistingAndMissingDevices(targetDevices, mafDeviceIpIdMap)
	for _, device := range missingDevices {
		firmwareupgrade.onDeviceResultProgressResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, wscommand.OperationsDeviceResult{
			Id:           device.Id,
			Status:       wscommand.OperationsDeviceResultStatusFailed.String(),
			Progress:     30,
			ErrorMessage: "Not found",
		})
	}
	// Get Project exists devices
	projectExistsDevices, _ := operations.SplitExistingAndMissingDevices(project.Devices, mafDeviceIpIdMap)

	// Set MAF Secret(All project Devices)
	err = operations.SetMAFDevicesSecret(projectExistsDevices, mafDeviceIpIdMap)
	if err != nil {
		logger.Error("Error SetMAFDevicesSecret():", err)
		firmwareupgrade.onFailedResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}
	firmwareupgrade.onProgressResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, 40)

	// Create Job Worker
	mafJobWorker := common.NewMAFJobWorker()

	// Enable SNMP
	// targetMafDevicesIdList, err := operations.GetTargetMAFDevicesId(project.Devices, mafDeviceIpIdMap)
	// if err != nil {
	// 	logger.Error("Error GetTargetMAFDevicesId():", err)
	// 	firmwareupgrade.onFailedResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
	// 	return
	// }
	mafDevicesIdList := operations.GetMAFDevicesId(mafDeviceIpIdMap)
	arpEnable := bool(false)
	err = mafJobWorker.EnableMAFDevicesSNMPByJob(mafDevicesIdList, arpEnable)
	if err != nil {
		logger.Error("Error EnableMAFDevicesSNMPByJob():", err)
		firmwareupgrade.onFailedResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}
	firmwareupgrade.onProgressResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, 60)

	// Create DeviceID map<ProjectDeviceID, MAFDeviceID>
	projectDeviceIdMap, err := operations.CreateDeviceIdMap(projectExistsDevices, mafDeviceIpIdMap)
	if err != nil {
		logger.Error("Error CreateDeviceIdMap():", err)
		firmwareupgrade.onFailedResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}

	// Sort by distance
	projectDevicesIdSequence, err := operations.SortDevicesByDistance(projectDeviceIdMap)
	if firmwareupgrade.stopFlag {
		return
	}
	if err != nil {
		logger.Error("Error SortDevicesByDistance():", err)
		firmwareupgrade.onFailedResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}

	// Get Target Device execute sequence
	targetDevicesIdSequence := operations.GetTargetDevicesIdSequence(targetDevices, projectDevicesIdSequence)
	err = firmwareupgrade.executeDevicesFirmwareUpgrade(targetDevicesIdSequence, projectDeviceIdMap, firmwareFileName)
	if firmwareupgrade.stopFlag {
		return
	}
	if err != nil {
		logger.Error("Error executeDevicesFirmwareUpgrade():", err)
		firmwareupgrade.onFailedResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}
	// Finished
	firmwareupgrade.onCompletedResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode)

}
