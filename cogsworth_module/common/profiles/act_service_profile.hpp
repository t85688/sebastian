/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_system.hpp"
#include "act_temperature.hpp"

/**
 * @brief The L1 category of device enum class
 *
 */
enum class ActL1CategoryEnum { kAccessories, kSwitches };

/**
 * @brief The QMap for L1 category enum mapping
 *
 */
static const QMap<QString, ActL1CategoryEnum> kActL1CategoryEnumMap = {{"Accessories", ActL1CategoryEnum::kAccessories},
                                                                       {"Switches", ActL1CategoryEnum::kSwitches}};

/**
 * @brief The currency enum class
 *
 */
enum class ActCurrencyEnum { kTWD, kUSD, kEUR };

/**
 * @brief The QMap for currency enum mapping
 *
 */
static const QMap<QString, ActCurrencyEnum> kActCurrencyEnumMap = {
    {"TWD", ActCurrencyEnum::kTWD}, {"USD", ActCurrencyEnum::kUSD}, {"EUR", ActCurrencyEnum::kEUR}};

/**
 * @brief The ActSku class
 *
 */
class ActSku : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_ENUM(ActL1CategoryEnum, l1_category, L1Category);
  ACT_JSON_FIELD(QString, data_version, DataVersion);
  ACT_JSON_FIELD(QString, icon_name, IconName);
  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_OBJECT(ActOperatingTemperatureC, operating_temperature_c, OperatingTemperatureC);
  ACT_JSON_COLLECTION_ENUM(QList, ActServiceProfileForDeviceProfileEnum, profiles, Profiles);

 public:
  ActSku() {
    this->id_ = -1;
    this->l1_category_ = ActL1CategoryEnum::kAccessories;
    this->data_version_ = ACT_SKU_DATA_VERSION;
  }
};

class ActSkuWithPrice : public ActSku {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, price, Price);
  ACT_JSON_ENUM(ActCurrencyEnum, currency, Currency);

 public:
  ActSkuWithPrice() {
    this->currency_ = ActCurrencyEnum::kEUR;
    this->price_ = "N/A";
  }
};

class ActUpdateSkuQuantityRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT(QMap, QString, quint64, sku_quantity, SkuQuantity);
};

class ActSkuQuantity : public ActSku {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, quantity, Quantity);
  ACT_JSON_FIELD(qint64, in_topology_count, InTopologyCount);
  ACT_JSON_FIELD(QString, price, Price);
  ACT_JSON_ENUM(ActCurrencyEnum, currency, Currency);
  ACT_JSON_FIELD(QString, total_price, TotalPrice);

 public:
  QList<QString> key_order_;

  ActSkuQuantity() {
    this->quantity_ = 0;
    this->in_topology_count_ = 0;
    this->price_ = "N/A";
    this->currency_ = ActCurrencyEnum::kEUR;
    this->total_price_ = "N/A";

    this->key_order_.append(QList<QString>({QString("Quantity"), QString("InTopologyCount"), QString("Price"),
                                            QString("Currency"), QString("TotalPrice")}));
  }
};

class ActSkuQuantities : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActSkuQuantity, sku_quantities_map, SkuQuantitiesMap);
};

class ActSkuList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_COLLECTION(QList, QString, sku_list, SkuList);
};

class ActSkuQuantityDelta : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, QString, deleted_devices, DeletedDevices);
  ACT_JSON_COLLECTION(QList, QString, deleted_accessories, DeletedAccessories);

  ACT_JSON_QT_DICT(QMap, QString, qint64, device_delta, DeviceDelta);
  ACT_JSON_QT_DICT(QMap, QString, qint64, accessory_delta, AccessoryDelta);
};