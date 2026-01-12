

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QDateTime>

#include "act_core.hpp"
#include "act_ws_cmd.hpp"
#include "act_ws_listener.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WSListener

oatpp::async::CoroutineStarter WSListener::onPing(const std::shared_ptr<AsyncWebSocket> &socket,
                                                  const oatpp::String &message) {
  // OATPP_LOGD(TAG, "onPing");
  qDebug() << "onPing";
  socket->sendPongAsync(message);
  return nullptr;
}

oatpp::async::CoroutineStarter WSListener::onPong(const std::shared_ptr<AsyncWebSocket> &socket,
                                                  const oatpp::String &message) {
  // no effect on the program's behavior
  static_cast<void>(socket);

  // no effect on the program's behavior
  static_cast<void>(message);

  // OATPP_LOGD(TAG, "onPong");
  qDebug() << "onPong";
  return nullptr;
}

oatpp::async::CoroutineStarter WSListener::onClose(const std::shared_ptr<AsyncWebSocket> &socket, v_uint16 code,
                                                   const oatpp::String &message) {
  // no effect on the program's behavior
  static_cast<void>(socket);

  // no effect on the program's behavior
  static_cast<void>(message);

  // OATPP_LOGD(TAG, "onClose code=%d", code);
  qDebug() << "onClose code=" << code;
  // qDebug() << "onClose message=" << message->c_str();

  return nullptr;
}

oatpp::async::CoroutineStarter WSListener::readMessage(const std::shared_ptr<AsyncWebSocket> &socket, v_uint8 opcode,
                                                       p_char8 data, oatpp::v_io_size size) {
  // no effect on the program's behavior
  static_cast<void>(opcode);

  ACT_STATUS_INIT();

  QMutexLocker lock(&act::core::g_core.mutex_);

  qint64 ws_listener_id;
  std::static_pointer_cast<WSListener>(socket->getListener())->getId(ws_listener_id);

  // debug log
  qDebug() << "WSListener ID:" << ws_listener_id;

  if (size == 0) {  // message transfer finished

    auto wholeMessage = m_messageBuffer.toString();
    m_messageBuffer.setCurrentPosition(0);

    qDebug() << "WS Message Body:" << wholeMessage->c_str();

    ActWSCommand cmd;
    try {
      cmd.FromString(wholeMessage->c_str());
      // qDebug() << "ActWSCommand:" << cmd.ToString().toStdString().c_str();

      act_status = cmd.TransformOpCodeEnum();
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Cannot transform from opcode";
        throw std::runtime_error("Cannot transform from opcode");
      }
    } catch (std::exception &e) {
      qCritical() << e.what();
      ActBadRequest act_bad_status(e.what());
      qCritical() << act_bad_status.ToString().toStdString().c_str();
      ActBaseErrorMessageResponse ws_resp(0, dynamic_cast<ActStatusBase &>(act_bad_status));
      this->sendMessage(ws_resp.ToString(ws_resp.key_order_).toStdString().c_str());
    }

    qint64 project_id = cmd.GetProjectId();
    qDebug() << QString("ActWSCommand->OpCodeEnum: %1")
                    .arg(kActWSCommandEnumMap.key(cmd.GetOpCodeEnum()))
                    .toStdString()
                    .c_str();
    switch (cmd.GetOpCodeEnum()) {
      case ActWSCommandEnum::kTestStart: {
        act_status = act::core::g_core.StartWSTest(project_id, ws_listener_id);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start WS Test failed";
        }
      } break;
      case ActWSCommandEnum::kTestStop: {
        act_status = act::core::g_core.StopWSTest(project_id, ws_listener_id);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop WS Test failed";
        }
      } break;
      case ActWSCommandEnum::kStartIntelligentRequest: {
        ActIntelligentRequestWSCommand intelligent_request_cmd;
        try {
          intelligent_request_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();

          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();

          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartIntelligentRequest, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        act_status = act::core::g_core.StartCommandLineInterfaceRequest(
            project_id, ws_listener_id, intelligent_request_cmd.GetIntelligentRequest());
        if (IsActStatusSuccess(act_status)) {
          break;
        }

        act_status = act::core::g_core.StartIntelligentRequest(project_id, ws_listener_id,
                                                               intelligent_request_cmd.GetIntelligentRequest());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start handle Intelligent request failed";
        }
      } break;
      case ActWSCommandEnum::kStopIntelligentRequest: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (IsActStatusSuccess(act_status)) {
          act::core::g_core.DisableMultipleTransaction(project_id);
          act::core::g_core.StopTransaction(project_id);
        } else {
          qCritical() << "Stop handle Intelligent request failed";
        }
      } break;
      case ActWSCommandEnum::kStartIntelligentQuestionnaireUpload: {
        ActIntelligentQuestionnaireUploadWSCommand intelligent_upload_cmd;
        try {
          intelligent_upload_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();

          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();

          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartIntelligentQuestionnaireUpload, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        act_status = act::core::g_core.StartIntelligentQuestionnaireUpload(
            project_id, ws_listener_id, intelligent_upload_cmd.GetIntelligentUpload());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start handle Intelligent upload failed";
        }
      } break;
      case ActWSCommandEnum::kStopIntelligentQuestionnaireUpload: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (IsActStatusSuccess(act_status)) {
          // act::core::g_core.DisableMultipleTransaction(project_id);
          // act::core::g_core.StopTransaction(project_id);
        } else {
          qCritical() << "Stop handle Intelligent upload failed";
        }
      } break;
      case ActWSCommandEnum::kStartIntelligentQuestionnaireDownload: {
        ActIntelligentQuestionnaireDownloadWSCommand intelligent_download_cmd;
        act_status = act::core::g_core.StartIntelligentQuestionnaireDownload(
            project_id, ws_listener_id, intelligent_download_cmd.GetIntelligentDownload());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start handle Intelligent download failed";
        }
      } break;
      case ActWSCommandEnum::kStopIntelligentQuestionnaireDownload: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (IsActStatusSuccess(act_status)) {
          act::core::g_core.DisableMultipleTransaction(project_id);
          act::core::g_core.StopTransaction(project_id);
        } else {
          qCritical() << "Stop handle Intelligent download failed";
        }
      } break;
      case ActWSCommandEnum::kStartCompute: {
        act_status = act::core::g_core.StartCompute(project_id, ws_listener_id);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start compute failed";
        }
      } break;
      case ActWSCommandEnum::kStopCompute: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop compute failed";
        }
      } break;
      case ActWSCommandEnum::kStartCompare: {
        act_status = act::core::g_core.StartCompare(project_id, ws_listener_id);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start compare failed";
        }
      } break;
      case ActWSCommandEnum::kStopCompare: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop compare failed";
        }
      } break;
      case ActWSCommandEnum::kStartDeploy: {
        ActDeployBaseWSCommand deploy_base_cmd;
        try {
          deploy_base_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());

          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
              ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeploy, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        act_status = act::core::g_core.StartDeployIni(project_id, ws_listener_id, deploy_base_cmd.GetId(),
                                                      deploy_base_cmd.GetSkipMappingDevice());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start Deploy failed";
        }
      } break;
      case ActWSCommandEnum::kStopDeploy: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop Deploy failed";
        }
      } break;
      case ActWSCommandEnum::kStartManufactureDeploy: {
        act_status = act::core::g_core.StartManufactureDeployIni(project_id, ws_listener_id);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start ManufactureDeploy failed";
        }
      } break;
      case ActWSCommandEnum::kStopManufactureDeploy: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop ManufactureDeploy failed";
        }
      } break;
      case ActWSCommandEnum::kStartScanTopology: {
        ActScanTopologyWSCommand scan_cmd;
        try {
          scan_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();

          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();

          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartScanTopology, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        act_status = act::core::g_core.StartScanTopology(project_id, ws_listener_id, scan_cmd.GetNewTopology());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start scan topology failed";
        }
      } break;
      case ActWSCommandEnum::kStopScanTopology: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop scan topology failed";
        }
      } break;
      case ActWSCommandEnum::kStartSyncDevices: {
        ActConfigDeviceIdListWSCommand dev_id_list_cmd;
        try {
          dev_id_list_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartSyncDevices, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        QList<qint64> dev_id_list = dev_id_list_cmd.GetId();
        act_status = act::core::g_core.StartSyncDevices(project_id, ws_listener_id, dev_id_list);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start Sync devices failed";
        }
      } break;
      case ActWSCommandEnum::kStopSyncDevices: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop Sync devices failed";
        }
      } break;
      case ActWSCommandEnum::kStartDeviceDiscovery: {
        ActDeviceDiscoveryConfig dev_discovery_cfg;
        try {
          dev_discovery_cfg.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartDeviceDiscovery, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        act_status = act::core::g_core.StartDeviceDiscovery(project_id, ws_listener_id, dev_discovery_cfg);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop DeviceDiscovery failed";
        }
      } break;
      case ActWSCommandEnum::kStartRetryConnect: {
        ActRetryConnectConfig retry_connect_cfg;
        try {
          retry_connect_cfg.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartRetryConnect, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        act_status = act::core::g_core.StartRetryConnect(project_id, ws_listener_id, retry_connect_cfg);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop RetryConnect failed";
        }
      } break;
      case ActWSCommandEnum::kStartLinkSequenceDetect: {
        ActLinkSequenceDetectWSCommand link_sequence_detect_cmd;
        try {
          link_sequence_detect_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartSetNetworkSetting, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }
        bool from_broadcast_search = link_sequence_detect_cmd.GetFromBroadcastSearch();
        QList<qint64> dev_id_list = link_sequence_detect_cmd.GetId();
        act_status =
            act::core::g_core.StartLinkSequenceDetect(project_id, ws_listener_id, dev_id_list, from_broadcast_search);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start LinkSequenceDetect failed";
        }
      } break;
      case ActWSCommandEnum::kStartDeviceConfig: {
        ActDeviceConfigWSCommand dev_config_cmd;
        try {
          dev_config_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartDeviceConfig, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }
        QList<qint64> dev_id_list = dev_config_cmd.GetId();
        ActDeviceConfigTypeEnum config_type = static_cast<ActDeviceConfigTypeEnum>(dev_config_cmd.GetType());
        act_status =
            act::core::g_core.StartDeviceConfigCommission(project_id, ws_listener_id, dev_id_list, config_type);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start DeviceConfig Commission failed";
        }
      } break;
      case ActWSCommandEnum::kStopDeviceConfig: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop DeviceConfig failed";
        }
      } break;
      case ActWSCommandEnum::kStartDeviceCommandLine: {
        ActDeviceCommandLineWSCommand dev_command_line_cmd;
        try {
          dev_command_line_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartDeviceCommandLine, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        act_status = act::core::g_core.StartDeviceCommandLine(project_id, ws_listener_id, dev_command_line_cmd.GetId(),
                                                              dev_command_line_cmd.GetCommand());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start DeviceConfig Commission failed";
        }
      } break;
      case ActWSCommandEnum::kStopDeviceCommandLine: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop Device Command Line failed";
        }
      } break;
      case ActWSCommandEnum::kStartSetNetworkSetting: {
        ActSetNetworkSettingWSCommand ip_configure_cmd;
        try {
          ip_configure_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartSetNetworkSetting, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }
        bool from_broadcast_search = ip_configure_cmd.GetFromBroadcastSearch();
        QList<ActDeviceIpConfiguration> dev_ip_config_list = ip_configure_cmd.GetPairs();
        act_status = act::core::g_core.StartIpConfiguration(project_id, ws_listener_id, dev_ip_config_list,
                                                            from_broadcast_search);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start IpConfiguration failed";
        }
      } break;
      case ActWSCommandEnum::kStartReboot: {
        ActConfigDeviceIdListWSCommand config_dev_id_list_cmd;
        try {
          config_dev_id_list_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
              ActWSResponseErrorTransfer(ActWSCommandEnum::kStartReboot, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        QList<qint64> dev_id_list = config_dev_id_list_cmd.GetId();
        act_status = act::core::g_core.StartConfigReboot(project_id, ws_listener_id, dev_id_list);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start ConfigReboot failed";
        }
      } break;
      case ActWSCommandEnum::kStartLocator: {
        ActConfigDeviceIdListLocatorWSCommand config_dev_id_list_locator_cmd;
        try {
          config_dev_id_list_locator_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartLocator, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        QList<qint64> dev_id_list = config_dev_id_list_locator_cmd.GetId();
        quint16 duration = config_dev_id_list_locator_cmd.GetDuration();
        act_status = act::core::g_core.StartConfigLocator(project_id, ws_listener_id, dev_id_list, duration);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start ConfigLocator failed";
        }
      } break;
      case ActWSCommandEnum::kStartGetEventLog: {
        ActConfigDeviceIdListWSCommand config_dev_id_list_cmd;
        try {
          config_dev_id_list_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartGetEventLog, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        QList<qint64> dev_id_list = config_dev_id_list_cmd.GetId();
        act_status = act::core::g_core.StartOperationEventLog(project_id, ws_listener_id, dev_id_list);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start OperationEventLog failed";
        }
      } break;
      case ActWSCommandEnum::kStartFactoryDefault: {
        ActConfigDeviceIdListWSCommand config_dev_id_list_cmd;
        try {
          config_dev_id_list_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartFactoryDefault, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        QList<qint64> dev_id_list = config_dev_id_list_cmd.GetId();
        act_status = act::core::g_core.StartConfigFactoryDefault(project_id, ws_listener_id, dev_id_list);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start ConfigFactoryDefault failed";
        }
      } break;
      case ActWSCommandEnum::kStartFirmwareUpgrade: {
        ActConfigDeviceIdListFirmwareWSCommand config_dev_id_list_fw_cmd;
        try {
          config_dev_id_list_fw_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartFirmwareUpgrade, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        QList<qint64> dev_id_list = config_dev_id_list_fw_cmd.GetId();
        QString firmware_name = config_dev_id_list_fw_cmd.GetFirmwareName();
        act_status =
            act::core::g_core.StartConfigFirmwareUpgrade(project_id, ws_listener_id, dev_id_list, firmware_name);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start ConfigFirmwareUpgrade failed";
        }
      } break;
      case ActWSCommandEnum::kStartEnableSnmp: {
        ActConfigDeviceIdListWSCommand config_dev_id_list_cmd;
        try {
          config_dev_id_list_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartEnableSnmp, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        QList<qint64> dev_id_list = config_dev_id_list_cmd.GetId();
        act_status = act::core::g_core.StartConfigEnableSnmp(project_id, ws_listener_id, dev_id_list);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start ConfigEnableSnmp failed";
        }
      } break;
      case ActWSCommandEnum::kStopDeviceDiscovery: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop device discovery failed";
        }
      } break;
      case ActWSCommandEnum::kStartProbeDeviceProfile: {
        ActProbeDeviceProfileWSCommand probe_device_cmd;
        try {
          probe_device_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartProbeDeviceProfile, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }
        QList<ActScanIpRangeEntry> scan_ip_ranges = probe_device_cmd.GetScanIpRanges();
        act_status = act::core::g_core.StartProbeDeviceProfile(ws_listener_id, scan_ip_ranges);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start ProbeDeviceProfile failed";
        }
      } break;
      case ActWSCommandEnum::kStopProbeDeviceProfile: {
        act_status = act::core::g_core.StopWSJobSystem(ws_listener_id);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop ProbeDeviceProfile failed";
        }
      } break;
      case ActWSCommandEnum::kGetProjectDataVersion: {
        act_status = act::core::g_core.GetProjectDataVersion(project_id, ws_listener_id);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Get project data version failed";
        }
      } break;
      case ActWSCommandEnum::kStartMonitor: {
        ActStartMonitorWSCommand start_monitor_cmd;
        try {
          start_monitor_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartProbeDeviceProfile, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        act_status = act::core::g_core.StartMonitor(project_id, ws_listener_id, start_monitor_cmd);
        if (!IsActStatusSuccess(act_status) && !IsActStatusSkip(act_status)) {
          qCritical() << "Start monitor failed";
        }
      } break;
      case ActWSCommandEnum::kStopMonitor: {
        ActStopMonitorWSCommand stop_monitor_cmd;
        try {
          stop_monitor_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartProbeDeviceProfile, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        act_status = act::core::g_core.StopMonitor(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop monitor failed";
        }
      } break;
      case ActWSCommandEnum::kStartTopologyMapping: {
        ActTopologyMappingWSCommand topology_mapping_cmd;
        try {
          topology_mapping_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());

          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartTopologyMapping, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

          break;
        }

        act_status = act::core::g_core.StartTopologyMapping(project_id, topology_mapping_cmd.GetDesignBaselineId(),
                                                            ws_listener_id);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start TopologyMapping failed";
        }

      } break;
      case ActWSCommandEnum::kStopTopologyMapping: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop TopologyMapping failed";
        }
      } break;
      case ActWSCommandEnum::kStartScanMapping: {
        act_status = act::core::g_core.StartScanMapping(project_id, ws_listener_id);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start ScanMapping failed";
        }

      } break;
      case ActWSCommandEnum::kStopScanMapping: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop ScanMapping failed";
        }
      } break;
      case ActWSCommandEnum::kStartExportDeviceConfig: {
        ActExportDeviceConfigWSCommand export_device_config_cmd;
        try {
          export_device_config_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartExportDeviceConfig, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
          break;
        }

        QList<qint64> dev_id_list = export_device_config_cmd.GetId();
        act_status = act::core::g_core.StartExportDeviceConfig(project_id, ws_listener_id, dev_id_list);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start export failed";
        }
      } break;
      case ActWSCommandEnum::kStopExportDeviceConfig: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop export failed";
        }
      } break;
      case ActWSCommandEnum::kStartImportDeviceConfig: {
        ActImportDeviceConfigWSCommand import_device_config_cmd;
        try {
          import_device_config_cmd.FromString(wholeMessage->c_str());
        } catch (std::exception &e) {
          qCritical() << e.what();
          ActBadRequest act_bad_status(e.what());
          qCritical() << act_bad_status.ToString().toStdString().c_str();
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(
              ActWSCommandEnum::kStartImportDeviceConfig, dynamic_cast<ActStatusBase &>(act_bad_status));
          this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
          break;
        }

        QList<qint64> dev_id_list = import_device_config_cmd.GetId();
        if (dev_id_list.isEmpty()) {
          qCritical() << "Device ID list is empty";
          break;
        }

        act_status = act::core::g_core.StartImportDeviceConfig(project_id, ws_listener_id, dev_id_list);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Start import failed";
        }
      } break;
      case ActWSCommandEnum::kStopImportDeviceConfig: {
        act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, cmd.GetOpCodeEnum());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Stop import failed";
        }
      } break;
      default:
        ActBadRequest act_bad_status("The opcode is not supported");
        qCritical() << act_bad_status.ToString().toStdString().c_str();
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(cmd.GetOpCodeEnum(), dynamic_cast<ActStatusBase &>(act_bad_status));
        this->sendMessage(ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
        // socket.sendClose();
    }
    // Check project id
  } else if (size > 0) {  // message frame received
    m_messageBuffer.writeSimple(data, size);
  }

  qDebug() << "WSListener::onMessage: done";

  return nullptr;
}

// void WSListener::sendMessage(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message)
// {
//   socket->sendOneFrameTextAsync("Hello from oatpp!:" + message);
// }

void WSListener::sendMessage(const oatpp::String &message) {
  class SendMessageCoroutine : public oatpp::async::Coroutine<SendMessageCoroutine> {
   private:
    oatpp::async::Lock *m_lock;
    std::shared_ptr<AsyncWebSocket> m_websocket;
    oatpp::String m_message;

   public:
    SendMessageCoroutine(oatpp::async::Lock *lock, const std::shared_ptr<AsyncWebSocket> &websocket,
                         const oatpp::String &message)
        : m_lock(lock), m_websocket(websocket), m_message(message) {}

    Action act() override {
      return oatpp::async::synchronize(m_lock, m_websocket->sendOneFrameTextAsync(m_message)).next(finish());
    }
  };
  m_asyncExecutor.execute<SendMessageCoroutine>(&m_writeLock, m_socket, message);
}

void WSListener::getId(qint64 &id) { id = m_id; }

void WSListener::getProjectId(qint64 &project_id) { project_id = m_project_id; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WSInstanceListener

std::atomic<v_int32> WSInstanceListener::SOCKETS(0);

void WSInstanceListener::onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket,
                                                   const std::shared_ptr<const ParameterMap> &params) {
  ACT_STATUS_INIT();

  auto project_id_param = params->find("projectId")->second;
  QString project_id_str(project_id_param->c_str());
  qint64 project_id = project_id_str.toInt();

  // Use the current time represent the listener id
  QMutexLocker lock(&act::core::g_core.mutex_);

  SOCKETS++;
  QDateTime current_date_time = QDateTime::currentDateTime();
  qint64 milliseconds_since_epoch = current_date_time.toMSecsSinceEpoch();
  auto ws_listener = std::make_shared<WSListener>(socket, milliseconds_since_epoch, project_id);

  act_status = act::core::g_core.AddWSListener(ws_listener);

  qint64 listener_id;
  ws_listener->getId(listener_id);
  qDebug() << QString("WS Connection created(ProjectID: %1, ListenerID: %2). Connection count= %3")
                  .arg(project_id)
                  .arg(listener_id)
                  .arg(SOCKETS.load())
                  .toStdString()
                  .c_str();

  socket->setListener(ws_listener);
}

void WSInstanceListener::onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket) {
  ACT_STATUS_INIT();

  // Get listener id
  auto ws_listener = std::static_pointer_cast<WSListener>(socket->getListener());
  qint64 listener_id;
  ws_listener->getId(listener_id);
  QMutexLocker lock(&act::core::g_core.mutex_);

  SOCKETS--;
  act_status = act::core::g_core.RemoveWSListener(listener_id);

  qDebug() << QString("WS Connection closed(ListenerID: %1). Connection count= %2")
                  .arg(listener_id)
                  .arg(SOCKETS.load())
                  .toStdString()
                  .c_str();
}
