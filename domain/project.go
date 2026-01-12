package domain

import (
	"encoding/json"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

type ProjectStatusEnum int

const (
	ProjectStatusEnum_Idle ProjectStatusEnum = iota
	ProjectStatusEnum_Computing
	ProjectStatusEnum_Comparing
	ProjectStatusEnum_Deploying
	ProjectStatusEnum_Syncing
	ProjectStatusEnum_Scanning
	ProjectStatusEnum_Monitoring
	ProjectStatusEnum_TopologyMapping
	ProjectStatusEnum_BroadcastSearching
	ProjectStatusEnum_DeviceConfiguring
	ProjectStatusEnum_IntelligentRequestSending
	ProjectStatusEnum_IntelligentUploadSending
	ProjectStatusEnum_IntelligentDownloadSending
	ProjectStatusEnum_Aborted
	ProjectStatusEnum_Finished
)

var ProjectStatusEnumToString = map[ProjectStatusEnum]string{
	ProjectStatusEnum_Idle:                       "Idle",
	ProjectStatusEnum_Computing:                  "Computing",
	ProjectStatusEnum_Comparing:                  "Comparing",
	ProjectStatusEnum_Deploying:                  "Deploying",
	ProjectStatusEnum_Syncing:                    "Syncing",
	ProjectStatusEnum_Scanning:                   "Scanning",
	ProjectStatusEnum_Monitoring:                 "Monitoring",
	ProjectStatusEnum_TopologyMapping:            "TopologyMapping",
	ProjectStatusEnum_BroadcastSearching:         "BroadcastSearching",
	ProjectStatusEnum_DeviceConfiguring:          "DeviceConfiguring",
	ProjectStatusEnum_IntelligentRequestSending:  "IntelligentRequestSending",
	ProjectStatusEnum_IntelligentUploadSending:   "IntelligentUploadSending",
	ProjectStatusEnum_IntelligentDownloadSending: "IntelligentDownloadSending",
	ProjectStatusEnum_Aborted:                    "Aborted",
	ProjectStatusEnum_Finished:                   "Finished",
}

var StringToProjectStatusEnum = map[string]ProjectStatusEnum{
	"Idle":                       ProjectStatusEnum_Idle,
	"Computing":                  ProjectStatusEnum_Computing,
	"Comparing":                  ProjectStatusEnum_Comparing,
	"Deploying":                  ProjectStatusEnum_Deploying,
	"Syncing":                    ProjectStatusEnum_Syncing,
	"Scanning":                   ProjectStatusEnum_Scanning,
	"Monitoring":                 ProjectStatusEnum_Monitoring,
	"TopologyMapping":            ProjectStatusEnum_TopologyMapping,
	"BroadcastSearching":         ProjectStatusEnum_BroadcastSearching,
	"DeviceConfiguring":          ProjectStatusEnum_DeviceConfiguring,
	"IntelligentRequestSending":  ProjectStatusEnum_IntelligentRequestSending,
	"IntelligentUploadSending":   ProjectStatusEnum_IntelligentUploadSending,
	"IntelligentDownloadSending": ProjectStatusEnum_IntelligentDownloadSending,
	"Aborted":                    ProjectStatusEnum_Aborted,
	"Finished":                   ProjectStatusEnum_Finished,
}

func (status ProjectStatusEnum) String() string {
	if s, ok := ProjectStatusEnumToString[status]; ok {
		return s
	}
	return "Unknown"
}

func ParseProjectStatus(s string) (ProjectStatusEnum, statuscode.Response) {
	if status, ok := StringToProjectStatusEnum[s]; ok {
		return status, statuscode.StatusOK(nil)
	}
	return ProjectStatusEnum_Idle, statuscode.StatusBadRequest("Invalid project status: "+s, 0)
}

type ProjectInfo struct {
	Profile           string `json:"Profile"`
	UserId            int64  `json:"UserId"`
	Description       string `json:"Description"`
	Id                int64  `json:"Id"`
	UUID              string `json:"UUID"`
	ProjectMode       string `json:"ProjectMode"`
	ProjectStatus     string `json:"ProjectStatus"`
	QuestionnaireMode string `json:"QuestionnaireMode,omitempty"`
	LastModifiedTime  int    `json:"LastModifiedTime"`
}

func (projectInfo ProjectInfo) String() string {
	jsonBytes, _ := json.MarshalIndent(projectInfo, "", "  ")
	return string(jsonBytes)
}

func (projectInfo *ProjectInfo) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &projectInfo)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

type ProjectConf struct {
	ProjectName string `json:"ProjectName"`
	Profile     string `json:"Profile"`
	UserId      int64  `json:"UserId"`
	Description string `json:"Description"`
}

func (projectConf ProjectConf) String() string {
	jsonBytes, _ := json.MarshalIndent(projectConf, "", "  ")
	return string(jsonBytes)
}

func (projectConf *ProjectConf) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &projectConf)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

func (projectConf *ProjectConf) CheckFeasibility() statuscode.Response {
	if len(projectConf.Description) > 254 {
		return statuscode.StatusUnprocessableEntity(
			statuscode.ErrorParamsData{
				ErrorParams: map[string]any{
					"Description": projectConf.Description,
					"size":        len(projectConf.Description),
				},
			},
			"Description must be less than 254 characters")
	}
	return statuscode.StatusOK(nil)
}

type ActSimpleProjects struct {
	SimpleProjectSet []ProjectInfo `json:"SimpleProjectSet"`
}

func (simple_projects ActSimpleProjects) String() string {
	jsonBytes, _ := json.MarshalIndent(simple_projects, "", "  ")
	return string(jsonBytes)
}

func (simple_projects *ActSimpleProjects) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &simple_projects)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

type Project struct {
	ProjectInfo

	Devices         map[string]*Device `json:"Devices,omitempty"`
	Links           map[string]*Link   `json:"Links,omitempty"`
	ProjectSetting  ProjectSetting     `json:"ProjectSetting"`
	TrafficDesign   TrafficDesign      `json:"TrafficDesign"`
	ComputedResult  ComputedResult     `json:"ComputedResult"`
	TopologySetting TopologySetting    `json:"TopologySetting"`
	// DeviceConfig     DeviceConfig           `json:"DeviceConfig,omitempty"`
	SkuQuantitiesMap map[string]*SkuQuantity `json:"SkuQuantitiesMap,omitempty"`
}

func (project Project) String() string {
	jsonBytes, _ := json.MarshalIndent(project, "", "  ")
	return string(jsonBytes)
}

func (project *Project) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &project)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

type ProjectWithDeviceConfig struct {
	Id           int64        `json:"Id"`
	DeviceConfig DeviceConfig `json:"DeviceConfig,omitempty"`
}

func (project *ProjectWithDeviceConfig) String() string {
	jsonBytes, _ := json.MarshalIndent(project, "", "  ")
	return string(jsonBytes)
}

func (project *ProjectWithDeviceConfig) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &project)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

type ProjectWithDevices struct {
	Id      int64    `json:"Id"`
	Devices []Device `json:"Devices,omitempty"`
}

func (project *ProjectWithDevices) String() string {
	jsonBytes, _ := json.MarshalIndent(project, "", "  ")
	return string(jsonBytes)
}

func (project *ProjectWithDevices) UnmarshalJSONData(data []byte) statuscode.Response {
	err := json.Unmarshal(data, &project)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}
