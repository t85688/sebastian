/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"
#include "act_system.hpp"
#include "deploy_entry/act_vlan_static_entry.hpp"

/**
 * @brief The ACT vlan-config class
 *
 */
class ActVlanConfig : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, vlan_id, VlanId);
  ACT_JSON_ENUM(ActVlanPortTypeEnum, port_type, PortType);
  ACT_JSON_QT_SET(qint64, devices, Devices);
  ACT_JSON_QT_SET(qint64, ports, Ports);
  ACT_JSON_FIELD(qint16, pvid, Pvid);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act vlan-view object
   *
   */
  ActVlanConfig() {
    this->key_order_.append(QList<QString>(
        {QString("VlanId"), QString("PortType"), QString("Devices"), QString("Ports"), QString("Pvid")}));
    this->vlan_id_ = -1;
    this->port_type_ = ActVlanPortTypeEnum::kAccess;
    this->pvid_ = ACT_VLAN_INIT_PVID;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActVlanConfig &obj) {
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
  friend bool operator==(const ActVlanConfig &x, const ActVlanConfig &y) { return x.vlan_id_ == y.vlan_id_; }
};