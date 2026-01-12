#include "act_schedule_result.hpp"

void ActScheduleResult::GenerateRoutingResult(ActComputedResult &computed_result, ActScheduleConfig &schedule_config) {
  QSet<ActRoutingResult> routing_results;
  QMap<qint64, ActScheduleConfigStream> &schedule_config_streams = schedule_config.GetScheduleConfigStreams();

  for (qint64 &stream_id : schedule_config_streams.keys()) {
    ActScheduleConfigStream &schedule_config_stream = schedule_config_streams[stream_id];

    ActScheduleConfigInterface &talker = schedule_config_stream.GetTalker();
    QSet<ActScheduleConfigInterface> &listeners = schedule_config_stream.GetListeners();

    ActRoutingResult routing_result;
    routing_result.SetStreamName(schedule_config_stream.GetStreamName());
    routing_result.SetStreamId(schedule_config_stream.GetStreamId());
    routing_result.SetVlanId(schedule_config_stream.GetVlanId());
    routing_result.SetQueueId(schedule_config_stream.GetQueueId());
    routing_result.SetPriorityCodePoint(schedule_config_stream.GetPriorityCodePoint());
    routing_result.SetCB(schedule_config_stream.GetCbEnable());
    routing_result.SetMulticast(schedule_config_stream.GetMulticast());
    QList<ActRoutingPath> &routing_paths = routing_result.GetRoutingPaths();

    for (ActScheduleConfigInterface listener : listeners) {
      ActRoutingPath routing_path;
      routing_path.SetTalkerIpAddress(schedule_config.GetScheduleConfigDevice(talker.GetDeviceId()).GetIpAddress());
      routing_path.SetListenerIpAddress(schedule_config.GetScheduleConfigDevice(listener.GetDeviceId()).GetIpAddress());

      ActScheduleResultStream &schedule_result_stream = this->GetScheduleResultStream(
          listener.GetDeviceId(), listener.GetInterfaceId(), schedule_config_stream.GetQueueId(), stream_id);
      for (qint64 &sequence_id : schedule_result_stream.GetScheduleResultSequences().keys()) {
        ActScheduleResultSequence &schedule_result_sequence =
            schedule_result_stream.GetScheduleResultSequence(sequence_id);

        QList<ActRedundantPath> redundant_paths;
        for (qint64 &frame_id : schedule_result_sequence.GetScheduleResultFrames().keys()) {
          ActScheduleResultFrame schedule_result_frame = schedule_result_sequence.GetScheduleResultFrame(frame_id);
          quint16 vlan_id = schedule_result_frame.GetVlanId();

          ActRedundantPath redundant_path;
          while (schedule_result_frame.GetDeviceId() != talker.GetDeviceId()) {
            redundant_path.GetDeviceIds().prepend(schedule_result_frame.GetDeviceId());
            redundant_path.GetLinkIds().prepend(schedule_result_frame.GetPrevLinkId());
            schedule_result_frame = this->GetScheduleResultFrame(
                schedule_result_frame.GetPrevDeviceId(), schedule_result_frame.GetPrevInterfaceId(),
                schedule_result_frame.GetQueueId(), schedule_result_frame.GetStreamId(),
                schedule_result_frame.GetSeqenceId(), schedule_result_frame.GetPrevFrameId());
            vlan_id = (schedule_result_frame.GetVlanId() != schedule_config_stream.GetVlanId())
                          ? schedule_result_frame.GetVlanId()
                          : vlan_id;
          }
          redundant_path.SetVlanId(vlan_id);
          redundant_path.GetDeviceIds().prepend(schedule_result_frame.GetDeviceId());

          if (vlan_id == schedule_config_stream.GetVlanId()) {
            redundant_paths.prepend(redundant_path);
          } else {
            redundant_paths.append(redundant_path);
          }
        }
        routing_path.SetRedundantPaths(redundant_paths);
        routing_paths.append(routing_path);
      }
    }
    routing_results.insert(routing_result);
  }
  computed_result.SetRoutingResults(routing_results);
}

void ActScheduleResult::GenerateDeviceResult(ActComputedResult &computed_result, ActScheduleConfig &schedule_config) {
  QSet<ActDeviceViewResult> device_view_results;

  for (qint64 &device_id : this->schedule_result_devices_.keys()) {
    ActScheduleResultDevice &schedule_result_device = this->GetScheduleResultDevice(device_id);
    ActScheduleConfigDevice &schedule_config_device = schedule_config.GetScheduleConfigDevice(device_id);

    ActDeviceViewResult device_view_result;
    device_view_result.SetDeviceId(device_id);
    device_view_result.SetDeviceIp(schedule_config_device.GetIpAddress());
    QSet<ActDeviceInterfaceResult> &interface_results = device_view_result.GetInterfaceResults();

    for (qint64 &interface_id : schedule_result_device.GetScheduleResultInterfaces().keys()) {
      ActScheduleResultInterface &schedule_result_interface =
          schedule_result_device.GetScheduleResultInterface(interface_id);
      ActScheduleConfigInterface &schedule_config_interface =
          schedule_config_device.GetScheduleConfigInterface(interface_id);

      ActDeviceInterfaceResult interface_result;
      interface_result.SetInterfaceId(interface_id);
      interface_result.SetInterfaceName(schedule_config_interface.GetInterfaceName());
      QSet<ActDeviceInterfaceStreamResult> &stream_results = interface_result.GetStreamResults();

      for (quint8 &queue_id : schedule_result_interface.GetScheduleResultQueues().keys()) {
        ActScheduleResultQueue &schedule_result_queue = schedule_result_interface.GetScheduleResultQueue(queue_id);

        for (qint64 &stream_id : schedule_result_queue.GetScheduleResultStreams().keys()) {
          ActScheduleResultStream &schedule_result_stream = schedule_result_queue.GetScheduleResultStream(stream_id);
          ActScheduleConfigStream &schedule_config_stream = schedule_config.GetScheduleConfigStream(stream_id);

          ActDeviceInterfaceStreamResult stream_result;
          stream_result.SetStreamId(stream_id);
          stream_result.SetStreamName(schedule_config_stream.GetStreamName());
          stream_result.SetPriorityCodePoint(schedule_config_stream.GetPriorityCodePoint());

          ActQueueIdEnum queue = static_cast<ActQueueIdEnum>(queue_id);

          ActScheduleConfigDevice &schedule_config_talker =
              schedule_config.GetScheduleConfigDevice(schedule_config_stream.GetTalker().GetDeviceId());
          stream_result.SetTalker(schedule_config_talker.GetIpAddress());
          if (device_id == schedule_config_stream.GetTalker().GetDeviceId()) {
            queue = ActQueueIdEnum::kTalker;
          }

          for (ActScheduleConfigInterface listener : schedule_config_stream.GetListeners()) {
            ActScheduleConfigDevice &schedule_config_listener =
                schedule_config.GetScheduleConfigDevice(listener.GetDeviceId());
            stream_result.GetListeners().append(schedule_config_listener.GetIpAddress());
            if (device_id == listener.GetDeviceId()) {
              queue = ActQueueIdEnum::kListener;
            }
          }

          stream_result.SetQueueId(queue);

          for (qint64 &sequence_id : schedule_result_stream.GetScheduleResultSequences().keys()) {
            ActScheduleResultSequence &schedule_result_sequence =
                schedule_result_stream.GetScheduleResultSequence(sequence_id);

            for (qint64 &frame_id : schedule_result_sequence.GetScheduleResultFrames().keys()) {
              ActScheduleResultFrame &schedule_result_frame = schedule_result_sequence.GetScheduleResultFrame(frame_id);

              stream_result.SetVlanId(schedule_result_frame.GetVlanId());
              stream_result.SetStartTime(schedule_result_frame.GetDequeueTime());
              stream_result.SetStopTime(schedule_result_frame.GetDequeueTime() + schedule_result_frame.GetDuration());

              stream_results.insert(stream_result);
            }
          }
        }
      }
      interface_results.insert(interface_result);
    }
    device_view_results.insert(device_view_result);
  }
  computed_result.SetDeviceViewResults(device_view_results);
}

void ActScheduleResult::GenerateStreamResult(ActComputedResult &computed_result, ActScheduleConfig &schedule_config) {
  QSet<ActStreamViewResult> stream_view_results;

  QMap<qint64, ActScheduleConfigStream> &schedule_config_streams = schedule_config.GetScheduleConfigStreams();

  for (qint64 &stream_id : schedule_config_streams.keys()) {
    ActScheduleConfigStream &schedule_config_stream = schedule_config_streams[stream_id];
    ActScheduleConfigInterface &talker = schedule_config_stream.GetTalker();

    ActStreamViewResult stream_view_result;
    stream_view_result.SetStreamId(stream_id);
    stream_view_result.SetStreamName(schedule_config_stream.GetStreamName());
    stream_view_result.SetStreamTrafficType(schedule_config_stream.GetTrafficType());
    stream_view_result.SetInterval(ActInterval(schedule_config_stream.GetInterval()));
    QList<ActStreamPathResult> &stream_path_results = stream_view_result.GetStreamPathResults();

    for (ActScheduleConfigInterface listener : schedule_config_stream.GetListeners()) {
      ActScheduleResultStream &schedule_result_stream = this->GetScheduleResultStream(
          listener.GetDeviceId(), listener.GetInterfaceId(), schedule_config_stream.GetPriorityCodePoint(), stream_id);
      for (qint64 &sequence_id : schedule_result_stream.GetScheduleResultSequences().keys()) {
        ActScheduleResultSequence &schedule_result_sequence =
            schedule_result_stream.GetScheduleResultSequence(sequence_id);

        ActStreamPathResult stream_path_result;
        QList<ActStreamRedundantPathResult> &stream_redundant_path_results =
            stream_path_result.GetStreamRedundantPathResults();
        for (qint64 &frame_id : schedule_result_sequence.GetScheduleResultFrames().keys()) {
          ActScheduleResultFrame schedule_result_frame = schedule_result_sequence.GetScheduleResultFrame(frame_id);
          schedule_result_frame.SetEgressInterfaceId(-1);

          // Generate stream routing path
          QList<ActScheduleResultFrame> schedule_result_frames;
          while (schedule_result_frame.GetDeviceId() != talker.GetDeviceId()) {
            schedule_result_frames.prepend(schedule_result_frame);
            schedule_result_frame = this->GetScheduleResultFrame(
                schedule_result_frame.GetPrevDeviceId(), schedule_result_frame.GetPrevInterfaceId(),
                schedule_result_frame.GetQueueId(), schedule_result_frame.GetStreamId(),
                schedule_result_frame.GetSeqenceId(), schedule_result_frame.GetPrevFrameId());
          }
          schedule_result_frames.prepend(schedule_result_frame);

          ActStreamRedundantPathResult stream_redundant_path_result;
          QList<ActStreamDeviceInterfaceResult> &device_interface_results =
              stream_redundant_path_result.GetDeviceInterfaceResults();
          bool redundant_vlan = false;
          for (ActScheduleResultFrame &schedule_result_frame_ : schedule_result_frames) {
            ActStreamDeviceInterfaceResult device_interface_result;
            device_interface_result.SetDeviceId(schedule_result_frame_.GetDeviceId());
            device_interface_result.SetDeviceIp(
                schedule_config.GetScheduleConfigDevice(schedule_result_frame_.GetDeviceId()).GetIpAddress());

            device_interface_result.SetIngressInterfaceId(schedule_result_frame_.GetIngressInterfaceId());
            device_interface_result.SetEgressInterfaceId(schedule_result_frame_.GetEgressInterfaceId());

            device_interface_result.SetPriorityCodePoint(schedule_result_frame_.GetPriorityCodePoint());

            device_interface_result.SetStartTime(schedule_result_frame_.GetDequeueTime());
            device_interface_result.SetStopTime(schedule_result_frame_.GetDequeueTime() +
                                                schedule_result_frame_.GetDuration());

            device_interface_result.SetIngressInterfaceName(
                schedule_config
                    .GetScheduleConfigInterface(schedule_result_frame_.GetDeviceId(),
                                                schedule_result_frame_.GetIngressInterfaceId())
                    .GetInterfaceName());
            device_interface_result.SetEgressInterfaceName(
                schedule_config
                    .GetScheduleConfigInterface(schedule_result_frame_.GetDeviceId(),
                                                schedule_result_frame_.GetEgressInterfaceId())
                    .GetInterfaceName());
            device_interface_result.SetAccumulatedLatency(schedule_result_frame_.GetCost());
            device_interface_result.SetStreamDuration(schedule_result_frame_.GetDuration());

            device_interface_result.SetQueueId((schedule_result_frame_.GetDeviceId() == talker.GetDeviceId())
                                                   ? ActQueueIdEnum::kTalker
                                               : (schedule_result_frame_.GetDeviceId() == listener.GetDeviceId())
                                                   ? ActQueueIdEnum::kListener
                                                   : static_cast<ActQueueIdEnum>(schedule_result_frame_.GetQueueId()));

            device_interface_result.SetVlanId(schedule_result_frame_.GetVlanId());
            device_interface_result.SetFrerType(schedule_result_frame_.GetFrerType());

            device_interface_results.append(device_interface_result);

            if (schedule_result_frame_.GetVlanId() != schedule_config_stream.GetVlanId()) {
              redundant_vlan = true;
            }
          }
          if (redundant_vlan) {
            stream_redundant_path_results.append(stream_redundant_path_result);
          } else {
            stream_redundant_path_results.prepend(stream_redundant_path_result);
          }
        }
        stream_path_results.append(stream_path_result);
      }
    }
    stream_view_results.insert(stream_view_result);
  }
  computed_result.SetStreamViewResults(stream_view_results);
}

void ActScheduleResult::GenerateGCLResult(ActComputedResult &computed_result, ActScheduleConfig &schedule_config) {
  QSet<ActGclResult> gcl_results;

  for (qint64 &device_id : this->schedule_result_devices_.keys()) {
    ActScheduleResultDevice &schedule_result_device = this->GetScheduleResultDevice(device_id);
    ActScheduleConfigDevice &schedule_config_device = schedule_config.GetScheduleConfigDevice(device_id);

    if (!schedule_config_device.GetTsnEnable()) {
      continue;
    }

    ActGclResult gcl_result(device_id, schedule_config_device.GetIpAddress());
    QList<ActInterfaceGateControls> &interface_gate_controls = gcl_result.GetInterfaceGateControls();

    for (qint64 &interface_id : schedule_result_device.GetScheduleResultInterfaces().keys()) {
      ActScheduleResultInterface &schedule_result_interface =
          schedule_result_device.GetScheduleResultInterface(interface_id);
      ActScheduleConfigInterface &schedule_config_interface =
          schedule_config_device.GetScheduleConfigInterface(interface_id);

      QList<ActScheduleResultFrame> schedule_result_frame_list;
      for (ActScheduleResultQueue &schedule_result_queue :
           schedule_result_interface.GetScheduleResultQueues().values()) {
        for (ActScheduleResultStream &schedule_result_stream :
             schedule_result_queue.GetScheduleResultStreams().values()) {
          for (ActScheduleResultSequence &schedule_result_sequence :
               schedule_result_stream.GetScheduleResultSequences().values()) {
            for (ActScheduleResultFrame &schedule_result_frame :
                 schedule_result_sequence.GetScheduleResultFrames().values()) {
              schedule_result_frame_list.append(schedule_result_frame);
            }
          }
        }
      }

      if (schedule_result_frame_list.isEmpty()) {
        continue;
      }

      std::sort(schedule_result_frame_list.begin(), schedule_result_frame_list.end());

      QSet<quint8> queue_set;
      for (quint8 queue_id = 0; queue_id < schedule_config_device.GetNumberOfQueue(); queue_id++) {
        queue_set.insert(queue_id);
      }
      quint8 gate_states_values = this->GetGateStatesValue(queue_set);

      // Sorted gate control list by start time
      QList<ActGateControl> gate_control_list;
      for (ActScheduleResultFrame &schedule_result_frame : schedule_result_frame_list) {
        ActGateControl gate_control(schedule_result_frame.GetDequeueTime() % schedule_config.GetCycleTime(),
                                    (schedule_result_frame.GetDequeueTime() + schedule_result_frame.GetDuration()) %
                                        schedule_config.GetCycleTime(),
                                    this->GetGateStatesValue(schedule_result_frame.GetQueueId()));
        gate_control_list.append(gate_control);
      }
      std::sort(gate_control_list.begin(), gate_control_list.end());

      ActInterfaceGateControls interface_gate_control(interface_id, schedule_config_interface.GetInterfaceName());
      QList<ActGateControl> &gate_controls = interface_gate_control.GetGateControls();
      for (ActGateControl &gate_control : gate_control_list) {
        if (gate_controls.isEmpty()) {
          if (gate_control.GetStartTime() != 0) {
            gate_controls.append(
                ActGateControl(0, gate_control.GetStartTime(), gate_states_values - gate_control.GetGateStatesValue()));
          }
        } else {
          ActGateControl &gate_control_last = gate_controls.last();
          if (gate_control_last.GetStopTime() < gate_control.GetStartTime()) {
            if (gate_control_last.GetGateStatesValue() == gate_control.GetGateStatesValue()) {
              gate_controls.append(ActGateControl(gate_control_last.GetStopTime(), gate_control.GetStartTime(),
                                                  gate_states_values - gate_control.GetGateStatesValue()));
            } else {
              gate_controls.append(ActGateControl(
                  gate_control_last.GetStopTime(), gate_control.GetStartTime(),
                  gate_states_values - gate_control_last.GetGateStatesValue() - gate_control.GetGateStatesValue()));
            }
          } else if (gate_control_last.GetGateStatesValue() == gate_control.GetGateStatesValue()) {
            gate_control_last.SetStopTime(gate_control.GetStopTime());
            continue;
          }
        }
        gate_controls.append(gate_control);
      }

      qint64 last_stop_time = gate_controls.last().GetStopTime();
      if ((schedule_config.GetCycleTime() - last_stop_time) > (schedule_config.GetReserveTime() * 2)) {
        gate_controls.append(ActGateControl(last_stop_time, last_stop_time + schedule_config.GetReserveTime(), 0));
        gate_controls.append(ActGateControl(last_stop_time + schedule_config.GetReserveTime(),
                                            schedule_config.GetCycleTime() - schedule_config.GetReserveTime(),
                                            gate_states_values));
        gate_controls.append(ActGateControl(schedule_config.GetCycleTime() - schedule_config.GetReserveTime(),
                                            schedule_config.GetCycleTime(), 0));
      } else {
        gate_controls.append(ActGateControl(last_stop_time, schedule_config.GetCycleTime(), 0));
      }

      interface_gate_controls.append(interface_gate_control);
    }

    gcl_results.insert(gcl_result);
  }

  computed_result.SetGclResults(gcl_results);
}

void ActScheduleResult::ConsiderQueueIsolation(ActScheduleResultFrame &schedule_result_frame, qint64 &offset) {
  qint64 stream_id = schedule_result_frame.GetStreamId();
  qint64 device_id = schedule_result_frame.GetDeviceId();
  qint64 interface_id = schedule_result_frame.GetEgressInterfaceId();
  qint64 queue_id = schedule_result_frame.GetQueueId();

  qint64 enqueue_time = schedule_result_frame.GetEnqueueTime();
  qint64 dequeue_time = schedule_result_frame.GetDequeueTime();
  qint64 duration = schedule_result_frame.GetDuration();

  if (this->IsScheduleResultQueueExist(device_id, interface_id, queue_id)) {
    ActScheduleResultQueue &schedule_result_queue = this->GetScheduleResultQueue(device_id, interface_id, queue_id);

    QList<ActScheduleResultFrame> schedule_result_frame_list;
    for (ActScheduleResultStream &schedule_result_stream_ : schedule_result_queue.GetScheduleResultStreams().values()) {
      for (ActScheduleResultSequence &schedule_result_sequence_ :
           schedule_result_stream_.GetScheduleResultSequences().values()) {
        for (ActScheduleResultFrame &schedule_result_frame_ :
             schedule_result_sequence_.GetScheduleResultFrames().values()) {
          schedule_result_frame_list.append(schedule_result_frame_);
        }
      }
    }

    std::sort(schedule_result_frame_list.begin(), schedule_result_frame_list.end());

    for (ActScheduleResultFrame &schedule_result_frame_ : schedule_result_frame_list) {
      if (schedule_result_frame_.GetEnqueueTime() < dequeue_time + duration &&
          schedule_result_frame_.GetDequeueTime() + schedule_result_frame_.GetDuration() > enqueue_time) {
        if (schedule_result_frame_.GetStreamId() != stream_id ||
            enqueue_time < schedule_result_frame_.GetEnqueueTime()) {
          enqueue_time = schedule_result_frame_.GetDequeueTime() + schedule_result_frame_.GetDuration();
        }
        dequeue_time = schedule_result_frame_.GetDequeueTime() + schedule_result_frame_.GetDuration();
      }
    }

    offset = enqueue_time - schedule_result_frame.GetEnqueueTime();
  }
}

void ActScheduleResult::ConsiderCycleTimeReservation(ActScheduleResultFrame &schedule_result_frame, qint64 &cycle_time,
                                                     qint64 &reserve_time) {
  qint64 &dequeue_time = schedule_result_frame.GetDequeueTime();
  qint64 duration = schedule_result_frame.GetDuration();

  if ((dequeue_time / cycle_time) != ((dequeue_time + duration) / cycle_time)) {
    dequeue_time = ((dequeue_time / cycle_time) + 1) * cycle_time;
  } else if ((dequeue_time + duration) % cycle_time > (cycle_time - reserve_time)) {
    dequeue_time = ((dequeue_time / cycle_time) + 1) * cycle_time;
  }
}

void ActScheduleResult::ConsiderLinkIsolation(ActScheduleResultFrame &schedule_result_frame, qint64 &cycle_time,
                                              qint64 &reserve_time) {
  qint64 device_id = schedule_result_frame.GetDeviceId();
  qint64 interface_id = schedule_result_frame.GetEgressInterfaceId();

  qint64 &dequeue_time = schedule_result_frame.GetDequeueTime();
  qint64 duration = schedule_result_frame.GetDuration();

  if (this->IsScheduleResultInterfaceExist(device_id, interface_id)) {
    ActScheduleResultInterface &schedule_result_interface = this->GetScheduleResultInterface(device_id, interface_id);

    QList<ActScheduleResultFrame> schedule_result_frame_list;
    for (ActScheduleResultQueue &schedule_result_queue_ :
         schedule_result_interface.GetScheduleResultQueues().values()) {
      for (ActScheduleResultStream &schedule_result_stream_ :
           schedule_result_queue_.GetScheduleResultStreams().values()) {
        for (ActScheduleResultSequence &schedule_result_sequence_ :
             schedule_result_stream_.GetScheduleResultSequences().values()) {
          for (ActScheduleResultFrame &schedule_result_frame_ :
               schedule_result_sequence_.GetScheduleResultFrames().values()) {
            schedule_result_frame_list.append(schedule_result_frame_);
          }
        }
      }
    }

    std::sort(schedule_result_frame_list.begin(), schedule_result_frame_list.end());

    for (ActScheduleResultFrame &schedule_result_frame_ : schedule_result_frame_list) {
      this->ConsiderCycleTimeReservation(schedule_result_frame, cycle_time, reserve_time);

      if (schedule_result_frame_.GetDequeueTime() % cycle_time < (dequeue_time + duration) % cycle_time &&
          (schedule_result_frame_.GetDequeueTime() + schedule_result_frame_.GetDuration()) % cycle_time >
              dequeue_time % cycle_time) {
        dequeue_time = schedule_result_frame_.GetDequeueTime() + schedule_result_frame_.GetDuration();
      }
    }
    this->ConsiderCycleTimeReservation(schedule_result_frame, cycle_time, reserve_time);
  }
}