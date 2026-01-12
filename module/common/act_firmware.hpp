/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_device.hpp"
#include "act_json.hpp"

class ActFirmware : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(qint64, id, Id);  ///< The unique id of the firmware
  ACT_JSON_FIELD(QString, data_version, DataVersion);
  ACT_JSON_FIELD(QString, firmware_name, FirmwareName);       ///< The name of the firmware
  ACT_JSON_FIELD(QString, model_name, ModelName);             ///< ModelName item
  ACT_JSON_ENUM(ActDeviceTypeEnum, device_type, DeviceType);  ///< DeviceType enum item

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new Act Firmware object
   *
   */
  ActFirmware() {
    this->id_ = -1;
    this->data_version_ = ACT_FIRMWARE_DATA_VERSION;
    this->firmware_name_ = "";
    this->model_name_ = "";
    this->device_type_ = ActDeviceTypeEnum::kTSNSwitch;

    this->key_order_.append(QList<QString>(
        {QString("Id"), QString("DataVersion"), QString("FirmwareName"), QString("ModelName"), QString("DeviceType")}));
  }

  /**
   * @brief Construct a new Act Firmware object
   *
   * @param id
   */
  ActFirmware(const qint64 &id) : ActFirmware() { this->id_ = id; }

  /**
   * @brief Construct a new Act Firmware object
   *
   * @param id
   * @param firmware_name
   */
  ActFirmware(const qint64 &id, const QString &firmware_name) : ActFirmware() {
    this->id_ = id;
    this->firmware_name_ = firmware_name;
  }

  /**
   * @brief Construct a new Act Firmware object
   *
   * @param firmware_name
   */
  ActFirmware(const QString &firmware_name) : ActFirmware() { this->firmware_name_ = firmware_name; }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActFirmware &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActFirmware &x, const ActFirmware &y) {
    return x.id_ == y.id_ || x.firmware_name_ == y.firmware_name_;
  }
};
