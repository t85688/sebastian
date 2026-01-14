#include "act_schedule_stream.hpp"

ACT_STATUS ActScheduleStream::ComputeScheduleStream(ActScheduleConfig &schedule_config,
                                                    ActScheduleResult &schedule_result) {
  ACT_STATUS_INIT();

  ActScheduleConfigStream &schedule_config_stream = schedule_config.GetScheduleConfigStream(this->stream_id_);

  ActScheduleResult schedule_result_ = schedule_result;
  while (this->sequence_id_ < this->cycle_time_ / this->interval_) {
    act_status = ActScheduleFeasibility::CheckTransmitOffset(schedule_config_stream, this->transmit_offset_,
                                                             this->interval_, this->sequence_id_);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    act_status = this->ComputeScheduleStreamRouting(schedule_config, schedule_result_);
    if (IsActStatusFeasibilityCheckFailed(act_status)) {
      return act_status;
    } else if (!IsActStatusSuccess(act_status)) {
      this->sequence_id_ = 0;
      schedule_result_ = schedule_result;
    } else {
      this->sequence_id_++;
    }
  }

  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  act_status = GenerateScheduleStreamRouting(schedule_config, schedule_result, schedule_result_);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActScheduleStream::ComputeScheduleStreamRouting(ActScheduleConfig &schedule_config,
                                                           ActScheduleResult &schedule_result) {
  ACT_STATUS_INIT();
  std::shared_ptr<ActStatusBase> error_msg = ACT_STATUS_SUCCESS;

  ActScheduleConfigStream &schedule_config_stream = schedule_config.GetScheduleConfigStream(this->stream_id_);

  ActScheduleConfigInterface &talker = schedule_config_stream.GetTalker();
  QSet<ActScheduleConfigInterface> &listeners = schedule_config_stream.GetListeners();

  std::priority_queue<ActScheduleResultFrame, QVector<ActScheduleResultFrame>, std::greater<ActScheduleResultFrame>>
      queue;

  qint64 frame_id_ = 0;
  qint64 duration = schedule_config.ComputeStreamDuration(this->stream_id_, talker.GetLinkId());

  ActScheduleResultFrame schedule_result_frame(talker.GetDeviceId(), talker.GetInterfaceId(), talker.GetLinkId(),
                                               this->queue_id_, this->stream_id_, this->sequence_id_, frame_id_,
                                               this->priority_code_point_, this->transmit_offset_, duration);
  queue.push(schedule_result_frame);
  bool is_listener_arrived = false;
  while (!queue.empty()) {
    schedule_result_frame = queue.top();
    queue.pop();

    ActScheduleConfigDevice device = schedule_config.GetScheduleConfigDevice(schedule_result_frame.GetDeviceId());

    if (schedule_config.IsListenerArrived(
            this->stream_id_, schedule_result_frame.GetDeviceId(), schedule_result_frame.GetIngressInterfaceId(),
            schedule_result_frame.GetEgressInterfaceId(), schedule_result_frame.GetLinkId())) {
      act_status = ActScheduleFeasibility::CheckReceiveOffset(schedule_result_frame, schedule_config);
      if (!IsActStatusSuccess(act_status)) {
        error_msg = act_status;
        continue;
      }

      schedule_result.SetScheduleResult(schedule_result_frame, schedule_config);
      is_listener_arrived = true;
      continue;
    }

    act_status = ActScheduleFeasibility::CheckRoutingPathDuplicated(schedule_result_frame, schedule_config);
    if (!IsActStatusSuccess(act_status)) {
      continue;
    }

    act_status = ActScheduleFeasibility::CheckDeviceFeature(schedule_result_frame, schedule_config);
    if (!IsActStatusSuccess(act_status)) {
      continue;
    }

    act_status =
        ActScheduleFeasibility::CheckGateControlListLength(schedule_result_frame, schedule_result, schedule_config);
    if (!IsActStatusSuccess(act_status)) {
      error_msg = act_status;
      continue;
    }

    schedule_result.ConsiderLinkIsolation(schedule_result_frame, this->cycle_time_, schedule_config.GetReserveTime());

    qint64 offset = 0;
    schedule_result.ConsiderQueueIsolation(schedule_result_frame, offset);
    if (offset) {
      this->transmit_offset_ += offset;
      return std::make_shared<ActStatusSchedulingFailed>(
          QString("Consider queue isolation, modify transmit offset from %1 to %2")
              .arg(this->transmit_offset_ - offset)
              .arg(this->transmit_offset_));
    }

    act_status = ActScheduleFeasibility::CheckQueueSize(schedule_result_frame, schedule_result, schedule_config);
    if (!IsActStatusSuccess(act_status)) {
      error_msg = act_status;
      continue;
    }

    schedule_result.SetScheduleResult(schedule_result_frame, schedule_config);

    qint64 frame_id = schedule_result_frame.GetFrameId();
    qint64 device_id = schedule_result_frame.GetDeviceId();
    qint64 egress_interface_id = schedule_result_frame.GetEgressInterfaceId();
    qint64 link_id = schedule_result_frame.GetLinkId();
    qint64 cost = schedule_result_frame.GetCost();
    qint64 enqueue_time = schedule_result_frame.GetEnqueueTime();
    qint64 dequeue_time = schedule_result_frame.GetDequeueTime();
    qint64 queue_delay = dequeue_time - enqueue_time;
    qint64 total_delay = schedule_config.ComputeTotalDelay(device_id, egress_interface_id, this->stream_id_);

    qint64 next_device_id =
        schedule_config.GetScheduleConfigInterface(device_id, egress_interface_id).GetConnectDeviceId();
    qint64 next_ingress_interface_id =
        schedule_config.GetScheduleConfigInterface(device_id, egress_interface_id).GetConnectInterfaceId();

    schedule_result_frame.SetDeviceId(next_device_id);
    schedule_result_frame.SetIngressInterfaceId(next_ingress_interface_id);
    schedule_result_frame.SetPrevFrameId(frame_id);
    schedule_result_frame.SetPrevDeviceId(device_id);
    schedule_result_frame.SetPrevInterfaceId(egress_interface_id);
    schedule_result_frame.SetPrevLinkId(link_id);
    schedule_result_frame.SetEnqueueTime(dequeue_time + total_delay);
    schedule_result_frame.SetDequeueTime(dequeue_time + total_delay);
    schedule_result_frame.SetCost(cost + queue_delay + total_delay);
    schedule_result_frame.GetPaths().append(device_id);

    ActScheduleConfigDevice &next_schedule_config_device = schedule_config.GetScheduleConfigDevice(next_device_id);
    for (auto &next_schedule_config_interface : next_schedule_config_device.GetScheduleConfigInterfaces().values()) {
      schedule_result_frame.SetFrameId(++frame_id_);
      schedule_result_frame.SetEgressInterfaceId(next_schedule_config_interface.GetInterfaceId());
      schedule_result_frame.SetLinkId(next_schedule_config_interface.GetLinkId());
      schedule_result_frame.SetDuration(
          schedule_config.ComputeStreamDuration(this->stream_id_, next_schedule_config_interface.GetLinkId()));
      queue.push(schedule_result_frame);
    }
  }

  return is_listener_arrived ? ACT_STATUS_SUCCESS : error_msg;
}

ACT_STATUS ActScheduleStream::GenerateScheduleStreamRouting(ActScheduleConfig &schedule_config,
                                                            ActScheduleResult &schedule_result,
                                                            ActScheduleResult &stream_schedule_result) {
  ACT_STATUS_INIT();

  ActScheduleConfigStream &schedule_config_stream = schedule_config.GetScheduleConfigStream(this->stream_id_);

  ActScheduleConfigInterface &talker = schedule_config_stream.GetTalker();
  QSet<ActScheduleConfigInterface> &listeners = schedule_config_stream.GetListeners();

  quint16 split_vlan_id = 0;
  if (schedule_config_stream.GetCbEnable()) {
    act_status = schedule_config.GetScheduleConfigVLAN().GenerateSystemAssignVLAN(
        split_vlan_id, schedule_config_stream.GetDestinationMac());
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  QMap<qint64, QMap<qint64, quint16>> vlan_map;  // device_id, egress_interface, vlan_id
  for (ActScheduleConfigInterface listener : listeners) {
    ActScheduleResultStream &schedule_result_stream = stream_schedule_result.GetScheduleResultStream(
        listener.GetDeviceId(), listener.GetInterfaceId(), this->queue_id_, this->stream_id_);

    for (qint64 &sequence_id : schedule_result_stream.GetScheduleResultSequences().keys()) {
      ActScheduleResultSequence &schedule_result_sequence =
          schedule_result_stream.GetScheduleResultSequence(sequence_id);

      quint16 vlan_id = schedule_config_stream.GetVlanId();
      QMap<qint64, QSet<ActScheduleResultFrame>> frame_map;
      QMap<qint64, QSet<qint64>> tx_map;  // device_id, egress_interface
      QMap<qint64, QSet<qint64>> rx_map;  // device_id, ingress_interface
      for (qint64 &frame_id : schedule_result_sequence.GetScheduleResultFrames().keys()) {
        ActScheduleResultFrame schedule_result_frame = schedule_result_sequence.GetScheduleResultFrame(frame_id);
        if (schedule_result_frame.GetIngressInterfaceId() != listener.GetInterfaceId()) {
          continue;
        }

        if (!vlan_map[schedule_result_frame.GetDeviceId()].contains(schedule_result_frame.GetEgressInterfaceId())) {
          vlan_map[schedule_result_frame.GetDeviceId()].insert(schedule_result_frame.GetEgressInterfaceId(), vlan_id);
        } else {
          vlan_id = (vlan_map[schedule_result_frame.GetDeviceId()][schedule_result_frame.GetEgressInterfaceId()] !=
                     schedule_config_stream.GetVlanId())
                        ? vlan_map[schedule_result_frame.GetDeviceId()][schedule_result_frame.GetEgressInterfaceId()]
                        : vlan_id;
        }

        frame_map[schedule_result_frame.GetDeviceId()].insert(schedule_result_frame);
        tx_map[schedule_result_frame.GetDeviceId()].insert(schedule_result_frame.GetEgressInterfaceId());
        rx_map[schedule_result_frame.GetDeviceId()].insert(schedule_result_frame.GetIngressInterfaceId());

        while (schedule_result_frame.GetDeviceId() != talker.GetDeviceId()) {
          schedule_result_frame = stream_schedule_result.GetScheduleResultFrame(
              schedule_result_frame.GetPrevDeviceId(), schedule_result_frame.GetPrevInterfaceId(),
              schedule_result_frame.GetQueueId(), schedule_result_frame.GetStreamId(),
              schedule_result_frame.GetSeqenceId(), schedule_result_frame.GetPrevFrameId());

          if (!vlan_map[schedule_result_frame.GetDeviceId()].contains(schedule_result_frame.GetEgressInterfaceId())) {
            vlan_map[schedule_result_frame.GetDeviceId()].insert(schedule_result_frame.GetEgressInterfaceId(), vlan_id);
          } else {
            vlan_id = (vlan_map[schedule_result_frame.GetDeviceId()][schedule_result_frame.GetEgressInterfaceId()] !=
                       schedule_config_stream.GetVlanId())
                          ? vlan_map[schedule_result_frame.GetDeviceId()][schedule_result_frame.GetEgressInterfaceId()]
                          : vlan_id;
          }

          frame_map[schedule_result_frame.GetDeviceId()].insert(schedule_result_frame);
          tx_map[schedule_result_frame.GetDeviceId()].insert(schedule_result_frame.GetEgressInterfaceId());
          rx_map[schedule_result_frame.GetDeviceId()].insert(schedule_result_frame.GetIngressInterfaceId());
        }

        if (!schedule_config_stream.GetCbEnable()) {
          break;
        }

        vlan_id = (vlan_id != schedule_config_stream.GetVlanId()) ? schedule_config_stream.GetVlanId() : split_vlan_id;
      }

      for (qint64 device_id : tx_map.keys()) {
        ActScheduleConfigDevice &schedule_config_device = schedule_config.GetScheduleConfigDevice(device_id);
        ActFrerTypeEnum frer_type = (!schedule_config_device.GetCbEnable()) ? ActFrerTypeEnum::kUndefined
                                    : (tx_map[device_id].count() > 1)       ? ActFrerTypeEnum::kSplit
                                    : (rx_map[device_id].count() > 1)       ? ActFrerTypeEnum::kMerge
                                                                            : ActFrerTypeEnum::kForward;
        for (ActScheduleResultFrame schedule_result_frame : frame_map[device_id]) {
          quint16 vlan_id = vlan_map[device_id][schedule_result_frame.GetEgressInterfaceId()];
          schedule_result_frame.SetVlanId(vlan_id);
          schedule_result_frame.SetFrerType(frer_type);
          schedule_result.SetScheduleResult(schedule_result_frame, schedule_config);
        }
      }
    }

    if (!schedule_result.IsScheduleResultStreamExist(listener.GetDeviceId(), listener.GetInterfaceId(), this->queue_id_,
                                                     this->stream_id_)) {
      QString error_msg =
          QString("Stream %1 - Cannot find the routing path").arg(schedule_config_stream.GetStreamName());
      qDebug() << error_msg.toStdString().c_str();
      return std::make_shared<ActStatusSchedulingFailed>(error_msg);
    }
  }

  return ACT_STATUS_SUCCESS;
}
