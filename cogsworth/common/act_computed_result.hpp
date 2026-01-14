/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_device.hpp"
#include "act_json.hpp"
#include "act_link.hpp"
#include "act_status.hpp"
#include "act_traffic.hpp"
#include "gcl/act_gcl_result.hpp"
#include "routing/act_routing_result.hpp"
#include "stream/act_traffic_specification.hpp"

/**
 * @brief The stream type enum class
 *
 */
enum class ActStreamTypeEnum { kUserDefinedVlan, kPerStreamPriority, kSystemAssignedVlanTag };

/**
 * @brief The QMap stream type enum mapping
 *
 */
static const QMap<QString, ActStreamTypeEnum> kActStreamTypeEnumMap = {
    {"UserDefinedVlan", ActStreamTypeEnum::kUserDefinedVlan},
    {"PerStreamPriority", ActStreamTypeEnum::kPerStreamPriority},
    {"SystemAssignedVlanTag", ActStreamTypeEnum::kSystemAssignedVlanTag}};

/**
 * @brief The queue id enum class
 *
 */
enum class ActQueueIdEnum { k0, k1, k2, k3, k4, k5, k6, k7, kTalker, kListener };

/**
 * @brief The QMap queue id enum mapping
 *
 */
static const QMap<QString, ActQueueIdEnum> kActQueueIdEnumMap = {
    {"0", ActQueueIdEnum::k0},           {"1", ActQueueIdEnum::k1},
    {"2", ActQueueIdEnum::k2},           {"3", ActQueueIdEnum::k3},
    {"4", ActQueueIdEnum::k4},           {"5", ActQueueIdEnum::k5},
    {"6", ActQueueIdEnum::k6},           {"7", ActQueueIdEnum::k7},
    {"Sender", ActQueueIdEnum::kTalker}, {"Receiver", ActQueueIdEnum::kListener}};

/**
 * @brief The ACT stream view result on each device interface class
 *
 */
class ActStreamDeviceInterfaceResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);                       ///< Device ID
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);                      ///< Device IP
  ACT_JSON_FIELD(qint64, ingress_interface_id, IngressInterfaceId);  ///< Ingress Interface ID
  ACT_JSON_FIELD(qint64, egress_interface_id, EgressInterfaceId);    ///< Egress Interface ID
  ACT_JSON_FIELD(quint8, priority_code_point, PriorityCodePoint);    ///< Priority Code Point
  ACT_JSON_FIELD(quint32, start_time, StartTime);                    ///< The start time of the transmission
  ACT_JSON_FIELD(quint32, stop_time, StopTime);                      ///< The stop time of the transmission

  // properties
  ACT_JSON_FIELD(QString, ingress_interface_name, IngressInterfaceName);  ///< Ingress InterfaceName
  ACT_JSON_FIELD(QString, egress_interface_name, EgressInterfaceName);    ///< Egress InterfaceName
  ACT_JSON_FIELD(quint32, accumulated_latency, AccumulatedLatency);  ///< AccumulatedLatency (not used in class-based)
  ACT_JSON_FIELD(quint32, stream_duration, StreamDuration);          ///< StreamDuration (not used in class-based)
  ACT_JSON_FIELD(quint32, frame_duration, FrameDuration);            ///< FrameDuration (not used in class-based)

  ACT_JSON_ENUM(ActQueueIdEnum, queue_id,
                QueueId);                    ///< The queue id
  ACT_JSON_FIELD(quint16, vlan_id, VlanId);  ///< VlanId (user-defined or system-assigned)
  ACT_JSON_ENUM(ActFrerTypeEnum, frer_type, FrerType);

 public:
  ActStreamDeviceInterfaceResult() {
    device_id_ = -1;
    ingress_interface_id_ = 0;
    egress_interface_id_ = 0;
    priority_code_point_ = 0;
    start_time_ = 0;
    stop_time_ = 0;
    accumulated_latency_ = 0;
    stream_duration_ = 0;
    frame_duration_ = 0;
    vlan_id_ = 0;
    queue_id_ = ActQueueIdEnum::k0;
    frer_type_ = ActFrerTypeEnum::kUndefined;
  }
};

class ActStreamRedundantPathResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActStreamDeviceInterfaceResult, device_interface_results,
                              DeviceInterfaceResults);  ///< The schedule result on each device interface
};

class ActStreamPathResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActStreamRedundantPathResult, stream_redundant_path_results,
                              StreamRedundantPathResults);  ///< The schedule result on each device interface
};

/**
 * @brief The ACT stream view result class
 *
 */
class ActStreamViewResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, stream_id, StreamId);                ///< ID of this stream
  ACT_JSON_ENUM(ActStreamTypeEnum, stream_type, StreamType);  ///< Type of this stream
  // ACT_JSON_ENUM(ActRoutingTypeEnum, routing_type, RoutingType);

  ACT_JSON_ENUM(ActFrameTypeEnum, frame_type, FrameType);           ///< FrameType item
  ACT_JSON_FIELD(QString, stream_traffic_type, StreamTrafficType);  ///< Stream traffic type enum

  ACT_JSON_COLLECTION_OBJECTS(QList, ActStreamPathResult, stream_path_results,
                              StreamPathResults);  ///< The stream-view result on a path (multicast has multiple paths)

  // properties
  ACT_JSON_FIELD(QString, stream_name, StreamName);        ///< StreamName
  ACT_JSON_OBJECT(ActInterval, interval, Interval);        ///< Interval object (not used in class-based)
  ACT_JSON_FIELD(quint8, time_slot_index, TimeSlotIndex);  ///< Time slot index

 public:
  /**
   * @brief Construct a new Act Stream View Result object
   *
   */
  ActStreamViewResult() : stream_id_(-1), time_slot_index_(0) {
    stream_type_ = ActStreamTypeEnum::kSystemAssignedVlanTag;
    frame_type_ = ActFrameTypeEnum::kNA;
  }

  /**
   * @brief Construct a new Act Stream View Result object
   *
   * @param stream_id
   */
  ActStreamViewResult(const qint64 &stream_id) : stream_id_(stream_id) {
    stream_type_ = ActStreamTypeEnum::kSystemAssignedVlanTag;
    frame_type_ = ActFrameTypeEnum::kNA;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActStreamViewResult &x) {
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
  friend bool operator==(const ActStreamViewResult &x, const ActStreamViewResult &y) {
    return x.stream_id_ == y.stream_id_;
  }
};

/**
 * @brief The stream of the ACT device view result on each interface class
 *
 */
class ActDeviceInterfaceStreamResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, stream_id, StreamId);                     ///< Stream ID
  ACT_JSON_FIELD(QString, stream_name, StreamName);                ///< Stream name
  ACT_JSON_FIELD(quint16, vlan_id, VlanId);                        ///< VlanId item (user-defined or system-assigned)
  ACT_JSON_FIELD(quint8, priority_code_point, PriorityCodePoint);  ///< Priority Code Point
  ACT_JSON_ENUM(ActQueueIdEnum, queue_id,
                QueueId);  ///< The queue id
  ACT_JSON_COLLECTION(QList, qint32, frame_offset_list,
                      FrameOffsetList);  ///< FrameOffsetList (not used in class-based)

  ACT_JSON_FIELD(quint32, start_time, StartTime);             ///< The start time of the transmission
  ACT_JSON_FIELD(quint32, stop_time, StopTime);               ///< The stop time of the transmission
  ACT_JSON_ENUM(ActStreamTypeEnum, stream_type, StreamType);  ///< Type of this stream
  ACT_JSON_FIELD(QString, talker, Talker);                    ///< Talker
  ACT_JSON_COLLECTION(QList, QString, listeners, Listeners);  ///< Listener

 public:
  ActDeviceInterfaceStreamResult() {
    queue_id_ = ActQueueIdEnum::k0;
    stream_type_ = ActStreamTypeEnum::kSystemAssignedVlanTag;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActDeviceInterfaceStreamResult &x) {
    qint64 frame_offset_value = 0;
    for (qint32 frame_offset : x.frame_offset_list_) {
      frame_offset_value += frame_offset;
    }
    return qHash(
        x.stream_id_,
        qHash(x.start_time_, qHash(x.stop_time_, qHash(x.vlan_id_, qHash(frame_offset_value, 0)))));  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActDeviceInterfaceStreamResult &x, const ActDeviceInterfaceStreamResult &y) {
    return (x.stream_id_ == y.stream_id_) && (x.start_time_ == y.start_time_) && (x.stop_time_ == y.stop_time_) &&
           (x.frame_offset_list_ == y.frame_offset_list_) && (x.vlan_id_ == y.vlan_id_);
  }
};

/**
 * @brief The ACT device view result on each interface class
 *
 */
class ActDeviceInterfaceResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, interface_id, InterfaceId);       ///< Interface ID
  ACT_JSON_FIELD(QString, interface_name, InterfaceName);  ///< InterfaceName
  ACT_JSON_QT_SET_OBJECTS(ActDeviceInterfaceStreamResult, stream_results, StreamResults);

 public:
  /**
   * @brief Construct a new Act Device Interface Result object
   *
   */
  ActDeviceInterfaceResult() : interface_id_(-1) {}

  /**
   * @brief Construct a new Act Device Interface Result object
   *
   * @param interface_id
   */
  ActDeviceInterfaceResult(const qint64 &interface_id) : interface_id_(interface_id) {}

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActDeviceInterfaceResult &x) {
    return qHash(x.interface_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActDeviceInterfaceResult &x, const ActDeviceInterfaceResult &y) {
    return x.interface_id_ == y.interface_id_;
  }
};

/**
 * @brief The ACT device view result class
 *
 */
class ActDeviceViewResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);  ///< Device ID

  ACT_JSON_QT_SET_OBJECTS(ActDeviceInterfaceResult, interface_results,
                          InterfaceResults);  ///< The schedule result on each interface

  // properties
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);  ///< Device IP

 public:
  /**
   * @brief Construct a new Act Device View Result object
   *
   */
  ActDeviceViewResult() : device_id_(-1) {}

  /**
   * @brief Construct a new Act Device View Result object
   *
   * @param device_id
   */
  ActDeviceViewResult(const qint64 &device_id) : device_id_(device_id) {}

  // /**
  //  * @brief Construct a new Act Device View Result object
  //  *
  //  * @param device_id
  //  * @param device_name
  //  */
  // ActDeviceViewResult(const qint64& device_id, const QString& device_name)
  //     : device_id_(device_id), device_name_(device_name) {}

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActDeviceViewResult &x) {
    return qHash(x.device_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActDeviceViewResult &x, const ActDeviceViewResult &y) {
    return x.device_id_ == y.device_id_;
  }
};

/**
 * @brief The ACT computed result class
 *
 */
class ActComputedResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActRoutingResult, routing_results,
                          RoutingResults);  ///< The routing result of the streams
  ACT_JSON_QT_SET_OBJECTS(ActGclResult, gcl_results,
                          GclResults);  ///< The scheduling result of the streams
  ACT_JSON_QT_SET_OBJECTS(ActStreamViewResult, stream_view_results,
                          StreamViewResults);  ///< The stream-view schedule result
  ACT_JSON_QT_SET_OBJECTS(ActDeviceViewResult, device_view_results,
                          DeviceViewResults);                        ///< The device-view schedule result
  ACT_JSON_QT_SET_OBJECTS(ActDevice, devices, Devices);              ///< The device set in the project
  ACT_JSON_QT_SET_OBJECTS(ActLink, links, Links);                    ///< The link set in the project
  ACT_JSON_QT_SET_OBJECTS(ActStream, streams, Streams);              ///< The stream set in the project
  ACT_JSON_FIELD(qint32, cycle_time, CycleTime);                     ///< The cycle time of the interface at this node
  ACT_JSON_OBJECT(ActTrafficDesign, traffic_design, TrafficDesign);  ///< The traffic design of the project

 public:
  ActComputedResult() : cycle_time_(0) {}

  /**
   * @brief Hide the password field
   *
   * @return ACT_STATUS
   */
  ACT_STATUS HidePassword() {
    ACT_STATUS_INIT();

    // Hide password in each device
    QSet<ActDevice> new_device_set;
    for (auto dev : this->GetDevices()) {
      dev.HidePassword();
      new_device_set.insert(dev);
    }
    this->SetDevices(new_device_set);

    return act_status;
  }

  /**
   * @brief Encrypt the password field
   *
   * @return QString
   */
  ACT_STATUS EncryptPassword() {
    ACT_STATUS_INIT();

    // Encrypt password each device
    QSet<ActDevice> new_device_set;
    for (auto dev : this->GetDevices()) {
      dev.EncryptPassword();
      new_device_set.insert(dev);
    }
    this->SetDevices(new_device_set);

    return act_status;
  }

  /**
   * @brief Decrypt the encrypted password
   *
   * @return QString
   */
  ACT_STATUS DecryptPassword() {
    ACT_STATUS_INIT();

    // Decrypt password each device
    QSet<ActDevice> new_device_set;
    for (auto dev : this->GetDevices()) {
      dev.DecryptPassword();
      new_device_set.insert(dev);
    }
    this->SetDevices(new_device_set);

    return act_status;
  }
};
