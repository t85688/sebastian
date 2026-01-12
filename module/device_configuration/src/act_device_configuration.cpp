#include "act_device_configuration.hpp"

#include <QDebug>

ActDeviceConfiguration::~ActDeviceConfiguration() {
  if ((device_config_thread_ != nullptr) && (device_config_thread_->joinable())) {
    device_config_thread_->join();
  }
}

ACT_STATUS ActDeviceConfiguration::GetStatus() {
  if (IsActStatusSuccess(device_config_act_status_) && (progress_ == 100)) {
    device_config_act_status_->SetStatus(ActStatusType::kFinished);
  }

  if ((!IsActStatusRunning(device_config_act_status_)) && (!IsActStatusFinished(device_config_act_status_))) {
    // failed
    return device_config_act_status_;
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*device_config_act_status_), progress_);
}

ACT_STATUS ActDeviceConfiguration::UpdateProgress(quint8 progress) {
  ACT_STATUS_INIT();
  progress_ = progress;
  qDebug() << __func__ << QString("Progress: %1%.").arg(GetProgress()).toStdString().c_str();
  return act_status;
}

ACT_STATUS ActDeviceConfiguration::Stop() {
  // Checking has the thread is running
  if (IsActStatusRunning(device_config_act_status_)) {
    qDebug() << "Stop DeviceConfiguration's running thread.";

    southbound_.SetStopFlag(true);

    // Send the stop signal and wait for the thread to finish.
    stop_flag_ = true;

    if ((device_config_thread_ != nullptr) && (device_config_thread_->joinable())) {
      device_config_thread_->join();  // wait thread finished
    }
  } else {
    qDebug() << __func__ << "The DeviceConfiguration's thread not running.";
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*device_config_act_status_), progress_);
}

ACT_STATUS ActDeviceConfiguration::DeviceConfigurationErrorHandler(QString called_func, const QString &error_reason,
                                                                   const ActDevice &device) {
  qCritical() << called_func.toStdString().c_str()
              << QString("Device: %1(%2): %3")
                     .arg(device.GetIpv4().GetIpAddress())
                     .arg(device.GetId())
                     .arg(error_reason)
                     .toStdString()
                     .c_str();

  result_queue_.enqueue(ActDeviceConfigureResult(device.GetId(), progress_, ActStatusType::kFailed, error_reason));

  return std::make_shared<ActStatusInternalError>("DeviceConfiguration");
}

ACT_STATUS ActDeviceConfiguration::UpdateDeviceFirmwareFeatureProfileId(ActDevice &dev) {
  ACT_STATUS_INIT();

  // Try to get FirmwareVersion to find firmware_feature_profile
  ActFeatureSubItem fw_feature_sub_item;
  act_status = GetDeviceFeatureSubItem(dev, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kAutoScan, "Identify", "FirmwareVersion", fw_feature_sub_item);
  if (IsActStatusSuccess(act_status)) {
    QString firmware_version;
    act_status = southbound_.ActionGetFirmwareVersion(dev, fw_feature_sub_item, firmware_version);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
    // Try to find firmware_feature_profile
    ActFirmwareFeatureProfile fw_feat_profile;
    auto find_status = ActFirmwareFeatureProfile::GetFirmwareFeatureProfile(profiles_.GetFirmwareFeatureProfiles(),
                                                                            dev.GetDeviceProperty().GetModelName(),
                                                                            firmware_version, fw_feat_profile);
    if (IsActStatusSuccess(find_status)) {
      dev.SetFirmwareFeatureProfileId(fw_feat_profile.GetId());
    }
  }

  return act_status;
}

ACT_STATUS ActDeviceConfiguration::CheckProjectMonitorEndpoint(const ActProject &project) {
  ACT_STATUS_INIT();

  ActDevice dev;

  auto src_dev_id = project.monitor_endpoint_.GetDeviceId();
  if (src_dev_id == -1) {
    return std::make_shared<ActStatusInternalError>("CheckProjectMonitorEndpoint");
  }

  act_status = project.GetDeviceById(dev, src_dev_id);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Assign Device LLDP chassis ID
  act_status = southbound_.AssignDeviceLldpData(dev);
  if (!IsActStatusSuccess(act_status)) {
    qWarning() << __func__ << "AssignDeviceLldpData() failed.";
    return act_status;
  }

  // Assign Device MAC table
  act_status = southbound_.AssignDeviceMacTable(dev);
  if (!IsActStatusSuccess(act_status)) {
    qWarning() << __func__ << "AssignDeviceLldpData() failed.";
    return act_status;
  }

  // Get SourceDevice(LLDP or MAC table)
  ActSourceDevice south_src_dev;
  act_status = southbound_.FindSourceDeviceByLLDPAndMacTable({dev}, south_src_dev);
  if (!IsActStatusSuccess(act_status)) {
    qWarning() << __func__ << "Find online source Device failed.";
    return act_status;
  }

  // Check Source Device
  if (src_dev_id != south_src_dev.GetDeviceId()) {
    return std::make_shared<ActStatusInternalError>("CheckProjectMonitorEndpoint");
  }

  return act_status;
}

ACT_STATUS ActDeviceConfiguration::SortDeviceIdList(const ActProject &project, QList<qint64> &dev_id_list) {
  ACT_STATUS_INIT();

  ACT_STATUS compute_status = std::make_shared<ActStatusBase>();
  QList<qint64> select_dev_id_list = dev_id_list;
  QList<qint64> new_dev_id_list;
  QList<ActDeviceDistanceEntry> distance_entry_list;
  QList<ActDevice> dev_list = project.GetDevices().values();
  QList<ActLink> link_list = project.GetLinks().values();
  act::topology::ActBroadcastSearch broadcast(profiles_);

  auto check_status = CheckProjectMonitorEndpoint(project);
  if (IsActStatusSuccess(check_status)) {
    // Use the EndPoint & Project's topology to detect the distance
    qint64 src_dev_id = project.monitor_endpoint_.GetDeviceId();
    compute_status = broadcast.ComputeDeviceDistance(dev_list, link_list, src_dev_id);
    if (IsActStatusSuccess(compute_status)) {
      // Append ActDeviceDistanceEntry
      for (auto dev : dev_list) {
        distance_entry_list.append(ActDeviceDistanceEntry(dev.GetId(), dev.GetDistance(), dev.mac_address_int));
      }
      // Sort result by distance from far to near
      std::sort(distance_entry_list.begin(), distance_entry_list.end());
    }
  } else {
    // Scan the physical topology to detect the distance
    broadcast.SetDevices(dev_list);
    bool from_broadcast_search = false;
    compute_status = broadcast.LinkDistanceDetect(from_broadcast_search, select_dev_id_list, distance_entry_list);
  }

  if (!IsActStatusSuccess(compute_status)) {
    qWarning() << __func__
               << "Detect distance failed, that the configure sequence uses the IP num (from small to large)";
    distance_entry_list.clear();
    // Sort by IP num (from small to large)
    quint32 ip_num;
    for (auto dev : dev_list) {
      if (select_dev_id_list.contains(dev.GetId())) {
        dev.GetIpv4().GetIpAddressNumber(ip_num);
        distance_entry_list.append(ActDeviceDistanceEntry(dev.GetId(), ip_num, dev.mac_address_int));
      }
    }
    // rsort
    std::sort(distance_entry_list.rbegin(), distance_entry_list.rend());
  }

  // Generate sorted Device ID list
  for (auto dist_entry : distance_entry_list) {
    if (select_dev_id_list.contains(dist_entry.GetId())) {
      new_dev_id_list.append(dist_entry.GetId());
    }
  }
  dev_id_list = new_dev_id_list;

  return ACT_STATUS_SUCCESS;
}