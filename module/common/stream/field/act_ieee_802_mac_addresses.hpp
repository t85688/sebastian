/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

/**
 * @brief The Field's Ieee802MacAddresses class
 *
 */
class ActIeee802MacAddresses : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, source_mac_address, SourceMacAddress);            ///< SourceMacAddress item
  ACT_JSON_FIELD(QString, destination_mac_address, DestinationMacAddress);  ///< DestinationMacAddress item

 public:
  /**
   * @brief Construct a new ActIeee802MacAddresses object
   *
   */
  ActIeee802MacAddresses() {};

  /**
   * @brief Construct a new ActIeee802MacAddresses object
   *
   * @param src_mac
   * @param dst_mac
   */
  ActIeee802MacAddresses(const QString &src_mac, const QString &dst_mac)
      : source_mac_address_(src_mac), destination_mac_address_(dst_mac) {
    // transform(src_mac.begin(), src_mac.end(), src_mac.begin(), toupper);
    // this->source_mac_address_ = src_mac;
    // transform(dst_mac.begin(), dst_mac.end(), dst_mac.begin(), toupper);
    // this->destination_mac_address_ = dst_mac;
  }
};
