/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"
#include "stream/act_stream_id.hpp"
#include "topology/act_link.hpp"

/**
 * @brief The stream routing type enum class
 *
 */
enum class ActRoutingTypeEnum { kShortestPath, kRedundantPath, kMultiplePath };

/**
 * @brief The QMap stream routing type enum mapping
 *
 */
static const QMap<QString, ActRoutingTypeEnum> kActRoutingTypeEnumMap = {
    {"ShortestPath", ActRoutingTypeEnum::kShortestPath},
    {"RedundantPath", ActRoutingTypeEnum::kRedundantPath},
    {"MultiplePath", ActRoutingTypeEnum::kMultiplePath}};

/**
 * @brief The routing's Path class
 *
 */
class ActRedundantPath : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint64, link_ids, LinkIds);      ///< LinksIds items list
  ACT_JSON_COLLECTION(QList, qint64, device_ids, DeviceIds);  ///< DeviceIds item list
  ACT_JSON_FIELD(quint16, vlan_id, VlanId);                   ///< VlanId item
  // ACT_JSON_QT_SET(quint16, zones, Zones);                  ///< Zone items set

 public:
  /**
   * @brief Construct a new Act Path object
   *
   */
  ActRedundantPath() {}

  /**
   * @brief Construct a new Act Path object
   *
   * @param links
   * @param device_ips
   * @param vlan_id
   */
  ActRedundantPath(const QList<qint64> &links, const QList<qint64> &device_ids, const quint16 &vlan_id)
      : link_ids_(links), device_ids_(device_ids), vlan_id_(vlan_id) {}
};

/**
 * @brief The routing's RoutingPath class
 *
 */
class ActRoutingPath : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, talker_ip_address, TalkerIpAddress);
  ACT_JSON_FIELD(QString, listener_ip_address, ListenerIpAddress);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActRedundantPath, redundant_paths,
                              RedundantPaths)  ///< RedundantPaths item list (origin / cb stream)

 public:
  /**
   * @brief Construct a new Act Routing Path object
   *
   */
  ActRoutingPath() {}
};

/**
 * @brief The path's RoutingResult class
 *
 */
class ActRoutingResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_FIELD(QString, stream_name, StreamName);  ///< StreamName item
  ACT_JSON_FIELD(qint64, stream_id, StreamId);       ///< StreamId item
  ACT_JSON_FIELD(quint16, vlan_id, VlanId);          ///< VlanId item (For CB to check the original stream vlan)
  ACT_JSON_FIELD(quint8, priority_code_point, PriorityCodePoint);  ///< PriorityCodePoint item
  ACT_JSON_FIELD(quint8, queue_id, QueueId);                       ///< Queue id of this stream
  ACT_JSON_FIELD(bool, cb, CB);                                    ///< CB boolean item
  ACT_JSON_FIELD(bool, multicast, Multicast);                      ///< Multicast boolean item
  // ACT_JSON_ENUM(ActRoutingTypeEnum, routing_type, RoutingType);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActRoutingPath, routing_paths,
                              RoutingPaths)  ///< RoutingPath item list (multicast)

 public:
  /**
   * @brief Construct a new Routing Result object
   *
   */
  ActRoutingResult() : stream_id_(-1) {}

  /**
   * @brief Construct a new Routing Result object
   *
   * @param stream_id
   */
  ActRoutingResult(const qint64 &stream_id) : stream_id_(stream_id) {}

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActRoutingResult &x) {
    return qHash(x.stream_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActRoutingResult &x, const ActRoutingResult &y) { return x.stream_id_ == y.stream_id_; }
};
