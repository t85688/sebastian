#include "act_device_configuration.hpp"
#include "deploy_entry/act_deploy_table.hpp"

ACT_STATUS ActDeviceConfiguration::StartSetNetworkSetting(const ActProject &act_project,
                                                          QList<ActDeviceIpConfiguration> dev_ip_config_list,
                                                          bool from_broadcast_search) {
  // Checking has the thread is running
  if (IsActStatusRunning(device_config_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("DeviceConfiguration");
  }

  // init ActDeviceConfiguration status
  progress_ = 0;
  stop_flag_ = false;
  device_config_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to start SetNetworkSetting
  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((device_config_thread_ != nullptr) && (device_config_thread_->joinable())) {
      device_config_thread_->join();
    }
    device_config_act_status_->SetStatus(ActStatusType::kRunning);
    device_config_thread_ =
        std::make_unique<std::thread>(&ActDeviceConfiguration::TriggerSetNetworkSettingForThread, this,
                                      std::cref(act_project), dev_ip_config_list, from_broadcast_search);

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggerSetNetworkSettingForThread";
    HRESULT hr = SetThreadDescription(device_config_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    qDebug() << "Start SetNetworkSetting thread.";

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(SetNetworkSetting) failed. Error:" << e.what();
    device_config_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("DeviceConfiguration");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*device_config_act_status_), progress_);
}

void ActDeviceConfiguration::TriggerSetNetworkSettingForThread(const ActProject &act_project,
                                                               QList<ActDeviceIpConfiguration> dev_ip_config_list,
                                                               bool from_broadcast_search) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the SetNetworkSetting and wait for the return, and update device_config_thread_.
  try {
    device_config_act_status_ = SetNetworkSetting(act_project, dev_ip_config_list, from_broadcast_search);

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(interrupt) failed. Error:" << e.what();
    device_config_act_status_ = std::make_shared<ActStatusInternalError>("DeviceConfiguration");
  }
}

ACT_STATUS ActDeviceConfiguration::SetNetworkSetting(const ActProject &act_project,
                                                     const QList<ActDeviceIpConfiguration> &dev_ip_config_list,
                                                     const bool &from_broadcast_search) {
  ACT_STATUS_INIT();

  UpdateProgress(10);

  // Try to update MacHostMap
  if (from_broadcast_search && mac_host_map_.isEmpty()) {
    act::core::g_core.GetMacHostMap(mac_host_map_);
  }

  // Generate device list
  QList<ActDevice> dev_list;
  for (auto dev_ip_config : dev_ip_config_list) {
    // Generate device
    ActDevice dev(dev_ip_config.GetId());
    dev.SetMacAddress(dev_ip_config.GetMacAddress());

    // Set MAC int
    qint64 mac_int = 0;
    MacAddressToQInt64(dev.GetMacAddress(), mac_int);
    dev.mac_address_int = mac_int;

    ActIpv4 ipv4(dev_ip_config.GetOriginIp());
    dev.SetIpv4(ipv4);
    dev.SetAccount(dev_ip_config.GetAccount());
    dev.SetSnmpConfiguration(dev_ip_config.GetSnmpConfiguration());
    dev.SetNetconfConfiguration(dev_ip_config.GetNetconfConfiguration());
    dev.SetRestfulConfiguration(dev_ip_config.GetRestfulConfiguration());
    dev.SetEnableSnmpSetting(dev_ip_config.GetEnableSnmpSetting());

    dev_list.append(dev);
  }

  // Update Devices ICMP status to check alive(not from_broadcast_search)
  if (!from_broadcast_search) {
    southbound_.UpdateDevicesIcmpStatus(dev_list);
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }
  }

  for (auto dev : dev_list) {
    if (stop_flag_) {  // stop flag
      return ACT_STATUS_STOP;
    }
    act_status = ACT_STATUS_SUCCESS;

    // Check ICMP status(not from_broadcast_search)
    if (!from_broadcast_search) {
      if (!dev.GetDeviceStatus().GetICMPStatus()) {
        DeviceConfigurationErrorHandler(__func__, "ICMP status is false(not alive)", dev);
        continue;
      }
    }

    // Update connect status to true for southbound
    dev.GetDeviceStatus().SetAllConnectStatus(true);

    // Find dev_ip_config
    qint32 dev_ip_config_index = -1;
    act_status =
        ActGetListItemIndexById<ActDeviceIpConfiguration>(dev_ip_config_list, dev.GetId(), dev_ip_config_index);
    if (!IsActStatusSuccess(act_status)) {
      DeviceConfigurationErrorHandler(__func__, "Device's DeviceIpConfiguration not found", dev);
      continue;
    }
    ActDeviceIpConfiguration dev_ip_config = dev_ip_config_list.at(dev_ip_config_index);

    // Generate network_setting_table
    ActNetworkSettingTable network_setting_table(dev.GetId(), dev_ip_config.GetNewIp());
    if (dev_ip_config.GetSubnetMask().isEmpty()) {
      dev_ip_config.SetSubnetMask("255.255.255.0");
    }
    network_setting_table.SetSubnetMask(dev_ip_config.GetSubnetMask());
    network_setting_table.SetGateway(dev_ip_config.GetGateway());
    network_setting_table.SetDNS1(dev_ip_config.GetDNS1());
    network_setting_table.SetDNS2(dev_ip_config.GetDNS2());

    // Set ArpTable
    if (from_broadcast_search) {  // has mac
      // Check Map has device's mac
      if (!mac_host_map_.contains(dev.GetMacAddress())) {  // hasn't
        DeviceConfigurationErrorHandler(__func__, "Device's MAC not found in mac_host_map_", dev);
        continue;
      }

      QString host = mac_host_map_[dev.GetMacAddress()];
      // auto msg = QString("SetLocalhostArpTable: dev: %1(%2, %3), host: %4")
      //                .arg(dev.GetId())
      //                .arg(dev.GetIpv4().GetIpAddress())
      //                .arg(dev.GetMacAddress())
      //                .arg(host);
      // qDebug() << __func__ << msg.toStdString().c_str();
      act_status = southbound_.SetLocalhostArpTable(dev, host);
      if (!IsActStatusSuccess(act_status)) {
        DeviceConfigurationErrorHandler(__func__, "Add the ARP entry failed", dev);
        continue;
      }
    }

    // Get NetworkSetting sub-item
    ActFeatureSubItem feature_sub_item;
    if (from_broadcast_search) {
      // Use FeatureProfiles
      act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kConfiguration,
                                                   "NetworkSetting", "Basic", feature_sub_item);
      if (!IsActStatusSuccess(act_status)) {
        DeviceConfigurationErrorHandler(__func__, act_status->GetErrorMessage(), dev);
        continue;
      }

    } else {
      // Update Device from project's device
      ActDevice proj_dev;
      act_status = act_project.GetDeviceById(proj_dev, dev.GetId());
      if (!IsActStatusSuccess(act_status)) {
        DeviceConfigurationErrorHandler(__func__, "Get device failed", dev);
        continue;
      }
      dev = proj_dev;

      // Try to get FirmwareVersion to find firmware_feature_profile
      // act_status = GetDeviceFeatureSubItem(dev, profiles_.GetFirmwareFeatureProfiles(),
      // profiles_.GetDeviceProfiles(),
      //                                      ActFeatureEnum::kAutoScan, "Identify", "FirmwareVersion",
      //                                      feature_sub_item);
      // if (IsActStatusSuccess(act_status)) {
      //   QString firmware_version;
      //   act_status = southbound_.ActionGetFirmwareVersion(dev, feature_sub_item, firmware_version);
      //   if (!IsActStatusSuccess(act_status)) {
      //     DeviceConfigurationErrorHandler(__func__, "Get device Firmware version failed", dev);
      //     continue;
      //   }
      //   // Try to find firmware_feature_profile
      //   ActFirmwareFeatureProfile fw_feat_profile;
      //   auto find_status =
      //   ActFirmwareFeatureProfile::GetFirmwareFeatureProfile(profiles_.GetFirmwareFeatureProfiles(),
      //                                                                           dev.GetDeviceProperty().GetModelName(),
      //                                                                           firmware_version, fw_feat_profile);
      //   if (IsActStatusSuccess(find_status)) {
      //     dev.SetFirmwareFeatureProfileId(fw_feat_profile.GetId());
      //   }
      // }

      act_status = GetDeviceFeatureSubItem(dev, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                           ActFeatureEnum::kConfiguration, "NetworkSetting", "Basic", feature_sub_item);
      if (!IsActStatusSuccess(act_status)) {
        DeviceConfigurationErrorHandler(__func__, "Get device feature item failed", dev);
        continue;
      }
    }

    // Set IP config
    act_status = southbound_.ActionSetNetworkSetting(dev, feature_sub_item, network_setting_table);
    if (!IsActStatusSuccess(act_status)) {
      DeviceConfigurationErrorHandler(__func__, act_status->GetErrorMessage(), dev);
      if (from_broadcast_search) {
        southbound_.DeleteLocalhostArpEntry(dev);
      }
      continue;
    }

    // Delete Arp entry
    if (from_broadcast_search) {
      act_status = southbound_.DeleteLocalhostArpEntry(dev);
      if (!IsActStatusSuccess(act_status)) {
        DeviceConfigurationErrorHandler(__func__, "Delete the ARP entry Failed", dev);
        continue;
      }
    }

    // Add success result to result_queue_
    result_queue_.enqueue(ActDeviceConfigureResult(dev.GetId(), ActStatusType::kSuccess));

    // Update progress(10~90/100)
    quint8 new_progress = progress_ + (80 / dev_list.size());
    if (new_progress >= 90) {
      UpdateProgress(90);
    } else {
      UpdateProgress(new_progress);
    }
  }

  SLEEP_MS(1000);
  UpdateProgress(100);
  act_status = ACT_STATUS_SUCCESS;
  return act_status;
}
