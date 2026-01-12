/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

/**
 * @brief The Stream's ID class
 * IEEE 802.1Qcc-2018 clause 35.2.2.8.2.
 */
class ActStreamId : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, mac_address, MacAddress);  ///< MacAddress item (Talks' mac)
  ACT_JSON_FIELD(quint16, unique_id, UniqueId);      ///< UniqueId item

 public:
  /**
   * @brief Construct a new Stream Id object
   *
   */
  ActStreamId() : unique_id_(0) {}

  /**
   * @brief Construct a new Stream Id object
   *
   * @param mac_address
   * @param unique_id
   */
  ActStreamId(const QString &mac_address, const quint16 &unique_id)
      : mac_address_(mac_address), unique_id_(unique_id) {}

  // /**
  //  * @brief Construct a new Stream Id object
  //  *
  //  * @param sid
  //  */
  // ActStreamId(const ActStreamId& sid) : ActStreamId(sid.mac_address_, sid.unique_id_) {}

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActStreamId &x) {
    return qHash(x.unique_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActStreamId &x, const ActStreamId &y) { return x.unique_id_ == y.unique_id_; }

  /**
   * @brief The comparison operator for QMap
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator<(const ActStreamId &x, const ActStreamId &y) { return x.unique_id_ < y.unique_id_; }
};
