/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <QMap>
#include <QMutex>
#include <QStack>
#include <future>  // for std::promise, std::future

#include "act_algorithm.hpp"
#include "act_deploy.hpp"

// #include "act_db.hpp"
#include "act_broadcast_search_config.hpp"
#include "act_device.hpp"
#include "act_device_ip_config.hpp"
#include "act_device_profile.hpp"
#include "act_device_setting.hpp"
#include "act_ethernet_module.hpp"
#include "act_event_log.hpp"
#include "act_feature_profile.hpp"
#include "act_firmware.hpp"
#include "act_firmware_feature_profile.hpp"
#include "act_host_adapter.hpp"
#include "act_import_export_project.hpp"
#include "act_intelligent_request.hpp"
#include "act_job.hpp"
#include "act_json.hpp"
#include "act_license.hpp"
#include "act_mqtt_client.hpp"
#include "act_network_baseline.hpp"
#include "act_notification_msg.hpp"
#include "act_power_device_profile.hpp"
#include "act_power_module.hpp"
#include "act_project.hpp"
#include "act_scan_ip_range.hpp"
#include "act_service_platform_request.hpp"
#include "act_service_profile.hpp"
#include "act_sfp_module.hpp"
#include "act_software_license_profile.hpp"
#include "act_status.hpp"
#include "act_system.hpp"
#include "act_topology.hpp"
#include "act_traffic.hpp"
#include "act_user.hpp"
#include "act_utilities.hpp"
#include "act_vlan_config.hpp"
#include "act_vlan_view.hpp"
#include "oatpp-websocket/AsyncWebSocket.hpp"
#include "oatpp-websocket/WebSocket.hpp"
#include "opcua_class_based_server.h"
#include "websocket/act_ws_listener.hpp"

#ifdef max
#undef max
#endif

#define HISTORY_LIMIT (16)
class WSListener;  // FWD

namespace act {
namespace core {
// typedef void (*cb_func)(ACT_STATUS status);

extern QMap<qint64, std::pair<std::shared_ptr<std::promise<void>>, std::shared_ptr<std::thread>>>
    ws_thread_handler_pools;

class ActWSTest {
  Q_GADGET
  ACT_JSON_FIELD(bool, stop_flag, StopFlag);
  ACT_JSON_FIELD(quint16, progress, Progress);

 private:
  std::unique_ptr<std::thread> thread_;
  ACT_STATUS act_status_;

 public:
  ActWSTest() : progress_(0), act_status_(std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug)) {}

  ~ActWSTest();

  ACT_STATUS Start();

  ACT_STATUS Stop();

  void Loop();

  ACT_STATUS GetStatus();
};

/**
 * @brief The ACT core class
 *
 */
class ActCore : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE
  // Create data members to be serialized - you can use this members in code
  ACT_JSON_OBJECT(ActLicense, license, License);
  ACT_JSON_OBJECT(ActSystem, system_config, SystemConfig);
  ACT_JSON_QT_SET_OBJECTS(ActUser, users, UserSet);
  ACT_JSON_QT_SET_OBJECTS(ActProject, projects, ProjectSet);
  ACT_JSON_QT_SET_OBJECTS(ActDeviceProfile, device_profiles, DeviceProfileSet);
  ACT_JSON_QT_SET_OBJECTS(ActDeviceProfile, default_device_profiles, DefaultDeviceProfileSet);
  ACT_JSON_QT_SET_OBJECTS(ActFeatureProfile, feature_profiles, FeatureProfileSet);
  ACT_JSON_QT_SET_OBJECTS(ActFirmwareFeatureProfile, firmware_feature_profiles, FirmwareFeatureProfileSet);
  ACT_JSON_QT_SET_OBJECTS(ActFirmware, firmwares, FirmwareSet);
  ACT_JSON_QT_SET_OBJECTS(ActPowerDeviceProfile, power_device_profiles, PowerDeviceProfileSet);
  ACT_JSON_QT_SET_OBJECTS(ActSoftwareLicenseProfile, software_license_profiles, SoftwareLicenseProfileSet);
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActEthernetModule, eth_modules,
                           EthernetModuleMap);                                            ///< <ID, Ethernet Module>
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActPowerModule, power_modules, PowerModuleMap);  ///< <ID, Power Module>
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActSFPModule, sfp_modules, SFPModuleMap);        ///< <ID, SFP Module>
  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActSkuWithPrice, general_profiles,
                           GeneralProfileMap);  ///< <Module Name, General Profile>

  ACT_JSON_ENUM(ActSystemStatusEnum, system_status, SystemStatus);
  ACT_JSON_QT_SET_OBJECTS(ActTopology, topologies, TopologySet);  ///< [feat:2241] Topology Management
  ACT_JSON_QT_SET_OBJECTS(ActNetworkBaseline, design_baselines, DesignBaselineSet);
  ACT_JSON_QT_SET_OBJECTS(ActNetworkBaseline, operation_baselines, OperationBaselineSet);

 public:
  QMutex mutex_;

 private:
  // [bug:2832] move the project status out of the project configuration
  //  project id, project status
  QMap<qint64, ActProjectStatusEnum> project_status_list;

  QMap<QString, qint64> power_module_name_id_map_;  ///< Power module name to id map
  QMap<QString, qint64> sfp_module_name_id_map_;    ///< SFP module name to id map
  QMap<QString, qint64> eth_module_name_id_map_;    ///< Ethernet module name to id map

  std::shared_ptr<std::thread> chamberlain_thread_;
  std::pair<std::shared_ptr<std::promise<void>>, std::shared_ptr<std::thread>> system_promise_thread_pair_;
  std::pair<std::shared_ptr<std::promise<void>>, std::shared_ptr<std::thread>> monitor_promise_thread_pair_;

  QQueue<ActJob> job_queue_;
  QMutex job_queue_mutex_;
  std::vector<std::thread> workers_;
  std::vector<QQueue<ActJob>> worker_job_queue_;
  std::vector<std::unique_ptr<QMutex>> worker_queue_mutex_;  // Added vector of locks for each worker queue

  qint64 last_assigned_user_id_;
  qint64 last_assigned_project_id_;
  qint64 last_assigned_device_profile_id_;
  qint64 last_assigned_default_device_profile_id_;
  qint64 last_assigned_firmware_id_;
  qint64 last_assigned_feature_profile_id_;
  qint64 last_assigned_firmware_feature_profile_id_;
  qint64 last_assigned_topology_id_;
  qint64 last_assigned_design_baseline_id_;
  QMap<qint64, std::shared_ptr<WSListener>> ws_listeners_;
  ActNotificationMsgTmp notification_tmp_;

  // [feat:1248] The login user should be logout after password change
  QMap<qint64, bool> user_password_changed_;

  QMap<QString, QString> mac_host_map_;
  QMap<qint64, QString> restful_token_map_;           // <DeviceID, TokenString>
  QMap<qint64, QString> service_platform_token_map_;  // <ProjectID, TokenString>

  QMap<qint64, QMap<qint64, ActDeviceIpConnectConfig>> project_dev_ip_conn_cfg_map_;

  QString cli_token;
  QMap<QString, qint64> tokens_;

  // [feat:955] Undo/Redo
  QMap<qint64, QList<ActProject>> transaction_list;  // Accumulated operations in a transaction
  QMap<qint64, bool> share_transaction_list;         // The intelligent module will call multiple APIs
  QMap<qint64, QStack<ActProject>>
      undo_operation_history;  // keeps the operations so that the user will be able to undo them
  QMap<qint64, QStack<ActProject>>
      redo_operation_history;  // keeps the operations so that the user will be able to redo them

  // Deploy available
  QMap<qint64, bool> deploy_available;  // Deploy available<project_id, bool>

  ACT_STATUS DeleteStreamFromComputedResult(ActProject &project, qint64 &stream_id);

  ACT_STATUS DeleteDeviceFromComputedResult(ActProject &project, qint64 &device_id);

  // For monitor
 private:
  QMutex monitor_mutex_;
  bool fake_monitor_mode_;
  ActProject monitor_project_;
  ActProject baseline_project_;
  std::shared_ptr<std::thread> monitor_process_thread_;
  std::shared_ptr<std::thread> mqtt_client_thread_;
  QQueue<ActMonitorData> monitor_process_queue_;
  std::shared_ptr<QMutex> monitor_process_queue_mutex_;  // Added vector of locks for each worker queue

 public:
  /**
   * @brief Construct a new Act Core object
   *
   */
  ActCore();

  /**
   * @brief Initial ACT core module
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Init();
  // void LockGuard() { QMutexLocker lock(&this->mutex_); }
  void Lock() { this->mutex_.lock(); }
  void Unlock() { this->mutex_.unlock(); }

  /**
   * @brief Generate unique id of the set
   *
   * @tparam T
   * @param set
   * @param last_assigned_id
   * @param id
   * @return ACT_STATUS
   */
  template <class T>
  ACT_STATUS GenerateUniqueId(QSet<T> set, qint64 &last_assigned_id, qint64 &id) {
    ACT_STATUS_INIT();

    // Limit the maximum find times
    qint64 try_count = 0;

    // Iterator of the set
    typename QSet<T>::const_iterator iterator;

    // Pick a non-used id
    id = last_assigned_id + 1;

    // The user defined id is start from 1000
    if (id < 50001) {
      id = 50001;
    }

    // qDebug() << "std::numeric_limits<quint16>::max():" << std::numeric_limits<quint16>::max();
    while (1) {
      try_count++;

      // 0x7FFF + 1
      // Reset the id
      if (id == 0x8000) {
        id = 50001;
      }

      // Check duplicated
      iterator = set.find(T(id));
      if (iterator == set.end()) {
        // Not duplicated
        last_assigned_id = id;
        return act_status;
      }

      // All the positive integer are tested
      // std::numeric_limits<qint64>::max() is 0x7FFF
      if (try_count == std::numeric_limits<quint16>::max()) {
        break;
      }

      id++;
    }

    QString error_msg = QString("Cannot get an available unique id");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  /**
   * @brief Generate unique id of the set
   *
   * @tparam T
   * @param set
   * @param last_assigned_id
   * @param id
   * @return ACT_STATUS
   */
  template <class T>
  ACT_STATUS GenerateUniqueIdFromTimeStamp(QSet<T> set, qint64 &last_assigned_id, qint64 &id) {
    ACT_STATUS_INIT();

    // Iterator of the set
    typename QSet<T>::const_iterator iterator;

    // qDebug() << "std::numeric_limits<quint16>::max():" << std::numeric_limits<quint16>::max();
    while (1) {
      qint64 current_timestamp = QDateTime::currentSecsSinceEpoch();

      // Check duplicated
      iterator = set.find(T(current_timestamp));
      if (iterator == set.end()) {
        // Not duplicated
        id = current_timestamp;
        last_assigned_id = current_timestamp;
        break;
      }
    }
    return act_status;
  }

  /**
   * @brief Enable transaction from multiple different APIs for the specified project.
   *
   * This function marks the beginning of a new transaction for the given project.
   * A transaction is a sequence of related operations might from different APIs that are treated as a single
   * unit for project changed.
   *
   * @param project_id The ID of the project.
   * @return ACT_STATUS
   */
  ACT_STATUS EnableMultipleTransaction(qint64 project_id);

  /**
   * @brief Disable transaction from multiple different APIs for the specified project.
   *
   * @param project_id The ID of the project.
   * @return ACT_STATUS
   */
  ACT_STATUS DisableMultipleTransaction(qint64 project_id);

  /**
   * @brief Starts a new transaction for the specified project.
   *
   * This function marks the beginning of a new transaction for the given project.
   * A transaction is a sequence of related operations that are treated as a single
   * unit for project changed.
   *
   * @param project_id The ID of the project for which to start a transaction.
   * @return ACT_STATUS
   */
  ACT_STATUS StartTransaction(qint64 project_id);

  /**
   * @brief Stops the current transaction for the specified project.
   *
   * This function destroys the current transaction for the given project
   * because something has gone wrong. The accumulated operations within the
   * transaction are ignored.
   *
   * @param project_id The ID of the project for which to stop the transaction.
   * @return ACT_STATUS
   */
  ACT_STATUS StopTransaction(qint64 project_id);

  /**
   * @brief Saves a new operation for the specified project.
   *
   * This function saves a new operation for the given project. The operation is
   * added to the project's operation history, allowing for undo and redo actions.
   *
   * @param project The ActProject containing the operation to be saved.
   * @return ACT_STATUS
   */
  ACT_STATUS SaveOperation(const ActProject &project);

  /**
   * @brief Commits the current transaction for the specified project.
   *
   * This function finalizes the current transaction for the given project. The
   * accumulated operations within the transaction are added to the project's
   * operation history, allowing them to be undone and redone as a single unit.
   *
   * @param project_id The ID of the project for which to commit the transaction.
   * @return ACT_STATUS
   */
  ACT_STATUS CommitTransaction(qint64 project_id);

  /**
   * @brief Checks if undo action is available for the specified project.
   *
   * This function checks if the project with the specified ID has any previous
   * operations that can be undone.
   *
   * @param project_id The ID of the project to check.
   * @return True if undo action is available, false otherwise.
   */
  bool CanUndoProject(qint64 project_id) const;

  /**
   * @brief Checks if redo action is available for the specified project.
   *
   * This function checks if the project with the specified ID has any undone
   * operations that can be redone.
   *
   * @param project_id The ID of the project to check.
   * @return True if redo action is available, false otherwise.
   */
  bool CanRedoProject(qint64 project_id) const;

  /**
   * @brief Performs an undo action for the specified project.
   *
   * This function performs an undo action for the project with the specified ID.
   * It reverts the most recent operation and updates the project's state
   * accordingly.
   *
   * @param project_id The ID of the project to perform undo action on.
   * @return ACT_STATUS
   */
  ACT_STATUS UndoProject(qint64 project_id);

  /**
   * @brief Performs a redo action for the specified project.
   *
   * This function performs a redo action for the project with the specified ID.
   * It re-applies an undone operation and updates the project's state accordingly.
   *
   * @param project_id The ID of the project to perform redo action on.
   * @return ACT_STATUS
   */
  ACT_STATUS RedoProject(qint64 project_id);

  /**
   * @brief Applies the operation at the specified index to the project.
   *
   * This function applies the operation at the given index to the project with
   * the specified ID. It updates the project's state to reflect the changes
   * introduced by the operation.
   *
   * @param project_id The ID of the project to apply the operation to.
   * @param project The project of the operation to be applied.
   * @return ACT_STATUS
   */
  ACT_STATUS ApplyOperation(qint64 project_id, ActProject &project);

  /**
   * @briefGet the Device and Interface Pair object in the link's project
   *
   * @param project[input]
   * @param link[input]
   * @param src_dev[output]
   * @param dst_dev[output]
   * @param src_intf[output]
   * @param dst_intf[output]
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceAndInterfacePairFromProject(const ActProject &project, const ActLink &link, ActDevice &src_dev,
                                                  ActDevice &dst_dev, ActInterface &src_intf, ActInterface &dst_intf);

  /**
   * @brief Get the Device And Interface Pair object in the link
   *
   * @param device_set
   * @param link
   * @param src_dev
   * @param dst_dev
   * @param src_intf
   * @param dst_intf
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceAndInterfacePair(const QSet<ActDevice> &device_set, const ActLink &link, ActDevice &src_dev,
                                       ActDevice &dst_dev, ActInterface &src_intf, ActInterface &dst_intf);

  /****************************
   *  Chamberlain Management  *
   * **************************/

  /**
   * @brief The callback function of chamberlain module
   *
   */
  void StartChamberlainThread();

  /**
   * @brief The start function of chamberlain module
   *
   * @return ACT_STATUS
   */
  ACT_STATUS StartChamberlain();

  /**
   * @brief The stop function of chamberlain module
   *
   * @return
   */
  void StopChamberlain();

  /**
   * @brief Open the Chamberlain Browser
   *
   */
  ACT_STATUS OpenBrowser();

  /*********************************
   *  Monitor intelligent server process  *
   * *******************************/
  void MonitorIntelligentServerProcess();

  /****************************
   *  Worker Management  *
   * **************************/

  /**
   * @brief The callback function of worker module
   *
   */
  void StartWorkerThread(qint8 worker_id);

  /**
   * @brief The start function of worker module
   *
   * @return ACT_STATUS
   */
  ACT_STATUS StartWorkers();

  /**
   * @brief The stop function of worker module
   *
   * @return
   */
  void StopWorkers();

  /**
   * @brief Distribute jobs among worker job queues based on their loading status
   *
   * @param userJobs
   */
  void DistributeWorkerJobs(const QList<ActJob> &jobs);

  /***************
   * MQTT Client *
   **************/

  /**
   * @brief The callback function of MQTT client
   *
   * @param project_id
   * @param ws_listener_id
   */
  void MqttClientThread(const qint64 project_id, const qint64 ws_listener_id);

  /**
   * @brief The callback function of MQTT client
   *
   * @param project_id
   * @param ws_listener_id
   */
  ACT_STATUS StartMqttClient(const qint64 project_id, const qint64 ws_listener_id);

  /**
   * @brief The stop function of MQTT client
   *
   * @return
   */
  void StopMqttClient();

  /****************************
   *  Monitor Process Management  *
   * **************************/

  /**
   * @brief The callback function of monitor process engine
   *
   * @param project_id
   * @param ws_listener_id
   */
  void MonitorProcessThread(const qint64 project_id, const qint64 ws_listener_id);

  /**
   * @brief The callback function of monitor process engine
   *
   * @param project_id
   * @param ws_listener_id
   */
  ACT_STATUS StartMonitorProcessEngine(const qint64 project_id, const qint64 ws_listener_id);

  /**
   * @brief The stop function of monitor process engine
   *
   * @return
   */
  void StopMonitorProcessEngine();

  /**
   * @brief Distribute monitor data in process queue
   *
   * @param data
   */
  void DistributeMonitorData(ActMonitorData &data);

  /**
   * @brief Handle port link up/down trap event
   *
   * @param topic
   * @param message
   * @param sync_to_websocket
   * @param send_tmp
   */
  void HandlePortLinkEvent(const ActMqttEventTopicEnum topic, const ActMqttMessage &message, bool sync_to_websocket,
                           bool send_tmp);
  /**
   * @brief Handle trap message
   *
   * @param message
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleTrapMessage(ActMqttMessage &message, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle ping result
   *
   * @param ping_device
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandlePingResult(ActPingDevice &ping_device, bool sync_to_websocket = true, bool send_tmp = false);

  /**
   * @brief Handle swift status
   *
   * @param ping_device
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleSwiftStatus(ActPingDevice &ping_device, bool sync_to_websocket = true, bool send_tmp = false);

  /**
   * @brief Handle basic status result
   *
   * @param device_basic_status
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleBasicStatusResult(ActMonitorBasicStatus &device_basic_status, bool sync_to_websocket = true,
                                     bool send_tmp = false);

  /**
   * @brief Handle traffic data result
   *
   * @param traffic_view
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleTrafficResult(ActDeviceMonitorTraffic &traffic, bool sync_to_websocket = true,
                                 bool send_tmp = false);

  /**
   * @brief Handle time status result
   *
   * @param time_synchronization_data
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleTimeStatusResult(ActMonitorTimeStatus &time_synchronization_data, bool sync_to_websocket = true,
                                    bool send_tmp = false);

  /**
   * @brief Handle Device Status
   *
   * @param device
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleDeviceConnectionStatus(ActDevice &device, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle Device ModuleConfigAndInterfaces
   *
   * @param device
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleModuleConfigAndInterfaces(ActDevice &device, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle identify Device
   *
   * @param device
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleIdentifyDevice(ActDevice &device, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle AssignScanLink data
   *
   * @param update_device
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleAssignScanLinkData(ActDevice &update_device, bool sync_to_websocket, bool send_tmp);
  /**
   * @brief Handle ManagementEndpoint
   *
   * @param src_device
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleManagementEndpoint(ActSourceDevice &src_device, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle Device ScanLinks
   *
   * @param scan_links_result
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleDeviceScanLinks(ActScanLinksResult &scan_links_result, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle RSTP status
   *
   * @param rstp_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleRSTPResult(ActMonitorRstpStatus &rstp_table, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle VLAN status
   *
   * @param vlan_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleVLANResult(ActVlanTable &vlan_table, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle PortDefaultPCP
   *
   * @param pcp_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandlePortDefaultPCPResult(ActDefaultPriorityTable &pcp_table, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle Network setting
   *
   * @param network_setting_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleNetworkSettingResult(ActNetworkSettingTable &network_setting_table, bool sync_to_websocket,
                                        bool send_tmp);

  /**
   * @brief Handle User Account
   *
   * @param user_account_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleUserAccountResult(ActUserAccountTable &user_account_table, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle Login Policy
   *
   * @param login_policy_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleLoginPolicyResult(ActLoginPolicyTable &login_policy_table, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle SnmpTrap setting
   *
   * @param snmp_trap_setting_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleSnmpTrapSettingResult(ActSnmpTrapSettingTable &snmp_trap_setting_table, bool sync_to_websocket,
                                         bool send_tmp);

  /**
   * @brief Handle Syslog setting
   *
   * @param snmp_trap_setting_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleSyslogSettingResult(ActSyslogSettingTable &syslog_setting_table, bool sync_to_websocket,
                                       bool send_tmp);

  /**
   * @brief Handle Time setting
   *
   * @param time_setting_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleTimeSettingResult(ActTimeSettingTable &time_setting_table, bool sync_to_websocket, bool send_tmp);

  /**
     * @brief Handle Port Time setting

   *
   * @param port_setting_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandlePortSettingResult(ActPortSettingTable &port_setting_table, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle StreamPriority Ingress
   *
   * @param stad_port_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleStreamPriorityIngressResult(ActStadPortTable &stad_port_table, bool sync_to_websocket,
                                               bool send_tmp);

  /**
   * @brief Handle StreamPriority Egress
   *
   * @param stad_config_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleStreamPriorityEgressResult(ActStadConfigTable &stad_config_table, bool sync_to_websocket,
                                              bool send_tmp);
  /**
   * @brief Handle Infomation setting
   *
   * @param info_setting_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleInformationSettingResult(ActInformationSettingTable &info_setting_table, bool sync_to_websocket,
                                            bool send_tmp);

  /**
   * @brief Handle Management Interface
   *
   * @param mgmt_interface_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleManagementInterfaceResult(ActManagementInterfaceTable &mgmt_interface_table, bool sync_to_websocket,
                                             bool send_tmp);

  /**
   * @brief Handle Unicast StaticForward
   *
   * @param static_forward_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleUnicastStaticResult(ActStaticForwardTable &static_forward_table, bool sync_to_websocket,
                                       bool send_tmp);

  /**
   * @brief Handle Multicast StaticForward
   *
   * @param static_forward_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleMulticastStaticResult(ActStaticForwardTable &static_forward_table, bool sync_to_websocket,
                                         bool send_tmp);

  /**
   * @brief Handle TimeAwareShaper
   *
   * @param gcl_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleTimeAwareShaperResult(ActGclTable &gcl_table, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle Loop Protection
   *
   * @param loop_protection_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleLoopProtectionResult(ActLoopProtectionTable &loop_protection_table, bool sync_to_websocket,
                                        bool send_tmp);

  /**
   * @brief Handle TimeSync setting
   *
   * @param time_sync_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleTimeSyncSettingResult(ActTimeSyncTable &time_sync_table, bool sync_to_websocket, bool send_tmp);

  /**
   * @brief Handle RSTP setting
   *
   * @param rstp_table
   * @param sync_to_websocket
   * @param send_tmp
   * @return ACT_STATUS
   */
  ACT_STATUS HandleRstpSettingResult(ActRstpTable &rstp_table, bool sync_to_websocket, bool send_tmp);

  /***********************
   *  System Management  *
   * *********************/

  /**
   * @brief Update system object
   *
   * @param user
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateSystem(ActSystem &system);

  /*********************
   *  User Management  *
   * *******************/

  /**
   * @brief Create a user object
   *
   * @param user
   * @return ACT_STATUS
   */
  ACT_STATUS CreateUser(ActUser &user);

  /**
   * @brief Get an existing user object
   *
   * @param id
   * @param user
   * @return ACT_STATUS
   */
  ACT_STATUS GetUser(qint64 &id, ActUser &user);

  /**
   * @brief Update user object
   *
   * @param user
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateUser(ActUser &user);

  /**
   * @brief Delete an existing user object
   *
   * @param id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteUser(qint64 &id);

  /**
   * @brief Check the user configuration
   *
   * @param user
   * @param allow_empty Allow the password field is empty for update method
   * @return ACT_STATUS
   */
  ACT_STATUS CheckUser(const ActUser &user, const bool allow_password_empty = false);

  /******************
   *  Login/Logout  *
   * ****************/

  /**
   * @brief Login to the system through CLI
   *
   * @param username
   * @param password
   * @return ACT_STATUS
   */
  ACT_STATUS CLILogin(const QString &username, const QString &password);

  /**
   * @brief Check CLI token exist
   *
   * @param user
   * @param token
   * @return ACT_STATUS
   */
  ACT_STATUS CheckCLITokenExist(ActUser &user, QString &token);

  /**
   * @brief Login to the system
   *
   * @param user
   * @param token
   * @return ACT_STATUS
   */
  ACT_STATUS Login(ActUser &user, QString &token);

  /**
   * @brief Renew the token
   *
   * @param orig_token
   * @param new_token
   * @param user
   * @return ACT_STATUS
   */
  ACT_STATUS RenewToken(const QString &orig_token, QString &new_token, ActUser &user);

  /**
   * @brief Logout from the system
   *
   * @param token
   * @return ACT_STATUS
   */
  ACT_STATUS Logout(const QString &token);

  /**
   * @brief Verify the token is valid
   *
   * @param token
   * @param role
   * @return ACT_STATUS
   */
  ACT_STATUS VerifyToken(QString token, ActRoleEnum &role);

  /**
   * @brief Check the hard time out in the token set
   *
   * @return ACT_STATUS
   */
  ACT_STATUS CheckTokenHardTimeout();

  /**
   * @brief Get the User Id By Token object
   *
   * @param token
   * @param user_id
   * @return ACT_STATUS
   */
  ACT_STATUS GetUserIdByToken(const QString &token, qint64 &user_id);

  /************************
   *  Project Management  *
   * **********************/

  /**
   * @brief Export project with related device profiles
   *
   * @param project_id
   * @param exported_project
   * @return ACT_STATUS
   */
  ACT_STATUS ExportProject(qint64 &project_id, ActExportProject &exported_project);

  /**
   * @brief Save project to database by project id
   *
   * @param project_id
   * @return ACT_STATUS
   */
  ACT_STATUS SaveProject(qint64 &project_id);

  /**
   * @brief Save project to database
   *
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS SaveProject(ActProject &project);

  /**
   * @brief Import a project object and related device profile
   *
   * @param imported_project
   * @return ACT_STATUS
   */
  ACT_STATUS ImportProject(ActImportProject &imported_project);

  /**
   * @brief Check the traffic type to pcp mapping
   *
   * @param traffic_type_to_pcp_mapping
   * @param project_name
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProject(ActProject &project, bool sync_to_websocket = false, bool is_operation = false);

  /**
   * @brief Copy an existing project object
   *
   * @param project_id Specify the project id should be copied.
   * @param copied_project
   * @return ACT_STATUS
   */
  ACT_STATUS CopyProject(qint64 &project_id, QString &project_name, ActProject &copied_project);

  /**
   * @brief Clone projet to specified mode
   *
   * @param project_id
   * @param mode
   * @param copied_project
   * @return ACT_STATUS
   */
  ACT_STATUS CloneProjectToOtherMode(qint64 &project_id, const ActProjectModeEnum &mode, ActProject &copied_project);

  /**
   * @brief Create a project object
   *
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS CreateProject(ActProject &project);

  /**
   * @brief Get an existing project object
   *
   * @param project_id
   * @param project
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetProject(const qint64 project_id, ActProject &project, bool is_operation = false);

  /**
   * @brief Get an existing project name
   *
   * @param project_id
   * @return ACT_STATUS
   */
  ACT_STATUS GetProjectName(const qint64 project_id, QString &project_name);

  /**
   * @brief Update project status
   *
   * @param project_id
   * @param status
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProjectStatus(const qint64 &project_id, const ActProjectStatusEnum &status);

  /**
   * @brief Get the Project By UUID From Mode object
   *
   * @param uuid
   * @param mode
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS GetProjectByUUIDFromMode(const QString &uuid, const ActProjectModeEnum &mode, ActProject &project);

  /**
   * @brief Get the Simple Project Set object
   *
   * @param simple_project_set
   * @return ACT_STATUS
   */
  ACT_STATUS GetSimpleProjectSet(QSet<ActSimpleProject> &simple_project_set);

  /**
   * @brief Get the Simple Project object
   *
   * @param project_id
   * @param simple_project
   * @return ACT_STATUS
   */
  ACT_STATUS GetSimpleProject(const qint64 project_id, ActSimpleProject &simple_project);

  /**
   * @brief Delete an existing project object
   *
   * @param id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteProject(qint64 &id);

  /**
   * @brief Handle ProjectSetting ScanIpRanges Field
   *
   * @param update_project_setting
   * @param db_project
   * @return ACT_STATUS
   */
  ACT_STATUS HandleProjectSettingScanIpRangesField(ActProjectSetting &update_project_setting, ActProject &db_project);

  /**
   * @brief Update and Replace ProjectSetting ScanIpRanges by Memory from Wizard
   *
   * @param project_id
   * @return ACT_STATUS
   */
  ACT_STATUS ReplaceProjSettingScanIpRangesByMemoryFromWizard(qint64 project_id, const QList<qint64> &skip_devices);

  /**
   * @brief Update project setting
   *
   * @param project_id
   * @param project_setting
   * @param is_operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProjectSetting(qint64 &project_id, ActProjectSetting &project_setting, bool is_operation = false);

  /**
   * @brief Update project setting
   *
   * @param project
   * @param project_setting
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProjectSetting(ActProject &project, ActProjectSetting &project_setting);

  /**
   * @brief Update member data of the project setting
   *
   * @param project_setting_member
   * @param project_id
   * @param update_data
   * @param is_operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProjectSettingMember(const ActProjectSettingMember &project_setting_member, qint64 &project_id,
                                        ActProjectSetting &update_data, bool is_operation = false);

  /**
   * @brief Update member data of the project setting
   *
   * @param project_setting_member
   * @param project
   * @param update_data
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProjectSettingMember(const ActProjectSettingMember &project_setting_member, ActProject &project,
                                        ActProjectSetting &update_data);

  /**
   * @brief Check the project setting feasibility
   *
   * @param project_setting
   * @param check_account
   * @return ACT_STATUS
   */
  ACT_STATUS CheckProjectSetting(const ActProjectSetting &project_setting, bool check_account);

  /**
   * @brief Check the project name
   *
   * @param project_name
   * @return ACT_STATUS
   */
  ACT_STATUS CheckProjectName(const QString &project_name);

  /**
   * @brief Check the project setting feasibility
   *

   * @param algorithm_config
   * @param project_name
   * @return ACT_STATUS
   */
  ACT_STATUS CheckAlgorithmConfiguration(const ActAlgorithmConfiguration &algorithm_config, QString project_name = "");

  /**
   * @brief Check the vlan range
   *
   * @param vlan_range
   * @param project_name
   * @return ACT_STATUS
   */
  ACT_STATUS CheckVlanRange(const ActVlanRange &vlan_range, QString project_name = "");

  /**
   * @brief Check TrafficType to PCP Mapping
   *
   * @param traffic_type_to_pcp_mapping
   * @param project_name
   * @return ACT_STATUS
   */
  ACT_STATUS CheckTrafficTypeToPCPMapping(const ActTrafficTypeToPriorityCodePointMapping &traffic_type_to_pcp_mapping,
                                          QString project_name = "");
  /**
   * @brief Check the Scan IP ranges
   *
   * @param scan_ip_ranges
   * @param project_name
   * @return ACT_STATUS
   */
  ACT_STATUS CheckScanIpRanges(const QList<ActScanIpRangeEntry> &scan_ip_ranges, QString project_name = "");

  /**
   * @brief Check the Project Start IP
   *
   * @param start_ip
   * @param project_name
   * @return ACT_STATUS
   */
  ACT_STATUS CheckProjectStartIP(const QString &start_ip, QString project_name);

  /**
   * @brief Check the monitor configuration
   *
   * @param config
   * @param project_name
   * @return ACT_STATUS
   */
  ACT_STATUS CheckMonitorConfiguration(ActMonitorConfiguration &config, QString project_name);

  /**
   * @brief Check the project feasibility
   *
   * @param project
   * @param check_feasibility check the feasibility of stream
   * @return ACT_STATUS
   */
  ACT_STATUS CheckProject(ActProject &project, bool check_feasibility = true);

  /**
   * @brief Create a group object
   *
   * @param project_id
   * @param group
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS CreateGroup(const qint64 project_id, ActGroup &group, bool is_operation = false);

  /**
   * @brief Create a group object
   *
   * @param project
   * @param group
   * @return ACT_STATUS
   */
  ACT_STATUS CreateGroup(ActProject &project, ActGroup &group);

  /**
   * @brief Check the group object
   *
   * @param project
   * @param group
   * @return ACT_STATUS
   */
  ACT_STATUS CheckGroup(ActProject &project, ActGroup &group);

  /**
   * @brief Update a group object
   *
   * @param project_id
   * @param group
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateGroup(const qint64 project_id, ActGroup &group, bool is_operation = false);

  /**
   * @brief Update a group object
   *
   * @param project
   * @param group
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateGroup(ActProject &project, ActGroup &group);

  /**
   * @brief Delete a group object and its devices
   *
   * @param project_id
   * @param group_id
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteGroup(const qint64 project_id, const qint64 group_id, bool is_operation = false);

  /**
   * @brief Delete a group object and its devices
   *
   * @param project
   * @param group_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteGroup(ActProject &project, const qint64 group_id);

  /**
   * @brief Ungroup object (move content to parent)
   *
   * @param project_id
   * @param group_id
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UnGroup(const qint64 project_id, const qint64 group_id, bool is_operation = false);

  /**
   * @brief Ungroup a group object (move content to parent)
   *
   * @param project
   * @param group_id
   * @return ACT_STATUS
   */
  ACT_STATUS UnGroup(ActProject &project, const qint64 group_id);

  /**
   * @brief Auto remove the project which is expired
   *
   * @return ACT_STATUS
   */
  ACT_STATUS CheckServerProjectExpired();

  /*******************************
   *  Connection Config Management  *
   * *****************************/

  /**
   * @brief Check the NETCONF configuration
   *
   * @param account
   * @param item_name
   * @return ACT_STATUS
   */
  ACT_STATUS CheckDeviceAccount(const ActDeviceAccount &account, QString item_name = "");

  /**
   * @brief Check the NETCONF configuration
   *
   * @param netconf_config
   * @param item_name
   * @return ACT_STATUS
   */
  ACT_STATUS CheckNetconfConfiguration(const ActNetconfConfiguration &netconf_config, QString item_name = "");

  /**
   * @brief Check the RESTful configuration
   *
   * @param restful_config
   * @param item_name
   * @return ACT_STATUS
   */
  ACT_STATUS CheckRestfulConfiguration(const ActRestfulConfiguration &restful_config, QString item_name = "");

  /**
   * @brief Check the SNMP configuration
   *
   * @param snmp_config
   * @param item_name
   * @return ACT_STATUS
   */
  ACT_STATUS CheckSnmpConfiguration(const ActSnmpConfiguration &snmp_config, QString item_name = "");

  /**
   * @brief Check the SNMP trap configuration
   *
   * @param snmp_trap_config
   * @param item_name
   * @return ACT_STATUS
   */
  ACT_STATUS CheckSnmpTrapConfiguration(const ActSnmpTrapConfiguration &snmp_trap_config, QString item_name = "");

  /**
   * @brief Handle Device connection account
   *
   * @param target_account
   * @param old_account
   * @param default_account
   * @return ACT_STATUS
   */
  ACT_STATUS HandleDeviceAccount(ActDeviceAccount &target_account, const ActDeviceAccount &old_account,
                                 const ActDeviceAccount &default_account);

  /**
   * @brief Handle NetconfConfiguration
   *
   * @param target_netconf_config
   * @param default_netconf_config
   * @return ACT_STATUS
   */
  ACT_STATUS HandleNetconfConfiguration(ActNetconfConfiguration &target_netconf_config,
                                        const ActNetconfConfiguration &default_netconf_config);

  /**
   * @brief Handle RestfulConfiguration
   *
   * @param target_restful_config
   * @param default_restful_config
   * @return ACT_STATUS
   */
  ACT_STATUS HandleRestfulConfiguration(ActRestfulConfiguration &target_restful_config,
                                        const ActRestfulConfiguration &default_restful_config);

  /**
   * @brief Handle SnmpConfiguration
   *
   * @param target_snmp_config
   * @param old_snmp_config
   * @param default_snmp_config
   * @return ACT_STATUS
   */
  ACT_STATUS HandleSnmpConfiguration(ActSnmpConfiguration &target_snmp_config,
                                     const ActSnmpConfiguration &old_snmp_config,
                                     const ActSnmpConfiguration &default_snmp_config);

  /**
   * @brief Sync account default configuration of Project
   *
   * @param account
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS SyncDeviceAccountDefault(const ActDeviceAccount &account, ActProject &project);

  /**
   * @brief Sync Netconf default configuration of Project
   *
   * @param netconf_config
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS SyncNetconfDefaultConfiguration(const ActNetconfConfiguration &netconf_config, ActProject &project);

  /**
   * @brief Sync Restful default configuration of Project
   *
   * @param restful_config
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS SyncRestfulDefaultConfiguration(const ActRestfulConfiguration &restful_config, ActProject &project);

  /**
   * @brief Sync Snmp default configuration of Project
   *
   * @param snmp_config
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS SyncSnmpDefaultConfiguration(const ActSnmpConfiguration &snmp_config, ActProject &project);

  /**
   * @brief Get the Project Data Version
   *
   * @param project_id
   * @param socket
   * @return ACT_STATUS
   */
  ACT_STATUS GetProjectDataVersion(qint64 &project_id, const qint64 &ws_listener_id);

  /*******************************
   *  Management Interfaces      *
   * *****************************/

  /**
   * @brief Check Management Interface object
   *
   * @param project
   * @param management_interface
   * @return ACT_STATUS
   */
  ACT_STATUS CheckManagementInterface(const ActProject &project, const ActManagementInterface &management_interface);

  /**
   * @brief Get the Management Interface object
   *
   * @param project_id
   * @param device_id
   * @param management_interface
   * @return ACT_STATUS
   */
  ACT_STATUS GetManagementInterface(qint64 &project_id, qint64 &device_id,
                                    ActManagementInterface &management_interface);

  /**
   * @brief Create a Management Interface object
   *
   * @param project_id
   * @param management_interface
   * @return ACT_STATUS
   */
  ACT_STATUS CreateManagementInterface(qint64 &project_id, ActManagementInterface &management_interface);

  /**
   * @brief Create a Management Interface object
   *
   * @param project
   * @param management_interface
   * @return ACT_STATUS
   */
  ACT_STATUS CreateManagementInterface(ActProject &project, ActManagementInterface &management_interface);

  /**
   * @brief Update a Management Interface object
   *
   * @param project_id
   * @param management_interface
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateManagementInterface(qint64 &project_id, ActManagementInterface &management_interface);

  /**
   * @brief Update a Management Interface object
   *
   * @param project
   * @param management_interface
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateManagementInterface(ActProject &project, ActManagementInterface &management_interface);

  /**
   * @brief Delete the Management Interface object
   *
   * @param project_id
   * @param device_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteManagementInterface(qint64 &project_id, const qint64 &device_id);

  /**
   * @brief Delete the Management Interface object
   *
   * @param project
   * @param device_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteManagementInterface(ActProject &project, const qint64 &device_id);

  /*******************************
   *  Websocket Listener   *
   * *****************************/

  /**
   * @brief Check the Websocket listener could be create
   *
   * @param ws_listener
   * @return ACT_STATUS
   */
  ACT_STATUS CheckWSConnect(qint64 &project_id);

  /**
   * @brief Add the Websocket listener
   *
   * @param ws_listener
   * @return ACT_STATUS
   */
  ACT_STATUS AddWSListener(const std::shared_ptr<WSListener> &ws_listener);

  /**
   * @brief Remove the Websocket listener
   *
   * @param id
   * @return ACT_STATUS
   */
  ACT_STATUS RemoveWSListener(const qint64 &id);

  /**
   * @brief Send message to Websocket listener
   *
   * @param id
   * @param message
   * @return ACT_STATUS
   */
  ACT_STATUS SendMessageToWSListener(const qint64 &id, const QString &message);

  /**
   * @brief Send message to all Websocket listeners
   *
   * @param message
   * @return ACT_STATUS
   */
  ACT_STATUS SendMessageToAllWSListeners(const QString &message);

  /**
   * @brief Send message to System Websocket listeners
   *
   * @param message
   * @return ACT_STATUS
   */
  ACT_STATUS SendMessageToSystemWSListeners(const QString &message);

  /**
   * @brief Send message to System Websocket listeners
   *
   * @param message
   * @return ACT_STATUS
   */
  ACT_STATUS SendMessageToProjectWSListeners(const qint64 &project_id, const QString &message);

  /*************************************
   *  WS Notification Temp Management  *
   * ***********************************/

  /**
   * @brief Init the WS Notification Message temp
   *
   * @return ACT_STATUS
   */
  ACT_STATUS InitNotificationTmp();

  /**
   * @brief Insert Link WS msg to core's Notification Tmp
   *
   * @param ActLinkWSNotificationMsg
   * @return ACT_STATUS
   */
  ACT_STATUS InsertLinkMsgToNotificationTmp(const ActLinkPatchUpdateMsg &update_msg);

  /**
   * @brief Insert Device msg to core's Notification Tmp
   *
   * @param update_msg
   * @return ACT_STATUS
   */
  ACT_STATUS InsertDeviceMsgToNotificationTmp(const ActDevicePatchUpdateMsg &update_msg);

  /**
   * @brief Insert Stream WS msg to core's Notification Tmp
   *
   * @param update_msg
   * @return ACT_STATUS
   */
  ACT_STATUS InsertStreamMsgToNotificationTmp(const ActStreamPatchUpdateMsg &update_msg);

  /**
   * @brief Send all Notification temp messages and clear temp
   *
   * @param project_id
   * @return ACT_STATUS
   */
  ACT_STATUS SendNotificationTmpMsgs(const qint64 &project_id);

  /**********************
   *  Service Platform  *
   **********************/

  /**
   * @brief Login to the service platform
   *
   * @param user_id
   * @param login_req
   * @param login_res
   *
   * @return ACT_STATUS
   */
  ACT_STATUS ServicePlatformLogin(const qint64 &user_id, const ActServicePlatformLoginRequest &login_req,
                                  ActServicePlatformLoginResponse &login_res);

  /**
   * @brief Check the service platform token
   *
   * @param user_id
   * @param status
   * @return ACT_STATUS
   */
  ACT_STATUS CheckServicePlatformToken(const qint64 &user_id, ActServicePlatformLoginCheck &status);

  /**
   * @brief Register project to the service platform
   *
   * @param user_id
   * @param project_id
   * @return ACT_STATUS
   */
  ACT_STATUS ServicePlatformRegister(const qint64 &user_id, const qint64 &project_id);

  /**
   * @brief Get the Service Platform Token Map object
   *
   * @param service_platform_token_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetServicePlatformTokenMap(QMap<qint64, QString> &service_platform_token_map);

  /**
   * @brief Set the Service Platform Token Map object
   *
   * @param service_platform_token_map
   * @return ACT_STATUS
   */
  ACT_STATUS SetServicePlatformTokenMap(const QMap<qint64, QString> &service_platform_token_map);

  /**
   * @brief Get the Service Platform Token object
   *
   * @param user_id
   * @param token
   * @return ACT_STATUS
   */
  ACT_STATUS GetServicePlatformToken(const qint64 &user_id, QString &token);

  /**
   * @brief Update the service platform token
   *
   * @param user_id
   * @param token
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateServicePlatformToken(const qint64 &user_id, const QString &token);

  /**
   * @brief Delete the service platform token
   *
   * @param user_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteServicePlatformToken(const qint64 &user_id);

  /********************************
   *  Service Profile Management  *
   * ******************************/

  /**
   * @brief Init the general profile
   *
   * @return ACT_STATUS
   */
  ACT_STATUS InitGeneralProfiles();

  /**
   * @brief Update the SKU quantities
   *
   * @param project_id
   * @param sku_quantity_request
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateSkuQuantity(const qint64 &project_id, ActUpdateSkuQuantityRequest sku_quantity_request);

  /**
   * @brief Update the SKU quantities
   *
   * @param project
   * @param sku_quantity_request
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateSkuQuantity(ActProject &project, ActUpdateSkuQuantityRequest sku_quantity_request);

  /**
   * @brief Update the SKU quantities by device added
   *
   * @param project
   * @param device
   * @param from_bag
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateSkuQuantityByAddDevice(ActProject &project, const ActDevice &device, const bool from_bag);

  /**
   * @brief Update the SKU quantities by device updated
   *
   * @param project
   * @param old_device
   * @param new_device
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateSkuQuantityByUpdateDevice(ActProject &project, const ActDevice &old_device,
                                             const ActDevice &new_device);

  /**
   * @brief Update the SKU quantities by device deleted
   *
   * @param project
   * @param device
   * @param to_bag
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateSkuQuantityByDeleteDevice(ActProject &project, const ActDevice &device, const bool to_bag);

  /**
   * @brief Delete the SKU quantities
   *
   * @param project_id
   * @param sku_list
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteSkuQuantity(const qint64 &project_id, ActSkuList &sku_list);

  /**
   * @brief Delete the SKU quantities
   *
   * @param project
   * @param sku_list
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteSkuQuantity(ActProject &project, const ActSkuList &sku_list);

  /**
   * @brief Get the Sku Quantities object
   *
   * @param project_id
   * @param sku_quantities
   * @return ACT_STATUS
   */
  ACT_STATUS GetSkuQuantities(const qint64 &project_id, ActSkuQuantities &sku_quantities);

  /**
   * @brief Update the SKU quantities and price in projects
   *
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateSkuQuantityPriceInProjects();

  /*******************************
   *  Host  *
   * *****************************/

  /**
   * @brief Get the Host Adapter Table object
   *
   * @param adapter_list
   * @return ACT_STATUS
   */
  ACT_STATUS GetHostAdapters(ActHostAdapterList &adapter_list);

  /*******************************
   *  Device Profile Management  *
   * *****************************/

  /**
   * @brief Initial the device profile set
   *
   * @return ACT_STATUS
   */
  ACT_STATUS InitDeviceProfiles();

  /**
   * @brief Initial the power device profile set
   *
   * @return ACT_STATUS
   */
  ACT_STATUS InitPowerDeviceProfiles();

  /**
   * @brief Initial the software license profile set
   *
   * @return ACT_STATUS
   */
  ACT_STATUS InitSoftwareLicenseProfiles();

  /**
   * @brief Upload a device profile object
   *
   * @param device_profile
   * @return ACT_STATUS
   */
  ACT_STATUS UploadDeviceProfile(ActDeviceProfile &device_profile);

  /**
   * @brief Delete an existing device profile
   *
   * @param id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteDeviceProfile(qint64 &id);

  /**
   * @brief Initial the default device profile set
   *
   * @return ACT_STATUS
   */
  ACT_STATUS InitDefaultDeviceProfiles();

  /**
   * @brief Update the specified device profile
   *
   * @param device_profile
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceProfile(ActDeviceProfile &device_profile);

  /*******************************
   *  Feature Profile Management  *
   * *****************************/

  /**
   * @brief Initial the feature profile set
   *
   * @return ACT_STATUS
   */
  ACT_STATUS InitFeatureProfiles();

  /*****************************************
   *  Firmware Feature Profile Management  *
   * ***************************************/

  /**
   * @brief Initial the firmware feature profile set
   *
   * @return ACT_STATUS
   */
  ACT_STATUS InitFirmwareFeatureProfiles();

  /*******************************
   *  Device Icon  *
   * *****************************/
  /**
   * @brief Update the device icon of the specific device profile
   *
   * @param device_profile_id
   * @param icon_name
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceProfileIcon(qint64 &device_profile_id, QString &icon_name);

  /******************************
   *  Device Module Management  *
   *****************************/

  /**
   * @brief Init the ethernet module
   *
   * @return ACT_STATUS
   */
  ACT_STATUS InitEthernetModules();

  /**
   * @brief Init the SFP module
   *
   * @return ACT_STATUS
   */
  ACT_STATUS InitSFPModules();

  /**
   * @brief Init the power module
   *
   * @return ACT_STATUS
   */
  ACT_STATUS InitPowerModules();

  /**
   * @brief Get the Ethernet Module Name ID Map object
   *
   * @param eth_module_name_id_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetEthernetModuleNameIdMap(QMap<QString, qint64> &eth_module_name_id_map);

  /**
   * @brief Get the SFP Module Name ID Map object
   *
   * @param sfp_module_name_id_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetSFPModuleNameIdMap(QMap<QString, qint64> &sfp_module_name_id_map);

  /**
   * @brief Get the Power Module Name ID Map object
   *
   * @param power_module_name_id_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetPowerModuleNameIdMap(QMap<QString, qint64> &power_module_name_id_map);

  /***********************
   *  Device Management  *
   * *********************/

  /**
   * @brief Match the device and device profile to assign device properties
   *
   * @param project
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS MatchDeviceProfile(ActProject &project, ActDevice &device);

  /**
   * @brief Create a device object
   *
   * @param project_id
   * @param device
   * @param from_bag
   * @param is_operation (default: false)
   * @return ACT_STATUS
   */
  ACT_STATUS CreateDevice(qint64 &project_id, ActDevice &device, const bool from_bag, bool is_operation = false);

  /**
   * @brief Create a device object to a specific project object
   *
   * @param project
   * @param device
   * @param from_bag
   * @return ACT_STATUS
   */
  ACT_STATUS CreateDevice(ActProject &project, ActDevice &device, const bool from_bag);

  /**
   * @brief Create device objects at once
   *
   * @param project_id
   * @param device_list
   * @param from_bag
   * @param created_device_ids
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS CreateDevices(qint64 &project_id, QList<ActDevice> &device_list, const bool from_bag,
                           QList<qint64> &created_device_ids, bool is_operation = false);

  /**
   * @brief Create device objects to a specific project object at once
   *
   * @param project
   * @param device_list
   * @param from_bag
   * @param created_device_ids
   * @return ACT_STATUS
   */
  ACT_STATUS CreateDevices(ActProject &project, QList<ActDevice> &device_list, const bool from_bag,
                           QList<qint64> &created_device_ids);

  /**
   * @brief Get an existing device object in specific project
   *
   * @param project_id
   * @param device_id
   * @param device
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDevice(const qint64 &project_id, const qint64 &device_id, ActDevice &device, bool is_operation = false);

  /**
   * @brief Update device object
   *
   * @param project_id
   * @param device
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDevice(qint64 &project_id, ActDevice &device, bool is_operation = false);

  /**
   * @brief Update device object in a specific project object
   *
   * @param project
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDevice(ActProject &project, ActDevice &device);

  /**
   * @brief Update device objects
   *
   * @param project_id
   * @param device_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDevices(qint64 &project_id, QList<ActDevice> &device_list, bool is_operation = false);

  /**
   * @brief Update device objects in a specific project object
   *
   * @param project
   * @param device_list
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDevices(ActProject &project, QList<ActDevice> &device_list);

  /**
   * @brief Update multiple device coordinate
   *
   * @param project_id
   * @param device_set
   * @param sync_to_websocket
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceCoordinates(qint64 &project_id, QSet<ActDeviceCoordinate> &device_set, bool sync_to_websocket,
                                     bool is_operation = false);

  /**
   * @brief Update multiple device coordinate
   *
   * @param project
   * @param device_set
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceCoordinates(ActProject &project, QSet<ActDeviceCoordinate> &device_set);

  /**
   * @brief Delete an existing device object
   *
   * @param project_id
   * @param device_id
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteDevice(qint64 &project_id, qint64 &device_id, bool is_operation = false);

  /**
   * @brief Delete an existing device object in a specific project object
   *
   * @param project
   * @param device_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteDevice(ActProject &project, qint64 &device_id);

  /**
   * @brief Delete select device objects
   *
   * @param project_id
   * @param dev_ids
   * @param to_bag
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteDevices(qint64 &project_id, QList<qint64> &dev_ids, const bool to_bag, bool is_operation = false);

  /**
   * @brief Delete select device objects in a specific project object
   *
   * @param project
   * @param dev_ids
   * @param to_bag
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteDevices(ActProject &project, QList<qint64> &dev_ids, const bool to_bag);

  /**
   * @brief Check the device feasibility
   *
   * @param project
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS CheckDevice(ActProject &project, ActDevice &device);

  /*********************
   *     Device Bag    *
   * *******************/

  /**
   * @brief Get device bag
   *
   * @param project_id
   * @param device_bag
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceBag(qint64 &project_id, ActDeviceBag &device_bag);

  /**
   * @brief Get device bag in a specific project object
   *
   * @param project
   * @param device_bag
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceBag(ActProject &project, ActDeviceBag &device_bag);

  /*********************
   *  Link Management  *
   * *******************/

  /**
   * @brief Check the link feasibility
   *
   * @param project
   * @param link
   * @return ACT_STATUS
   */
  ACT_STATUS CheckLink(ActProject &project, ActLink &link);

  /**
   * @brief Create a link object
   *
   * @param project_id
   * @param link
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS CreateLink(qint64 &project_id, ActLink &link, bool is_operation = false);

  /**
   * @brief Create a link object to a specific project object
   *
   * @param project
   * @param link
   * @param assign_id Specify the link id should be assigned. If the link is create via auto scan topology module, the
   * id should be assigned before
   * @return ACT_STATUS
   */
  ACT_STATUS CreateLink(ActProject &project, ActLink &link);

  /**
   * @brief Create multiple link objects
   *
   * @param project_id
   * @param link_list
   * @param created_link_ids
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS CreateLinks(qint64 &project_id, QList<ActLink> &link_list, QList<qint64> &created_link_ids,
                         bool is_operation = false);

  /**
   * @brief Create multiple link objects to a specific project object
   *
   * @param project
   * @param link_list
   * @param created_link_ids
   * @param assign_id Specify the link id should be assigned. If the link is create via auto scan topology module, the
   * id should be assigned before
   * @return ACT_STATUS
   */
  ACT_STATUS CreateLinks(ActProject &project, QList<ActLink> &link_list, QList<qint64> &created_link_ids);

  /**
   * @brief Get an existing link object in specific project
   *
   * @param project_id
   * @param link_id
   * @param link
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetLink(qint64 &project_id, qint64 &link_id, ActLink &link, bool is_operation = false);

  /**
   * @brief Update link object
   *
   * @param project_id
   * @param link
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateLink(qint64 &project_id, ActLink &link, bool is_operation = false);

  /**
   * @brief Update link object in a specific project object
   *
   * @param project
   * @param link
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateLink(ActProject &project, ActLink &link);

  /**
   * @brief Update link objects
   *
   * @param project_id
   * @param link_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateLinks(qint64 &project_id, QList<ActLink> &link_list, bool is_operation = false);

  /**
   * @brief Update link objects in a specific project object
   *
   * @param project
   * @param link_list
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateLinks(ActProject &project, QList<ActLink> &link_list);

  /**
   * @brief Delete an existing link object
   *
   * @param project_id
   * @param link_id
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteLink(qint64 &project_id, qint64 &link_id, bool is_operation = false);

  /**
   * @brief Delete an existing link object in a specific project object
   *
   * @param project
   * @param link_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteLink(ActProject &project, qint64 &link_id);

  /**
   * @brief Delete existing link objects at once
   *
   * @param project_id
   * @param link_ids
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteLinks(qint64 &project_id, QList<qint64> &link_ids, bool is_operation = false);

  /**
   * @brief Delete existing links object in a specific project object at once
   *
   * @param project
   * @param link_ids
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteLinks(ActProject &project, QList<qint64> &link_ids);

  /***********************
   *  Stream Management  *
   * *********************/
  /**
   * @brief Check the stream feasibility
   *
   * @param project
   * @param stream
   * @param check_feasibility check the feasibility of stream
   * @return ACT_STATUS
   */
  ACT_STATUS CheckStream(ActProject &project, ActStream &stream, bool check_feasibility = true);

  /**
   * @brief Create a stream object
   *
   * @param project_id
   * @param stream
   * @return ACT_STATUS
   */
  ACT_STATUS CreateStream(qint64 &project_id, ActStream &stream);

  /**
   * @brief Create a stream object to a specific project object
   *
   * @param project
   * @param stream
   * @return ACT_STATUS
   */
  ACT_STATUS CreateStream(ActProject &project, ActStream &stream);

  /**
   * @brief Create multiple stream objects
   *
   * @param project_id
   * @param stream_list
   * @param created_stream_ids
   * @return ACT_STATUS
   */
  ACT_STATUS CreateStreams(qint64 &project_id, QList<ActStream> &stream_list, QSet<qint64> &created_stream_ids);

  /**
   * @brief Create a stream object to a specific project object
   *
   * @param project
   * @param stream_list
   * @param created_stream_ids
   * @return ACT_STATUS
   */
  ACT_STATUS CreateStreams(ActProject &project, QList<ActStream> &stream_list, QSet<qint64> &created_stream_ids);

  /**
   * @brief Get an existing stream object in specific project
   *
   * @param project_id
   * @param stream_id
   * @param stream
   * @return ACT_STATUS
   */
  ACT_STATUS GetStream(qint64 &project_id, qint64 &stream_id, ActStream &stream);

  /**
   * @brief Update stream object
   *
   * @param project_id
   * @param stream
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateStream(qint64 &project_id, ActStream &stream);

  /**
   * @brief Update stream object in a specific project object
   *
   * @param project
   * @param stream
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateStream(ActProject &project, ActStream &stream);

  /**
   * @brief Update stream objects
   *
   * @param project_id
   * @param stream_list
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateStreams(qint64 &project_id, QList<ActStream> &stream_list);

  /**
   * @brief Update stream objects in a specific project object
   *
   * @param project
   * @param stream_list
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateStreams(ActProject &project, QList<ActStream> &stream_list);

  /**
   * @brief Delete an existing stream object
   *
   * @param project_id
   * @param stream_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteStream(qint64 &project_id, qint64 &stream_id);

  /**
   * @brief Delete an existing stream object in a specific project object
   *
   * @param project
   * @param stream_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteStream(ActProject &project, qint64 &stream_id);

  /**
   * @brief Delete existing stream objects
   *
   * @param project_id
   * @param stream_ids
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteStreams(qint64 &project_id, QList<qint64> &stream_ids);

  /**
   * @brief Delete existing stream objects in a specific project object
   *
   * @param project
   * @param stream_ids
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteStreams(ActProject &project, QList<qint64> &stream_ids);

  /**
   * @brief Update the stream status of all the streams in the project
   *
   * @param project
   * @param status
   * @return ActStatus
   */
  ACT_STATUS UpdateAllStreamStatus(ActProject &project, ActStreamStatusEnum status);

  /***************************
   *  Ini Config Management  *
   * ************************/

  /**
   * @brief Generate the DeviceIniConfig Zip file
   *
   * @param project_id
   * @param zip_file
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateDeviceIniConfigZipFile(qint64 &project_id, ActDeviceBackupFile &zip_file);

  /**
   * @brief Check device_offline_config_file_map[deviceId] has value (generated offline config)
   *
   * @param project
   * @param dev_id_list
   * @param device_offline_config_file_map
   * @return ACT_STATUS
   */
  ACT_STATUS CheckDeviceConfigValuesNotEmpty(const ActProject &project, const QList<qint64> &dev_id_list,
                                             const ActDeviceOfflineConfigFileMap &device_offline_config_file_map);

  /**
   * @brief Generate the Deploy DeviceIniConfig file at the DesignBaseline
   *
   * @param project_id
   * @param design_baseline_id
   * @param dev_id_list
   * @param device_offline_config_file_map
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateDesignBaselineDeployDeviceIniConfigFile(
      const qint64 &project_id, const qint64 &design_baseline_id, const QList<qint64> &dev_id_list,
      ActDeviceOfflineConfigFileMap &device_offline_config_file_map);

  /**
   * @brief Generate the Device Ini config file
   *
   * @param project
   * @param dev_id_list
   * @param device_offline_config_file_map
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateDeviceIniConfigFile(const ActProject &project, const QList<qint64> &dev_id_list,
                                         ActDeviceOfflineConfigFileMap &device_offline_config_file_map);

  /**
   * @brief Export and unzip the config zip file
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS ExportAndUnzipConfigFile(const ActProject &project, const QList<qint64> &dev_id_list,
                                      ActDeviceOfflineConfigFileMap &device_offline_config_file_map);

  /************
   *  Module  *
   * **********/

  /**
   * @brief Update SFP counts
   *
   * @param project_id
   * @param sfp_counts
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateSFPCounts(qint64 &project_id, QMap<QString, quint32> sfp_counts);

  /**
   * @brief Patch SFP counts
   *
   * @param project_id
   * @param sfp_counts
   * @return ACT_STATUS
   */
  ACT_STATUS PatchSFPCounts(qint64 &project_id, QMap<QString, quint32> sfp_counts);

  /**
   * @brief Patch Clear SFP counts
   *
   * @param project_id
   * @return ACT_STATUS
   */
  ACT_STATUS ClearSFPCounts(qint64 &project_id);

  /************************
   *  Device Setting  *
   * *********************/

  /**
   * @brief Get Device Information object
   *
   * @param project_id
   * @param device_id
   * @param device_information_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceInformation(qint64 &project_id, qint64 &device_id, ActDeviceInformation &device_information,
                                  bool is_operation = false);

  /**
   * @brief Get Device Information object
   *
   * @param project_id
   * @param device_information_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceInformations(qint64 &project_id, ActDeviceInformationList &device_information_list,
                                   bool is_operation = false);

  /**
   * @brief Update Device Information object
   *
   * @param project_id
   * @param device_information_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceInformations(qint64 &project_id, ActDeviceInformationList &device_information_list,
                                      bool is_operation = false);

  /**
   * @brief Get Device Login Policy object
   *
   * @param project_id
   * @param device_id
   * @param device_login_policy_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceLoginPolicy(qint64 &project_id, qint64 &device_id, ActDeviceLoginPolicy &device_login_policy,
                                  bool is_operation = false);

  /**
   * @brief Get Device Login Policy object
   *
   * @param project_id
   * @param device_login_policy_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceLoginPolicies(qint64 &project_id, ActDeviceLoginPolicyList &device_login_policy_list,
                                    bool is_operation = false);

  /**
   * @brief Update Device Login Policy object
   *
   * @param project_id
   * @param device_login_policy_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceLoginPolicies(qint64 &project_id, ActDeviceLoginPolicyList &device_login_policy_list,
                                       bool is_operation = false);

  /**
   * @brief Get Device Loop Protection object
   *
   * @param project_id
   * @param device_id
   * @param device_loop_protection_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceLoopProtection(qint64 &project_id, qint64 &device_id,
                                     ActDeviceLoopProtection &device_loop_protection, bool is_operation = false);

  /**
   * @brief Get Device Loop Protection object
   *
   * @param project_id
   * @param device_loop_protection_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceLoopProtections(qint64 &project_id, ActDeviceLoopProtectionList &device_loop_protection_list,
                                      bool is_operation = false);

  /**
   * @brief Update Device Loop Protection object
   *
   * @param project_id
   * @param device_loop_protection_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceLoopProtections(qint64 &project_id, ActDeviceLoopProtectionList &device_loop_protection_list,
                                         bool is_operation = false);

  /**
   * @brief Get Device Syslog Setting object
   *
   * @param project_id
   * @param device_id
   * @param device_syslog_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceSyslogSetting(qint64 &project_id, qint64 &device_id,
                                    ActDeviceSyslogSetting &device_syslog_setting, bool is_operation = false);

  /**
   * @brief Get Device Syslog Setting object
   *
   * @param project_id
   * @param device_syslog_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceSyslogSettings(qint64 &project_id, ActDeviceSyslogSettingList &device_syslog_setting_list,
                                     bool is_operation = false);

  /**
   * @brief Update Device Syslog Setting object
   *
   * @param project_id
   * @param device_syslog_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceSyslogSettings(qint64 &project_id, ActDeviceSyslogSettingList &device_syslog_setting_list,
                                        bool is_operation = false);

  /**
   * @brief Get Device SNMP Trap Setting object
   *
   * @param project_id
   * @param device_id
   * @param device_snmp_trap_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceSnmpTrapSetting(qint64 &project_id, qint64 &device_id,
                                      ActDeviceSnmpTrapSetting &device_snmp_trap_setting, bool is_operation = false);

  /**
   * @brief Get Device SNMP Trap Setting object
   *
   * @param project_id
   * @param device_snmp_trap_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceSnmpTrapSettings(qint64 &project_id, ActDeviceSnmpTrapSettingList &device_snmp_trap_setting_list,
                                       bool is_operation = false);

  /**
   * @brief Update Device SNMP Trap Setting object
   *
   * @param project_id
   * @param device_snmp_trap_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceSnmpTrapSettings(qint64 &project_id,
                                          ActDeviceSnmpTrapSettingList &device_snmp_trap_setting_list,
                                          bool is_operation = false);

  /**
   * @brief Get Device Backup Setting object
   *
   * @param project_id
   * @param device_backup_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceBackups(qint64 &project_id, ActDeviceBackupSettingList &device_backup_setting_list,
                              bool is_operation = false);

  /**
   * @brief Import Device Backup File object
   *
   * @param project_id
   * @param device_backup_file
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS ImportDeviceBackupFile(qint64 &project_id, qint64 &device_id, ActDeviceBackupFile &device_backup_file,
                                    bool is_operation = false);

  /**
   * @brief Export Device Backup file object
   *
   * @param project_id
   * @param device_backup_file
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS ExportDeviceBackupFile(qint64 &project_id, ActDeviceBackupFile &device_backup_file,
                                    bool is_operation = false);

  /**
   * @brief Get Device VLAN Setting object
   *
   * @param project_id
   * @param device_id
   * @param device_vlan_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceVlanSetting(qint64 &project_id, qint64 &device_id, ActDeviceVlanSetting &device_vlan_setting,
                                  bool is_operation = false);

  /**
   * @brief Get Device VLAN Setting object
   *
   * @param project_id
   * @param device_vlan_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceVlanSettings(qint64 &project_id, ActDeviceVlanSettingList &device_vlan_setting_list,
                                   bool is_operation = false);

  /**
   * @brief Update Device VLAN Setting object
   *
   * @param project_id
   * @param device_vlan_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceVlanSettings(qint64 &project_id, ActDeviceVlanSettingList &device_vlan_setting_list,
                                      bool is_operation = false);

  /**
   * @brief Get Device Time Setting object
   *
   * @param project_id
   * @param device_id
   * @param device_time_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceTimeSetting(qint64 &project_id, qint64 &device_id, ActDeviceTimeSetting &device_time_setting,
                                  bool is_operation = false);

  /**
   * @brief Get Device Time Setting object
   *
   * @param project_id
   * @param device_time_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceTimeSettings(qint64 &project_id, ActDeviceTimeSettingList &device_time_setting_list,
                                   bool is_operation = false);

  /**
   * @brief Update Device Time Setting object
   *
   * @param project_id
   * @param device_time_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceTimeSettings(qint64 &project_id, ActDeviceTimeSettingList &device_time_setting_list,
                                      bool is_operation = false);

  /**
   * @brief Get Device Port Setting object
   *
   * @param project_id
   * @param device_id
   * @param device_port_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDevicePortSetting(qint64 &project_id, qint64 &device_id, ActDevicePortSetting &device_port_setting,
                                  bool is_operation = false);

  /**
   * @brief Get Device Port Setting object
   *
   * @param project_id
   * @param device_port_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDevicePortSettings(qint64 &project_id, ActDevicePortSettingList &device_port_setting_list,
                                   bool is_operation = false);

  /**
   * @brief Update Device Port Setting object
   *
   * @param project_id
   * @param device_port_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDevicePortSettings(qint64 &project_id, ActDevicePortSettingList &device_port_setting_list,
                                      bool is_operation = false);

  /**
   * @brief Get Device Ip Setting object
   *
   * @param project_id
   * @param device_id
   * @param device_ip_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceIpSetting(qint64 &project_id, qint64 &device_id, ActDeviceIpSetting &device_ip_setting,
                                bool is_operation = false);

  /**
   * @brief Get Device Ip Setting object
   *
   * @param project_id
   * @param device_ip_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceIpSettings(qint64 &project_id, ActDeviceIpSettingList &device_ip_setting_list,
                                 bool is_operation = false);

  /**
   * @brief Update Device Ip Setting object
   *
   * @param project_id
   * @param device_ip_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceIpSettings(qint64 &project_id, ActDeviceIpSettingList &device_ip_setting_list,
                                    bool is_operation = false);

  /**
   * @brief Get Device Rstp Setting object
   *
   * @param project_id
   * @param device_id
   * @param device_rstp_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceRstpSetting(qint64 &project_id, qint64 &device_id, ActDeviceRstpSetting &device_rstp_setting,
                                  bool is_operation = false);

  /**
   * @brief Get Device Rstp Setting object
   *
   * @param project_id
   * @param device_rstp_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceRstpSettings(qint64 &project_id, ActDeviceRstpSettingList &device_rstp_setting_list,
                                   bool is_operation = false);

  /**
   * @brief Update Device Rstp Setting object
   *
   * @param project_id
   * @param device_rstp_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceRstpSettings(qint64 &project_id, ActDeviceRstpSettingList &device_rstp_setting_list,
                                      bool is_operation = false);

  /**
   * @brief Get Device Per-stream Priority Setting object
   *
   * @param project_id
   * @param device_id
   * @param device_setting
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDevicePerStreamPrioritySetting(qint64 &project_id, qint64 &device_id,
                                               ActDevicePerStreamPrioritySetting &device_setting,
                                               bool is_operation = false);

  /**
   * @brief Get Device Per-stream Priority Setting object
   *
   * @param project_id
   * @param device_per_stream_priority_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDevicePerStreamPrioritySettings(
      qint64 &project_id, ActDevicePerStreamPrioritySettingList &device_per_stream_priority_setting_list,
      bool is_operation = false);

  /**
   * @brief Update Device Per-stream Priority Setting object
   *
   * @param project_id
   * @param device_per_stream_priority_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDevicePerStreamPrioritySettings(
      qint64 &project_id, ActDevicePerStreamPrioritySettingList &device_per_stream_priority_setting_list,
      bool is_operation = false);

  /**
   * @brief Get Device Time Slot Setting object
   *
   * @param project_id
   * @param device_id
   * @param device_setting
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceTimeSlotSetting(qint64 &project_id, qint64 &device_id, ActDeviceTimeSlotSetting &device_setting,
                                      bool is_operation = false);

  /**
   * @brief Get Device Time Slot Setting object
   *
   * @param project_id
   * @param device_time_slot_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceTimeSlotSettings(qint64 &project_id, ActDeviceTimeSlotSettingList &device_time_slot_setting_list,
                                       bool is_operation = false);

  /**
   * @brief Update Device Time Slot Setting object
   *
   * @param project_id
   * @param device_time_slot_setting_list
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceTimeSlotSettings(qint64 &project_id,
                                          ActDeviceTimeSlotSettingList &device_time_slot_setting_list,
                                          bool is_operation = false);

  /**
   * @brief Clear the folder file & sub-folder
   *
   * @param path
   * @return ACT_STATUS
   */
  ACT_STATUS ClearFolder(const QString &path);

  /***********************
   *  Deploy Management  *
   * ********************/

  /**
   * @brief Check DeviceConfig can activate Deploy
   *
   * @param project
   * @return true
   * @return false
   */
  bool CheckDeployAvailableByDeviceConfig(const ActProject &project);

  /**
   * @brief Performs an deploy action for the specified project.
   *
   * @param project_id The ID of the project to perform deploy action on.
   * @return ACT_STATUS
   */
  bool CanDeployProject(qint64 project_id) const;

  /**
   * @brief Get the Design Baseline Deploy Devices object
   *
   * @param project_id
   * @param design_baseline_id
   * @param deploy_device_list
   * @return ACT_STATUS
   */
  ACT_STATUS GetDesignBaselineDeployDevices(qint64 &project_id, const qint64 &design_baseline_id,
                                            ActDeployDeviceList &deploy_device_list);

  /**
   * @brief Get the Deploy Devices object
   *
   * @param project_id
   * @param deploy_device_list
   * @return ACT_STATUS
   */
  ACT_STATUS GetProjectDeployDevices(qint64 &project_id, ActDeployDeviceList &deploy_device_list);

  /***********************************
   *  ManufactureResult Management  *
   * ********************************/
  /**
   * @brief Init the Manufacture Result object
   *
   * @param project_id
   * @param manufacture_result
   * @return ACT_STATUS
   */
  ACT_STATUS InitManufactureResult(qint64 &project_id, ActManufactureResult &manufacture_result);

  /**
   * @brief Init the Manufacture Result object
   *
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS InitManufactureResult(ActProject &project);

  /**
   * @brief Update the order in the ManufactureResult
   *
   * @param project_id
   * @param order
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateManufactureResultOrder(qint64 &project_id, const QString &order,
                                          ActManufactureResult &manufacture_result);

  /**
   * @brief Update the Re-Manufacture devices to ManufactureResult's Remain
   *
   * @param project_id
   * @param dev_ids
   * @param manufacture_result
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateReManufactureDevices(qint64 &project_id, const QList<qint64> &dev_ids,
                                        ActManufactureResult &manufacture_result);

  /**
   * @brief Update the Re-Manufacture devices to ManufactureResult's Remain
   *
   * @param project
   * @param dev_ids
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateReManufactureDevices(ActProject &project, const QList<qint64> &dev_ids);

  /**
   * @brief Get the Manufacture Result object
   *
   * @param project_id
   * @param manufacture_result
   * @return ACT_STATUS
   */
  ACT_STATUS GetManufactureResult(qint64 &project_id, ActManufactureResult &manufacture_result);

  /**
   * @brief Get the Total Manufacture Result object
   *
   * @param project_id
   * @param total_manufacture_result
   * @return ACT_STATUS
   */
  ACT_STATUS GetTotalManufactureResult(qint64 &project_id, ActTotalManufactureResult &total_manufacture_result);

  /*********************************
   *   Baseline Management  *
   * *******************************/

  /**
   * @brief Get the  Baseline List object
   *
   * @param project_id
   * @param baseline_list
   * @return ACT_STATUS
   */
  ACT_STATUS GetDesignBaselineList(qint64 &project_id, ActNetworkBaselineList &baseline_list);

  /**
   * @brief Get the Operation Baseline List object
   *
   * @param project_id
   * @param baseline_list
   * @return ACT_STATUS
   */
  ACT_STATUS GetOperationBaselineList(qint64 &project_id, ActNetworkBaselineList &baseline_list);

  /**
   * @brief Get the Activate Operation Simple Baseline object
   *
   * @param project_id
   * @param simple_baseline
   * @return ACT_STATUS
   */
  ACT_STATUS GetActivateOperationSimpleBaseline(qint64 &project_id, ActSimpleNetworkBaseline &simple_baseline);

  /**
   * @brief Get the Activate Operation Baseline Project object
   *
   * @param project_id
   * @param baseline_project
   * @return ACT_STATUS
   */
  ACT_STATUS GetActivateOperationBaselineProject(qint64 &project_id, ActProject &baseline_project);

  /**
   * @brief Get the Design Baseline Project object
   *
   * @param project_id
   * @param baseline_id
   * @param baseline_project
   * @return ACT_STATUS
   */
  ACT_STATUS GetDesignBaselineProject(qint64 &project_id, const qint64 &baseline_id, ActProject &baseline_project);

  /**
   * @brief Check a Design Baseline object
   *
   * @param project_id
   * @param baseline
   * @return ACT_STATUS
   */
  ACT_STATUS CheckDesignBaseline(const qint64 &project_id, const ActNetworkBaseline &baseline);

  /**
   * @brief Check a Operation Baseline object
   *
   * @param project_id
   * @param baseline
   * @return ACT_STATUS
   */
  ACT_STATUS CheckOperationBaseline(const qint64 &project_id, const ActNetworkBaseline &baseline);

  /**
   * @brief Create a Design Baseline object
   *
   * @param project_id
   * @param baseline_info
   * @param created_user_id
   * @param baseline
   * @return ACT_STATUS
   */
  ACT_STATUS CreateDesignBaseline(qint64 &project_id, const ActNetworkBaselineInfo &baseline_info,
                                  const qint64 &created_user_id, ActNetworkBaseline &baseline);

  /**
   * @brief Copy the Design Baseline to the Operation Baseline set
   *
   * @param project
   * @param design_baseline_id
   * @return ACT_STATUS
   */
  ACT_STATUS CopyDesignBaselineToOperation(ActProject &project, const qint64 &design_baseline_id);

  /**
   * @brief Activate Design Baseline
   *
   * @param project_id
   * @param activate_baseline_id
   * @param activate_user_id
   * @return ACT_STATUS
   */
  ACT_STATUS ActivateDesignBaseline(qint64 &project_id, const qint64 &activate_baseline_id,
                                    const qint64 &activate_user_id);

  /**
   * @brief Activate a Baseline at the the Design BaselineSet
   *
   * @param unactivate_baseline_id
   * @param activate_baseline_id
   * @param activate_user
   * @param activate_date
   * @return ACT_STATUS
   */
  ACT_STATUS ActivateBaselineAtDesignBaselineSet(const qint64 &unactivate_baseline_id,
                                                 const qint64 &activate_baseline_id, const QString &activate_user,
                                                 const quint64 &activate_date);

  /**
   * @brief Activate a Baseline at the the Operation BaselineSet
   *
   * @param unactivate_baseline_id
   * @param activate_baseline_id
   * @param activate_user
   * @param activate_date
   * @return ACT_STATUS
   */
  ACT_STATUS ActivateBaselineAtOperationBaselineSet(const qint64 &unactivate_baseline_id,
                                                    const qint64 &activate_baseline_id, const QString &activate_user,
                                                    const quint64 &activate_date);

  /**
   * @brief Create a Design Baseline And Activate object
   *
   * @param project_id
   * @param baseline_info
   * @param created_user_id
   * @param baseline
   * @return ACT_STATUS
   */
  ACT_STATUS CreateDesignBaselineAndActivate(qint64 &project_id, const ActNetworkBaselineInfo &baseline_info,
                                             const qint64 &created_user_id, ActNetworkBaseline &baseline);

  /**
   * @brief Check DesignBaseline Project diff with Project
   *
   * @param project_id
   * @param baseline_id
   * @param diff_report
   * @return ACT_STATUS
   */
  ACT_STATUS CheckDesignBaselineProjectDiffWithProject(qint64 &project_id, const qint64 &baseline_id,
                                                       ActBaselineProjectDiffReport &diff_report);

  /**
   * @brief Register Design Baseline
   *
   * @param project_id
   * @param baseline_id
   * @param register_user_id
   * @return ACT_STATUS
   */
  ACT_STATUS RegisterDesignBaseline(qint64 &project_id, const qint64 &baseline_id, const qint64 &register_user_id);

  /**
   * @brief Rollback Design Baseline
   *
   * @param project_id
   * @param baseline_id
   * @return ACT_STATUS
   */
  ACT_STATUS RollbackDesignBaseline(qint64 &project_id, const qint64 &baseline_id);

  /**
   * @brief Update a Design Baseline object
   *
   * @param project_id
   * @param baseline
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDesignBaseline(qint64 &project_id, ActNetworkBaseline &baseline);

  /**
   * @brief Update a Operation Baseline object
   *
   * @param project_id
   * @param baseline
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateOperationBaseline(qint64 &project_id, ActNetworkBaseline &baseline);

  /**
   * @brief Get the Design Baseline data object(no devices content)
   *
   * @param project_id
   * @param baseline_id
   * @param baseline
   * @return ACT_STATUS
   */
  ACT_STATUS GetDesignBaseline(qint64 &project_id, qint64 &baseline_id, ActNetworkBaseline &baseline);

  /**
   * @brief Get the Operation Baseline data object(no devices content)
   *
   * @param project_id
   * @param baseline_id
   * @param baseline
   * @return ACT_STATUS
   */
  ACT_STATUS GetOperationBaseline(qint64 &project_id, qint64 &baseline_id, ActNetworkBaseline &baseline);

  /**
   * @brief Get the Design Baseline BOM Detail object
   *
   * @param project_id
   * @param baseline_id
   * @param baseline_bom_detail
   * @return ACT_STATUS
   */
  ACT_STATUS GetDesignBaselineBOMDetail(qint64 &project_id, qint64 &baseline_id,
                                        ActBaselineBOMDetail &baseline_bom_detail);

  /**
   * @brief Get the Operation Baseline BOM Detail object
   *
   * @param project_id
   * @param baseline_id
   * @param baseline_bom_detail
   * @return ACT_STATUS
   */
  ACT_STATUS GetOperationBaselineBOMDetail(qint64 &project_id, qint64 &baseline_id,
                                           ActBaselineBOMDetail &baseline_bom_detail);

  /**
   * @brief Get the Design Baseline with devices data object
   *
   * @param project_id
   * @param baseline_id
   * @param baseline
   * @return ACT_STATUS
   */
  ACT_STATUS GetDesignBaselineWithDevices(qint64 &project_id, qint64 &baseline_id, ActNetworkBaseline &baseline);

  /**
   * @brief Get the Operation Baseline with devices data object
   *
   * @param project_id
   * @param baseline_id
   * @param baseline
   * @return ACT_STATUS
   */
  ACT_STATUS GetOperationBaselineWithDevices(qint64 &project_id, qint64 &baseline_id, ActNetworkBaseline &baseline);

  /**
   * @brief Delete Design and Operation(if exists) Baseline
   *
   * @param project_id
   * @param baseline_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteDesignAndOperationBaseline(qint64 &project_id, qint64 &baseline_id);

  /**
   * @brief Delete Design Baseline object
   *
   * @param project_id
   * @param baseline_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteDesignBaseline(qint64 &project_id, qint64 &baseline_id);

  /**
   * @brief Delete Design Baseline object
   *
   * @param project
   * @param baseline_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteDesignBaseline(ActProject &project, qint64 &baseline_id);

  /**
   * @brief Delete Operation Baseline object
   *
   * @param project_id
   * @param baseline_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteOperationBaseline(qint64 &project_id, qint64 &baseline_id);

  /**
   * @brief Delete Operation Baseline object
   *
   * @param project
   * @param baseline_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteOperationBaseline(ActProject &project, qint64 &baseline_id);

  /**
   * @brief Delete Project's NetworkBaselines object
   *
   * @param project_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteProjectAllBaselines(qint64 &project_id);

  /******************************
   *  TrafficDesign Management  *
   * ***************************/

  /**
   * @brief Get Traffic Type Configuration Setting object
   *
   * @param project_id
   * @param traffic_type_configuration_setting
   * @return ACT_STATUS
   */
  ACT_STATUS GetTrafficTypeConfigurationSetting(qint64 &project_id,
                                                ActTrafficTypeConfigurationSetting &traffic_type_configuration_setting);

  /**
   * @brief Update Traffic Type Configuration Setting object
   *
   * @param project_id
   * @param traffic_type_configuration
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateTrafficTypeConfigurationSetting(qint64 &project_id,
                                                   ActTrafficTypeConfiguration &traffic_type_configuration);

  /**
   * @brief Update Traffic Application Setting object
   *
   * @param project_id
   * @param traffic_application_setting
   * @return ACT_STATUS
   */
  ACT_STATUS GetTrafficApplicationSetting(qint64 &project_id,
                                          ActTrafficApplicationSetting &traffic_application_setting);

  /**
   * @brief Create Traffic Application Setting object
   *
   * @param project_id
   * @param traffic_application
   * @return ACT_STATUS
   */
  ACT_STATUS CreateTrafficApplicationSetting(qint64 &project_id, ActTrafficApplication &traffic_application);

  /**
   * @brief Create Traffic Application Setting object
   *
   * @param project_id
   * @param application_id
   * @param traffic_application
   * @return ACT_STATUS
   */
  ACT_STATUS CopyTrafficApplicationSetting(qint64 &project_id, qint64 &application_id,
                                           ActTrafficApplication &traffic_application);

  /**
   * @brief Update Traffic Application Setting object
   *
   * @param project_id
   * @param traffic_application
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateTrafficApplicationSetting(qint64 &project_id, ActTrafficApplication &traffic_application);

  /**
   * @brief Delete Traffic Application Setting object
   *
   * @param project_id
   * @param traffic_application_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteTrafficApplicationSetting(qint64 &project_id, qint64 &traffic_application_id);

  /**
   * @brief Update Traffic Stream Setting object
   *
   * @param project_id
   * @param traffic_stream_setting
   * @return ACT_STATUS
   */
  ACT_STATUS GetTrafficStreamSetting(qint64 &project_id, ActTrafficStreamSetting &traffic_stream_setting);

  /**
   * @brief Create Traffic Stream Setting object
   *
   * @param project_id
   * @param traffic_stream
   * @return ACT_STATUS
   */
  ACT_STATUS CreateTrafficStreamSetting(qint64 &project_id, ActTrafficStream &traffic_stream);

  /**
   * @brief Create Traffic Stream Setting object
   *
   * @param project_id
   * @param traffic_stream
   * @return ACT_STATUS
   */
  ACT_STATUS CopyTrafficStreamSetting(qint64 &project_id, qint64 &stream_id, ActTrafficStream &traffic_stream);

  /**
   * @brief Update Traffic Stream Setting object
   *
   * @param project_id
   * @param traffic_stream
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateTrafficStreamSetting(qint64 &project_id, ActTrafficStream &traffic_stream);

  /**
   * @brief Update Traffic Stream Setting object
   *
   * @param project
   * @param traffic_stream
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateTrafficStreamSetting(ActProject &project, ActTrafficStream &traffic_stream);

  /**
   * @brief Delete Traffic Stream Setting object
   *
   * @param project_id
   * @param traffic_stream_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteTrafficStreamSetting(qint64 &project_id, qint64 &traffic_stream_id);

  /**
   * @brief Delete Traffic Stream Setting object
   *
   * @param project
   * @param traffic_stream_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteTrafficStreamSetting(ActProject &project, qint64 &traffic_stream_id);

  /**************************
   *  DeviceConfig Management  *
   * ************************/

  /**
   * @brief Update DeviceConfig object
   *
   * @param project_id
   * @param device_config
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceConfig(qint64 &project_id, ActDeviceConfig &device_config);

  /**
   * @brief Update DeviceConfig object in a specific project object
   *
   * @param project
   * @param device_config
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceConfig(ActProject &project, ActDeviceConfig &device_config);

  /**
   * @brief Delete DeviceConfig by device
   *
   * @param project
   * @param device_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteDeviceDeviceConfig(ActProject &project, qint64 &device_id);

  /**
   * @brief Update DeviceConfig by device
   *
   * @param project
   * @param update_device_config
   * @param device_id
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceDeviceConfig(ActProject &project, const ActDeviceConfig &update_device_config,
                                      const qint64 &device_id);

  /**
   * @brief Check the DeviceConfig feasibility
   *
   * @param device_config
   * @return ACT_STATUS
   */
  ACT_STATUS CheckDeviceConfig(const ActDeviceConfig &device_config);

  /**
   * @brief Generate the DeviceConfig by Project's ComputeResult
   *
   * @param project
   * @param device_config
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateDeviceConfigByComputeResult(const ActProject &project, ActDeviceConfig &device_config);

  /**
   * @brief Generate the VLAN Static DeviceConfig by Project's ComputeResult
   *
   * @param project
   * @param device_config
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateVLANStaticDeviceConfigByComputeResult(const ActProject &project, ActDeviceConfig &device_config);

  /*************************************
   *  StaticForward config Management  *
   * **********************************/

  /**
   * @brief Get the Unicast Static Forward Config object
   *
   * @param project_id
   * @param device_id
   * @param static_forward_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetUnicastStaticForwardConfig(qint64 &project_id, qint64 &device_id,
                                           ActStaticForwardTable &static_forward_table);

  /**
   * @brief Get the Multicast Static Forward Config object
   *
   * @param project_id
   * @param device_id
   * @param static_forward_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetMulticastStaticForwardConfig(qint64 &project_id, qint64 &device_id,
                                             ActStaticForwardTable &static_forward_table);

  /****************************
   *  Vlan-config Management  *
   * *************************/

  /**
   * @brief Update the Vlan Config object
   *
   * @param project_id
   * @param intelligent_vlan
   * @return ACT_STATUS
   */
  ACT_STATUS CreateVlanGroup(qint64 &project_id, ActIntelligentVlan &intelligent_vlan);

  /**
   * @brief Update the Vlan Config object
   *
   * @param project
   * @param intelligent_vlan
   * @return ACT_STATUS
   */
  ACT_STATUS CreateVlanGroup(ActProject &project, ActIntelligentVlan &intelligent_vlan);

  /**
   * @brief Update the Vlan Config object
   *
   * @param project_id
   * @param intelligent_vlan
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateVlanGroup(qint64 &project_id, ActIntelligentVlan &intelligent_vlan);

  /**
   * @brief Update the Vlan Config object
   *
   * @param project
   * @param intelligent_vlan
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateVlanGroup(ActProject &project, ActIntelligentVlan &intelligent_vlan);

  /**
   * @brief Delete the Vlan Config object
   *
   * @param project_id
   * @param vlan_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteVlanGroup(qint64 &project_id, quint16 &vlan_id);

  /**
   * @brief Delete project's Vlan Config object
   *
   * @param project
   * @param vlan_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteVlanGroup(ActProject &project, quint16 &vlan_id);

  /**
   * @brief Update the Vlan Config object
   *
   * @param project_id
   * @param vlan_config_table
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateVlanConfig(qint64 &project_id, ActVlanTable &vlan_config_table);

  /**
   * @brief Update the Vlan Config object
   *
   * @param project
   * @param vlan_config_table
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateVlanConfig(ActProject &project, ActVlanTable &vlan_config_table);

  /**
   * @brief Update the Vlan Config object
   *
   * @param project_id
   * @param vlan_config
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateVlanConfig(qint64 &project_id, ActVlanConfig &vlan_config);

  /**
   * @brief Update the Vlan Config object
   *
   * @param project
   * @param vlan_config
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateVlanConfig(ActProject &project, ActVlanConfig &vlan_config);

  /**
   * @brief Delete the Vlan Config object
   *
   * @param project_id
   * @param device_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteVlanConfig(qint64 &project_id, qint64 &device_id);

  /**
   * @brief Delete project's Vlan Config object
   *
   * @param project
   * @param device_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteVlanConfig(ActProject &project, qint64 &device_id);

  /**
   * @brief Delete the Vlan Config object
   *
   * @param project_id
   * @param vlan_config
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteVlanConfig(qint64 &project_id, ActVlanConfig &vlan_config);

  /**
   * @brief Delete project's Vlan Config object
   *
   * @param project
   * @param vlan_config
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteVlanConfig(ActProject &project, ActVlanConfig &vlan_config);

  /**
   * @brief Get the Vlan Config object
   *
   * @param project_id
   * @param device_id
   * @param vlan_config_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetVlanConfig(qint64 &project_id, qint64 &device_id, ActVlanTable &vlan_config_table);

  /**
   * @brief Get the Vlan Config object
   *
   * @param project
   * @param device_id
   * @param vlan_config_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetVlanConfig(ActProject &project, qint64 &device_id, ActVlanTable &vlan_config_table);

  /**
   * @brief Compute the Vlan Config object
   *
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS ComputeVlanConfig(ActProject &project);

  /**************************
   *  Vlan-view Management  *
   * ************************/

  /**
   * @brief Get all vlan-view objects in specific project
   *
   * @param project_id
   * @param vlan_view_ids
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetVlanViewIds(qint64 &project_id, ActVlanViewIds &vlan_view_ids, bool is_operation = false);

  /**
   * @brief Get all vlan-view objects in specific project
   *
   * @param project_id
   * @param vlan_view_table
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetVlanViews(qint64 &project_id, ActVlanViewTable &vlan_view_table, bool is_operation = false);

  /**
   * @brief Get all vlan-view objects in specific project
   *
   * @param project
   * @param vlan_view_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetVlanViews(ActProject &project, ActVlanViewTable &vlan_view_table);

  /**
   * @brief Get a vlan-view object in specific project
   *
   * @param project_id
   * @param vlan_id
   * @param vlan_view
   * @param is_operation Indicate whether this API is called in an operation
   * @return ACT_STATUS
   */
  ACT_STATUS GetVlanView(qint64 &project_id, qint32 &vlan_id, ActVlanView &vlan_view, bool is_operation = false);

  /**
   * @brief Get a vlan-view object in specific project
   *
   * @param project
   * @param vlan_id
   * @param vlan_view
   * @return ACT_STATUS
   */
  ACT_STATUS GetVlanView(ActProject &project, qint32 &vlan_id, ActVlanView &vlan_view);

  /*************************
   *  Topology Management  *
   * ***********************/

  /**
   * @brief Check the topology configuration is correct
   *
   * @param topology
   * @return ACT_STATUS
   */
  ACT_STATUS CheckTopology(const ActTopology &topology);

  /**
   * @brief Get an existing topology object
   *
   * @param id
   * @param topology
   * @return ACT_STATUS
   */
  ACT_STATUS GetTopology(const qint64 &id, ActTopology &topology);

  /**
   * @brief Save the topology
   *
   * @param topology_param
   * @return ACT_STATUS
   */
  ACT_STATUS SaveTopology(const ActTopologyCreateParam &topology_param);

  /**
   * @brief Append the topology configuration to specific project
   *
   * @param project_id
   * @param topology_cfg
   * @return ACT_STATUS
   */
  ACT_STATUS AppendTopology(qint64 &project_id, const ActTopology &topology_cfg);

  /**
   * @brief Append the device to the project from topology configuration
   *
   * @param project
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AppendDevice(ActProject &project, ActDevice &topo_device);

  /**
   * @brief Append the link to the project from topology configuration
   *
   * @param project
   * @param link
   * @return ACT_STATUS
   */
  ACT_STATUS AppendLink(ActProject &project, ActLink &topo_link);

  /**
   * @brief Append the stream to the project from topology configuration
   *
   * @param project
   * @param stream
   * @return ACT_STATUS
   */
  ACT_STATUS AppendStream(ActProject &project, ActStream &topo_stream);

  /**
   * @brief Update the specified topology
   *
   * @param topology
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateTopology(ActTopology &topology);

  /**
   * @brief Delete the topology from topology management
   *
   * @param id The id of the topology
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteTopology(const qint64 &id);

  /**
   * @brief Copy devices & related links
   *
   * @param project_id
   * @param dev_ids
   * @param copied_dev_ids
   * @param copied_link_ids
   * @return ACT_STATUS
   */
  ACT_STATUS CopyTopology(qint64 &project_id, QList<qint64> &dev_ids, QList<qint64> &copied_dev_ids,
                          QList<qint64> &copied_link_ids);

  /**
   * @brief Copy devices & related links to a specific project object
   *
   * @param project
   * @param dev_ids
   * @param copied_dev_ids
   * @param copied_link_ids
   * @return ACT_STATUS
   */
  ACT_STATUS CopyTopology(ActProject &project, QList<qint64> &dev_ids, QList<qint64> &copied_dev_ids,
                          QList<qint64> &copied_link_ids);

  /**
   * @brief Check Topology loop to specific project
   *
   * @param project_id
   * @return ACT_STATUS
   */
  ACT_STATUS CheckTopologyLoop(qint64 &project_id, ActTopologyStatus &topology_status);

  /**
   * @brief Create Topology template to specific project
   * This function is for UI only, so it will auto sync to OPCUA
   *
   * @param project_id
   * @param topology_template
   * @param copied_dev_ids
   * @param copied_link_ids
   * @return ACT_STATUS
   */
  ACT_STATUS CreateTopologyTemplate(qint64 &project_id, ActTopologyTemplate &topology_template,
                                    QList<qint64> &copied_dev_ids, QList<qint64> &copied_link_ids);

  /**************************
   *  Redundant Group       *
   * ************************/
  /**
   * @brief Check redundant group
   *
   * @param project
   * @param rstp
   * @return ACT_STATUS
   */
  ACT_STATUS CheckRedundantRSTP(const ActProject &project, const ActRSTP &rstp);

  /**
   * @brief Create redundant group
   *
   * @param project_id
   * @param rstp
   * @return ACT_STATUS
   */
  ACT_STATUS CreateRedundantRSTP(qint64 &project_id, ActRSTP &rstp);

  /**
   * @brief Create redundant group
   *
   * @param project_id
   * @param rstp
   * @return ACT_STATUS
   */
  ACT_STATUS CreateRedundantRSTP(ActProject &project, ActRSTP &rstp);

  /**
   * @brief Update redundant group
   *
   * @param project_id
   * @param rstp
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateRedundantRSTP(qint64 &project_id, ActRSTP &rstp);

  /**
   * @brief Update redundant group
   *
   * @param project_id
   * @param rstp
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateRedundantRSTP(ActProject &project, ActRSTP &rstp);

  /**
   * @brief Delete redundant group
   *
   * @param project_id
   * @param rstp_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteRedundantRSTP(qint64 &project_id, qint64 &rstp_id);

  /**
   * @brief Delete redundant group
   *
   * @param project_id
   * @param rstp_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteRedundantRSTP(ActProject &project, qint64 &rstp_id);

  /**
   * @brief Compute redundant group
   *
   * @param project
   * @return ACT_STATUS
   */
  void ComputeRedundantRSTP(ActProject &project);

  /**
   * @brief Check redundant swift
   *
   * @param project
   * @param swift
   * @return ACT_STATUS
   */
  ACT_STATUS CheckRedundantSwift(ActProject &project, ActSwift &swift);

  /**
   * @brief Update redundant swift
   *
   * @param project_id
   * @param swift
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateRedundantSwift(qint64 &project_id, ActSwift &swift);

  /**
   * @brief Update redundant swift
   *
   * @param project
   * @param swift
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateRedundantSwift(ActProject &project, ActSwift &swift);

  /**
   * @brief Delete redundant swift
   *
   * @param project_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteRedundantSwift(qint64 &project_id);

  /**
   * @brief Delete redundant swift
   *
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteRedundantSwift(ActProject &project);

  /**
   * @brief Compute redundant swift
   *
   * @param project_id
   * @return ACT_STATUS
   */
  ACT_STATUS ComputeRedundantSwift(qint64 &project_id);

  /**
   * @brief Compute redundant swift
   *
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS ComputeRedundantSwift(ActProject &project);

  /**
   * @brief Compute redundant swift
   *
   * @param project_id
   * @param swift
   * @return ACT_STATUS
   */
  ACT_STATUS ComputeRedundantSwift(qint64 &project_id, ActSwift &swift);

  /**
   * @brief Compute redundant swift
   *
   * @param project
   * @param swift
   * @return ACT_STATUS
   */
  ACT_STATUS ComputeRedundantSwift(ActProject &project, ActSwift &swift);

  /**
   * @brief Compute redundant swift candidates
   *
   * @param project_id
   * @param swift_candidates
   * @return ACT_STATUS
   */
  ACT_STATUS ComputeRedundantSwiftCandidate(qint64 &project_id, ActSwiftCandidates &swift_candidates);

  /**
   * @brief Compute redundant swift candidates
   *
   * @param project
   * @param swift_candidates
   * @return ACT_STATUS
   */
  ACT_STATUS ComputeRedundantSwiftCandidate(ActProject &project, ActSwiftCandidates &swift_candidates);

  /************************************
   *  Command line interface Request  *
   * **********************************/
  /**
   * @brief Start CLI Request
   *
   * @param project_id
   * @param ws_listener_id
   * @param request
   * @return ACT_STATUS
   */
  ACT_STATUS StartCommandLineInterfaceRequest(qint64 &project_id, const qint64 &ws_listener_id,
                                              ActIntelligentRecognizeRequest &request);

  /**
   * @brief Parse VLAN Request
   *
   * @param project_id
   * @param ws_listener_id
   * @param request
   */
  void ParseVlanRequest(qint64 &project_id, const qint64 &ws_listener_id, QString &request);

  /**
   * @brief Parse Backup Request
   *
   * @param project_id
   * @param ws_listener_id
   * @param request
   * @param caller_skip_finish
   */
  void ParseBackupRequest(qint64 &project_id, const qint64 &ws_listener_id, QString &request, bool &caller_skip_finish);

  /**
   * @brief Parse Restore Request
   *
   * @param project_id
   * @param ws_listener_id
   * @param request
   * @param caller_skip_finish
   */
  void ParseRestoreRequest(qint64 &project_id, const qint64 &ws_listener_id, QString &request,
                           bool &caller_skip_finish);

  /**
   * @brief Parse Abort Request
   *
   * @param project_id
   * @param ws_listener_id
   * @param cli_request
   */
  void ParseAbortRequest(qint64 &project_id, const qint64 &ws_listener_id, QString &cli_request);

  /**************************
   *  Intelligent Request   *
   * ************************/

  /**
   * @brief Start Intelligent Request thread
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   * @param request
   */
  void StartIntelligentRequestThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                                     ActIntelligentRecognizeRequest request);

  /**
   * @brief Start Intelligent Request(for streaming reply data)
   *
   * @param project_id
   * @param ws_listener_id
   * @param request
   * @return ACT_STATUS
   */
  ACT_STATUS StartIntelligentRequest(qint64 &project_id, const qint64 &ws_listener_id,
                                     ActIntelligentRecognizeRequest &request);

  /**
   * @brief Intelligent request
   *
   * @param request
   * @param response
   * @return ACT_STATUS
   */
  ACT_STATUS IntelligentRequest(ActIntelligentRecognizeRequest &request, ActIntelligentResponse &response);

  /**
   * @brief Report feedback of the intelligent response
   *
   * @param report
   * @param response
   * @return ACT_STATUS
   */
  ACT_STATUS IntelligentReport(ActIntelligentReport &report, ActIntelligentResponse &response);

  /**
   * @brief Get the Intelligent History object
   *
   * @param history
   * @param response
   * @return ACT_STATUS
   */
  ACT_STATUS GetIntelligentHistory(ActIntelligentHistory &history, ActIntelligentHistoryResponse &response);

  /*******************************
   *  Intelligent Questionnaire  *
   * ****************************/

  ACT_STATUS IntelligentUploadFile(ActIntelligentUploadFile &upload_file);

  /**
   * @brief Start Intelligent questionnaire excel upload (for streaming reply data)
   *
   * @param ws_listener_id
   * @param upload
   * @return ACT_STATUS
   */
  ACT_STATUS StartIntelligentQuestionnaireUpload(qint64 &project_id, const qint64 &ws_listener_id,
                                                 ActIntelligentQuestionnaireUpload &upload);

  /**
   * @brief Start Intelligent questionnaire excel Upload thread
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   * @param upload
   */
  void StartIntelligentQuestionnaireUploadThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                                 qint64 project_id, ActIntelligentQuestionnaireUpload upload);

  /**
   * @brief Start Intelligent questionnaire excel template download
   *
   * @param project_id
   * @param ws_listener_id
   * @param download
   * @return ACT_STATUS
   */
  ACT_STATUS StartIntelligentQuestionnaireDownload(qint64 &project_id, const qint64 &ws_listener_id,
                                                   ActIntelligentQuestionnaireDownload &download);

  /**
   * @brief Start Intelligent questionnaire excel template download thread
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   */
  void StartIntelligentQuestionnaireDownloadThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                                   qint64 project_id, ActIntelligentQuestionnaireDownload download);

  /****************************
   *  Class Based Management  *
   * **************************/

  /**
   * @brief Update class based cycle setting
   *
   * @param project_id
   * @param cycle_setting
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateCycleSetting(qint64 &project_id, ActCycleSetting &cycle_setting);

  /**
   * @brief Update class based cycle setting in a specific project object
   *
   * @param project
   * @param cycle_setting
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateCycleSetting(ActProject &project, ActCycleSetting &cycle_setting);

  /***************************
   *  Web Socket Management  *
   * *************************/

  /**
   * @brief The callback function of web socket test module
   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   */
  void StartWSTestThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id);

  /**
   * @brief Start test ws procedure
   *
   * @param project_id
   * @param socket
   * @return ACT_STATUS
   */
  // ACT_STATUS StartWSTest(qint64 &project_id, const qint64 &ws_listener_id);
  ACT_STATUS StartWSTest(qint64 &project_id, const qint64 &ws_listener_id);

  /**
   * @brief Stop test ws procedure
   *
   * @param project_id
   * @param ws_listener_id
   * @return ACT_STATUS
   */
  ACT_STATUS StopWSTest(qint64 &project_id, const qint64 &ws_listener_id);

  /**
   * @brief Stop job in web socket thread
   *
   * @param project_id
   * @param ws_listener_id
   * @param op_code_enum
   * @return ACT_STATUS
   */
  ACT_STATUS StopWSJob(qint64 &project_id, const qint64 &ws_listener_id, const ActWSCommandEnum &op_code_enum);

  /**
   * @brief Remove job in web socket thread
   *
   * @param project_id
   * @return ACT_STATUS
   */
  ACT_STATUS RemoveWSJob(qint64 &project_id);

  /**
   * @brief Stop job in web socket system thread
   *
   * @param ws_listener_id
   * @return ACT_STATUS
   */
  ACT_STATUS StopWSJobSystem(const qint64 &ws_listener_id);

  /**
   * @brief Stop all ACT Web socket threads
   *
   * @return ACT_STATUS
   */
  ACT_STATUS DestroyWSJob();

  /**
   * @brief Stop ACT monitor Web socket threads
   *
   * @return ACT_STATUS
   */
  ACT_STATUS DestroyWSJobMonitor();

  /**
   * @brief Check the WS job thread can start
   *
   * @param new_job_status
   * @param project_id
   * @param result_project_name
   * @return ACT_STATUS
   */
  ACT_STATUS CheckWSJobCanStart(const ActProjectStatusEnum &new_job_status, qint64 &project_id,
                                QString &result_project_name);

  /**
   * @brief The callback function of scan topology module for OPC UA
   *
   * @param project_id
   * @param scan_ip_ranges The list of scan ip ranges
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartScanTopologyThread(qint64 project_id, ActScanIpRange scan_ip_ranges, bool new_topology,
                                    std::future<void> signal_receiver, void (*cb_func)(ACT_STATUS status, void *arg),
                                    void *arg);

  /**
   * @brief Start scan topology procedure for OPC UA
   *
   * @param project_id
   * @param scan_ip_ranges The list of scan ip ranges
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartScanTopology(qint64 &project_id, ActScanIpRange &scan_ip_ranges, bool new_topology,
                                    std::future<void> signal_receiver, void (*cb_func)(ACT_STATUS status, void *arg),
                                    void *arg);

  /**
   * @brief The callback function of scan topology module
   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   * @param new_topology
   */
  void StartScanTopologyThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                               bool new_topology);

  /**
   * @brief Start scan topology procedure
   *
   * @param project_id
   * @param socket
   * @param new_topology
   * @return ACT_STATUS
   */
  ACT_STATUS StartScanTopology(qint64 &project_id, const qint64 &ws_listener_id, bool new_topology);

  /**
   * @brief The callback function of Sync devices module
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   */
  void StartSyncDevicesThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                              QList<qint64> dev_id_list);

  /**
   * @brief Start Sync devices procedure
   *
   * @param project_id
   * @param socket
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartSyncDevices(qint64 &project_id, const qint64 &ws_listener_id, const QList<qint64> &dev_id_list);

  /**
   * @brief The callback function of Sync devices module
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   */
  void OpcUaStartSyncDevicesThread(qint64 project_id, QList<qint64> dev_id_list, std::future<void> signal_receiver,
                                   void (*cb_func)(ACT_STATUS status, void *arg), void *arg);
  /**
   * @brief Start Sync devices procedure
   *
   * @param project_id
   * @param socket
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartSyncDevices(qint64 &project_id, const QList<qint64> &dev_id_list,
                                   std::future<void> signal_receiver, void (*cb_func)(ACT_STATUS status, void *arg),
                                   void *arg);

  /**
   * @brief The callback function of topology mapping module
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   * @param design_baseline_id
   */
  void StartTopologyMappingThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                                  qint64 design_baseline_id);

  /**
   * @brief Start topology mapping procedure
   *
   * @param project_id
   * @param design_baseline_id
   * @param ws_listener_id
   * @return ACT_STATUS
   */
  ACT_STATUS StartTopologyMapping(qint64 &project_id, qint64 &design_baseline_id, const qint64 &ws_listener_id);

  /**
   * @brief The callback function of Scan mapping module
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   */
  void StartScanMappingThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id);

  /**
   * @brief Start Scan topology mapping procedure
   *
   * @param project_id
   * @param ws_listener_id
   * @return ACT_STATUS
   */
  ACT_STATUS StartScanMapping(qint64 &project_id, const qint64 &ws_listener_id);

  /**
   * @brief The callback function of compute module for OPC UA
   *
   * @param project_id
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartComputeThread(qint64 project_id, std::future<void> signal_receiver,
                               void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief Start compute procedure for OPC UA
   *
   * @param project_id
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartCompute(qint64 &project_id, std::future<void> signal_receiver,
                               void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief The callback function of compute module
   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   */
  void StartComputeThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id);

  /**
   * @brief Start compute procedure
   *
   * @param project_id
   * @param socket
   * @return ACT_STATUS
   */
  ACT_STATUS StartCompute(qint64 &project_id, const qint64 &ws_listener_id);

  /**
   * @brief Get the Compute Result object
   *
   * @param project_id
   * @param computed_rlt
   * @return ACT_STATUS
   */
  ACT_STATUS GetComputedResult(qint64 &project_id, ActComputedResult &computed_rlt);

  /**
   * @brief Delete the Compute Result from the project
   *
   * @param project_id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteComputedResult(qint64 &project_id);

  /**
   * @brief compute topology setting from the project
   *
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS ComputeTopologySetting(ActProject &project);

  /**
   * @brief The callback function of compare module for OPC UA
   *
   * @param project_id
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartCompareThread(qint64 project_id, std::future<void> signal_receiver,
                               void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief Start compare procedure for OPC UA
   *
   * @param project_id
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartCompare(qint64 &project_id, std::future<void> signal_receiver,
                               void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief The callback function of compare module
   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   */
  void StartCompareThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id);

  /**
   * @brief Start compare procedure
   *
   * @param project_id
   * @param socket
   * @return ACT_STATUS
   */
  ACT_STATUS StartCompare(qint64 &project_id, const qint64 &ws_listener_id);

  /**
   * @brief The callback function of deploy module
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   * @param action
   * @param skip_mapping_dev
   * @param parameter_base
   */
  void StartDeployThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                         QList<qint64> dev_id_list, ActDeployActionEnum action, bool skip_mapping_dev,
                         ActDeployParameterBase *parameter_base);

  /**
   * @brief Start deploy procedure
   *
   * @param project_id
   * @param ws_listener_id
   * @param dev_id_list
   * @param action
   * @param skip_mapping_dev
   * @param parameter_base
   * @return ACT_STATUS
   */
  ACT_STATUS StartDeploy(qint64 &project_id, const qint64 &ws_listener_id, const QList<qint64> &dev_id_list,
                         const ActDeployActionEnum &action, const bool &skip_mapping_dev,
                         ActDeployParameterBase *parameter_base);

  /**
   * @brief The Opc Ua callback function of deploy ini module

   *
   * @param project_id
   * @param dev_id_list
   * @param skip_mapping_dev
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartDeployIniThread(qint64 project_id, QList<qint64> dev_id_list, bool skip_mapping_dev,
                                 std::future<void> signal_receiver, void (*cb_func)(ACT_STATUS status, void *arg),
                                 void *arg);

  /**
   * @brief Start Opc Ua deploy ini procedure
   *
   * @param project_id
   * @param dev_id_list
   * @param skip_mapping_dev
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartDeployIni(qint64 &project_id, const QList<qint64> &dev_id_list, const bool &skip_mapping_dev,
                                 std::future<void> signal_receiver, void (*cb_func)(ACT_STATUS status, void *arg),
                                 void *arg);

  /**
   * @brief The callback function of deploy ini module

   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   * @param skip_mapping_dev
   */
  void StartDeployIniThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                            QList<qint64> dev_id_list, bool skip_mapping_dev);

  /**
   * @brief Start deploy ini procedure
   *
   * @param project_id
   * @param ws_listener_id
   * @param dev_id_list
   * @param skip_mapping_dev
   * @return ACT_STATUS
   */
  ACT_STATUS StartDeployIni(qint64 &project_id, const qint64 &ws_listener_id, const QList<qint64> &dev_id_list,
                            const bool &skip_mapping_dev);

  /**
   * @brief The callback function of Manufacture deploy ini module
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   */
  void StartManufactureDeployIniThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                       qint64 project_id);

  /**
   * @brief Start Manufacture deploy ini procedure
   *
   * @param project_id
   * @param ws_listener_id
   * @return ACT_STATUS
   */
  ACT_STATUS StartManufactureDeployIni(qint64 &project_id, const qint64 &ws_listener_id);

  /**
   * @brief The callback function of BroadcastSearch's DeviceDiscovery module for OPC UA
   *
   * @param project_id
   * @param dev_discovery_cfg
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartDeviceDiscoveryThread(qint64 project_id, ActDeviceDiscoveryConfig dev_discovery_cfg,
                                       std::future<void> signal_receiver, void (*cb_func)(ACT_STATUS status, void *arg),
                                       void *arg);

  /**
   * @brief Start BroadcastSearch's DeviceDiscovery procedure for OPC UA
   *
   * @param project_id
   * @param dev_discovery_cfg
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartDeviceDiscovery(qint64 &project_id, ActDeviceDiscoveryConfig &dev_discovery_cfg,
                                       std::future<void> signal_receiver, void (*cb_func)(ACT_STATUS status, void *arg),
                                       void *arg);

  /**
   * @brief The callback function of BroadcastSearch's DeviceDiscovery module
   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   * @param define_device_type
   * @param define_network_interface
   * @param restful_config
   * @param snmp_config
   * @param netconf_config
   */
  void StartDeviceDiscoveryThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                                  ActDeviceDiscoveryConfig dev_discovery_cfg);

  /**
   * @brief Start BroadcastSearch's DeviceDiscovery procedure
   *
   * @param project_id
   * @param socket
   * @param define_device_type
   * @param define_network_interface
   * @param restful_config
   * @param snmp_config
   * @param netconf_config
   * @return ACT_STATUS
   */
  ACT_STATUS StartDeviceDiscovery(qint64 &project_id, const qint64 &ws_listener_id,
                                  ActDeviceDiscoveryConfig &dev_discovery_cfg);

  /**
   * @brief The callback function of BroadcastSearch's RetryConnect module for OPC UA
   *
   * @param project_id
   * @param retry_connect_cfg
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartRetryConnectThread(qint64 project_id, ActRetryConnectConfig retry_connect_cfg,
                                    std::future<void> signal_receiver, void (*cb_func)(ACT_STATUS status, void *arg),
                                    void *arg);

  /**
   * @brief Start BroadcastSearch's RetryConnect procedure for OPC UA
   *
   * @param project_id
   * @param retry_connect_cfg
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartRetryConnect(qint64 &project_id, ActRetryConnectConfig &retry_connect_cfg,
                                    std::future<void> signal_receiver, void (*cb_func)(ACT_STATUS status, void *arg),
                                    void *arg);

  /**
   * @brief The callback function of BroadcastSearch's RetryConnect module
   *
   * @param retry_connect_cfg
   * @param netconf_config
   */
  void StartRetryConnectThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                               ActRetryConnectConfig retry_connect_cfg);

  /**
   * @brief  Start BroadcastSearch's RetryConnect procedure
   *
   * @param project_id
   * @param socket
   * @param retry_connect_cfg
   * @return ACT_STATUS
   */
  ACT_STATUS StartRetryConnect(qint64 &project_id, const qint64 &ws_listener_id,
                               ActRetryConnectConfig &retry_connect_cfg);

  /**
   * @brief  The callback function of BroadcastSearch's LinkSequenceDetect module for OPC UA
   *
   * @param project_id
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartLinkSequenceDetectThread(qint64 project_id, std::future<void> signal_receiver,
                                          void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief Start BroadcastSearch's LinkSequenceDetect procedure for OPC UA
   *
   * @param project_id
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartLinkSequenceDetect(qint64 &project_id, std::future<void> signal_receiver,
                                          void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
     * @brief The callback function of BroadcastSearch's LinkSequenceDetect module

   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   * @param from_broadcast_search
   */
  void StartLinkSequenceDetectThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                                     QList<qint64> select_dev_id_list, bool from_broadcast_search);

  /**
   * @brief  Start BroadcastSearch's LinkSequenceDetect procedure
   *
   * @param project_id
   * @param socket
   * @param dev_id_list
   * @param from_broadcast_search
   * @return ACT_STATUS
   */
  ACT_STATUS StartLinkSequenceDetect(qint64 &project_id, const qint64 &ws_listener_id,
                                     QList<qint64> &select_dev_id_list, bool from_broadcast_search);

  /**
   * @brief The callback function of DeviceConfiguration's IpConfiguration module for OPC UA
   *
   * @param project_id
   * @param dev_ip_config_list
   * @param from_broadcast_search
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartIpConfigurationThread(qint64 project_id, QList<ActDeviceIpConfiguration> dev_ip_config_list,
                                       bool from_broadcast_search, std::future<void> signal_receiver,
                                       void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief Start DeviceConfiguration's IpConfiguration procedure for OPC UA
   *
   * @param project_id
   * @param dev_ip_config_list
   * @param from_broadcast_search
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartIpConfiguration(qint64 &project_id, QList<ActDeviceIpConfiguration> &dev_ip_config_list,
                                       bool from_broadcast_search, std::future<void> signal_receiver,
                                       void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief The callback function of DeviceConfiguration's IpConfiguration module
   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   * @param dev_ip_config_list
   * @param from_broadcast_search
   */
  void StartIpConfigurationThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                                  QList<ActDeviceIpConfiguration> dev_ip_config_list, bool from_broadcast_search);

  /**
   * @brief Start DeviceConfiguration's IpConfiguration procedure
   *
   * @param project_id
   * @param socket
   * @param dev_ip_config_list
   * @param from_broadcast_search
   * @return ACT_STATUS
   */
  ACT_STATUS StartIpConfiguration(qint64 &project_id, const qint64 &ws_listener_id,
                                  QList<ActDeviceIpConfiguration> &dev_ip_config_list, bool from_broadcast_search);

  /**
   * @brief The callback function of DeviceConfiguration's IpConfiguration module for OPC UA
   *
   * @param project_id
   * @param dev_id_list
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartConfigRebootThread(qint64 project_id, QList<qint64> dev_id_list, std::future<void> signal_receiver,
                                    void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief Start DeviceConfiguration's IpConfiguration procedure for OPC UA
   *
   * @param project_id
   * @param dev_id_list
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartConfigReboot(qint64 &project_id, QList<qint64> &dev_id_list, std::future<void> signal_receiver,
                                    void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief The callback function of DeviceConfiguration's Reboot module
   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   */
  void StartConfigRebootThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                               QList<qint64> dev_id_list);

  /**
   * @brief Start DeviceConfiguration's Reboot procedure
   *
   * @param project_id
   * @param socket
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartConfigReboot(qint64 &project_id, const qint64 &ws_listener_id, QList<qint64> &dev_id_list);

  /**
   * @brief The callback function of DeviceConfiguration's Locator module
   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   * @param duration
   */
  void StartConfigLocatorThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                                QList<qint64> dev_id_list, quint16 duration);

  /**
   * @brief Start DeviceConfiguration's Locator procedure
   *
   * @param project_id
   * @param socket
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartConfigLocator(qint64 &project_id, const qint64 &ws_listener_id, QList<qint64> &dev_id_list,
                                quint16 &duration);

  /**
   * @brief The callback function of Operation's EventLog module
   *
   * @param project_id
   * @param dev_id_list
   * @param export_path
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartOperationEventLogThread(qint64 project_id, QList<qint64> dev_id_list, QString export_path,
                                         std::future<void> signal_receiver,
                                         void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief Start Operation's EventLog procedure
   *
   * @param project_id
   * @param dev_id_list
   * @param export_path
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartOperationEventLog(qint64 &project_id, QList<qint64> &dev_id_list, QString &export_path,
                                         std::future<void> signal_receiver,
                                         void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief The callback function of Operation's EventLog module
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   */
  void StartOperationEventLogThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                                    QList<qint64> dev_id_list);

  /**
   * @brief Start Operation's EventLog procedure
   *
   * @param project_id
   * @param ws_listener_id
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartOperationEventLog(qint64 &project_id, const qint64 &ws_listener_id, QList<qint64> &dev_id_list);

  /**
   * @brief The callback function of DeviceConfiguration's IpConfiguration module for OPC UA
   *
   * @param project_id
   * @param dev_id_list
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartConfigFactoryDefaultThread(qint64 project_id, QList<qint64> dev_id_list,
                                            std::future<void> signal_receiver,
                                            void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief Start DeviceConfiguration's IpConfiguration procedure for OPC UA
   *
   * @param project_id
   * @param dev_id_list
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartConfigFactoryDefault(qint64 &project_id, QList<qint64> &dev_id_list,
                                            std::future<void> signal_receiver,
                                            void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief The callback function of DeviceConfiguration's FactoryDefault module
   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   */
  void StartConfigFactoryDefaultThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                       qint64 project_id, QList<qint64> dev_id_list);

  /**
   * @brief Start DeviceConfiguration's FactoryDefault procedure
   *
   * @param project_id
   * @param socket
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartConfigFactoryDefault(qint64 &project_id, const qint64 &ws_listener_id, QList<qint64> &dev_id_list);

  /**
   * @brief The callback function of DeviceConfiguration's IpConfiguration module for OPC UA
   *
   * @param project_id
   * @param dev_id_list
   * @param firmware_name
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartConfigFirmwareUpgradeThread(qint64 project_id, QList<qint64> dev_id_list, QString firmware_name,
                                             std::future<void> signal_receiver,
                                             void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief Start DeviceConfiguration's IpConfiguration procedure for OPC UA
   *
   * @param project_id
   * @param dev_id_list
   * @param firmware_name
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartConfigFirmwareUpgrade(qint64 &project_id, QList<qint64> &dev_id_list, QString &firmware_name,
                                             std::future<void> signal_receiver,
                                             void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief The callback function of DeviceConfiguration's FirmwareUpgrade module
   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   * @param firmware_name
   */
  void StartConfigFirmwareUpgradeThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                        qint64 project_id, QList<qint64> dev_id_list, QString firmware_name);

  /**
   * @brief Start DeviceConfiguration's FirmwareUpgrade procedure
   *
   * @param project_id
   * @param socket
   * @param dev_id_list
   * @param firmware_name
   * @return ACT_STATUS
   */
  ACT_STATUS StartConfigFirmwareUpgrade(qint64 &project_id, const qint64 &ws_listener_id, QList<qint64> &dev_id_list,
                                        QString &firmware_name);

  /**
   * @brief The callback function of DeviceConfiguration's EnableSnmp module
   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   */
  void StartConfigEnableSnmpThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                                   QList<qint64> dev_id_list);

  /**
   * @brief Start DeviceConfiguration's EnableSnmp procedure
   *
   * @param project_id
   * @param socket
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartConfigEnableSnmp(qint64 &project_id, const qint64 &ws_listener_id, QList<qint64> &dev_id_list);

  /**
   * @brief The callback function of DeviceConfiguration's IpConfiguration module for OPC UA
   *
   * @param project_id
   * @param dev_id_list
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartExportDeviceConfigThread(qint64 project_id, QList<qint64> dev_id_list, QString export_path,
                                          std::future<void> signal_receiver,
                                          void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief Start DeviceConfiguration's IpConfiguration procedure for OPC UA
   *
   * @param project_id
   * @param dev_id_list
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartExportDeviceConfig(qint64 &project_id, QList<qint64> &dev_id_list, QString &export_path,
                                          std::future<void> signal_receiver,
                                          void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief The callback function of DeviceConfiguration's Export module
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   * @param path
   */
  void StartExportDeviceConfigThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                                     QList<qint64> dev_id_list);

  /**
   * @brief Start DeviceConfiguration's Export procedure
   *
   * @param project_id
   * @param ws_listener_id
   * @param dev_id_list
   * @param path
   * @return ACT_STATUS
   */
  ACT_STATUS StartExportDeviceConfig(qint64 &project_id, const qint64 &ws_listener_id, QList<qint64> &dev_id_list);

  /**
   * @brief The callback function of DeviceConfiguration's IpConfiguration module for OPC UA
   *
   * @param project_id
   * @param dev_id_list
   * @param signal_receiver
   * @param cb_func
   * @param arg
   */
  void OpcUaStartImportDeviceConfigThread(qint64 project_id, QList<qint64> dev_id_list, QString import_path,
                                          std::future<void> signal_receiver,
                                          void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief Start DeviceConfiguration's IpConfiguration procedure for OPC UA
   *
   * @param project_id
   * @param dev_id_list
   * @param signal_receiver
   * @param cb_func
   * @param arg
   * @return ACT_STATUS
   */
  ACT_STATUS OpcUaStartImportDeviceConfig(qint64 &project_id, QList<qint64> &dev_id_list, QString &import_path,
                                          std::future<void> signal_receiver,
                                          void (*cb_func)(ACT_STATUS status, void *arg), void *arg);

  /**
   * @brief The callback function of DeviceConfiguration's Import module
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   * @param path
   */
  void StartImportDeviceConfigThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                                     QList<qint64> dev_id_list);

  /**
   * @brief Start DeviceConfiguration's Import procedure
   *
   * @param project_id
   * @param ws_listener_id
   * @param dev_id_list
   * @param path
   * @return ACT_STATUS
   */
  ACT_STATUS StartImportDeviceConfig(qint64 &project_id, const qint64 &ws_listener_id, QList<qint64> &dev_id_list);

  /**
   * @brief The callback function of Intelligent DeviceConfiguration's Export module
   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   * @param dev_ip_list
   * @param duration
   */
  void StartIntelligentConfigExportThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                          qint64 project_id, QList<QString> dev_ip_list, QString path);

  /**
   * @brief Start Intelligent DeviceConfiguration's Export procedure
   *
   * @param project_id
   * @param socket
   * @param dev_ip_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartIntelligentConfigExport(qint64 &project_id, const qint64 &ws_listener_id, QList<QString> &dev_ip_list,
                                          QString &path);

  /**
   * @brief The callback function of Intelligent DeviceConfiguration's Import module
   *
   * @param socket
   * @param signal_receiver
   * @param project_id
   * @param dev_ip_list
   * @param duration
   */
  void StartIntelligentConfigImportThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                          qint64 project_id, QList<QString> dev_ip_list, QString path);

  /**
   * @brief Start Intelligent DeviceConfiguration's Import procedure
   *
   * @param project_id
   * @param socket
   * @param dev_ip_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartIntelligentConfigImport(qint64 &project_id, const qint64 &ws_listener_id, QList<QString> &dev_ip_list,
                                          QString &path);

  /**
   * @brief The callback function of AutoProbe's ProbeDeviceProfile module
   *
   * @param socket
   * @param signal_receiver
   * @param scan_ip_ranges
   */
  void StartProbeDeviceProfileThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                     QList<ActScanIpRangeEntry> scan_ip_ranges);

  /**
   * @brief Start AutoProbe's ProbeDeviceProfile procedure
   *
   * @param socket
   * @param scan_ip_ranges
   * @return ACT_STATUS
   */
  ACT_STATUS StartProbeDeviceProfile(const qint64 &ws_listener_id, QList<ActScanIpRangeEntry> &scan_ip_ranges);

  /**
   * @brief The callback function of DeviceConfiguration's Commission module
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   * @param config_type
   */
  void StartDeviceConfigCommissionThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                         qint64 project_id, QList<qint64> dev_id_list,
                                         ActDeviceConfigTypeEnum config_type);

  /**
   * @brief Start DeviceConfiguration's Commission procedure
   *
   * @param project_id
   * @param ws_listener_id
   * @param dev_id_list
   * @param config_type
   * @return ACT_STATUS
   */
  ACT_STATUS StartDeviceConfigCommission(qint64 &project_id, const qint64 &ws_listener_id,
                                         const QList<qint64> &dev_id_list, const ActDeviceConfigTypeEnum &config_type);

  /**
   * @brief The callback function of DeviceCommandLine's module
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   * @param dev_id_list
   * @param command
   */
  void StartDeviceCommandLineThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                                    QList<qint64> dev_id_list, QString command);

  /**
   * @brief Start DeviceCommandLine's procedure
   *
   * @param project_id
   * @param ws_listener_id
   * @param dev_id_list
   * @param command
   * @return ACT_STATUS
   */
  ACT_STATUS StartDeviceCommandLine(qint64 &project_id, const qint64 &ws_listener_id, const QList<qint64> &dev_id_list,
                                    const QString &command);

  /***********************
   *  Monitor Management  *
   * *********************/

  /**
   * @brief The callback function of monitor module
   *
   * @param ws_listener_id
   * @param signal_receiver
   * @param project_id
   * @param command
   */
  void StartMonitorThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, const qint64 project_id,
                          const ActStartMonitorWSCommand &command);

  /**
   * @brief Start monitor procedure
   *
   * @param project_id
   * @param ws_listener_id
   * @param command
   * @return ACT_STATUS
   */
  ACT_STATUS StartMonitor(qint64 project_id, const qint64 ws_listener_id, const ActStartMonitorWSCommand &command);

  /**
   * @brief Stop monitor procedure
   *
   * @param project_id
   * @param ws_listener_id
   * @param op_code_enum
   * @param save_to_project
   * @return ACT_STATUS
   */
  ACT_STATUS StopMonitor(qint64 project_id, const qint64 ws_listener_id, const ActWSCommandEnum &op_code_enum);

  /**
   * @brief Get the Monitor Project SFP List object
   *
   * @param project_id
   * @param sfp_list
   * @return ACT_STATUS
   */
  ACT_STATUS GetMonitorSFPList(const qint64 &project_id, ActSFPList &sfp_list);

  /**
   * @brief Get the Device Basic Status object
   *
   * @param project_id
   * @param device_id
   * @param basic_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceBasicStatus(const qint64 &project_id, const qint64 &device_id,
                                  ActMonitorBasicStatus &basic_status);

  /**
   * @brief Get the Device SFP Status object
   *
   * @param project_id
   * @param device_id
   * @param sfp_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceSFPStatus(const qint64 &project_id, const qint64 &device_id,
                                ActMonitorDeviceSFPStatus &sfp_status);

  /**
   * @brief Get the Device Basic Information object
   *
   * @param device_ip
   * @param basic_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetMonitorDeviceBasicInfo(const QString &device_ip, ActMonitorDeviceBasicInfo &basic_status);

  /**
   * @brief Get the Device SFP Status object
   *
   * @param device_ip
   * @param sfp_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetMonitorDeviceSFPStatus(const QString &device_ip, ActMonitorDeviceSFPStatus &sfp_status);

  /**
   * @brief Get the Device Port Status object
   *
   * @param project_id
   * @param device_id
   * @param port_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetDevicePortStatus(const qint64 &project_id, const qint64 &device_id,
                                 ActMonitorDevicePortStatus &port_status);

  /**
   * @brief Get the Device Port Status object
   *
   * @param device_ip
   * @param port_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetMonitorDevicePortStatus(const QString &device_ip, ActMonitorDevicePortStatus &port_status);

  /**
   * @brief Get the Device Traffic Status object
   *
   * @param project_id
   * @param device_id
   * @param traffic_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceTrafficStatus(const qint64 &project_id, const qint64 &device_id,
                                    ActMonitorDeviceTrafficStatus &traffic_status);

  /**
   * @brief Get the Device Traffic Status object
   *
   * @param device_id
   * @param traffic_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetMonitorDeviceTrafficStatus(const QString &device_ip, ActMonitorDeviceTrafficStatus &traffic_status);

  /*****************************************
   *  Device RESTful token Map Management  *
   * ***************************************/

  /**
   * @brief Get the RESTful token Map object
   *
   * @param mac_host_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetRESTfulTokenMap(QMap<qint64, QString> &restful_token_map);

  /**
   * @brief Get the RESTful token object
   *
   * @param mac_host_map
   * @return ACT_STATUS
   */
  ACT_STATUS SetRESTfulTokenMap(const QMap<qint64, QString> &restful_token_map);

  /**
   * @brief Get the Device RESTful Token Map object
   *
   * @param device_id
   * @param token
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceRESTfulTokenMap(const qint64 &device_id, QString &token);

  /**
   * @brief Update the Device RESTful Token Map object
   *
   * @param device_id
   * @param token
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceRESTfulTokenMap(const qint64 &device_id, const QString &token);

  /****************************
   *  MacHost Map Management  *
   * *************************/

  /**
   * @brief Get the Mac Host Map object
   *
   * @param mac_host_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetMacHostMap(QMap<QString, QString> &mac_host_map);

  /**
   * @brief Get the Mac Host Map object
   *
   * @param mac_host_map
   * @return ACT_STATUS
   */
  ACT_STATUS SetMacHostMap(const QMap<QString, QString> &mac_host_map);

  /**
   * @brief Get the Project Device config password Map object
   *
   * @param project_id
   * @param dev_ip_conn_cfg_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetProjectDevIpConnCfgMap(const qint64 &project_id,
                                       QMap<qint64, ActDeviceIpConnectConfig> &dev_ip_conn_cfg_map);

  /**
   * @brief Update the Project Device config password Map object
   *
   * @param project_id
   * @param dev_ip_conn_cfg_map
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProjectDevIpConnCfgMap(const qint64 &project_id,
                                          const QMap<qint64, ActDeviceIpConnectConfig> &dev_ip_conn_cfg_map);

  /******************************************
   *  SNMP global resource Management  *
   * ****************************************/

  /**
   * @brief Init the SNMP global resource object
   *
   */
  ACT_STATUS InitSnmpGlobalResource();

  /**
   * @brief Clear the SNMP global resource object
   *
   */
  ACT_STATUS ClearSnmpGlobalResource();

  /****************************
   *  Firmware Management  *
   * *************************/

  /**
   * @brief Upload firmware object
   *
   * @param firmware
   * @return ACT_STATUS
   */
  ACT_STATUS UploadFirmware(ActFirmware &firmware);

  /**
   * @brief Delete firmware object
   *
   * @param id
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteFirmware(const qint64 &id);

  /***********************
   *  QuaZip Compressor  *
   * *********************/

  /**
   * @brief CompressFolder object
   *
   * @param folderPath
   * @param zipFilePath
   * @return ACT_STATUS
   */
  ACT_STATUS CompressFolder(const QString &folderPath, const QString &zipFilePath);

  /**
   * @brief ReadFileContent object
   *
   * @param filePath
   * @param fileContent
   * @return ACT_STATUS
   */
  ACT_STATUS ReadFileContent(const QString &filePath, QByteArray &fileContent);

  /**
   * @brief UnZipFile object
   *
   * @param zip_file_path
   * @param destination_path
   * @return ACT_STATUS
   */
  ACT_STATUS UnZipFile(const QString &zip_file_path, const QString &destination_path);

  /******************************************
   *  Connection Config Management Template *
   * *************
   * ***************************/

  /**
   * @brief Copy ConnectionConfigField from src to dest
   *
   * @tparam DestT
   * @tparam SrcT
   * @param dest_item
   * @param src_item
   * @return ACT_STATUS
   */
  template <typename DestT, typename SrcT>
  ACT_STATUS CopyConnectionConfigField(DestT &dest_item, SrcT &src_item) {
    ACT_STATUS_INIT();

    dest_item.SetAccount(src_item.GetAccount());
    dest_item.SetNetconfConfiguration(src_item.GetNetconfConfiguration());
    dest_item.SetRestfulConfiguration(src_item.GetRestfulConfiguration());
    dest_item.SetSnmpConfiguration(src_item.GetSnmpConfiguration());

    return act_status;
  }
  /**
   * @brief Fulfill the empty field for the configuration
   *
   * @tparam T
   * @tparam OldT
   * @tparam DefaultT
   * @param item
   * @param old_item
   * @param default_item
   * @return ACT_STATUS
   */
  template <typename T, typename OldT, typename DefaultT>
  ACT_STATUS HandleConnectionConfigField(T &item, OldT &old_item, DefaultT &default_item) {
    ACT_STATUS_INIT();

    // Account
    HandleDeviceAccount(item.GetAccount(), old_item.GetAccount(), default_item.GetAccount());

    // NETCONF
    HandleNetconfConfiguration(item.GetNetconfConfiguration(), default_item.GetNetconfConfiguration());

    // RESTFul
    HandleRestfulConfiguration(item.GetRestfulConfiguration(), default_item.GetRestfulConfiguration());

    // SNMP
    HandleSnmpConfiguration(item.GetSnmpConfiguration(), old_item.GetSnmpConfiguration(),
                            default_item.GetSnmpConfiguration());

    return act_status;
  }

  /**
  Check connection fields for the configuration
   *
   * @tparam T
   * @param item
   * @return ACT_STATUS
   */
  template <typename T>
  ACT_STATUS CheckConnectionConfigField(T &item, QString item_name = "") {
    ACT_STATUS_INIT();

    // Check Account when the Username length not empty
    if (item.GetAccount().GetPassword().length() > 0) {
      act_status = CheckDeviceAccount(item.GetAccount(), item_name);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "Check the Account failed";
        return act_status;
      }
    }

    // Check SNMP when the ReadCommunity or Username length not empty
    // SNMPv1 & SNMPv2c
    if (item.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV2c ||
        item.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
      if (item.GetSnmpConfiguration().GetReadCommunity().length() > 0) {
        act_status = CheckSnmpConfiguration(item.GetSnmpConfiguration(), item_name);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Check the SnmpConfiguration failed";
          return act_status;
        }
      }
    }

    // SNMPv3
    if (item.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV3) {
      // AuthenticationType == none
      // or AuthenticationPassword.length > 0
      if ((item.GetSnmpConfiguration().GetAuthenticationType() == ActSnmpAuthenticationTypeEnum::kNone) ||
          (item.GetSnmpConfiguration().GetAuthenticationPassword().length() > 0)) {
        act_status = CheckSnmpConfiguration(item.GetSnmpConfiguration(), item_name);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Check the SnmpConfiguration failed";
          return act_status;
        }
      }
    }

    return act_status;
  }

  /**
  Compare connection fields for the configuration
   *
   * @tparam T
   * @param item
   * @return ACT_STATUS
   */
  template <typename T>
  bool CompareConnectionConfigConsistent(T item1, T item2) {
    // Check Account
    if (!ActDeviceAccount::CompareConnectConsistent(item1.GetAccount(), item2.GetAccount())) {
      return false;
    }

    // Check SNMP
    if (!ActSnmpConfiguration::CompareConnectConsistent(item1.GetSnmpConfiguration(), item2.GetSnmpConfiguration())) {
      return false;
    }

    // Check RESTful
    if (!ActRestfulConfiguration::CompareConnectConsistent(item1.GetRestfulConfiguration(),
                                                           item2.GetRestfulConfiguration())) {
      return false;
    }

    // Compare NETCONF configuration
    if (!ActNetconfConfiguration::CompareConnectConsistent(item1.GetNetconfConfiguration(),
                                                           item2.GetNetconfConfiguration())) {
      return false;
    }

    // Check Enable SNMP
    if (item1.GetEnableSnmpSetting() != item2.GetEnableSnmpSetting()) {
      return false;
    }

    return true;
  }

  /******************************************
   * Websocket and Opc Ua message listener  *
   * ****************************************/

  /**
  Send message or temp messages object to web socket and opcua
   *
   * @param ws_type  ActWSTypeEnum: kSystem = 1, kProject = 2, kSpecified = 3
   * @param send_tmp {bool} to send all messages in notification_tmp
   * @tparam T notification message, if send_tmp is true, message type can be ActBaseResponse
   * @param id id may be project id or ws listener id, it depends on ws_type to decide
   * @return ACT_STATUS
   */
  template <typename T>
  ACT_STATUS SendMessageToListener(const ActWSTypeEnum &ws_type, bool send_tmp, [[maybe_unused]] T &message,
                                   qint64 listener_id = -1) {
    ACT_STATUS_INIT();

    // execute all actions in notification_tmp
    if (send_tmp) {
      auto local_notification_tmp = this->notification_tmp_;

      // Devices
      for (auto msg : local_notification_tmp.GetDeviceUpdateMsgs()) {
        act_status = this->SendMessageToListener(ws_type, false, msg, listener_id);
      }

      // Links
      for (auto msg : local_notification_tmp.GetLinkUpdateMsgs()) {
        act_status = this->SendMessageToListener(ws_type, false, msg, listener_id);
      }

      // Streams
      for (auto msg : local_notification_tmp.GetStreamUpdateMsgs()) {
        act_status = this->SendMessageToListener(ws_type, false, msg, listener_id);
      }

      return act_status;
    }

    if (true /* this->GetLicense().GetFeature().GetOpcua() */) {
      auto op_code = message.GetOpCode();  // qint64

      if (op_code == static_cast<qint64>(ActWSCommandEnum::kPatchUpdate)) {
        auto action = message.GetAction();  // Action: Create/Delete/Update
        auto data = message.GetData();      // Data Type: ActDevice, ActLink...

        switch (action) {
          case (ActPatchUpdateActionEnum::kCreate):
            act_status = this->AddToOpcua(listener_id, data);
            break;

          case (ActPatchUpdateActionEnum::kDelete):
            act_status = this->DeleteToOpcua(listener_id, data);
            break;

          case (ActPatchUpdateActionEnum::kUpdate):
            act_status = this->UpdateToOpcua(listener_id, data);
            break;

          default:  // error handle
            QString error_msg = QString("notification action type unknown.");
            qCritical() << error_msg.toStdString().c_str();
            return std::make_shared<ActBadRequest>(error_msg);
        }
      }
    }

    if ((message.GetSyncToWebsocket()) /* &&  (this->GetLicense().GetFeature().GetHttps()) */) {
      QString message_str = message.ToString(message.key_order_).toStdString().c_str();
      // TODO: clear delete data (by reorder key_order_)

      switch (ws_type) {
        case (ActWSTypeEnum::kSystem):
          act_status = this->SendMessageToSystemWSListeners(message_str);
          break;

        case (ActWSTypeEnum::kProject):
          act_status = this->SendMessageToProjectWSListeners(listener_id, message_str);
          break;

        case (ActWSTypeEnum::kSpecified):
          act_status = this->SendMessageToWSListener(listener_id, message_str);
          break;

        default:  // error handle
          QString error_msg = QString("WebSocket type unknown.");
          qCritical() << error_msg.toStdString().c_str();
          return std::make_shared<ActBadRequest>(error_msg);
      }
    }

    return act_status;
  }

  /**
   Send message pointer to web socket
    *
    * @param ws_type  ActWSTypeEnum: kSystem = 1, kProject = 2, kSpecified = 3
    * @tparam T notification message, if send_tmp is true, message can be ActProject
    * @param id id may be project id or ws listener id, it depends on ws_type to decide
    * @return ACT_STATUS
    */
  template <typename T>
  ACT_STATUS SendMessageToListener([[maybe_unused]] const ActWSTypeEnum &ws_type, [[maybe_unused]] bool send_tmp,
                                   [[maybe_unused]] std::shared_ptr<T> &message,
                                   [[maybe_unused]] qint64 listener_id = -1) {
    ACT_STATUS_INIT();

    QString message_str = message->ToString(message->key_order_).toStdString().c_str();

    if (true /* this->GetLicense().GetFeature().GetHttps()*/) {
      switch (ws_type) {
        case (ActWSTypeEnum::kSystem):
          act_status = this->SendMessageToSystemWSListeners(message_str);
          break;

        case (ActWSTypeEnum::kProject):
          act_status = this->SendMessageToProjectWSListeners(listener_id, message_str);
          break;

        case (ActWSTypeEnum::kSpecified):
          act_status = this->SendMessageToWSListener(listener_id, message_str);
          break;

        default:  // error handle
          QString error_msg = QString("WebSocket type unknown.");
          qCritical() << error_msg.toStdString().c_str();
          return std::make_shared<ActBadRequest>(error_msg);
      }
    }

    // can expand opcua part

    return act_status;
  }

  /**
  Send add message to opcua
   *
   * @param id project id
   * @tparam T data to opcua function
   * @return ACT_STATUS
   */

  template <typename T>
  ACT_STATUS AddToOpcua(qint64 &id, const T &data) {
    ACT_STATUS_INIT();

    if constexpr (std::is_same_v<T, ActProject>) {  // data type == ActProject
                                                    // create to opcua
                                                    // [bugfix:2619] copy project and opc ua only create project object
      act_status = MoxaOpcUaClassBased::updateOpcUaProject(data);

      // error message
      if (!IsActStatusSuccess(act_status)) {
        QString error_msg = QString("Create project on OPC UA server failed.");
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    } else if constexpr (std::is_same_v<T, ActDeviceProfile>) {  // data type == ActDeviceProfile
                                                                 // create to opcua
      act_status = MoxaOpcUaClassBased::uploadOpcUaDeviceProfile(data);

      // error message
      if (!IsActStatusSuccess(act_status)) {
        QString error_msg = QString("Upload device profile to OPC UA server failed.");
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }

    return act_status;
  }

  /**
  Send delete message to opcua
   *
   * @param id project id
   * @tparam T data to opcua function
   * @return ACT_STATUS
   */
  template <typename T>
  ACT_STATUS DeleteToOpcua(qint64 &id, const T &data) {
    ACT_STATUS_INIT();

    if constexpr (std::is_same_v<T, ActProject>) {  // data type == ActProject
                                                    // delete to opcua
      act_status = MoxaOpcUaClassBased::removeOpcUaProject(data);

      // error message
      if (!IsActStatusSuccess(act_status)) {
        QString error_msg = QString("Delete project on OPC UA server failed.");
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    } else if constexpr (std::is_same_v<T, ActDeviceProfile>) {  // data type == ActDeviceProfile
                                                                 // delete to opcua
      act_status = MoxaOpcUaClassBased::removeOpcUaDeviceProfile(data);

      // error message
      if (!IsActStatusSuccess(act_status)) {
        QString error_msg = QString("Delete device profile to OPC UA server failed.");
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }

    return act_status;
  }

  /**
  Send update message to opcua
   *
   * @param id project id
   * @tparam T data to opcua function
   * @return ACT_STATUS
   */
  template <typename T>
  ACT_STATUS UpdateToOpcua(qint64 &id, const T &data) {
    ACT_STATUS_INIT();

    if constexpr (std::is_same_v<T, ActProject>) {  // data type == ActProject
                                                    // update to opcua
      act_status = MoxaOpcUaClassBased::updateOpcUaProject(data);

      // error message
      if (!IsActStatusSuccess(act_status)) {
        QString error_msg = QString("Update project on OPC UA server failed.");
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }

    return act_status;
  }

  /************************************
   *  Event Logs  *
   * **********************************/
  /**
   * @brief Get event logs
   *
   * @param limit
   * @param offset
   * @param response
   * @return ACT_STATUS
   */
  ACT_STATUS GetEventLogs(qint32 limit, qint32 offset, ActEventLogsResponse &response);
  /**
   * @brief get event logs and save as a csv file to specified file path
   *
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS SaveEventsAsCsv(const QString &file_path);
  /**
   * @brief Get syslogs
   *
   * @param query_data
   * @param response
   * @return ACT_STATUS
   */
  ACT_STATUS GetSyslogs(ActSyslogQueryData &query_data, ActSyslogsResponse &response);
  /**
   * @brief get specified syslogs and save as a csv file to specified file path
   *
   * @param query_data
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS SaveSyslogsAsCsv(ActSyslogQueryData &query_data, const QString &file_path);
  /**
   * @brief get all syslogs and save as a csv file to specified file path
   *
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS SaveSyslogsAsCsv(const QString &file_path);

  /**
   * @brief Delete all syslogs
   *
   * @param response
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteSyslogs(ActDeleteSyslogsResponse &response);
  /**
   * @brief Get syslog server configuration
   *
   * @param response
   * @return ACT_STATUS
   */
  ACT_STATUS GetSyslogConfiguration(ActSyslogGetConfiguration &response);
  /**
   * @brief Set syslog server configuration
   *
   * @param request
   * @param response
   * @return ACT_STATUS
   */
  ACT_STATUS SetSyslogConfiguration(ActSyslogPutConfiguration &request, ActSyslogPutConfiguration &response);
};

extern ActCore g_core;

}  // namespace core
}  // namespace act
