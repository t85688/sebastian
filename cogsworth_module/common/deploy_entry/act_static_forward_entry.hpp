/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"

/**
 * @brief The StaticForwardEntry class
 * MIB Q-BRIDGE-MIB & XXXX
 * OID(Unicast)    1.3.6.1.2.1.17.7.1.3.1.1 & XXXX
 * OID(Multicast) 1.3.6.1.2.1.17.7.1.3.2.1 & XXXX
 * PATH
 *
 */
class ActStaticForwardEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, vlan_id, VlanId);
  ACT_JSON_FIELD(QString, mac, MAC);
  ACT_JSON_QT_SET(qint64, egress_ports, EgressPorts);
  ACT_JSON_FIELD(qint32, dot1q_status, Dot1qStatus);

  // ACT_JSON_FIELD(qint64, ingress_port, IngressPort);   // not used
  ACT_JSON_QT_SET(qint64, forbidden_egress_ports, ForbiddenEgressPorts);  // not used
  // ACT_JSON_FIELD(qint32, storage_type, StorageType); // not used
  // ACT_JSON_FIELD(qint32, row_status, RowStatus);// not used

  // Dot1qStatus: other(1), invalid(2), permanent(3), deleteOnReset(4), deleteOnTimeout(5)
  // RowStatus: active(1) destroy(6) // for other mib

 public:
  /**
   * @brief Construct a new Static Forward Entry object
   *
   */
  ActStaticForwardEntry() {
    this->vlan_id_ = -1;
    this->dot1q_status_ = 3;
    // this->ingress_port_ = -1;
    // this->storage_type_ = 2;
    // this->row_status_ = 1;
  }

  ActStaticForwardEntry(const qint32 &vlan_id, const QString &mac) : ActStaticForwardEntry() {
    this->vlan_id_ = vlan_id;
    this->mac_ = mac;
  }

  /**
   * @brief Insert the EgressPort to the EgressPorts set
   *
   * @param egress_port
   */
  void InsertEgressPort(const qint64 &egress_port) { egress_ports_.insert(egress_port); }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActStaticForwardEntry &x) {
    return qHash(QString::number(x.vlan_id_, 10) + x.mac_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActStaticForwardEntry &x, const ActStaticForwardEntry &y) {
    return QString::number(x.vlan_id_, 10) + x.mac_ == QString::number(y.vlan_id_, 10) + y.mac_;
  }
};
