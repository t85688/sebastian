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

/**
 * @brief The supported series of module enum class
 *
 */
enum class ActModuleSupportedSeriesEnum {
  kTSN,
  kSDS,
  kTWS,
  kPT_7528,
  kPT_G500,
  kRKS,
  kEDS_G4000,
  kMDS,
  kEDS_2000,
  kEDS_200A,
  kEDS_500E,
  kEDS_G2000,
  kEDS_G205,
  kEDS_G200A,
  kEDS_G300,
  kEDS_G500E,
  kIKS_6700A,
  kSDS_G3000
};

/**
 * @brief The QMap module supported series enum mapping
 *
 */
static const QMap<QString, ActModuleSupportedSeriesEnum> kActModuleSupportedSeriesEnumMap = {
    {"TSN", ActModuleSupportedSeriesEnum::kTSN},
    {"SDS", ActModuleSupportedSeriesEnum::kSDS},
    {"TWS", ActModuleSupportedSeriesEnum::kTWS},
    {"PT-7528", ActModuleSupportedSeriesEnum::kPT_7528},
    {"PT-G500", ActModuleSupportedSeriesEnum::kPT_G500},
    {"RKS", ActModuleSupportedSeriesEnum::kRKS},
    {"EDS-(G)4000", ActModuleSupportedSeriesEnum::kEDS_G4000},
    {"MDS", ActModuleSupportedSeriesEnum::kMDS},
    {"EDS-2000", ActModuleSupportedSeriesEnum::kEDS_2000},
    {"EDS-200A", ActModuleSupportedSeriesEnum::kEDS_200A},
    {"EDS-500E", ActModuleSupportedSeriesEnum::kEDS_500E},
    {"EDS-G2000", ActModuleSupportedSeriesEnum::kEDS_G2000},
    {"EDS-G205", ActModuleSupportedSeriesEnum::kEDS_G205},
    {"EDS-G200A", ActModuleSupportedSeriesEnum::kEDS_G200A},
    {"EDS-G300", ActModuleSupportedSeriesEnum::kEDS_G300},
    {"EDS-G500E", ActModuleSupportedSeriesEnum::kEDS_G500E},
    {"IKS-6700A", ActModuleSupportedSeriesEnum::kIKS_6700A},
    {"SDS-(G)3000", ActModuleSupportedSeriesEnum::kSDS_G3000}};

class ActOperatingVoltage : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qreal, min, Min);
  ACT_JSON_FIELD(qreal, max, Max);

 public:
  ActOperatingVoltage() {
    this->min_ = 0;
    this->max_ = 0;
  };
};

class ActVDC : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, supported, Supported);
  ACT_JSON_COLLECTION(QList, qint32, input, Input);
  ACT_JSON_OBJECT(ActOperatingVoltage, operating_voltage, OperatingVoltage);

 public:
  ActVDC() { this->supported_ = false; };
};

class ActVAC : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, supported, Supported);
  ACT_JSON_COLLECTION(QList, qint32, input, Input);
  ACT_JSON_OBJECT(ActOperatingVoltage, operating_voltage, OperatingVoltage);

 public:
  ActVAC() { this->supported_ = false; }
};

class ActPowerModule : public QSerializer {
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
  ACT_JSON_FIELD(bool, built_in, BuiltIn);
  ACT_JSON_ENUM(ActModuleSupportedSeriesEnum, series, Series);
  ACT_JSON_FIELD(bool, redundant_input, RedundantInput);
  ACT_JSON_FIELD(bool, poe, PoE);
  ACT_JSON_FIELD(bool, dying_gasp, DyingGasp);
  ACT_JSON_OBJECT(ActVDC, vdc, VDC);
  ACT_JSON_OBJECT(ActVAC, vac, VAC);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new power module object
   *
   */
  ActPowerModule() {
    this->id_ = -1;
    this->icon_name_ = "";
    this->data_version_ = ACT_POWER_MODULE_DATA_VERSION;
    this->module_name_ = "";
    this->purchasable_ = false;
    this->module_type_ = "";
    this->description_ = "";
    this->profiles_.append(ActServiceProfileForDeviceProfileEnum::kSelfPlanning);
    this->built_in_ = false;
    this->series_ = ActModuleSupportedSeriesEnum::kRKS;
    this->redundant_input_ = false;
    this->poe_ = false;
    this->dying_gasp_ = false;
    this->key_order_ = QList<QString>(
        {QString("Id"), QString("IconName"), QString("DataVersion"), QString("ModuleName"), QString("Purchasable"),
         QString("ModuleType"), QString("Description"), QString("Profiles"), QString("BuiltIn"), QString("Series"),
         QString("RedundantInput"), QString("PoE"), QString("DyingGasp"), QString("VDC"), QString("VAC")});
  }

  /**
   * @brief Construct a new power module object
   *
   * @param id
   */
  ActPowerModule(const qint64 &id) : ActPowerModule() { this->id_ = id; }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActPowerModule &x) { return x.id_; }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActPowerModule &x, const ActPowerModule &y) { return x.id_ == y.id_; }
};
