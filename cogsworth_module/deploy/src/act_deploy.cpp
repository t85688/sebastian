#include "act_deploy.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QJsonDocument>
#include <QJsonObject>

#include "act_algorithm.hpp"

act::deploy::ActDeploy::~ActDeploy() {
  if ((deployer_thread_ != nullptr) && (deployer_thread_->joinable())) {
    deployer_thread_->join();
  }
}

ACT_STATUS act::deploy::ActDeploy::Start(const ActProject &project, const QList<qint64> &dev_id_list,
                                         const ActDeployControl &deploy_control, const bool &skip_mapping_dev,
                                         ActDeployParameterBase *parameter_base) {
  ACT_STATUS_INIT();

  // Checking has the thread is running
  if (IsActStatusRunning(deployer_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("Deploy");
  }

  // init deployer status
  progress_ = 0;
  deployer_stop_flag_ = false;
  deployer_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to triggered the deployer
  try {
    // check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((deployer_thread_ != nullptr) && (deployer_thread_->joinable())) {
      deployer_thread_->join();
    }

    deployer_act_status_->SetStatus(ActStatusType::kRunning);
    deployer_thread_ = std::make_unique<std::thread>(
        &act::deploy::ActDeploy::TriggeredDeployerForThread, this, std::cref(project), std::cref(dev_id_list),
        std::cref(deploy_control), std::cref(skip_mapping_dev), std::ref(parameter_base));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggeredDeployerForThread";
    HRESULT hr = SetThreadDescription(deployer_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(deployer) failed. Error:" << e.what();
    deployer_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("Deploy");
  }

  qDebug() << "Start deployer thread.";
  return std::make_shared<ActProgressStatus>(ActStatusBase(*deployer_act_status_), progress_);
}

ACT_STATUS act::deploy::ActDeploy::Stop() {
  // Checking has the thread is running
  if (IsActStatusRunning(deployer_act_status_)) {
    qDebug() << "Stop deployer thread.";

    southbound_.SetStopFlag(true);

    // Send the stop signal to the deployer and wait for the thread to finish.
    deployer_stop_flag_ = true;
    if ((deployer_thread_ != nullptr) && (deployer_thread_->joinable())) {
      deployer_thread_->join();  // wait thread finished
    }
  } else {
    qDebug() << __func__ << "The Deployer thread not running.";
  }
  return std::make_shared<ActProgressStatus>(ActStatusBase(*deployer_act_status_), progress_);
}

ACT_STATUS act::deploy::ActDeploy::GetStatus() {
  if (IsActStatusSuccess(deployer_act_status_) && (progress_ == 100)) {
    deployer_act_status_->SetStatus(ActStatusType::kFinished);
  }

  if ((!IsActStatusRunning(deployer_act_status_)) && (!IsActStatusFinished(deployer_act_status_))) {
    // failed
    return deployer_act_status_;
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*deployer_act_status_), progress_);
}

ACT_STATUS act::deploy::ActDeploy::UpdateProgress(quint8 progress) {
  ACT_STATUS_INIT();
  progress_ = progress;
  qDebug() << __func__ << QString("Progress: %1%.").arg(GetProgress()).toStdString().c_str();
  return act_status;
}

ACT_STATUS act::deploy::ActDeploy::DeployErrorHandler(QString called_func, const QString &error_reason,
                                                      const QString &error_detail, const ActDevice &device) {
  qCritical() << called_func.toStdString().c_str()
              << QString("Device: %1(%2): %3. Detail: %4")
                     .arg(device.GetIpv4().GetIpAddress())
                     .arg(device.GetId())
                     .arg(error_reason)
                     .arg(error_detail)
                     .toStdString()
                     .c_str();

  result_queue_.enqueue(
      ActDeviceConfigureResult(device.GetId(), progress_, ActStatusType::kFailed, error_reason, error_detail));

  failed_device_id_set_.insert(device.GetId());
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS act::deploy::ActDeploy::DeployIniErrorHandler(QString called_func, const QString &error_reason,
                                                         const ActDevice &device) {
  qCritical() << called_func.toStdString().c_str()
              << QString("Device: %1(%2): %3")
                     .arg(device.GetIpv4().GetIpAddress())
                     .arg(device.GetId())
                     .arg(error_reason)
                     .toStdString()
                     .c_str();

  result_queue_.enqueue(ActDeviceConfigureResult(device.GetId(), progress_, ActStatusType::kFailed, error_reason));

  failed_device_id_set_.insert(device.GetId());
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS act::deploy::ActDeploy::CheckFeatureErrorHandler(QString called_func, const QString &check_item,
                                                            const ActDevice &device) {
  QString error_msg = QString("Device not support %1").arg(check_item);

  qCritical() << called_func.toStdString().c_str()
              << QString("%1. Device:%2(%3)")
                     .arg(error_msg)
                     .arg(device.GetIpv4().GetIpAddress())
                     .arg(device.GetId())
                     .toStdString()
                     .c_str();

  return std::make_shared<ActStatusCheckFeatureFailed>(error_msg);
}

void act::deploy::ActDeploy::TriggeredDeployerForThread(const ActProject &project, const QList<qint64> &dev_id_list,
                                                        const ActDeployControl &deploy_control,
                                                        const bool &skip_mapping_dev,
                                                        ActDeployParameterBase *parameter_base) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the deployer and wait for the return, and update deployer_act_status_.
  try {
    deployer_act_status_ = Deployer(project, dev_id_list, deploy_control, skip_mapping_dev, parameter_base);
  } catch (std::exception &e) {
    qCritical() << __func__ << "Deployer() failed. Error:" << e.what();
    deployer_act_status_ = std::make_shared<ActStatusInternalError>("Deploy");
  }
}

// netconf time out is 6s (ITRI 20)
// snmp time out is 6s (session default)
// snmp per entry is 0.025s (almost)
ACT_STATUS act::deploy::ActDeploy::Deployer(const ActProject &project, const QList<qint64> &dev_id_list,
                                            const ActDeployControl &deploy_control, const bool &skip_mapping_dev,
                                            ActDeployParameterBase *parameter_base) {
  ACT_STATUS_INIT();

  qInfo() << __func__ << QString("Deploy list size:%1").arg(dev_id_list.size()).toStdString().c_str();
  auto dev_config = project.GetDeviceConfig();
  // Generate the Deploy device list
  QList<ActDevice> deploy_dev_list;
  for (auto &dev_id : dev_id_list) {
    // Get project device by dev_id
    ActDevice dev;
    act_status = project.GetDeviceById(dev, dev_id);
    if (!IsActStatusSuccess(act_status)) {
      DeployErrorHandler(__func__, "Device not found in the project", act_status->GetErrorMessage(), dev);
      continue;
    }

    deploy_dev_list.append(dev);
    // qInfo() << __func__
    //         << QString("Deploy sequence %1: %2")
    //                .arg(deploy_dev_list.size())
    //                .arg(dev.GetIpv4().GetIpAddress())
    //                .toStdString()
    //                .c_str();
  }

  // Check feature
  bool devices_checked_result = true;
  for (auto &dev : deploy_dev_list) {
    // Mapping Device firmware (Update FeatureGroup and FirmwareFeatureProfileId)
    act_status = MappingDeviceFirmware(dev, dev_config, skip_mapping_dev);
    if (!IsActStatusSuccess(act_status)) {
      DeployErrorHandler(__func__, "Mapping Device firmware failed", act_status->GetErrorMessage(), dev);
      devices_checked_result = false;
      continue;
    }
    // Check feature group (ImportExport)
    act_status = CheckDeviceFeature(dev, dev_config);
    if (!dev.GetDeviceProperty().GetFeatureGroup().GetOperation().GetImportExport()) {
      DeployErrorHandler(__func__, "Check Feature failed", "Not support the Import operation", dev);
      devices_checked_result = false;
      continue;
    }
  }
  // Any one device check failed would stop & another devices would response stop status
  if (!devices_checked_result) {
    for (auto &dev : deploy_dev_list) {
      if (!failed_device_id_set_.contains(dev.GetId())) {
        result_queue_.enqueue(ActDeviceConfigureResult(dev.GetId(), progress_, ActStatusType::kStop, "Stop configure",
                                                       "Some devices check the features failed"));
      }
    }

    SLEEP_MS(100);
    UpdateProgress(100);
    act_status = ACT_STATUS_SUCCESS;
    return act_status;
  }
  UpdateProgress(5);

  // Configure all MappingDeviceIpSettingTables
  auto ip_setting_tables = dev_config.GetMappingDeviceIpSettingTables();
  for (auto &dev : deploy_dev_list) {
    if ((skip_mapping_dev == false) && ip_setting_tables.contains(dev.GetId())) {
      act_status = DeployMappingDeviceIpSetting(dev, ip_setting_tables[dev.GetId()]);
      if (deployer_stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {
        DeployErrorHandler(__func__, "Configure the offline design IP failed", act_status->GetErrorMessage(), dev);
        continue;
      }
    } else {  // Configure Same IP device's IPv4 config (Not change IP)
      ActNetworkSettingTable network_setting_table(dev);
      act_status = DeployNetworkSetting(dev, network_setting_table, deploy_control.GetFromBroadcastSearch());
      if (deployer_stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {
        DeployErrorHandler(__func__, "Configure Network Setting failed", act_status->GetErrorMessage(), dev);
        continue;
      }
    }
  }

  // If any one device changes IP would sleep 2 sec
  if (!ip_setting_tables.isEmpty()) {
    SLEEP_MS(2000);
  }

  // Check all Device alive
  southbound_.ClearArpCache();
  southbound_.UpdateDevicesIcmpStatus(deploy_dev_list);
  if (deployer_stop_flag_) {
    return ACT_STATUS_STOP;
  }

  for (auto &dev : deploy_dev_list) {
    if (!dev.GetDeviceStatus().GetICMPStatus()) {
      DeployErrorHandler(__func__, "Device not alive", "ICMP status is false", dev);
      continue;
    }
  }

  UpdateProgress(10);

  // Update all devices status
  for (auto &dev : deploy_dev_list) {
    if (deployer_stop_flag_) {
      return ACT_STATUS_STOP;
    }
    // Check device not failed
    if (failed_device_id_set_.contains(dev.GetId())) {
      qDebug() << __func__
               << QString("Skip Device(%1). Because it already failed")
                      .arg(dev.GetIpv4().GetIpAddress())
                      .toStdString()
                      .c_str();
      continue;
    }

    // Update Device Connect status
    act_status = southbound_.FeatureAssignDeviceStatus(false, dev);
    if (!IsActStatusSuccess(act_status)) {  // not alive
      DeployErrorHandler(__func__, "Update device connect status failed", act_status->GetErrorMessage(), dev);

      continue;
    }
  }

  UpdateProgress(20);
  // Clear all devices RSTP configurations
  if (deploy_control.GetSpanningTree()) {
    for (auto &dev : deploy_dev_list) {
      if (deployer_stop_flag_) {
        return ACT_STATUS_STOP;
      }

      // Check device not failed
      if (failed_device_id_set_.contains(dev.GetId())) {
        qDebug() << __func__
                 << QString("Skip Device(%1). Because it already failed")
                        .arg(dev.GetIpv4().GetIpAddress())
                        .toStdString()
                        .c_str();
        continue;
      }

      // Deploy SpanningTree init configure
      if (dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetRSTP()) {
        ActRstpTable rstp_table(dev);
        // Deploy
        act_status = southbound_.ConfigureSpanningTree(dev, rstp_table);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          DeployErrorHandler(__func__, "Configure Spanning Tree init configuration failed",
                             act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }
  }

  UpdateProgress(30);

  // Start deploy (30%~90%)
  quint8 dev_progress_slot = 60;
  if (deploy_dev_list.size() > 0) {
    dev_progress_slot = dev_progress_slot / deploy_dev_list.size();
  }
  const quint8 dev_progress_update_times = 4;

  for (auto &dev : deploy_dev_list) {
    quint8 tsn_switch_retry_times = 0;

    if (deployer_stop_flag_) {
      return ACT_STATUS_STOP;
    }
    // Check device not failed
    if (failed_device_id_set_.contains(dev.GetId())) {
      qDebug() << __func__
               << QString("Skip Device(%1). Because it already failed")
                      .arg(dev.GetIpv4().GetIpAddress())
                      .toStdString()
                      .c_str();
      continue;
    }

    // Clear TSN-Switch configuration sync flag
    if (dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetCheckConfigSynchronization()) {
      // Use RESTful to clear
      bool check_result;
      act_status = southbound_.GetTSNSwitchConfigurationSyncStatus(dev, check_result);
      if (deployer_stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {
        DeployErrorHandler(__func__, "Clear Device sync status failed", act_status->GetErrorMessage(), dev);
        continue;
      }
    }

  start_deploy_flow:  // goto tag
    if (deployer_stop_flag_) {
      return ACT_STATUS_STOP;
    }

    if (tsn_switch_retry_times > 0) {
      qDebug() << __func__
               << QString("Retry deploy Device(%1)(%2/%3).")
                      .arg(dev.GetIpv4().GetIpAddress())
                      .arg(tsn_switch_retry_times)
                      .arg(ACT_DEPLOY_TSN_SWITCH_RETRY_TIMES)
                      .toStdString()
                      .c_str();
    }

    // Deploy NetworkSetting
    if (deploy_control.GetNetworkSetting()) {
      auto network_setting_tables = dev_config.GetNetworkSettingTables();
      if (network_setting_tables.contains(dev.GetId())) {
        // Deploy
        act_status =
            DeployNetworkSetting(dev, network_setting_tables[dev.GetId()], deploy_control.GetFromBroadcastSearch());
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          DeployErrorHandler(__func__, "Configure Network Setting failed", act_status->GetErrorMessage(), dev);
          continue;
        }

        // Insert to network_setting success device set
        network_setting_success_devices_.insert(dev.GetId());

        // Update device ip_address
        auto &ipv4 = dev.GetIpv4();
        ipv4.SetIpAddress(network_setting_tables[dev.GetId()].GetIpAddress());
        qDebug() << __func__
                 << QString("Use the new IP to continue to deploy the device(%1(%2))")
                        .arg(dev.GetIpv4().GetIpAddress())
                        .arg(dev.GetId())
                        .toStdString()
                        .c_str();
      }
    }

    // Deploy LoginPolicy
    if (deploy_control.GetLoginPolicy()) {
      auto login_policy_tables = dev_config.GetLoginPolicyTables();
      if (login_policy_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigureLoginPolicy(dev, login_policy_tables[dev.GetId()]);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          DeployErrorHandler(__func__, "Configure LoginPolicy failed", act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }

    // Deploy InformationSetting
    if (deploy_control.GetInformationSetting()) {
      auto info_setting_tables = dev_config.GetInformationSettingTables();
      if (info_setting_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigureInformationSetting(dev, info_setting_tables[dev.GetId()]);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          DeployErrorHandler(__func__, "Configure InformationSetting failed", act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }

    // Deploy SnmpTrapSetting
    if (deploy_control.GetSnmpTrapSetting()) {
      auto snmp_trap_setting_tables = dev_config.GetSnmpTrapSettingTables();
      if (snmp_trap_setting_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigureSnmpTrapSetting(dev, snmp_trap_setting_tables[dev.GetId()]);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          DeployErrorHandler(__func__, "Configure SnmpTrapSetting failed", act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }

    // Deploy SyslogSetting
    if (deploy_control.GetSyslogSetting()) {
      auto syslog_setting_tables = dev_config.GetSyslogSettingTables();
      if (syslog_setting_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigureSyslogSetting(dev, syslog_setting_tables[dev.GetId()]);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          DeployErrorHandler(__func__, "Configure SyslogSetting failed", act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }

    // Deploy TimeSetting
    if (deploy_control.GetTimeSetting()) {
      auto time_setting_tables = dev_config.GetTimeSettingTables();
      if (time_setting_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigureTimeSetting(dev, time_setting_tables[dev.GetId()]);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          DeployErrorHandler(__func__, "Configure TimeSetting failed", act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }

    // Deploy PortSetting(AdminStatus)
    if (deploy_control.GetPortSetting()) {
      auto port_setting_tables = dev_config.GetPortSettingTables();
      if (port_setting_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigurePortSetting(dev, port_setting_tables[dev.GetId()]);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          DeployErrorHandler(__func__, "Configure PortSetting failed", act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }

    // Deploy LoopProtection
    if (deploy_control.GetLoopProtection()) {
      auto loop_protection_tables = dev_config.GetLoopProtectionTables();
      if (loop_protection_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigureLoopProtection(dev, loop_protection_tables[dev.GetId()]);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          DeployErrorHandler(__func__, "Configure LoopProtection failed", act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }

    // Update progress
    quint8 new_progress = progress_ + (dev_progress_slot / dev_progress_update_times);
    if (tsn_switch_retry_times == 0) {
      UpdateProgress(new_progress);
    }

    // Deploy VLAN(VlanStatic, PortVlan(PVID), VlanPortType)
    if (deploy_control.GetVlan()) {
      auto vlan_tables = dev_config.GetVlanTables();
      if (vlan_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigureVlan(dev, vlan_tables[dev.GetId()]);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          // Check TSN-Switch configuration sync
          if (dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetCheckConfigSynchronization()) {
            bool check_result = true;
            CheckTSNSwitchConfigurationSynchronized(dev, dev_config, check_result);
            if (check_result == false) {
              tsn_switch_retry_times++;
              if ((tsn_switch_retry_times <= ACT_DEPLOY_TSN_SWITCH_RETRY_TIMES)) {
                goto start_deploy_flow;
              }
            }
          }

          DeployErrorHandler(__func__, "Configure VLAN failed", act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }

    // Update progress
    new_progress = progress_ + (dev_progress_slot / dev_progress_update_times);
    if (tsn_switch_retry_times == 0) {
      UpdateProgress(new_progress);
    }

    // Deploy Port Default PCP
    if (deploy_control.GetPortDefaultPcp()) {
      auto port_default_pcp_tables = dev_config.GetPortDefaultPCPTables();
      if (port_default_pcp_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigurePortDefaultPCP(dev, port_default_pcp_tables[dev.GetId()]);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          DeployErrorHandler(__func__, "Configure Default PCP failed", act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }

    // Deploy UnicastStaticForward
    if (deploy_control.GetUnicastStaticForward()) {
      auto uni_static_forward_tables = dev_config.GetUnicastStaticForwardTables();
      if (uni_static_forward_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigureStaticForward(dev, uni_static_forward_tables[dev.GetId()], true);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          // Check TSN-Switch configuration sync
          if (dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetCheckConfigSynchronization()) {
            bool check_result = true;
            CheckTSNSwitchConfigurationSynchronized(dev, dev_config, check_result);
            if (check_result == false) {
              tsn_switch_retry_times++;
              if ((tsn_switch_retry_times <= ACT_DEPLOY_TSN_SWITCH_RETRY_TIMES)) {
                goto start_deploy_flow;
              }
            }
          }

          DeployErrorHandler(__func__, "Configure Unicast Static Forward failed", act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }

    // Deploy MulticastStaticForward
    if (deploy_control.GetMulticastStaticForward()) {
      auto mul_static_forward_tables = dev_config.GetMulticastStaticForwardTables();
      if (mul_static_forward_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigureStaticForward(dev, mul_static_forward_tables[dev.GetId()], false);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          // Check TSN-Switch configuration sync
          if (dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetCheckConfigSynchronization()) {
            bool check_result = true;
            CheckTSNSwitchConfigurationSynchronized(dev, dev_config, check_result);
            if (check_result == false) {
              tsn_switch_retry_times++;
              if ((tsn_switch_retry_times <= ACT_DEPLOY_TSN_SWITCH_RETRY_TIMES)) {
                goto start_deploy_flow;
              }
            }
          }

          DeployErrorHandler(__func__, "Configure Multicast Static Forward failed", act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }

    // Deploy StreamPriorityIngress
    if (deploy_control.GetStreamPriorityIngress()) {
      auto stad_ingress_tables = dev_config.GetStreamPriorityIngressTables();
      if (stad_ingress_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigureStreamPriorityIngress(dev, stad_ingress_tables[dev.GetId()]);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          // Check TSN-Switch configuration sync
          if (dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetCheckConfigSynchronization()) {
            bool check_result = true;
            CheckTSNSwitchConfigurationSynchronized(dev, dev_config, check_result);
            if (check_result == false) {
              tsn_switch_retry_times++;
              if ((tsn_switch_retry_times <= ACT_DEPLOY_TSN_SWITCH_RETRY_TIMES)) {
                goto start_deploy_flow;
              }
            }
          }

          DeployErrorHandler(__func__, "Configure Per-Stream Priority (Ingress) failed", act_status->GetErrorMessage(),
                             dev);
          continue;
        }
      }
    }

    // Deploy StreamPriorityEgress
    if (deploy_control.GetStreamPriorityEgress()) {
      auto stad_egress_tables = dev_config.GetStreamPriorityEgressTables();
      if (stad_egress_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigureStreamPriorityEgress(dev, stad_egress_tables[dev.GetId()]);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          DeployErrorHandler(__func__, "Configure Per-Stream Priority (Egress) failed", act_status->GetErrorMessage(),
                             dev);
          continue;
        }
      }
    }

    // Update progress
    new_progress = progress_ + (dev_progress_slot / dev_progress_update_times);
    if (tsn_switch_retry_times == 0) {
      UpdateProgress(new_progress);
    }

    // Deploy Spanning Tree
    if (deploy_control.GetSpanningTree()) {
      auto rstp_tables = dev_config.GetRstpTables();
      if (rstp_tables.contains(dev.GetId())) {
        // Deploy
        act_status = southbound_.ConfigureSpanningTree(dev, rstp_tables[dev.GetId()]);
        if (deployer_stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          DeployErrorHandler(__func__, "Configure Spanning Tree failed", act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }

    // NETCONF(802.1CB, 802.1Qbv)
    // Check TSN-Switch configuration sync
    if (deploy_control.GetIeee802Dot1Cb()) {
      auto ieee_802_1cb_table = dev_config.GetCbTables();
      if (ieee_802_1cb_table.contains(dev.GetId())) {
        if (dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetCheckConfigSynchronization()) {
          bool check_result = true;
          act_status = CheckTSNSwitchConfigurationSynchronized(dev, dev_config, check_result);
          if (deployer_stop_flag_) {
            return ACT_STATUS_STOP;
          }
          if (!IsActStatusSuccess(act_status)) {
            DeployErrorHandler(__func__, "Check Device configuration status failed", act_status->GetErrorMessage(),
                               dev);
            continue;
          }
          if (check_result == false) {
            tsn_switch_retry_times++;
            if ((tsn_switch_retry_times <= ACT_DEPLOY_TSN_SWITCH_RETRY_TIMES)) {
              goto start_deploy_flow;
            } else {  // retry 3 times failed
              DeployErrorHandler(__func__, "Device configuration not synchronized", act_status->GetErrorMessage(), dev);
              continue;
            }
          }
        }
      }
    }

    act_status = ConfigureNetconf(dev, deploy_control, dev_config);
    if (deployer_stop_flag_) {
      return ACT_STATUS_STOP;
    }
    if (!IsActStatusSuccess(act_status)) {
      // Check TSN-Switch configuration sync
      if (dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetCheckConfigSynchronization()) {
        bool check_result = true;
        CheckTSNSwitchConfigurationSynchronized(dev, dev_config, check_result);
        if (check_result == false) {
          tsn_switch_retry_times++;
          if ((tsn_switch_retry_times <= ACT_DEPLOY_TSN_SWITCH_RETRY_TIMES)) {
            goto start_deploy_flow;
          }
        }
      }
      DeployErrorHandler(__func__, "Configure NETCONF failed", act_status->GetErrorMessage(), dev);
      continue;
    }

    // Reboot
    if (deploy_control.GetReboot()) {
      act_status = Reboot(dev);
      if (deployer_stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {
        DeployErrorHandler(__func__, "Reboot failed", act_status->GetErrorMessage(), dev);
        continue;
      }
    }

    // FactoryDefault
    if (deploy_control.GetFactoryDefault()) {
      act_status = FactoryDefault(dev);
      if (deployer_stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {
        DeployErrorHandler(__func__, "Factory Default failed", act_status->GetErrorMessage(), dev);
        continue;
      }
    }

    // FirmwareUpgrade
    if (deploy_control.GetFirmwareUpgrade()) {
      // Check Parameter > FirmwareName
      auto parameter_fw = dynamic_cast<ActDeployParameterFirmware *>(parameter_base);
      if (parameter_fw->GetFirmwareName().isEmpty()) {
        DeployErrorHandler(__func__, "The FirmwareName parameter is empty", act_status->GetErrorMessage(), dev);
        continue;
      }

      act_status = FirmwareUpgrade(dev, parameter_fw->GetFirmwareName());
      if (deployer_stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {
        DeployErrorHandler(__func__, "Firmware Upgrade failed", act_status->GetErrorMessage(), dev);
        continue;
      }
    }

    // Check TSN-Switch configuration sync
    if (dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetCheckConfigSynchronization()) {
      bool check_result = true;
      act_status = CheckTSNSwitchConfigurationSynchronized(dev, dev_config, check_result);
      if (deployer_stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {
        DeployErrorHandler(__func__, "Check Device configuration status failed", act_status->GetErrorMessage(), dev);
        continue;
      }
      if (check_result == false) {
        tsn_switch_retry_times++;
        if ((tsn_switch_retry_times <= ACT_DEPLOY_TSN_SWITCH_RETRY_TIMES)) {
          goto start_deploy_flow;
        } else {  // retry 3 times failed
          DeployErrorHandler(__func__, "Device configuration not synchronized", act_status->GetErrorMessage(), dev);
          continue;
        }
      }
    }

    // Add success result to result_queue_
    result_queue_.enqueue(ActDeviceConfigureResult(dev.GetId(), progress_, ActStatusType::kSuccess));

    // Update progress(30~90)
    new_progress = progress_ + (dev_progress_slot / dev_progress_update_times);
    if (new_progress >= 90) {
      UpdateProgress(90);
    } else {
      UpdateProgress(new_progress);
    }
  }

  SLEEP_MS(100);
  UpdateProgress(100);
  act_status = ACT_STATUS_SUCCESS;
  return act_status;
}

ACT_STATUS act::deploy::ActDeploy::CheckTSNSwitchConfigurationSynchronized(const ActDevice &dev,
                                                                           ActDeviceConfig &dev_config, bool &result) {
  ACT_STATUS_INIT();

  // Use RESTful to check
  bool check_result = false;  // Synchronized
  act_status = southbound_.GetTSNSwitchConfigurationSyncStatus(dev, check_result);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetTSNSwitchConfigurationSyncStatus failed.";
  }

  if (check_result == true) {
    result = false;
    qDebug() << __func__ << "Check Configuration Synchronized result(false). The Device Sync status is enable.";
    return act_status;
  }

  // Check Device VLAN configuration
  auto vlan_tables = dev_config.GetVlanTables();
  if (vlan_tables.contains(dev.GetId())) {
    bool check_vlan_result = true;  // Synchronized
    act_status = southbound_.CheckDeviceVLANConfiguration(dev, vlan_tables[dev.GetId()], check_vlan_result);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "CheckDeviceVLANConfiguration failed.";

      return act_status;
    }
    if (check_vlan_result == false) {
      result = false;
      qDebug() << __func__
               << "Check Configuration Synchronized result(false). Compare Device VLAN configuration is inconsistent";
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS act::deploy::ActDeploy::CheckDeviceAlive(const ActDevice &device, bool from_broadcast_search) {
  ACT_STATUS_INIT();

  // Set ArpTable
  if (from_broadcast_search) {  // has mac
    // Check Map has device's mac
    if (!mac_host_map_.contains(device.GetMacAddress())) {  // hasn't
      qCritical()
          << __func__
          << QString("Device's(%1) MAC(%2) not found in mac_host_map").arg(device.GetId()).arg(device.GetMacAddress());

      return std::make_shared<ActStatusNotFound>(QString("MAC(%1) in mac_host_map").arg(device.GetMacAddress()));
    }

    QString host = mac_host_map_[device.GetMacAddress()];
    auto msg = QString("SetLocalhostArpTable: device: %1(%2, %3), host: %4")
                   .arg(device.GetId())
                   .arg(device.GetIpv4().GetIpAddress())
                   .arg(device.GetMacAddress())
                   .arg(host);
    qDebug() << __func__ << msg.toStdString().c_str();
    act_status = southbound_.SetLocalhostArpTable(device, host);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Ping
  act_status = southbound_.PingIpAddress(device.GetIpv4().GetIpAddress(), ACT_PING_REPEAT_TIMES);

  // Delete Arp entry
  if (from_broadcast_search) {
    auto act_status_arp = southbound_.DeleteLocalhostArpEntry(device);
    if (!IsActStatusSuccess(act_status_arp)) {
      qCritical() << __func__ << "DeleteLocalhostArpEntry() failed.";
    }
  }

  return act_status;
}

ACT_STATUS act::deploy::ActDeploy::CheckGclAdminControlListLength(const ActDevice &device,
                                                                  const ActGclTable &gate_control_table) {
  ACT_STATUS_INIT();

  // Check AdminControlListLength
  for (auto port_gate_parameters : gate_control_table.GetInterfacesGateParameters()) {
    if (device.GetDeviceProperty().GetGateControlListLength() <
        port_gate_parameters.GetGateParameters().GetAdminControlListLength()) {
      qCritical() << __func__ << "Interface support admin_control_list_length < target's admin_control_list_length";
      return std::make_shared<ActStatusInternalError>("Deploy");
    }
  }

  return act_status;
}

ACT_STATUS act::deploy::ActDeploy::MappingDeviceFirmware(ActDevice &device, const ActDeviceConfig &device_config,
                                                         const bool &skip_mapping_dev) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  bool set_arp_bool = false;
  auto ip_setting_tables = device_config.GetMappingDeviceIpSettingTables();
  ActMappingDeviceIpSettingTable ip_setting_table;
  if (skip_mapping_dev == false) {
    if (ip_setting_tables.contains(device.GetId())) {  // would modify IP device(need to use the old IP to get FW)
      ip_setting_table = ip_setting_tables[device.GetId()];
      set_arp_bool = true;
    }
  }

  // Generate new device for online_device(IP & MAC)
  ActDevice online_device(device);
  if (set_arp_bool) {
    online_device.GetIpv4().SetIpAddress(ip_setting_table.GetOnlineIP());
    online_device.SetMacAddress(ip_setting_table.GetMacAddress());
  }

  // Update connect status to true for southbound
  online_device.GetDeviceStatus().SetAllConnectStatus(true);

  // Set ArpTable
  if (set_arp_bool) {
    // Check Map has device's mac
    if (!mac_host_map_.contains(online_device.GetMacAddress())) {  // hasn't
      qCritical() << __func__
                  << QString("Device's(%1) MAC(%2) not found in mac_host_map")
                         .arg(online_device.GetId())
                         .arg(online_device.GetMacAddress());

      return std::make_shared<ActStatusNotFound>(QString("MAC(%1) in mac_host_map").arg(online_device.GetMacAddress()));
    }

    QString host = mac_host_map_[online_device.GetMacAddress()];
    act_status = southbound_.SetLocalhostArpTable(online_device, host);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get FirmwareVersion
  // Get Sub-item
  ActFeatureSubItem feature_sub_item;
  act_status =
      GetDeviceFeatureSubItem(online_device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                              ActFeatureEnum::kAutoScan, "Identify", "FirmwareVersion", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
    if (set_arp_bool) {
      auto act_status_arp = southbound_.DeleteLocalhostArpEntry(online_device);
      if (!IsActStatusSuccess(act_status_arp)) {
        qCritical() << __func__ << "DeleteLocalhostArpEntry() failed.";
      }
    }

    return act_status;
  }

  QString firmware_version;
  act_status = southbound_.ActionGetFirmwareVersion(online_device, feature_sub_item, firmware_version);

  // Delete Arp entry
  if (set_arp_bool) {
    auto act_status_arp = southbound_.DeleteLocalhostArpEntry(online_device);
    if (!IsActStatusSuccess(act_status_arp)) {
      qCritical() << __func__ << "DeleteLocalhostArpEntry() failed.";
    }
  }

  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get FirmwareVersion failed.";
    return act_status;
  }

  // v2.2.2 Build 2024_0507_1457
  // QString firmware_version = "v2.1 Build 2021_1021_1531";
  qDebug() << __func__
           << QString("Device(%1:%2(%3)) FirmwareVersion: %4")
                  .arg(device.GetIpv4().GetIpAddress())
                  .arg(online_device.GetMacAddress())
                  .arg(online_device.GetId())
                  .arg(firmware_version);

  // Match Feature table by Firmware

  // Find FirmwareFeatureProfile (ModelName & FirmwareVersion)
  auto model_name = device.GetDeviceProperty().GetModelName();

  ActFirmwareFeatureProfile fw_feat_profile;
  auto find_status = ActFirmwareFeatureProfile::GetFirmwareFeatureProfile(
      profiles_.GetFirmwareFeatureProfiles(), model_name, firmware_version, fw_feat_profile);
  if (IsActStatusSuccess(find_status)) {
    // Update  Feature Group
    device.GetDeviceProperty().SetFeatureGroup(fw_feat_profile.GetFeatureGroup());

    // Update Firmware Feature Profile ID
    device.SetFirmwareFeatureProfileId(fw_feat_profile.GetId());

    qDebug()
        << __func__
        << QString("Device mapping to Firmware Feature Profile(ID: %1).").arg(device.GetFirmwareFeatureProfileId());

  } else {
    // Update Latest Feature Group to device (if the fw different with the offline device)
    if (device.GetFirmwareVersion() != firmware_version) {
      ActDeviceProfile dev_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(profiles_.GetDeviceProfiles(), device.GetDeviceProfileId(), dev_profile);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__
                    << QString("The device profile not found. Device:%1(%2), DeviceProfile:%3")
                           .arg(device.GetIpv4().GetIpAddress())
                           .arg(device.GetId())
                           .arg(device.GetDeviceProfileId())
                           .toStdString()
                           .c_str();

        return act_status;
      }
      device.GetDeviceProperty().SetFeatureGroup(dev_profile.GetFeatureGroup());
    }

    // Update Firmware Feature Profile ID
    device.SetFirmwareFeatureProfileId(-1);
  }

  return act_status;
}

ACT_STATUS act::deploy::ActDeploy::CheckDeviceFeature(const ActDevice &device, const ActDeviceConfig &device_config) {
  ACT_STATUS_INIT();

  auto feature_group = device.GetDeviceProperty().GetFeatureGroup();

  // NetworkSetting
  if (device_config.GetMappingDeviceIpSettingTables().contains(device.GetId()) ||
      device_config.GetNetworkSettingTables().contains(device.GetId())) {
    if (!feature_group.GetConfiguration().GetNetworkSetting()) {
      return CheckFeatureErrorHandler(__func__, "Network Setting", device);
    }
  }

  // TSN > IEEE802Dot1Qbv
  if (device_config.GetGCLTables().contains(device.GetId())) {
    for (auto entry : device_config.GetGCLTables()[device.GetId()].GetInterfacesGateParameters()) {
      // check fist enable
      if (entry.GetGateParameters().GetGateEnabled() == true) {
        if (!feature_group.GetConfiguration().GetTSN().GetIEEE802Dot1Qbv()) {
          return CheckFeatureErrorHandler(__func__, "Time Slot Setting(Qbv)", device);
        }
        break;
      }
    }
  }

  // TSN > IEEE802Dot1CB
  if (device_config.GetCbTables().contains(device.GetId())) {
    // check configuration not empty (skip default configuration)
    if (!device_config.GetCbTables()[device.GetId()].GetStreamIdentityList().isEmpty()) {
      if (!feature_group.GetConfiguration().GetTSN().GetIEEE802Dot1CB()) {
        return CheckFeatureErrorHandler(__func__, "CB", device);
      }
    }
  }

  // STPRSTP
  if (device_config.GetRstpTables().contains(device.GetId())) {
    if (!feature_group.GetConfiguration().GetSTPRSTP().GetRSTP()) {
      return CheckFeatureErrorHandler(__func__, "RSTP", device);
    }
  }

  // VLANSetting
  if (device_config.GetVlanTables().contains(device.GetId())) {
    // Port Type
    if (!device_config.GetVlanTables()[device.GetId()].GetVlanPortTypeEntries().isEmpty()) {
      // VLANSetting > AccessTrunkMode
      if (!feature_group.GetConfiguration().GetVLANSetting().GetAccessTrunkMode()) {
        return CheckFeatureErrorHandler(__func__, "Port Type(Access/Trunk)", device);
      }

      // VLANSetting > HybridMode
      for (auto entry : device_config.GetVlanTables()[device.GetId()].GetVlanPortTypeEntries()) {
        // check fist Hybrid port
        if (entry.GetVlanPortType() == ActVlanPortTypeEnum::kHybrid) {
          if (!feature_group.GetConfiguration().GetVLANSetting().GetHybridMode()) {
            return CheckFeatureErrorHandler(__func__, "Port Type(Hybrid)", device);
          }
          break;
        }
      }
    }

    // VLANSetting > DefaultPVID
    // if (!device_config.GetVlanTables()[device.GetId()].GetPortVlanEntries().isEmpty()) {
    //   if (!feature_group.GetConfiguration().GetVLANSetting().GetDefaultPVID()) {
    //     return CheckFeatureErrorHandler(__func__, "PVID", device);
    //   }
    // }
    for (auto entry : device_config.GetVlanTables()[device.GetId()].GetPortVlanEntries()) {
      // check fist not PVID(1)
      if (entry.GetPVID() != ACT_VLAN_INIT_PVID) {
        if (!feature_group.GetConfiguration().GetVLANSetting().GetDefaultPVID()) {
          return CheckFeatureErrorHandler(__func__, "PVID", device);
        }
        break;
      }
    }

    // VLANSetting > TEMSTID (in VLAN StaticEntries)
    for (auto entry : device_config.GetVlanTables()[device.GetId()].GetVlanStaticEntries()) {
      // check fist TEMSTID enable
      if (entry.GetTeMstid()) {
        if (!feature_group.GetConfiguration().GetVLANSetting().GetTEMSTID()) {
          return CheckFeatureErrorHandler(__func__, "TE-MSTID", device);
        }
        break;
      }
    }
  }

  // VLANSetting > DefaultPCP
  if (device_config.GetPortDefaultPCPTables().contains(device.GetId())) {
    for (auto entry : device_config.GetPortDefaultPCPTables()[device.GetId()].GetDefaultPriorityEntries()) {
      // check fist not PCP(0)
      if (entry.GetDefaultPCP() != ACT_INIT_DEFAULT_PCP) {
        if (!feature_group.GetConfiguration().GetVLANSetting().GetDefaultPCP()) {
          return CheckFeatureErrorHandler(__func__, "Default PCP", device);
        }
        break;
      }
    }
  }

  // VLANSetting > PerStreamPriority(Ingress)
  if (device_config.GetStreamPriorityIngressTables().contains(device.GetId())) {
    // check ingress configuration not empty (skip default configuration)
    auto stad_ingress_table = device_config.GetStreamPriorityIngressTables()[device.GetId()];
    if (!stad_ingress_table.GetInterfaceStadPortEntries().isEmpty()) {
      // V2 configuration
      if (stad_ingress_table.IsPerStreamPriorityV2()) {
        if (!feature_group.GetConfiguration().GetVLANSetting().GetPerStreamPriorityV2()) {
          return CheckFeatureErrorHandler(__func__, "L3 Per-Stream Priority", device);
        }
      } else {  // V1 configuration (Can use V1 or V2 deploy)
        if ((!feature_group.GetConfiguration().GetVLANSetting().GetPerStreamPriority()) &&
            (!feature_group.GetConfiguration().GetVLANSetting().GetPerStreamPriorityV2())) {
          return CheckFeatureErrorHandler(__func__, "Per-Stream Priority", device);
        }
      }
    }
  }

  // VLANSetting > PerStreamPriority(Egress)
  if (device_config.GetStreamPriorityEgressTables().contains(device.GetId())) {
    for (auto entry : device_config.GetStreamPriorityEgressTables()[device.GetId()].GetStadConfigEntries()) {
      // check fist not disable configuration
      if (entry.GetEgressUntag() == 1) {  // true(1)
        if (!feature_group.GetConfiguration()
                 .GetVLANSetting()
                 .GetPerStreamPriority()) {  // PerStreamPriorityV2 not support egress
          return CheckFeatureErrorHandler(__func__, "Per-Stream Priority", device);
        }
        break;
      }
    }
  }

  // StaticForwardSetting > Unicast
  if (device_config.GetUnicastStaticForwardTables().contains(device.GetId())) {
    // check configuration not empty (skip default configuration)
    if (!device_config.GetUnicastStaticForwardTables()[device.GetId()].GetStaticForwardEntries().isEmpty()) {
      if (!feature_group.GetConfiguration().GetStaticForwardSetting().GetUnicast()) {
        return CheckFeatureErrorHandler(__func__, "Unicast Static Forward", device);
      }
    }
  }

  // StaticForwardSetting > Multicast
  if (device_config.GetMulticastStaticForwardTables().contains(device.GetId())) {
    // check configuration not empty (skip default configuration)
    if (!device_config.GetMulticastStaticForwardTables()[device.GetId()].GetStaticForwardEntries().isEmpty()) {
      if (!feature_group.GetConfiguration().GetStaticForwardSetting().GetMulticast()) {
        return CheckFeatureErrorHandler(__func__, "Multicast Static Forward", device);
      }
    }
  }

  return act_status;
}
