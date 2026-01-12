/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_traffic.hpp"
#include "stream/act_stream_id.hpp"

/**
 * @brief The StreamPriority's StadPortEntry class
 * MIB: MOXA-STREAM-ADAPTER-MIB
 * OID: .1.3.6.1.4.1.8691.603.2.10.1.2.1
 * PATH:.iso.org.dod.internet.private.enterprises.moxa.switching.basicLayer2.mxStAd.stadConfiguration.stadPortTable.stadPortEntry
 */
class ActStadPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // base (L2)
  ACT_JSON_FIELD(qint64, port_id, PortId);                  ///< The Port ID(Port Index)
  ACT_JSON_FIELD(qint32, ingress_index, IngressIndex);      ///< The Ingress Index (0-9) (config index)
  ACT_JSON_FIELD(qint32, index_enable, IndexEnable);        ///< The Index Enable (INTEGER(true(1), false(2))) (enable)
  ACT_JSON_FIELD(qint32, vlan_id, VlanId);                  ///< The Vlan ID (2-4094) (vid)
  ACT_JSON_FIELD(qint32, vlan_pcp, VlanPcp);                ///< The Vlan PCP (0-7) (pcp)
  ACT_JSON_FIELD(qint32, ethertype_value, EthertypeValue);  ///< The Ethertype Value (ethertype)
  ACT_JSON_FIELD(qint32, subtype_enable,
                 SubtypeEnable);  ///< The Subtype Enable (frametype) (INTEGER(true(1), false(2))) (enable)
  ACT_JSON_FIELD(qint32, subtype_value, SubtypeValue);  ///< The Subtype Value (frametypevalue)

  // L3: type, tcpPort, udpPort (only RESTfulAPI)
  ACT_JSON_QT_SET_ENUM(ActStreamPriorityTypeEnum, type, Type);  ///< The Types
  ACT_JSON_FIELD(qint32, udp_port, UdpPort);                    ///< The UdpPort
  ACT_JSON_FIELD(qint32, tcp_port, TcpPort);                    ///< The TcpPort

 public:
  /**
   * @brief Construct a new Stad Port Entry object
   *
   */
  ActStadPortEntry() {
    this->port_id_ = -1;
    this->ingress_index_ = 0;
    this->index_enable_ = 2;
    this->vlan_id_ = 1;
    this->vlan_pcp_ = 0;
    this->ethertype_value_ = 0;
    this->subtype_enable_ = 2;
    this->subtype_value_ = 0;

    this->type_ = {ActStreamPriorityTypeEnum::kEthertype};
    this->udp_port_ = 1;
    this->tcp_port_ = 1;
  }

  ActStadPortEntry(const qint64 &port_id, const qint32 &ingress_index) : ActStadPortEntry() {
    this->port_id_ = port_id;
    this->ingress_index_ = ingress_index;
  }

  ActStadPortEntry(const qint64 &port_id, const qint32 &vlan_id, const qint32 &vlan_pcp, const qint32 &index_enable,
                   const qint32 &ingress_index, const qint32 &ethertype_value)
      : ActStadPortEntry() {
    this->port_id_ = port_id;
    this->vlan_id_ = vlan_id;
    this->vlan_pcp_ = vlan_pcp;
    this->index_enable_ = index_enable;
    this->ingress_index_ = ingress_index;
    this->ethertype_value_ = ethertype_value;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActStadPortEntry &x) {
    // return qHash(QString::number(x.ethertype_value_, 10) + "-" + QString::number(x.subtype_value_, 10),
    //              0);  // arbitrary value

    return qHash(QString::number(x.port_id_, 10) + "-" + QString::number(x.ingress_index_, 10), 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActStadPortEntry &x, const ActStadPortEntry &y) {
    // QString x_key = QString::number(x.ethertype_value_, 10) + "-" + QString::number(x.subtype_value_, 10);
    // QString y_key = QString::number(y.ethertype_value_, 10) + "-" + QString::number(y.subtype_value_, 10);
    // return x_key == y_key;

    QString x_key = QString::number(x.port_id_, 10) + "-" + QString::number(x.ingress_index_, 10);
    QString y_key = QString::number(y.port_id_, 10) + "-" + QString::number(y.ingress_index_, 10);
    return x_key == y_key;

    // return x.ingress_index_ == y.ingress_index_;
  }
};

class ActInterfaceStadPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, interface_id, InterfaceId);                              ///< The Interface ID
  ACT_JSON_QT_SET_OBJECTS(ActStadPortEntry, stad_port_entries, StadPortEntries);  ///< The StadPortEntry

 private:
  void CheckStadPortEntriesHasSameEntry(const ActStadPortEntry &stad_port_entry_target, bool &result) {
    QString target_entry_key = QString("%1.%2.%3.%4.%5")
                                   .arg(stad_port_entry_target.GetPortId())
                                   .arg(stad_port_entry_target.GetEthertypeValue())
                                   .arg(stad_port_entry_target.GetSubtypeValue())
                                   .arg(stad_port_entry_target.GetVlanId())
                                   .arg(stad_port_entry_target.GetVlanPcp());

    for (auto stad_port_entry : stad_port_entries_) {
      // PortId.EtherType.Subtype.vid.pcp
      QString exist_entry_key = QString("%1.%2.%3.%4.%5")
                                    .arg(stad_port_entry.GetPortId())
                                    .arg(stad_port_entry.GetEthertypeValue())
                                    .arg(stad_port_entry.GetSubtypeValue())
                                    .arg(stad_port_entry.GetVlanId())
                                    .arg(stad_port_entry.GetVlanPcp());
      if (target_entry_key == exist_entry_key) {
        result = true;
        break;
      }
    }
    result = false;
  }
  void GetSameTypeValueStadPortEntry(const ActStadPortEntry &stad_port_entry_target,
                                     QSet<ActStadPortEntry>::iterator &iter) {
    QString target_entry_key = QString("%1.%2.%3.%4.%5")
                                   .arg(stad_port_entry_target.GetPortId())
                                   .arg(stad_port_entry_target.GetEthertypeValue())
                                   .arg(stad_port_entry_target.GetSubtypeValue())
                                   .arg(stad_port_entry_target.GetVlanId())
                                   .arg(stad_port_entry_target.GetVlanPcp());

    for (iter = stad_port_entries_.begin(); iter != stad_port_entries_.end(); ++iter) {
      // PortId.EtherType.Subtype.vid.pcp
      QString exist_entry_key = QString("%1.%2.%3.%4.%5")
                                    .arg(iter->GetPortId())
                                    .arg(iter->GetEthertypeValue())
                                    .arg(iter->GetSubtypeValue())
                                    .arg(iter->GetVlanId())
                                    .arg(iter->GetVlanPcp());
      if (target_entry_key == exist_entry_key) {
        break;
      }
    }
  }

 public:
  /**
   * @brief Construct a new ActInterfaceStadPortEntry object
   *
   */
  ActInterfaceStadPortEntry() : interface_id_(-1) {}

  /**
   * @brief Construct a new ActInterfaceStadPortEntry object
   *
   * @param interface_id
   */
  ActInterfaceStadPortEntry(const qint64 &interface_id) : interface_id_(interface_id) {}

  /**
   * @brief Construct a new ActInterfaceStadPortEntry object
   *
   * @param interface_id
   * @param stad_port_entries
   */
  ActInterfaceStadPortEntry(const qint64 &interface_id, const QSet<ActStadPortEntry> &stad_port_entries)
      : interface_id_(interface_id), stad_port_entries_(stad_port_entries) {}

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActInterfaceStadPortEntry &x) {
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
  friend bool operator==(const ActInterfaceStadPortEntry &x, const ActInterfaceStadPortEntry &y) {
    return x.interface_id_ == y.interface_id_;
  }
};
