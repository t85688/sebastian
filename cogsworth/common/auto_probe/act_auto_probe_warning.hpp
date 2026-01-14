/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_device_profile.hpp"
#include "act_feature_profile.hpp"
#include "act_json.hpp"

// static const QMap<QString, QString> ActWarningReason = {
//     {"NotFoundFeatureProfile", "FeatureProfile(%1) not found"},
//     {"NotFoundAction", "Action(%1) in FeatureProfile(%2) not found"},
//     {"MustActionFailed", "The Must Action detection failed"},
//     {"AllActionsFailed", "All Actions detection failed"},
//     {"ConnectionFailed", "Device %1 connection status is False"},
//     {"DisConnectionFailed", "Device %1 disconnection"},
//     {"NotImplemented", "The Method was not implemented"},
//     {"MethodAccessFailed", "The %1 method access failed"},
//     {"YangNotSupport", "Device not support YANG(%1)"},
//     {"ValidateEmptyFailed", "Validate %1 value failed(value is empty)"},
//     {"GenerateDataFailed", "Generate %1 failed"}};

/**
 * @brief The ActionMethod Warning class
 *
 */
class ActFeatureMethodWarning : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, reason, Reason);  ///< Reason item

 public:
  /**
   * @brief Construct a new Feature Method object
   *
   */
  ActFeatureMethodWarning() { this->reason_ = ""; }

  /**
   * @brief Construct a new Feature Method Warning object
   *
   * @param reason
   */
  ActFeatureMethodWarning(const QString &reason) { this->reason_ = reason; }
};

/**
 * @brief The FeatureSubItem Warning class
 *
 */
class ActFeatureSubItemWarning : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, reason, Reason);

  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActFeatureMethodWarning, methods, Methods);  ///< The Methods map

 public:
  /**
   * @brief Construct a new Act Feature Item Warning object
   *
   */
  ActFeatureSubItemWarning() { this->reason_ = ""; }

  /**
   * @brief Construct a new Act Feature Sub Item Warning object
   *
   * @param reason
   */
  ActFeatureSubItemWarning(const QString &reason) { this->reason_ = reason; }
};

/**
 * @brief The FeatureItem Warning class
 *
 */
class ActFeatureItemWarning : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActFeatureSubItemWarning, sub_items, SubItems);  ///< The SubItems map
};

class ActFeatureWarning : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);                   ///< The unique id
  ACT_JSON_ENUM(ActFeatureEnum, feature, Feature);  ///< Feature item
  ACT_JSON_FIELD(QString, reason, Reason);
  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActFeatureItemWarning, items, Items);  ///< The Items map

 public:
  /**
   * @brief Construct a new Act Feature Warning object
   *
   */
  ActFeatureWarning() {
    this->id_ = -1;
    this->reason_ = "";
  };

  /**
   * @brief Construct a new Act Feature Warning object
   *
   * @param feature
   */
  ActFeatureWarning(const ActFeatureEnum &feature) : ActFeatureWarning() {
    this->feature_ = feature;
    this->id_ = static_cast<qint64>(feature);
  }

  /**
   * @brief Construct a new Act Feature Warning object
   *
   * @param feature
   * @param reason
   */
  ActFeatureWarning(const ActFeatureEnum &feature, const QString &reason) : ActFeatureWarning() {
    this->feature_ = feature;
    this->reason_ = reason;
    this->id_ = static_cast<qint64>(feature);
  }
};

class ActAutoProbeWarning : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // ACT_JSON_FIELD(qint64, device_profile_id, DeviceProfileId);  ///< DeviceProfileId item
  ACT_JSON_FIELD(QString, ip_address, IpAddress);  ///< IpAddress item
  ACT_JSON_FIELD(qint64, device_id, DeviceId);     ///< DeviceId item
  ACT_JSON_FIELD(QString, model_name, ModelName);  ///< ModelName item

  ACT_JSON_COLLECTION_OBJECTS(QList, ActFeatureWarning, warning_reason, WarningReason);  ///< Actions list

 public:
  /**
   * @brief Construct a new Act Auto Probe Warning object
   *
   */
  ActAutoProbeWarning() {
    // this->device_profile_id_ = -1;
    this->ip_address_ = "";
    this->device_id_ = -1;
  }
};
