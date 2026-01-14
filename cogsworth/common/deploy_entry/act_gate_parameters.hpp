/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
// #include <QtGlobal>

#include "act_json.hpp"
#include "gcl/act_admin_base_time.hpp"
#include "gcl/act_admin_cycle_time.hpp"

/**
 * @brief The ActSGSParams class
 *
 */
class ActSGSParams : public QSerializer {  // set-gate-states
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, gate_states_value, GateStatesValue);       ///< The value of gate state
  ACT_JSON_FIELD(quint32, time_interval_value, TimeIntervalValue);  ///< The value of time interval

 public:
  /**
   * @brief Construct a new ActSGSParams object
   *
   */
  ActSGSParams() : gate_states_value_(0), time_interval_value_(0) {}

  /**
   * @brief Construct a new ActSGSParams object
   *
   * @param gate_states_value
   * @param time_interval_value
   */
  ActSGSParams(const quint8 &gate_states_value, const quint32 &time_interval_value)
      : gate_states_value_(gate_states_value), time_interval_value_(time_interval_value) {}

  /**
   * @brief Get the Gate State Value By Queue Set. E.g., queue 7 => 10000000
   *
   * @param gate_state_value
   * @param queue_set
   * @return ACT_STATUS
   */
  inline quint8 GetGateStateValueByQueueSet(const QList<quint8> &queue_set) {
    quint8 gate_state_value = 0;
    for (const quint8 &queue : queue_set) {
      gate_state_value += 1 << queue;
    }

    return gate_state_value;
  }

  /**
   * @brief Get the Queue Set By Gate State Value. E.g., queue 10000000 -> queue 7
   *
   * @param gate_state_value
   * @return QSet<quint8>
   */
  inline auto GetQueueSetByGateStateValue(quint8 &gate_state_value) -> QList<quint8> {
    QList<quint8> queue_set;

    for (quint8 i = 0; i < 8; ++i) {
      if (gate_state_value & (1 << i)) {
        queue_set.append(i);
      }
    }

    return queue_set;
  }
};

/**
 * @brief The AdminControl class
 *
 */
class ActAdminControl : public QSerializer {  // gate-control-entry

  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint32, index, Index);                   ///< The index of AdminControl
  ACT_JSON_FIELD(QString, operation_name, OperationName);  ///< operation Name
  ACT_JSON_OBJECT(ActSGSParams, sgs_params,
                  SgsParams);  ///< SGSParams item (Contains parameters for the SetGateStates operation)

 public:
  /**
   * @brief Construct a new Admin Control object
   *
   */
  ActAdminControl() : index_(0) {}

  /**
   * @brief Construct a new Admin Control object
   *
   * @param index
   * @param operation_name
   * @param sgs_params
   */
  ActAdminControl(const quint32 &index, const QString &operation_name, const ActSGSParams &sgs_params)
      : index_(index), operation_name_(operation_name), sgs_params_(sgs_params) {}
};

/**
 * @brief The ActGateParameters class
 *
 * ieee802-dot1q-sched@2018-09-19.yang
 * A list that contains the per-port manageable parameters for traffic scheduling.
 *
 */
class ActGateParameters : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, gate_enabled, GateEnabled);             ///< The boolean of Gate Enabled
  ACT_JSON_FIELD(bool, config_change, ConfigChange);           ///< The boolean of Config Change
  ACT_JSON_FIELD(quint8, admin_gate_states, AdminGateStates);  ///< The states of Admin Gate

  ACT_JSON_OBJECT(ActAdminBaseTime, admin_base_time, AdminBaseTime);  ///< The Admin Base time
  // the base time at which gating cycles begin.
  ACT_JSON_OBJECT(ActAdminCycleTime, admin_cycle_time, AdminCycleTime);  ///< The Admin Cycle time
  // the administrative value of the gating cycle time for the Port.
  ACT_JSON_FIELD(quint32, admin_cycle_time_extension,
                 AdminCycleTimeExtension);  ///< The time extension of Admin Cycle
  // An unsigned integer number of nanoseconds

  ACT_JSON_FIELD(quint32, admin_control_list_length, AdminControlListLength);  ///< The length of Admin Control list v
  ACT_JSON_COLLECTION_OBJECTS(QList, ActAdminControl, admin_control_list,
                              AdminControlList);  ///< The Admin Control list v
                                                  // the administrative value of the gate control list for the Port.

 public:
  /**
   * @brief Construct a new Gate Parameters object
   *
   */
  ActGateParameters()
      : gate_enabled_(false),
        admin_gate_states_(0),
        admin_cycle_time_extension_(0),
        config_change_(true),
        admin_control_list_length_(0) {}

  ActGateParameters(const bool &gate_enabled, const bool &config_change, const quint8 &admin_gate_states,
                    const ActAdminBaseTime &admin_base_time, const ActAdminCycleTime &admin_cycle_time,
                    const quint32 &admin_cycle_time_extension, const quint32 &admin_control_list_length,
                    const QList<ActAdminControl> &admin_control_list)
      : gate_enabled_(gate_enabled),
        config_change_(config_change),
        admin_gate_states_(admin_gate_states),
        admin_base_time_(admin_base_time),
        admin_cycle_time_(admin_cycle_time),
        admin_cycle_time_extension_(admin_cycle_time_extension),
        admin_control_list_length_(admin_control_list_length),
        admin_control_list_(admin_control_list) {}
};

/**
 * @brief The ActInterfaceGateParameters class
 *
 */
class ActInterfaceGateParameters : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, interface_id, InterfaceId);                    ///< The Interface ID
  ACT_JSON_OBJECT(ActGateParameters, gate_parameters, GateParameters);  ///< The Gate Parameters

 public:
  /**
   * @brief Construct a new Port Gate Parameters object
   *
   */
  ActInterfaceGateParameters() : interface_id_(-1) {}

  /**
   * @brief Construct a new ActInterfaceGateParameters object
   *
   * @param interface_id
   */
  ActInterfaceGateParameters(const qint64 &interface_id) : interface_id_(interface_id) {}

  /**
   * @brief Construct a new Port Gate Parameters object
   *
   * @param interface_id
   * @param gate_parameters
   */
  ActInterfaceGateParameters(const qint64 &interface_id, const ActGateParameters &gate_parameters)
      : interface_id_(interface_id), gate_parameters_(gate_parameters) {}

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActInterfaceGateParameters &x) {
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
  friend bool operator==(const ActInterfaceGateParameters &x, const ActInterfaceGateParameters &y) {
    return x.interface_id_ == y.interface_id_;
  }

  /**
   * @brief The comparison operator for std:set
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator<(const ActInterfaceGateParameters &x, const ActInterfaceGateParameters &y) {
    return std::tie(x.interface_id_) < std::tie(y.interface_id_);
  }
};
