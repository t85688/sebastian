/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include <QDebug>

#include "act_json.hpp"
#include "act_status.hpp"
#include "json_utils.hpp"
#include "topology/act_link.hpp"

/**
 * @brief The Device's InterfaceProperty class
 *
 */
class ActInterfaceProperty : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, interface_id, InterfaceId);
  ACT_JSON_FIELD(QString, interface_name, InterfaceName);
  ACT_JSON_COLLECTION(QList, qint64, support_speeds, SupportSpeeds);
  ACT_JSON_QT_SET_ENUM(ActCableTypeEnum, cable_types, CableTypes);

 public:
  ActInterfaceProperty() {
    this->interface_id_ = 0;
    this->interface_name_ = "";

    // SupportSpeeds
    QList<qint64> support_speeds;
    support_speeds << 10 << 100 << 1000 << 10000;
    this->support_speeds_ = support_speeds;

    // CableTypes
    QSet<ActCableTypeEnum> cable_types;
    cable_types.insert(ActCableTypeEnum::kCopper);
    cable_types.insert(ActCableTypeEnum::kFiber);
    this->cable_types_ = cable_types;
  };
  ActInterfaceProperty(const qint64 &interface_id) : ActInterfaceProperty() { this->interface_id_ = interface_id; }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActInterfaceProperty &x) {
    return qHash(x.interface_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActInterfaceProperty &x, const ActInterfaceProperty &y) {
    return x.interface_id_ == y.interface_id_;
  }
};

/**
 * @brief The ActEthernetModule class
 *
 */
class ActEthernetModule : public QSerializer {
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
  ACT_JSON_FIELD(qint16, number_of_interfaces, NumberOfInterfaces);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActInterfaceProperty, interfaces, Interfaces);

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new ethernet module object
   *
   */
  ActEthernetModule() {
    this->id_ = -1;
    this->icon_name_ = "";
    this->data_version_ = ACT_ETHERNET_MODULE_DATA_VERSION;
    this->module_name_ = "";
    this->purchasable_ = false;
    this->module_type_ = "";
    this->description_ = "";
    this->profiles_.append(ActServiceProfileForDeviceProfileEnum::kSelfPlanning);
    this->number_of_interfaces_ = 0;
    this->key_order_ =
        QList<QString>({QString("Id"), QString("IconName"), QString("DataVersion"), QString("ModuleName"),
                        QString("Purchasable"), QString("ModuleType"), QString("Description"), QString("Profiles"),
                        QString("NumberOfInterfaces"), QString("Interfaces")});
  }

  /**
   * @brief Construct a new Act ethernet module object
   *
   * @param id
   */
  ActEthernetModule(const qint64 &id) : ActEthernetModule() { this->id_ = id; }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActEthernetModule &x) { return x.id_; }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActEthernetModule &x, const ActEthernetModule &y) { return x.id_ == y.id_; }
};
