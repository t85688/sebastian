/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"

enum class ActVlanPortTypeEnum { kAccess = 1, kTrunk = 2, kHybrid = 3 };

static const QMap<QString, ActVlanPortTypeEnum> kActVlanPortTypeEnumMap = {
    {"Access", ActVlanPortTypeEnum::kAccess},
    {"Trunk", ActVlanPortTypeEnum::kTrunk},
    {"Hybrid", ActVlanPortTypeEnum::kHybrid}};  ///< The the string mapping map of the ActVlanPortTypeEnum

enum class ActVlanPriorityEnum { kNonTSN = 1, kTSNUser = 2, kTSNSystem = 3 };

static const QMap<QString, ActVlanPriorityEnum> kActVlanPriorityEnumMap = {
    {"NonTSN", ActVlanPriorityEnum::kNonTSN},
    {"TSNUser", ActVlanPriorityEnum::kTSNUser},
    {"TSNSystem", ActVlanPriorityEnum::kTSNSystem}};  ///< The the string mapping map of the ActVlanPriorityEnum

class ActVlanPortTypeEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActVlanPriorityEnum, vlan_priority, VlanPriority);  ///< VlanPriority enum item

  ACT_JSON_FIELD(qint64, port_id, PortId);
  ACT_JSON_ENUM(ActVlanPortTypeEnum, vlan_port_type, VlanPortType);

 public:
  /**
   * @brief Construct a new Act Vlan Port Type Entry object
   *
   */
  ActVlanPortTypeEntry() {
    this->vlan_priority_ = ActVlanPriorityEnum::kNonTSN;
    this->port_id_ = -1;
    this->vlan_port_type_ = ActVlanPortTypeEnum::kAccess;
  }

  /**
   * @brief Construct a new Act Vlan Port Type Entry object
   *
   * @param port_id
   */
  ActVlanPortTypeEntry(const qint64 &port_id) : ActVlanPortTypeEntry() { this->port_id_ = port_id; }

  /**
   * @brief Construct a new Act Vlan Port Type Entry object
   *
   * @param port_id
   * @param vlan_port_type
   */
  ActVlanPortTypeEntry(const qint64 &port_id, const ActVlanPortTypeEnum &vlan_port_type,
                       const ActVlanPriorityEnum &vlan_priority)
      : ActVlanPortTypeEntry() {
    this->port_id_ = port_id;
    this->vlan_port_type_ = vlan_port_type;
    this->vlan_priority_ = vlan_priority;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActVlanPortTypeEntry &x) {
    return qHash(x.port_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActVlanPortTypeEntry &x, const ActVlanPortTypeEntry &y) {
    return x.port_id_ == y.port_id_;
  }
};

/**
 * @brief The VLAN's VlanStaticEntry class
 * MIB  Q-BRIDGE-MIB
 * OID  1.3.6.1.2.1.17.7.1.4.3.1
 * PATH
 * .iso.org.dod.internet.mgmt.mib-2.dot1dBridge.qBridgeMIB.qBridgeMIBObjects.dot1qVlan.dot1qVlanStaticTable.dot1qVlanStaticEntry
 */
class ActVlanStaticEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActVlanPriorityEnum, vlan_priority, VlanPriority);  ///< VlanPriority enum item

  ACT_JSON_FIELD(qint32, vlan_id, VlanId);                                ///< VlanId item
  ACT_JSON_FIELD(QString, name, Name);                                    ///< Name item
  ACT_JSON_QT_SET(qint64, untagged_ports, UntaggedPorts);                 ///< UntaggedPorts item
  ACT_JSON_QT_SET(qint64, egress_ports, EgressPorts);                     ///< EgressPorts item
  ACT_JSON_QT_SET(qint64, forbidden_egress_ports, ForbiddenEgressPorts);  ///< ForbiddenEgressPorts item
  ACT_JSON_FIELD(qint32, row_status, RowStatus);                          ///< RowStatus item
  ACT_JSON_FIELD(bool, te_mstid, TeMstid);                                ///< Te-Mstid item
  // ROWSTATUS INTEGER {active(1), notInService(2), notReady(3), createAndGo(4), createAndWait(5), destroy(6)}
  // skip(0) keep for Chamberlain use. SNMP skip to set createAndGo(4) & active(1)

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Vlan Static Entry object
   *
   */
  ActVlanStaticEntry() {
    this->vlan_priority_ = ActVlanPriorityEnum::kNonTSN;
    this->vlan_id_ = -1;
    this->name_ = "";
    this->row_status_ = 1;
    this->te_mstid_ = false;
  }

  /**
   * @brief Construct a new Vlan Static Entry object
   *
   * @param vlan_id
   */
  ActVlanStaticEntry(const qint32 &vlan_id) : ActVlanStaticEntry() {
    this->vlan_id_ = vlan_id;
    this->name_ = QString("v%1").arg(vlan_id);
  }

  /**
   * @brief Construct a new Vlan Static Entry object
   *
   * @param vlan_priority
   * @param vlan_id
   * @param name
   * @param egress_port
   * @param row_status
   * @param untag
   * @param te_mstid
   */
  ActVlanStaticEntry(const ActVlanPriorityEnum &vlan_priority, const qint32 &vlan_id, const QString &name,
                     const qint64 &egress_port, const qint32 &row_status, const bool &untag, const bool &te_mstid)
      : ActVlanStaticEntry() {
    this->vlan_priority_ = vlan_priority;
    this->vlan_id_ = vlan_id;
    this->name_ = name;
    this->row_status_ = row_status;
    this->egress_ports_.insert(egress_port);
    if (untag) {
      this->untagged_ports_.insert(egress_port);
    } else {
      this->untagged_ports_.remove(egress_port);
    }
    this->te_mstid_ = te_mstid;
  }

  /**
   * @brief Construct a new Vlan Static Entry object
   *
   * @param vlan_priority
   * @param vlan_id
   * @param name
   * @param egress_ports
   * @param row_status
   * @param te_mstid
   */
  ActVlanStaticEntry(const ActVlanPriorityEnum &vlan_priority, const qint32 &vlan_id, const QString &name,
                     const QSet<qint64> &egress_ports, const qint32 &row_status, const bool &te_mstid)
      : ActVlanStaticEntry() {
    this->vlan_priority_ = vlan_priority;
    this->vlan_id_ = vlan_id;
    this->name_ = name;
    this->row_status_ = row_status;
    this->egress_ports_ = egress_ports;
    this->te_mstid_ = te_mstid;
  }

  /**
   * @brief Insert the EgressPort to the InsertEgressPorts set
   *
   * @param egress_port
   */
  void InsertEgressPorts(const qint64 &egress_port) { egress_ports_.insert(egress_port); }

  /**
   * @brief Insert the UntaggedPort to the InsertUntaggedPorts set
   *
   * @param untagged_port
   */
  void InsertUntaggedPorts(const qint64 &untagged_port) { untagged_ports_.insert(untagged_port); }

  /**
   * @brief Insert the UntaggedPort to the InsertUntaggedPorts set
   *
   * @param untagged_port
   */
  void RemoveUntaggedPorts(const qint64 &untagged_port) { untagged_ports_.remove(untagged_port); }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActVlanStaticEntry &x) {
    // return qHash(QString::number(x.vlan_id_, 10) + x.name_, 0);  // arbitrary value
    return qHash(x.vlan_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActVlanStaticEntry &x, const ActVlanStaticEntry &y) { return x.vlan_id_ == y.vlan_id_; }
};

class ActPortVlanEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActVlanPriorityEnum, vlan_priority, VlanPriority);  ///< VlanPriority enum item

  ACT_JSON_FIELD(qint64, port_id, PortId);
  ACT_JSON_FIELD(quint16, pvid, PVID);

 public:
  /**
   * @brief Construct a new Act Vlan Port Type Entry object
   *
   */
  ActPortVlanEntry() {
    this->vlan_priority_ = ActVlanPriorityEnum::kNonTSN;
    this->port_id_ = -1;
    this->pvid_ = ACT_VLAN_INIT_PVID;
  }

  /**
   * @brief Construct a new Act Vlan Port Type Entry object
   *
   * @param port_id
   */
  ActPortVlanEntry(const qint64 &port_id) : ActPortVlanEntry() { this->port_id_ = port_id; }

  /**
   * @brief Construct a new Act Vlan Port Type Entry object
   *
   * @param port_id
   * @param pvid
   */
  ActPortVlanEntry(const qint64 &port_id, const quint16 &pvid, const ActVlanPriorityEnum &vlan_priority)
      : port_id_(port_id), pvid_(pvid), vlan_priority_(vlan_priority) {}

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActPortVlanEntry &x) {
    return qHash(x.port_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActPortVlanEntry &x, const ActPortVlanEntry &y) { return x.port_id_ == y.port_id_; }
};