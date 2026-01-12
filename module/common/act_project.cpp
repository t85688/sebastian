
#include "act_project.hpp"

// ACT_STATUS ActProject::IncreaseDataVersion() {
//   ACT_STATUS_INIT();
//   this->data_version_++;

//   return act_status;
// }

ACT_STATUS ActProject::SetProjectName(const QString &project_name) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetProjectName(project_name);
  return act_status;
}

QString ActProject::GetProjectName() const { return this->GetProjectSetting().GetProjectName(); }

ACT_STATUS ActProject::SetAlgorithmConfiguration(ActAlgorithmConfiguration &algorithm_configuration) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetAlgorithmConfiguration(algorithm_configuration);
  return act_status;
}

ActAlgorithmConfiguration ActProject::GetAlgorithmConfiguration() const {
  return this->GetProjectSetting().GetAlgorithmConfiguration();
}

ACT_STATUS ActProject::SetVlanRange(ActVlanRange &vlan_range) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetVlanRange(vlan_range);
  return act_status;
}

ActVlanRange ActProject::GetVlanRange() const { return this->GetProjectSetting().GetVlanRange(); }

ACT_STATUS ActProject::SetCfgWizardSetting(ActCfgWizardSetting &cfg_wizard_setting) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetCfgWizardSetting(cfg_wizard_setting);
  return act_status;
}

ActCfgWizardSetting ActProject::GetCfgWizardSetting() const { return this->GetProjectSetting().GetCfgWizardSetting(); }

ACT_STATUS ActProject::SetAccount(ActDeviceAccount &account) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetAccount(account);
  return act_status;
}

ActDeviceAccount ActProject::GetAccount() const { return this->GetProjectSetting().GetAccount(); }

ACT_STATUS ActProject::SetNetconfConfiguration(ActNetconfConfiguration &netconf_configuration) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetNetconfConfiguration(netconf_configuration);
  return act_status;
}

ActNetconfConfiguration ActProject::GetNetconfConfiguration() const {
  return this->GetProjectSetting().GetNetconfConfiguration();
}

ACT_STATUS ActProject::SetSnmpConfiguration(ActSnmpConfiguration &snmp_configuration) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetSnmpConfiguration(snmp_configuration);
  return act_status;
}

ActSnmpConfiguration ActProject::GetSnmpConfiguration() const {
  return this->GetProjectSetting().GetSnmpConfiguration();
}

ACT_STATUS ActProject::SetRestfulConfiguration(ActRestfulConfiguration &restful_configuration) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetRestfulConfiguration(restful_configuration);
  return act_status;
}

ActRestfulConfiguration ActProject::GetRestfulConfiguration() const {
  return this->GetProjectSetting().GetRestfulConfiguration();
}

ACT_STATUS ActProject::SetTrafficTypeToPriorityCodePointMapping(
    ActTrafficTypeToPriorityCodePointMapping &traffic_type_to_priority_code_point_mapping) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetTrafficTypeToPriorityCodePointMapping(traffic_type_to_priority_code_point_mapping);
  return act_status;
}

ActTrafficTypeToPriorityCodePointMapping ActProject::GetTrafficTypeToPriorityCodePointMapping() const {
  return this->GetProjectSetting().GetTrafficTypeToPriorityCodePointMapping();
}

ACT_STATUS ActProject::SetPriorityCodePointToQueueMapping(
    QMap<quint8, QSet<quint8>> &priority_code_point_to_queue_mapping) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetPriorityCodePointToQueueMapping(priority_code_point_to_queue_mapping);
  return act_status;
}

QMap<quint8, QSet<quint8>> ActProject::GetPriorityCodePointToQueueMapping() const {
  return this->GetProjectSetting().GetPriorityCodePointToQueueMapping();
}

ACT_STATUS ActProject::SetScanIpRanges(QList<ActScanIpRangeEntry> &scan_ip_ranges) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetScanIpRanges(scan_ip_ranges);
  return act_status;
}

QList<ActScanIpRangeEntry> ActProject::GetScanIpRanges() const { return this->GetProjectSetting().GetScanIpRanges(); }

ACT_STATUS ActProject::SetProjectStartIp(QString &start_ip) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetProjectStartIp(start_ip);
  return act_status;
}

ActIpv4 ActProject::GetProjectStartIp() const { return this->GetProjectSetting().GetProjectStartIp(); }

QString ActProject::GetDeviceIp(qint64 &device_id) const {
  ACT_STATUS_INIT();
  QString ip_address;
  ActDevice device;
  act_status = this->GetDeviceById(device, device_id);
  if (IsActStatusSuccess(act_status)) {
    ip_address = device.GetIpv4().GetIpAddress();
  }
  return ip_address;
};

ACT_STATUS ActProject::SetSnmpTrapConfiguration(ActSnmpTrapConfiguration &snmp_trap_configuration) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetSnmpTrapConfiguration(snmp_trap_configuration);
  return act_status;
}

ActSnmpTrapConfiguration ActProject::GetSnmpTrapConfiguration() const {
  return this->GetProjectSetting().GetSnmpTrapConfiguration();
}

ACT_STATUS ActProject::SetMonitorConfiguration(ActMonitorConfiguration &monitor_configuration) {
  ACT_STATUS_INIT();
  this->GetProjectSetting().SetMonitorConfiguration(monitor_configuration);
  return act_status;
}

ActMonitorConfiguration ActProject::GetMonitorConfiguration() const {
  return this->GetProjectSetting().GetMonitorConfiguration();
}

ACT_STATUS ActProject::HidePassword() {
  ACT_STATUS_INIT();

  // Hide password in the project setting
  this->GetProjectSetting().HidePassword();

  // Hide password in each device
  QSet<ActDevice> device_set;
  for (auto dev : this->GetDevices()) {
    dev.HidePassword();
    device_set.insert(dev);
  }
  this->SetDevices(device_set);

  // Hide password in compute result each device
  this->GetComputedResult().HidePassword();

  // Device Config
  this->GetDeviceConfig().HidePassword();

  return act_status;
}

ACT_STATUS ActProject::EncryptPassword() {
  ACT_STATUS_INIT();

  // Encrypt password in the project setting
  this->GetProjectSetting().EncryptPassword();

  // Encrypt password in each device
  QSet<ActDevice> device_set;
  for (auto dev : this->GetDevices()) {
    dev.EncryptPassword();
    device_set.insert(dev);
  }
  this->SetDevices(device_set);

  // Encrypt password in compute result
  this->GetComputedResult().EncryptPassword();

  // Device Config
  this->GetDeviceConfig().EncryptPassword();

  return act_status;
}

ACT_STATUS ActProject::DecryptPassword() {
  ACT_STATUS_INIT();

  // Decrypt password in the project setting
  this->GetProjectSetting().DecryptPassword();

  // Decrypt password in each device
  QSet<ActDevice> device_set;
  for (auto dev : this->GetDevices()) {
    dev.DecryptPassword();
    device_set.insert(dev);
  }
  this->SetDevices(device_set);

  // Decrypt password in compute result each device
  this->GetComputedResult().DecryptPassword();

  // Device Config
  this->GetDeviceConfig().DecryptPassword();

  return act_status;
}

void ActProject::GetHostPairsFromEndStationInterfaces(QList<ActHostPair> &host_pairs, const ActStream &stream) const {
  const qint64 &talker_device_id = stream.GetTalker().GetEndStationInterface().GetDeviceId();
  const qint64 &talker_interface_id = stream.GetTalker().GetEndStationInterface().GetInterfaceId();
  const QString &talker_ip_address = stream.GetTalker().GetEndStationInterface().GetIpAddress();
  for (const ActListener &listener : stream.GetListeners()) {
    const qint64 &listener_device_id = listener.GetEndStationInterface().GetDeviceId();
    const qint64 &listener_interface_id = listener.GetEndStationInterface().GetInterfaceId();
    const QString &listener_ip_address = listener.GetEndStationInterface().GetIpAddress();
    host_pairs.push_back(ActHostPair(talker_device_id, talker_interface_id, talker_ip_address, listener_device_id,
                                     listener_interface_id, listener_ip_address));
  }
  return;
}

ACT_STATUS ActProject::GetDeviceByIp(const QString &ip, ActDevice &device) const {
  ACT_STATUS_INIT();

  for (ActDevice proj_device : this->GetDevices()) {
    if (proj_device.GetIpv4().GetIpAddress() == ip) {
      device = proj_device;
      return act_status;
    }
  }
  return std::make_shared<ActStatusNotFound>(ip);
}

ACT_STATUS ActProject::GetDeviceIdByIp(qint64 &device_id, const QString &ip) const {
  ACT_STATUS_INIT();

  for (ActDevice device : this->GetDevices()) {
    if (device.GetIpv4().GetIpAddress() == ip) {
      device_id = device.GetId();
      return act_status;
    }
  }
  return std::make_shared<ActStatusNotFound>(ip);
}

ACT_STATUS ActProject::GetDeviceById(ActDevice &device, const qint64 &device_id) const {
  return ActGetItemById<ActDevice>(this->GetDevices(), device_id, device);
}

ACT_STATUS ActProject::GetStreamById(ActStream &stream, const qint64 &stream_id) const {
  return ActGetItemById<ActStream>(this->GetStreams(), stream_id, stream);
}

ACT_STATUS ActProject::GetStreamsByTalkerInterface(QSet<ActStream> &streams, const qint64 &device_id,
                                                   const qint64 &intf_id) {
  ACT_STATUS_INIT();

  for (ActStream stream : this->GetStreams()) {
    const qint64 talker_device_id = stream.GetTalker().GetEndStationInterface().GetDeviceId();
    const qint64 talker_intf_id = stream.GetTalker().GetEndStationInterface().GetInterfaceId();

    if (talker_device_id == device_id && talker_intf_id == intf_id) {
      streams.insert(stream);
    }
  }

  return act_status;
}

ACT_STATUS ActProject::GetStreamsByListenerInterface(QSet<ActStream> &streams, const qint64 &device_id,
                                                     const qint64 &intf_id) {
  ACT_STATUS_INIT();

  for (ActStream stream : this->GetStreams()) {
    QList<ActListener> listeners = stream.GetListeners();
    for (ActListener listener : listeners) {
      const qint64 listener_device_id = listener.GetEndStationInterface().GetDeviceId();
      const qint64 listener_intf_id = listener.GetEndStationInterface().GetInterfaceId();

      if (listener_device_id == device_id && listener_intf_id == intf_id) {
        streams.insert(stream);
      }
    }
  }

  return act_status;
}

ACT_STATUS ActProject::GetLinkById(ActLink &link, const qint64 &link_id) const {
  return ActGetItemById<ActLink>(this->GetLinks(), link_id, link);
}

ACT_STATUS ActProject::GetLinkByDeviceIds(ActLink &act_link, const QPair<qint64, qint64> &ids) const {
  ACT_STATUS_INIT();

  for (const ActLink &link : this->GetLinks()) {
    if (link.GetSourceDeviceId() == ids.first && link.GetDestinationDeviceId() == ids.second) {
      act_link = link;
      return act_status;
    } else if (link.GetSourceDeviceId() == ids.second && link.GetDestinationDeviceId() == ids.first) {
      act_link = link;
      return act_status;
    }
  }
  act_status->SetStatus(ActStatusType::kNotFound);
  return act_status;
}

ACT_STATUS ActProject::GetLinkByInterfaceId(ActLink &act_link, const qint64 &device_id,
                                            const qint64 &interface_id) const {
  ACT_STATUS_INIT();

  QSet<ActLink> link_set = this->GetLinks();

  for (const ActLink &link : link_set) {
    if (link.GetSourceDeviceId() == device_id && link.GetSourceInterfaceId() == interface_id) {
      act_link = link;
      return act_status;
    } else if (link.GetDestinationDeviceId() == device_id && link.GetDestinationInterfaceId() == interface_id) {
      act_link = link;
      return act_status;
    }
  }
  act_status->SetStatus(ActStatusType::kNotFound);
  return act_status;
}

ACT_STATUS ActProject::GetAvailablePriorityCodePointsForTrafficType(
    QSet<quint8> &priority_code_point_set, const ActStreamTrafficTypeEnum &traffic_type) const {
  ACT_STATUS_INIT();

  switch (traffic_type) {
    // case ActStreamTrafficTypeEnum::kNA:
    //   priority_code_point_set = this->GetTrafficTypeToPriorityCodePointMapping().GetNA();
    //   break;
    case ActStreamTrafficTypeEnum::kBestEffort:
      priority_code_point_set =
          priority_code_point_set.unite(this->GetTrafficTypeToPriorityCodePointMapping().GetBestEffort());
      break;
    case ActStreamTrafficTypeEnum::kCyclic:
      priority_code_point_set =
          priority_code_point_set.unite(this->GetTrafficTypeToPriorityCodePointMapping().GetCyclic());
    case ActStreamTrafficTypeEnum::kTimeSync:
      priority_code_point_set =
          priority_code_point_set.unite(this->GetTrafficTypeToPriorityCodePointMapping().GetTimeSync());
      break;
    default:
      qCritical() << __func__;
      qCritical() << "get an unknown traffic type (stream).";
      return std::make_shared<ActStatusInternalError>("Core");
      break;
  }

  return act_status;
}

ACT_STATUS ActProject::GetAvailablePriorityCodePointsForTrafficType(
    QSet<quint8> &priority_code_point_set, const ActTimeSlotTrafficTypeEnum &traffic_type) const {
  ACT_STATUS_INIT();

  switch (traffic_type) {
    // case ActStreamTrafficTypeEnum::kNA:
    //   priority_code_point_set = this->GetTrafficTypeToPriorityCodePointMapping().GetNA();
    //   break;
    case ActTimeSlotTrafficTypeEnum::kBestEffort:
      priority_code_point_set =
          priority_code_point_set.unite(this->GetTrafficTypeToPriorityCodePointMapping().GetBestEffort());
      break;
    case ActTimeSlotTrafficTypeEnum::kCyclic:
      priority_code_point_set =
          priority_code_point_set.unite(this->GetTrafficTypeToPriorityCodePointMapping().GetCyclic());
      break;
    case ActTimeSlotTrafficTypeEnum::kTimeSync:
      priority_code_point_set =
          priority_code_point_set.unite(this->GetTrafficTypeToPriorityCodePointMapping().GetTimeSync());
      break;
    default:
      qCritical() << __func__;
      qCritical() << "get an unknown traffic type (time slot).";
      return std::make_shared<ActStatusInternalError>("Core");
      break;
  }

  return act_status;
}

ACT_STATUS ActProject::GetAvailableQueuesForPriorityCodePoints(QSet<quint8> &available_queues,
                                                               const QSet<quint8> &priority_code_point_set) const {
  ACT_STATUS_INIT();

  QMap<quint8, QSet<quint8>> pcp_to_queue_mapping = this->GetPriorityCodePointToQueueMapping();
  for (quint8 pcp : priority_code_point_set) {
    if (!pcp_to_queue_mapping.contains(pcp)) {
      qCritical() << __func__;
      qCritical() << "There is no PCP to queue mapping for PCP" << QString::number(pcp);
      qCritical() << "the project mapping:" << this->ToString("PriorityCodePointToQueueMapping").toStdString().c_str();
      return std::make_shared<ActStatusInternalError>("Core");
    }

    QSet<quint8> queue_set = pcp_to_queue_mapping[pcp];
    if (queue_set.size() == 0) {
      qWarning() << "The PCP" << pcp << "maps no queue.";
    }

    available_queues = available_queues.unite(queue_set);
  }

  return act_status;
}

ACT_STATUS ActProject::GetAvailableQueuesForTrafficType(QSet<quint8> &available_queues,
                                                        const ActStreamTrafficTypeEnum &traffic_type) const {
  ACT_STATUS_INIT();

  // get PCP set
  QSet<quint8> priority_code_point_set;
  act_status = GetAvailablePriorityCodePointsForTrafficType(priority_code_point_set, traffic_type);
  if (!IsActStatusSuccess(act_status)) {
    // qCritical() << __func__;
    qCritical() << "GetAvailablePriorityCodePointsForTrafficType() failed";
    return act_status;
  }

  // get available queues for those PCPs
  return GetAvailableQueuesForPriorityCodePoints(available_queues, priority_code_point_set);
}

ACT_STATUS ActProject::GetAvailableQueuesForTrafficType(QSet<quint8> &available_queues,
                                                        const ActTimeSlotTrafficTypeEnum &traffic_type) const {
  ACT_STATUS_INIT();

  // get PCP set
  QSet<quint8> priority_code_point_set;
  act_status = GetAvailablePriorityCodePointsForTrafficType(priority_code_point_set, traffic_type);
  if (!IsActStatusSuccess(act_status)) {
    // qCritical() << __func__;
    qCritical() << "GetAvailablePriorityCodePointsForTrafficType() failed";
    return act_status;
  }

  // get available queues for those PCPs
  return GetAvailableQueuesForPriorityCodePoints(available_queues, priority_code_point_set);
}

void ActProject::SetUsedInterface(const qint64 &device_id, const qint64 &interface_id) {
  QSet<ActDevice>::iterator device_iter = this->GetDevices().find(ActDevice(device_id));
  if (device_iter == this->GetDevices().end()) {
    return;
  }
  ActDevice device = *device_iter;
  this->GetDevices().erase(device_iter);

  for (ActInterface &intf : device.GetInterfaces()) {
    if (intf.GetInterfaceId() == interface_id) {
      ActLink link;
      if (IsActStatusNotFound(this->GetLinkByInterfaceId(link, device_id, interface_id))) {
        intf.SetUsed(false);
      } else {
        intf.SetUsed(true);
      }
    }
  }

  this->GetDevices().insert(device);
}

ACT_STATUS ActProject::UpdateSFPList(QMap<qint64, bool> monitor_link_status) {
  ACT_STATUS_INIT();

  QSet<ActLink> link_set = this->GetLinks();
  QSet<ActMonitorFiberCheckEntry> fiber_check_entries = this->fiber_check_entries_;

  QList<ActSFPPair> sfp_list;

  // For each alive link, check if the link's source or destination is in the fiber_check_entries_
  for (ActLink link : link_set) {
    if (!monitor_link_status[link.GetId()]) {
      continue;
    }

    ActMonitorFiberCheckEntry src_entry =
        ActMonitorFiberCheckEntry(link.GetSourceDeviceId(), link.GetSourceInterfaceId());
    ActMonitorFiberCheckEntry dst_entry =
        ActMonitorFiberCheckEntry(link.GetDestinationDeviceId(), link.GetDestinationInterfaceId());

    ActSFPPair pair;
    bool src_found = false;
    bool dst_found = false;
    if (fiber_check_entries.contains(src_entry)) {
      // Fetch the source entry from the fiber_check_entries_
      typename QSet<ActMonitorFiberCheckEntry>::const_iterator iterator;
      iterator = fiber_check_entries.find(src_entry);
      pair.SetSource(*iterator);
      src_found = true;
    }

    if (fiber_check_entries.contains(dst_entry)) {
      // Fetch the destination entry from the fiber_check_entries_
      typename QSet<ActMonitorFiberCheckEntry>::const_iterator iterator;
      iterator = fiber_check_entries.find(dst_entry);
      pair.SetTarget(*iterator);
      dst_found = true;
    }

    if (src_found && dst_found) {
      sfp_list.append(pair);
    }
  }

  sfp_list_.SetSFPList(sfp_list);

  return act_status;
}
