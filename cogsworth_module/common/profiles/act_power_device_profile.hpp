/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include <QDebug>

#include "act_json.hpp"
#include "act_status.hpp"
#include "act_system.hpp"
#include "act_temperature.hpp"
#include "device_modules/act_ethernet_module.hpp"
#include "json_utils.hpp"
#include "topology/act_device.hpp"

class ActPowerDeviceProfile : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, icon_name, IconName);
  ACT_JSON_FIELD(QString, data_version, DataVersion);
  ACT_JSON_FIELD(bool, purchasable, Purchasable);
  ACT_JSON_COLLECTION_ENUM(QList, ActServiceProfileForDeviceProfileEnum, profiles, Profiles);
  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_COLLECTION_ENUM(QList, ActMountTypeEnum, mount_type, MountType);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_OBJECT(ActOperatingTemperatureC, operating_temperature_c, OperatingTemperatureC);

  ACT_JSON_FIELD(bool, built_in, BuiltIn);
  ACT_JSON_FIELD(bool, hide, Hide);
  ACT_JSON_FIELD(bool, certificate, Certificate);
  ACT_JSON_FIELD(QString, vendor, Vendor);
  ACT_JSON_ENUM(ActDeviceTypeEnum, device_type, DeviceType);

  ACT_JSON_FIELD(qint64, max_port_speed, MaxPortSpeed);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActInterfaceProperty, interfaces, Interfaces);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new power device profile object
   *
   */
  ActPowerDeviceProfile() {
    this->id_ = -1;
    this->icon_name_ = "";
    this->data_version_ = ACT_POWER_DEVICE_PROFILE_DATA_VERSION;
    this->purchasable_ = false;
    this->profiles_.append(ActServiceProfileForDeviceProfileEnum::kSelfPlanning);
    this->model_name_ = "";
    this->description_ = "";

    this->built_in_ = false;
    this->hide_ = false;
    this->certificate_ = false;
    this->vendor_ = "";
    this->device_type_ = ActDeviceTypeEnum::kPoeAccessory;
    this->key_order_ =
        QList<QString>({QString("Id"), QString("IconName"), QString("DataVersion"), QString("Purchasable"),
                        QString("Profiles"), QString("ModelName"), QString("MountType"), QString("Description"),
                        QString("OperatingTemperatureC"), QString("BuiltIn"), QString("Hide"), QString("Certificate"),
                        QString("Vendor"), QString("DeviceType"), QString("MaxPortSpeed"), QString("Interfaces")});
  }

  /**
   * @brief Construct a new power device profile object
   *
   * @param id
   */
  ActPowerDeviceProfile(const qint64 &id) : ActPowerDeviceProfile() { this->id_ = id; }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActPowerDeviceProfile &x) { return x.id_; }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActPowerDeviceProfile &x, const ActPowerDeviceProfile &y) { return x.id_ == y.id_; }
};
