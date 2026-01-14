/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_DEVICE_CONFIGURATION_HPP
#define ACT_DEVICE_CONFIGURATION_HPP

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QQueue>
#include <QString>
#include <thread>

#include "act_broadcast_search.hpp"
#include "act_core.hpp"
#include "act_device.hpp"
#include "act_device_config_type.hpp"
#include "act_device_ip_config.hpp"
#include "act_json.hpp"
#include "act_southbound.hpp"
#include "act_status.hpp"

class ActCommandLineOutput : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, execResult, execResult);
  ACT_JSON_FIELD(QString, responseStr, responseStr);
};

class ActDeviceConfiguration {
  Q_GADGET

  // for thread
  std::unique_ptr<std::thread> device_config_thread_;  ///< DeviceConfiguration thread item
  ACT_JSON_FIELD(bool, stop_flag, StopFlag);           ///< StopFlag item
  ACT_JSON_FIELD(quint8, progress, Progress);          ///< Progress item
  ACT_STATUS device_config_act_status_;                ///< DeviceConfiguration thread status

  // ACT_JSON_COLLECTION_OBJECTS(QQueue, qint64, result_queue, ResultQueue);
  ACT_JSON_QT_DICT(QMap, QString, QString, mac_host_map, MacHostMap);  ///< DeviceHostMap item <mac, host_ip>
  ACT_JSON_OBJECT(ActProfiles, profiles, Profiles);                    ///< Profiles item

  ActSouthbound southbound_;

  QMutex mutex_;

 private:
  /**
   * @brief DeviceConfiguration Error handler object
   *
   * @param called_func
   * @param error_reason
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS DeviceConfigurationErrorHandler(QString called_func, const QString &error_reason, const ActDevice &device);

  /**
   * @brief Update Device FirmwareFeatureProfile ID
   *
   * @param dev
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceFirmwareFeatureProfileId(ActDevice &dev);

  /**
   * @brief Update Device connection status
   *
   * @param dev
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceConnectByFeature(ActDevice &dev);

  /**
   * @brief Sort the Device ID list object
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS SortDeviceIdList(const ActProject &project, QList<qint64> &dev_id_list);

  /**
   * @brief Check the Project's MonitorEndpoint object
   *
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS CheckProjectMonitorEndpoint(const ActProject &project);

  /**
   * @brief Trigger Commission for thread
   *
   * @param project
   * @param dev_id_list
   * @param config_type
   * @param sync_dev_config
   */
  void TriggerCommissionForThread(const ActProject &project, const QList<qint64> &dev_id_list,
                                  const ActDeviceConfigTypeEnum &config_type, ActDeviceConfig &sync_dev_config);

  /**
   * @brief Trigger SetNetworkSetting for thread
   *
   * @param act_project
   * @param dev_ip_config_list
   * @param from_broadcast_search
   */
  void TriggerSetNetworkSettingForThread(const ActProject &act_project,
                                         QList<ActDeviceIpConfiguration> dev_ip_config_list,
                                         bool from_broadcast_search);

  /**
   * @brief Trigger Reboot for thread
   *
   * @param dev_list
   */
  void TriggerRebootForThread(const ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Trigger FactoryDefault for thread
   *
   * @param project
   * @param dev_id_list
   */
  void TriggerFactoryDefaultForThread(const ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Trigger FirmwareUpgrade for thread
   *
   * @param project
   * @param dev_id_list
   * @param file_path
   */
  void TriggerFirmwareUpgradeForThread(const ActProject &project, const QList<qint64> &dev_id_list,
                                       const QString &file_path);

  /**
   * @brief Trigger EnableSnmp for thread
   *
   * @param project
   * @param dev_id_list
   */
  void TriggerEnableSnmpForThread(const ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Trigger Locator for thread
   *
   * @param project
   * @param dev_id_list
   * @param duration
   */
  void TriggerLocatorForThread(const ActProject &project, const QList<qint64> &dev_id_list, const quint16 &duration);

  /**
   * @brief Trigger GetEventLog for thread
   *
   * @param project
   * @param dev_id_list
   */
  void TriggerGetEventLogForThread(const ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Trigger ImportConfig for thread
   *
   * @param project
   * @param dev_id_list
   * @param file_path
   */
  void TriggerImportConfigForThread(const ActProject &project, const QList<qint64> &dev_id_list,
                                    const QString &file_path);

  /**
   * @brief Trigger ExportConfig for thread
   *
   * @param project
   * @param dev_id_list
   * @param path
   */
  void TriggerExportConfigForThread(const ActProject &project, const QList<qint64> &dev_id_list, const QString &path);

  /**
   * @brief Trigger CommandLine for thread
   *
   * @param project
   * @param dev_id_list
   * @param command
   */
  void TriggerCommandLineForThread(const ActProject &project, const QList<qint64> &dev_id_list, const QString &command);

  /**
   * @brief  Update progress
   *
   * @param progress
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProgress(quint8 progress);

 public:
  QQueue<ActDeviceConfigureResult> result_queue_;
  QQueue<ActDeviceEventLogResult> event_log_result_queue_;

  /**
   * @brief Construct a new Act BroadcastSearch object
   *
   */
  ActDeviceConfiguration(const ActProfiles &profiles)
      : profiles_(profiles),
        progress_(0),
        stop_flag_(false),
        device_config_act_status_(std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug)),
        device_config_thread_(nullptr) {
    act::core::g_core.GetMacHostMap(mac_host_map_);

    southbound_.SetProfiles(profiles);
  }

  /**
   * @brief Destroy the Act BroadcastSearch object
   *
   */
  ~ActDeviceConfiguration();

  /**
   * @brief Stop thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Stop();

  /**
   * @brief Get the status of the BroadcastSearch thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetStatus();

  /**
   * @brief Start Commission thread object
   *
   * @param project
   * @param dev_id_list
   * @param config_type
   * @param sync_dev_config
   * @return ACT_STATUS
   */
  ACT_STATUS StartCommission(const ActProject &project, const QList<qint64> &dev_id_list,
                             const ActDeviceConfigTypeEnum &config_type, ActDeviceConfig &sync_dev_config);

  /**
   * @brief Commission object
   *
   * @param project
   * @param dev_id_list
   * @param config_type
   * @param sync_dev_config
   * @return ACT_STATUS
   */
  ACT_STATUS Commission(const ActProject &project, const QList<qint64> &dev_id_list,
                        const ActDeviceConfigTypeEnum &config_type, ActDeviceConfig &sync_dev_config);

  /**
   * @brief Start SetNetworkSetting thread object
   *
   * @param act_project
   * @param dev_ip_config_list
   * @param from_broadcast_search
   * @return ACT_STATUS
   */
  ACT_STATUS StartSetNetworkSetting(const ActProject &act_project, QList<ActDeviceIpConfiguration> dev_ip_config_list,
                                    bool from_broadcast_search);

  /**
   * @brief Ip Configure object
   *
   * @param act_project
   * @param dev_ip_config_list
   * @param from_broadcast_search
   * @return ACT_STATUS
   */
  ACT_STATUS SetNetworkSetting(const ActProject &act_project, const QList<ActDeviceIpConfiguration> &dev_ip_config_list,
                               const bool &from_broadcast_search);

  /**
   * @brief Start Reboot thread object
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartReboot(const ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Reboot object
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS Reboot(const ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Start Factory Default thread object
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartFactoryDefault(const ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Factory Default object
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS FactoryDefault(const ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Start Firmware upgrade thread object
   *
   * @param project
   * @param dev_id_list
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS StartFirmwareUpgrade(const ActProject &project, const QList<qint64> &dev_id_list,
                                  const QString &file_path);

  /**
   * @brief Firmware upgrade object
   *
   * @param project
   * @param dev_id_list
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS FirmwareUpgrade(const ActProject &project, const QList<qint64> &dev_id_list, const QString &file_path);

  /**
   * @brief Start EnableSnmp thread object
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartEnableSnmp(const ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief EnableSnmp object
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS EnableSnmp(const ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Start Locator thread object
   *
   * @param project
   * @param dev_id_list
   * @param duration
   * @return ACT_STATUS
   */
  ACT_STATUS StartLocator(const ActProject &project, const QList<qint64> &dev_id_list, const quint16 &duration);

  /**
   * @brief Locator object
   *
   * @param project
   * @param dev_id_list
   * @param duration
   * @return ACT_STATUS
   */
  ACT_STATUS Locator(const ActProject &project, const QList<qint64> &dev_id_list, const quint16 &duration);

  /**
   * @brief Start GetEventLog thread object
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartGetEventLog(const ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Get the EventLog object
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS GetEventLog(const ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Start ImportConfig thread object
   *
   * @param project
   * @param dev_id_list
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS StartImportConfig(const ActProject &project, const QList<qint64> &dev_id_list, const QString &path);

  /**
   * @brief ImportConfig object
   *
   * @param project
   * @param dev_id_list
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS ImportConfig(const ActProject &project, const QList<qint64> &dev_id_list, const QString &path);

  /**
   * @brief Start ExportConfig thread object
   *
   * @param project
   * @param dev_id_list
   * @param path
   * @return ACT_STATUS
   */
  ACT_STATUS StartExportConfig(const ActProject &project, const QList<qint64> &dev_id_list, const QString &path);

  /**
   * @brief ExportConfig object
   *
   * @param project
   * @param dev_id_list
   * @param path
   * @return ACT_STATUS
   */
  ACT_STATUS ExportConfig(const ActProject &project, const QList<qint64> &dev_id_list, const QString &path);

  /**
   * @brief Start CommandLine object
   *
   * @param project
   * @param dev_id_list
   * @param command
   * @return ACT_STATUS
   */
  ACT_STATUS StartCommandLine(const ActProject &project, const QList<qint64> &dev_id_list, const QString &command);

  /**
   * @brief CommandLine object
   *
   * @param project
   * @param dev_id_list
   * @param path
   * @return ACT_STATUS
   */
  ACT_STATUS CommandLine(const ActProject &project, const QList<qint64> &dev_id_list, const QString &path);
};

#endif /* ACT_DEVICE_CONFIGURATION_HPP */