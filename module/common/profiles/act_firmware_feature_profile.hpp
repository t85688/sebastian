/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_feature.hpp"
#include "act_feature_profile.hpp"
#include "act_json.hpp"
#include "act_status.hpp"

/**
 * @brief The FirmwareFeatureProfile class
 *
 */
class ActFirmwareFeatureProfile : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);  ///< The unique id
  ACT_JSON_FIELD(QString, data_version, DataVersion);
  ACT_JSON_FIELD(QString, model_name, ModelName);                                     ///< ModelName item, TSN-G5004
  ACT_JSON_QT_SET(QString, firmware_versions, FirmwareVersions);                      ///< FirmwareVersion set
  ACT_JSON_OBJECT(ActFeatureGroup, feature_group, FeatureGroup);                      ///< FeatureGroup item
  ACT_JSON_QT_SET_OBJECTS(ActFeatureProfile, feature_capability, FeatureCapability);  ///< FeatureCapability set

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act Feature Profile object
   *
   */
  ActFirmwareFeatureProfile() {
    this->id_ = -1;
    this->data_version_ = ACT_FIRMWARE_FEATURE_PROFILE_DATA_VERSION;
    this->key_order_.append(
        QList<QString>({QString("Id"), QString("DataVersion"), QString("ModelName"), QString("FirmwareVersions"),
                        QString("FeatureGroup"), QString("FeatureCapability")}));
  }

  /**
   * @brief Construct a new Act Feature Profile object
   *
   * @param feature
   */
  ActFirmwareFeatureProfile(const qint64 &id) : ActFirmwareFeatureProfile() { this->id_ = id; }

  /**
   * @brief Construct a new Act Feature Profile object
   *
   * @param feature
   */
  ActFirmwareFeatureProfile(const qint64 &id, const QString &model_name) : ActFirmwareFeatureProfile() {
    this->id_ = id;
    this->model_name_ = model_name;
  }

  /**
   * @brief Get the Method By Std Find object
   *
   * @param target_action
   * @return ACT_STATUS
   */
  ACT_STATUS GetFeatureProfile(ActFeatureProfile &feature_profile) {
    ACT_STATUS_INIT();

    // Get device profile first
    QSet<ActFeatureProfile>::const_iterator iterator = this->feature_capability_.find(feature_profile);
    if (iterator == this->feature_capability_.end()) {
      QString log_msg = QString("FeatureProfile(%1) not found in FirmwareFeatureProfile(%2)")
                            .arg(kActFeatureEnumMap.key(feature_profile.GetFeature()))
                            .arg(this->id_);
      qCritical() << log_msg;
      QString error_msg = QString("FeatureProfile(%1) at the FirmwareFeatureProfile(%2)")
                              .arg(kActFeatureEnumMap.key(feature_profile.GetFeature()))
                              .arg(this->id_);
      return std::make_shared<ActStatusNotFound>(error_msg);
    }

    feature_profile = ActFeatureProfile(*iterator);
    return act_status;
  }

  static ACT_STATUS GetFirmwareFeatureProfile(const QSet<ActFirmwareFeatureProfile> firmware_feature_profiles,
                                              const QString model_name, const QString firmware_version,
                                              ActFirmwareFeatureProfile &result_firmware_feature_profile) {
    ACT_STATUS_INIT();
    auto iterator = std::find_if(firmware_feature_profiles.begin(), firmware_feature_profiles.end(),
                                 [&model_name, &firmware_version](const ActFirmwareFeatureProfile &fw_feat_profile) {
                                   return (fw_feat_profile.GetModelName() == model_name) &&
                                          fw_feat_profile.GetFirmwareVersions().contains(firmware_version);
                                 });

    // Check find result
    if (iterator == firmware_feature_profiles.end()) {
      // Not found
      return std::make_shared<ActStatusNotFound>(
          QString("FirmwareFeatureProfile(%1:%2)").arg(model_name).arg(firmware_version));
    }

    result_firmware_feature_profile = *iterator;
    return act_status;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActFirmwareFeatureProfile &x) {
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
  friend bool operator==(const ActFirmwareFeatureProfile &x, const ActFirmwareFeatureProfile &y) {
    return (x.id_ == y.id_);
  }
};

/**
 * @brief The FirmwareFeatureProfile class
 *
 */
class ActSimpleFirmwareFeatureProfile : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);  ///< The unique id
  ACT_JSON_FIELD(QString, data_version, DataVersion);
  ACT_JSON_FIELD(QString, model_name, ModelName);                 ///< ModelName item, TSN-G5004
  ACT_JSON_QT_SET(QString, firmware_versions, FirmwareVersions);  ///< FirmwareVersion set
  ACT_JSON_OBJECT(ActFeatureGroup, feature_group, FeatureGroup);  ///< FeatureGroup item

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act Feature Profile object
   *
   */
  ActSimpleFirmwareFeatureProfile() {
    this->id_ = -1;
    this->data_version_ = ACT_FIRMWARE_FEATURE_PROFILE_DATA_VERSION;
    this->key_order_.append(QList<QString>({QString("Id"), QString("DataVersion"), QString("ModelName"),
                                            QString("FirmwareVersions"), QString("FeatureGroup")}));
  }

  /**
   * @brief Construct a new Act Simple Device Profile object
   *
   * @param profile
   */
  ActSimpleFirmwareFeatureProfile(ActFirmwareFeatureProfile &profile) {
    this->id_ = profile.GetId();
    this->data_version_ = profile.GetDataVersion();
    this->model_name_ = profile.GetModelName();
    this->firmware_versions_ = profile.GetFirmwareVersions();
    this->feature_group_ = profile.GetFeatureGroup();
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActSimpleFirmwareFeatureProfile &x) {
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
  friend bool operator==(const ActSimpleFirmwareFeatureProfile &x, const ActSimpleFirmwareFeatureProfile &y) {
    return (x.id_ == y.id_);
  }
};

class ActSimpleFirmwareFeatureProfileSet : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActSimpleFirmwareFeatureProfile, simple_firmware_feature_profile_set,
                          SimpleFirmwareFeatureProfileSet);
};
