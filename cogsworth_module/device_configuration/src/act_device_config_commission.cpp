
#include "act_device_configuration.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

ACT_STATUS ActDeviceConfiguration::StartCommission(const ActProject &project, const QList<qint64> &dev_id_list,
                                                   const ActDeviceConfigTypeEnum &config_type,
                                                   ActDeviceConfig &sync_dev_config) {
  // Checking has the thread is running
  if (IsActStatusRunning(device_config_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("DeviceConfiguration");
  }

  // init ActDeviceConfiguration status
  progress_ = 0;
  stop_flag_ = false;
  device_config_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to start Commission
  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((device_config_thread_ != nullptr) && (device_config_thread_->joinable())) {
      device_config_thread_->join();
    }
    device_config_act_status_->SetStatus(ActStatusType::kRunning);
    device_config_thread_ =
        std::make_unique<std::thread>(&ActDeviceConfiguration::TriggerCommissionForThread, this, std::cref(project),
                                      std::cref(dev_id_list), std::cref(config_type), std::ref(sync_dev_config));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggerCommissionForThread";
    HRESULT hr = SetThreadDescription(device_config_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    qDebug() << "Start Commission thread.";

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(Commission) failed. Error:" << e.what();
    device_config_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("DeviceConfiguration");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*device_config_act_status_), progress_);
}

void ActDeviceConfiguration::TriggerCommissionForThread(const ActProject &project, const QList<qint64> &dev_id_list,
                                                        const ActDeviceConfigTypeEnum &config_type,
                                                        ActDeviceConfig &sync_dev_config) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the Commission and wait for the return, and update device_config_thread_.
  try {
    device_config_act_status_ = Commission(project, dev_id_list, config_type, sync_dev_config);
  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(interrupt) failed. Error:" << e.what();
    device_config_act_status_ = std::make_shared<ActStatusInternalError>("DeviceConfiguration");
  }
}

ACT_STATUS ActDeviceConfiguration::UpdateDeviceConnectByFeature(ActDevice &dev) {
  ACT_STATUS_INIT();
  ActDeviceConnectStatusControl total_control(false, false, false, false);
  ActDeviceConnectStatusControl config_control(false, false, false, false);
  ActDeviceConnectStatusControl scan_control(false, false, false, false);

  // Get Configuration feature
  ActFeatureProfile config_feature_profile;
  auto config_status = GetDeviceFeature(dev, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                        ActFeatureEnum::kConfiguration, config_feature_profile);
  if (!IsActStatusSuccess(config_status)) {
    qCritical() << __func__
                << QString("Device(%1) GetDeviceFeature(Configuration) failed.")
                       .arg(dev.GetIpv4().GetIpAddress())
                       .toStdString()
                       .c_str();
  } else {
    // Find Feature used connect protocols
    GetFeaturesUsedConnectProtocol(config_feature_profile, config_control);
  }

  // Get AutoScan feature
  ActFeatureProfile auto_feature_profile;
  auto auto_status = GetDeviceFeature(dev, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                      ActFeatureEnum::kAutoScan, auto_feature_profile);
  if (!IsActStatusSuccess(auto_status)) {
    qCritical() << __func__
                << QString("Device(%1) GetDeviceFeature(AutoScan) failed.")
                       .arg(dev.GetIpv4().GetIpAddress())
                       .toStdString()
                       .c_str();
  } else {
    // Find Feature used connect protocols
    GetFeaturesUsedConnectProtocol(auto_feature_profile, scan_control);
  }

  // Aggregate control
  if (scan_control.GetRESTful() || config_control.GetRESTful()) {
    total_control.SetRESTful(true);
  }
  if (scan_control.GetSNMP() || config_control.GetSNMP()) {
    total_control.SetSNMP(true);
  }
  if (scan_control.GetNETCONF() || config_control.GetNETCONF()) {
    total_control.SetNETCONF(true);
  }
  if (scan_control.GetNewMOXACommand() || config_control.GetNewMOXACommand()) {
    total_control.SetNewMOXACommand(true);
  }

  // Update Device connect
  act_status = southbound_.FeatureAssignDeviceStatus(false, dev, total_control);
  if (!IsActStatusSuccess(act_status)) {
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActDeviceConfiguration::Commission(const ActProject &project, const QList<qint64> &dev_id_list,
                                              const ActDeviceConfigTypeEnum &config_type,
                                              ActDeviceConfig &sync_dev_config) {
  ACT_STATUS_INIT();

  QMap<QString, QString> mac_host_map;

  // Create device list
  QList<qint64> sorted_dev_id_list = dev_id_list;
  SortDeviceIdList(project, sorted_dev_id_list);

  QList<ActDevice> dev_list;
  for (auto &dev_id : sorted_dev_id_list) {
    ActDevice dev;
    act_status = project.GetDeviceById(dev, dev_id);
    if (!IsActStatusSuccess(act_status)) {
      DeviceConfigurationErrorHandler(__func__, "Device not found in the project", dev);
      continue;
    }
    dev_list.append(dev);
  }

  // Update Devices ICMP status to check alive
  southbound_.UpdateDevicesIcmpStatus(dev_list);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  UpdateProgress(10);

  auto dev_config = project.GetDeviceConfig();
  // For test
  // dev_config = project.GetDeviceConfig();

  for (auto dev : dev_list) {
    if (stop_flag_) {  // stop flag
      return ACT_STATUS_STOP;
    }
    act_status = ACT_STATUS_SUCCESS;

    // Check ICMP status
    if (!dev.GetDeviceStatus().GetICMPStatus()) {
      DeviceConfigurationErrorHandler(__func__, "ICMP status is false(not alive)", dev);
      continue;
    }

    // Update connect status to true for southbound
    // dev.GetDeviceStatus().SetAllConnectStatus(true);
    UpdateDeviceConnectByFeature(dev);

    // Update device's firmware feature profile
    act_status = UpdateDeviceFirmwareFeatureProfileId(dev);
    if (!IsActStatusSuccess(act_status)) {
      DeviceConfigurationErrorHandler(__func__, "Update Device Firmware feature profile failed", dev);
      continue;
    }

    if (stop_flag_) {  // stop flag
      return ACT_STATUS_STOP;
    }
    // Execute commission task
    switch (config_type) {
      case ActDeviceConfigTypeEnum::kNetworkSetting: {
        if (dev_config.GetNetworkSettingTables().contains(dev.GetId())) {
          auto set_status = southbound_.ConfigureNetworkSetting(dev, dev_config.GetNetworkSettingTables()[dev.GetId()]);
          if (IsActStatusSuccess(set_status)) {
            auto cfg_table = dev_config.GetNetworkSettingTables()[dev.GetId()];
            // Modify Device IP address
            dev.GetIpv4().SetIpAddress(cfg_table.GetIpAddress());
          }

          ActNetworkSettingTable scan_table(dev.GetId());
          auto get_status = southbound_.ScanNetworkSettingTable(dev, scan_table);
          if (IsActStatusSuccess(get_status)) {
            sync_dev_config.GetNetworkSettingTables()[dev.GetId()] = scan_table;
          }

          if (!IsActStatusSuccess(set_status)) {
            DeviceConfigurationErrorHandler(__func__, set_status->GetErrorMessage(), dev);
            continue;
          }

          if (!IsActStatusSuccess(get_status)) {
            DeviceConfigurationErrorHandler(__func__, "Sync Network Setting failed", dev);
            continue;
          }

        } else {
          DeviceConfigurationErrorHandler(__func__, "Not found the Network Setting table", dev);
          continue;
        }
      } break;
      case ActDeviceConfigTypeEnum::kInformationSetting: {
        if (dev_config.GetInformationSettingTables().contains(dev.GetId())) {
          auto set_status =
              southbound_.ConfigureInformationSetting(dev, dev_config.GetInformationSettingTables()[dev.GetId()]);

          ActInformationSettingTable scan_table(dev.GetId());
          auto get_status = southbound_.ScanInformationSettingTable(dev, scan_table);
          if (IsActStatusSuccess(get_status)) {
            sync_dev_config.GetInformationSettingTables()[dev.GetId()] = scan_table;
          }

          if (!IsActStatusSuccess(set_status)) {
            DeviceConfigurationErrorHandler(__func__, set_status->GetErrorMessage(), dev);
            continue;
          }
          if (!IsActStatusSuccess(get_status)) {
            DeviceConfigurationErrorHandler(__func__, "Sync Information Setting failed", dev);
            continue;
          }
        } else {
          DeviceConfigurationErrorHandler(__func__, "Not found the Information Setting table", dev);
          continue;
        }
      } break;
      case ActDeviceConfigTypeEnum::kLoginPolicy: {
        if (dev_config.GetLoginPolicyTables().contains(dev.GetId())) {
          auto set_status = southbound_.ConfigureLoginPolicy(dev, dev_config.GetLoginPolicyTables()[dev.GetId()]);

          ActLoginPolicyTable scan_table(dev.GetId());
          auto get_status = southbound_.ScanLoginPolicyTable(dev, scan_table);
          if (IsActStatusSuccess(get_status)) {
            sync_dev_config.GetLoginPolicyTables()[dev.GetId()] = scan_table;
          }

          if (!IsActStatusSuccess(set_status)) {
            DeviceConfigurationErrorHandler(__func__, set_status->GetErrorMessage(), dev);
            continue;
          }
          if (!IsActStatusSuccess(get_status)) {
            DeviceConfigurationErrorHandler(__func__, "Sync Login Policy failed", dev);
            continue;
          }
        } else {
          DeviceConfigurationErrorHandler(__func__, "Not found the Login Policy table", dev);
          continue;
        }
      } break;
      case ActDeviceConfigTypeEnum::kSNMPTrapSetting: {
        if (dev_config.GetSnmpTrapSettingTables().contains(dev.GetId())) {
          auto set_status =
              southbound_.ConfigureSnmpTrapSetting(dev, dev_config.GetSnmpTrapSettingTables()[dev.GetId()]);

          ActSnmpTrapSettingTable scan_table(dev.GetId());
          auto get_status = southbound_.ScanSnmpTrapSettingTable(dev, scan_table);
          if (IsActStatusSuccess(get_status)) {
            sync_dev_config.GetSnmpTrapSettingTables()[dev.GetId()] = scan_table;
          }

          if (!IsActStatusSuccess(set_status)) {
            DeviceConfigurationErrorHandler(__func__, set_status->GetErrorMessage(), dev);
            continue;
          }
          if (!IsActStatusSuccess(get_status)) {
            DeviceConfigurationErrorHandler(__func__, "Sync SNMP Trap Setting failed", dev);
            continue;
          }
        } else {
          DeviceConfigurationErrorHandler(__func__, "Not found the SNMP Trap Setting table", dev);
          continue;
        }
      } break;
      case ActDeviceConfigTypeEnum::kSyslogSetting: {
        if (dev_config.GetSyslogSettingTables().contains(dev.GetId())) {
          auto set_status = southbound_.ConfigureSyslogSetting(dev, dev_config.GetSyslogSettingTables()[dev.GetId()]);

          ActSyslogSettingTable scan_table(dev.GetId());
          auto get_status = southbound_.ScanSyslogSettingTable(dev, scan_table);
          if (IsActStatusSuccess(get_status)) {
            sync_dev_config.GetSyslogSettingTables()[dev.GetId()] = scan_table;
          }

          if (!IsActStatusSuccess(set_status)) {
            DeviceConfigurationErrorHandler(__func__, set_status->GetErrorMessage(), dev);
            continue;
          }
          if (!IsActStatusSuccess(get_status)) {
            DeviceConfigurationErrorHandler(__func__, "Sync Syslog Setting failed", dev);
            continue;
          }
        } else {
          DeviceConfigurationErrorHandler(__func__, "Not found the Syslog Setting table", dev);
          continue;
        }
      } break;
      case ActDeviceConfigTypeEnum::kTimeSetting: {
        if (dev_config.GetTimeSettingTables().contains(dev.GetId())) {
          auto set_status = southbound_.ConfigureTimeSetting(dev, dev_config.GetTimeSettingTables()[dev.GetId()]);

          ActTimeSettingTable scan_table(dev.GetId());
          auto get_status = southbound_.ScanTimeSettingTable(dev, scan_table);
          if (IsActStatusSuccess(get_status)) {
            sync_dev_config.GetTimeSettingTables()[dev.GetId()] = scan_table;
          }

          if (!IsActStatusSuccess(set_status)) {
            DeviceConfigurationErrorHandler(__func__, set_status->GetErrorMessage(), dev);
            continue;
          }
          if (!IsActStatusSuccess(get_status)) {
            DeviceConfigurationErrorHandler(__func__, "Sync Time Setting failed", dev);
            continue;
          }

        } else {
          DeviceConfigurationErrorHandler(__func__, "Not found the Time Setting table", dev);
          continue;
        }
      } break;
      case ActDeviceConfigTypeEnum::kLoopProtection: {
        if (dev_config.GetLoopProtectionTables().contains(dev.GetId())) {
          auto set_status = southbound_.ConfigureLoopProtection(dev, dev_config.GetLoopProtectionTables()[dev.GetId()]);

          ActLoopProtectionTable scan_table(dev.GetId());
          auto get_status = southbound_.ScanLoopProtectionTable(dev, scan_table);
          if (IsActStatusSuccess(get_status)) {
            sync_dev_config.GetLoopProtectionTables()[dev.GetId()] = scan_table;
          }

          if (!IsActStatusSuccess(set_status)) {
            DeviceConfigurationErrorHandler(__func__, set_status->GetErrorMessage(), dev);
            continue;
          }
          if (!IsActStatusSuccess(get_status)) {
            DeviceConfigurationErrorHandler(__func__, "Sync Loop Protection failed", dev);
            continue;
          }

        } else {
          DeviceConfigurationErrorHandler(__func__, "Not found the Loop Protection table", dev);
          continue;
        }
      } break;
      case ActDeviceConfigTypeEnum::kVLANSetting: {
        if (dev_config.GetVlanTables().contains(dev.GetId())) {
          // Check with the PCP table
          bool with_pcp = false;
          if (dev_config.GetPortDefaultPCPTables().contains(dev.GetId())) {
            with_pcp = true;
          }

          ACT_STATUS set_pcp_status;
          if (with_pcp) {
            set_pcp_status =
                southbound_.ConfigurePortDefaultPCP(dev, dev_config.GetPortDefaultPCPTables()[dev.GetId()]);
          }
          if (stop_flag_) {
            return ACT_STATUS_STOP;
          }

          auto set_vlan_status = southbound_.ConfigureVlan(dev, dev_config.GetVlanTables()[dev.GetId()]);

          // Check ICMP status
          SLEEP_MS(1000);
          auto ping_status = southbound_.PingIpAddress(dev.GetIpv4().GetIpAddress(), ACT_PING_REPEAT_TIMES);
          if (!IsActStatusSuccess(ping_status)) {
            qWarning() << QString("Device(%1) not alive").arg(dev.GetIpv4().GetIpAddress()).toStdString().c_str();

            if (!IsActStatusSuccess(set_pcp_status)) {
              DeviceConfigurationErrorHandler(__func__, set_pcp_status->GetErrorMessage(), dev);
              continue;
            } else {
              sync_dev_config.GetPortDefaultPCPTables()[dev.GetId()] =
                  dev_config.GetPortDefaultPCPTables()[dev.GetId()];
            }

            if (!IsActStatusSuccess(set_vlan_status)) {
              DeviceConfigurationErrorHandler(__func__, set_vlan_status->GetErrorMessage(), dev);
              continue;
            } else {
              sync_dev_config.GetVlanTables()[dev.GetId()] = dev_config.GetVlanTables()[dev.GetId()];
              break;
            }
          }

          ActDefaultPriorityTable scan_pcp_table(dev.GetId());
          auto get_pcp_status = southbound_.ScanPortDefaultPCPTable(dev, scan_pcp_table);
          if (stop_flag_) {
            return ACT_STATUS_STOP;
          }
          if (IsActStatusSuccess(get_pcp_status)) {
            sync_dev_config.GetPortDefaultPCPTables()[dev.GetId()] = scan_pcp_table;
          }

          ActVlanTable scan_vlan_table(dev.GetId());
          auto get_vlan_status = southbound_.ScanVlanTable(dev, scan_vlan_table);
          if (stop_flag_) {
            return ACT_STATUS_STOP;
          }
          if (IsActStatusSuccess(get_vlan_status)) {
            sync_dev_config.GetVlanTables()[dev.GetId()] = scan_vlan_table;
          }

          if (with_pcp) {
            if (!IsActStatusSuccess(set_pcp_status)) {
              DeviceConfigurationErrorHandler(__func__, set_pcp_status->GetErrorMessage(), dev);
              continue;
            }
          }

          if (!IsActStatusSuccess(set_vlan_status)) {
            DeviceConfigurationErrorHandler(__func__, set_vlan_status->GetErrorMessage(), dev);
            continue;
          }

          if (!IsActStatusSuccess(get_pcp_status)) {
            DeviceConfigurationErrorHandler(__func__, "Sync PCP failed", dev);
            continue;
          }
          if (!IsActStatusSuccess(get_vlan_status)) {
            DeviceConfigurationErrorHandler(__func__, "Sync VLAN failed", dev);
            continue;
          }

        } else {
          DeviceConfigurationErrorHandler(__func__, "Not found the VLAN table", dev);
          continue;
        }

      } break;
      case ActDeviceConfigTypeEnum::kPortSetting: {
        if (dev_config.GetPortSettingTables().contains(dev.GetId())) {
          auto set_status = southbound_.ConfigurePortSetting(dev, dev_config.GetPortSettingTables()[dev.GetId()]);

          // Check ICMP status
          SLEEP_MS(1000);
          auto ping_status = southbound_.PingIpAddress(dev.GetIpv4().GetIpAddress(), ACT_PING_REPEAT_TIMES);
          if (IsActStatusSuccess(ping_status)) {
            qWarning() << QString("Device(%1) not alive").arg(dev.GetIpv4().GetIpAddress()).toStdString().c_str();
            if (!IsActStatusSuccess(set_status)) {
              DeviceConfigurationErrorHandler(__func__, set_status->GetErrorMessage(), dev);
              continue;
            } else {
              sync_dev_config.GetPortSettingTables()[dev.GetId()] = dev_config.GetPortSettingTables()[dev.GetId()];
              break;
            }
          }

          ActPortSettingTable scan_table(dev.GetId());
          auto get_status = southbound_.ScanPortSettingTable(dev, scan_table);
          if (IsActStatusSuccess(get_status)) {
            sync_dev_config.GetPortSettingTables()[dev.GetId()] = scan_table;
          }

          if (!IsActStatusSuccess(set_status)) {
            DeviceConfigurationErrorHandler(__func__, set_status->GetErrorMessage(), dev);
            continue;
          }
          if (!IsActStatusSuccess(get_status)) {
            DeviceConfigurationErrorHandler(__func__, "Sync Port Setting failed", dev);
            continue;
          }

        } else {
          DeviceConfigurationErrorHandler(__func__, "Not found the Port Setting table", dev);
          continue;
        }
      } break;
      case ActDeviceConfigTypeEnum::kSTPRSTP: {
        if (dev_config.GetRstpTables().contains(dev.GetId())) {
          auto set_status = southbound_.ConfigureSpanningTree(dev, dev_config.GetRstpTables()[dev.GetId()]);

          // Check ICMP status
          SLEEP_MS(1000);
          auto ping_status = southbound_.PingIpAddress(dev.GetIpv4().GetIpAddress(), ACT_PING_REPEAT_TIMES);
          if (IsActStatusSuccess(ping_status)) {
            qWarning() << QString("Device(%1) not alive").arg(dev.GetIpv4().GetIpAddress()).toStdString().c_str();
            if (!IsActStatusSuccess(set_status)) {
              DeviceConfigurationErrorHandler(__func__, set_status->GetErrorMessage(), dev);
              continue;
            } else {
              sync_dev_config.GetRstpTables()[dev.GetId()] = dev_config.GetRstpTables()[dev.GetId()];
              break;
            }
          }

          ActRstpTable scan_table(dev.GetId());
          auto get_status = southbound_.ScanRstpTable(dev, scan_table);
          if (IsActStatusSuccess(get_status)) {
            sync_dev_config.GetRstpTables()[dev.GetId()] = scan_table;
          }

          if (!IsActStatusSuccess(set_status)) {
            DeviceConfigurationErrorHandler(__func__, set_status->GetErrorMessage(), dev);
          }
          if (!IsActStatusSuccess(get_status)) {
            DeviceConfigurationErrorHandler(__func__, "Sync Spanning Tree failed", dev);
            continue;
          }

        } else {
          DeviceConfigurationErrorHandler(__func__, "Not found the Spanning Tree table", dev);
          continue;
        }
      } break;
      case ActDeviceConfigTypeEnum::kManagementInterface: {
        if (dev_config.GetManagementInterfaceTables().contains(dev.GetId())) {
          auto set_status =
              southbound_.ConfigureManagementInterface(dev, dev_config.GetManagementInterfaceTables()[dev.GetId()]);

          ActManagementInterfaceTable scan_table(dev.GetId());
          auto get_status = southbound_.ScanManagementInterfaceTable(dev, scan_table);
          if (IsActStatusSuccess(get_status)) {
            sync_dev_config.GetManagementInterfaceTables()[dev.GetId()] = scan_table;
          }

          if (!IsActStatusSuccess(set_status)) {
            DeviceConfigurationErrorHandler(__func__, set_status->GetErrorMessage(), dev);
            continue;
          }
          if (!IsActStatusSuccess(get_status)) {
            DeviceConfigurationErrorHandler(__func__, "Sync Management Interface failed", dev);
            continue;
          }
        } else {
          DeviceConfigurationErrorHandler(__func__, "Not found the Management Interface table", dev);
          continue;
        }
      } break;
      default: {
        QString error_msg = QString("Commission not support config(%1)")
                                .arg(kActDeviceConfigTypeEnumMap.key(config_type))
                                .toStdString()
                                .c_str();
        qCritical() << __func__ << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
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