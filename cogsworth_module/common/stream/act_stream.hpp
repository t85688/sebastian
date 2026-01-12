/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include <QtMath>

#include "act_interface_capability.hpp"
#include "act_json.hpp"
#include "stream/act_data_frame_specification.hpp"
#include "stream/act_interface.hpp"
#include "stream/act_stream_id.hpp"
#include "stream/act_traffic_specification.hpp"
#include "stream/act_user_to_network_requirement.hpp"

/**
 * @brief The Stream untagged mode enum class
 *
 */
enum class ActStreamUntaggedModeEnum { kPVID = 0, kPerStreamPriority = 1 };

/**
 * @brief The QMap for Stream untagged mode enum mapping
 *
 */
static const QMap<QString, ActStreamUntaggedModeEnum> kActStreamUntaggedModeEnumMap = {
    {"PVID", ActStreamUntaggedModeEnum::kPVID}, {"PerStreamPriority", ActStreamUntaggedModeEnum::kPerStreamPriority}};

/**
 * @brief The Stream traffic type enum class
 *
 */
enum class ActStreamTrafficTypeEnum { kNA = 0, kBestEffort = 1, kCyclic = 2, kTimeSync = 3 };

/**
 * @brief The QMap for Stream traffic type enum mapping
 *
 */
static const QMap<QString, ActStreamTrafficTypeEnum> kActStreamTrafficTypeEnumMap = {
    {"NA", ActStreamTrafficTypeEnum::kNA},
    {"BestEffort", ActStreamTrafficTypeEnum::kBestEffort},
    {"Cyclic", ActStreamTrafficTypeEnum::kCyclic},
    {"TimeSync", ActStreamTrafficTypeEnum::kTimeSync}};

/**
 * @brief The Stream status type enum class
 *
 */
enum class ActStreamStatusEnum { kPlanned = 1, kScheduled = 2, kModified = 3 };

/**
 * @brief The QMap for Stream status type enum mapping
 *
 */
static const QMap<QString, ActStreamStatusEnum> kActStreamStatusEnumMap = {
    {"Planned", ActStreamStatusEnum::kPlanned},
    {"Scheduled", ActStreamStatusEnum::kScheduled},
    {"Modified", ActStreamStatusEnum::kModified}};

/**
 * @brief The Qos type enum class
 *
 */
enum class ActQosTypeEnum { kBandwidth, kBoundedLatency, kDeadline };

/**
 * @brief The QMap for Stream type enum mapping
 *
 */
static const QMap<QString, ActQosTypeEnum> kActQosTypeEnumMap = {{"Bandwidth", ActQosTypeEnum::kBandwidth},
                                                                 {"Bounded Latency", ActQosTypeEnum::kBoundedLatency},
                                                                 {"Deadline", ActQosTypeEnum::kDeadline}};

/**
 * @brief The Frame type class enum class
 *
 */
enum class ActFrameTypeEnum {
  kNA = 0,
  kCCLinkIeTsn = 1,
  kEtherCAT = 2,
  kProfinet = 3,
  kEthernetIp = 4,
  kUserDefined = 5
};

/**
 * @brief The QMap for Frame type class enum mapping
 *
 */
static const QMap<QString, ActFrameTypeEnum> kActFrameTypeEnumMap = {{"NA", ActFrameTypeEnum::kNA},
                                                                     {"CC-LinkIETSN", ActFrameTypeEnum::kCCLinkIeTsn},
                                                                     {"EtherCAT", ActFrameTypeEnum::kEtherCAT},
                                                                     {"Profinet", ActFrameTypeEnum::kProfinet},
                                                                     {"EthernetIp", ActFrameTypeEnum::kEthernetIp},
                                                                     {"UserDefined", ActFrameTypeEnum::kUserDefined}};

/**
 * @brief The Stream's Listener class
 * 8021qcc 2018: 46.2.4 Listener
 *
 */
class ActListener : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActInterface, end_station_interface,
                  EndStationInterface);  ///< EndStationInterface items (46.2.3.3)
  ACT_JSON_OBJECT(ActUserToNetworkRequirement, user_to_network_requirement,
                  UserToNetworkRequirement);  ///< UserToNetworkRequirement item (46.2.3.6)
  ACT_JSON_OBJECT(ActInterfaceCapability, interface_capability,
                  InterfaceCapability);  ///< InterfaceCapability item (46.2.3.7)

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActListener &x, const ActListener &y) {
    return x.GetEndStationInterface().GetDeviceId() == y.GetEndStationInterface().GetDeviceId() &&
           x.GetEndStationInterface().GetInterfaceId() == y.GetEndStationInterface().GetInterfaceId();
  }
};

/**
 * @brief The Stream's Talker class
 * 8021qcc 2018: 46.2.3 Talker
 *
 */
class ActTalker : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActInterface, end_station_interface,
                  EndStationInterface);  ///< EndStationInterface items (46.2.3.3)
  ACT_JSON_COLLECTION_OBJECTS(QList, ActDataFrameSpecification, data_frame_specifications,
                              DataFrameSpecifications);  ///< DataFrameSpecification items list (46.2.3.4)
  ACT_JSON_OBJECT(ActTrafficSpecification, traffic_specification, TrafficSpecification);  ///< TrafficSpecification item
  // (46.2.3.5)

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActTalker &x, const ActTalker &y) {
    return x.GetEndStationInterface().GetDeviceId() == y.GetEndStationInterface().GetDeviceId() &&
           x.GetEndStationInterface().GetInterfaceId() == y.GetEndStationInterface().GetInterfaceId();
  }
};

/**
 * @brief The StreamPriority's CCLinkIeTsn class
 *
 */
class ActCCLinkIeTsn : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint32, ether_type, EtherType);        ///< EtherType item
  ACT_JSON_FIELD(bool, enable_sub_type, EnableSubType);  ///< EnableSubType item
  ACT_JSON_FIELD(quint16, sub_type, SubType);            ///< SubType item

 public:
  ActCCLinkIeTsn() {
    this->SetEtherType(0);
    this->SetEnableSubType(false);
    this->SetSubType(0);
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActCCLinkIeTsn &x) {
    return qHash(x.sub_type_, qHash(x.enable_sub_type_, qHash(x.ether_type_, 0)));  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActCCLinkIeTsn &x, const ActCCLinkIeTsn &y) {
    return x.GetEtherType() == y.GetEtherType() && x.GetEnableSubType() == y.GetEnableSubType() &&
           x.GetSubType() == y.GetSubType();
  }
};

/**
 * @brief The Stream's StreamPriority class
 *
 */
class ActStreamPriority : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActFrameTypeEnum, frame_type, FrameType);        ///< FrameType item
  ACT_JSON_OBJECT(ActCCLinkIeTsn, cc_link_ie_tsn, CCLinkIeTSN);  ///< CC-LinkIETSN item

 public:
  ActStreamPriority() { this->SetFrameType(ActFrameTypeEnum::kCCLinkIeTsn); }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActStreamPriority &x) {
    return qHash(x.cc_link_ie_tsn_, static_cast<quint16>(x.frame_type_));  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActStreamPriority &x, const ActStreamPriority &y) {
    return x.GetFrameType() == y.GetFrameType() && x.GetCCLinkIeTSN() == y.GetCCLinkIeTSN();
  }
};

/**
 * @brief The Stream's CB class
 *
 */
class ActCB : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, active, Active);                    ///< Active item
  ACT_JSON_COLLECTION(QList, quint16, vlan_ids, VlanIds);  ///< VlanId item (user-defined or system-assigned)

 public:
  /**
   * @brief Construct a new CB object
   *
   */
  ActCB() : active_(false) {}
  ActCB(bool active, QList<quint16> vlan_ids) : active_(active), vlan_ids_(vlan_ids) {}
};

/**
 * @brief The class of stream unique ids
 *
 */
class ActStreamUniqueIds : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_COLLECTION(QList, qint64, stream_unique_ids, StreamUniqueIds);
};

/**
 * @brief The Stream class
 *
 */
class ActStream : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);                     ///< UniqueId item
  ACT_JSON_OBJECT(ActStreamId, stream_id, StreamId);  ///< StreamId item (46.2.3.1) deprecated in current version
  ACT_JSON_FIELD(QString, stream_name, StreamName);   ///< StreamName item
  ACT_JSON_FIELD(bool, tagged, Tagged);               ///< Boolean of tagged stream
  ACT_JSON_ENUM(ActStreamUntaggedModeEnum, untagged_mode, UntaggedMode);            ///< Stream untagged mode enum
  ACT_JSON_ENUM(ActStreamTrafficTypeEnum, stream_traffic_type, StreamTrafficType);  ///< Stream traffic type enum
  ACT_JSON_ENUM(ActQosTypeEnum, qos_type, QosType);                                 ///< QosType item
  ACT_JSON_FIELD(quint16, vlan_id, VlanId);  ///< VlanId item (user-defined or system-assigned)
  ACT_JSON_FIELD(quint8, priority_code_point,
                 PriorityCodePoint);           ///< PriorityCodePoint item (0~7) (user-defined or system-assigned)
  ACT_JSON_OBJECT(ActCB, cb, CB);              ///< Specify CB enable
  ACT_JSON_FIELD(bool, multicast, Multicast);  ///< Specify multicast enable
  ACT_JSON_FIELD(bool, user_defined_vlan, UserDefinedVlan);               ///< Specify user defined vlan enable
  ACT_JSON_FIELD(quint8, stream_rank, StreamRank);                        ///< StreamRank item (46.2.3.2)
  ACT_JSON_OBJECT(ActStreamPriority, stream_priority, StreamPriority);    ///< StreamPriority object item
  ACT_JSON_FIELD(quint8, time_slot_index, TimeSlotIndex);                 ///< Time slot index
  ACT_JSON_OBJECT(ActTalker, talker, Talker);                             ///< Talker object item
  ACT_JSON_COLLECTION_OBJECTS(QList, ActListener, listeners, Listeners);  ///< Listener item list

  ACT_JSON_ENUM(ActStreamStatusEnum, stream_status,
                StreamStatus);  ///< The stream status of the stream, from 802.1Qdj D0.3

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new Stream object
   *
   */
  ActStream() {
    this->id_ = -1;
    this->stream_traffic_type_ = ActStreamTrafficTypeEnum::kNA;
    this->qos_type_ = ActQosTypeEnum::kBandwidth;
    this->stream_status_ = ActStreamStatusEnum::kPlanned;
    this->vlan_id_ = 0;
    this->priority_code_point_ = 0;
    this->tagged_ = false;
    this->untagged_mode_ = ActStreamUntaggedModeEnum::kPVID;
    this->multicast_ = false;
    this->user_defined_vlan_ = false;
    this->key_order_.append(QList<QString>(
        {QString("Id"), QString("StreamId"), QString("StreamName"), QString("StreamTrafficType"), QString("QosType"),
         QString("UntaggedMode"), QString("VlanId"), QString("CB"), QString("Multicast"), QString("UserDefinedVlan"),
         QString("StreamRank"), QString("StreamPriority"), QString("TimeSlotIndex"), QString("Talker"),
         QString("Listeners"), QString("StreamStatus")}));
  }

  /**
   * @brief Construct a new Stream object
   *
   * @param id
   */
  ActStream(const qint64 &id) : ActStream() { this->id_ = id; }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActStream &x) {
    return qHash(x.id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActStream &x, const ActStream &y) {
    // return (x.id_ == y.id_) ||
    //        (x.stream_id_.GetMacAddress() == y.stream_id_.GetMacAddress() && x.vlan_id_ == y.vlan_id_);
    return x.id_ == y.id_;
  }

  /**
   * @brief Get MAC address from talker
   *
   * @param dst_mac
   * @return ACT_STATUS
   */
  ACT_STATUS GetTalkerAddressFromStream(QString &dst_mac) {
    ACT_STATUS_INIT();

    ActTalker talker = this->GetTalker();
    ActInterface intf = talker.GetEndStationInterface();
    dst_mac = intf.GetMacAddress();
    return act_status;
  }

  /**
   * @brief Get DA from talker or multicast da
   *
   * @param dst_mac
   * @return ACT_STATUS
   */
  ACT_STATUS GetDAFromStream(QString &dst_mac) const {
    ACT_STATUS_INIT();
    // check multicast
    ActTalker talker = this->GetTalker();
    QList<ActDataFrameSpecification> data_frame_specifications = talker.GetDataFrameSpecifications();
    for (ActDataFrameSpecification dfs : data_frame_specifications) {
      if (dfs.GetType() == ActFieldTypeEnum::kIeee802MacAddresses) {
        dst_mac = dfs.GetIeee802MacAddresses().GetDestinationMacAddress();
        return act_status;
      }
    }
    QString message = QString("Stream (%1) - Get destination MAC address from Talker failed, please check talker exist")
                          .arg(this->GetStreamName());
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  /**
   * @brief Check user defined vid + pcp
   *
   * @param tag
   * @return ACT_STATUS
   */
  ACT_STATUS GetUserDefinedVlan(ActIeee802VlanTag &tag) const {
    ACT_STATUS_INIT();

    ActTalker talker = this->GetTalker();

    // Check use defined vlan flag is set
    if (this->GetUserDefinedVlan()) {
      QList<ActDataFrameSpecification> data_frame_specifications = talker.GetDataFrameSpecifications();
      for (ActDataFrameSpecification dfs : data_frame_specifications) {
        if (dfs.GetType() == ActFieldTypeEnum::kIeee802VlanTag) {
          tag = dfs.GetIeee802VlanTag();
          return act_status;
        }
      }
    } else {
      tag.SetVlanId(0);
      tag.SetPriorityCodePoint(0);
      return act_status;
    }

    QString message = QString("Stream (%1) - Get User-Defined VLAN tag failed").arg(this->GetStreamName());
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  /**
   * @brief Get the Payload bps
   *
   * @param media_specific_overhead_bytes
   * @param payload
   * @return ACT_STATUS
   */
  ACT_STATUS GetPayload(quint32 media_specific_overhead_bytes, quint32 &payload) {
    ACT_STATUS_INIT();

    ActTrafficSpecification traffic_spec = this->GetTalker().GetTrafficSpecification();
    // set stream payload including frame and media specific overhead
    // IF MaxBytesPerInterval == 0, consider the case below
    quint32 max_bytes_per_interval = (traffic_spec.GetMaxBytesPerInterval() == 0)
                                         ? traffic_spec.GetMaxFrameSize() * traffic_spec.GetMaxFramesPerInterval()
                                         : traffic_spec.GetMaxBytesPerInterval();
    quint32 max_frames_per_interval =
        qCeil(static_cast<qreal>(max_bytes_per_interval) / static_cast<qreal>(traffic_spec.GetMaxFrameSize()));

    media_specific_overhead_bytes =
        (this->GetCB().GetActive()) ? (media_specific_overhead_bytes + 6) : media_specific_overhead_bytes;

    payload = (max_bytes_per_interval + media_specific_overhead_bytes * max_frames_per_interval) * 8;

    return act_status;
  }

  /**
   * @brief Get the Stream Interval in ns
   *
   * @param interval
   * @return ACT_STATUS
   */
  ACT_STATUS GetStreamInterval(quint32 &interval) {
    ACT_STATUS_INIT();

    ActTrafficSpecification traffic_spec = this->GetTalker().GetTrafficSpecification();

    // calculate interval
    qreal interval_in_ns = static_cast<qreal>(traffic_spec.GetInterval().GetNumerator()) /
                           static_cast<qreal>(traffic_spec.GetInterval().GetDenominator()) * qPow(10, 9);
    interval = qCeil(interval_in_ns);  // round to ns

    return act_status;
  }
};

/**
 * @brief [feat:2700] A map class to store stream id (key) and patch content (value)
 *
 */
class ActPatchStreamMap : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT(QMap, qint64, QString, patch_stream_map, PatchStreamMap);
};

/**
 * @brief [feat:2699] A list class to store streams
 *
 */
class ActStreamList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActStream, stream_list, StreamList);
};
