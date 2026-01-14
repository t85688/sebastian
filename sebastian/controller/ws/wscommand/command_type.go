package wscommand

type ActWSCommandType int64

const (
	ActWSCommandUnknown                               ActWSCommandType = 0x0000
	ActWSCommandTestStart                             ActWSCommandType = 0xFFFE
	ActWSCommandTestStop                              ActWSCommandType = 0xFFFF
	ActWSCommandStartCompute                          ActWSCommandType = 0x0001
	ActWSCommandStopCompute                           ActWSCommandType = 0x0002
	ActWSCommandStartCompare                          ActWSCommandType = 0x0101
	ActWSCommandStopCompare                           ActWSCommandType = 0x0102
	ActWSCommandStartDeploy                           ActWSCommandType = 0x0201
	ActWSCommandStopDeploy                            ActWSCommandType = 0x0202
	ActWSCommandStartManufactureDeploy                ActWSCommandType = 0x0203
	ActWSCommandStopManufactureDeploy                 ActWSCommandType = 0x0204
	ActWSCommandStartScanTopology                     ActWSCommandType = 0x0301
	ActWSCommandStopScanTopology                      ActWSCommandType = 0x0302
	ActWSCommandStartSyncDevices                      ActWSCommandType = 0x0303
	ActWSCommandStopSyncDevices                       ActWSCommandType = 0x0304
	ActWSCommandStartDeviceDiscovery                  ActWSCommandType = 0x0401
	ActWSCommandStartRetryConnect                     ActWSCommandType = 0x0402
	ActWSCommandStartLinkSequenceDetect               ActWSCommandType = 0x0403
	ActWSCommandStartSetNetworkSetting                ActWSCommandType = 0x0404
	ActWSCommandStopDeviceDiscovery                   ActWSCommandType = 0x0405
	ActWSCommandStartReboot                           ActWSCommandType = 0x0406
	ActWSCommandStartFactoryDefault                   ActWSCommandType = 0x0407
	ActWSCommandStartFirmwareUpgrade                  ActWSCommandType = 0x0408
	ActWSCommandStartEnableSnmp                       ActWSCommandType = 0x0409
	ActWSCommandStartLocator                          ActWSCommandType = 0x040A
	ActWSCommandStartGetEventLog                      ActWSCommandType = 0x040B
	ActWSCommandStartServicePlatform                  ActWSCommandType = 0x040E
	ActWSCommandStopServicePlatform                   ActWSCommandType = 0x040F
	ActWSCommandStartDeviceConfig                     ActWSCommandType = 0x0410
	ActWSCommandStopDeviceConfig                      ActWSCommandType = 0x0411
	ActWSCommandStartDeviceCommandLine                ActWSCommandType = 0x0412
	ActWSCommandStopDeviceCommandLine                 ActWSCommandType = 0x0413
	ActWSCommandStopReboot                            ActWSCommandType = 0x041A
	ActWSCommandStopFactoryDefault                    ActWSCommandType = 0x041B
	ActWSCommandStopFirmwareUpgrade                   ActWSCommandType = 0x041C
	ActWSCommandStopLocator                           ActWSCommandType = 0x041E
	ActWSCommandStartProbeDeviceProfile               ActWSCommandType = 0x0501
	ActWSCommandStopProbeDeviceProfile                ActWSCommandType = 0x0502
	ActWSCommandStartMonitor                          ActWSCommandType = 0x0601
	ActWSCommandStopMonitor                           ActWSCommandType = 0x0602
	ActWSCommandMonitorAliveUpdate                    ActWSCommandType = 0x0603
	ActWSCommandMonitorStatusUpdate                   ActWSCommandType = 0x0604
	ActWSCommandMonitorTrafficUpdate                  ActWSCommandType = 0x0605
	ActWSCommandMonitorTimeStatusUpdate               ActWSCommandType = 0x0606
	ActWSCommandMonitorEndpointUpdate                 ActWSCommandType = 0x0607
	ActWSCommandMonitorSwiftStatusUpdate              ActWSCommandType = 0x0608
	ActWSCommandStartTopologyMapping                  ActWSCommandType = 0x0701
	ActWSCommandStopTopologyMapping                   ActWSCommandType = 0x0702
	ActWSCommandStartScanMapping                      ActWSCommandType = 0x0703
	ActWSCommandStopScanMapping                       ActWSCommandType = 0x0704
	ActWSCommandStartIntelligentRequest               ActWSCommandType = 0x0801
	ActWSCommandStopIntelligentRequest                ActWSCommandType = 0x0802
	ActWSCommandStartIntelligentQuestionnaireDownload ActWSCommandType = 0x0803
	ActWSCommandStopIntelligentQuestionnaireDownload  ActWSCommandType = 0x0804
	ActWSCommandStartIntelligentQuestionnaireUpload   ActWSCommandType = 0x0805
	ActWSCommandStopIntelligentQuestionnaireUpload    ActWSCommandType = 0x0806
	ActWSCommandStartExportDeviceConfig               ActWSCommandType = 0x0901
	ActWSCommandStopExportDeviceConfig                ActWSCommandType = 0x0902
	ActWSCommandStartImportDeviceConfig               ActWSCommandType = 0x0903
	ActWSCommandStopImportDeviceConfig                ActWSCommandType = 0x0904
	ActWSCommandPatchUpdate                           ActWSCommandType = 0x1001
	ActWSCommandFeaturesAvailableStatus               ActWSCommandType = 0x1002
	ActWSCommandGetProjectDataVersion                 ActWSCommandType = 0x8001
)

var ActWSCommandMap = map[ActWSCommandType]string{
	ActWSCommandUnknown:                               "Unknown",
	ActWSCommandTestStart:                             "TestStart",
	ActWSCommandTestStop:                              "TestStop",
	ActWSCommandStartCompute:                          "StartCompute",
	ActWSCommandStopCompute:                           "StopCompute",
	ActWSCommandStartCompare:                          "StartCompare",
	ActWSCommandStopCompare:                           "StopCompare",
	ActWSCommandStartDeploy:                           "StartDeploy",
	ActWSCommandStopDeploy:                            "StopDeploy",
	ActWSCommandStartManufactureDeploy:                "StartManufactureDeploy",
	ActWSCommandStopManufactureDeploy:                 "StopManufactureDeploy",
	ActWSCommandStartScanTopology:                     "StartScanTopology",
	ActWSCommandStopScanTopology:                      "StopScanTopology",
	ActWSCommandStartSyncDevices:                      "StartSyncDevices",
	ActWSCommandStopSyncDevices:                       "StopSyncDevices",
	ActWSCommandStartDeviceDiscovery:                  "StartDeviceDiscovery",
	ActWSCommandStartRetryConnect:                     "StartRetryConnect",
	ActWSCommandStartLinkSequenceDetect:               "StartLinkSequenceDetect",
	ActWSCommandStartSetNetworkSetting:                "StartSetNetworkSetting",
	ActWSCommandStopDeviceDiscovery:                   "StopDeviceDiscovery",
	ActWSCommandStartReboot:                           "StartReboot",
	ActWSCommandStartFactoryDefault:                   "StartFactoryDefault",
	ActWSCommandStartFirmwareUpgrade:                  "StartFirmwareUpgrade",
	ActWSCommandStartEnableSnmp:                       "StartEnableSnmp",
	ActWSCommandStartLocator:                          "StartLocator",
	ActWSCommandStartGetEventLog:                      "StartGetEventLog",
	ActWSCommandStartServicePlatform:                  "StartServicePlatform",
	ActWSCommandStopServicePlatform:                   "StopServicePlatform",
	ActWSCommandStartDeviceConfig:                     "StartDeviceConfig",
	ActWSCommandStopDeviceConfig:                      "StopDeviceConfig",
	ActWSCommandStartDeviceCommandLine:                "StartDeviceCommandLine",
	ActWSCommandStopDeviceCommandLine:                 "StopDeviceCommandLine",
	ActWSCommandStopReboot:                            "StopReboot",
	ActWSCommandStopFactoryDefault:                    "StopFactoryDefault",
	ActWSCommandStopFirmwareUpgrade:                   "StopFirmwareUpgrade",
	ActWSCommandStopLocator:                           "StopLocator",
	ActWSCommandStartProbeDeviceProfile:               "StartProbeDeviceProfile",
	ActWSCommandStopProbeDeviceProfile:                "StopProbeDeviceProfile",
	ActWSCommandStartMonitor:                          "StartMonitor",
	ActWSCommandStopMonitor:                           "StopMonitor",
	ActWSCommandMonitorAliveUpdate:                    "MonitorAliveUpdate",
	ActWSCommandMonitorStatusUpdate:                   "MonitorStatusUpdate",
	ActWSCommandMonitorTrafficUpdate:                  "MonitorTrafficUpdate",
	ActWSCommandMonitorTimeStatusUpdate:               "MonitorTimeStatusUpdate",
	ActWSCommandMonitorEndpointUpdate:                 "MonitorEndpointUpdate",
	ActWSCommandMonitorSwiftStatusUpdate:              "MonitorSwiftStatusUpdate",
	ActWSCommandStartTopologyMapping:                  "StartTopologyMapping",
	ActWSCommandStopTopologyMapping:                   "StopTopologyMapping",
	ActWSCommandStartScanMapping:                      "StartScanMapping",
	ActWSCommandStopScanMapping:                       "StopScanMapping",
	ActWSCommandStartIntelligentRequest:               "StartIntelligentRequest",
	ActWSCommandStopIntelligentRequest:                "StopIntelligentRequest",
	ActWSCommandStartIntelligentQuestionnaireDownload: "StartIntelligentQuestionnaireDownload",
	ActWSCommandStopIntelligentQuestionnaireDownload:  "StopIntelligentQuestionnaireDownload",
	ActWSCommandStartIntelligentQuestionnaireUpload:   "StartIntelligentQuestionnaireUpload",
	ActWSCommandStopIntelligentQuestionnaireUpload:    "StopIntelligentQuestionnaireUpload",
	ActWSCommandStartExportDeviceConfig:               "StartExportDeviceConfig",
	ActWSCommandStopExportDeviceConfig:                "StopExportDeviceConfig",
	ActWSCommandStartImportDeviceConfig:               "StartImportDeviceConfig",
	ActWSCommandStopImportDeviceConfig:                "StopImportDeviceConfig",
	ActWSCommandPatchUpdate:                           "PatchUpdate",
	ActWSCommandFeaturesAvailableStatus:               "FeaturesAvailableStatus",
	ActWSCommandGetProjectDataVersion:                 "GetProjectDataVersion",
}

func ParseActWSCommand(opCode int64) ActWSCommandType {
	if _, ok := ActWSCommandMap[ActWSCommandType(opCode)]; ok {
		return ActWSCommandType(opCode)
	}
	return ActWSCommandUnknown
}

func (cmd ActWSCommandType) Int64() int64 {
	return int64(cmd)
}

func (cmd ActWSCommandType) String() string {
	if name, ok := ActWSCommandMap[cmd]; ok {
		return name
	}
	return ActWSCommandMap[ActWSCommandUnknown]
}
