/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_SCHEDULE_STREAM_H
#define ACT_SCHEDULE_STREAM_H

#include <queue>

#include "act_json.hpp"
#include "act_schedule_config.hpp"
#include "act_schedule_feasibility.hpp"
#include "act_schedule_result.hpp"
#include "act_utilities.hpp"

class ActScheduleStream : public ActScheduleFeasibility {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, stream_id, StreamId);
  ACT_JSON_FIELD(quint8, queue_id, QueueId);
  ACT_JSON_FIELD(quint8, priority_code_point, PriorityCodePoint);
  ACT_JSON_FIELD(qint64, transmit_offset, TransmitOffset);
  ACT_JSON_FIELD(qint64, sequence_id, SequenceId);
  ACT_JSON_FIELD(qint64, interval, Interval);     // ns
  ACT_JSON_FIELD(qint64, cycle_time, CycleTime);  // ns

 public:
  ActScheduleStream()
      : stream_id_(-1),
        queue_id_(0),
        priority_code_point_(0),
        transmit_offset_(0),
        sequence_id_(0),
        interval_(1000000),
        cycle_time_(1000000) {}

  void SetScheduleStream(const qint64 &stream_id, const quint8 &queue_id, const quint8 &priority_code_point,
                         const qint64 &transmit_offset, const qint64 &interval, const qint64 &cycle_time) {
    this->stream_id_ = stream_id;
    this->queue_id_ = queue_id;
    this->priority_code_point_ = priority_code_point;
    this->transmit_offset_ = transmit_offset;
    this->sequence_id_ = 0;
    this->interval_ = interval;
    this->cycle_time_ = cycle_time;
  }

  qint64 GenerateTransmitOffset() { return this->transmit_offset_ + this->interval_ * this->sequence_id_; }

  ACT_STATUS ComputeScheduleStream(ActScheduleConfig &schedule_config, ActScheduleResult &schedule_result);

  ACT_STATUS ComputeScheduleStreamRouting(ActScheduleConfig &schedule_config, ActScheduleResult &schedule_result);

  ACT_STATUS GenerateScheduleStreamRouting(ActScheduleConfig &schedule_config, ActScheduleResult &schedule_result,
                                           ActScheduleResult &stream_schedule_result);
};

#endif