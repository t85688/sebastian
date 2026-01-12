/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#define TRANSFER_RATE 1000

#include "act_json.hpp"
#include "act_status.hpp"

/**
 * @brief The Cable type class enum class
 *
 */
enum class ActCableTypeEnum { kCopper, kFiber, kTwoWire, kSPE };

/**
 * @brief The QMap for Cable type class enum mapping
 *
 */
static const QMap<QString, ActCableTypeEnum> kActCableTypeEnumMap = {{"Copper", ActCableTypeEnum::kCopper},
                                                                     {"Fiber", ActCableTypeEnum::kFiber},
                                                                     {"Two-wire", ActCableTypeEnum::kTwoWire},
                                                                     {"SPE", ActCableTypeEnum::kSPE}};

// For QSet (Currently used in ActInterface & ActInterfaceProperty)
inline uint qHash(ActCableTypeEnum key, uint seed) { return qHash(static_cast<uint>(key), seed); }  ///< For QSet

/**
 * @brief The class of link ids
 *
 */
class LinkIds : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_COLLECTION(QList, qint64, link_ids, LinkIds);
};

/**
 * @brief The ACT Link class
 *
 */
class ActLink : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(bool, alive, Alive);  ///< Link alive or not in monitor
  // ACT_JSON_FIELD(quint64, bandwidth_value, BandwidthValue);      ///< BandwidthValue item in bit/s
  ACT_JSON_FIELD(quint16, speed, Speed);                         ///< The speed of the link in Mbps
  ACT_JSON_FIELD(quint32, cable_length, CableLength);            ///< CableLength item
  ACT_JSON_ENUM(ActCableTypeEnum, cable_type, CableType);        ///< The cable type enum
  ACT_JSON_FIELD(quint32, propagation_delay, PropagationDelay);  ///< PropagationDelay item (1 ~ 204000 ns)

  ACT_JSON_FIELD(qint64, destination_device_id, DestinationDeviceId);        ///< DestinationDeviceId item
  ACT_JSON_FIELD(QString, destination_device_ip, DestinationDeviceIp);       ///< DestinationDeviceIp item
  ACT_JSON_FIELD(qint64, destination_interface_id, DestinationInterfaceId);  ///< DestinationInterfaceId item
  // ACT_JSON_FIELD(QString, destination_interface_name, DestinationInterfaceName);  ///< DestinationInterfaceName item

  ACT_JSON_FIELD(qint64, source_device_id, SourceDeviceId);        ///< SourceDeviceId item
  ACT_JSON_FIELD(QString, source_device_ip, SourceDeviceIp);       ///< SourceDeviceIp item
  ACT_JSON_FIELD(qint64, source_interface_id, SourceInterfaceId);  ///< SourceInterfaceId item
  // ACT_JSON_FIELD(QString, source_interface_name, SourceInterfaceName);  ///< SourceInterfaceName item
  ACT_JSON_FIELD(bool, redundant, Redundant);  ///< Redundant link or not

 public:
  /**
   * @brief Construct a new Act Link object
   *
   */
  ActLink() {
    this->id_ = -1;
    this->alive_ = true;
    // [bugfix:550] Import project failed on class based mode
    // There are no related fields on the UI
    this->speed_ = 1000;
    this->cable_length_ = 1;
    this->cable_type_ = ActCableTypeEnum::kCopper;
    this->propagation_delay_ = 5;

    this->destination_device_id_ = -1;
    this->destination_interface_id_ = -1;
    this->source_device_id_ = -1;
    this->source_interface_id_ = -1;
    this->redundant_ = false;
  }

  /**
   * @brief Construct a new Act Link object
   *
   * @param id
   */
  ActLink(const qint64 &id) : ActLink() { this->id_ = id; }

  /**
   * @brief Construct a new Act Link object
   *
   * @param id
   */
  ActLink(const qint64 &id, const qint64 &source_device_id, const qint64 &destination_device_id,
          const qint64 &source_interface_id, const qint64 &destination_interface_id)
      : ActLink() {
    id_ = id;
    source_device_id_ = source_device_id;
    destination_device_id_ = destination_device_id;
    source_interface_id_ = source_interface_id;
    destination_interface_id_ = destination_interface_id;
    speed_ = 1000;
    cable_length_ = 1;
    cable_type_ = ActCableTypeEnum::kCopper;
    propagation_delay_ = 5;
    redundant_ = false;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActLink &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
    // Because the operator== contains id and other fields, so we need to put all entries at the same index
    return 0;
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActLink &x, const ActLink &y) {
    return (x.id_ == y.id_) ||
           (x.source_device_id_ == y.source_device_id_ && x.source_interface_id_ == y.source_interface_id_) ||
           (x.destination_device_id_ == y.destination_device_id_ &&
            x.destination_interface_id_ == y.destination_interface_id_) ||
           (x.source_device_id_ == y.destination_device_id_ && x.source_interface_id_ == y.destination_interface_id_) ||
           (x.destination_device_id_ == y.source_device_id_ && x.destination_interface_id_ == y.source_interface_id_);
  }

  /**
   * @brief Get the Bandwidth Value
   *
   * @param bandwidth_value
   * @return ACT_STATUS
   */
  ACT_STATUS GetBandwidthValue(quint64 &bandwidth_value) {
    ACT_STATUS_INIT();

    // check speed is supported by interface?
    // check speed == 100 || speed == 1000? (1024?)

    bandwidth_value = static_cast<quint64>(speed_) * TRANSFER_RATE * TRANSFER_RATE;

    return act_status;
  }
};

class ActLinkList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActLink, links, Links);
};

/**
 * @brief [feat:2703] A map class to store link id (key) and patch content (value)
 *
 */
class ActPatchLinkMap : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT(QMap, qint64, QString, patch_link_map, PatchLinkMap);
};