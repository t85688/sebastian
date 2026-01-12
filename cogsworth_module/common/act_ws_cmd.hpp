/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_auto_probe_config.hpp"
#include "act_broadcast_search_config.hpp"
#include "act_deploy_parameter.hpp"
#include "act_device_config_type.hpp"
#include "act_device_ip_config.hpp"
#include "act_json.hpp"
#include "act_scan_ip_range.hpp"

#define ACT_SYSTEM_WS_PROJECT_NAME "SYSTEM"  // For system websocket control
#define ACT_SYSTEM_WS_PROJECT_ID (40404)     // For system websocket control

/**
 * @brief The project status enum class
 *
 */
enum class ActWSCommandEnum {
  kTestStart = 0xFFFE,
  kTestStop = 0xFFFF,
  kStartCompute = 0x0001,
  kStopCompute = 0x0002,
  kStartCompare = 0x0101,
  kStopCompare = 0x0102,
  kStartDeploy = 0x0201,
  kStopDeploy = 0x0202,
  kStartManufactureDeploy = 0x0203,
  kStopManufactureDeploy = 0x0204,
  kStartScanTopology = 0x0301,
  kStopScanTopology = 0x0302,
  kStartSyncDevices = 0x0303,
  kStopSyncDevices = 0x0304,
  kStartDeviceDiscovery = 0x0401,
  kStartRetryConnect = 0x0402,
  kStartLinkSequenceDetect = 0x0403,
  kStartSetNetworkSetting = 0x0404,
  kStopDeviceDiscovery = 0x0405,
  kStartReboot = 0x0406,
  kStartFactoryDefault = 0x0407,
  kStartFirmwareUpgrade = 0x0408,
  kStartEnableSnmp = 0x0409,
  kStartLocator = 0x040A,
  kStartGetEventLog = 0x040B,
  kStartDeviceConfig = 0x0410,
  kStopDeviceConfig = 0x0411,
  kStartDeviceCommandLine = 0x0412,
  kStopDeviceCommandLine = 0x0413,
  kStartProbeDeviceProfile = 0x0501,
  kStopProbeDeviceProfile = 0x0502,
  kStartMonitor = 0x0601,
  kStopMonitor = 0x0602,
  kMonitorAliveUpdate = 0x0603,
  kMonitorStatusUpdate = 0x0604,
  kMonitorTrafficUpdate = 0x0605,
  kMonitorTimeStatusUpdate = 0x0606,
  kMonitorEndpointUpdate = 0x0607,
  kMonitorSwiftStatusUpdate = 0x0608,
  kStartTopologyMapping = 0x0701,
  kStopTopologyMapping = 0x0702,
  kStartScanMapping = 0x0703,
  kStopScanMapping = 0x0704,
  kStartIntelligentRequest = 0x0801,
  kStopIntelligentRequest = 0x0802,
  kStartIntelligentQuestionnaireDownload = 0x0803,
  kStopIntelligentQuestionnaireDownload = 0x0804,
  kStartIntelligentQuestionnaireUpload = 0x0805,
  kStopIntelligentQuestionnaireUpload = 0x0806,
  kStartExportDeviceConfig = 0x0901,
  kStopExportDeviceConfig = 0x0902,
  kStartImportDeviceConfig = 0x0903,
  kStopImportDeviceConfig = 0x0904,
  kPatchUpdate = 0x1001,
  kFeaturesAvailableStatus = 0x1002,
  kGetProjectDataVersion = 0x8001
};

/**
 * @brief The QMap for project status enum mapping
 *
 */
static const QMap<QString, ActWSCommandEnum> kActWSCommandEnumMap = {
    {"TestStart", ActWSCommandEnum::kTestStart},
    {"TestStop", ActWSCommandEnum::kTestStop},
    {"StartCompute", ActWSCommandEnum::kStartCompute},
    {"StopCompute", ActWSCommandEnum::kStopCompute},
    {"StartCompare", ActWSCommandEnum::kStartCompare},
    {"StopCompare", ActWSCommandEnum::kStopCompare},
    {"StartDeploy", ActWSCommandEnum::kStartDeploy},
    {"StopDeploy", ActWSCommandEnum::kStopDeploy},
    {"StartManufactureDeploy", ActWSCommandEnum::kStartManufactureDeploy},
    {"StopManufactureDeploy", ActWSCommandEnum::kStopManufactureDeploy},
    {"StartScanTopology", ActWSCommandEnum::kStartScanTopology},
    {"StopScanTopology", ActWSCommandEnum::kStopScanTopology},
    {"StartSyncDevices", ActWSCommandEnum::kStartSyncDevices},
    {"StopSyncDevices", ActWSCommandEnum::kStopSyncDevices},
    {"StartDeviceDiscovery", ActWSCommandEnum::kStartDeviceDiscovery},
    {"StartRetryConnect", ActWSCommandEnum::kStartRetryConnect},
    {"StartLinkSequenceDetect", ActWSCommandEnum::kStartLinkSequenceDetect},
    {"StartSetNetworkSetting", ActWSCommandEnum::kStartSetNetworkSetting},
    {"StopDeviceDiscovery", ActWSCommandEnum::kStopDeviceDiscovery},
    {"StartReboot", ActWSCommandEnum::kStartReboot},
    {"StartFactoryDefault", ActWSCommandEnum::kStartFactoryDefault},
    {"StartFirmwareUpgrade", ActWSCommandEnum::kStartFirmwareUpgrade},
    {"StartEnableSnmp", ActWSCommandEnum::kStartEnableSnmp},
    {"StartLocator", ActWSCommandEnum::kStartLocator},
    {"StartGetEventLog", ActWSCommandEnum::kStartGetEventLog},
    {"StartDeviceConfig", ActWSCommandEnum::kStartDeviceConfig},
    {"StopDeviceConfig", ActWSCommandEnum::kStopDeviceConfig},
    {"StartDeviceCommandLine", ActWSCommandEnum::kStartDeviceCommandLine},
    {"StopDeviceCommandLine", ActWSCommandEnum::kStopDeviceCommandLine},
    {"StartProbeDeviceProfile", ActWSCommandEnum::kStartProbeDeviceProfile},
    {"StopProbeDeviceProfile", ActWSCommandEnum::kStopProbeDeviceProfile},
    {"StartMonitor", ActWSCommandEnum::kStartMonitor},
    {"StopMonitor", ActWSCommandEnum::kStopMonitor},
    {"MonitorAliveUpdate", ActWSCommandEnum::kMonitorAliveUpdate},
    {"MonitorStatusUpdate", ActWSCommandEnum::kMonitorStatusUpdate},
    {"MonitorTrafficUpdate", ActWSCommandEnum::kMonitorTrafficUpdate},
    {"MonitorTimeStatusUpdate", ActWSCommandEnum::kMonitorTimeStatusUpdate},
    {"MonitorEndpointUpdate", ActWSCommandEnum::kMonitorEndpointUpdate},
    {"MonitorSwiftStatusUpdate", ActWSCommandEnum::kMonitorSwiftStatusUpdate},
    {"StartTopologyMapping", ActWSCommandEnum::kStartTopologyMapping},
    {"StopTopologyMapping", ActWSCommandEnum::kStopTopologyMapping},
    {"StartScanMapping", ActWSCommandEnum::kStartScanMapping},
    {"StopScanMapping", ActWSCommandEnum::kStopScanMapping},
    {"StartIntelligentRequest", ActWSCommandEnum::kStartIntelligentRequest},
    {"StopIntelligentRequest", ActWSCommandEnum::kStopIntelligentRequest},
    {"StartIntelligentQuestionnaireDownload", ActWSCommandEnum::kStartIntelligentQuestionnaireDownload},
    {"StopIntelligentQuestionnaireDownload", ActWSCommandEnum::kStopIntelligentQuestionnaireDownload},
    {"StartIntelligentQuestionnaireUpload", ActWSCommandEnum::kStartIntelligentQuestionnaireUpload},
    {"StopIntelligentQuestionnaireUpload", ActWSCommandEnum::kStopIntelligentQuestionnaireUpload},
    {"StartExportDeviceConfig", ActWSCommandEnum::kStartExportDeviceConfig},
    {"StopExportDeviceConfig", ActWSCommandEnum::kStopExportDeviceConfig},
    {"StartImportDeviceConfig", ActWSCommandEnum::kStartImportDeviceConfig},
    {"StopImportDeviceConfig", ActWSCommandEnum::kStopImportDeviceConfig},
    {"PatchUpdate", ActWSCommandEnum::kPatchUpdate},
    {"FeaturesAvailableStatus", ActWSCommandEnum::kFeaturesAvailableStatus},
    {"GetProjectDataVersion", ActWSCommandEnum::kGetProjectDataVersion}};

/**
 * @brief The deploy action enum class
 *
 */
enum class ActDeployActionEnum {
  kAll = 0,
  kReboot = 1,
  kFactoryDefault = 2,
  kFirmwareUpgrade = 3,
  kNetworkSetting = 4,
  kVLAN = 5
};

/**
 * @brief The QMap for deploy action enum mapping
 *
 */
static const QMap<QString, ActDeployActionEnum> kActDeployActionEnumMap = {
    {"All", ActDeployActionEnum::kAll},
    {"Reboot", ActDeployActionEnum::kReboot},
    {"FactoryDefault", ActDeployActionEnum::kFactoryDefault},
    {"FirmwareUpgrade", ActDeployActionEnum::kFirmwareUpgrade},
    {"NetworkSetting", ActDeployActionEnum::kNetworkSetting},
    {"VLAN", ActDeployActionEnum::kVLAN}};

/**
 * @brief The ACT websocket command class
 *
 */
class ActWSCommand : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code

  ACT_JSON_ENUM(ActWSCommandEnum, opcode_enum, OpCodeEnum);  ///< The opcode enum in the websocket
  ACT_JSON_FIELD(qint64, opcode, OpCode);                    ///< The opcode in the websocket
  ACT_JSON_FIELD(qint64, project_id, ProjectId);             ///< The id of the specified project

 public:
  ActWSCommand() {
    this->opcode_ = 0xffff;
    this->opcode_enum_ = ActWSCommandEnum::kTestStop;
    this->project_id_ = -1;
  }

  // void FromString(QString str) {
  //   QByteArray ba = str.toUtf8();
  //   this->fromJson(ba);
  //   opcode_enum_ = static_cast<ActWSCommandEnum>(opcode_);
  // }

  ACT_STATUS TransformOpCodeEnum() {
    ACT_STATUS_INIT();
    switch (this->GetOpCode()) {
      case 0xFFFE:
        this->opcode_enum_ = ActWSCommandEnum::kTestStart;
        break;
      case 0xFFFF:
        this->opcode_enum_ = ActWSCommandEnum::kTestStop;
        break;
      case 0x1001:
        this->opcode_enum_ = ActWSCommandEnum::kPatchUpdate;
        break;
      case 0x0001:
        this->opcode_enum_ = ActWSCommandEnum::kStartCompute;
        break;
      case 0x0002:
        this->opcode_enum_ = ActWSCommandEnum::kStopCompute;
        break;
      case 0x0101:
        this->opcode_enum_ = ActWSCommandEnum::kStartCompare;
        break;
      case 0x0102:
        this->opcode_enum_ = ActWSCommandEnum::kStopCompare;
        break;
      case 0x0201:
        this->opcode_enum_ = ActWSCommandEnum::kStartDeploy;
        break;
      case 0x0202:
        this->opcode_enum_ = ActWSCommandEnum::kStopDeploy;
        break;
      case 0x0203:
        this->opcode_enum_ = ActWSCommandEnum::kStartManufactureDeploy;
        break;
      case 0x0204:
        this->opcode_enum_ = ActWSCommandEnum::kStopManufactureDeploy;
        break;
      case 0x0301:
        this->opcode_enum_ = ActWSCommandEnum::kStartScanTopology;
        break;
      case 0x0302:
        this->opcode_enum_ = ActWSCommandEnum::kStopScanTopology;
        break;
      case 0x0303:
        this->opcode_enum_ = ActWSCommandEnum::kStartSyncDevices;
        break;
      case 0x0304:
        this->opcode_enum_ = ActWSCommandEnum::kStopSyncDevices;
        break;
      case 0x0401:
        this->opcode_enum_ = ActWSCommandEnum::kStartDeviceDiscovery;
        break;
      case 0x0402:
        this->opcode_enum_ = ActWSCommandEnum::kStartRetryConnect;
        break;
      case 0x0403:
        this->opcode_enum_ = ActWSCommandEnum::kStartLinkSequenceDetect;
        break;
      case 0x0404:
        this->opcode_enum_ = ActWSCommandEnum::kStartSetNetworkSetting;
        break;
      case 0x0405:
        this->opcode_enum_ = ActWSCommandEnum::kStopDeviceDiscovery;
        break;
      case 0x0406:
        this->opcode_enum_ = ActWSCommandEnum::kStartReboot;
        break;
      case 0x0407:
        this->opcode_enum_ = ActWSCommandEnum::kStartFactoryDefault;
        break;
      case 0x0408:
        this->opcode_enum_ = ActWSCommandEnum::kStartFirmwareUpgrade;
        break;
      case 0x0409:
        this->opcode_enum_ = ActWSCommandEnum::kStartEnableSnmp;
        break;
      case 0x040A:
        this->opcode_enum_ = ActWSCommandEnum::kStartLocator;
        break;
      case 0x040B:
        this->opcode_enum_ = ActWSCommandEnum::kStartGetEventLog;
        break;
      case 0x0410:
        this->opcode_enum_ = ActWSCommandEnum::kStartDeviceConfig;
        break;
      case 0x0411:
        this->opcode_enum_ = ActWSCommandEnum::kStopDeviceConfig;
        break;
      case 0x0412:
        this->opcode_enum_ = ActWSCommandEnum::kStartDeviceCommandLine;
        break;
      case 0x0413:
        this->opcode_enum_ = ActWSCommandEnum::kStopDeviceCommandLine;
        break;
      case 0x0501:
        this->opcode_enum_ = ActWSCommandEnum::kStartProbeDeviceProfile;
        break;
      case 0x0502:
        this->opcode_enum_ = ActWSCommandEnum::kStopProbeDeviceProfile;
        break;
      case 0x0601:
        this->opcode_enum_ = ActWSCommandEnum::kStartMonitor;
        break;
      case 0x0602:
        this->opcode_enum_ = ActWSCommandEnum::kStopMonitor;
        break;
      case 0x0603:
        this->opcode_enum_ = ActWSCommandEnum::kMonitorAliveUpdate;
        break;
      case 0x0604:
        this->opcode_enum_ = ActWSCommandEnum::kMonitorStatusUpdate;
        break;
      case 0x0605:
        this->opcode_enum_ = ActWSCommandEnum::kMonitorTrafficUpdate;
        break;
      case 0x0606:
        this->opcode_enum_ = ActWSCommandEnum::kMonitorTimeStatusUpdate;
        break;
      case 0x0701:
        this->opcode_enum_ = ActWSCommandEnum::kStartTopologyMapping;
        break;
      case 0x0702:
        this->opcode_enum_ = ActWSCommandEnum::kStopTopologyMapping;
        break;
      case 0x0703:
        this->opcode_enum_ = ActWSCommandEnum::kStartScanMapping;
        break;
      case 0x0704:
        this->opcode_enum_ = ActWSCommandEnum::kStopScanMapping;
        break;
      case 0x0801:
        this->opcode_enum_ = ActWSCommandEnum::kStartIntelligentRequest;
        break;
      case 0x0802:
        this->opcode_enum_ = ActWSCommandEnum::kStopIntelligentRequest;
        break;
      case 0x0803:
        this->opcode_enum_ = ActWSCommandEnum::kStartIntelligentQuestionnaireDownload;
        break;
      case 0x0804:
        this->opcode_enum_ = ActWSCommandEnum::kStopIntelligentQuestionnaireDownload;
        break;
      case 0x0805:
        this->opcode_enum_ = ActWSCommandEnum::kStartIntelligentQuestionnaireUpload;
        break;
      case 0x0806:
        this->opcode_enum_ = ActWSCommandEnum::kStopIntelligentQuestionnaireUpload;
        break;
      case 0x0901:
        this->opcode_enum_ = ActWSCommandEnum::kStartExportDeviceConfig;
        break;
      case 0x0902:
        this->opcode_enum_ = ActWSCommandEnum::kStopExportDeviceConfig;
        break;
      case 0x0903:
        this->opcode_enum_ = ActWSCommandEnum::kStartImportDeviceConfig;
        break;
      case 0x0904:
        this->opcode_enum_ = ActWSCommandEnum::kStopImportDeviceConfig;
        break;
      case 0x8001:
        this->opcode_enum_ = ActWSCommandEnum::kGetProjectDataVersion;
        break;
      default:
        qCritical() << "get an unknown ws opcode:" << this->GetOpCode();
        return std::make_shared<ActStatusInternalError>("WS");
    }
    return act_status;
  }
};

/**
 * @brief The ACT ScanTopology websocket command class
 *
 */
class ActScanTopologyWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(bool, new_topology, NewTopology);

 public:
  /**
   * @brief Construct a new ActScanTopologyWSCommand object
   *
   */
  ActScanTopologyWSCommand() { new_topology_ = true; }
};

/**
 * @brief The ACT TopologyMapping websocket command class
 *
 */
class ActIntelligentRequestWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_OBJECT(ActIntelligentRecognizeRequest, intelligent_request, IntelligentRequest);

 public:
  /**
   * @brief Construct a new ActIntelligentRequestWSCommand object
   *
   */
  ActIntelligentRequestWSCommand() {}
};

/**
 * @brief The ACT Intelligent Upload websocket command class
 *
 */
class ActIntelligentQuestionnaireUploadWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_OBJECT(ActIntelligentQuestionnaireUpload, intelligent_upload, IntelligentUpload);

 public:
  /**
   * @brief Construct a new ActIntelligentQuestionnaireUploadWSCommand object
   *
   */
  ActIntelligentQuestionnaireUploadWSCommand() {}
};

class ActIntelligentQuestionnaireDownloadWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_OBJECT(ActIntelligentQuestionnaireDownload, intelligent_download, IntelligentDownload);

 public:
  /**
   * @brief Construct a new ActIntelligentQuestionnaireDownloadWSCommand object
   *
   */
  ActIntelligentQuestionnaireDownloadWSCommand() {}
};

/**
 * @brief The ACT DeviceDiscovery websocket command class
 *
 */
class ActDeviceDiscoveryWSCommand : public ActWSCommand, public ActDeviceDiscoveryConfig {
  Q_GADGET
  QS_SERIALIZABLE
};

/**
 * @brief The ACT RetryConnect websocket command class
 *
 */
class ActRetryConnectWSCommand : public ActWSCommand, public ActRetryConnectConfig {
  Q_GADGET
  QS_SERIALIZABLE
};

/**
 * @brief The ACT SetNetworkSetting websocket command class
 *
 */
class ActSetNetworkSettingWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, from_broadcast_search, FromBroadcastSearch);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceIpConfiguration, pairs, Pairs);  ///< The DeviceIpConfiguration id list

 public:
  /**
   * @brief Construct a new ActSetNetworkSettingWSCommand object
   *
   */
  ActSetNetworkSettingWSCommand() { from_broadcast_search_ = true; }
};

class ActLinkSequenceDetectWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, from_broadcast_search, FromBroadcastSearch);
  ACT_JSON_COLLECTION(QList, qint64, id, Id);  ///< The device id list

 public:
  /**
   * @brief Construct a new ActLinkSequenceDetectWSCommand object
   *
   */
  ActLinkSequenceDetectWSCommand() { from_broadcast_search_ = true; }
};

class ActDeviceConfigWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint64, id, Id);
  // ACT_JSON_ENUM(ActDeviceConfigTypeEnum, type_enum, TypeEnum);
  ACT_JSON_FIELD(qint64, type, Type);

 public:
  /**
   * @brief Construct a new ActDeviceConfigWSCommand object
   *
   */
  ActDeviceConfigWSCommand() {}
};

class ActDeviceCommandLineWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint64, id, Id);
  ACT_JSON_FIELD(QString, command, Command);

 public:
  /**
   * @brief Construct a new ActDeviceCommandLineWSCommand object
   *
   */
  ActDeviceCommandLineWSCommand() {}
};
/**
 * @brief The ACT Config devices ID list websocket command class
 *
 */
class ActConfigDeviceIdListWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint64, id, Id);  ///< The device id list
};

class ActTopologyMappingWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_FIELD(qint64, design_baseline_id, DesignBaselineId);

 public:
  /**
   * @brief Construct a new ActTopologyMappingWSCommand object
   *
   */
  ActTopologyMappingWSCommand() { design_baseline_id_ = -1; }
};

/**
 * @brief The ACT Deploy websocket command class
 *
 */
class ActDeployBaseWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint64, id, Id);                    ///< The device id list
  ACT_JSON_ENUM(ActDeployActionEnum, action, Action);            ///< The DeployAction enum
  ACT_JSON_FIELD(bool, skip_mapping_device, SkipMappingDevice);  ///< The SkipMappingDevice boolean

 public:
  /**
   * @brief Construct a new ActLinkSequenceDetectWSCommand object
   *
   */
  ActDeployBaseWSCommand() {
    action_ = ActDeployActionEnum::kAll;
    skip_mapping_device_ = false;
  }
};

/**
 * @brief The ACT Deploy websocket command class
 *
 */
class ActDeployFirmwareWSCommand : public ActDeployBaseWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, firmware_name, FirmwareName);  ///< The FirmwareName item
};

/**
 * @brief The ACT Config firmware devices ID list  websocket command class
 *
 */

class ActConfigDeviceIdListFirmwareWSCommand : public ActConfigDeviceIdListWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, firmware_name, FirmwareName);  ///< FirmwareName item
};

/**
 * @brief The ACT Config locator devices ID list websocket command class
 *
 */
class ActConfigDeviceIdListLocatorWSCommand : public ActConfigDeviceIdListWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, duration, Duration);  ///< Duration item

 public:
  ActConfigDeviceIdListLocatorWSCommand() { this->duration_ = ACT_LOCATOR_DURATION_MIN; }
};

class ActProbeDeviceProfileWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActScanIpRangeEntry, scan_ip_ranges,
                              ScanIpRanges);  ///< The ProbeDeviceProfileConfig list
};

/**
 * @brief The ACT start monitor websocket command class
 *
 */
class ActStartMonitorWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, fake_mode, FakeMode);  ///< FakeMode to test monitor module

 public:
  /**
   * @brief Construct a new ActStartMonitorWSCommand object
   *
   */
  ActStartMonitorWSCommand() { fake_mode_ = false; }
};

/**
 * @brief The ACT stop monitor websocket command class
 *
 */
class ActStopMonitorWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, save_to_project, SaveToProject);  ///< Save to offline topology

 public:
  /**
   * @brief Construct a new ActStopMonitorWSCommand object
   *
   */
  ActStopMonitorWSCommand() { save_to_project_ = true; }
};

class ActExportDeviceConfigWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint64, Id, Id);
};

class ActImportDeviceConfigWSCommand : public ActWSCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint64, Id, Id);
};