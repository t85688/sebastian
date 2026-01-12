#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QSet>
#include <thread>

#include "act_device_configuration.hpp"

namespace act {
namespace core {

void ActCore::OpcUaStartIpConfigurationThread(qint64 project_id, QList<ActDeviceIpConfiguration> dev_ip_config_list,
                                              bool from_broadcast_search, std::future<void> signal_receiver,
                                              void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;

    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }
  auto backup_project = project;
  // Update Account
  // [bugfix:1750] Broadcast Search - Missing device password in the config IP stage
  // [bugfix:1844] Hide Connection config
  QList<ActDeviceIpConfiguration> new_dev_ip_config_list;
  if (from_broadcast_search) {
    qDebug() << __func__ << "From broadcast_search to trigger StartSetNetworkSetting";

    // Assign the password from core or project
    QMap<qint64, ActDeviceIpConnectConfig> dev_ip_conn_cfg_map;
    act::core::g_core.GetProjectDevIpConnCfgMap(project_id, dev_ip_conn_cfg_map);
    for (auto dev_ip_config : dev_ip_config_list) {
      // Set Password
      if (dev_ip_conn_cfg_map.contains(dev_ip_config.GetId())) {  // found
        qDebug() << __func__
                 << QString("Device(%1) use core_project saved restful config.")
                        .arg(dev_ip_config.GetId())
                        .toStdString()
                        .c_str();
        dev_ip_config.SetAccount(dev_ip_conn_cfg_map[dev_ip_config.GetId()].GetAccount());
        dev_ip_config.SetSnmpConfiguration(dev_ip_conn_cfg_map[dev_ip_config.GetId()].GetSnmpConfiguration());
        dev_ip_config.SetNetconfConfiguration(dev_ip_conn_cfg_map[dev_ip_config.GetId()].GetNetconfConfiguration());
        dev_ip_config.SetRestfulConfiguration(dev_ip_conn_cfg_map[dev_ip_config.GetId()].GetRestfulConfiguration());
        dev_ip_config.SetEnableSnmpSetting(dev_ip_conn_cfg_map[dev_ip_config.GetId()].GetEnableSnmpSetting());

        // Update ActDeviceIpConnectConfig's IpAddress
        // [bugfix:1780] Broadcast Search finished not connecting to the Auto-Scan page
        dev_ip_conn_cfg_map[dev_ip_config.GetId()].SetIpAddress(dev_ip_config.GetNewIp());

      } else {  // not found, use project snmp config password
        qDebug() << __func__
                 << QString("Device(%1) use project_setting restful config.")
                        .arg(dev_ip_config.GetId())
                        .toStdString()
                        .c_str();
        dev_ip_config.SetAccount(project.GetAccount());
        dev_ip_config.SetSnmpConfiguration(project.GetSnmpConfiguration());
        dev_ip_config.SetNetconfConfiguration(project.GetNetconfConfiguration());
        dev_ip_config.SetRestfulConfiguration(project.GetRestfulConfiguration());
        dev_ip_config.SetEnableSnmpSetting(true);
      }
      new_dev_ip_config_list.append(dev_ip_config);
    }
    // Update dev_ip_conn_cfg_map to core
    // [bugfix:1780] Broadcast Search finished not connecting to the Auto-Scan page
    act::core::g_core.UpdateProjectDevIpConnCfgMap(project_id, dev_ip_conn_cfg_map);
  } else {
    qDebug() << __func__ << "Not from broadcast_search to trigger StartSetNetworkSetting";
    for (auto dev_ip_config : dev_ip_config_list) {
      auto dev_id = dev_ip_config.GetId();
      ActDevice dev;
      act_status = project.GetDeviceById(dev, dev_id);  // find project's device by id
      if (IsActStatusNotFound(act_status)) {            // not found device
        qCritical() << __func__ << "Project has no device id:" << dev_id;

        act_status = std::make_shared<ActStatusNotFound>(QString("Device id %1").arg(dev_id));
        cb_func(act_status, arg);
        this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
        return;
      }
      dev_ip_config.SetOriginIp(dev.GetIpv4().GetIpAddress());
      dev_ip_config.SetAccount(dev.GetAccount());
      dev_ip_config.SetSnmpConfiguration(dev.GetSnmpConfiguration());
      dev_ip_config.SetNetconfConfiguration(dev.GetNetconfConfiguration());
      dev_ip_config.SetRestfulConfiguration(dev.GetRestfulConfiguration());
      dev_ip_config.SetEnableSnmpSetting(dev.GetEnableSnmpSetting());

      new_dev_ip_config_list.append(dev_ip_config);
    }
  }

  // Trigger SetNetworkSetting procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartSetNetworkSetting(project, new_dev_ip_config_list, from_broadcast_search);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start DeviceConfiguration failed";

    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kDeviceConfiguring;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Wait 0.1 second
    signal_receiver.wait_for(std::chrono::milliseconds(100));

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort ip configuration";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  // Dequeue
  QSet<qint64> success_dev_id_set;
  while (!device_configuration.result_queue_.isEmpty()) {
    ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

    if (dev_config_result.GetStatus() == ActStatusType::kSuccess) {
      success_dev_id_set.insert(dev_config_result.GetId());
    }

    act_status->SetErrorMessage(dev_config_result.GetErrorMessage());
    act_status->SetStatus(dev_config_result.GetStatus());
    cb_func(act_status, arg);
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Update project's devices set, if success config
  if (!from_broadcast_search) {
    auto dev_set = project.GetDevices();
    for (auto dev : dev_set) {
      qDebug() << __func__
               << QString("Insert Device(%1) to project's new devices set.").arg(dev.GetId()).toStdString().c_str();

      if (success_dev_id_set.contains(dev.GetId())) {  // update device's ipv4
        auto dev_ipv4 = dev.GetIpv4();
        // new_dev_ip_config_list
        // ActGetListItemIndexById
        qint32 dev_ip_config_index = -1;
        act_status =
            ActGetListItemIndexById<ActDeviceIpConfiguration>(new_dev_ip_config_list, dev.GetId(), dev_ip_config_index);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__
                      << QString(
                             "The target in the new_dev_ip_config_list is not found. "
                             "DeviceIpConfiguration: %1")
                             .arg(dev.GetId())
                             .toStdString()
                             .c_str();
          act_status = std::make_shared<ActStatusNotFound>(QString("Device id %1").arg(dev.GetId()));
          cb_func(act_status, arg);
          this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
          return;
        }
        ActDeviceIpConfiguration new_dev_ip_config = new_dev_ip_config_list.at(dev_ip_config_index);

        dev_ipv4.SetIpAddress(new_dev_ip_config.GetNewIp());
        dev_ipv4.SetGateway(new_dev_ip_config.GetGateway());
        dev_ipv4.SetSubnetMask(new_dev_ip_config.GetSubnetMask());
        dev_ipv4.SetDNS1(new_dev_ip_config.GetDNS1());
        dev_ipv4.SetDNS2(new_dev_ip_config.GetDNS2());
        dev.SetIpv4(dev_ipv4);

        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << project.GetProjectName() << "Cannot update the device";
          cb_func(act_status, arg);
          this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
          return;
        }

        ActDevicePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project.GetId(), dev, true);
        this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);
      }
    }
  }

  // Write back the project status to the core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Cannot update the project";
    cb_func(act_status, arg);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << project.GetProjectName() << "Cannot update the backup project";
    }

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  return;
}

ACT_STATUS ActCore::OpcUaStartIpConfiguration(qint64 &project_id, QList<ActDeviceIpConfiguration> &dev_ip_config_list,
                                              bool from_broadcast_search, std::future<void> signal_receiver,
                                              void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support IP Setting";
    qCritical() << __func__ << error_msg;

    return std::make_shared<ActLicenseNotActiveFailed>("IP Setting");
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << __func__ << project_name << "Spawn OPC UA IpConfiguration thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartIpConfigurationThread, this, project_id,
                                             dev_ip_config_list, from_broadcast_search, std::move(signal_receiver),
                                             cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartIpConfigurationThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartIpConfigurationThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                         qint64 project_id, QList<ActDeviceIpConfiguration> dev_ip_config_list,
                                         bool from_broadcast_search) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, true);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get Operation project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartSetNetworkSetting, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }
  auto backup_project = project;
  // Update Account
  // [bugfix:1750] Broadcast Search - Missing device password in the config IP stage
  // [bugfix:1844] Hide Connection config
  QList<ActDeviceIpConfiguration> new_dev_ip_config_list;
  if (from_broadcast_search) {
    // qDebug() << __func__ << "From broadcast_search to trigger StartSetNetworkSetting";

    // Assign the password from core or project
    QMap<qint64, ActDeviceIpConnectConfig> dev_ip_conn_cfg_map;
    act::core::g_core.GetProjectDevIpConnCfgMap(project_id, dev_ip_conn_cfg_map);
    for (auto dev_ip_config : dev_ip_config_list) {
      // Set Password
      if (dev_ip_conn_cfg_map.contains(dev_ip_config.GetId())) {  // found
        // qDebug() << __func__
        //          << QString("Device(%1) use core_project saved restful config.")
        //                 .arg(dev_ip_config.GetId())
        //                 .toStdString()
        //                 .c_str();
        dev_ip_config.SetAccount(dev_ip_conn_cfg_map[dev_ip_config.GetId()].GetAccount());
        dev_ip_config.SetSnmpConfiguration(dev_ip_conn_cfg_map[dev_ip_config.GetId()].GetSnmpConfiguration());
        dev_ip_config.SetNetconfConfiguration(dev_ip_conn_cfg_map[dev_ip_config.GetId()].GetNetconfConfiguration());
        dev_ip_config.SetRestfulConfiguration(dev_ip_conn_cfg_map[dev_ip_config.GetId()].GetRestfulConfiguration());
        dev_ip_config.SetEnableSnmpSetting(dev_ip_conn_cfg_map[dev_ip_config.GetId()].GetEnableSnmpSetting());

        // Update ActDeviceIpConnectConfig's IpAddress
        // [bugfix:1780] Broadcast Search finished not connecting to the Auto-Scan page
        dev_ip_conn_cfg_map[dev_ip_config.GetId()].SetIpAddress(dev_ip_config.GetNewIp());

      } else {  // not found, use project config password
        // qDebug() << __func__
        //          << QString("Device(%1) use project_setting restful config.")
        //                 .arg(dev_ip_config.GetId())
        //                 .toStdString()
        //                 .c_str();
        dev_ip_config.SetAccount(project.GetAccount());
        dev_ip_config.SetSnmpConfiguration(project.GetSnmpConfiguration());
        dev_ip_config.SetNetconfConfiguration(project.GetNetconfConfiguration());
        dev_ip_config.SetRestfulConfiguration(project.GetRestfulConfiguration());
        dev_ip_config.SetEnableSnmpSetting(true);
      }
      new_dev_ip_config_list.append(dev_ip_config);
    }
    // Update dev_ip_conn_cfg_map to core
    // [bugfix:1780] Broadcast Search finished not connecting to the Auto-Scan page
    act::core::g_core.UpdateProjectDevIpConnCfgMap(project_id, dev_ip_conn_cfg_map);
  } else {
    // qDebug() << __func__ << "Not from broadcast_search to trigger StartSetNetworkSetting";
    for (auto dev_ip_config : dev_ip_config_list) {
      auto dev_id = dev_ip_config.GetId();
      ActDevice dev;
      act_status = project.GetDeviceById(dev, dev_id);  // find project's device by id
      if (IsActStatusNotFound(act_status)) {            // not found device
        qCritical() << __func__ << "Project has no device id:" << dev_id;

        act_status = std::make_shared<ActStatusNotFound>(QString("Device id %1").arg(dev_id));
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartSetNetworkSetting, *act_status);
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
        return;
      }
      dev_ip_config.SetOriginIp(dev.GetIpv4().GetIpAddress());
      dev_ip_config.SetAccount(dev.GetAccount());
      dev_ip_config.SetSnmpConfiguration(dev.GetSnmpConfiguration());
      dev_ip_config.SetNetconfConfiguration(dev.GetNetconfConfiguration());
      dev_ip_config.SetRestfulConfiguration(dev.GetRestfulConfiguration());
      dev_ip_config.SetEnableSnmpSetting(dev.GetEnableSnmpSetting());

      new_dev_ip_config_list.append(dev_ip_config);
    }
  }

  // Trigger SetNetworkSetting procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartSetNetworkSetting(project, new_dev_ip_config_list, from_broadcast_search);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start DeviceConfiguration failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartSetNetworkSetting, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  QSet<qint64> success_dev_id_set;
  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Dequeue
    while (!device_configuration.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

      if (dev_config_result.GetStatus() == ActStatusType::kSuccess) {
        success_dev_id_set.insert(dev_config_result.GetId());
      }

      ActDeviceConfigResultWSResponse ws_resp(ActWSCommandEnum::kStartSetNetworkSetting, ActStatusType::kRunning,
                                              dev_config_result);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartSetNetworkSetting, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }

      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort ip configuration";

    return;
  }

  // Update project's devices set, if success config
  if (!from_broadcast_search) {
    auto dev_set = project.GetDevices();
    for (auto dev : dev_set) {
      // qDebug() << __func__
      //          << QString("Insert Device(%1) to project's new devices set.").arg(dev.GetId()).toStdString().c_str();

      if (success_dev_id_set.contains(dev.GetId())) {  // update device's ipv4
        auto dev_ipv4 = dev.GetIpv4();
        // new_dev_ip_config_list
        // ActGetListItemIndexById
        qint32 dev_ip_config_index = -1;
        act_status =
            ActGetListItemIndexById<ActDeviceIpConfiguration>(new_dev_ip_config_list, dev.GetId(), dev_ip_config_index);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__
                      << QString(
                             "The target in the new_dev_ip_config_list is not found. "
                             "DeviceIpConfiguration: %1")
                             .arg(dev.GetId())
                             .toStdString()
                             .c_str();
          act_status = std::make_shared<ActStatusNotFound>(QString("Device id %1").arg(dev.GetId()));
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
              ActWSResponseErrorTransfer(ActWSCommandEnum::kStartReboot, *act_status);
          this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
          return;
        }
        ActDeviceIpConfiguration new_dev_ip_config = new_dev_ip_config_list.at(dev_ip_config_index);

        dev_ipv4.SetIpAddress(new_dev_ip_config.GetNewIp());
        dev_ipv4.SetGateway(new_dev_ip_config.GetGateway());
        dev_ipv4.SetSubnetMask(new_dev_ip_config.GetSubnetMask());
        dev_ipv4.SetDNS1(new_dev_ip_config.GetDNS1());
        dev_ipv4.SetDNS2(new_dev_ip_config.GetDNS2());
        dev.SetIpv4(dev_ipv4);

        act_status = this->UpdateDevice(project, dev);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << project.GetProjectName() << "Cannot update the device";
          std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
              ActWSResponseErrorTransfer(ActWSCommandEnum::kStartSetNetworkSetting, *act_status);
          this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
          return;
        }

        // Send device update msg

        ActDevicePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project.GetId(), dev, true);
        this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);
      }
    }
  }

  // Write back the project status to the core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Cannot update the project";
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartSetNetworkSetting, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << project.GetProjectName() << "Cannot update the backup project";
    }

    return;
  }

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartSetNetworkSetting, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartIpConfiguration(qint64 &project_id, const qint64 &ws_listener_id,
                                         QList<ActDeviceIpConfiguration> &dev_ip_config_list,
                                         bool from_broadcast_search) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support IP Setting";
    qCritical() << __func__ << error_msg;

    act_status = std::make_shared<ActLicenseNotActiveFailed>("IP Setting");
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartSetNetworkSetting, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartSetNetworkSetting, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << project_name << "Spawn IpConfiguration thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartIpConfigurationThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, dev_ip_config_list, from_broadcast_search);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartIpConfigurationThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::OpcUaStartConfigRebootThread(qint64 project_id, QList<qint64> dev_id_list,
                                           std::future<void> signal_receiver,
                                           void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;

    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  // Trigger Reboot procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartReboot(project, dev_id_list);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start DeviceConfiguration failed";

    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kDeviceConfiguring;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's reboot result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Wait 0.1 second
    signal_receiver.wait_for(std::chrono::milliseconds(100));

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort reboot";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Dequeue
  while (!device_configuration.result_queue_.isEmpty()) {
    ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

    act_status->SetErrorMessage(dev_config_result.GetErrorMessage());
    act_status->SetStatus(dev_config_result.GetStatus());
    cb_func(act_status, arg);
  }

  return;
}

ACT_STATUS ActCore::OpcUaStartConfigReboot(qint64 &project_id, QList<qint64> &dev_id_list,
                                           std::future<void> signal_receiver,
                                           void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support Reboot";
    qCritical() << __func__ << error_msg;
    act_status = std::make_shared<ActLicenseNotActiveFailed>("Reboot");
    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << __func__ << project_name << "Spawn OPC UA Reboot thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartConfigRebootThread, this, project_id,
                                             dev_id_list, std::move(signal_receiver), cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartConfigRebootThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartConfigRebootThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                      qint64 project_id, QList<qint64> dev_id_list) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project, true);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get Operation project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  // Trigger Reboot procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartReboot(project, dev_id_list);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start DeviceConfiguration failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartReboot, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's reboot result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Dequeue
    while (!device_configuration.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

      ActDeviceConfigResultWSResponse ws_resp(ActWSCommandEnum::kStartReboot, ActStatusType::kRunning,
                                              dev_config_result);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartReboot, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort reboot";

    return;
  }

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartReboot, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartConfigReboot(qint64 &project_id, const qint64 &ws_listener_id, QList<qint64> &dev_id_list) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support Reboot";
    qCritical() << __func__ << error_msg;

    act_status = std::make_shared<ActLicenseNotActiveFailed>("Reboot");
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartReboot, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartReboot, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << QString("Project(%1) Spawn Reboot thread...").arg(project_id);
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartConfigRebootThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, dev_id_list);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartConfigRebootThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartConfigLocatorThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                       qint64 project_id, QList<qint64> dev_id_list, quint16 duration) {
  ACT_STATUS_INIT();
  // Waiting for caller thread
  std::this_thread::yield();
  // Check duration in 30 ~ 300
  if ((duration < ACT_LOCATOR_DURATION_MIN) || (duration > ACT_LOCATOR_DURATION_MAX)) {
    QString error_msg = QString("The Duration(%1) should be %2 - %3")
                            .arg(duration)
                            .arg(ACT_LOCATOR_DURATION_MIN)
                            .arg(ACT_LOCATOR_DURATION_MAX);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    act_status = std::make_shared<ActBadRequest>(error_msg);
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartLocator, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }
  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project, true);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get Operation project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  // Trigger Locator procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartLocator(project, dev_id_list, duration);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start Locator failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartLocator, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's Locator result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Dequeue
    while (!device_configuration.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

      ActDeviceConfigResultWSResponse ws_resp(ActWSCommandEnum::kStartLocator, ActStatusType::kRunning,
                                              dev_config_result);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartLocator, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort locator";
    return;
  }

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartLocator, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartConfigLocator(qint64 &project_id, const qint64 &ws_listener_id, QList<qint64> &dev_id_list,
                                       quint16 &duration) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support Locator";
    qCritical() << __func__ << error_msg;

    act_status = std::make_shared<ActLicenseNotActiveFailed>("Locator");
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartLocator, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  } */

  // // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartLocator, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << QString("Project(%1) Spawn Locator thread...").arg(project_id);
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartConfigLocatorThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, dev_id_list, duration);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartConfigLocatorThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::OpcUaStartOperationEventLogThread(qint64 project_id, QList<qint64> dev_id_list, QString export_path,
                                                std::future<void> signal_receiver,
                                                void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    cb_func(act_status, arg);
    return;
  }

  // Trigger Locator procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartGetEventLog(project, dev_id_list);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start GetEventLog failed";

    cb_func(act_status, arg);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kDeviceConfiguring;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's Eventlog result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Dequeue - Successful result
    while (!device_configuration.event_log_result_queue_.isEmpty()) {
      ActDeviceEventLogResult dev_event_log_result = device_configuration.event_log_result_queue_.dequeue();
      ActDeviceEventLog &response = dev_event_log_result.GetResponse();
      QList<ActDeviceEventLogEntry> &entries = response.GetEntries();
      qint64 &device_id = response.GetDeviceId();

      ActDevice device;
      act_status = project.GetDeviceById(device, device_id);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "Get device failed with device id:" << device_id;
        cb_func(act_status, arg);
        return;
      }

      // save to csv
      QString file_name = QString("%1/%2_%3_EventLogTable_%4.csv")
                              .arg(export_path)
                              .arg(device.GetIpv4().GetIpAddress())
                              .arg(device.GetDeviceProperty().GetModelName())
                              .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmm"));
      QFile file(file_name);
      if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setCodec("UTF-8");

        // Get columns key
        if (!entries.isEmpty()) {
          QJsonObject firstObj = entries[0].toJson();
          QStringList keys = firstObj.keys();

          // Write columns key to header
          out << keys.join(",") << "\n";

          // Write event log entries
          for (const ActDeviceEventLogEntry &entry : entries) {
            QJsonObject obj = entry.toJson();
            QStringList row;
            for (const QString &key : keys) {
              QString val = obj.value(key).toString();
              if (val.contains(',')) {
                val = "\"" + val + "\"";
              }
              row << val;
            }
            out << row.join(",") << "\n";
          }
        }
        file.close();
      }

      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort GetEventLog";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;
  cb_func(ACT_STATUS_SUCCESS, arg);

  return;
}

ACT_STATUS ActCore::OpcUaStartOperationEventLog(qint64 &project_id, QList<qint64> &dev_id_list, QString &export_path,
                                                std::future<void> signal_receiver,
                                                void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support EventLog";
    qCritical() << __func__ << error_msg;

    return std::make_shared<ActBadRequest>(error_msg);
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << __func__ << project_name << "Spawn OPC UA EventLog thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartOperationEventLogThread, this, project_id,
                                             dev_id_list, export_path, std::move(signal_receiver), cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartOperationEventLogThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartOperationEventLogThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                           qint64 project_id, QList<qint64> dev_id_list) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project, true);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get Operation project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartGetEventLog, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  // Trigger Locator procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartGetEventLog(project, dev_id_list);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start GetEventLog failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartGetEventLog, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's Eventlog result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Dequeue - Successful result
    while (!device_configuration.event_log_result_queue_.isEmpty()) {
      ActDeviceEventLogResult dev_event_log_result = device_configuration.event_log_result_queue_.dequeue();

      ActDeviceEventLogResultWSResponse ws_resp(ActWSCommandEnum::kStartGetEventLog, ActStatusType::kRunning,
                                                dev_event_log_result);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      // ActDeviceEventLog & result_event_log
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    // Dequeue - Failed result
    while (!device_configuration.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

      ActDeviceConfigResultWSResponse ws_resp(ActWSCommandEnum::kStartGetEventLog, ActStatusType::kRunning,
                                              dev_config_result);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartGetEventLog, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort GetEventLog";

    return;
  }

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartGetEventLog, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartOperationEventLog(qint64 &project_id, const qint64 &ws_listener_id,
                                           QList<qint64> &dev_id_list) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support EventLog";
    qCritical() << __func__ << error_msg;

    act_status = std::make_shared<ActLicenseNotActiveFailed>("EventLog");
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartGetEventLog, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartGetEventLog, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << project_name << "Spawn EventLog thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartOperationEventLogThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, dev_id_list);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OperationEventLogThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::OpcUaStartConfigFactoryDefaultThread(qint64 project_id, QList<qint64> dev_id_list,
                                                   std::future<void> signal_receiver,
                                                   void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;

    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  // Trigger Reboot procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartFactoryDefault(project, dev_id_list);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start DeviceConfiguration failed";

    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kDeviceConfiguring;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's reboot result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Wait 0.1 second
    signal_receiver.wait_for(std::chrono::milliseconds(100));

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort factory default";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Dequeue
  while (!device_configuration.result_queue_.isEmpty()) {
    ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

    act_status->SetErrorMessage(dev_config_result.GetErrorMessage());
    act_status->SetStatus(dev_config_result.GetStatus());
    cb_func(act_status, arg);
  }

  return;
}

ACT_STATUS ActCore::OpcUaStartConfigFactoryDefault(qint64 &project_id, QList<qint64> &dev_id_list,
                                                   std::future<void> signal_receiver,
                                                   void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support Factory Default";
    qCritical() << __func__ << error_msg;
    act_status = std::make_shared<ActLicenseNotActiveFailed>("Factory Default");
    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << __func__ << project_name << "Spawn OPC UA Factory Default thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartConfigFactoryDefaultThread, this,
                                             project_id, dev_id_list, std::move(signal_receiver), cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartConfigFactoryDefaultThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartConfigFactoryDefaultThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                              qint64 project_id, QList<qint64> dev_id_list) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project, true);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get Operation project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  // Trigger FactoryDefault procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartFactoryDefault(project, dev_id_list);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start DeviceConfiguration failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartFactoryDefault, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's FactoryDefault result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Dequeue
    while (!device_configuration.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

      ActDeviceConfigResultWSResponse ws_resp(ActWSCommandEnum::kStartFactoryDefault, ActStatusType::kRunning,
                                              dev_config_result);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartFactoryDefault, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }

      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort Factory Default";
    return;
  }

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartFactoryDefault, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartConfigFactoryDefault(qint64 &project_id, const qint64 &ws_listener_id,
                                              QList<qint64> &dev_id_list) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support Factory Default";
    qCritical() << __func__ << error_msg;

    act_status = std::make_shared<ActLicenseNotActiveFailed>("Factory Default");
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartFactoryDefault, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartFactoryDefault, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << project_name << "Spawn FactoryDefault thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartConfigFactoryDefaultThread, this,
                                    std::cref(ws_listener_id), std::move(signal_receiver), project_id, dev_id_list);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartConfigFactoryDefaultThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::OpcUaStartConfigFirmwareUpgradeThread(qint64 project_id, QList<qint64> dev_id_list,
                                                    QString firmware_file_path, std::future<void> signal_receiver,
                                                    void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;

    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  // Trigger Reboot procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartFirmwareUpgrade(project, dev_id_list, firmware_file_path);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start DeviceConfiguration failed";

    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kDeviceConfiguring;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's reboot result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Wait 0.1 second
    signal_receiver.wait_for(std::chrono::milliseconds(100));

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort factory default";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Dequeue
  while (!device_configuration.result_queue_.isEmpty()) {
    ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

    act_status->SetErrorMessage(dev_config_result.GetErrorMessage());
    act_status->SetStatus(dev_config_result.GetStatus());
    cb_func(act_status, arg);
  }

  return;
}

ACT_STATUS ActCore::OpcUaStartConfigFirmwareUpgrade(qint64 &project_id, QList<qint64> &dev_id_list,
                                                    QString &firmware_file_path, std::future<void> signal_receiver,
                                                    void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support Firmware Upgrade";
    qCritical() << __func__ << error_msg;
    act_status = std::make_shared<ActLicenseNotActiveFailed>("Firmware Upgrade");
    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << __func__ << project_name << "Spawn OPC UA Firmware Upgrade thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartConfigFirmwareUpgradeThread, this, project_id,
                                    dev_id_list, firmware_file_path, std::move(signal_receiver), cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartConfigFirmwareUpgradeThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartConfigFirmwareUpgradeThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                               qint64 project_id, QList<qint64> dev_id_list, QString firmware_name) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Find firmware by firmware_name
  QSet<ActFirmware> firmware_set = this->GetFirmwareSet();
  auto fw_iterator = firmware_set.find(ActFirmware(firmware_name));
  if (fw_iterator == firmware_set.end()) {
    act_status = std::make_shared<ActStatusNotFound>(QString("Firmware(%1) not found").arg(firmware_name));
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartFirmwareUpgrade, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project, true);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get Operation project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  // Trigger FirmwareUpgrade procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  QString file_path = QString("%1/%2").arg(ACT_FIRMWARE_FILE_FOLDER).arg(firmware_name);
  act_status = device_configuration.StartFirmwareUpgrade(project, dev_id_list, file_path);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start DeviceConfiguration failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartFirmwareUpgrade, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    this->DeleteFirmware(fw_iterator->GetId());
    return;
  }

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's FirmwareUpgrade result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Dequeue
    while (!device_configuration.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

      ActDeviceConfigResultWSResponse ws_resp(ActWSCommandEnum::kStartFirmwareUpgrade, ActStatusType::kRunning,
                                              dev_config_result);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartFirmwareUpgrade, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }

      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort Firmware Upgrade";
    return;
  }

  // Delete Firmware
  this->DeleteFirmware(fw_iterator->GetId());

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartFirmwareUpgrade, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartConfigFirmwareUpgrade(qint64 &project_id, const qint64 &ws_listener_id,
                                               QList<qint64> &dev_id_list, QString &firmware_name) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support Firmware Upgrade";
    qCritical() << __func__ << error_msg;

    act_status = std::make_shared<ActLicenseNotActiveFailed>("Firmware Upgrade");
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartFirmwareUpgrade, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartFirmwareUpgrade, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << project_name << "Spawn FirmwareUpgrade thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::StartConfigFirmwareUpgradeThread, this,
                                             std::cref(ws_listener_id), std::move(signal_receiver), project_id,
                                             dev_id_list, firmware_name);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartConfigFirmwareUpgradeThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartConfigEnableSnmpThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                          qint64 project_id, QList<qint64> dev_id_list) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  bool is_operation = true;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get Operation project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  // Trigger EnableSnmp procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartEnableSnmp(project, dev_id_list);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start DeviceConfiguration failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartEnableSnmp, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's EnableSnmp result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();
    // Dequeue
    while (!device_configuration.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

      ActDeviceConfigResultWSResponse ws_resp(ActWSCommandEnum::kStartEnableSnmp, ActStatusType::kRunning,
                                              dev_config_result);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartEnableSnmp, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort enable SNMP";

    return;
  }

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartEnableSnmp, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartConfigEnableSnmp(qint64 &project_id, const qint64 &ws_listener_id,
                                          QList<qint64> &dev_id_list) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support Enable SNMP";
    qCritical() << __func__ << error_msg;

    act_status = std::make_shared<ActLicenseNotActiveFailed>("Enable SNMP");
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartEnableSnmp, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartEnableSnmp, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << project_name << "Spawn EnableSnmp thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartConfigEnableSnmpThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, dev_id_list);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartConfigEnableSnmpThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartDeviceConfigCommissionThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                                qint64 project_id, QList<qint64> dev_id_list,
                                                ActDeviceConfigTypeEnum config_type) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  bool is_operation = true;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get Operation project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  // Trigger Commission procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  ActDeviceConfig sync_dev_config;
  act_status = device_configuration.StartCommission(project, dev_id_list, config_type, sync_dev_config);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start Commission failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's reboot result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Dequeue
    while (!device_configuration.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

      ActDeviceConfigResultWSResponse ws_resp(ActWSCommandEnum::kStartDeviceConfig, ActStatusType::kRunning,
                                              dev_config_result);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceConfig, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // Update the syn_dev_config to DeviceConfig
  for (auto dev_id : dev_id_list) {
    UpdateDeviceDeviceConfig(project, sync_dev_config, dev_id);
  }

  // Sync Device data
  if (config_type == ActDeviceConfigTypeEnum::kNetworkSetting) {
    //  Ipv4
    for (auto dev_id : sync_dev_config.GetNetworkSettingTables().keys()) {
      ActDevice dev;
      act_status = project.GetDeviceById(dev, dev_id);
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__ << "Not found the device in the Monitor Project";
        continue;
      }
      auto cfg_table = sync_dev_config.GetNetworkSettingTables()[dev_id];
      dev.GetIpv4().SetIpAddress(cfg_table.GetIpAddress());
      dev.GetIpv4().SetSubnetMask(cfg_table.GetSubnetMask());
      dev.GetIpv4().SetGateway(cfg_table.GetGateway());
      dev.GetIpv4().SetDNS1(cfg_table.GetDNS1());
      dev.GetIpv4().SetDNS2(cfg_table.GetDNS2());
      act_status = act::core::g_core.UpdateDevice(project, dev);
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__ << "Cannot update device:" << dev.ToString().toStdString().c_str();
        continue;
      }
      // Notify the user that the device be update
      ActDevicePatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, project_id, dev, true);
      this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project_id);
    }
  } else if (config_type == ActDeviceConfigTypeEnum::kInformationSetting) {
    // DeviceName & Location & Description
    for (auto dev_id : sync_dev_config.GetInformationSettingTables().keys()) {
      ActDevice dev;
      act_status = project.GetDeviceById(dev, dev_id);
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__ << "Not found the device in the Monitor Project";
        continue;
      }
      auto cfg_table = sync_dev_config.GetInformationSettingTables()[dev_id];
      dev.SetDeviceName(cfg_table.GetDeviceName());
      dev.GetDeviceInfo().SetLocation(cfg_table.GetLocation());
      dev.GetDeviceProperty().SetDescription(cfg_table.GetDescription());
      act_status = act::core::g_core.UpdateDevice(project, dev);
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__ << "Cannot update device:" << dev.ToString().toStdString().c_str();
        continue;
      }
      // Notify the user that the device be update
      ActDevicePatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, project_id, dev, true);
      this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project_id);
    }
  }

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qWarning() << __func__ << project.GetProjectName() << "Abort DeviceConfig commission";

    return;
  }

  // // Write back the project to the core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Cannot update the project";
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartDeviceConfig, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartDeviceConfigCommission(qint64 &project_id, const qint64 &ws_listener_id,
                                                const QList<qint64> &dev_id_list,
                                                const ActDeviceConfigTypeEnum &config_type) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support DeviceConfig";
    qCritical() << __func__ << error_msg;

    act_status = std::make_shared<ActLicenseNotActiveFailed>("DeviceConfig");
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << project_name << "Spawn Commission thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::StartDeviceConfigCommissionThread, this,
                                             std::cref(ws_listener_id), std::move(signal_receiver), project_id,
                                             dev_id_list, config_type);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartDeviceConfigCommissionThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartDeviceCommandLineThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                           qint64 project_id, QList<qint64> dev_id_list, QString command) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  bool is_operation = true;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get Operation project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceCommandLine, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  // Trigger Reboot procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartCommandLine(project, dev_id_list, command);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start DeviceConfiguration failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceCommandLine, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's reboot result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Dequeue
    while (!device_configuration.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

      ActDeviceConfigResultWSResponse ws_resp(ActWSCommandEnum::kStartDeviceCommandLine, ActStatusType::kRunning,
                                              dev_config_result);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceCommandLine, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort reboot";
    return;
  }

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartDeviceCommandLine, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartDeviceCommandLine(qint64 &project_id, const qint64 &ws_listener_id,
                                           const QList<qint64> &dev_id_list, const QString &command) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceCommandLine, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << project_name << "Spawn Command Line thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartDeviceCommandLineThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, dev_id_list, command);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartDeviceCommandLineThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

}  // namespace core
}  // namespace act
