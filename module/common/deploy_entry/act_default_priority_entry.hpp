/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_system.hpp"

class ActDefaultPriorityEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, port_id, PortId);
  ACT_JSON_FIELD(quint8, default_pcp, DefaultPCP);

 public:
  /**
   * @brief Construct a new Act Vlan Port Type Entry object
   *
   */
  ActDefaultPriorityEntry() : port_id_(-1), default_pcp_(ACT_INIT_DEFAULT_PCP) {}

  /**
   * @brief Construct a new Act Vlan Port Type Entry object
   *
   */
  ActDefaultPriorityEntry(const qint64 &port_id) : ActDefaultPriorityEntry() { this->port_id_ = port_id; }

  /**
   * @brief Construct a new Act Vlan Port Type Entry object
   *
   * @param port_id
   * @param default_pcp
   */
  ActDefaultPriorityEntry(const qint32 &port_id, const quint8 &default_pcp)
      : port_id_(port_id), default_pcp_(default_pcp) {}

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActDefaultPriorityEntry &x) {
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
  friend bool operator==(const ActDefaultPriorityEntry &x, const ActDefaultPriorityEntry &y) {
    return x.port_id_ == y.port_id_;
  }
};
