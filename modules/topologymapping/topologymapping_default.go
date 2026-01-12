package topologymapping

import (
	"encoding/json"
	"fmt"
	"sync"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/logging"
)

type TopologyMapper struct {
	stopFlag       bool
	wsConnId       string
	mutex          sync.Mutex
	simpleProfiles *domain.SimpleDeviceProfiles

	onFailedResponse    func(connId string, statusCode int64, errMessage string)
	onProgressResponse  func(connId string, progress int64)
	onCompletedResponse func(connId string, mappingResult domain.TopologyMappingResult)
}

func NewTopologyMapper(
	onFailedResponse func(connId string, statusCode int64, errMessage string),
	onProgressResponse func(connId string, progress int64),
	onCompletedResponse func(connId string, mappingResult domain.TopologyMappingResult),
) ITopologyMapper {
	return &TopologyMapper{
		stopFlag:            false,
		onFailedResponse:    onFailedResponse,
		onProgressResponse:  onProgressResponse,
		onCompletedResponse: onCompletedResponse,
	}
}

var logger = logging.NewWithField("module", "topologyMapping")

const (
	broadcastSearchTimeout = 5 // 5 seconds
	mafGenericErrorBase    = 100000000
	moxaVendorId           = "8691"
	moxaVendorString       = "MOXA"
)

func (topologyMapper *TopologyMapper) Stop() {
	logger.Info("Stop TopologyMapping module")
	topologyMapper.stopFlag = true
	return
}

// "Id": [], // Device ID list, Empty:All
// "DaselineId": -1 // Use the current project(for OPCUA)
// "SkipMappingDevice": true // Skip the toplogy mapping
func (topologyMapper *TopologyMapper) TopologyMapping(connId string, projectId int64, baselineId int64) {
	logger.Info("Start TopologyMapping module")
	topologyMapper.wsConnId = connId
	topologyMapper.stopFlag = false

	var project domain.Project
	var projectWithdeviceConfig domain.ProjectWithDeviceConfig
	// var deployDeviceList domain.DeployDeviceList
	var status statuscode.Response
	var err error

	// Get Device Simple Profile
	topologyMapper.simpleProfiles, status = core.GetSimpleDeviceProfiles()
	if !status.IsSuccess() {
		errorMsg := fmt.Sprintf("Failed to get simple device profile. Error:%s", status.ErrorMessage)
		logger.Error(errorMsg)
		topologyMapper.onFailedResponse(topologyMapper.wsConnId, int64(status.StatusCode), errorMsg)
		return
	}

	// Update Project Status to Deploying
	deploying_status := domain.ProjectStatusEnum_Deploying
	status = core.UpdateProjectStatus(projectId, deploying_status)
	if !status.IsSuccess() {
		logger.Error("Error UpdateProjectStatus:", status.ErrorMessage)
		topologyMapper.onFailedResponse(topologyMapper.wsConnId, int64(status.StatusCode), status.ErrorMessage)
		return
	}

	// Use defer to handle Stop response
	defer func() {
		if topologyMapper.stopFlag {
			msg := "TopologyMapping has been stopped by user request"
			logger.Info(msg)
			topologyMapper.onFailedResponse(topologyMapper.wsConnId, wscommand.ActWSResponseStatusStop.Int64(), msg)
		}

		idle_status := domain.ProjectStatusEnum_Idle
		status = core.UpdateProjectStatus(projectId, idle_status)
		if !status.IsSuccess() {
			logger.Error("Error UpdateProjectStatus:", status.ErrorMessage)
		}
	}()

	// Get Project & DeviceConfig
	if baselineId == -1 {
		// CURRENT Baseline use the current project to deploy
		is_operation := false
		show_password := true
		project, status = core.GetProject(projectId, is_operation, show_password)
		if !status.IsSuccess() {
			logger.Error("Error GetProject:", status.ErrorMessage)
			topologyMapper.onFailedResponse(topologyMapper.wsConnId, int64(status.StatusCode), status.ErrorMessage)
			return
		}
		projectWithdeviceConfig, status = core.GetProjectDeviceConfig(projectId)
		if !status.IsSuccess() {
			logger.Error("Error GetProjectDeviceConfig:", status.ErrorMessage)
			topologyMapper.onFailedResponse(topologyMapper.wsConnId, int64(status.StatusCode), status.ErrorMessage)
			return
		}
		// deployDeviceList, status = core.GetProjectDeployDevices(projectId)
		// if !status.IsSuccess() {
		// 	logger.Error("Error GetProjectDeployDevices:", status.ErrorMessage)
		// 	topologyMapper.onFailedResponse(topologyMapper.wsConnId, int64(status.StatusCode), status.ErrorMessage)
		// 	return
		// }
	} else {
		// Get Baseline Project & DeviceConfig to deploy
		project, status = core.GetDesignBaselineProject(projectId, baselineId)
		if !status.IsSuccess() {
			logger.Error("Error GetDesignBaselineProject:", status.ErrorMessage)
			topologyMapper.onFailedResponse(topologyMapper.wsConnId, int64(status.StatusCode), status.ErrorMessage)
			return
		}
		projectWithdeviceConfig, status = core.GetDesignBaselineProjectDeviceConfig(projectId, baselineId)
		if !status.IsSuccess() {
			logger.Error("Error GetDesignBaselineProjectDeviceConfig:", status.ErrorMessage)
			topologyMapper.onFailedResponse(topologyMapper.wsConnId, int64(status.StatusCode), status.ErrorMessage)
			return
		}
		// deployDeviceList, status = core.GetDesignBaselineDeployDevices(projectId, baselineId)
		// if !status.IsSuccess() {
		// 	logger.Error("Error GetDesignBaselineDeployDevices:", status.ErrorMessage)
		// 	topologyMapper.onFailedResponse(topologyMapper.wsConnId, int64(status.StatusCode), status.ErrorMessage)
		// 	return
		// }

		logger.Info(fmt.Sprintf("MappingDeviceIpSettingTables: %v", projectWithdeviceConfig.DeviceConfig.DeviceIpSettingTables))
	}
	if topologyMapper.stopFlag {
		return
	}

	// Clear MAF Device
	err = topologyMapper.clearMAFCache()
	if err != nil {
		logger.Error("Error clearMAFCache():", err)
		topologyMapper.onFailedResponse(topologyMapper.wsConnId, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}

	topologyMapper.onProgressResponse(topologyMapper.wsConnId, 20)
	// Scan Online Topology(Physical)
	// var onlineTopology mappingTopology
	onlineTopology, err := topologyMapper.getPhysicalTopology(project)
	if topologyMapper.stopFlag {
		return
	}
	if err != nil {
		logger.Error("Error clearMAFCache():", err)
		topologyMapper.onFailedResponse(topologyMapper.wsConnId, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}
	jsonBytes, _ := json.MarshalIndent(onlineTopology, "", "  ")
	logger.Info("PhysicalTopology:\n" + string(jsonBytes))
	topologyMapper.onProgressResponse(topologyMapper.wsConnId, 80)

	// Offline Topology(Design)
	offlineTopology := topologyMapper.createMappingTopologyByProject(project)

	// Find the Offline Source candidates (Design Management Endpoint)
	offlineSourceCandidates, err := topologyMapper.findOfflineSourceCandidates(offlineTopology, onlineTopology)
	if topologyMapper.stopFlag {
		return
	}

	if err != nil {
		logger.Error("Error findOfflineSourceCandidates():", err)
		topologyMapper.onFailedResponse(topologyMapper.wsConnId, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}

	// Mapping Topology
	mappingResult, err := topologyMapper.compareTopologyByCandidates(offlineTopology, onlineTopology, offlineSourceCandidates)
	if topologyMapper.stopFlag {
		return
	}

	if err != nil {
		logger.Error("Error compareTopologyByCandidates():", err)
		topologyMapper.onFailedResponse(topologyMapper.wsConnId, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}
	jsonBytes, _ = json.MarshalIndent(mappingResult, "", "  ")
	logger.Info("MappingResult:\n" + string(jsonBytes))
	topologyMapper.onProgressResponse(topologyMapper.wsConnId, 90)
	if topologyMapper.stopFlag {
		return
	}

	if topologyMapper.stopFlag {
		return
	}

	// Update Project DeviceIpSettingConfig by Mapping result
	err = topologyMapper.updateProjectDeviceIpSettingConfigByResult(project, mappingResult)
	if err != nil {
		logger.Error("Error updateProjectDeviceIpSettingConfigByResult():", err)
		topologyMapper.onFailedResponse(topologyMapper.wsConnId, wscommand.ActWSResponseStatusFailed.Int64(), err.Error())
		return
	}

	// Finished
	topologyMapper.onCompletedResponse(topologyMapper.wsConnId, mappingResult)

}
