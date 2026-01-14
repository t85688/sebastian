/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_SCHEDULE_RESULT_H
#define ACT_SCHEDULE_RESULT_H

#include "act_json.hpp"
#include "act_project.hpp"
#include "act_schedule_config.hpp"
#include "act_utilities.hpp"

class ActScheduleResultFrame : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, frame_id, FrameId);
  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(qint64, ingress_interface_id, IngressInterfaceId);
  ACT_JSON_FIELD(qint64, egress_interface_id, EgressInterfaceId);
  ACT_JSON_FIELD(qint64, link_id, LinkId);
  ACT_JSON_FIELD(quint8, queue_id, QueueId);
  ACT_JSON_FIELD(qint64, stream_id, StreamId);
  ACT_JSON_FIELD(qint64, sequence_id, SeqenceId);

  ACT_JSON_FIELD(qint64, prev_frame_id, PrevFrameId);
  ACT_JSON_FIELD(qint64, prev_device_id, PrevDeviceId);
  ACT_JSON_FIELD(qint64, prev_interface_id, PrevInterfaceId);
  ACT_JSON_FIELD(qint64, prev_link_id, PrevLinkId);

  ACT_JSON_FIELD(quint8, priority_code_point, PriorityCodePoint);

  ACT_JSON_FIELD(qint64, enqueue_time, EnqueueTime);
  ACT_JSON_FIELD(qint64, dequeue_time, DequeueTime);
  ACT_JSON_FIELD(qint64, duration, Duration);
  ACT_JSON_FIELD(qint64, cost, Cost);
  ACT_JSON_COLLECTION(QList, qint64, paths, Paths);

  ACT_JSON_FIELD(quint16, vlan_id, VlanId);
  ACT_JSON_ENUM(ActFrerTypeEnum, frer_type, FrerType);

 public:
  ActScheduleResultFrame() {
    this->frame_id_ = 0;
    this->device_id_ = -1;
    this->ingress_interface_id_ = -1;
    this->egress_interface_id_ = -1;
    this->link_id_ = -1;
    this->queue_id_ = 0;
    this->stream_id_ = -1;
    this->sequence_id_ = 0;
    this->prev_frame_id_ = -1;
    this->prev_device_id_ = -1;
    this->prev_interface_id_ = -1;
    this->prev_link_id_ = -1;
    this->priority_code_point_ = 0;
    this->enqueue_time_ = 0;
    this->dequeue_time_ = 0;
    this->duration_ = 0;
    this->cost_ = 0;

    this->vlan_id_ = 0;
  }

  ActScheduleResultFrame(const qint64 &device_id, const qint64 &interface_id, const qint64 &link_id,
                         const quint8 &queue_id, const qint64 &stream_id, const qint64 &sequence_id,
                         const qint64 &frame_id, const quint8 &priority_code_point, const qint64 &enqueue_time,
                         const qint64 &duration)
      : ActScheduleResultFrame() {
    this->device_id_ = device_id;
    this->egress_interface_id_ = interface_id;
    this->link_id_ = link_id;
    this->queue_id_ = queue_id;
    this->stream_id_ = stream_id;
    this->sequence_id_ = sequence_id;
    this->frame_id_ = frame_id;

    this->priority_code_point_ = priority_code_point;

    this->enqueue_time_ = enqueue_time;
    this->dequeue_time_ = enqueue_time;
    this->duration_ = duration;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param stream
   * @return uint
   */
  friend uint qHash(const ActScheduleResultFrame &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The "==" comparison of Scheduling Stream for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActScheduleResultFrame &x, const ActScheduleResultFrame &y) {
    return x.frame_id_ == y.frame_id_;
  }

  /**
   * @brief The ">" comparison of Routing Link for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator>(const ActScheduleResultFrame &x, const ActScheduleResultFrame &y) { return x.cost_ > y.cost_; }

  /**
   * @brief The "<" comparison of Routing Link for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator<(const ActScheduleResultFrame &x, const ActScheduleResultFrame &y) {
    return x.enqueue_time_ < y.enqueue_time_;
  }
};

class ActScheduleResultSequence : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActScheduleResultFrame, schedule_result_frames, ScheduleResultFrames);

 public:
  ActScheduleResultSequence() {}

  ActScheduleResultFrame &GetScheduleResultFrame(qint64 frame_id) { return this->schedule_result_frames_[frame_id]; }

  bool IsScheduleResultFrameExist(qint64 frame_id) { return this->schedule_result_frames_.contains(frame_id); }
};

class ActScheduleResultStream : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActScheduleResultSequence, schedule_result_sequences, ScheduleResultSequences);

 public:
  ActScheduleResultStream() {}

  ActScheduleResultSequence &GetScheduleResultSequence(qint64 sequence_id) {
    return this->schedule_result_sequences_[sequence_id];
  }

  ActScheduleResultFrame &GetScheduleResultFrame(qint64 sequence_id, qint64 frame_id) {
    return this->GetScheduleResultSequence(sequence_id).GetScheduleResultFrame(frame_id);
  }

  bool IsScheduleResultSequenceExist(qint64 sequence_id) {
    return this->schedule_result_sequences_.contains(sequence_id);
  }

  bool IsScheduleResultFrameExist(qint64 sequence_id, qint64 frame_id) {
    return this->IsScheduleResultSequenceExist(sequence_id) &&
           this->GetScheduleResultSequence(sequence_id).IsScheduleResultFrameExist(frame_id);
  }
};

class ActScheduleResultQueue : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActScheduleResultStream, schedule_result_streams, ScheduleResultStreams);

 public:
  ActScheduleResultQueue() {}

  ActScheduleResultStream &GetScheduleResultStream(qint64 stream_id) {
    return this->schedule_result_streams_[stream_id];
  }

  ActScheduleResultSequence &GetScheduleResultSequence(qint64 stream_id, qint64 sequence_id) {
    return this->GetScheduleResultStream(stream_id).GetScheduleResultSequence(sequence_id);
  }

  ActScheduleResultFrame &GetScheduleResultFrame(qint64 stream_id, qint64 sequence_id, qint64 frame_id) {
    return this->GetScheduleResultSequence(stream_id, sequence_id).GetScheduleResultFrame(frame_id);
  }

  bool IsScheduleResultStreamExist(qint64 stream_id) { return this->schedule_result_streams_.contains(stream_id); }

  bool IsScheduleResultSequenceExist(qint64 stream_id, qint64 sequence_id) {
    return this->IsScheduleResultStreamExist(stream_id) &&
           this->GetScheduleResultStream(stream_id).IsScheduleResultSequenceExist(sequence_id);
  }

  bool IsScheduleResultFrameExist(qint64 stream_id, qint64 sequence_id, qint64 frame_id) {
    return this->IsScheduleResultSequenceExist(stream_id, sequence_id) &&
           this->GetScheduleResultSequence(stream_id, sequence_id).IsScheduleResultFrameExist(frame_id);
  }
};

class ActScheduleResultInterface : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, quint8, ActScheduleResultQueue, schedule_result_queues, ScheduleResultQueues);

 public:
  ActScheduleResultInterface() {}

  ActScheduleResultQueue &GetScheduleResultQueue(quint8 queue_id) { return this->schedule_result_queues_[queue_id]; }

  ActScheduleResultStream &GetScheduleResultStream(quint8 queue_id, qint64 stream_id) {
    return this->GetScheduleResultQueue(queue_id).GetScheduleResultStream(stream_id);
  }

  ActScheduleResultSequence &GetScheduleResultSequence(quint8 queue_id, qint64 stream_id, qint64 sequence_id) {
    return this->GetScheduleResultStream(queue_id, stream_id).GetScheduleResultSequence(sequence_id);
  }

  ActScheduleResultFrame &GetScheduleResultFrame(quint8 queue_id, qint64 stream_id, qint64 sequence_id,
                                                 qint64 frame_id) {
    return this->GetScheduleResultSequence(queue_id, stream_id, sequence_id).GetScheduleResultFrame(frame_id);
  }

  bool IsScheduleResultQueueExist(quint8 queue_id) { return this->schedule_result_queues_.contains(queue_id); }

  bool IsScheduleResultStreamExist(quint8 queue_id, qint64 stream_id) {
    return this->IsScheduleResultQueueExist(queue_id) &&
           this->GetScheduleResultQueue(queue_id).IsScheduleResultStreamExist(stream_id);
  }

  bool IsScheduleResultSequenceExist(quint8 queue_id, qint64 stream_id, qint64 sequence_id) {
    return this->IsScheduleResultStreamExist(queue_id, stream_id) &&
           this->GetScheduleResultStream(queue_id, stream_id).IsScheduleResultSequenceExist(sequence_id);
  }

  bool IsScheduleResultFrameExist(quint8 queue_id, qint64 stream_id, qint64 sequence_id, qint64 frame_id) {
    return this->IsScheduleResultSequenceExist(queue_id, stream_id, sequence_id) &&
           this->GetScheduleResultSequence(queue_id, stream_id, sequence_id).IsScheduleResultFrameExist(frame_id);
  }
};

class ActScheduleResultDevice : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActScheduleResultInterface, schedule_result_interfaces,
                           ScheduleResultInterfaces);

 public:
  ActScheduleResultDevice() {}

  ActScheduleResultInterface &GetScheduleResultInterface(qint64 interface_id) {
    return this->schedule_result_interfaces_[interface_id];
  }

  ActScheduleResultQueue &GetScheduleResultQueue(qint64 interface_id, quint8 queue_id) {
    return this->GetScheduleResultInterface(interface_id).GetScheduleResultQueue(queue_id);
  }

  ActScheduleResultStream &GetScheduleResultStream(qint64 interface_id, quint8 queue_id, qint64 stream_id) {
    return this->GetScheduleResultQueue(interface_id, queue_id).GetScheduleResultStream(stream_id);
  }

  ActScheduleResultSequence &GetScheduleResultSequence(qint64 interface_id, quint8 queue_id, qint64 stream_id,
                                                       qint64 sequence_id) {
    return this->GetScheduleResultStream(interface_id, queue_id, stream_id).GetScheduleResultSequence(sequence_id);
  }

  ActScheduleResultFrame &GetScheduleResultFrame(qint64 interface_id, quint8 queue_id, qint64 stream_id,
                                                 qint64 sequence_id, qint64 frame_id) {
    return this->GetScheduleResultSequence(interface_id, queue_id, stream_id, sequence_id)
        .GetScheduleResultFrame(frame_id);
  }

  bool IsScheduleResultInterfaceExist(qint64 interface_id) {
    return this->schedule_result_interfaces_.contains(interface_id);
  }

  bool IsScheduleResultQueueExist(qint64 interface_id, quint8 queue_id) {
    return this->IsScheduleResultInterfaceExist(interface_id) &&
           this->GetScheduleResultInterface(interface_id).IsScheduleResultQueueExist(queue_id);
  }

  bool IsScheduleResultStreamExist(qint64 interface_id, quint8 queue_id, qint64 stream_id) {
    return this->IsScheduleResultQueueExist(interface_id, queue_id) &&
           this->GetScheduleResultQueue(interface_id, queue_id).IsScheduleResultStreamExist(stream_id);
  }

  bool IsScheduleResultSequenceExist(qint64 interface_id, quint8 queue_id, qint64 stream_id, qint64 sequence_id) {
    return this->IsScheduleResultStreamExist(interface_id, queue_id, stream_id) &&
           this->GetScheduleResultStream(interface_id, queue_id, stream_id).IsScheduleResultSequenceExist(sequence_id);
  }

  bool IsScheduleResultFrameExist(qint64 interface_id, quint8 queue_id, qint64 stream_id, qint64 sequence_id,
                                  qint64 frame_id) {
    return this->IsScheduleResultSequenceExist(interface_id, queue_id, stream_id, sequence_id) &&
           this->GetScheduleResultSequence(interface_id, queue_id, stream_id, sequence_id)
               .IsScheduleResultFrameExist(frame_id);
  }
};

class ActScheduleResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActScheduleResultDevice, schedule_result_devices, ScheduleResultDevices);

 public:
  ActScheduleResult() {}

  void ConsiderQueueIsolation(ActScheduleResultFrame &schedule_result_frame, qint64 &offset);
  void ConsiderCycleTimeReservation(ActScheduleResultFrame &schedule_result_frame, qint64 &cycle_time,
                                    qint64 &reserve_time);
  void ConsiderLinkIsolation(ActScheduleResultFrame &schedule_result_frame, qint64 &cycle_time, qint64 &reserve_time);

  ActScheduleResultDevice &GetScheduleResultDevice(qint64 device_id) {
    return this->schedule_result_devices_[device_id];
  }

  ActScheduleResultInterface &GetScheduleResultInterface(qint64 device_id, qint64 interface_id) {
    return this->GetScheduleResultDevice(device_id).GetScheduleResultInterface(interface_id);
  }

  ActScheduleResultQueue &GetScheduleResultQueue(qint64 device_id, qint64 interface_id, quint8 queue_id) {
    return this->GetScheduleResultInterface(device_id, interface_id).GetScheduleResultQueue(queue_id);
  }

  ActScheduleResultStream &GetScheduleResultStream(qint64 device_id, qint64 interface_id, quint8 queue_id,
                                                   qint64 stream_id) {
    return this->GetScheduleResultQueue(device_id, interface_id, queue_id).GetScheduleResultStream(stream_id);
  }

  ActScheduleResultSequence &GetScheduleResultSequence(qint64 device_id, qint64 interface_id, quint8 queue_id,
                                                       qint64 stream_id, qint64 sequence_id) {
    return this->GetScheduleResultStream(device_id, interface_id, queue_id, stream_id)
        .GetScheduleResultSequence(sequence_id);
  }

  ActScheduleResultFrame &GetScheduleResultFrame(qint64 device_id, qint64 interface_id, quint8 queue_id,
                                                 qint64 stream_id, qint64 sequence_id, qint64 frame_id) {
    return this->GetScheduleResultSequence(device_id, interface_id, queue_id, stream_id, sequence_id)
        .GetScheduleResultFrame(frame_id);
  }

  bool IsScheduleResultDeviceExist(qint64 device_id) { return this->schedule_result_devices_.contains(device_id); }

  bool IsScheduleResultInterfaceExist(qint64 device_id, qint64 interface_id) {
    return this->IsScheduleResultDeviceExist(device_id) &&
           this->GetScheduleResultDevice(device_id).IsScheduleResultInterfaceExist(interface_id);
  }

  bool IsScheduleResultQueueExist(qint64 device_id, qint64 interface_id, quint8 queue_id) {
    return this->IsScheduleResultInterfaceExist(device_id, interface_id) &&
           this->GetScheduleResultInterface(device_id, interface_id).IsScheduleResultQueueExist(queue_id);
  }

  bool IsScheduleResultStreamExist(qint64 device_id, qint64 interface_id, quint8 queue_id, qint64 stream_id) {
    return this->IsScheduleResultQueueExist(device_id, interface_id, queue_id) &&
           this->GetScheduleResultQueue(device_id, interface_id, queue_id).IsScheduleResultStreamExist(stream_id);
  }

  bool IsScheduleResultSequenceExist(qint64 device_id, qint64 interface_id, quint8 queue_id, qint64 stream_id,
                                     qint64 sequence_id) {
    return this->IsScheduleResultStreamExist(device_id, interface_id, queue_id, stream_id) &&
           this->GetScheduleResultStream(device_id, interface_id, queue_id, stream_id)
               .IsScheduleResultSequenceExist(sequence_id);
  }

  bool IsScheduleResultFrameExist(qint64 device_id, qint64 interface_id, quint8 queue_id, qint64 stream_id,
                                  qint64 sequence_id, qint64 frame_id) {
    return this->IsScheduleResultSequenceExist(device_id, interface_id, queue_id, stream_id, sequence_id) &&
           this->GetScheduleResultSequence(device_id, interface_id, queue_id, stream_id, sequence_id)
               .IsScheduleResultFrameExist(frame_id);
  }

  void SetScheduleResult(ActScheduleResultFrame &schedule_result_frame, ActScheduleConfig &schedule_config) {
    ActScheduleConfigStream &schedule_config_stream =
        schedule_config.GetScheduleConfigStream(schedule_result_frame.GetStreamId());

    ActScheduleResultSequence &schedule_result_sequence = this->GetScheduleResultSequence(
        schedule_result_frame.GetDeviceId(), schedule_result_frame.GetEgressInterfaceId(),
        schedule_result_frame.GetQueueId(), schedule_result_frame.GetStreamId(), schedule_result_frame.GetSeqenceId());

    schedule_result_sequence.GetScheduleResultFrames().insert(schedule_result_frame.GetFrameId(),
                                                              schedule_result_frame);
  }

  quint8 GetGateStatesValue(const QSet<quint8> &queue_set) {
    quint8 gate_states_value = 0;
    for (const quint8 &queue : queue_set) {
      gate_states_value += 1 << queue;
    }
    return gate_states_value;
  }

  quint8 GetGateStatesValue(const quint8 &queue_id) { return 1 << queue_id; }

  void GenerateRoutingResult(ActComputedResult &computed_result, ActScheduleConfig &schedule_config);
  void GenerateDeviceResult(ActComputedResult &computed_result, ActScheduleConfig &schedule_config);
  void GenerateStreamResult(ActComputedResult &computed_result, ActScheduleConfig &schedule_config);
  void GenerateGCLResult(ActComputedResult &computed_result, ActScheduleConfig &schedule_config);
};

#endif