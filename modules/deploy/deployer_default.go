package deploy

import (
	"fmt"
	"sync"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/logging"
)

type Deployer struct {
	stopFlag                       bool
	wsConnId                       string
	mutex                          sync.Mutex
	deployArpEnable                bool
	onFailedResponse               func(connId string, statusCode int64, errMessage string)
	onProgressResponse             func(connId string, progress int64)
	onDeviceResultProgressResponse func(connId string, progress int64, deployDeviceResult DeployDeviceResult)
	onCompletedResponse            func(connId string)
}

func NewDeployer(
	onFailedResponse func(connId string, statusCode int64, errMessage string),
	onProgressResponse func(connId string, progress int64),
	onDeviceResultProgressResponse func(connId string, progress int64, deployDeviceResult DeployDeviceResult),
	onCompletedResponse func(connId string),
) IDeployer {
	return &Deployer{
		stopFlag:                       false,
		onFailedResponse:               onFailedResponse,
		onProgressResponse:             onProgressResponse,
		onDeviceResultProgressResponse: onDeviceResultProgressResponse,
		onCompletedResponse:            onCompletedResponse,
	}
}

var logger = logging.NewWithField("module", "deploy")
var stopStatusCode = int64(1005)
var failedStatusCode = int64(1006)
var MafGenericErrorBase = int(100000000) // # Generic Errors

func (deployer *Deployer) Stop() {
	logger.Info("Stop Deploy module")
	deployer.stopFlag = true
	return
}

// "Id": [], // Device ID list, Empty:All
// "DaselineId": -1 // Use the current project(for OPCUA)
// "SkipMappingDevice": true // Skip the toplogy mapping
func (deployer *Deployer) Deploy(connId string, projectId int64, baselineId int64, deviceIds []int64, skipMappingDevice bool) {
	logger.Info("Start Deploy module")
	deployer.wsConnId = connId
	deployer.stopFlag = false
	deployer.deployArpEnable = !skipMappingDevice

	var project domain.Project
	var projectWithdeviceConfig domain.ProjectWithDeviceConfig
	var deployDeviceList domain.DeployDeviceList
	var status statuscode.Response
	var err error

	// Update Project Status to Deploying
	deploying_status := domain.ProjectStatusEnum_Deploying
	status = core.UpdateProjectStatus(projectId, deploying_status)
	if !status.IsSuccess() {
		logger.Error("Error UpdateProjectStatus:", status.ErrorMessage)
		deployer.onFailedResponse(deployer.wsConnId, int64(status.StatusCode), status.ErrorMessage)
		return
	}

	// Use defer to handle Stop response
	defer func() {
		if deployer.stopFlag {
			msg := "Deployment has been stopped by user request"
			logger.Info(msg)
			deployer.onFailedResponse(deployer.wsConnId, stopStatusCode, msg)
		}

		idle_status := domain.ProjectStatusEnum_Idle
		status = core.UpdateProjectStatus(projectId, idle_status)
		if !status.IsSuccess() {
			logger.Error("Error UpdateProjectStatus:", status.ErrorMessage)
		}
	}()

	// 1. Generate the Offline configuration (Cogsworth)
	//    -> response: MAP(DeviceID, FileID)
	// 2. Search Device
	//   2.1 No duplicated IP by IP list (DM)
	//   2.2 Duplicated IP by Broadcast Search (DM)
	// 3. Mapping MAF devices & Chamberlain device
	//   3.1 Get Devices (DM)
	// 4. Enable SNMP service (DM)
	// 5. Update the offline configuration to DUT
	//   5.1 Sort-distance (DM)
	//   5.2 Upload offline_config to DUT(DM)
	//     - Use Jobs

	// Get Project & DeviceConfig
	if baselineId == -1 {
		// CURRENT Baseline use the current project to deploy
		is_operation := false
		show_password := false
		project, status = core.GetProject(projectId, is_operation, show_password)
		if !status.IsSuccess() {
			logger.Error("Error GetProject:", status.ErrorMessage)
			deployer.onFailedResponse(deployer.wsConnId, int64(status.StatusCode), status.ErrorMessage)
			return
		}
		projectWithdeviceConfig, status = core.GetProjectDeviceConfig(projectId)
		if !status.IsSuccess() {
			logger.Error("Error GetProjectDeviceConfig:", status.ErrorMessage)
			deployer.onFailedResponse(deployer.wsConnId, int64(status.StatusCode), status.ErrorMessage)
			return
		}
		deployDeviceList, status = core.GetProjectDeployDevices(projectId)
		if !status.IsSuccess() {
			logger.Error("Error GetProjectDeployDevices:", status.ErrorMessage)
			deployer.onFailedResponse(deployer.wsConnId, int64(status.StatusCode), status.ErrorMessage)
			return
		}
	} else {
		// Get Baseline Project & DeviceConfig to deploy
		project, status = core.GetDesignBaselineProject(projectId, baselineId)
		if !status.IsSuccess() {
			logger.Error("Error GetDesignBaselineProject:", status.ErrorMessage)
			deployer.onFailedResponse(deployer.wsConnId, int64(status.StatusCode), status.ErrorMessage)
			return
		}
		projectWithdeviceConfig, status = core.GetDesignBaselineProjectDeviceConfig(projectId, baselineId)
		if !status.IsSuccess() {
			logger.Error("Error GetDesignBaselineProjectDeviceConfig:", status.ErrorMessage)
			deployer.onFailedResponse(deployer.wsConnId, int64(status.StatusCode), status.ErrorMessage)
			return
		}
		deployDeviceList, status = core.GetDesignBaselineDeployDevices(projectId, baselineId)
		if !status.IsSuccess() {
			logger.Error("Error GetDesignBaselineDeployDevices:", status.ErrorMessage)
			deployer.onFailedResponse(deployer.wsConnId, int64(status.StatusCode), status.ErrorMessage)
			return
		}

		logger.Info(fmt.Sprintf("MappingDeviceIpSettingTables: %v", projectWithdeviceConfig.DeviceConfig.DeviceIpSettingTables))
	}
	deployer.onProgressResponse(deployer.wsConnId, 10)
	if deployer.stopFlag {
		return
	}

	// Clear MAF Device
	err = deployer.clearMAFCache()
	if err != nil {
		logger.Error("Error clearMAFCache():", err)
		deployer.onFailedResponse(deployer.wsConnId, failedStatusCode, err.Error())
		return
	}

	if len(deviceIds) == 0 {
		for _, deployDevice := range deployDeviceList.DeviceList {
			deviceIds = append(deviceIds, deployDevice.DeviceId)
		}
	}
	logger.Info(fmt.Sprintf("Deploy deviceId size:%d", len(deviceIds)))

	// 1. Generate the Offline configuration (Cogsworth)
	deviceOfflineConfigMap, status := core.GenerateDevicesOfflineConfig(projectId, baselineId, deviceIds)
	if !status.IsSuccess() {
		logger.Error(fmt.Sprintf("Generate DevicesOfflineConfig failed. err:%s", status.ErrorMessage))
		deployer.onFailedResponse(deployer.wsConnId, int64(status.StatusCode), status.ErrorMessage)
		return
	}
	logger.Info(fmt.Sprintf("DeviceOfflineConfigMap size:%d", len(deviceOfflineConfigMap)))

	// 2. Search Device
	// 3. Mapping MAF devices & Chamberlain device
	var deviceMappingMap map[int64]string
	if skipMappingDevice {
		//  2.1 No duplicated IP by IP list (DM)
		//  3.1 Get Devices (DM)
		deviceMappingMap, err = deployer.ipListDeviceSearch(project, deviceIds)
		if err != nil {
			logger.Error("Error ipListDeviceSearch():", err)
			deployer.onFailedResponse(deployer.wsConnId, failedStatusCode, err.Error())
			return
		}
	} else {
		// 2.2 Duplicated IP by Broadcast Search (DM)
		// 3.1 Get Devices (DM)
		deviceMappingMap, err = deployer.broadcastDeviceSearch(project, deviceIds, projectWithdeviceConfig)
		if err != nil {
			logger.Error("Error broadcastDeviceSearch():", err)
			deployer.onFailedResponse(deployer.wsConnId, failedStatusCode, err.Error())
			return
		}
	}
	deployer.onProgressResponse(deployer.wsConnId, 20)
	if deployer.stopFlag {
		return
	}

	// Check deploy devices exists
	deployer.checkDevicesExists(deviceIds, deviceMappingMap, 25)
	// Renew the deviceIds for found devices
	deviceIds = getNewDeployDeviceIdsByMappingMap(deviceMappingMap)

	// Check new deviceIds not empty
	if len(deviceIds) == 0 {
		err := fmt.Errorf("Not found any devices")
		logger.Error(err)
		deployer.onFailedResponse(deployer.wsConnId, failedStatusCode, "Not found devices")
		return
	}

	logger.Info(fmt.Sprintf("DeviceMappingMap size:%d", len(deviceMappingMap)))
	for deviceId, mafId := range deviceMappingMap {
		logger.Info(fmt.Sprintf("DeviceId: %v, MafDeviceId: %v", deviceId, mafId))
	}

	// Update MAF Device Secret
	err = deployer.setMAFDevicesSecret(project, deviceIds, deviceMappingMap)
	if err != nil {
		logger.Error("Error setMAFDevicesSecret():", err)
		deployer.onFailedResponse(deployer.wsConnId, failedStatusCode, err.Error())
		return
	}

	// 4. Enable SNMP service (DM)
	// _ = enableDevicesSNMP(deviceIds, deviceMappingMap)
	_ = deployer.enableDevicesSNMPByJob(deviceIds, deviceMappingMap)
	if deployer.stopFlag {
		return
	}

	// 5. Upload the offline configuration to DUT
	// 5.1 Sort-distance (DM)
	var deploySequenceByDeviceIds []int64
	deploySequenceByDeviceIds, err = deployer.sortDevicesByDistance(deviceMappingMap)
	if err != nil {
		logger.Warn("Warning sortDevicesByDistance() failed:", err)
	}
	if len(deploySequenceByDeviceIds) == 0 {
		deploySequenceByDeviceIds, err = deployer.sortDevicesByIp(project, deviceMappingMap)
		if err != nil {
			logger.Error("Error sortDevicesByIp():", err)
			deployer.onFailedResponse(deployer.wsConnId, failedStatusCode, err.Error())
			return
		}
	}

	deployer.onProgressResponse(deployer.wsConnId, 30)
	if deployer.stopFlag {
		return
	}

	// 5.2 Upload offline_config to DUT(DM)
	err = deployer.uploadDevicesOfflineConfig(deploySequenceByDeviceIds, deviceMappingMap, deviceOfflineConfigMap)
	if deployer.stopFlag {
		return
	}
	if err != nil {
		logger.Error("Error uploadDevicesOfflineConfig():", err)
		deployer.onFailedResponse(deployer.wsConnId, failedStatusCode, err.Error())
		return
	}

	var deviceConfig domain.DeviceConfig
	err = deployer.syncDevicesUserAccount(project, deviceConfig.UserAccountTables, deviceIds)
	if err != nil {
		logger.Error("Error syncDevicesUserAccount():", err)
		deployer.onFailedResponse(deployer.wsConnId, failedStatusCode, err.Error())
		return
	}

	// Update Project's Devices
	projectWithDevices := domain.ProjectWithDevices{
		Id:      projectId,
		Devices: project.Devices,
	}

	if baselineId == -1 {
		status := core.UpdateProjectDevices(projectWithDevices)
		if !status.IsSuccess() {
			logger.Error("Error UpdateProjectDevices():", status.ErrorMessage)
			deployer.onFailedResponse(deployer.wsConnId, int64(status.StatusCode), status.ErrorMessage)
			return
		}
	} else {
		status := core.UpdateDesignBaselineProjectDevices(baselineId, projectWithDevices)
		if !status.IsSuccess() {
			logger.Error("Error UpdateDesignBaselineProjectDevices():", status.ErrorMessage)
			deployer.onFailedResponse(deployer.wsConnId, int64(status.StatusCode), status.ErrorMessage)
			return
		}
	}

	// Finished
	deployer.onCompletedResponse(deployer.wsConnId)

}
