/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_ARP_TABLE_HPP
#define ACT_ARP_TABLE_HPP

#include <QString>

#include "act_json.hpp"

namespace act {
namespace topology {

class ActArpEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, ip_address, IpAddress);    ///< IP address item
  ACT_JSON_FIELD(QString, mac_address, MacAddress);  ///< Mac address item

 public:
  /**
   * @brief Construct a new Act Arp Entry object
   *
   */
  ActArpEntry() {}

  /**
   * @brief Construct a new Act Arp Entry object
   *
   * @param ip_address
   * @param mac_address
   */
  ActArpEntry(const QString &ip_address, const QString &mac_address)
      : ip_address_(ip_address), mac_address_(mac_address) {}

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActArpEntry &x) {
    return qHash(x.ip_address_, 0);
    // return 0;
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActArpEntry &x, const ActArpEntry &y) {
    return x.ip_address_ == y.ip_address_;
    // return x.ip_address_ == y.ip_address_ || x.mac_address_ == y.mac_address_;
  }
};

class ActArpTable : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActArpEntry, arp_entries, ArpEntries);  ///< The link set in the project
};

}  // namespace topology
}  // namespace act

#endif /* ACT_ARP_TABLE_HPP */