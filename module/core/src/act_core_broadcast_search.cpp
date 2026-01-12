#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QSet>
#include <thread>

#include "act_broadcast_search.hpp"
#include "act_core.hpp"

namespace act {
namespace core {

void ActCore::OpcUaStartDeviceDiscoveryThread(qint64 project_id, ActDeviceDiscoveryConfig dev_discovery_cfg,
                                              std::future<void> signal_receiver,
                                              void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;

    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }
  auto backup_project = project;

  QList<ActDevice> result_devices;
  // Trigger DeviceDiscovery procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::topology::ActBroadcastSearch broadcast_search(profiles);

  // [feat:1662] Hide password
  // If the input is empty, which means it should use default setting
  this->HandleConnectionConfigField(dev_discovery_cfg, project.GetProjectSetting(), project.GetProjectSetting());

  act_status = broadcast_search.StartDeviceDiscovery(dev_discovery_cfg, result_devices);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start DeviceDiscovery failed";

    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kBroadcastSearching;

  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = broadcast_search.GetStatus();

    if (act_status->GetStatus() != ActStatusType::kFinished) {
      qDebug() << act_status->ToString().toStdString().c_str();
      cb_func(act_status, arg);
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    broadcast_search.Stop();
    qCritical() << project.GetProjectName() << "Abort device discovery";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  project.broadcast_search_devices_ = result_devices;  // save result_devices to project
  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Write back the project status to the core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
    cb_func(act_status, arg);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << project.GetProjectName() << "Cannot update the backup project";
    }

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  // Send finished status reply to client
  // [feat:1662] Hide password
  QMap<qint64, ActDeviceIpConnectConfig> dev_ip_conn_cfg_map;
  QList<ActDevice> result_devices_without_password;
  for (auto dev : result_devices) {
    // Update dev_ip_conn_cfg_map
    // [bugfix:1750] Broadcast Search - Missing device password in the config IP stage
    ActDeviceIpConnectConfig dev_ip_conn_cfg;
    dev_ip_conn_cfg.SetIpAddress(dev.GetIpv4().GetIpAddress());
    dev_ip_conn_cfg.SetEnableSnmpSetting(dev.GetEnableSnmpSetting());
    act::core::g_core.CopyConnectionConfigField(dev_ip_conn_cfg, dev);

    dev_ip_conn_cfg_map[dev.GetId()] = dev_ip_conn_cfg;

    // Hide Password
    dev.HidePassword();
    result_devices_without_password.push_back(dev);
  }
  // Update dev_ip_conn_cfg_map to core
  act::core::g_core.UpdateProjectDevIpConnCfgMap(project_id, dev_ip_conn_cfg_map);

  // ActDeviceDiscoveryFinishedStatus finished_status;
  // finished_status.SetDevices(result_devices_without_password);
  // // Send finished status reply to client
  // qDebug() << finished_status.ToString().toStdString().c_str();
  // this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,ws_resp, ws_listener_id);

  act_status = broadcast_search.GetStatus();
  cb_func(act_status, arg);

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;
  return;
}

ACT_STATUS ActCore::OpcUaStartDeviceDiscovery(qint64 &project_id, ActDeviceDiscoveryConfig &dev_discovery_cfg,
                                              std::future<void> signal_receiver,
                                              void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kBroadcastSearching, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << project_name << "Spawn DeviceDiscovery thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartDeviceDiscoveryThread, this, project_id,
                                             dev_discovery_cfg, std::move(signal_receiver), cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartDeviceDiscoveryThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

ACT_STATUS ActCore::OpcUaStartLinkSequenceDetect(qint64 &project_id, std::future<void> signal_receiver,
                                                 void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kBroadcastSearching, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << project_name << "Spawn OPC UA LinkSequenceDetect thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartLinkSequenceDetectThread, this, project_id,
                                             std::move(signal_receiver), cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartLinkSequenceDetectThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::OpcUaStartLinkSequenceDetectThread(qint64 project_id, std::future<void> signal_receiver,
                                                 void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  // TODO: Add from_broadcast_search & select_dev_id_list parameters from function input
  bool from_broadcast_search = true;
  QList<qint64> select_dev_id_list;

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }
  auto backup_project = project;
  // Trigger LinkSequenceDetect procedure
  QList<ActDeviceDistanceEntry> distance_entry_list;
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::topology::ActBroadcastSearch broadcast_search(profiles, project.broadcast_search_devices_);
  act_status = broadcast_search.StartLinkDistanceDetect(from_broadcast_search, select_dev_id_list, distance_entry_list);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start LinkDistanceDetect failed";
    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kBroadcastSearching;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = broadcast_search.GetStatus();

    if (act_status->GetStatus() != ActStatusType::kFinished) {
      qDebug() << act_status->ToString().toStdString().c_str();
      cb_func(act_status, arg);
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    broadcast_search.Stop();
    qCritical() << project.GetProjectName() << "Abort link sequence detection";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  project.broadcast_search_devices_ = broadcast_search.GetDevices();
  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Write back the project status to the core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
    cb_func(act_status, arg);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << project.GetProjectName() << "Cannot update the backup project";
    }

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  act_status = broadcast_search.GetStatus();
  cb_func(act_status, arg);

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;
  return;
}

ACT_STATUS ActCore::OpcUaStartRetryConnect(qint64 &project_id, ActRetryConnectConfig &retry_connect_cfg,
                                           std::future<void> signal_receiver,
                                           void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kBroadcastSearching, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << project_name << "Spawn OPC UA RetryConnect thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartRetryConnectThread, this, project_id,
                                             retry_connect_cfg, std::move(signal_receiver), cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartRetryConnectThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::OpcUaStartRetryConnectThread(qint64 project_id, ActRetryConnectConfig retry_connect_cfg,
                                           std::future<void> signal_receiver,
                                           void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }
  auto backup_project = project;
  QList<ActDevice> result_devices;
  // Trigger DeviceDiscovery procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::topology::ActBroadcastSearch broadcast_search(profiles, project.broadcast_search_devices_);
  // broadcast_search.SetDevices(project.broadcast_search_devices_);

  // [feat:1662] Hide password
  // If the input is empty, which means it should use default setting
  this->HandleConnectionConfigField(retry_connect_cfg, project.GetProjectSetting(), project.GetProjectSetting());

  act_status = broadcast_search.StartRetryConnect(retry_connect_cfg, result_devices);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start RetryConnect failed";
    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kBroadcastSearching;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = broadcast_search.GetStatus();

    if (act_status->GetStatus() != ActStatusType::kFinished) {
      qDebug() << act_status->ToString().toStdString().c_str();
      cb_func(act_status, arg);
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    broadcast_search.Stop();
    qCritical() << project.GetProjectName() << "Abort retry connect";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  project.broadcast_search_devices_ = broadcast_search.GetDevices();
  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Write back the project status to the core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
    cb_func(act_status, arg);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << project.GetProjectName() << "Cannot update the backup project";
    }

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  // Send finished status reply to client
  // [feat:1662] Hide password
  QMap<qint64, ActDeviceIpConnectConfig> dev_ip_conn_cfg_map;
  act::core::g_core.GetProjectDevIpConnCfgMap(project_id, dev_ip_conn_cfg_map);
  QList<ActDevice> result_devices_without_password;
  for (auto dev : result_devices) {
    // Update password to dev_ip_conn_cfg_map
    // [bugfix:1750] Broadcast Search - Missing device password in the config IP stage
    ActDeviceIpConnectConfig dev_ip_conn_cfg;
    dev_ip_conn_cfg.SetIpAddress(dev.GetIpv4().GetIpAddress());
    dev_ip_conn_cfg.SetEnableSnmpSetting(dev.GetEnableSnmpSetting());
    act::core::g_core.CopyConnectionConfigField(dev_ip_conn_cfg, dev);
    dev_ip_conn_cfg_map[dev.GetId()] = dev_ip_conn_cfg;

    // Hide Password
    dev.HidePassword();
    result_devices_without_password.push_back(dev);
  }
  // Update dev_ip_conn_cfg_map to core
  act::core::g_core.UpdateProjectDevIpConnCfgMap(project_id, dev_ip_conn_cfg_map);

  // ActDevicesResultWSResponse ws_resp(ActWSCommandEnum::kStartRetryConnect, result_devices_without_password);
  // qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  // this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,ws_resp, ws_listener_id);

  act_status = broadcast_search.GetStatus();
  cb_func(act_status, arg);

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;
  return;
}

void ActCore::StartDeviceDiscoveryThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                         qint64 project_id, ActDeviceDiscoveryConfig dev_discovery_cfg) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceDiscovery, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  auto backup_project = project;
  QList<ActDevice> result_devices;
  // Trigger DeviceDiscovery procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::topology::ActBroadcastSearch broadcast_search(profiles);

  // [feat:1662] Hide password
  // If the input is empty, which means it should use default setting
  this->HandleConnectionConfigField(dev_discovery_cfg, project.GetProjectSetting(), project.GetProjectSetting());

  act_status = broadcast_search.StartDeviceDiscovery(dev_discovery_cfg, result_devices);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start DeviceDiscovery failed";
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceDiscovery, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kBroadcastSearching;

  quint8 previous_progress = 255;  // Initialize previous progress to an invalid value

  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = broadcast_search.GetStatus();

    if (act_status->GetStatus() != ActStatusType::kFinished) {
      if (act_status->GetStatus() != ActStatusType::kRunning) {  // error

        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceDiscovery, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      } else {
        ActProgressWSResponse ws_resp(ActWSCommandEnum::kStartDeviceDiscovery,
                                      dynamic_cast<ActProgressStatus &>(*act_status));
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

        quint8 current_progress = ws_resp.GetData().GetProgress();
        if (current_progress != previous_progress) {
          qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
          previous_progress = current_progress;  // Update previous progress
        }
      }
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    broadcast_search.Stop();
    qCritical() << project.GetProjectName() << "Abort device discovery";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  project.broadcast_search_devices_ = result_devices;  // save result_devices to project
  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Write back the project status to the core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceDiscovery, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << project.GetProjectName() << "Cannot update the backup project";
    }
    return;
  }

  // Send finished status reply to client
  // [feat:1662] Hide password
  QMap<qint64, ActDeviceIpConnectConfig> dev_ip_conn_cfg_map;
  QList<ActDevice> result_devices_without_password;
  for (auto dev : result_devices) {
    // Update dev_ip_conn_cfg_map
    // [bugfix:1750] Broadcast Search - Missing device password in the config IP stage
    ActDeviceIpConnectConfig dev_ip_conn_cfg;
    dev_ip_conn_cfg.SetIpAddress(dev.GetIpv4().GetIpAddress());
    dev_ip_conn_cfg.SetEnableSnmpSetting(dev.GetEnableSnmpSetting());
    act::core::g_core.CopyConnectionConfigField(dev_ip_conn_cfg, dev);

    dev_ip_conn_cfg_map[dev.GetId()] = dev_ip_conn_cfg;

    // Hide Password
    dev.HidePassword();
    result_devices_without_password.push_back(dev);
  }
  // Update dev_ip_conn_cfg_map to core
  act::core::g_core.UpdateProjectDevIpConnCfgMap(project_id, dev_ip_conn_cfg_map);

  ActDevicesResultWSResponse ws_resp(ActWSCommandEnum::kStartDeviceDiscovery, result_devices_without_password);
  QList<QString> dump_key_order{"OpCode", "StatusCode"};
  qDebug() << ws_resp.ToString(dump_key_order).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  // ActDeviceDiscoveryFinishedStatus finished_status;
  // finished_status.SetDevices(result_devices_without_password);
  // // Send finished status reply to client
  // qDebug() << finished_status.ToString().toStdString().c_str();
  // this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, finished_status, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartDeviceDiscovery(qint64 &project_id, const qint64 &ws_listener_id,
                                         ActDeviceDiscoveryConfig &dev_discovery_cfg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kBroadcastSearching, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeviceDiscovery, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << project_name << "Spawn DeviceDiscovery thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartDeviceDiscoveryThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, dev_discovery_cfg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"DeviceDiscoveryThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartRetryConnectThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                      qint64 project_id, ActRetryConnectConfig retry_connect_cfg) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartRetryConnect, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }
  auto backup_project = project;

  QList<ActDevice> result_devices;
  // Trigger DeviceDiscovery procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::topology::ActBroadcastSearch broadcast_search(profiles, project.broadcast_search_devices_);
  // broadcast_search.SetDevices(project.broadcast_search_devices_);

  // [feat:1662] Hide password
  // If the input is empty, which means it should use default setting
  this->HandleConnectionConfigField(retry_connect_cfg, project.GetProjectSetting(), project.GetProjectSetting());

  act_status = broadcast_search.StartRetryConnect(retry_connect_cfg, result_devices);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start DeviceDiscovery failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartRetryConnect, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kBroadcastSearching;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = broadcast_search.GetStatus();

    if (act_status->GetStatus() != ActStatusType::kFinished) {
      if (act_status->GetStatus() != ActStatusType::kRunning) {  // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartRetryConnect, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      } else {
        ActProgressWSResponse ws_resp(ActWSCommandEnum::kStartRetryConnect,
                                      dynamic_cast<ActProgressStatus &>(*act_status));
        qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    broadcast_search.Stop();
    qCritical() << project.GetProjectName() << "Abort retry connection";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  project.broadcast_search_devices_ = broadcast_search.GetDevices();
  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Write back the project status to the core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartRetryConnect, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << project.GetProjectName() << "Cannot update the backup project";
    }

    return;
  }

  // Send finished status reply to client
  // [feat:1662] Hide password
  QMap<qint64, ActDeviceIpConnectConfig> dev_ip_conn_cfg_map;
  act::core::g_core.GetProjectDevIpConnCfgMap(project_id, dev_ip_conn_cfg_map);
  QList<ActDevice> result_devices_without_password;
  for (auto dev : result_devices) {
    // Update password to dev_ip_conn_cfg_map
    // [bugfix:1750] Broadcast Search - Missing device password in the config IP stage
    ActDeviceIpConnectConfig dev_ip_conn_cfg;
    dev_ip_conn_cfg.SetIpAddress(dev.GetIpv4().GetIpAddress());
    dev_ip_conn_cfg.SetEnableSnmpSetting(dev.GetEnableSnmpSetting());
    act::core::g_core.CopyConnectionConfigField(dev_ip_conn_cfg, dev);

    dev_ip_conn_cfg_map[dev.GetId()] = dev_ip_conn_cfg;

    // Hide Password
    dev.HidePassword();
    result_devices_without_password.push_back(dev);
  }
  // Update dev_ip_conn_cfg_map to core
  act::core::g_core.UpdateProjectDevIpConnCfgMap(project_id, dev_ip_conn_cfg_map);

  ActDevicesResultWSResponse ws_resp(ActWSCommandEnum::kStartRetryConnect, result_devices_without_password);
  qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartRetryConnect(qint64 &project_id, const qint64 &ws_listener_id,
                                      ActRetryConnectConfig &retry_connect_cfg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kBroadcastSearching, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartRetryConnect, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << project_name << "Spawn RetryConnect thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartRetryConnectThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, retry_connect_cfg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartRetryConnectThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartLinkSequenceDetectThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                            qint64 project_id, QList<qint64> select_dev_id_list,
                                            bool from_broadcast_search) {
  ACT_STATUS_INIT();
  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartLinkSequenceDetect, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  auto backup_project = project;
  // Trigger LinkSequenceDetect procedure
  QList<ActDeviceDistanceEntry> distance_entry_list;
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::topology::ActBroadcastSearch broadcast_search(profiles);
  if (from_broadcast_search) {
    // Use project broadcast_search devices
    broadcast_search.SetDevices(project.broadcast_search_devices_);
  } else {
    // Use project devices
    QList<ActDevice> project_devices_list(project.GetDevices().begin(), project.GetDevices().end());
    broadcast_search.SetDevices(project_devices_list);
  }

  act_status = broadcast_search.StartLinkDistanceDetect(from_broadcast_search, select_dev_id_list, distance_entry_list);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start LinkDistanceDetect failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartLinkSequenceDetect, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kBroadcastSearching;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = broadcast_search.GetStatus();

    if (act_status->GetStatus() != ActStatusType::kFinished) {
      if (act_status->GetStatus() != ActStatusType::kRunning) {  // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartLinkSequenceDetect, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      } else {
        ActProgressWSResponse ws_resp(ActWSCommandEnum::kStartLinkSequenceDetect,
                                      dynamic_cast<ActProgressStatus &>(*act_status));
        qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    broadcast_search.Stop();
    qCritical() << project.GetProjectName() << "Abort link sequence detection";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  project.broadcast_search_devices_ = broadcast_search.GetDevices();
  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Write back the project status to the core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartLinkSequenceDetect, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << project.GetProjectName() << "Cannot update the backup project";
    }
    return;
  }

  // Send finished status reply to client
  ActDevicesSequenceResultWSResponse ws_resp(ActWSCommandEnum::kStartLinkSequenceDetect, distance_entry_list);
  qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartLinkSequenceDetect(qint64 &project_id, const qint64 &ws_listener_id,
                                            QList<qint64> &select_dev_id_list, bool from_broadcast_search) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kBroadcastSearching, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartLinkSequenceDetect, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << project_name << "Spawn LinkSequenceDetect thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartLinkSequenceDetectThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, select_dev_id_list, from_broadcast_search);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartLinkSequenceDetectThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

}  // namespace core
}  // namespace act
