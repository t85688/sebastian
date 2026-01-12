/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

/**
 * @brief The Field's Ipv4Tuple class
 *
 */
class ActIpv4Tuple : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, source_ip_address, SourceIpAddress);            ///< SourceIpAddress item
  ACT_JSON_FIELD(QString, destination_ip_address, DestinationIpAddress);  ///< DestinationIpAddress item
  ACT_JSON_FIELD(quint8, dscp, Dscp);                                     ///< DSCP item
  ACT_JSON_FIELD(quint16, protocol, Protocol);                            ///< Protocol item
  ACT_JSON_FIELD(quint16, source_port, SourcePort);                       ///< SourcePort item
  ACT_JSON_FIELD(quint16, destination_port, DestinationPort);             ///< DestinationPort item

 public:
  /**
   * @brief Construct a new Ipv 4 Tuple object
   *
   */
  ActIpv4Tuple() : source_port_(0), destination_port_(0), dscp_(0), protocol_(0) {}

  /**
   * @brief Construct a new Ipv 4 Tuple object
   *
   * @param dst_ip
   * @param dst_port
   * @param dscp
   * @param protocol
   * @param src_ip
   * @param src_port
   */
  ActIpv4Tuple(const QString &src_ip, const QString &dst_ip, const quint8 &dscp, const quint16 &protocol,
               const quint16 &src_port, const quint16 &dst_port)
      : destination_ip_address_(dst_ip),
        destination_port_(dst_port),
        dscp_(dscp),
        protocol_(protocol),
        source_ip_address_(src_ip),
        source_port_(src_port) {}
};
