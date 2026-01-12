/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_SCHEDULE_CONFIG_H
#define ACT_SCHEDULE_CONFIG_H

#include "act_json.hpp"
#include "act_project.hpp"
#include "act_utilities.hpp"

class ActScheduleConfigInterface : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(qint64, interface_id, InterfaceId);
  ACT_JSON_FIELD(QString, interface_name, InterfaceName);

  ACT_JSON_FIELD(qint64, link_id, LinkId);

  ACT_JSON_FIELD(qint64, connect_device_id, ConnectDeviceId);
  ACT_JSON_FIELD(qint64, connect_interface_id, ConnectInterfaceId);

  ACT_JSON_FIELD(quint16, independent_processing_delay,
                 IndependentProcessingDelay);  ///< the independent processing delay of the device in ns
  ACT_JSON_FIELD(quint16, dependent_processing_delay_ratio,
                 DependentProcessingDelayRatio);  ///< the dependent processing delay of the device in ns/bit

 public:
  /**
   * @brief Construct a new Act Scheduling Stream object
   *
   */
  ActScheduleConfigInterface()
      : device_id_(-1),
        interface_id_(-1),
        link_id_(-1),
        connect_device_id_(-1),
        connect_interface_id_(-1),
        independent_processing_delay_(0),
        dependent_processing_delay_ratio_(0) {}

  ActScheduleConfigInterface(const qint64 &device_id, const qint64 &interface_id, const qint64 &link_id)
      : ActScheduleConfigInterface() {
    this->device_id_ = device_id;
    this->interface_id_ = interface_id;
    this->link_id_ = link_id;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param stream
   * @return uint
   */
  friend uint qHash(const ActScheduleConfigInterface &schedule_config_interface) {
    return qHash(schedule_config_interface.device_id_, 0x0);
  }

  /**
   * @brief The "==" comparison of Scheduling Stream for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActScheduleConfigInterface &x, const ActScheduleConfigInterface &y) {
    return x.device_id_ == y.device_id_ && x.interface_id_ == y.interface_id_ && x.link_id_ == y.link_id_;
  }
};

class ActScheduleConfigStream : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, stream_id, StreamId);       ///< the stream identity
  ACT_JSON_FIELD(QString, stream_name, StreamName);  ///< the stream name
  ACT_JSON_FIELD(bool, multicast, Multicast);
  ACT_JSON_FIELD(QString, destination_mac, DestinationMac);

  ACT_JSON_OBJECT(ActScheduleConfigInterface, talker, Talker);
  ACT_JSON_QT_SET_OBJECTS(ActScheduleConfigInterface, listeners, Listeners);

  ACT_JSON_FIELD(QString, application_name, ApplicationName);
  ACT_JSON_FIELD(bool, tsn_enable, TsnEnable);
  ACT_JSON_FIELD(bool, cb_enable, CbEnable);
  ACT_JSON_FIELD(bool, keep_previous_result, KeepPreviousResult);

  ACT_JSON_FIELD(QString, traffic_type, TrafficType);
  ACT_JSON_ENUM(ActTrafficQosTypeEnum, qos_type, QosType);  ///< The QoS type
  ACT_JSON_QT_SET(quint8, pcps, PCPs);

  ACT_JSON_FIELD(qint64, min_receive_offset, MinReceiveOffset);  ///< MinReceiveOffset in ns
  ACT_JSON_FIELD(qint64, max_receive_offset, MaxReceiveOffset);  ///< MaxReceiveOffset in ns

  ACT_JSON_FIELD(quint16, vlan_id, VlanId);
  ACT_JSON_FIELD(quint8, queue_id, QueueId);
  ACT_JSON_FIELD(quint8, priority_code_point, PriorityCodePoint);

  ACT_JSON_FIELD(qint64, interval, Interval);  ///< the stream interval in ns
  ACT_JSON_FIELD(qint64, payload, Payload);    ///< the total payload of a stream in bit

  ACT_JSON_FIELD(qint64, earliest_transmit_offset, EarliestTransmitOffset);  ///< the earliest transmit offset in ns
  ACT_JSON_FIELD(qint64, latest_transmit_offset, LatestTransmitOffset);      ///< the latest transmit offset in ns
  ACT_JSON_FIELD(qint64, jitter, Jitter);                                    ///< the transmission jitter in ns

 public:
  ActScheduleConfigStream() {
    this->stream_id_ = -1;
    this->multicast_ = false;
    this->tsn_enable_ = false;
    this->cb_enable_ = false;
    this->qos_type_ = ActTrafficQosTypeEnum::kBandwidth;
    this->min_receive_offset_ = 0;
    this->max_receive_offset_ = 0;

    this->vlan_id_ = 0;
    this->queue_id_ = 0;
    this->priority_code_point_ = 0;

    this->interval_ = 1000000;
    this->payload_ = 0;

    this->earliest_transmit_offset_ = 0;
    this->latest_transmit_offset_ = 1000000;
    this->jitter_ = 0;
  }
};

class ActScheduleConfigDevice : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(QString, ip_address, IpAddress);
  ACT_JSON_FIELD(quint16, tick_granularity,
                 TickGranularity);  ///< the tick granularity of device in ns
  ACT_JSON_FIELD(quint16, number_of_queue, NumberOfQueue);
  ACT_JSON_FIELD(quint16, per_queue_size, PerQueueSize);
  ACT_JSON_FIELD(quint16, gate_control_list_length, GateControlListLength);
  ACT_JSON_FIELD(bool, tsn_enable, TsnEnable);
  ACT_JSON_FIELD(bool, cb_enable, CbEnable);
  ACT_JSON_FIELD(bool, end_station, EndStation);
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActScheduleConfigInterface, schedule_config_interfaces,
                           ScheduleConfigInterfaces);

 public:
  ActScheduleConfigDevice() {}

  ActScheduleConfigInterface &GetScheduleConfigInterface(const qint64 &interface_id);
};

class ActScheduleConfigLink : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, link_id, LinkId);                      ///< the link identity
  ACT_JSON_FIELD(qint64, bandwidth, Bandwidth);                 ///< the bandwidth of the link
  ACT_JSON_FIELD(qint64, propagation_delay, PropagationDelay);  ///< the propagation delay of the link in ns
  ACT_JSON_FIELD(qint64, time_sync_delay, TimeSyncDelay);  ///< time synchronization delay of the source device in ns
};

class ActScheduleConfigVLAN : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, min_vlan, MinVLAN);
  ACT_JSON_FIELD(quint16, max_vlan, MaxVLAN);
  ACT_JSON_FIELD(quint16, last_assign_vlan, LastAssignVLAN);

 private:
  QMap<QString, QSet<quint16>> vlan_map_;

 public:
  ActScheduleConfigVLAN() : min_vlan_(0), max_vlan_(65535), last_assign_vlan_(0) {}

  bool SetVLAN(quint16 &vlan_id, QString &destination_mac);
  ACT_STATUS GenerateSystemAssignVLAN(quint16 &vlan_id, QString &destination_mac);
  bool IsAvailableVLAN(quint16 &vlan_id, QString &destination_mac);
};

class ActScheduleConfig : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, cycle_time, CycleTime);
  ACT_JSON_FIELD(qint64, reserve_time, ReserveTime);
  ACT_JSON_OBJECT(ActScheduleConfigVLAN, schedule_confic_vlan, ScheduleConfigVLAN);
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActScheduleConfigStream, schedule_config_streams, ScheduleConfigStreams);
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActScheduleConfigDevice, schedule_config_devices, ScheduleConfigDevices);
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActScheduleConfigLink, schedule_config_links, ScheduleConfigLinks);

 public:
  ActScheduleConfig() : cycle_time_(1), reserve_time_(50) {}

  ACT_STATUS PrepareScheduleConfigVLAN(ActProject &act_project);
  ACT_STATUS PrepareScheduleConfigStream(ActProject &act_project);
  ACT_STATUS PrepareScheduleConfigDevice(ActProject &act_project);
  ACT_STATUS PrepareScheduleConfigLink(ActProject &act_project);

  void ComputeCycleTime();
  QList<qint64> SortScheduleConfigStreams();
  bool IsListenerArrived(const qint64 &stream_id, const qint64 &device_id, const qint64 &ingress_interface_id,
                         const qint64 &egress_interface_id, const qint64 &link_id);

  ActScheduleConfigStream &GetScheduleConfigStream(const qint64 &stream_id);
  ActScheduleConfigDevice &GetScheduleConfigDevice(const qint64 &device_id);
  ActScheduleConfigLink &GetScheduleConfigLink(const qint64 &link_id);
  ActScheduleConfigInterface &GetScheduleConfigInterface(const qint64 &device_id, const qint64 &interface_id);

  qint64 ComputeStreamDuration(const qint64 &stream_id, const qint64 &link_id);
  qint64 ComputeTotalDelay(const qint64 &device_id, const qint64 &interface_id, const qint64 &stream_id);
  qint64 ComputeTransmitDelay(const qint64 &link_id);
  qint64 ComputeProcessingDelay(const qint64 &device_id, const qint64 &interface_id, const qint64 &stream_id);
};

#endif