/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <QRandomGenerator>
#include <QVariant>

#include "act_device.hpp"

/**
 * @brief The job type enum class
 *
 */
enum class ActJobTypeEnum { kMultiplePing = 2, kScan = 3, kIdentify = 4, kScanLink = 5, kMultipleHeartbeat = 6 };

/**
 * @brief The ActJob class to receive variant data
 *
 */
class ActJob {
 public:
  ActJob() { id_ = QRandomGenerator::system()->generate(); }

  template <typename T>
  void AssignJob(const qint64 &project_id, const ActJobTypeEnum &type, T &data) {
    project_id_ = project_id;
    type_ = type;
    data_ = QVariant::fromValue(data);
  }

  qint64 GetId() { return id_; }
  qint64 GetProjectId() { return project_id_; }
  ActJobTypeEnum GetType() { return type_; }
  QVariant GetData() { return data_; }

 private:
  qint64 id_;
  qint64 project_id_;
  ActJobTypeEnum type_;
  QVariant data_;  // QVariant to store different types of data
};

/**
 * @brief The ActPingJob class
 *
 */
class ActPingJob : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);  ///< The device id
  ACT_JSON_FIELD(QString, ip, Ip);              ///< The ICMP

  ACT_JSON_OBJECT(ActDeviceAccount, account, Account);                           ///< Device connection account item
  ACT_JSON_OBJECT(ActSnmpConfiguration, snmp_configuration, SnmpConfiguration);  ///< SNMP configuration item
  ACT_JSON_OBJECT(ActNetconfConfiguration, netconf_configuration,
                  NetconfConfiguration);  ///< Netconf configuration item
  ACT_JSON_OBJECT(ActRestfulConfiguration, restful_configuration,
                  RestfulConfiguration);  ///< Restful configuration item

  ACT_JSON_FIELD(bool, enable_snmp_setting, EnableSnmpSetting);  ///< The Enable SnmpSetting item

 public:
  /**
   * @brief Construct a new Act Scan Range Entry object
   *
   */
  ActPingJob() {
    device_id_ = -1;
    ip_ = "";
    enable_snmp_setting_ = false;
  }

  ActPingJob(const ActScanIpRangeEntry ip_range_entry) : ActPingJob() {
    this->SetAccount(ip_range_entry.GetAccount());
    this->SetSnmpConfiguration(ip_range_entry.GetSnmpConfiguration());
    this->SetNetconfConfiguration(ip_range_entry.GetNetconfConfiguration());
    this->SetRestfulConfiguration(ip_range_entry.GetRestfulConfiguration());
    this->SetEnableSnmpSetting(ip_range_entry.GetEnableSnmpSetting());
  }

  ActPingJob(const ActDevice device) : ActPingJob() {
    this->SetDeviceId(device.GetId());
    this->SetAccount(device.GetAccount());
    this->SetSnmpConfiguration(device.GetSnmpConfiguration());
    this->SetNetconfConfiguration(device.GetNetconfConfiguration());
    this->SetRestfulConfiguration(device.GetRestfulConfiguration());
    this->SetEnableSnmpSetting(device.GetEnableSnmpSetting());
  }
};

class ActScanJob : public ActPingJob {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDevice, device, Device);                    ///< The device object
  ACT_JSON_FIELD(bool, keep_connect_status, KeepConnectStatus);  ///< The device object

 public:
  /**
   * @brief Construct a new Act Scan Range Entry object
   *
   */
  ActScanJob() { this->keep_connect_status_ = false; }

  ActScanJob(const ActDevice device) : ActPingJob() { this->SetDevice(device); }
};

class ActScanLinkJob : public ActPingJob {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDevice, device, Device);                           ///< The device object
  ACT_JSON_QT_SET_OBJECTS(ActDevice, project_devices, ProjectDevices);  ///< The project device set object

 public:
  /**
   * @brief Construct a new Act Scan Range Entry object
   *
   */
  ActScanLinkJob() {}

  ActScanLinkJob(const ActDevice device, const QSet<ActDevice> project_devices) : ActScanLinkJob() {
    this->SetDevice(device);
    this->SetProjectDevices(project_devices);
  }
};

class ActIdentifyJob : public ActPingJob {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDevice, device, Device);  ///< The device object

 public:
  /**
   * @brief Construct a new Act Scan Range Entry object
   *
   */
  ActIdentifyJob() {}

  ActIdentifyJob(const ActDevice device) : ActPingJob() { this->SetDevice(device); }
};

class ActHeartbeatJob : public ActPingJob {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDevice, device, Device);  ///< The device object

 public:
  /**
   * @brief Construct a new Act Scan Range Entry object
   *
   */
  ActHeartbeatJob() {}

  ActHeartbeatJob(const ActDevice device) : ActPingJob() { this->SetDevice(device); }
};
