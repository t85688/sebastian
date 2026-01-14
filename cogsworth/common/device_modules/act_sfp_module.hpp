/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include <QDebug>

#include "act_interface.hpp"
#include "act_json.hpp"
#include "act_status.hpp"
#include "act_system.hpp"
#include "act_temperature.hpp"
#include "json_utils.hpp"

/**
 * @brief The SFP mode enum class
 *
 */
enum class ActSFPModeEnum { kSingle, kMulti };

/**
 * @brief The QMap SFP mode enum mapping
 *
 */
static const QMap<QString, ActSFPModeEnum> kActSFPModeEnumMap = {{"Single", ActSFPModeEnum::kSingle},
                                                                 {"Multi", ActSFPModeEnum::kMulti}};

/**
 * @brief The SFP connector type enum class
 *
 */
enum class ActSFPConnectorTypeEnum { kLC, kRJ45 };

/**
 * @brief The QMap SFP connector type enum mapping
 *
 */
static const QMap<QString, ActSFPConnectorTypeEnum> kActSFPConnectorTypeEnumMap = {
    {"LC", ActSFPConnectorTypeEnum::kLC}, {"RJ45", ActSFPConnectorTypeEnum::kRJ45}};

/**
 * @brief The Act Wave length in nanometer class
 *
 */
class ActWavelengthNM : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint16, tx, TX);
  ACT_JSON_FIELD(qint16, rx, RX);
};

/**
 * @brief The ActSFPModule class
 *
 */
class ActSFPModule : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, icon_name, IconName);
  ACT_JSON_FIELD(QString, data_version, DataVersion);
  ACT_JSON_FIELD(QString, module_name, ModuleName);
  ACT_JSON_FIELD(bool, purchasable, Purchasable);
  ACT_JSON_FIELD(QString, module_type, ModuleType);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_COLLECTION_ENUM(QList, ActServiceProfileForDeviceProfileEnum, profiles, Profiles);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActInterface, interfaces, Interfaces);
  ACT_JSON_FIELD(qint32, speed, Speed);
  ACT_JSON_FIELD(bool, wdm, WDM);
  ACT_JSON_COLLECTION(QList, qint64, support_speeds, SupportSpeeds);  ///< SupportSpeed items list in Mbps
  ACT_JSON_ENUM(ActSFPModeEnum, mode, Mode);
  ACT_JSON_ENUM(ActSFPConnectorTypeEnum, connector_type, ConnectorType);
  ACT_JSON_FIELD(qreal, transmission_distance_km, TransmissionDistanceKM);
  ACT_JSON_OBJECT(ActOperatingTemperatureC, operating_temperature_c, OperatingTemperatureC);
  ACT_JSON_OBJECT(ActWavelengthNM, wavelength_nm, WavelengthNM);

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new SFP module object
   *
   */
  ActSFPModule() {
    this->id_ = -1;
    this->icon_name_ = "";
    this->data_version_ = ACT_SFP_MODULE_DATA_VERSION;
    this->module_name_ = "";
    this->purchasable_ = false;
    this->module_type_ = "";
    this->description_ = "";
    this->profiles_.append(ActServiceProfileForDeviceProfileEnum::kSelfPlanning);
    this->wdm_ = false;
    this->key_order_ = QList<QString>(
        {QString("Id"), QString("IconName"), QString("DataVersion"), QString("ModuleName"), QString("Purchasable"),
         QString("ModuleType"), QString("Description"), QString("Profiles"), QString("Interfaces"), QString("Speed"),
         QString("WDM"), QString("SupportSpeeds"), QString("Mode"), QString("ConnectorType"),
         QString("TransmissionDistanceKM"), QString("OperatingTemperatureC"), QString("WavelengthNM")});
  }

  /**
   * @brief Construct a new Act SFP module object
   *
   * @param id
   */
  ActSFPModule(const qint64 &id) : ActSFPModule() { this->id_ = id; }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActSFPModule &x) { return x.id_; }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActSFPModule &x, const ActSFPModule &y) { return x.id_ == y.id_; }
};

/**
 * @brief The ActSFPModule class
 *
 */
class ActSFPCounts : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT(QMap, QString, quint32, sfp_counts, SFPCounts);
};