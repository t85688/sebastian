/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_device.hpp"
#include "act_json.hpp"
#include "act_status.hpp"
#include "act_topology_mapping_result.hpp"

/**
 * @brief The ACT status type
 *
 */
enum class ActManufactureStatus {
  kSuccess = 1,
  kFailed = 2,
  kReady = 3,  // wait deploy
  kRemain = 4,
};

/**
 * @brief The mapping table for ActManufactureStatus
 *
 */
static const QMap<QString, ActManufactureStatus> kActManufactureStatusMap = {
    {"Success", ActManufactureStatus::kSuccess},
    {"Failed", ActManufactureStatus::kFailed},
    {"Ready", ActManufactureStatus::kReady},
    {"Remain", ActManufactureStatus::kRemain}};

class ActManufactureResultDevice : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, index, Index);
  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(QString, ip_address, IpAddress);
  ACT_JSON_FIELD(QString, device_name, DeviceName);
  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(QString, serial_number, SerialNumber);
  ACT_JSON_FIELD(QString, order, Order);

  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActDeviceSimpleEthernetModule, ethernet_module,
                           EthernetModule);  ///< The Ethernet module map <SlotID, SimpleEthernetModule>
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActDeviceSimplePowerModule, power_module,
                           PowerModule);  ///< The Ethernet module map <SlotID, SimplePowerModule>

 public:
  quint32 ip_num_;

  /**
   * @brief Construct a new Act Manufacture Result Device object
   *
   */
  ActManufactureResultDevice() {
    ip_num_ = 0;
    index_ = -1;
    device_id_ = -1;
    ip_address_ = "";
    device_name_ = "";
    model_name_ = "";
    serial_number_ = "";
    order_ = "";
  }

  ActManufactureResultDevice(const qint64 &index, const ActDevice &device) {
    index_ = index;
    device_id_ = device.GetId();
    ip_address_ = device.GetIpv4().GetIpAddress();
    device_name_ = device.GetDeviceName();
    model_name_ = device.GetDeviceProperty().GetModelName();
    serial_number_ = device.GetDeviceInfo().GetSerialNumber();

    ActIpv4::AddressStrToNumber(ip_address_, ip_num_);
  }
  void UpdateToRemainStatus() {
    serial_number_ = "";
    order_ = "";
    for (auto slot : ethernet_module_.keys()) {
      ethernet_module_[slot].SetSerialNumber("");
    }
    for (auto slot : power_module_.keys()) {
      power_module_[slot].SetSerialNumber("");
    }
  }

  friend bool operator<(const ActManufactureResultDevice &x, const ActManufactureResultDevice &y) {
    return x.ip_num_ < y.ip_num_;
  }

  friend bool operator==(const ActManufactureResultDevice &x, const ActManufactureResultDevice &y) {
    return x.device_id_ == y.device_id_;
  }
};

class ActManufactureResultItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, batch, Batch);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActManufactureResultDevice, devices, Devices);

  // Result information
  ACT_JSON_ENUM(ActManufactureStatus, status, Status);
  ACT_JSON_FIELD(qint64, time_stamp, TimeStamp);

 public:
  ActManufactureResultItem() {
    batch_ = -1;
    status_ = ActManufactureStatus::kReady;
    time_stamp_ = 0;
  }

  ActManufactureResultItem(const qint64 &batch) : ActManufactureResultItem() { batch_ = batch; }

  ActManufactureResultItem(const qint64 &batch, const QList<ActManufactureResultDevice> devices,
                           const ActManufactureStatus &status)
      : ActManufactureResultItem() {
    batch_ = batch;
    devices_ = devices;
    status_ = status;
  }
};

class ActManufactureResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, order, Order);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActManufactureResultItem, manufacture_report, ManufactureReport);
  ACT_JSON_OBJECT(ActManufactureResultItem, ready, Ready);
  ACT_JSON_OBJECT(ActManufactureResultItem, remain, Remain);

 public:
  ActManufactureResult() {
    order_ = "";
    ready_.SetStatus(ActManufactureStatus::kReady);
    remain_.SetStatus(ActManufactureStatus::kRemain);
  }

  // void Init(const QList<ActDevice> &devices) {
  //   ready_.SetStatus(ActManufactureStatus::kReady);
  //   remain_.SetStatus(ActManufactureStatus::kRemain);

  //   manufacture_report_.clear();
  //   ready_.GetDevices().clear();
  //   remain_.GetDevices().clear();

  //   // qint64 device_number = 0;
  //   for (auto device : devices) {
  //     // device_number += 1;

  //     if (device.CheckCanDeploy()) {
  //       ActManufactureResultDevice manufacture_device(-1, device);
  //       remain_.GetDevices().append(manufacture_device);
  //     }
  //   }

  //   std::sort(remain_.GetDevices().begin(), remain_.GetDevices().end());
  // }

  bool CheckIsFirstTime() { return (manufacture_report_.isEmpty() && remain_.GetDevices().isEmpty()); }
};

class ActTotalManufactureResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActManufactureResultDevice, total_report, TotalReport);
};
