/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"

class ActPortSettingEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, port_id, PortId);
  ACT_JSON_FIELD(bool, admin_status,
                 AdminStatus);  ///< The AdminStatus(enable) of the port_table

 public:
  /**
   * @brief Construct a new RSTP Port Entry object
   *
   */
  ActPortSettingEntry() {
    this->port_id_ = -1;
    this->admin_status_ = true;
  }

  /**
   * @brief Construct a new RSTP Port Entry object
   *
   * @param port_id
   */
  ActPortSettingEntry(const qint32 &port_id) : ActPortSettingEntry() { this->port_id_ = port_id; }

  /**
   * @brief Construct a new RSTP Port Entry object
   *
   * @param port_id
   * @param admin_status
   */
  ActPortSettingEntry(const qint32 &port_id, const bool &admin_status) : ActPortSettingEntry() {
    this->port_id_ = port_id;
    this->admin_status_ = admin_status;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActPortSettingEntry &x) {
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
  friend bool operator==(const ActPortSettingEntry &x, const ActPortSettingEntry &y) {
    return x.port_id_ == y.port_id_;
  }
};
