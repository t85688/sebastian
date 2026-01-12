/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"
#include "topology/act_link.hpp"

/**
 * @brief The Stream's Interface class
 * 8021qcc 2018: 46.2.3.3
 *
 */
class ActInterface : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(qint64, interface_id, InterfaceId);
  ACT_JSON_FIELD(QString, interface_name, InterfaceName);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_FIELD(QString, mac_address, MacAddress);
  ACT_JSON_FIELD(QString, ip_address, IpAddress);
  ACT_JSON_FIELD(bool, active, Active);
  ACT_JSON_FIELD(bool, used, Used);
  ACT_JSON_FIELD(bool, management, Management);
  ACT_JSON_FIELD(bool, root_guard, RootGuard);

  // From Device Profile
  // ACT_JSON_FIELD(quint16, gate_control_list_length,
  //                GateControlListLength);
  // ACT_JSON_FIELD(quint16, number_of_queue, NumberOfQueue);
  // ACT_JSON_FIELD(quint16, tick_granularity, TickGranularity);
  // ACT_JSON_FIELD(quint16, stream_priority_config_ingress_index_max,
  //                StreamPriorityConfigIngressIndexMax);

  ACT_JSON_COLLECTION(QList, qint64, support_speeds, SupportSpeeds);
  ACT_JSON_QT_SET_ENUM(ActCableTypeEnum, cable_types, CableTypes);

 public:
  qint64 mac_address_int;

 public:
  /**
   * @brief Construct a new End Station Interface object
   *
   */
  ActInterface() {
    active_ = true;
    used_ = false;
    management_ = false;
    root_guard_ = false;
    interface_id_ = -1;
    mac_address_int = 0;
    device_id_ = -1;
  }

  /**
   * @brief Construct a new Act End Station Interface object
   *
   * @param interface_id
   */
  ActInterface(qint64 &interface_id) : ActInterface() { interface_id_ = interface_id; }

  /**
   * @brief Construct a new End Station Interface object
   *
   * @param interface_name
   * @param mac_address
   */
  ActInterface(const QString &interface_name, const QString &mac_address) : ActInterface() {
    interface_name_ = interface_name;
    mac_address_ = mac_address;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActInterface &x) {
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
  friend bool operator==(const ActInterface &x, const ActInterface &y) { return x.interface_id_ == y.interface_id_; }

  // /**
  //  * @brief The comparison operator for std:set & std:sort
  //  *
  //  * @param x
  //  * @param y
  //  * @return true
  //  * @return false
  //  */
  friend bool operator<(const ActInterface &x, const ActInterface &y) { return x.interface_id_ < y.interface_id_; }
};

inline bool InterfaceIdThan(const ActInterface &d1, const ActInterface &d2) {
  return d1.GetInterfaceId() < d2.GetInterfaceId();  // sort by interface id
}
