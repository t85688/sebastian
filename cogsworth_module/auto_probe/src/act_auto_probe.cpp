#include "act_auto_probe.hpp"

#include "act_utilities.hpp"

act::auto_probe::ActAutoProbe::~ActAutoProbe() {
  if ((auto_probe_thread_ != nullptr) && (auto_probe_thread_->joinable())) {
    auto_probe_thread_->join();
  }
}

ACT_STATUS act::auto_probe::ActAutoProbe::GetStatus() {
  if (IsActStatusSuccess(auto_probe_act_status_) && (progress_ == 100)) {
    auto_probe_act_status_->SetStatus(ActStatusType::kFinished);
  }

  if ((!IsActStatusRunning(auto_probe_act_status_)) && (!IsActStatusFinished(auto_probe_act_status_))) {
    // failed
    return auto_probe_act_status_;
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*auto_probe_act_status_), progress_);
}

ACT_STATUS act::auto_probe::ActAutoProbe::UpdateProgress(quint8 progress) {
  ACT_STATUS_INIT();
  progress_ = progress;
  qDebug() << __func__ << QString("Progress: %1%.").arg(GetProgress()).toStdString().c_str();
  return act_status;
}

ACT_STATUS act::auto_probe::ActAutoProbe::Stop() {
  // Checking has the thread is running
  if (IsActStatusRunning(auto_probe_act_status_)) {
    qDebug() << "Stop AutoProbe's running thread.";

    southbound_.SetStopFlag(true);

    // Send the stop signal and wait for the thread to finish.
    stop_flag_ = true;
    if ((auto_probe_thread_ != nullptr) && (auto_probe_thread_->joinable())) {
      auto_probe_thread_->join();  // wait thread finished
    }
  } else {
    qDebug() << __func__ << "The AutoProbe's thread not running.";
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*auto_probe_act_status_), progress_);
}

ACT_STATUS act::auto_probe::ActAutoProbe::AutoProbeErrorHandler(const QString &error_fun, const QString &error_reason,
                                                                const ActDevice &device) {
  qCritical() << QString("%1 > %2. Device: %3(%4)")
                     .arg(error_fun)
                     .arg(error_reason)
                     .arg(device.GetIpv4().GetIpAddress())
                     .arg(device.GetId())
                     .toStdString()
                     .c_str();

  return std::make_shared<ActStatusInternalError>("AutoProbe");
}

ACT_STATUS act::auto_probe::ActAutoProbe::NotFoundMethodErrorHandler(QString called_func, ActActionMethod method) {
  qCritical()
      << called_func.toStdString().c_str()
      << QString("Method not found. Method(%1): %2").arg(method.GetKey()).arg(method.ToString()).toStdString().c_str();

  return std::make_shared<ActStatusNotFound>(QString("Method(%1)").arg(method.GetKey()));
}

ACT_STATUS act::auto_probe::ActAutoProbe::StartAutoProbe(ActDevice &device, ActAutoProbeWarning &result_probe_warning,
                                                         ActDeviceProfile &result_dev_profile) {
  // Checking has the thread is running
  if (IsActStatusRunning(auto_probe_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("AutoProbe");
  }

  // init ActAutoProbe status
  progress_ = 0;
  stop_flag_ = false;
  auto_probe_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to start AutoProbe
  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((auto_probe_thread_ != nullptr) && (auto_probe_thread_->joinable())) {
      auto_probe_thread_->join();
    }
    auto_probe_act_status_->SetStatus(ActStatusType::kRunning);
    auto_probe_thread_ =
        std::make_unique<std::thread>(&act::auto_probe::ActAutoProbe::TriggerAutoProbeForThread, this, std::ref(device),
                                      std::ref(result_probe_warning), std::ref(result_dev_profile));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"AutoProbeThread";
    HRESULT hr = SetThreadDescription(auto_probe_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    qDebug() << "Start AutoProbe thread.";

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(AutoProbe) failed. Error:" << e.what();
    auto_probe_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("AutoProbe");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*auto_probe_act_status_), progress_);
}

void act::auto_probe::ActAutoProbe::TriggerAutoProbeForThread(ActDevice &device,
                                                              ActAutoProbeWarning &result_probe_warning,
                                                              ActDeviceProfile &result_dev_profile) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the AutoProbe and wait for the return, and update auto_probe_thread_.
  try {
    auto_probe_act_status_ = AutoProbe(device, result_probe_warning, result_dev_profile);

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(interrupt) failed. Error:" << e.what();
    auto_probe_act_status_ = std::make_shared<ActStatusInternalError>("AutoProbe");
  }
}

ACT_STATUS act::auto_probe::ActAutoProbe::StartAutoProbeByScanIpRange(QList<ActScanIpRangeEntry> &scan_ip_ranges) {
  // Checking has the thread is running
  if (IsActStatusRunning(auto_probe_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("AutoProbeByScanIpRange");
  }

  // init ActAutoProbeByScanIpRange status
  progress_ = 0;
  stop_flag_ = false;
  auto_probe_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to start AutoProbeByScanIpRange
  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((auto_probe_thread_ != nullptr) && (auto_probe_thread_->joinable())) {
      auto_probe_thread_->join();
    }
    auto_probe_act_status_->SetStatus(ActStatusType::kRunning);
    auto_probe_thread_ = std::make_unique<std::thread>(
        &act::auto_probe::ActAutoProbe::TriggerAutoProbeByScanIpRangeForThread, this, std::ref(scan_ip_ranges));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"AutoProbeThread";
    HRESULT hr = SetThreadDescription(auto_probe_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    qDebug() << "Start AutoProbeByScanIpRange thread.";

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(AutoProbeByScanIpRange) failed. Error:" << e.what();
    auto_probe_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("AutoProbeByScanIpRange");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*auto_probe_act_status_), progress_);
}

void act::auto_probe::ActAutoProbe::TriggerAutoProbeByScanIpRangeForThread(QList<ActScanIpRangeEntry> &scan_ip_ranges) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the AutoProbeByScanIpRange and wait for the return, and update auto_probe_thread_.
  try {
    auto_probe_act_status_ = AutoProbeByScanIpRange(scan_ip_ranges);

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(interrupt) failed. Error:" << e.what();
    auto_probe_act_status_ = std::make_shared<ActStatusInternalError>("AutoProbeByScanIpRange");
  }
}

ACT_STATUS act::auto_probe::ActAutoProbe::AutoProbeByScanIpRange(QList<ActScanIpRangeEntry> &scan_ip_ranges) {
  ACT_STATUS_INIT();

  qint64 new_dev_profile_id = 10000;

  // Scan devices
  QSet<ActDevice> alive_devices;
  act_status = southbound_.ScanDevicesByScanIpRange(scan_ip_ranges, alive_devices);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanDevicesByScanIpRange() failed.";
    return act_status;
  }

  // Assign devices Moxa vendor by BroadcastSearch
  act_status = southbound_.FeatureAssignDevicesMoxaVendorByBroadcastSearch(alive_devices);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "FeatureAssignDevicesMoxaVendorByBroadcastSearch() failed.";
    return act_status;
  }

  for (auto dev : alive_devices) {
    ActAutoProbeWarning probe_warning;
    ActDeviceProfile probe_dev_profile;

    // Start probe each device
    auto act_status_probe = AutoProbe(dev, probe_warning, probe_dev_profile);
    qDebug() << __func__
             << QString("Device ProbeFeature Warning Report: %1").arg(probe_warning.ToString()).toStdString().c_str();

    // Check Success
    if (!IsActStatusSuccess(act_status_probe)) {
      result_queue_.enqueue(
          ActAutoProbeResult(dev.GetIpv4().GetIpAddress(), probe_dev_profile, probe_warning, ActStatusType::kFailed));
      continue;
    }

    // Check ModelName is empty (Uncertified-Device)
    if (probe_dev_profile.GetModelName().isEmpty()) {
      result_queue_.enqueue(
          ActAutoProbeResult(dev.GetIpv4().GetIpAddress(), probe_dev_profile, probe_warning, ActStatusType::kFailed));
      continue;
    }

    // Check ICMP DeviceProfile Id
    if (probe_dev_profile.GetId() == ACT_ICMP_DEVICE_PROFILE_ID) {
      result_queue_.enqueue(
          ActAutoProbeResult(dev.GetIpv4().GetIpAddress(), probe_dev_profile, probe_warning, ActStatusType::kFailed));
      continue;
    }

    if (!profiles_.GetDeviceProfiles().contains(probe_dev_profile)) {
      result_queue_.enqueue(
          ActAutoProbeResult(dev.GetIpv4().GetIpAddress(), probe_dev_profile, probe_warning, ActStatusType::kSuccess));

      // Assign temporarily DeviceProfile ID
      // Get Unique ID
      while (true) {
        ActDeviceProfile target_dev_profile;
        if (!IsActStatusSuccess(ActGetItemById<ActDeviceProfile>(profiles_.GetDeviceProfiles(), new_dev_profile_id,
                                                                 target_dev_profile))) {
          break;
        }
        new_dev_profile_id += 1;
      }
      probe_dev_profile.SetId(new_dev_profile_id);
      profiles_.GetDeviceProfiles().insert(probe_dev_profile);

    } else {
      result_queue_.enqueue(
          ActAutoProbeResult(dev.GetIpv4().GetIpAddress(), probe_dev_profile, probe_warning, ActStatusType::kSkip));
    }
  }
  UpdateProgress(100);
  return act_status;
}

ACT_STATUS act::auto_probe::ActAutoProbe::AutoProbe(ActDevice &device, ActAutoProbeWarning &result_probe_warning,
                                                    ActDeviceProfile &result_dev_profile) {
  ACT_STATUS_INIT();

  reidentify_ = false;
  // reidentify_ = (device.GetDeviceProfileId() == -1) ? false : true;
  // qDebug() << __func__ << "Re-Identify Device:" << reidentify_;

  // Init WarningReport
  result_probe_warning.SetDeviceId(device.GetId());
  result_probe_warning.SetIpAddress(device.GetIpv4().GetIpAddress());

  UpdateProgress(10);

  // [feat:2396] Refactor - AutoScan performance enhance
  ActDeviceProfile dev_profile;
  ActFirmwareFeatureProfile fw_feature_profile;
  auto identify_act_status = southbound_.FeatureIdentifyDeviceAndGetProfilesByProbe(!reidentify_, device, profiles_,
                                                                                    dev_profile, fw_feature_profile);

  if (reidentify_) {
    if ((device.GetDeviceProperty().GetModelName() != dev_profile.GetModelName()) ||
        (dev_profile.GetId() == ACT_ICMP_DEVICE_PROFILE_ID)) {  // disable all features
      device.DisableAllFeatures();
      result_dev_profile.SetId(device.GetDeviceProfileId());
      UpdateProgress(100);
      return identify_act_status;
    }
  }

  if ((!IsActStatusSuccess(identify_act_status)) || (dev_profile.GetId() == ACT_ICMP_DEVICE_PROFILE_ID)) {
    return AutoProbeErrorHandler(__func__, "FeatureIdentifyDeviceAndGetProfilesByProbe() Failed", device);
  }

  qDebug() << __func__
           << QString("FeatureIdentifyDeviceAndGetProfilesByProbe() result. Device: %1, DeviceProfileID: %2")
                  .arg(device.GetIpv4().GetIpAddress())
                  .arg(dev_profile.GetId())
                  .toStdString()
                  .c_str();

  // Return DeviceProfile if it exists at ACT.
  if (profiles_.GetDeviceProfiles().contains(dev_profile)) {
    qDebug() << __func__
             << QString("System DeviceProfiles already exist DeviceProfile(%1)")
                    .arg(dev_profile.GetId())
                    .toStdString()
                    .c_str();

    result_dev_profile = dev_profile;
    UpdateProgress(100);
    return act_status;
  }

  // Update Device connect status(RESTful, NETCONF, NewMOXACommand)
  ActDeviceConnectStatusControl connect_status_control(
      true, false, true, true);  // (RESTful(true), SNMP(false), NETCONF(true), NewMOXACommand(true))

  act_status = southbound_.FeatureAssignDeviceStatus(true, device, connect_status_control);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Add FeatureCapability to DeviceProfile(ProbeFeatures)
  qDebug() << "Start append the DeviceProfile's FeatureCapability(Probe features)";
  act_status = ProbeFeatures(device, dev_profile, result_probe_warning);
  if (IsActStatusStop(act_status)) {  // stop handle
    return act_status;
  }
  if (!IsActStatusSuccess(act_status)) {
    return AutoProbeErrorHandler(__func__, "ProbeFeatures() Failed", device);
  }

  // Get DeviceProfileInfos & edit DeviceProfile
  act_status = GetDeviceProfileInfos(device, dev_profile);
  if (IsActStatusStop(act_status)) {  // stop handle
    return act_status;
  }

  // Generate DeviceProfile's FeatureGroup
  GenerateFeatureGroup(dev_profile);

  // Assign result_dev_profile
  result_dev_profile = dev_profile;

  UpdateProgress(100);
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS act::auto_probe::ActAutoProbe::GetDeviceProfileInfos(const ActDevice &device,
                                                                ActDeviceProfile &device_profile) {
  ACT_STATUS_INIT();

  // DeviceName
  ActFeatureSubItem device_name_sub_item;
  act_status = GetDeviceProfileFeatureSubItem(device.GetDeviceProfileId(), {device_profile}, ActFeatureEnum::kAutoScan,
                                              "DeviceInformation", "DeviceName", device_name_sub_item);
  // If has this sub-item would access it
  if (IsActStatusSuccess(act_status)) {
    QString device_name;
    act_status = southbound_.ActionGetDeviceName(device, device_name_sub_item, device_name);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "ActionGetDeviceName() failed.";
    } else {
      device_profile.SetDeviceName(device_name);
      qDebug() << __func__ << QString("Set DeviceName: %1").arg(device_name).toStdString().c_str();
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Interface
  // If Interfaces is empty, would probe it
  if (device_profile.GetInterfaces().isEmpty()) {
    // InterfaceName
    QMap<qint64, QString> if_name_map;
    ActFeatureSubItem if_name_sub_item;
    act_status =
        GetDeviceProfileFeatureSubItem(device.GetDeviceProfileId(), {device_profile}, ActFeatureEnum::kAutoScan,
                                       "DeviceInformation", "InterfaceName", if_name_sub_item);
    // If has this sub-item would access it
    if (IsActStatusSuccess(act_status)) {
      act_status = southbound_.ActionGetInterfaceName(device, if_name_sub_item, if_name_map);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetInterfaceName() failed.";
      } else {
        QMapIterator<qint64, QString> map_it(if_name_map);
        QList<ActInterfaceProperty> if_list;
        while (map_it.hasNext()) {
          map_it.next();
          ActInterfaceProperty new_if_property(map_it.key());  // id
          new_if_property.SetInterfaceName(map_it.value());

          // Append
          if_list.append(new_if_property);
        }
        device_profile.SetInterfaces(if_list);
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS act::auto_probe::ActAutoProbe::GenerateFeatureGroup(ActDeviceProfile &device_profile) {
  ACT_STATUS_INIT();

  ActFeatureGroup feature_group;

  ActFeatureSubItem feature_sub_item;

  // AutoScan > BroadcastSearch
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kAutoScan,
                                              "BroadcastSearch", "Basic", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetAutoScan().GetBroadcastSearch() = true;
  }

  // AutoScan > Identify > ModelName
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kAutoScan,
                                              "Identify", "ModelName", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetAutoScan().GetIdentify().GetModelName() = true;
  }

  // AutoScan > Identify > VendorID
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kAutoScan,
                                              "Identify", "VendorID", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetAutoScan().GetIdentify().GetVendorID() = true;
  }

  // AutoScan > LLDP
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kAutoScan,
                                              "LLDP", "Basic", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetAutoScan().GetLLDP() = true;
  }

  // AutoScan > DeviceInformation > DeviceName
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kAutoScan,
                                              "DeviceInformation", "DeviceName", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetAutoScan().GetDeviceInformation().GetDeviceName() = true;
  }

  // AutoScan > DeviceInformation > MACTable
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kAutoScan,
                                              "DeviceInformation", "MACTable", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetAutoScan().GetDeviceInformation().GetMACTable() = true;
  }

  // AutoScan > DeviceInformation > InterfaceName
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kAutoScan,
                                              "DeviceInformation", "InterfaceName", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetAutoScan().GetDeviceInformation().GetInterfaceName() = true;
  }

  // TODO: remove
  // [feat:3661] Remove fake MAC
  // act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kAutoScan,
  //                                             "DeviceInformation", "InterfaceMAC", feature_sub_item);
  // if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
  //   feature_group.GetAutoScan().GetDeviceInformation().GetInterfaceMAC() = true;
  // }

  // AutoScan > DeviceInformation > PortSpeed
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kAutoScan,
                                              "DeviceInformation", "PortSpeed", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetAutoScan().GetDeviceInformation().GetPortSpeed() = true;
  }

  // Operation > Reboot
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kOperation,
                                              "Reboot", "Basic", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetOperation().GetReboot() = true;
  }

  // Operation > FactoryDefault
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kOperation,
                                              "FactoryDefault", "Basic", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetOperation().GetFactoryDefault() = true;
  }

  // Operation > FirmwareUpgrade
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kOperation,
                                              "FirmwareUpgrade", "Basic", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetOperation().GetFirmwareUpgrade() = true;
  }

  // Operation > EnableSNMPService
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kOperation,
                                              "EnableSNMPService", "Basic", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetOperation().GetEnableSNMPService() = true;
  }

  // Configuration > NetworkSetting
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kConfiguration,
                                              "NetworkSetting", "Basic", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetConfiguration().GetNetworkSetting() = true;
  }

  // Configuration > VLANSetting > VLANMethod
  bool vlan_method_active = false;
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kConfiguration,
                                              "VLANSetting", "VLANMethod", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    vlan_method_active = true;
  }

  // Configuration > VLANSetting > AccessTrunkMode(Base on VLANMethod)
  if (vlan_method_active) {
    act_status =
        GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kConfiguration,
                                       "VLANSetting", "AccessTrunkMode", feature_sub_item);
    if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
      feature_group.GetConfiguration().GetVLANSetting().GetAccessTrunkMode() = true;
    }
  }

  // Configuration > VLANSetting > HybridMode(Base on VLANMethod & AccessTrunkMode)
  if (vlan_method_active && feature_group.GetConfiguration().GetVLANSetting().GetAccessTrunkMode()) {
    feature_group.GetConfiguration().GetVLANSetting().GetHybridMode() = true;
  }

  // Configuration > VLANSetting > TEMSTID(Base on VLANMethod)
  if (vlan_method_active) {
    act_status =
        GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kConfiguration,
                                       "VLANSetting", "TEMSTID", feature_sub_item);
    if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
      feature_group.GetConfiguration().GetVLANSetting().GetTEMSTID() = true;
    }
  }

  // Configuration > VLANSetting > DefaultPVID(Base on VLANMethod)
  if (vlan_method_active) {
    act_status =
        GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kConfiguration,
                                       "VLANSetting", "DefaultPVID", feature_sub_item);
    if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
      feature_group.GetConfiguration().GetVLANSetting().GetDefaultPVID() = true;
    }
  }

  // Configuration > VLANSetting > DefaultPCP(Base on VLANMethod)
  if (vlan_method_active) {
    act_status =
        GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kConfiguration,
                                       "VLANSetting", "DefaultPCP", feature_sub_item);
    if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
      feature_group.GetConfiguration().GetVLANSetting().GetDefaultPCP() = true;
    }
  }

  // Configuration > VLANSetting > PerStreamPriority(Base on VLANMethod)
  if (vlan_method_active) {
    act_status =
        GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kConfiguration,
                                       "VLANSetting", "PerStreamPriority", feature_sub_item);
    if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
      feature_group.GetConfiguration().GetVLANSetting().GetPerStreamPriority() = true;
    }
  }

  // Configuration > StaticForwardSetting > Unicast(Base on VLANMethod)
  if (vlan_method_active) {
    act_status =
        GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kConfiguration,
                                       "StaticForwardSetting", "Unicast", feature_sub_item);
    if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
      feature_group.GetConfiguration().GetStaticForwardSetting().GetUnicast() = true;
    }
  }

  // Configuration > StaticForwardSetting > Multicast(Base on VLANMethod)
  if (vlan_method_active) {
    act_status =
        GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kConfiguration,
                                       "StaticForwardSetting", "Multicast", feature_sub_item);
    if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
      feature_group.GetConfiguration().GetStaticForwardSetting().GetMulticast() = true;
    }
  }

  // Configuration > STPRSTP > RSTPMethod
  bool rstp_method_active = false;
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kConfiguration,
                                              "STPRSTP", "RSTPMethod", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    rstp_method_active = true;
  }

  // Configuration > STPRSTP > RootGuard(Base on RSTPMethod)
  if (rstp_method_active) {
    act_status =
        GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kConfiguration,
                                       "STPRSTP", "RootGuard", feature_sub_item);
    if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
      feature_group.GetConfiguration().GetSTPRSTP().GetRootGuard() = true;
    }
  }

  // Configuration > TSN > IEEE802Dot1Qbv
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kConfiguration,
                                              "TSN", "IEEE802Dot1Qbv", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetConfiguration().GetTSN().GetIEEE802Dot1Qbv() = true;
  }

  // Configuration > TSN > IEEE802Dot1CB
  act_status = GetDeviceProfileFeatureSubItem(device_profile.GetId(), {device_profile}, ActFeatureEnum::kConfiguration,
                                              "TSN", "IEEE802Dot1CB", feature_sub_item);
  if (IsActStatusSuccess(act_status) && (!feature_sub_item.GetMethods().isEmpty())) {
    feature_group.GetConfiguration().GetTSN().GetIEEE802Dot1CB() = true;
  }

  device_profile.SetFeatureGroup(feature_group);
  return ACT_STATUS_SUCCESS;
}
