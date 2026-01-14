package sebastian

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/featuremanager"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/restfulAPI"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wsdispatcher"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wshandler/wsv2"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/deploy"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/monitor"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations/exportconfig"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations/factorydefault"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations/firmwareupgrade"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations/importconfig"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations/locator"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations/reboot"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/scan"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/serviceplatform"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/topologymapping"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/topomanager"
	devicemanager "gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/manager"
	netmgr "gitlab.com/moxa/sw/maf/moxa-app-framework/network/netmgr/api/definition"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/bootstrap"
	mafcache "gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/cache"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/confmap"
	httpconf "gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/httpserver/conf"
	mafipc "gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/ipc/client"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/logging"
)

const (
	sebastianServiceName = "SebastianService"
	moduleName           = "sebastian"
	licenseFileName      = "License.txt"
)

type SebastianService struct {
	root   bootstrap.ServiceRoot
	logger logging.Logger
	nmMgr  netmgr.INetManager
}

func Module(boot *bootstrap.BootManager, appConf []byte) bootstrap.Module {
	module := bootstrap.Module{
		Name: moduleName,
		// EventLogProfile: eventLogList,
		ConfMap: confmap.New(appConf),
		HttpConf: httpconf.ModuleConf{
			Handles: []httpconf.Handle{
				// 加了 WebSocket 欄位無法連線，所以 wss 先走 /api/v2/web/sebastian/*，可以正常連線收封包，等 MAF 修正此問題
				// {
				// 	Mode: httpconf.HandleRoute,
				// 	Route: httpconf.Route{
				// 		Path:                "/api/v2/web/sebastian/ws/*",
				// 		WebSocket:           true,
				// 		ToInternalApiServer: true,
				// 	},
				// },
				{
					Mode: httpconf.HandleRoute,
					Route: httpconf.Route{
						Path:                "/api/v2/web/sebastian/*",
						ToInternalApiServer: true,
					},
				},
			},
		},
		Services: []bootstrap.ServiceMeta{
			{
				Name:           sebastianServiceName,
				ServiceNewFunc: sebastianHttpsServiceNew,
			},
		},
	}

	return module
}

func sebastianHttpsServiceNew(root bootstrap.ServiceRoot) bootstrap.Service {
	// dipool.SetLogger(root.GetLogger())
	// _nmMgr, _ := bootstrap.GetInterface[network.NetManager](root)
	// _dmMgr, _ := bootstrap.GetInterface[devicemgr.DeviceManager](root)
	// scan.Init(_nmMgr, _dmMgr)

	// get networkmgr
	nmMgr, err := bootstrap.GetInterface[netmgr.INetManager](root)
	if err != nil {
		root.GetLogger().Errorln("[Https] Failed to get NM interface")
	}

	return &SebastianService{
		root:   root,
		logger: root.GetLogger(),
		nmMgr:  nmMgr,
	}
}

func (s *SebastianService) registerWebsocketHandlers() {
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStartScanTopology.String(), wsv2.StartScanTopology)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStopScanTopology.String(), wsv2.StopScanTopology)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStartDeploy.String(), wsv2.StartDeploy)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStopDeploy.String(), wsv2.StopDeploy)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStartMonitor.String(), wsv2.StartMonitor)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStopMonitor.String(), wsv2.StopMonitor)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStartTopologyMapping.String(), wsv2.StartTopologyMapping)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStopTopologyMapping.String(), wsv2.StopTopologyMapping)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStartReboot.String(), wsv2.StartReboot)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStopReboot.String(), wsv2.StopReboot)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStartFactoryDefault.String(), wsv2.StartFactoryDefault)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStopFactoryDefault.String(), wsv2.StopFactoryDefault)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStartLocator.String(), wsv2.StartLocator)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStopLocator.String(), wsv2.StopLocator)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStartExportDeviceConfig.String(), wsv2.StartExportConfig)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStopExportDeviceConfig.String(), wsv2.StopExportConfig)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStartImportDeviceConfig.String(), wsv2.StartImportConfig)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStopImportDeviceConfig.String(), wsv2.StopImportConfig)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStartFirmwareUpgrade.String(), wsv2.StartFirmwareUpgrade)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStopFirmwareUpgrade.String(), wsv2.StopFirmwareUpgrade)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStartServicePlatform.String(), wsv2.StartServicePlatform)
	wsdispatcher.RegisterHandleFunc(wscommand.ActWSCommandStopServicePlatform.String(), wsv2.StopServicePlatform)
}

func (s *SebastianService) registerWebsocketRoutes() {
	s.root.Route().GET("/api/v2/web/sebastian/ws/project/:projectId", ws.WebsocketRouteV2)
	s.root.Route().GET("/api/v2/web/sebastian/ws", ws.WebsocketRouteV2)
}

func (s *SebastianService) registerDomainService() {
	// Scanner
	scanner := scan.NewScanner(scan.OnScanTopologyProgressWithWebsocket)
	dipool.Register(func() scan.IScanner {
		return scanner
	})

	// Deployer
	deployer := deploy.NewDeployer(deploy.DeployerFailedResponse, deploy.DeployerProgressResponse, deploy.DeployerDeviceResultProgressResponse, deploy.DeployerCompletedResponse)
	dipool.Register(func() deploy.IDeployer {
		return deployer
	})

	m := monitor.NewMonitor()
	dipool.Register(func() monitor.IMonitor {
		return m
	})

	// TopologyMapper
	topologyMapper := topologymapping.NewTopologyMapper(topologymapping.TopologyMapperFailedResponse, topologymapping.TopologyMapperProgressResponse, topologymapping.TopologyMapperCompletedResponse)
	dipool.Register(func() topologymapping.ITopologyMapper {
		return topologyMapper
	})

	// Operations > Reboot
	opReboot := reboot.NewReboot(operations.OperationFailedResponse, operations.OperationProgressResponse, operations.OperationDeviceResultProgressResponse, operations.OperationCompletedResponse)
	dipool.Register(func() reboot.IReboot {
		return opReboot
	})

	// Operations > FactoryDefault
	opFactoryDefault := factorydefault.NewFactoryDefault(operations.OperationFailedResponse, operations.OperationProgressResponse, operations.OperationDeviceResultProgressResponse, operations.OperationCompletedResponse)
	dipool.Register(func() factorydefault.IFactoryDefault {
		return opFactoryDefault
	})

	// Operations > Locator
	opLocator := locator.NewLocator(operations.OperationFailedResponse, operations.OperationProgressResponse, operations.OperationDeviceResultProgressResponse, operations.OperationCompletedResponse)
	dipool.Register(func() locator.ILocator {
		return opLocator
	})

	// Operations > ExportConfig
	opExportConfig := exportconfig.NewExportConfig(operations.OperationFailedResponse, operations.OperationProgressResponse, operations.OperationDeviceResultProgressResponse, operations.OperationCompletedResponse)
	dipool.Register(func() exportconfig.IExportConfig {
		return opExportConfig
	})

	// Operations > ImportConfig
	opImportConfig := importconfig.NewImportConfig(operations.OperationFailedResponse, operations.OperationProgressResponse, operations.OperationDeviceResultProgressResponse, operations.OperationCompletedResponse)
	dipool.Register(func() importconfig.IImportConfig {
		return opImportConfig
	})

	// Operations > FirmwareUpgrade
	opFirmwareUpgrade := firmwareupgrade.NewFirmwareUpgrade(operations.OperationFailedResponse, operations.OperationProgressResponse, operations.OperationDeviceResultProgressResponse, operations.OperationCompletedResponse)
	dipool.Register(func() firmwareupgrade.IFirmwareUpgrade {
		return opFirmwareUpgrade
	})

	// Service Platform Controller
	servicePlatform := serviceplatform.NewServicePlatform(serviceplatform.ServicePlatformStatusResponse, serviceplatform.ServicePlatformFailedResponse)
	dipool.Register(func() serviceplatform.IServicePlatform {
		return servicePlatform
	})
}

func (s *SebastianService) Start() error {
	activationStatus := featuremanager.GetInstance().GetActivationStatus()
	if activationStatus != featuremanager.ActivationStatusActivated {
		panic(fmt.Errorf("system is not activated. status: %s", activationStatus.String()))
	}

	dmManager, err := bootstrap.GetInterface[devicemanager.DeviceManager](s.root)
	if err != nil {
		panic(err)
	}

	dipool.Register(func() devicemanager.DeviceManager {
		return dmManager
	})

	nmMgr, err := bootstrap.GetInterface[netmgr.INetManager](s.root)
	if err != nil {
		panic(err)
	}

	dipool.Register(func() netmgr.INetManager {
		return nmMgr
	})

	netCache, err := bootstrap.GetInterface[mafcache.NetCache](s.root)
	if err != nil {
		panic(err)
	}
	dipool.Register(func() mafcache.NetCache {
		return netCache
	})

	actDeviceManager := topomanager.NewActDeviceManager()
	dipool.Register(func() topomanager.IActDeviceManager {
		return actDeviceManager
	})

	mafIPCClient := s.root.GetIPCClient()
	dipool.Register(func() mafipc.Client {
		return mafIPCClient
	})

	httputils.InitClient()

	s.logger.Infoln("[Https] Start\n")

	s.registerWebsocketHandlers()
	s.registerWebsocketRoutes()
	s.registerDomainService()

	common.Logger = s.logger

	s.root.Route().POST("/api/v2/web/sebastian/login", restfulAPI.Login)
	s.root.Route().GET("/api/v2/web/sebastian/login/check", restfulAPI.CheckTokenExist)
	s.root.Route().GET("/api/v2/web/sebastian/renew", restfulAPI.RenewToken)
	s.root.Route().POST("/api/v2/web/sebastian/logout", restfulAPI.Logout)

	// s.root.Route().GET("/api/v2/web/sebastian/ws", ws.WebsocketRouteV2)
	// s.root.Route().GET("/api/v2/web/sebastian/ws/projects/:projectId", ws.WebsocketRouteV2)

	s.root.Route().POST("/api/v2/web/sebastian/projects", restfulAPI.CreateProject)                   // FR-7-1
	s.root.Route().GET("/api/v2/web/sebastian/projects", restfulAPI.GetProjectInfos)                  // FR-7-2
	s.root.Route().GET("/api/v2/web/sebastian/projects/:projectId", restfulAPI.GetProject)            // FR-7-2-5
	s.root.Route().PUT("/api/v2/web/sebastian/projects/:projectId", restfulAPI.UpdateProject)         // FR-7-3
	s.root.Route().PATCH("/api/v2/web/sebastian/projects/:projectId", restfulAPI.PatchUpdateProject)  // FR-7-3
	s.root.Route().DELETE("/api/v2/web/sebastian/projects", restfulAPI.DeleteProjects)                // FR-7-4
	s.root.Route().POST("/api/v2/web/sebastian/projects/:projectId/copy", restfulAPI.CopyProject)     // FR-7-5
	s.root.Route().POST("/api/v2/web/sebastian/projects/:projectId/export", restfulAPI.ExportProject) // FR-7-6
	s.root.Route().POST("/api/v2/web/sebastian/projects/import", restfulAPI.ImportProject)            // FR-7-7
	s.root.Route().PUT("/api/v2/web/sebastian/projects/:projectId/status", restfulAPI.UpdateProjectStatus)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/:projectId/undo", restfulAPI.UndoProject)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/:projectId/redo", restfulAPI.RedoProject)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/:projectId/status", restfulAPI.GetProjectStatus)

	s.root.Route().POST("/api/v2/web/sebastian/projects/:projectId/devices", restfulAPI.CreateDevice)
	s.root.Route().PUT("/api/v2/web/sebastian/projects/:projectId/devices/:deviceId", restfulAPI.UpdateDevice)
	s.root.Route().PATCH("/api/v2/web/sebastian/projects/:projectId/devices/:deviceId", restfulAPI.PatchDevice)
	s.root.Route().GET("/api/v2/web/sebastian/projects/:projectId/devices/:deviceId", restfulAPI.GetDevice)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/:projectId/devices", restfulAPI.GetDevices)
	s.root.Route().DELETE("/api/v2/web/sebastian/projects/:projectId/devices", restfulAPI.DeleteDevices)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/{projectId}/device/coordinates", s.postDeviceCoordinates)

	s.root.Route().POST("/api/v2/web/sebastian/projects/:projectId/links", restfulAPI.CreateLink)
	s.root.Route().PUT("/api/v2/web/sebastian/projects/:projectId/links/:linkId", restfulAPI.UpdateLink)
	s.root.Route().PATCH("/api/v2/web/sebastian/projects/:projectId/links/:linkId", restfulAPI.PatchLink)
	s.root.Route().GET("/api/v2/web/sebastian/projects/:projectId/links/:linkId", restfulAPI.GetLink)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/:projectId/links", restfulAPI.GetLinks)
	s.root.Route().DELETE("/api/v2/web/sebastian/projects/:projectId/links", restfulAPI.DeleteLinks)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/links", s.getLinks)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/{projectId}/links", s.postLinks)
	// s.root.Route().PATCH("/api/v2/web/sebastian/projects/{projectId}/links", s.patchLinks)

	s.root.Route().GET("/api/v2/web/sebastian/projects/:projectId/setting", restfulAPI.GetProjectSetting)
	s.root.Route().PUT("/api/v2/web/sebastian/projects/:projectId/setting", restfulAPI.UpdateProjectSetting)
	s.root.Route().PATCH("/api/v2/web/sebastian/projects/:projectId/setting", restfulAPI.PatchProjectSetting)

	s.root.Route().GET("/api/v2/web/sebastian/system", restfulAPI.GetSystem)
	s.root.Route().PUT("/api/v2/web/sebastian/system", restfulAPI.UpdateSystem)

	s.root.Route().GET("/api/v2/web/sebastian/project/:projectId/monitor/sfp_list", restfulAPI.GetSFPList)
	s.root.Route().PUT("/api/v2/web/sebastian/monitor/start", restfulAPI.StartMonitor)
	s.root.Route().PUT("/api/v2/web/sebastian/monitor/stop", restfulAPI.StopMonitor)

	s.root.Route().GET("/api/v2/web/sebastian/feature/activation-status", restfulAPI.GetFeatureActivationStatus)
	s.root.Route().POST("/api/v2/web/sebastian/license/activation-code/import", restfulAPI.ImportLicenseActivationCode)
	s.root.Route().POST("/api/v2/web/sebastian/license/activation-code/upload", restfulAPI.UploadLicenseActivationCode)
	s.root.Route().GET("/api/v2/web/sebastian/license/information", restfulAPI.GetLicenseInformation)
	s.root.Route().GET("/api/v2/web/sebastian/license/user-code", restfulAPI.GetLicenseUserCode)
	// s.root.Route().GET("/api/v2/web/sebastian/license", s.getLicense)

	s.root.Route().POST("/api/v2/web/sebastian/service-platform/auth/device", restfulAPI.RequestServicePlatformDeviceCode)
	s.root.Route().GET("/api/v2/web/sebastian/service-platform/endpoint", restfulAPI.GetServicePlatformEndpoint)
	s.root.Route().GET("/api/v2/web/sebastian/service-platform/contracts", restfulAPI.GetServicePlatformContracts)
	s.root.Route().POST("/api/v2/web/sebastian/service-platform/prices", restfulAPI.GetServicePlatformPrices)
	s.root.Route().POST("/api/v2/web/sebastian/service-platform/register/:projectId", restfulAPI.RegisterServicePlatformDesignHistory)

	s.root.Route().GET("/api/v2/web/sebastian/projects/:projectId/intelligent_vlan", restfulAPI.GetIntelligentVlan)
	s.root.Route().POST("/api/v2/web/sebastian/projects/:projectId/intelligent_vlan", restfulAPI.CreateIntelligentVlan)
	s.root.Route().PUT("/api/v2/web/sebastian/projects/:projectId/intelligent_vlan/:vlanId", restfulAPI.UpdateIntelligentVlan)
	s.root.Route().DELETE("/api/v2/web/sebastian/projects/:projectId/intelligent_vlan/:vlanId", restfulAPI.DeleteIntelligentVlan)
	// s.root.Route().GET("/api/v2/web/sebastian/users", s.getUsers)
	// s.root.Route().GET("/api/v2/web/sebastian/user", s.getUser)
	// s.root.Route().PUT("/api/v2/web/sebastian/user", s.putUser)
	// s.root.Route().POST("/api/v2/web/sebastian/user", s.postUser)
	// s.root.Route().DELETE("/api/v2/web/sebastian/user/{userId}", s.deleteUser)

	// s.root.Route().GET("/api/v2/web/sebastian/simple-device-profiles", s.getSimpleDeviceProfiles)
	// s.root.Route().POST("/api/v2/web/sebastian/device-profile", s.postDeviceProfile)
	// s.root.Route().DELETE("/api/v2/web/sebastian/device-profile/{deviceProfileId}", s.deleteDeviceProfile)

	// Device Icon
	// s.root.Route().POST("/api/v2/web/sebastian/device-icon/{deviceProfileId}/{iconName}", s.postDeviceIcon)
	// s.root.Route().GET("/api/v2/web/sebastian/device-icon/{deviceProfileId}", s.getDeviceIconByProfileId)
	// s.root.Route().GET("/api/v2/web/sebastian/device-icon/name/{iconName}", s.getDeviceIconByName)
	// s.root.Route().GET("/api/v2/web/sebastian/default-device-icon", s.getDefaultDeviceIcon)
	// s.root.Route().GET("/api/v2/web/sebastian/default-icmp-icon", s.getDefaultIcmpIcon)
	// s.root.Route().GET("/api/v2/web/sebastian/default-switch-icon", s.getDefaultSwitchIcon)
	// s.root.Route().GET("/api/v2/web/sebastian/default-end-station-icon", s.getDefaultEndStationIcon)
	// s.root.Route().GET("/api/v2/web/sebastian/default-bridged-end-station-icon", s.getDefaultBridgedEndStationIcon)

	// Device Setting
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/device-informations", s.getDeviceInformations)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/device-informations", s.putDeviceInformations)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/login-policies", s.getLoginPolicies)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/login-policies", s.putLoginPolicies)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/loop-protections", s.getLoopProtections)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/loop-protections", s.putLoopProtections)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/syslog-settings", s.getSyslogSettings)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/syslog-settings", s.putSyslogSettings)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/snmp-trap-settings", s.getSnmpTrapSettings)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/snmp-trap-settings", s.putSnmpTrapSettings)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/device-backups", s.getDeviceBackups)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/device-backups/export", s.getDeviceBackupsExport)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/device-backups/import/device/{deviceId}", s.putDeviceBackupsImport)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/vlan-settings", s.getVlanSettings)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/vlan-settings", s.putVlanSettings)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/time-settings", s.getTimeSettings)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/time-settings", s.putTimeSettings)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/port-settings", s.getPortSettings)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/port-settings", s.putPortSettings)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/ip-settings", s.getIpSettings)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/ip-settings", s.putIpSettings)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/rstp-settings", s.getRstpSettings)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/rstp-settings", s.putRstpSettings)

	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/deploy-device-list", s.getDeployDeviceList)

	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/download-device-config-zip", s.getDownloadDeviceConfigZip)
	// s.root.Route().PUT("/api/v2/web/sebastian/upload-device-config-zip", s.putUploadDeviceConfigZip)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/manufacture-order", s.getManufactureOrder)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/manufacture-order", s.putManufactureOrder)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/clear-manufacture-result", s.putClearManufactureResult)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/stage-manufacture-result", s.getStageManufactureResult)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/re-manufacture-devices", s.putReManufactureDevices)

	// s.root.Route().GET("/api/v2/web/sebastian/power-modules", s.getPowerModules)
	// s.root.Route().GET("/api/v2/web/sebastian/ethernet-modules", s.getEthernetModules)

	// s.root.Route().POST("/api/v2/web/sebastian/firmware", s.postFirmware)

	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/vlan-config/devices", s.getVlanConfigDevices)

	// s.root.Route().GET("/api/v2/web/sebastian/general-profiles", s.getGeneralProfiles)
	// s.root.Route().GET("/api/v2/web/sebastian/foxboro-profiles", s.getFoxboroProfiles)
	// s.root.Route().GET("/api/v2/web/sebastian/self-planning-profiles", s.getSelfPlanningProfiles)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/sku-quantities", s.getSkuQuantities)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/{projectId}/sku-quantity", s.postSkuQuantity)
	// s.root.Route().DELETE("/api/v2/web/sebastian/projects/{projectId}/sku-quantity", s.deleteSkuQuantity)

	// s.root.Route().POST("/api/v2/web/sebastian/projects/{projectId}/intelligent/report", s.postIntelligentReport)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/{projectId}/intelligent/history", s.postIntelligentHistory)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/intelligent/upload-file", s.putIntelligentUploadFile)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/intelligent/download-questionnaire-template/{filename}", s.getIntelligentDownloadQuestionnaireTemplate)

	// s.root.Route().GET("/api/v2/web/sebastian/simple-topologies", s.getSimpleTopologies)
	// s.root.Route().GET("/api/v2/web/sebastian/topology-icon/{topologyId}", s.getTopologyIcon)
	// s.root.Route().POST("/api/v2/web/sebastian/topology/copy/projects/{projectId}/", s.postTopologyCopy)
	// s.root.Route().GET("/api/v2/web/sebastian/topology/projects/{projectId}/loop", s.getTopologyProjectLoop)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/{projectId}/topology-template", s.postTopologyTemplate)

	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/network-baseline-list", s.getNetworkBaselineList)
	// s.root.Route().DELETE("/api/v2/web/sebastian/projects/{projectId}/network-baseline/{networkBaselineId}", s.deleteNetworkBaseline)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/network-baseline/bom-detail/{networkBaselineId}", s.getBaselineBomDetail)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/{projectId}/network-baseline", s.postNetworkBaseline)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/network-baseline/{networkBaselineId}/name", s.putNetworkBaselineName)

	// s.root.Route().GET("/api/v2/web/sebastian/event-logs", s.getEventLogs)
	// s.root.Route().GET("/api/v2/web/sebastian/syslogs", s.getSyslogs)
	// s.root.Route().DELETE("/api/v2/web/sebastian/syslogs", s.deleteSyslogs)
	// s.root.Route().GET("/api/v2/web/sebastian/syslog-configuration", s.getSyslogConfiguration)
	// s.root.Route().PUT("/api/v2/web/sebastian/syslog-configuration", s.putSyslogConfiguration)

	// TODO:重新設計畫面 Swift Setting
	// s.root.Route().GET("/api/v2/web/sebastian/{projectId}/redundant-group/swift", s.getRedundantGroupSwift)
	// s.root.Route().PUT("/api/v2/web/sebastian/{projectId}/redundant-group/swift", s.putRedundantGroupSwift)
	// s.root.Route().DELETE("/api/v2/web/sebastian/{projectId}/redundant-group/swift", s.deleteRedundantGroupSwift)
	// s.root.Route().GET("/api/v2/web/sebastian/{projectId}/redundant-group/swift/candidate", s.getRedundantGroupSwiftCandidate)
	// s.root.Route().PUT("/api/v2/web/sebastian/{projectId}/redundant-group/swift/compute", s.putRedundantGroupSwiftCompute)

	// TODO:重新設計畫面 Traffic Design
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/traffic-design/mode", s.getTrafficDesignMode)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/traffic-design/mode", s.putTrafficDesignMode)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/traffic-design/time-slot-setting", s.getTrafficDesignTimeSlotSetting)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/traffic-design/time-slot-setting", s.putTrafficDesignTimeSlotSetting)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/{projectId}/traffic-design/time-slot-setting", s.postTrafficDesignTimeSlotSetting)
	// s.root.Route().DELETE("/api/v2/web/sebastian/projects/{projectId}/traffic-design/time-slot-setting/{trafficTimeSlotId}", s.deleteTrafficDesignTimeSlotSetting)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/traffic-design/per-stream-priority-setting", s.getTrafficDesignPerStreamPrioritySetting)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/traffic-design/per-stream-priority-setting", s.putTrafficDesignPerStreamPrioritySetting)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/{projectId}/traffic-design/per-stream-priority-setting", s.postTrafficDesignPerStreamPrioritySetting)
	// s.root.Route().DELETE("/api/v2/web/sebastian/projects/{projectId}/traffic-design/per-stream-priority-setting/{trafficPerStreamPriorityId}", s.deleteTrafficDesignPerStreamPrioritySetting)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/traffic-design/traffic-type-configuration-setting", s.getTrafficDesignTrafficTypeConfigurationSetting)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/traffic-design/traffic-type-configuration-setting", s.putTrafficDesignTrafficTypeConfigurationSetting)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/traffic-design/application-setting", s.getTrafficDesignApplicationSetting)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/traffic-design/application-setting", s.putTrafficDesignApplicationSetting)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/{projectId}/traffic-design/application-setting", s.postTrafficDesignApplicationSetting)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/{projectId}/traffic-design/application-setting/copy/{applicationId}", s.postTrafficDesignApplicationSettingCopy)
	// s.root.Route().DELETE("/api/v2/web/sebastian/projects/{projectId}/traffic-design/application-setting/{trafficApplicationId}", s.deleteTrafficDesignApplicationSetting)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/traffic-design/stream-setting", s.getTrafficDesignStreamSetting)
	// s.root.Route().PUT("/api/v2/web/sebastian/projects/{projectId}/traffic-design/stream-setting", s.putTrafficDesignStreamSetting)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/{projectId}/traffic-design/stream-setting", s.postTrafficDesignStreamSetting)
	// s.root.Route().POST("/api/v2/web/sebastian/projects/{projectId}/traffic-design/stream-setting/copy/{streamId}", s.postTrafficDesignStreamSettingCopy)
	// s.root.Route().DELETE("/api/v2/web/sebastian/projects/{projectId}/traffic-design/stream-setting/{trafficStreamId}", s.deleteTrafficDesignStreamSetting)

	// TODO:重新設計畫面 Compute Result
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/compute/result/stream", s.getComputeResultStream)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/compute/result/gcl", s.getComputeResultGcl)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/compute/result/device", s.getComputeResultDevice)
	// s.root.Route().DELETE("/api/v2/web/sebastian/projects/{projectId}/compute/result", s.deleteComputeResult)

	// TODO:重新設計畫面 VLAN View
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/vlan-view/vlans", s.getVlanViewVlans)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/vlan-view/vlan/{vlanId}", s.getVlanViewVlan)
	// s.root.Route().GET("/api/v2/web/sebastian/projects/{projectId}/vlan-view/vlan-ids", s.getVlanViewVlanIds)
	return nil
}

func (s *SebastianService) Stop() error {
	s.logger.Infoln("[Https] Stop\n")
	return nil
}

func (s *SebastianService) Export() interface{} {
	s.logger.Infoln("[Https] Export\n")
	return nil
}
