/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_device.hpp"
#include "act_json.hpp"
#include "act_link.hpp"
#include "act_status.hpp"

/**
 * @brief The ACT status type
 *
 */
enum class ActDeviceMapStatus {
  kSuccess = 1,   // Success (Found Mapping device & ModelName correctly)
  kWarning = 2,   // Warning
  kChecked = 3,   // EndStation Checked
  kFailed = 4,    // Found Mapping Device but the ModelName not correctly
  kNotFound = 5,  // Not Found the Mapping Device
  kSkip = 6       // Skip to Mapping online device(Not MOXA device)
};

/**
 * @brief The mapping table for ActDeviceMapStatus
 *
 */
static const QMap<QString, ActDeviceMapStatus> kActDeviceMapStatusMap = {
    {"Success", ActDeviceMapStatus::kSuccess},    {"Warning", ActDeviceMapStatus::kWarning},
    {"Checked", ActDeviceMapStatus::kChecked},    {"Failed", ActDeviceMapStatus::kFailed},
    {"Not Found", ActDeviceMapStatus::kNotFound}, {"Skip", ActDeviceMapStatus::kSkip}};

class ActMapTopology : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDevice, devices, Devices);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActLink, links, Links);
  ACT_JSON_FIELD(qint64, source_device_id, SourceDeviceId);

 public:
  ActMapTopology() { source_device_id_ = -1; }

  ActMapTopology(const QList<ActDevice> &devices, const QList<ActLink> &links) : ActMapTopology() {
    devices_ = devices;
    links_ = links;
  }

  ActMapTopology(const QList<ActDevice> &devices, const QList<ActLink> &links, const qint64 &source_device_id) {
    devices_ = devices;
    links_ = links;
    source_device_id_ = source_device_id;
  }
};

class ActMapDeviceResultItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);

  // Offline device information
  ACT_JSON_FIELD(qint64, offline_device_id, OfflineDeviceId);
  ACT_JSON_FIELD(QString, offline_ip_address, OfflineIpAddress);
  ACT_JSON_FIELD(QString, offline_model_name, OfflineModelName);

  // Online device information
  ACT_JSON_FIELD(qint64, online_device_id, OnlineDeviceId);
  ACT_JSON_FIELD(QString, online_ip_address, OnlineIpAddress);
  ACT_JSON_FIELD(QString, online_model_name, OnlineModelName);
  ACT_JSON_FIELD(QString, online_mac_address, OnlineMacAddress);
  ACT_JSON_FIELD(QString, online_serial_number, OnlineSerialNumber);
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActDeviceSimpleEthernetModule, online_ethernet_module,
                           OnlineEthernetModule);  ///< The Ethernet module map <SlotID, SimpleEthernetModule>
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActDeviceSimplePowerModule, online_power_module,
                           OnlinePowerModule);  ///< The Ethernet module map <SlotID, SimplePowerModule>

  // Result information
  ACT_JSON_ENUM(ActDeviceMapStatus, status, Status);
  ACT_JSON_FIELD(QString, error_message, ErrorMessage);

 private:
  void UpdateOfflineDevice(const ActDevice &offline_device) {
    offline_device_id_ = offline_device.GetId();
    offline_ip_address_ = offline_device.GetIpv4().GetIpAddress();
    offline_model_name_ = offline_device.GetDeviceProperty().GetModelName();
  }

  void UpdateOnlineDevice(const ActDevice &online_device, const bool &built_in_power) {
    online_device_id_ = online_device.GetId();
    online_ip_address_ = online_device.GetIpv4().GetIpAddress();
    online_model_name_ = online_device.GetDeviceProperty().GetModelName();
    online_mac_address_ = online_device.GetMacAddress();
    online_serial_number_ = online_device.GetDeviceInfo().GetSerialNumber();

    for (auto slot_id : online_device.modular_info_.GetEthernet().keys()) {
      // Skip not use slot by modular configuration
      if (!online_device.GetModularConfiguration().GetEthernet().contains(slot_id)) {
        continue;
      }
      online_ethernet_module_[slot_id] =
          ActDeviceSimpleEthernetModule(online_device.modular_info_.GetEthernet()[slot_id]);
    }

    if (!built_in_power) {
      for (auto slot_id : online_device.modular_info_.GetPower().keys()) {
        // Skip not use slot by modular configuration
        if (!online_device.GetModularConfiguration().GetPower().contains(slot_id)) {
          continue;
        }
        online_power_module_[slot_id] = ActDeviceSimplePowerModule(online_device.modular_info_.GetPower()[slot_id]);
      }
    }
  }

 public:
  ActMapDeviceResultItem() {
    id_ = -1;

    offline_device_id_ = -1;
    offline_ip_address_ = "";
    offline_model_name_ = "";

    online_device_id_ = -1;
    online_ip_address_ = "";
    online_model_name_ = "";
    online_mac_address_ = "";
    online_serial_number_ = "";

    status_ = ActDeviceMapStatus::kNotFound;
    error_message_ = "";
  }

  ActMapDeviceResultItem(const qint64 &id) : ActMapDeviceResultItem() { id_ = id; }

  ActMapDeviceResultItem(const qint64 &id, const ActDevice &offline_device, const ActDevice &online_device,
                         const bool &built_in_power, const ActDeviceMapStatus &status)
      : ActMapDeviceResultItem() {
    id_ = id;
    UpdateOfflineDevice(offline_device);
    UpdateOnlineDevice(online_device, built_in_power);
    status_ = status;
  }

  ActMapDeviceResultItem(const qint64 &id, const ActDevice &online_device, const bool &built_in_power,
                         const ActDeviceMapStatus &status, const QString &error_message)
      : ActMapDeviceResultItem() {
    id_ = id;

    UpdateOnlineDevice(online_device, built_in_power);
    error_message_ = error_message;

    status_ = status;
  }
  ActMapDeviceResultItem(const ActDevice &offline_device, const ActDevice &online_device, const bool &built_in_power,
                         const ActDeviceMapStatus &status, const QString &error_message)
      : ActMapDeviceResultItem() {
    UpdateOfflineDevice(offline_device);
    UpdateOnlineDevice(online_device, built_in_power);

    status_ = status;
    error_message_ = error_message;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActMapDeviceResultItem &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
    return 0;
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActMapDeviceResultItem &x, const ActMapDeviceResultItem &y) { return x.id_ == y.id_; }

  // /**
  //  * @brief The comparison operator for std:set & std:sort
  //  *
  //  * @param x
  //  * @param y
  //  * @return true
  //  * @return false
  //  */
  friend bool operator<(const ActMapDeviceResultItem &x, const ActMapDeviceResultItem &y) {
    // status > offline_ip(offline_ip != 0) > online_mac

    // status
    if (x.status_ != y.status_) {
      return x.status_ < y.status_;
    }

    // offline_ip(offline_ip != 0)
    quint32 x_offline_ip_int = 0;
    quint32 y_offline_ip_int = 0;
    if (!x.offline_ip_address_.isEmpty()) {
      ActIpv4::AddressStrToNumber(x.offline_ip_address_, x_offline_ip_int);
    }
    if (!y.offline_ip_address_.isEmpty()) {
      ActIpv4::AddressStrToNumber(y.offline_ip_address_, y_offline_ip_int);
    }
    if (x_offline_ip_int != y_offline_ip_int) {
      return x_offline_ip_int < y_offline_ip_int;
    }

    // online_mac
    qint64 x_online_mac_int = 0;
    qint64 y_online_mac_int = 0;
    if (!x.online_mac_address_.isEmpty()) {
      MacAddressToQInt64(x.online_mac_address_, x_online_mac_int);
    }
    if (!y.online_mac_address_.isEmpty()) {
      MacAddressToQInt64(y.online_mac_address_, y_online_mac_int);
    }
    return x_online_mac_int < y_online_mac_int;
  }
};

class ActScanDeviceResultItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);

  // Offline device information
  ACT_JSON_FIELD(QString, offline_ip_address, OfflineIpAddress);
  ACT_JSON_FIELD(qint64, offline_device_id, OfflineDeviceId);

  // Online device information
  // ACT_JSON_FIELD(qint64, online_device_id, OnlineDeviceId);
  ACT_JSON_FIELD(QString, online_ip_address, OnlineIpAddress);
  ACT_JSON_FIELD(QString, online_model_name, OnlineModelName);
  ACT_JSON_FIELD(QString, online_serial_number, OnlineSerialNumber);
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActDeviceSimpleEthernetModule, online_ethernet_module,
                           OnlineEthernetModule);  ///< The Ethernet module map <SlotID, SimpleEthernetModule>
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActDeviceSimplePowerModule, online_power_module,
                           OnlinePowerModule);  ///< The Ethernet module map <SlotID, SimplePowerModule>

  // Result information
  ACT_JSON_ENUM(ActDeviceMapStatus, result, Result);
  ACT_JSON_FIELD(QString, error_message, ErrorMessage);  ///< The ErrorMessage

 public:
  ActScanDeviceResultItem() {
    id_ = -1;

    offline_ip_address_ = "";
    offline_device_id_ = -1;

    // online_device_id_ = -1;
    online_ip_address_ = "";
    online_model_name_ = "";
    online_serial_number_ = "";
    result_ = ActDeviceMapStatus::kNotFound;
  }

  ActScanDeviceResultItem(const ActMapDeviceResultItem &map_device_result) {
    id_ = map_device_result.GetId();

    offline_ip_address_ = map_device_result.GetOfflineIpAddress();
    offline_device_id_ = map_device_result.GetOfflineDeviceId();

    // online_device_id_ = map_device_result.GetOnlineDeviceId();
    online_ip_address_ = map_device_result.GetOnlineIpAddress();
    online_model_name_ = map_device_result.GetOnlineModelName();
    online_serial_number_ = map_device_result.GetOnlineSerialNumber();
    online_ethernet_module_ = map_device_result.GetOnlineEthernetModule();
    online_power_module_ = map_device_result.GetOnlinePowerModule();

    result_ = map_device_result.GetStatus();
    error_message_ = map_device_result.GetErrorMessage();
  }
};

class ActTopologyMappingResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActMapDeviceResultItem, mapping_report, MappingReport);

  ACT_JSON_FIELD(bool, deploy, Deploy);  ///< Deploy available status

 public:
  ActTopologyMappingResult() { deploy_ = false; }
};

class ActScanMappingResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActScanDeviceResultItem, scan_mapping_report, ScanMappingReport);

  ACT_JSON_FIELD(bool, deploy, Deploy);

 public:
  ActScanMappingResult() { deploy_ = false; }

  ActScanMappingResult(const ActTopologyMappingResult &topology_mapping_result) {
    deploy_ = topology_mapping_result.GetDeploy();
    for (auto map_dev_item : topology_mapping_result.GetMappingReport()) {
      scan_mapping_report_.append(ActScanDeviceResultItem(map_dev_item));
    }
  }
};
