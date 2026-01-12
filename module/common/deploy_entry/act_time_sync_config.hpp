/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "stream/act_interface.hpp"

class ActTimeSync802Dot1ASPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, port_id, PortId);
  ACT_JSON_FIELD(bool, enable, Enable);

  ACT_JSON_FIELD(qint32, announce_interval, AnnounceInterval);
  ACT_JSON_FIELD(qint32, announce_receipt_timeout, AnnounceReceiptTimeout);
  ACT_JSON_FIELD(qint32, sync_interval, SyncInterval);
  ACT_JSON_FIELD(qint32, sync_receipt_timeout, SyncReceiptTimeout);
  ACT_JSON_FIELD(qint32, pdelay_req_interval, PdelayReqInterval);
  ACT_JSON_FIELD(qint32, neighbor_prop_delay_thresh, NeighborPropDelayThresh);

 public:
  /**
   * @brief Construct a new Act 802Dot1AS Port Entry object
   *
   */
  ActTimeSync802Dot1ASPortEntry() {
    this->port_id_ = -1;
    this->enable_ = true;
    this->announce_interval_ = 0;
    this->announce_receipt_timeout_ = 3;
    this->sync_interval_ = -3;
    this->sync_receipt_timeout_ = 3;
    this->pdelay_req_interval_ = 0;
    this->neighbor_prop_delay_thresh_ = 800;
  }

  /**
   * @brief Construct a new Act 802Dot1AS Port Entry object
   *
   * @param port_id
   */
  ActTimeSync802Dot1ASPortEntry(const qint32 &port_id) : ActTimeSync802Dot1ASPortEntry() { this->port_id_ = port_id; }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActTimeSync802Dot1ASPortEntry &x) {
    return qHash(x.port_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActTimeSync802Dot1ASPortEntry &x, const ActTimeSync802Dot1ASPortEntry &y) {
    return x.port_id_ == y.port_id_;
  }
};

class ActTimeSync802Dot1ASConfig : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_FIELD(quint32, priority1, Priority1);
  ACT_JSON_FIELD(quint32, priority2, Priority2);
  ACT_JSON_FIELD(quint32, clock_class, ClockClass);
  ACT_JSON_FIELD(quint32, clock_accuracy, ClockAccuracy);
  ACT_JSON_FIELD(quint32, accuracy_alert, AccuracyAlert);

  ACT_JSON_QT_SET_OBJECTS(ActTimeSync802Dot1ASPortEntry, port_entries, PortEntries);

 public:
  /**
   * @brief Construct a new Act IEEE_802Dot1AS_2011 Profile Config object
   *
   */
  ActTimeSync802Dot1ASConfig() {
    this->priority1_ = 246;
    this->priority2_ = 248;
    this->clock_class_ = 248;
    this->clock_accuracy_ = 254;
    this->accuracy_alert_ = 500;
  }

  ActTimeSync802Dot1ASConfig(const QList<ActInterface> &interfaces) : ActTimeSync802Dot1ASConfig() {
    for (ActInterface intf : interfaces) {
      this->port_entries_.insert(ActTimeSync802Dot1ASPortEntry(intf.GetInterfaceId()));
    }
  }
};

class ActTimeSync1588PortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, port_id, PortId);
  ACT_JSON_FIELD(bool, enable, Enable);

  ACT_JSON_FIELD(qint32, announce_interval, AnnounceInterval);
  ACT_JSON_FIELD(qint32, announce_receipt_timeout, AnnounceReceiptTimeout);
  ACT_JSON_FIELD(qint32, sync_interval, SyncInterval);
  ACT_JSON_FIELD(qint32, delay_req_interval, DelayReqInterval);
  ACT_JSON_FIELD(qint32, pdelay_req_interval, PdelayReqInterval);

 public:
  /**
   * @brief Construct a new Act IEEE_1588_2008 Port Entry object
   *
   */
  ActTimeSync1588PortEntry() {
    this->port_id_ = -1;
    this->enable_ = true;
    this->announce_interval_ = 1;
    this->announce_receipt_timeout_ = 3;
    this->sync_interval_ = 0;
    this->delay_req_interval_ = 0;
    this->pdelay_req_interval_ = 0;
  }

  /**
   * @brief Construct a new Act IEEE_1588_2008 Port Entry object
   *
   * @param port_id
   */
  ActTimeSync1588PortEntry(const qint32 &port_id) : ActTimeSync1588PortEntry() { this->port_id_ = port_id; }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActTimeSync1588PortEntry &x) {
    return qHash(x.port_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActTimeSync1588PortEntry &x, const ActTimeSync1588PortEntry &y) {
    return x.port_id_ == y.port_id_;
  }
};

enum class Act1588ClockTypeEnum { kBoundaryClock = 2, kTransparentClock = 3 };
static const QMap<QString, Act1588ClockTypeEnum> kAct1588ClockTypeEnumMap = {
    {"BoundaryClock", Act1588ClockTypeEnum::kBoundaryClock},
    {"TransparentClock", Act1588ClockTypeEnum::kTransparentClock}};

enum class Act1588DelayMechanismEnum { kEndToEnd = 1, kPeerToPeer = 2 };
static const QMap<QString, Act1588DelayMechanismEnum> kAct1588DelayMechanismEnumMap = {
    {"EndToEnd", Act1588DelayMechanismEnum::kEndToEnd}, {"PeerToPeer", Act1588DelayMechanismEnum::kPeerToPeer}};

enum class Act1588TransportTypeEnum { kUDPIPv4 = 1, kUDPIPv6 = 2, kIEEE802Dot3Ethernet = 3 };
static const QMap<QString, Act1588TransportTypeEnum> kAct1588TransportTypeEnumMap = {
    {"UDPIPv4", Act1588TransportTypeEnum::kUDPIPv4},
    {"UDPIPv6", Act1588TransportTypeEnum::kUDPIPv6},
    {"IEEE802Dot3Ethernet", Act1588TransportTypeEnum::kIEEE802Dot3Ethernet}};

enum class Act1588ClockModeEnum { kOneStep = 1, kTwoStep = 2 };
static const QMap<QString, Act1588ClockModeEnum> kAct1588ClockModeEnumMap = {
    {"OneStep", Act1588ClockModeEnum::kOneStep}, {"TwoStep", Act1588ClockModeEnum::kTwoStep}};

class ActTimeSync1588Config : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(Act1588ClockTypeEnum, clock_type, ClockType);
  ACT_JSON_ENUM(Act1588DelayMechanismEnum, delay_mechanism, DelayMechanism);
  ACT_JSON_ENUM(Act1588TransportTypeEnum, transport_type, TransportType);

  ACT_JSON_FIELD(quint32, priority1, Priority1);
  ACT_JSON_FIELD(quint32, priority2, Priority2);
  ACT_JSON_FIELD(quint32, domain_number, DomainNumber);
  ACT_JSON_ENUM(Act1588ClockModeEnum, clock_mode, ClockMode);  // enum one-step(1), two-step(2)
  ACT_JSON_FIELD(quint32, accuracy_alert, AccuracyAlert);
  ACT_JSON_FIELD(quint32, maximum_steps_removed, MaximumStepsRemoved);  // tsn no data

  ACT_JSON_FIELD(quint32, clock_class, ClockClass);        // UI no field
  ACT_JSON_FIELD(quint32, clock_accuracy, ClockAccuracy);  //  UI no field

  ACT_JSON_QT_SET_OBJECTS(ActTimeSync1588PortEntry, port_entries, PortEntries);

 public:
  /**
   * @brief Construct a new Act IEEE_1588_2008 Profile Config object
   *
   */
  ActTimeSync1588Config() {
    this->clock_type_ = Act1588ClockTypeEnum::kBoundaryClock;
    this->delay_mechanism_ = Act1588DelayMechanismEnum::kEndToEnd;
    this->transport_type_ = Act1588TransportTypeEnum::kIEEE802Dot3Ethernet;

    this->priority1_ = 128;
    this->priority2_ = 128;
    this->domain_number_ = 0;
    this->clock_mode_ = Act1588ClockModeEnum::kTwoStep;
    this->accuracy_alert_ = 1000;
    this->maximum_steps_removed_ = 255;

    this->clock_class_ = 248;
    this->clock_accuracy_ = 254;
  }

  ActTimeSync1588Config(const QList<ActInterface> &interfaces) : ActTimeSync1588Config() {
    for (ActInterface intf : interfaces) {
      this->port_entries_.insert(ActTimeSync1588PortEntry(intf.GetInterfaceId()));
    }
  }
};

class ActTimeSyncDefaultPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, port_id, PortId);
  ACT_JSON_FIELD(bool, enable, Enable);

  ACT_JSON_FIELD(qint32, announce_interval, AnnounceInterval);
  ACT_JSON_FIELD(qint32, announce_receipt_timeout, AnnounceReceiptTimeout);
  ACT_JSON_FIELD(qint32, sync_interval, SyncInterval);
  ACT_JSON_FIELD(qint32, delay_req_interval, DelayReqInterval);
  ACT_JSON_FIELD(qint32, pdelay_req_interval, PdelayReqInterval);

 public:
  /**
   * @brief Construct a new Act 802Dot1AS Port Entry object
   *
   */
  ActTimeSyncDefaultPortEntry() {
    this->port_id_ = -1;
    this->enable_ = true;
    this->announce_interval_ = 0;
    this->announce_receipt_timeout_ = 3;
    this->sync_interval_ = 0;
    this->delay_req_interval_ = 0;
    this->pdelay_req_interval_ = 0;
  }

  /**
   * @brief Construct a new Act 802Dot1AS Port Entry object
   *
   * @param port_id
   */
  ActTimeSyncDefaultPortEntry(const qint32 &port_id) : ActTimeSyncDefaultPortEntry() { this->port_id_ = port_id; }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActTimeSyncDefaultPortEntry &x) {
    return qHash(x.port_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActTimeSyncDefaultPortEntry &x, const ActTimeSyncDefaultPortEntry &y) {
    return x.port_id_ == y.port_id_;
  }
};

class ActTimeSyncIec61850Config : public ActTimeSync1588Config {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(Act1588ClockTypeEnum, clock_type, ClockType);
  ACT_JSON_ENUM(Act1588DelayMechanismEnum, delay_mechanism, DelayMechanism);
  ACT_JSON_ENUM(Act1588TransportTypeEnum, transport_type, TransportType);

  ACT_JSON_FIELD(quint32, priority1, Priority1);
  ACT_JSON_FIELD(quint32, priority2, Priority2);
  ACT_JSON_FIELD(quint32, domain_number, DomainNumber);
  ACT_JSON_ENUM(Act1588ClockModeEnum, clock_mode, ClockMode);  // enum one-step(1), two-step(2)
  ACT_JSON_FIELD(quint32, accuracy_alert, AccuracyAlert);
  ACT_JSON_FIELD(quint32, maximum_steps_removed, MaximumStepsRemoved);  // tsn no data

  ACT_JSON_FIELD(quint32, clock_class, ClockClass);        // UI no field
  ACT_JSON_FIELD(quint32, clock_accuracy, ClockAccuracy);  //  UI no field

  ACT_JSON_QT_SET_OBJECTS(ActTimeSyncDefaultPortEntry, port_entries, PortEntries);

 public:
  /**
   * @brief Construct a new Act IEEE_61850_2008 Profile Config object
   *
   */
  ActTimeSyncIec61850Config() {
    this->clock_type_ = Act1588ClockTypeEnum::kBoundaryClock;
    this->delay_mechanism_ = Act1588DelayMechanismEnum::kPeerToPeer;
    this->transport_type_ = Act1588TransportTypeEnum::kIEEE802Dot3Ethernet;

    this->priority1_ = 128;
    this->priority2_ = 128;
    this->domain_number_ = 0;
    this->clock_mode_ = Act1588ClockModeEnum::kTwoStep;
    this->accuracy_alert_ = 1000;
    this->maximum_steps_removed_ = 255;

    this->clock_class_ = 248;
    this->clock_accuracy_ = 254;
  }

  ActTimeSyncIec61850Config(const QList<ActInterface> &interfaces) : ActTimeSyncIec61850Config() {
    for (ActInterface intf : interfaces) {
      this->port_entries_.insert(ActTimeSyncDefaultPortEntry(intf.GetInterfaceId()));
    }
  }
};

class ActTimeSyncC37Dot238Config : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_ENUM(Act1588ClockTypeEnum, clock_type, ClockType);
  ACT_JSON_ENUM(Act1588DelayMechanismEnum, delay_mechanism, DelayMechanism);
  ACT_JSON_ENUM(Act1588TransportTypeEnum, transport_type, TransportType);

  ACT_JSON_FIELD(quint32, priority1, Priority1);
  ACT_JSON_FIELD(quint32, priority2, Priority2);
  ACT_JSON_FIELD(quint32, domain_number, DomainNumber);
  ACT_JSON_ENUM(Act1588ClockModeEnum, clock_mode, ClockMode);  // enum one-step(1), two-step(2)
  ACT_JSON_FIELD(quint32, accuracy_alert, AccuracyAlert);
  ACT_JSON_FIELD(quint32, grandmaster_id, GrandmasterId);

  ACT_JSON_FIELD(quint32, clock_class, ClockClass);                     // UI no field
  ACT_JSON_FIELD(quint32, clock_accuracy, ClockAccuracy);               //  UI no field
  ACT_JSON_FIELD(quint32, maximum_steps_removed, MaximumStepsRemoved);  // UI no field

  ACT_JSON_QT_SET_OBJECTS(ActTimeSyncDefaultPortEntry, port_entries, PortEntries);

 public:
  /**
   * @brief Construct a new Act IEEE_1588_2008 Profile Config object
   *
   */
  ActTimeSyncC37Dot238Config() {
    this->clock_type_ = Act1588ClockTypeEnum::kBoundaryClock;
    this->delay_mechanism_ = Act1588DelayMechanismEnum::kPeerToPeer;
    this->transport_type_ = Act1588TransportTypeEnum::kIEEE802Dot3Ethernet;

    this->priority1_ = 128;
    this->priority2_ = 128;
    this->domain_number_ = 254;
    this->clock_mode_ = Act1588ClockModeEnum::kTwoStep;
    this->accuracy_alert_ = 1000;
    this->grandmaster_id_ = 255;

    this->clock_class_ = 248;
    this->clock_accuracy_ = 254;
    this->maximum_steps_removed_ = 255;
  }

  ActTimeSyncC37Dot238Config(const QList<ActInterface> &interfaces) : ActTimeSyncC37Dot238Config() {
    for (ActInterface intf : interfaces) {
      this->port_entries_.insert(ActTimeSyncDefaultPortEntry(intf.GetInterfaceId()));
    }
  }
};
