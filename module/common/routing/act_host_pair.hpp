/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

/**
 * @brief The path's HostPair class
 *
 */
class ActHostPair : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, source_device_id, SourceDeviceId);        ///< the source device id of the stream
  ACT_JSON_FIELD(qint64, source_interface_id, SourceInterfaceId);  ///< the source interface id of the stream
  ACT_JSON_FIELD(QString, source_ip, SourceIp);
  ACT_JSON_FIELD(qint64, destination_device_id, DestinationDeviceId);  ///< the destination device id of the stream
  ACT_JSON_FIELD(qint64, destination_interface_id,
                 DestinationInterfaceId);  ///< the destination interface id of the stream
  ACT_JSON_FIELD(QString, destination_ip, DestinationIp);

 public:
  /**
   * @brief Construct a new Host Pair object
   *
   */
  ActHostPair() {}

  /**
   * @brief Construct a new Host Pair object
   *
   * @param source_device_id
   * @param destination_device_id
   */
  ActHostPair(const qint64 &source_device_id, const QString &source_ip, const qint64 &destination_device_id,
              const QString &destination_ip)
      : source_device_id_(source_device_id),
        source_interface_id_(0),
        source_ip_(source_ip),
        destination_device_id_(destination_device_id),
        destination_interface_id_(0),
        destination_ip_(destination_ip) {}

  /**
   * @brief Construct a new Host Pair object
   *
   * @param source_device_id
   * @param source_interface_id
   * @param destination_device_id
   * @param destination_interface_id
   */
  ActHostPair(const qint64 &source_device_id, const qint64 &source_interface_id, const QString &source_ip,
              const qint64 &destination_device_id, const qint64 &destination_interface_id,
              const QString &destination_ip)
      : source_device_id_(source_device_id),
        source_interface_id_(source_interface_id),
        source_ip_(source_ip),
        destination_device_id_(destination_device_id),
        destination_interface_id_(destination_interface_id),
        destination_ip_(destination_ip) {}

  /**
   * @brief The "==" comparison of host pair (currently used by RoutingStream class)
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActHostPair &x, const ActHostPair &y) {
    return x.source_device_id_ == y.source_device_id_ && x.source_interface_id_ == y.source_interface_id_ &&
           x.source_ip_ == y.source_ip_ && x.destination_device_id_ == y.destination_device_id_ &&
           x.destination_interface_id_ == y.destination_interface_id_ && x.destination_ip_ == y.destination_ip_;
  }
};
