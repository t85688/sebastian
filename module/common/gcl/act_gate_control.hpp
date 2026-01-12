/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_status.hpp"

/**
 * @brief The GCL's ActGateControl class
 *
 */
class ActGateControl : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, start_time, StartTime);                   ///< StartTime(ns) item
  ACT_JSON_FIELD(qint64, stop_time, StopTime);                     ///< StopTime(ns) item
  ACT_JSON_FIELD(quint8, gate_states_value, GateStatesValue);      ///< GateStatesValue item (NETCONF use uint8)
  ACT_JSON_FIELD(quint16, vlan_id, VlanId);                        ///< VlanId item (user-defined or system-assigned)
  ACT_JSON_FIELD(quint8, priority_code_point, PriorityCodePoint);  ///< Priority Code Point
  ACT_JSON_FIELD(quint8, queue_id, QueueId);                       ///< Queue id of this stream
  // GateStatesValue: Is the gate states for this entry for the Interface(A bit value of 0 indicates closed; a bit value
  // of 1 indicates open.)(From ieee802-dot1q-sched@2018-09-10 yang)

 public:
  /**
   * @brief Construct a new Gate Control object
   *
   */
  ActGateControl()
      : start_time_(0), stop_time_(0), gate_states_value_(0), vlan_id_(0), priority_code_point_(0), queue_id_(0) {}

  /**
   * @brief Construct a new Gate Control object
   *
   * @param start_time
   * @param stop_time
   * @param gate_states_value
   */
  ActGateControl(const qint64 &start_time, const qint64 &stop_time, const quint8 &gate_states_value)
      : ActGateControl() {
    this->start_time_ = start_time;
    this->stop_time_ = stop_time;
    this->gate_states_value_ = gate_states_value;
  }

  /**
   * @brief Construct a new Gate Control object
   *
   * @param start_time
   * @param stop_time
   * @param gate_states_value
   */
  ActGateControl(const qint64 &start_time, const qint64 &stop_time, const quint8 &gate_states_value,
                 const quint16 &vlan_id, const quint8 &priority_code_point, const quint8 &queue_id)
      : start_time_(start_time),
        stop_time_(stop_time),
        gate_states_value_(gate_states_value),
        vlan_id_(vlan_id),
        priority_code_point_(priority_code_point),
        queue_id_(queue_id) {}

  /**
   * @brief Construct a new Act Gate Control object
   *
   * @param control
   */
  ActGateControl(const ActGateControl &control) {
    this->start_time_ = control.GetStartTime();
    this->stop_time_ = control.GetStopTime();
    this->gate_states_value_ = control.GetGateStatesValue();
    this->vlan_id_ = control.GetVlanId();
    this->priority_code_point_ = control.GetPriorityCodePoint();
    this->queue_id_ = control.GetQueueId();
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActGateControl &x) {
    return qHash(x.start_time_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActGateControl &x, const ActGateControl &y) { return x.start_time_ == y.start_time_; }

  /**
   * @brief The comparison operator for STL Set
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator<(const ActGateControl &x, const ActGateControl &y) { return x.start_time_ < y.start_time_; }
};

/**
 * @brief Get the Gate State Value By Queue Set. E.g., queue 7 => 10000000
 *
 * @param gate_state_value
 * @param queue_set
 * @return ACT_STATUS
 */
inline ACT_STATUS GetGateStateValueByQueueSet(quint8 &gate_state_value, const QSet<quint8> &queue_set) {
  ACT_STATUS_INIT();

  gate_state_value = 0;
  for (const quint8 &queue : queue_set) {
    if (queue > 7) {
      qCritical() << __func__ << "receive an invalid queue value" << QString::number(queue);
      qCritical() << "Queue set:";
      for (const quint8 &queue_ : queue_set) {
        qCritical() << QString::number(queue_);
      }
      return std::make_shared<ActStatusInternalError>("Core");
    }
    gate_state_value += 1 << queue;
  }

  return act_status;
}

/**
 * @brief The path's ActInterfaceGateControls class
 *
 */
class ActInterfaceGateControls : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, interface_id, InterfaceId);                                ///< InterfaceId item
  ACT_JSON_FIELD(QString, interface_name, InterfaceName);                           ///< InterfaceName item
  ACT_JSON_COLLECTION_OBJECTS(QList, ActGateControl, gate_controls, GateControls);  ///< GateControl items list

 public:
  /**
   * @brief Construct a new Act Interface Gate Controls object
   *
   */
  ActInterfaceGateControls() : interface_id_(-1) {}

  /**
   * @brief Construct a new Act Interface Gate Controls object
   *
   * @param interface_id
   * @param interface_name
   */
  ActInterfaceGateControls(const qint64 &interface_id, const QString &interface_name)
      : interface_id_(interface_id), interface_name_(interface_name) {}

  /**
   * @brief Construct a new Act Interface Gate Controls object
   *
   * @param interface_id
   * @param interface_name
   * @param gcl
   */
  ActInterfaceGateControls(const qint64 &interface_id, const QString &interface_name, const QList<ActGateControl> &gcl)
      : interface_id_(interface_id), interface_name_(interface_name), gate_controls_(gcl) {}

  // /**
  //  * @brief Construct a new Act Interface Gate Controls object
  //  *
  //  * @param controls
  //  */
  // ActInterfaceGateControls(const ActInterfaceGateControls& controls) {
  //   this->interface_id_ = controls.GetInterfaceId();
  //   this->gate_controls_ = controls.GetGateControls();
  // }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActInterfaceGateControls &x) {
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
  friend bool operator==(const ActInterfaceGateControls &x, const ActInterfaceGateControls &y) {
    return x.interface_id_ == y.interface_id_;
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator<(const ActInterfaceGateControls &x, const ActInterfaceGateControls &y) {
    return x.interface_id_ < y.interface_id_;
  }
};
