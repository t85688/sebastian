#include "act_schedule_feasibility.hpp"

ACT_STATUS ActScheduleFeasibility::CheckTransmitOffset(ActScheduleConfigStream &schedule_config_stream,
                                                       qint64 &transmit_offset, qint64 &interval, qint64 &sequence_id) {
  qint64 &jitter = schedule_config_stream.GetJitter();
  qint64 &earliest_transmit_offset = schedule_config_stream.GetEarliestTransmitOffset();
  qint64 &latest_transmit_offset = schedule_config_stream.GetLatestTransmitOffset();
  qint64 transmit_offset_ = transmit_offset + interval * sequence_id;

  if (!((transmit_offset_ >= (earliest_transmit_offset + jitter)) &&
        (transmit_offset_ <= (latest_transmit_offset - jitter)))) {
    if (schedule_config_stream.GetKeepPreviousResult()) {
      return std::make_shared<ActStatusFeasibilityCheckFailed>(QString("Disable keep previous result and recompute"));
    } else {
      return std::make_shared<ActStatusFeasibilityCheckFailed>(
          QString("%7 - Transmit offset unavailable (EarliestTransmitOffset(%1 ns) + Jitter(%2 ns) <= "
                  "TransmitOffset(%4 ns) + "
                  "Interval(%5 ns) * %6 <= LatestTransmitOffset(%3 ns) - Jitter(%2 ns))")
              .arg(earliest_transmit_offset)
              .arg(jitter)
              .arg(latest_transmit_offset)
              .arg(transmit_offset)
              .arg(interval)
              .arg(sequence_id)
              .arg(schedule_config_stream.GetStreamName()));
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActScheduleFeasibility::CheckReceiveOffset(ActScheduleResultFrame &schedule_result_frame,
                                                      ActScheduleConfig &schedule_config) {
  ActScheduleConfigStream &schedule_config_stream =
      schedule_config.GetScheduleConfigStream(schedule_result_frame.GetStreamId());

  if (schedule_result_frame.GetCost() < schedule_config_stream.GetMinReceiveOffset() ||
      schedule_result_frame.GetCost() > schedule_config_stream.GetMaxReceiveOffset()) {
    if (schedule_config_stream.GetQosType() == ActTrafficQosTypeEnum::kBandwidth) {
      QString error_msg = QString("%1 - Stream interval need to modify larger than %2 us")
                              .arg(schedule_config_stream.GetStreamName())
                              .arg(std::ceil(static_cast<qreal>(schedule_result_frame.GetCost()) / 1000));
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActStatusFeasibilityCheckFailed>(error_msg);
    } else {
      QString error_msg = QString("%1 - Receive offset %2 us is out of range %3 ~ %4 us")
                              .arg(schedule_config_stream.GetStreamName())
                              .arg(static_cast<qreal>(schedule_result_frame.GetCost()) / 1000)
                              .arg(static_cast<qreal>(schedule_config_stream.GetMinReceiveOffset()) / 1000)
                              .arg(static_cast<qreal>(schedule_config_stream.GetMaxReceiveOffset()) / 1000);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActStatusFeasibilityCheckFailed>(error_msg);
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActScheduleFeasibility::CheckRoutingPathDuplicated(ActScheduleResultFrame &schedule_result_frame,
                                                              ActScheduleConfig &schedule_config) {
  ActScheduleConfigInterface &schedule_config_interface = schedule_config.GetScheduleConfigInterface(
      schedule_result_frame.GetDeviceId(), schedule_result_frame.GetEgressInterfaceId());
  qint64 next_device_id = schedule_config_interface.GetConnectDeviceId();

  if (schedule_result_frame.GetPaths().contains(next_device_id)) {
    return std::make_shared<ActStatusFeasibilityCheckFailed>("CheckRoutingPathDuplicated");
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActScheduleFeasibility::CheckDeviceFeature(ActScheduleResultFrame &schedule_result_frame,
                                                      ActScheduleConfig &schedule_config) {
  ActScheduleConfigStream &schedule_config_stream =
      schedule_config.GetScheduleConfigStream(schedule_result_frame.GetStreamId());

  ActScheduleConfigDevice &schedule_config_device =
      schedule_config.GetScheduleConfigDevice(schedule_result_frame.GetDeviceId());

  QSet<ActScheduleConfigInterface> &listeners = schedule_config_stream.GetListeners();

  if (!schedule_config_device.GetEndStation()) {
    if (schedule_config_stream.GetTsnEnable() && !schedule_config_device.GetTsnEnable()) {
      return std::make_shared<ActStatusFeasibilityCheckFailed>(QString("%1 - Check device %2 TSN not support")
                                                                   .arg(schedule_config_stream.GetStreamName())
                                                                   .arg(schedule_config_device.GetIpAddress()));
    } else if (schedule_config_stream.GetCbEnable() && !schedule_config_device.GetCbEnable()) {
      return std::make_shared<ActStatusFeasibilityCheckFailed>(QString("%1 - Check device %2 CB not support")
                                                                   .arg(schedule_config_stream.GetStreamName())
                                                                   .arg(schedule_config_device.GetIpAddress()));
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActScheduleFeasibility::CheckGateControlListLength(ActScheduleResultFrame &schedule_result_frame,
                                                              ActScheduleResult &schedule_result,
                                                              ActScheduleConfig &schedule_config) {
  ActScheduleConfigStream &schedule_config_stream =
      schedule_config.GetScheduleConfigStream(schedule_result_frame.GetStreamId());

  ActScheduleConfigDevice &schedule_config_device =
      schedule_config.GetScheduleConfigDevice(schedule_result_frame.GetDeviceId());

  if (!schedule_config_stream.GetTsnEnable() || !schedule_config_device.GetTsnEnable()) {
    return ACT_STATUS_SUCCESS;
  }

  ActScheduleResultQueue &schedule_result_queue = schedule_result.GetScheduleResultQueue(
      schedule_result_frame.GetDeviceId(), schedule_result_frame.GetEgressInterfaceId(),
      schedule_result_frame.GetQueueId());

  quint16 gate_control_list_length = 0;
  for (auto &schedule_result_stream : schedule_result_queue.GetScheduleResultStreams().values()) {
    for (auto &schedule_result_sequence : schedule_result_stream.GetScheduleResultSequences().values()) {
      gate_control_list_length += schedule_result_sequence.GetScheduleResultFrames().size();
    }
  }

  gate_control_list_length = gate_control_list_length * 2 + 3;
  if (gate_control_list_length > schedule_config_device.GetGateControlListLength()) {
    return std::make_shared<ActStatusFeasibilityCheckFailed>(
        QString("%1 - Check gate control list length %5 too large on device(%2) port(%3) queue(%4).")
            .arg(schedule_config_stream.GetStreamName())
            .arg(schedule_config_device.GetIpAddress())
            .arg(schedule_result_frame.GetEgressInterfaceId())
            .arg(schedule_result_frame.GetQueueId())
            .arg(gate_control_list_length));
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActScheduleFeasibility::CheckQueueSize(ActScheduleResultFrame &schedule_result_frame,
                                                  ActScheduleResult &schedule_result,
                                                  ActScheduleConfig &schedule_config) {
  ActScheduleConfigStream &schedule_config_stream =
      schedule_config.GetScheduleConfigStream(schedule_result_frame.GetStreamId());

  ActScheduleConfigDevice &schedule_config_device =
      schedule_config.GetScheduleConfigDevice(schedule_result_frame.GetDeviceId());

  ActScheduleConfigLink &schedule_config_link =
      schedule_config.GetScheduleConfigLink(schedule_result_frame.GetLinkId());

  if (!schedule_config_stream.GetTsnEnable() || !schedule_config_device.GetTsnEnable()) {
    return ACT_STATUS_SUCCESS;
  }

  ActScheduleResultStream &schedule_result_stream = schedule_result.GetScheduleResultStream(
      schedule_result_frame.GetDeviceId(), schedule_result_frame.GetEgressInterfaceId(),
      schedule_result_frame.GetQueueId(), schedule_result_frame.GetStreamId());

  qint64 queue_size = 0;
  for (auto &schedule_result_sequence_ : schedule_result_stream.GetScheduleResultSequences().values()) {
    for (auto &schedule_result_frame_ : schedule_result_sequence_.GetScheduleResultFrames().values()) {
      if (schedule_result_frame.GetEnqueueTime() >=
          schedule_result_frame_.GetDequeueTime() + schedule_result_frame_.GetDuration()) {
        continue;
      } else if (schedule_result_frame.GetDequeueTime() + schedule_result_frame.GetDuration() <=
                 schedule_result_frame_.GetEnqueueTime()) {
        continue;
      }

      if (schedule_result_frame_.GetDequeueTime() <
          schedule_result_frame_.GetEnqueueTime() + schedule_result_frame_.GetDuration()) {
        queue_size += qCeil(
            static_cast<qreal>(schedule_config_stream.GetPayload()) *
            static_cast<qreal>(schedule_result_frame_.GetDequeueTime() - schedule_result_frame_.GetEnqueueTime()) /
            static_cast<qreal>(schedule_result_frame_.GetDuration()));
      } else {
        queue_size += schedule_config_stream.GetPayload();
      }
    }
  }

  if (queue_size > schedule_config_device.GetPerQueueSize() * 8) {
    return std::make_shared<ActStatusFeasibilityCheckFailed>(
        QString("%1 - Queue size %5 bytes is out of range %6 bytes on device(%2) port(%3) queue(%4)")
            .arg(schedule_config_stream.GetStreamName())
            .arg(schedule_config_device.GetIpAddress())
            .arg(schedule_result_frame.GetEgressInterfaceId())
            .arg(schedule_result_frame.GetQueueId())
            .arg(queue_size / 8)
            .arg(schedule_config_device.GetPerQueueSize()));
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActScheduleFeasibility::CheckRoutingResultFeasibility(ActRoutingResult &routing_result,
                                                                 ActProject &act_project) {
  ACT_STATUS_INIT();

  for (ActRoutingPath &routing_path : routing_result.GetRoutingPaths()) {
    if (routing_result.GetCB()) {
      if (routing_path.GetRedundantPaths().size() < 2) {
        act_status = std::make_shared<ActStatusFeasibilityCheckFailed>(
            QString("%1 - CB stream needs redundant path. (1)Check whether the topology has a redundant "
                    "path. (2)Check whether the received offset range in the application settings is valid.")
                .arg(routing_result.GetStreamName()));
        return act_status;
      }
    } else {
      if (routing_path.GetRedundantPaths().size() != 1) {
        act_status = std::make_shared<ActStatusFeasibilityCheckFailed>(
            QString("%1 - Origin stream need only 1 path").arg(routing_result.GetStreamName()));
        return act_status;
      }
    }
  }

  return act_status;
}