/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include <QDebug>

#include "act_json.hpp"
#include "act_status.hpp"
#include "act_system.hpp"
#include "json_utils.hpp"
#include "topology/act_device.hpp"

class ActSoftwareLicenseProfile : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, icon_name, IconName);
  ACT_JSON_FIELD(QString, data_version, DataVersion);
  ACT_JSON_FIELD(bool, purchasable, Purchasable);
  ACT_JSON_COLLECTION_ENUM(QList, ActServiceProfileForDeviceProfileEnum, profiles, Profiles);
  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(QString, description, Description);

  ACT_JSON_FIELD(bool, built_in, BuiltIn);
  ACT_JSON_FIELD(bool, hide, Hide);
  ACT_JSON_FIELD(bool, certificate, Certificate);
  ACT_JSON_FIELD(QString, vendor, Vendor);
  ACT_JSON_ENUM(ActDeviceTypeEnum, device_type, DeviceType);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new software license profile object
   *
   */
  ActSoftwareLicenseProfile() {
    this->id_ = -1;
    this->icon_name_ = "";
    this->data_version_ = ACT_SOFTWARE_LICENSE_PROFILE_DATA_VERSION;
    this->purchasable_ = false;
    this->profiles_.append(ActServiceProfileForDeviceProfileEnum::kSelfPlanning);
    this->model_name_ = "";
    this->description_ = "";

    this->built_in_ = false;
    this->hide_ = false;
    this->certificate_ = false;
    this->vendor_ = "";
    this->device_type_ = ActDeviceTypeEnum::kNetworkMgmtSoftware;
    this->key_order_ =
        QList<QString>({QString("Id"), QString("IconName"), QString("DataVersion"), QString("Purchasable"),
                        QString("Profiles"), QString("ModelName"), QString("Description"), QString("BuiltIn"),
                        QString("Hide"), QString("Certificate"), QString("Vendor"), QString("DeviceType")});
  }

  /**
   * @brief Construct a new software license profile object
   *
   * @param id
   */
  ActSoftwareLicenseProfile(const qint64 &id) : ActSoftwareLicenseProfile() { this->id_ = id; }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActSoftwareLicenseProfile &x) { return x.id_; }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActSoftwareLicenseProfile &x, const ActSoftwareLicenseProfile &y) {
    return x.id_ == y.id_;
  }
};
