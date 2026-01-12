/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"

/**
 * @brief The StreamPriority's StadConfigEntry class
 * MIB MOXA-STREAM-ADAPTER-MIB
 * OID  .1.3.6.1.4.1.8691.603.2.10.1.1.1
 * PATH
 * .iso.org.dod.internet.private.enterprises.moxa.switching.basicLayer2.mxStAd.stadConfiguration.stadConfigTable.stadConfigEntry
 */
class ActStadConfigEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // ACT_JSON_FIELD(qint32, port_index, PortIndex);      ///< The Port Index (MIB)
  ACT_JSON_FIELD(qint64, port_id, PortId);            ///< The Port ID
  ACT_JSON_FIELD(qint32, egress_untag, EgressUntag);  ///< The Egress Untag (INTEGER(true(1), false(2)))

 public:
  /**
   * @brief Construct a new Stad Config Entry object
   *
   */
  ActStadConfigEntry() {
    this->port_id_ = -1;
    this->egress_untag_ = 2;  // false
  }

  /**
   * @brief Construct a new Stad Config Entry object
   *
   * @param port_id
   */
  ActStadConfigEntry(const qint64 &port_id) : ActStadConfigEntry() { this->port_id_ = port_id; }

  /**
   * @brief Construct a new Stad Config Entry object
   *
   * @param port_id
   * @param egress_untag
   */
  ActStadConfigEntry(const qint64 &port_id, const qint32 &egress_untag) : ActStadConfigEntry() {
    this->port_id_ = port_id;
    this->egress_untag_ = egress_untag;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActStadConfigEntry &x) {
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
  friend bool operator==(const ActStadConfigEntry &x, const ActStadConfigEntry &y) { return x.port_id_ == y.port_id_; }
};
