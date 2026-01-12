#include "act_schedule_config.hpp"

void ActScheduleConfig::ComputeCycleTime() {
  for (ActScheduleConfigStream schedule_config_stream_ : this->schedule_config_streams_.values()) {
    this->cycle_time_ = std::lcm(this->cycle_time_, schedule_config_stream_.GetInterval());
  }
}

ACT_STATUS ActScheduleConfig::PrepareScheduleConfigVLAN(ActProject &act_project) {
  ActScheduleConfigVLAN &schedule_config_vlan = this->schedule_confic_vlan_;
  schedule_config_vlan.SetMinVLAN(act_project.GetVlanRange().GetMin());
  schedule_config_vlan.SetMaxVLAN(act_project.GetVlanRange().GetMax());
  schedule_config_vlan.SetLastAssignVLAN(act_project.GetVlanRange().GetMin());

  ActTrafficDesign &traffic_design = act_project.GetTrafficDesign();
  QSet<ActTrafficApplication> &application_setting = traffic_design.GetApplicationSetting();
  QSet<ActTrafficStream> &stream_setting = traffic_design.GetStreamSetting();
  for (ActTrafficStream traffic_stream : stream_setting) {
    if (!traffic_stream.GetActive()) {
      continue;
    }
    QString &destination_mac = traffic_stream.GetDestinationMac();
    QSet<ActTrafficApplication>::iterator application_iter =
        application_setting.find(ActTrafficApplication(traffic_stream.GetApplicationId()));
    if (application_iter == application_setting.end()) {
      QString error_msg = QString("Cannot find application %1").arg(traffic_stream.GetApplicationId());
      return std::make_shared<ActStatusFeasibilityCheckFailed>(error_msg);
    }

    ActTrafficVlanSetting vlan_setting = application_iter->GetVlanSetting();
    if (vlan_setting.GetTagged() || vlan_setting.GetUserDefinedVlan()) {
      quint16 vlan_id = vlan_setting.GetVlanId();
      if (!schedule_config_vlan.SetVLAN(vlan_id, destination_mac)) {
        QString error_msg = QString("Vlan %1 + MAC %2 duplicated").arg(vlan_id).arg(destination_mac);
        return std::make_shared<ActStatusFeasibilityCheckFailed>(error_msg);
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

bool ActScheduleConfigVLAN::SetVLAN(quint16 &vlan_id, QString &destination_mac) {
  if (!this->IsAvailableVLAN(vlan_id, destination_mac)) {
    return false;
  }

  this->vlan_map_[destination_mac].insert(vlan_id);
  return true;
}

ACT_STATUS ActScheduleConfigVLAN::GenerateSystemAssignVLAN(quint16 &vlan_id, QString &destination_mac) {
  vlan_id = this->last_assign_vlan_;
  if (this->vlan_map_.contains(destination_mac)) {
    QSet<quint16> vlan_set = this->vlan_map_[destination_mac];
    while (vlan_set.contains(vlan_id)) {
      vlan_id = (vlan_id == this->max_vlan_) ? this->min_vlan_ : (vlan_id + 1);
      if (vlan_id == this->last_assign_vlan_) {
        QString error_msg = QString("System assign VLAN %1 failed").arg(vlan_id);
        return std::make_shared<ActStatusFeasibilityCheckFailed>(error_msg);
      }
    }
  }

  this->SetVLAN(vlan_id, destination_mac);
  this->last_assign_vlan_ = vlan_id + 1;

  return ACT_STATUS_SUCCESS;
}

bool ActScheduleConfigVLAN::IsAvailableVLAN(quint16 &vlan_id, QString &destination_mac) {
  if (vlan_id < this->min_vlan_ || vlan_id > this->max_vlan_) {
    return false;
  } else if (this->vlan_map_.contains(destination_mac) && this->vlan_map_[destination_mac].contains(vlan_id)) {
    return false;
  }

  return true;
}

ACT_STATUS ActScheduleConfig::PrepareScheduleConfigDevice(ActProject &act_project) {
  ACT_STATUS_INIT();

  for (ActDevice device : act_project.GetDevices()) {
    ActScheduleConfigDevice schedule_config_device;
    schedule_config_device.SetDeviceId(device.GetId());
    schedule_config_device.SetIpAddress(device.GetIpv4().GetIpAddress());

    ActDeviceProperty &device_property = device.GetDeviceProperty();
    schedule_config_device.SetTickGranularity(device_property.GetTickGranularity());
    schedule_config_device.SetNumberOfQueue(device_property.GetNumberOfQueue());
    schedule_config_device.SetPerQueueSize(device_property.GetPerQueueSize());
    schedule_config_device.SetGateControlListLength(device_property.GetGateControlListLength());

    schedule_config_device.SetTsnEnable(
        device_property.GetFeatureGroup().GetConfiguration().GetTSN().GetIEEE802Dot1Qbv());
    schedule_config_device.SetCbEnable(
        device_property.GetFeatureGroup().GetConfiguration().GetTSN().GetIEEE802Dot1CB());
    schedule_config_device.SetEndStation((device.GetDeviceType() == ActDeviceTypeEnum::kEndStation ||
                                          device.GetDeviceType() == ActDeviceTypeEnum::kBridgedEndStation)
                                             ? true
                                             : false);

    QMap<quint16, ActProcessingDelay> &processing_delay_map = device_property.GetProcessingDelayMap();
    QMap<qint64, ActScheduleConfigInterface> &schedule_config_interfaces =
        schedule_config_device.GetScheduleConfigInterfaces();
    for (ActInterface &interface : device.GetInterfaces()) {
      if (!interface.GetUsed()) {
        continue;
      }
      ActScheduleConfigInterface schedule_config_interface;
      schedule_config_interface.SetDeviceId(device.GetId());
      schedule_config_interface.SetInterfaceId(interface.GetInterfaceId());
      schedule_config_interface.SetInterfaceName(interface.GetInterfaceName());

      ActLink link;
      act_status = act_project.GetLinkByInterfaceId(link, device.GetId(), interface.GetInterfaceId());
      if (!IsActStatusSuccess(act_status)) {
        continue;
      }
      schedule_config_interface.SetLinkId(link.GetId());

      schedule_config_interface.SetConnectDeviceId(
          link.GetSourceDeviceId() == device.GetId() ? link.GetDestinationDeviceId() : link.GetSourceDeviceId());
      schedule_config_interface.SetConnectInterfaceId(link.GetSourceInterfaceId() == interface.GetInterfaceId()
                                                          ? link.GetDestinationInterfaceId()
                                                          : link.GetSourceInterfaceId());

      if (processing_delay_map.contains(link.GetSpeed())) {
        ActProcessingDelay &processing_delay = processing_delay_map[link.GetSpeed()];
        schedule_config_interface.SetDependentProcessingDelayRatio(processing_delay.GetDependentDelayRatio());
        schedule_config_interface.SetIndependentProcessingDelay(processing_delay.GetIndependentDelay());
      }
      schedule_config_interfaces.insert(interface.GetInterfaceId(), schedule_config_interface);
    }

    this->schedule_config_devices_.insert(device.GetId(), schedule_config_device);
  }

  return act_status;
}

ACT_STATUS ActScheduleConfig::PrepareScheduleConfigStream(ActProject &act_project) {
  ACT_STATUS_INIT();

  bool keep_previous_result = act_project.GetProjectSetting().GetAlgorithmConfiguration().GetKeepPreviousResult();
  ActTrafficDesign &traffic_design = act_project.GetTrafficDesign();
  for (ActTrafficStream stream_setting : traffic_design.GetStreamSetting()) {
    if (!stream_setting.GetActive()) {
      continue;
    }
    ActScheduleConfigStream schedule_config_stream;
    schedule_config_stream.SetStreamId(stream_setting.GetId());
    schedule_config_stream.SetStreamName(stream_setting.GetStreamName());
    schedule_config_stream.SetMulticast(stream_setting.GetMulticast());
    schedule_config_stream.SetDestinationMac(stream_setting.GetDestinationMac());

    ActTrafficStreamInterface &talker = stream_setting.GetTalker();
    ActScheduleConfigInterface &schedule_config_talker =
        this->GetScheduleConfigInterface(talker.GetDeviceId(), talker.GetInterfaceId());
    schedule_config_stream.SetTalker(schedule_config_talker);

    for (ActTrafficStreamInterface listener : stream_setting.GetListeners()) {
      ActScheduleConfigInterface &schedule_config_listener =
          this->GetScheduleConfigInterface(listener.GetDeviceId(), listener.GetInterfaceId());
      schedule_config_stream.GetListeners().insert(schedule_config_listener);
    }

    QSet<ActTrafficApplication> &traffic_application_set = traffic_design.GetApplicationSetting();
    QSet<ActTrafficApplication>::iterator traffic_application_iter =
        traffic_application_set.find(ActTrafficApplication(stream_setting.GetApplicationId()));
    if (traffic_application_iter != traffic_application_set.end()) {
      schedule_config_stream.SetApplicationName(traffic_application_iter->GetApplicationName());

      ActTrafficSetting traffic_setting = traffic_application_iter->GetTrafficSetting();
      ActTrafficStreamParameter stream_parameter = traffic_application_iter->GetStreamParameter();
      ActTrafficTSN tsn = traffic_application_iter->GetTSN();
      ActTrafficVlanSetting vlan_setting = traffic_application_iter->GetVlanSetting();

      schedule_config_stream.SetTsnEnable(traffic_application_iter->GetTSN().GetActive());
      schedule_config_stream.SetCbEnable(traffic_application_iter->GetTSN().GetFRER());
      schedule_config_stream.SetKeepPreviousResult(keep_previous_result);
      schedule_config_stream.SetQosType(traffic_setting.GetQosType());
      schedule_config_stream.SetInterval(qint64(stream_parameter.GetInterval() * 1000));
      schedule_config_stream.SetEarliestTransmitOffset(qint64(stream_parameter.GetEarliestTransmitOffset() * 1000));
      schedule_config_stream.SetLatestTransmitOffset(qint64(stream_parameter.GetLatestTransmitOffset() * 1000));
      schedule_config_stream.SetJitter(qint64(stream_parameter.GetJitter() * 1000));

      if (traffic_setting.GetQosType() == ActTrafficQosTypeEnum::kBandwidth) {
        schedule_config_stream.SetMinReceiveOffset(0);
        schedule_config_stream.SetMaxReceiveOffset(stream_parameter.GetInterval() * 1000);
      } else if (traffic_setting.GetQosType() == ActTrafficQosTypeEnum::kBoundedLatency) {
        schedule_config_stream.SetMinReceiveOffset(static_cast<qint64>(traffic_setting.GetMinReceiveOffset() * 1000));
        schedule_config_stream.SetMaxReceiveOffset(static_cast<qint64>(traffic_setting.GetMaxReceiveOffset() * 1000));
      } else if (traffic_setting.GetQosType() == ActTrafficQosTypeEnum::kDeadline) {
        schedule_config_stream.SetMinReceiveOffset(0);
        schedule_config_stream.SetMaxReceiveOffset(traffic_setting.GetMaxReceiveOffset() * 1000);
      }

      for (ActTrafficTypeConfiguration &traffic_type_configuration :
           traffic_design.GetTrafficTypeConfigurationSetting()) {
        if (traffic_type_configuration.GetTrafficClass() != traffic_setting.GetTrafficTypeClass()) {
          continue;
        }

        schedule_config_stream.SetTrafficType(traffic_type_configuration.GetTrafficType());

        // set priority code point
        if (vlan_setting.GetTagged() || vlan_setting.GetUserDefinedVlan()) {
          schedule_config_stream.GetPCPs().insert(vlan_setting.GetPriorityCodePoint());
        } else {
          schedule_config_stream.SetPCPs(traffic_type_configuration.GetPriorityCodePointSet());
        }
      }

      // set stream payload including frame and media specific overhead
      // IF MaxBytesPerInterval == 0, consider the case below
      qint64 max_bytes_per_interval =
          (stream_parameter.GetMaxBytesPerInterval() == 0)
              ? stream_parameter.GetMaxFrameSize() * stream_parameter.GetMaxFramesPerInterval()
              : stream_parameter.GetMaxBytesPerInterval();
      qint64 max_frames_per_interval =
          qCeil(static_cast<qreal>(max_bytes_per_interval) / static_cast<qreal>(stream_parameter.GetMaxFrameSize()));
      qint64 media_specific_overhead_bytes = act_project.GetAlgorithmConfiguration().GetMediaSpecificOverheadBytes() +
                                             ((traffic_application_iter->GetTSN().GetFRER()) ? 6 : 0);
      qint64 payload = (max_bytes_per_interval + media_specific_overhead_bytes * max_frames_per_interval) * 8;
      schedule_config_stream.SetPayload(payload);

      // set VLAN
      quint16 vlan_id = vlan_setting.GetVlanId();
      if (!vlan_setting.GetTagged() && !vlan_setting.GetUserDefinedVlan()) {
        act_status = this->schedule_confic_vlan_.GenerateSystemAssignVLAN(vlan_id, stream_setting.GetDestinationMac());
        if (!IsActStatusSuccess(act_status)) {
          return act_status;
        }
      }
      schedule_config_stream.SetVlanId(vlan_id);
    }

    if (keep_previous_result) {
      QSet<ActStreamViewResult> &stream_view_results = act_project.GetComputedResult().GetStreamViewResults();
      QSet<ActStreamViewResult>::iterator stream_view_result_iter =
          stream_view_results.find(ActStreamViewResult(stream_setting.GetId()));
      if (stream_view_result_iter != stream_view_results.end()) {
        ActStreamViewResult stream_view_result = *stream_view_result_iter;
        ActStreamPathResult &stream_path_result = stream_view_result.GetStreamPathResults().first();
        ActStreamRedundantPathResult &stream_redundant_path_result =
            stream_path_result.GetStreamRedundantPathResults().first();
        ActStreamDeviceInterfaceResult &device_interface_result =
            stream_redundant_path_result.GetDeviceInterfaceResults().first();
        schedule_config_stream.SetEarliestTransmitOffset(device_interface_result.GetStartTime());
        schedule_config_stream.SetLatestTransmitOffset(device_interface_result.GetStartTime());
      }
    }

    this->schedule_config_streams_.insert(stream_setting.GetId(), schedule_config_stream);
  }

  return act_status;
}

ACT_STATUS ActScheduleConfig::PrepareScheduleConfigLink(ActProject &act_project) {
  ACT_STATUS_INIT();

  for (ActLink link : act_project.GetLinks()) {
    ActScheduleConfigLink schedule_config_link;
    schedule_config_link.SetLinkId(link.GetId());
    schedule_config_link.SetBandwidth(link.GetSpeed() * TRANSFER_RATE * TRANSFER_RATE);
    schedule_config_link.SetPropagationDelay(link.GetPropagationDelay());
    schedule_config_link.SetTimeSyncDelay(act_project.GetAlgorithmConfiguration().GetTimeSyncDelay());

    this->schedule_config_links_.insert(link.GetId(), schedule_config_link);
  }

  return act_status;
}

QList<qint64> ActScheduleConfig::SortScheduleConfigStreams() {
  QMap<quint8, QSet<qint64>> pcp_stream_map;
  for (qint64 stream_id : this->schedule_config_streams_.keys()) {
    ActScheduleConfigStream &schedule_config_stream = this->GetScheduleConfigStream(stream_id);

    quint8 pcp = schedule_config_stream.GetPCPs().values().first();
    pcp_stream_map[pcp].insert(stream_id);
  }

  QList<qint64> stream_list;
  for (quint8 pcp : pcp_stream_map.keys()) {
    for (qint64 stream_id : pcp_stream_map[pcp]) {
      stream_list.prepend(stream_id);
    }
  }

  return stream_list;
}

bool ActScheduleConfig::IsListenerArrived(const qint64 &stream_id, const qint64 &device_id,
                                          const qint64 &ingress_interface_id, const qint64 &egress_interface_id,
                                          const qint64 &link_id) {
  ActScheduleConfigStream &schedule_config_stream = this->GetScheduleConfigStream(stream_id);
  QSet<ActScheduleConfigInterface> &listeners = schedule_config_stream.GetListeners();
  ActScheduleConfigInterface schedule_config_interface(device_id, ingress_interface_id, link_id);
  return listeners.contains(schedule_config_interface) && (ingress_interface_id == egress_interface_id);
}

ActScheduleConfigStream &ActScheduleConfig::GetScheduleConfigStream(const qint64 &stream_id) {
  return this->schedule_config_streams_[stream_id];
}

ActScheduleConfigDevice &ActScheduleConfig::GetScheduleConfigDevice(const qint64 &device_id) {
  return this->schedule_config_devices_[device_id];
}

ActScheduleConfigLink &ActScheduleConfig::GetScheduleConfigLink(const qint64 &link_id) {
  return this->schedule_config_links_[link_id];
}

ActScheduleConfigInterface &ActScheduleConfigDevice::GetScheduleConfigInterface(const qint64 &interface_id) {
  return this->schedule_config_interfaces_[interface_id];
}

ActScheduleConfigInterface &ActScheduleConfig::GetScheduleConfigInterface(const qint64 &device_id,
                                                                          const qint64 &interface_id) {
  return this->GetScheduleConfigDevice(device_id).GetScheduleConfigInterface(interface_id);
}

qint64 ActScheduleConfig::ComputeStreamDuration(const qint64 &stream_id, const qint64 &link_id) {
  ActScheduleConfigStream &schedule_config_stream = this->GetScheduleConfigStream(stream_id);
  ActScheduleConfigLink &schedule_config_link = this->GetScheduleConfigLink(link_id);

  return qCeil(static_cast<qreal>(schedule_config_stream.GetPayload()) /
               static_cast<qreal>(schedule_config_link.GetBandwidth()) * qPow(10, 9));
}

qint64 ActScheduleConfig::ComputeTotalDelay(const qint64 &device_id, const qint64 &interface_id,
                                            const qint64 &stream_id) {
  ActScheduleConfigInterface &schedule_config_interface = this->GetScheduleConfigInterface(device_id, interface_id);
  return this->ComputeTransmitDelay(schedule_config_interface.GetLinkId()) +
         this->ComputeProcessingDelay(device_id, interface_id, stream_id);
}

qint64 ActScheduleConfig::ComputeTransmitDelay(const qint64 &link_id) {
  ActScheduleConfigLink &schedule_config_link = this->GetScheduleConfigLink(link_id);
  return schedule_config_link.GetPropagationDelay() + schedule_config_link.GetTimeSyncDelay();
}

qint64 ActScheduleConfig::ComputeProcessingDelay(const qint64 &device_id, const qint64 &interface_id,
                                                 const qint64 &stream_id) {
  ActScheduleConfigInterface &schedule_config_interface = this->GetScheduleConfigInterface(device_id, interface_id);
  return schedule_config_interface.GetIndependentProcessingDelay() +
         schedule_config_interface.GetDependentProcessingDelayRatio() *
             this->ComputeStreamDuration(stream_id, schedule_config_interface.GetLinkId());
}
