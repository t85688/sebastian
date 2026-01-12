/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_status.hpp"
#include "gcl/act_admin_base_time.hpp"
#include "gcl/act_admin_cycle_time.hpp"
#include "stream/act_stream.hpp"

/**
 * @brief The Stream traffic type enum class
 *
 */
enum class ActTimeSlotTrafficTypeEnum {
  kNA = 0,
  kBestEffort = 1,
  kCyclic = 2,
  kTimeSync = 3,
};

/**
 * @brief The QMap for Stream traffic type enum mapping
 *
 */
static const QMap<QString, ActTimeSlotTrafficTypeEnum> kActTimeSlotTrafficTypeEnumMap = {
    {"NA", ActTimeSlotTrafficTypeEnum::kNA},
    {"BestEffort", ActTimeSlotTrafficTypeEnum::kBestEffort},
    {"Cyclic", ActTimeSlotTrafficTypeEnum::kCyclic},
    {"TimeSync", ActTimeSlotTrafficTypeEnum::kTimeSync}};

/**
 * @brief The ACT Time Slot Setting class
 *
 */
class ActTimeSlotSetting : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, active, Active);                                  ///< Specifies the time slot is active
  ACT_JSON_FIELD(quint8, index, Index);                                  ///< The index of the time slot
  ACT_JSON_FIELD(quint32, period, Period);                               ///< The period of the time slot in nanosecond
  ACT_JSON_ENUM(ActTimeSlotTrafficTypeEnum, traffic_type, TrafficType);  ///< The traffic type enum of the time slot

 public:
  ActTimeSlotSetting() {
    active_ = false;
    period_ = 0;
    traffic_type_ = ActTimeSlotTrafficTypeEnum::kNA;
  }

  ActTimeSlotSetting(quint8 index) : index_(index) {
    active_ = false;
    period_ = 0;
    traffic_type_ = ActTimeSlotTrafficTypeEnum::kNA;
  }

  ActTimeSlotSetting(quint8 index, quint32 period, ActTimeSlotTrafficTypeEnum traffic_type)
      : index_(index), period_(period), traffic_type_(traffic_type) {
    active_ = true;
  }
};

/**
 * @brief The ACT Cycle Setting class
 *
 */
class ActCycleSetting : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_OBJECT(ActAdminBaseTime, admin_base_time,
                  AdminBaseTime);  ///< The Admin Base time the base time at which gating cycles begin.
  ACT_JSON_OBJECT(
      ActAdminCycleTime, admin_cycle_time,
      AdminCycleTime);  ///< The Admin Cycle time the administrative value of the gating cycle time for the Port.
  ACT_JSON_COLLECTION_OBJECTS(QList, ActTimeSlotSetting, time_slots, TimeSlots);  ///< The list of time slot setting

 public:
  QList<QString> key_order_;

  ActCycleSetting() {
    this->time_slots_.append(ActTimeSlotSetting(0, 480000, ActTimeSlotTrafficTypeEnum::kBestEffort));
    this->time_slots_.append(ActTimeSlotSetting(1, 500000, ActTimeSlotTrafficTypeEnum::kCyclic));
    this->time_slots_.append(ActTimeSlotSetting(2, 20000, ActTimeSlotTrafficTypeEnum::kTimeSync));
    this->time_slots_.append(ActTimeSlotSetting(3));
    this->time_slots_.append(ActTimeSlotSetting(4));
    this->time_slots_.append(ActTimeSlotSetting(5));
    this->time_slots_.append(ActTimeSlotSetting(6));
    this->time_slots_.append(ActTimeSlotSetting(7));
    this->key_order_.append(
        QList<QString>({QString("AdminBaseTime"), QString("AdminCycleTime"), QString("TimeSlots")}));
  }
};
