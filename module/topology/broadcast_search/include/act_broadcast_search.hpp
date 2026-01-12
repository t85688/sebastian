/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_BROADCAST_SEARCH_HPP
#define ACT_BROADCAST_SEARCH_HPP

#include <QMutex>
#include <QString>
#include <thread>

#include "act_broadcast_search_config.hpp"
#include "act_core.hpp"
#include "act_device.hpp"
#include "act_json.hpp"
#include "act_link.hpp"
#include "act_project.hpp"
#include "act_southbound.hpp"
#include "act_status.hpp"
namespace act {
namespace topology {

class ActBroadcastSearch {
  Q_GADGET

  // for thread
  std::unique_ptr<std::thread> broadcast_search_thread_;  ///< BroadcastSearch thread item
  ACT_JSON_FIELD(bool, stop_flag, StopFlag);              ///< StopFlag item
  ACT_JSON_FIELD(quint8, progress, Progress);             ///< Progress item
  ACT_STATUS broadcast_search_act_status_;                ///< BroadcastSearch thread status

  // member
  ACT_JSON_OBJECT(ActProfiles, profiles, Profiles);  ///< Profiles item

  ACT_JSON_QT_DICT(QMap, QString, QString, mac_host_map, MacHostMap);  ///< DeviceHostMap item <mac, host_ip>
  ACT_JSON_COLLECTION_OBJECTS(QList, ActDevice, devices, Devices);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActLink, links, Links);

  // QMap<QString, QString> adapter_table;
  ActSouthbound southbound_;
  QMutex mutex_;

 private:
  ACT_STATUS UpdateProgress(quint8 progress);

  /**
   * @brief Trigger DeviceDiscoveryBroadcastSearch for thread
   *
   * @param dev_discovery_cfg
   * @param result_devices
   */
  void TriggerDeviceDiscoveryBroadcastSearchForThread(ActDeviceDiscoveryConfig dev_discovery_cfg,
                                                      QList<ActDevice> &result_devices);

  /**
   * @brief Trigger RetryConnect for thread
   *
   * @param retry_connect_cfg
   * @param result_devices
   */
  void TriggerRetryConnectForThread(ActRetryConnectConfig retry_connect_cfg, QList<ActDevice> &result_devices);

  /**
   * @brief Trigger LinkDistanceDetect for thread
   *
   * @param from_broadcast_search
   * @param result_distance_entry_list
   */
  void TriggerLinkDistanceDetectForThread(bool from_broadcast_search, QList<qint64> select_dev_id_list,
                                          QList<ActDeviceDistanceEntry> &result_distance_entry_list);

  /**
   * @brief Broadcast InternalError handler object
   *
   * @param error_reason
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS BroadcastInternalErrorHandler(const QString &error_fun, const QString &error_reason,
                                           const ActDevice &device);

  /**
   * @brief Device error log handler
   *
   * @param error_fun
   * @param error_reason
   * @param device
   */
  void DeviceErrorLogHandler(const QString &error_fun, const QString &error_reason, const ActDevice &device);

  /**
   * @brief Search device object
   *
   * @param devices
   * @return ACT_STATUS
   */
  ACT_STATUS SearchDevicesHandle(QList<ActDevice> &devices);

  /**
   * @brief Search device partial infos object
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDevicePartialInfos(ActDevice &device);

  /**
   * @brief Update device object
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDevice(ActDevice &device);

  /**
   * @brief Create a Devices Path Links object
   *
   * @param config_arp_table
   * @return ACT_STATUS
   */
  ACT_STATUS CreateDevicesPathLinks(const bool &config_arp_table);

  /**
   * @brief Filter devices
   *
   * @return ACT_STATUS
   */
  ACT_STATUS LinkDistanceDetectFilterDevices();

  /**
   * @brief Split not alive devices
   *
   * @param not_alive_devices
   * @return ACT_STATUS
   */
  ACT_STATUS LinkDistanceDetectSplitNotAliveDevices(QList<ActDevice> &not_alive_devices);

  /**
   * @brief Update devices SNMP status And enable SNMP service
   *
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDevicesSnmpStatusAndEnable();

  /**
   * @brief Assign devices MacAddress by ACT arp table
   *
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDevicesMacAddressByArpTable();

 public:
  ActBroadcastSearch() {}

  /**
   * @brief Construct a new Act BroadcastSearch object
   *
   */
  ActBroadcastSearch(const ActProfiles &profiles)
      : profiles_(profiles),
        progress_(0),
        stop_flag_(false),
        broadcast_search_act_status_(std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug)),
        broadcast_search_thread_(nullptr) {
    act::core::g_core.GetMacHostMap(mac_host_map_);
    southbound_.SetProfiles(profiles);
  }

  ActBroadcastSearch(const ActProfiles &profiles, QList<ActDevice> &member_devices)
      : profiles_(profiles),
        devices_(member_devices),
        progress_(0),
        stop_flag_(false),
        broadcast_search_act_status_(std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug)),
        broadcast_search_thread_(nullptr) {
    act::core::g_core.GetMacHostMap(mac_host_map_);
    southbound_.SetProfiles(profiles);
  }

  /**
   * @brief Destroy the Act BroadcastSearch object
   *
   */
  ~ActBroadcastSearch();

  /**
   * @brief Stop thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Stop();

  /**
   * @brief Start DeviceDiscovery thread object
   *
   * @param dev_discovery_cfg
   * @param result_devices
   * @return ACT_STATUS
   */
  ACT_STATUS StartDeviceDiscovery(ActDeviceDiscoveryConfig dev_discovery_cfg, QList<ActDevice> &result_devices);

  /**
   * @brief Start RetryConnect thread object
   *
   * @param retry_connect_cfg
   * @param result_devices
   * @return ACT_STATUS
   */
  ACT_STATUS StartRetryConnect(ActRetryConnectConfig retry_connect_cfg, QList<ActDevice> &result_devices);

  /**
   * @brief Start LinkDistanceDetect thread object
   *
   * @param from_broadcast_search
   * @param result_distance_entry_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartLinkDistanceDetect(bool from_broadcast_search, QList<qint64> select_dev_id_list,
                                     QList<ActDeviceDistanceEntry> &result_distance_entry_list);

  /**
   * @brief Get the status of the BroadcastSearch thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetStatus();

  /**
   * @brief DeviceDiscovery's broadcast search object
   *
   * @param dev_discovery_cfg
   * @param result_devices
   * @return ACT_STATUS
   */
  ACT_STATUS DeviceDiscoveryBroadcastSearch(const ActDeviceDiscoveryConfig &dev_discovery_cfg,
                                            QList<ActDevice> &result_devices);

  /**
   * @brief Device's Link distance detect object
   *
   * @param from_broadcast_search
   * @param result_distance_entry_list
   * @return ACT_STATUS
   */
  ACT_STATUS LinkDistanceDetect(bool from_broadcast_search, QList<qint64> select_dev_id_list,
                                QList<ActDeviceDistanceEntry> &result_distance_entry_list);

  /**
   * @brief Compute the device distance
   *
   * @param device_list
   * @param link_list
   * @param src_device_id
   * @return ACT_STATUS
   */
  ACT_STATUS ComputeDeviceDistance(QList<ActDevice> &device_list, const QList<ActLink> &link_list,
                                   const qint64 &src_device_id);

  /**
   * @brief Retry connection object
   *
   * @param retry_connect_cfg
   * @param result_devices
   * @return ACT_STATUS
   */
  ACT_STATUS RetryConnect(const ActRetryConnectConfig &retry_connect_cfg, QList<ActDevice> &result_devices);
};

}  // namespace topology
}  // namespace act

#endif /* ACT_BROADCAST_SEARCH_HPP */