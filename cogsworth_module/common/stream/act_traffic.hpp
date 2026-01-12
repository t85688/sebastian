/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include <QtMath>

#include "act_json.hpp"
#include "act_utilities.hpp"

/**
 * @brief The traffic QoS type enum class
 *
 */
enum class ActTrafficQosTypeEnum { kBandwidth = 0, kBoundedLatency = 1, kDeadline = 2 };

/**
 * @brief The QMap for traffic QoS type enum mapping
 *
 */
static const QMap<QString, ActTrafficQosTypeEnum> kActTrafficQosTypeEnumMap = {
    {"Bandwidth", ActTrafficQosTypeEnum::kBandwidth},
    {"BoundedLatency", ActTrafficQosTypeEnum::kBoundedLatency},
    {"Deadline", ActTrafficQosTypeEnum::kDeadline}};

/**
 * @brief The traffic untagged mode enum class
 *
 */
enum class ActTrafficUntaggedModeEnum { kPVID = 0, kPerStreamPriority = 1 };

/**
 * @brief The QMap for traffic untagged mode enum mapping
 *
 */
static const QMap<QString, ActTrafficUntaggedModeEnum> kActTrafficUntaggedModeEnumMap = {
    {"PVID", ActTrafficUntaggedModeEnum::kPVID}, {"PerStreamPriority", ActTrafficUntaggedModeEnum::kPerStreamPriority}};

/**
 * @brief The advanced traffic type configuration class
 *
 */
class ActTrafficTypeConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, traffic_class, TrafficClass);
  ACT_JSON_FIELD(QString, traffic_type, TrafficType);
  ACT_JSON_QT_SET(quint8, priority_code_point_set, PriorityCodePointSet);
  ACT_JSON_FIELD(qreal, reserved_time, ReservedTime);

 public:
  ActTrafficTypeConfiguration() {
    this->traffic_class_ = 0;
    this->reserved_time_ = 0;
  }

  ActTrafficTypeConfiguration(const quint8 &traffic_class, const QString &traffic_type, const quint32 &reserved_time)
      : ActTrafficTypeConfiguration() {
    this->traffic_class_ = traffic_class;
    this->traffic_type_ = traffic_type;
    this->priority_code_point_set_ = {traffic_class};
    this->reserved_time_ = reserved_time;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   */
  friend bool operator==(const ActTrafficTypeConfiguration &x, const ActTrafficTypeConfiguration &y) {
    return x.traffic_class_ == y.traffic_class_;
  }
};

/**
 * @brief The simple traffic type configuration list class
 *
 */
class ActTrafficTypeConfigurationSetting : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActTrafficTypeConfiguration, traffic_type_configuration_setting,
                              TrafficTypeConfigurationSetting);
};

/**
 * @brief The advanced traffic setting class
 *
 */
class ActTrafficSetting : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, traffic_type_class, TrafficTypeClass);
  ACT_JSON_ENUM(ActTrafficQosTypeEnum, qos_type, QosType);
  ACT_JSON_FIELD(qreal, min_receive_offset, MinReceiveOffset);
  ACT_JSON_FIELD(qreal, max_receive_offset, MaxReceiveOffset);

 public:
  ActTrafficSetting() {
    this->traffic_type_class_ = 6;
    this->qos_type_ = ActTrafficQosTypeEnum::kBandwidth;
    this->min_receive_offset_ = 1;
    this->max_receive_offset_ = 1000;
  }
};

/**
 * @brief The advanced traffic tsn class
 *
 */
class ActTrafficTSN : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, active, Active);
  ACT_JSON_FIELD(bool, frer, FRER);

 public:
  ActTrafficTSN() {
    this->active_ = false;
    this->frer_ = false;
  }
};

enum class ActStreamPriorityTypeEnum { kInactive = -1, kEthertype = 0, kTcp = 1, kUdp = 2 };

static const QMap<QString, ActStreamPriorityTypeEnum> kActStreamPriorityTypeEnumMap = {
    {"inactive", ActStreamPriorityTypeEnum::kInactive},
    {"etherType", ActStreamPriorityTypeEnum::kEthertype},
    {"tcpPort", ActStreamPriorityTypeEnum::kTcp},
    {"udpPort", ActStreamPriorityTypeEnum::kUdp}};  ///< The the string mapping map of the ActStreamPriorityTypeEnum

// For QSet (Currently used in ActStadPortEntry)
inline uint qHash(ActStreamPriorityTypeEnum key, uint seed) {
  return qHash(static_cast<uint>(key), seed);
}  ///< For QSet

// Note: keep for ActDeployData.SetStadPortTable(device, port_id, stream_priority_setting)
class ActStreamPrioritySetting : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Base (L2)
  ACT_JSON_FIELD(quint16, vlan_id, VlanId);
  ACT_JSON_FIELD(quint8, priority_code_point, PriorityCodePoint);
  ACT_JSON_FIELD(quint32, ether_type, EtherType);
  ACT_JSON_FIELD(quint16, sub_type, SubType);
  ACT_JSON_FIELD(bool, enable_sub_type, EnableSubType);

  // L3: type, tcpPort, udpPort
  ACT_JSON_QT_SET_ENUM(ActStreamPriorityTypeEnum, type, Type);  ///< The Types
  ACT_JSON_FIELD(qint32, udp_port, UdpPort);                    ///< The UdpPort
  ACT_JSON_FIELD(qint32, tcp_port, TcpPort);                    ///< The TcpPort

 public:
  ActStreamPrioritySetting() {
    this->vlan_id_ = 0;
    this->priority_code_point_ = 0;
    this->ether_type_ = 0;
    this->sub_type_ = 0;
    this->enable_sub_type_ = false;
    this->type_ = {ActStreamPriorityTypeEnum::kEthertype};
    this->udp_port_ = 1;
    this->tcp_port_ = 1;
  }
};

/**
 * @brief The advanced traffic vlan class
 *
 */
class ActTrafficVlanSetting : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // per-stream priority
  ACT_JSON_QT_SET_ENUM(ActStreamPriorityTypeEnum, type, Type);
  ACT_JSON_FIELD(quint32, ether_type, EtherType);
  ACT_JSON_FIELD(bool, enable_sub_type, EnableSubType);
  ACT_JSON_FIELD(quint16, sub_type, SubType);
  ACT_JSON_FIELD(qint32, udp_port, UdpPort);
  ACT_JSON_FIELD(qint32, tcp_port, TcpPort);

  ACT_JSON_FIELD(quint16, vlan_id, VlanId);
  ACT_JSON_FIELD(quint8, priority_code_point, PriorityCodePoint);

  ACT_JSON_FIELD(bool, tagged, Tagged);
  ACT_JSON_ENUM(ActTrafficUntaggedModeEnum, untagged_mode, UntaggedMode);
  ACT_JSON_FIELD(bool, user_defined_vlan, UserDefinedVlan);

 public:
  ActTrafficVlanSetting() {
    this->type_ = {ActStreamPriorityTypeEnum::kEthertype};
    this->ether_type_ = 0;
    this->enable_sub_type_ = false;
    this->sub_type_ = 0;
    this->udp_port_ = 1;
    this->tcp_port_ = 1;
    this->vlan_id_ = 0;
    this->priority_code_point_ = 0;

    this->tagged_ = false;
    this->untagged_mode_ = ActTrafficUntaggedModeEnum::kPerStreamPriority;
    this->user_defined_vlan_ = false;
  }
};

/**
 * @brief The advanced traffic tsn class
 *
 */
class ActTrafficStreamParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qreal, interval, Interval);
  ACT_JSON_FIELD(quint32, max_frame_size, MaxFrameSize);
  ACT_JSON_FIELD(quint32, max_frames_per_interval, MaxFramesPerInterval);
  ACT_JSON_FIELD(quint32, max_bytes_per_interval, MaxBytesPerInterval);
  ACT_JSON_FIELD(qreal, earliest_transmit_offset, EarliestTransmitOffset);
  ACT_JSON_FIELD(qreal, latest_transmit_offset, LatestTransmitOffset);
  ACT_JSON_FIELD(qreal, jitter, Jitter);

 public:
  ActTrafficStreamParameter() {
    this->interval_ = 1000;
    this->max_frame_size_ = 46;
    this->max_frames_per_interval_ = 1;
    this->max_bytes_per_interval_ = 46;
    this->earliest_transmit_offset_ = 0;
    this->latest_transmit_offset_ = 1000;
    this->jitter_ = 0;
  }
};

/**
 * @brief The advanced traffic application class
 *
 */
class ActTrafficApplication : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, application_name, ApplicationName);
  ACT_JSON_OBJECT(ActTrafficSetting, traffic_setting, TrafficSetting);
  ACT_JSON_OBJECT(ActTrafficTSN, tsn, TSN);
  ACT_JSON_OBJECT(ActTrafficVlanSetting, vlan_setting, VlanSetting);
  ACT_JSON_OBJECT(ActTrafficStreamParameter, stream_parameter, StreamParameter);

 public:
  ActTrafficApplication() { this->id_ = 0; }

  ActTrafficApplication(qint64 id) : ActTrafficApplication() { this->id_ = id; }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActTrafficApplication &obj) {
    return qHash(obj.id_, 0);  // arbitrary value is 0
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   */
  friend bool operator==(const ActTrafficApplication &x, const ActTrafficApplication &y) { return x.id_ == y.id_; }
};

/**
 * @brief The simple traffic application list class
 *
 */
class ActTrafficApplicationSetting : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActTrafficApplication, application_setting, ApplicationSetting);
};

/**
 * @brief The advanced traffic stream interface class
 *
 */
class ActTrafficStreamInterface : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(qint64, interface_id, InterfaceId);

 public:
  ActTrafficStreamInterface() {
    this->device_id_ = 0;
    this->interface_id_ = 0;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActTrafficStreamInterface &obj) {
    return qHash(obj.device_id_, 0);  // arbitrary value is 0
  }

  /**
   * @brief The equal operator
   *
   * @param source The copied source
   */
  friend bool operator==(const ActTrafficStreamInterface &x, const ActTrafficStreamInterface &y) {
    return x.device_id_ == y.device_id_ && x.interface_id_ == y.interface_id_;
  }
};

/**
 * @brief The advanced traffic stream class
 *
 */
class ActTrafficStream : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(bool, active, Active);
  ACT_JSON_FIELD(QString, stream_name, StreamName);
  ACT_JSON_FIELD(qint64, application_id, ApplicationId);
  ACT_JSON_FIELD(bool, multicast, Multicast);
  ACT_JSON_FIELD(QString, destination_mac, DestinationMac);
  ACT_JSON_OBJECT(ActTrafficStreamInterface, talker, Talker);
  ACT_JSON_QT_SET_OBJECTS(ActTrafficStreamInterface, listeners, Listeners);

 public:
  ActTrafficStream() {
    this->id_ = 0;
    this->active_ = true;
    this->application_id_ = 0;
    this->multicast_ = false;
  }

  ActTrafficStream(qint64 id) : ActTrafficStream() { this->id_ = id; }

  ACT_STATUS GetApplicationSetting(const QSet<ActTrafficApplication> &applications,
                                   ActTrafficApplication &result_application) {
    ACT_STATUS_INIT();

    act_status = ActGetItemById<ActTrafficApplication>(applications, this->application_id_, result_application);
    if (!IsActStatusSuccess(act_status)) {
      QString not_found_elem = QString("Application(%1)").arg(this->application_id_);
      qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
      return std::make_shared<ActStatusNotFound>(not_found_elem);
    }

    return act_status;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActTrafficStream &obj) {
    return qHash(obj.id_, 0);  // arbitrary value is 0
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   */
  friend bool operator==(const ActTrafficStream &x, const ActTrafficStream &y) { return x.id_ == y.id_; }
};

/**
 * @brief The simple traffic stream list class
 *
 */
class ActTrafficStreamSetting : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActTrafficStream, stream_setting, StreamSetting);
};

/**
 * @brief The advanced traffic design class
 *
 */
class ActTrafficDesign : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActTrafficTypeConfiguration, traffic_type_configuration_setting,
                              TrafficTypeConfigurationSetting);
  ACT_JSON_QT_SET_OBJECTS(ActTrafficApplication, application_setting, ApplicationSetting);
  ACT_JSON_QT_SET_OBJECTS(ActTrafficStream, stream_setting, StreamSetting);

 public:
  ActTrafficDesign() {
    this->traffic_type_configuration_setting_.append(ActTrafficTypeConfiguration(0, "Best Effort Low", 30));
    this->traffic_type_configuration_setting_.append(ActTrafficTypeConfiguration(1, "Best Effort High", 30));
    this->traffic_type_configuration_setting_.append(ActTrafficTypeConfiguration(2, "Configuration & Diagnostics", 20));
    this->traffic_type_configuration_setting_.append(ActTrafficTypeConfiguration(3, "Alarms and Events", 20));
    this->traffic_type_configuration_setting_.append(ActTrafficTypeConfiguration(4, "Network Control", 50));
    this->traffic_type_configuration_setting_.append(ActTrafficTypeConfiguration(5, "Cyclic-Asynchronous", 500));
    this->traffic_type_configuration_setting_.append(ActTrafficTypeConfiguration(6, "Cyclic-Synchronous", 500));
    this->traffic_type_configuration_setting_.append(ActTrafficTypeConfiguration(7, "Isochronous", 500));
  }

  ACT_STATUS GetStreamSettingByStreamID(const qint64 &stream_id, ActTrafficStream &result_stream) {
    ACT_STATUS_INIT();

    act_status = ActGetItemById<ActTrafficStream>(this->stream_setting_, stream_id, result_stream);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "StreamSetting not found. StreamID:" << stream_id;
      return std::make_shared<ActStatusInternalError>("Deploy");
    }

    return act_status;
  }

  ACT_STATUS GetApplicationSettingByStreamID(const qint64 &stream_id, ActTrafficApplication &result_application) {
    ACT_STATUS_INIT();

    // Get TrafficStream
    ActTrafficStream stream_setting;
    act_status = GetStreamSettingByStreamID(stream_id, stream_setting);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStreamSettingByStreamID() failed";
      return act_status;
    }

    // Get ApplicationSetting
    ActTrafficApplication app_setting;
    act_status = stream_setting.GetApplicationSetting(this->application_setting_, app_setting);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__
                  << "ApplicationSetting not found. ApplicationSettingID:" << stream_setting.GetApplicationId();
      return std::make_shared<ActStatusInternalError>("Deploy");
    }

    result_application = app_setting;
    return act_status;
  }
};