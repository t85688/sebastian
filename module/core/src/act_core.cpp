#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QDesktopServices>

#include "act_algorithm.hpp"
#include "act_core.hpp"
#include "act_db.hpp"
// #include "jwt.h"

namespace act {
namespace core {

ActCore g_core;

QMap<qint64, std::pair<std::shared_ptr<std::promise<void>>, std::shared_ptr<std::thread>>> ws_thread_handler_pools;

ActCore::ActCore() {
  this->fake_monitor_mode_ = false;
  this->last_assigned_user_id_ = -1;
  this->last_assigned_project_id_ = -1;
  this->last_assigned_device_profile_id_ = -1;
  this->last_assigned_firmware_id_ = -1;
  this->last_assigned_topology_id_ = -1;
  this->mac_host_map_.clear();
  this->system_status_ = ActSystemStatusEnum::kIdle;
}

ACT_STATUS ActCore::Init() {
  ACT_STATUS_INIT();

  // Set system config
  ActSystem sys;
  act_status = act::database::system::RetrieveData(sys);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial system config failed";
    return act_status;
  }

  this->SetSystemConfig(sys);

  // Set user config
  QSet<ActUser> user_set;
  qint64 last_assigned_user_id = -1;
  act_status = act::database::user::RetrieveData(user_set, last_assigned_user_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial user config failed";
    return act_status;
  }
  this->SetUserSet(user_set);
  this->last_assigned_user_id_ = last_assigned_user_id;

  // Read device profiles from configuration folder
  act_status = this->InitDeviceProfiles();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial device profile failed";
    return act_status;
  }

  // Read default device profiles from configuration folder
  act_status = this->InitDefaultDeviceProfiles();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial default device profile failed";
    return act_status;
  }

  // Read feature profiles from configuration folder
  act_status = this->InitFeatureProfiles();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial feature profile failed";
    return act_status;
  }

  // Read firmware feature profiles from configuration folder
  act_status = this->InitFirmwareFeatureProfiles();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial firmware feature profile failed";
    return act_status;
  }

  // Read power device profiles from configuration folder
  act_status = this->InitPowerDeviceProfiles();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial power device profile failed";
    return act_status;
  }

  // Read software license profiles from configuration folder
  act_status = this->InitSoftwareLicenseProfiles();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial software license profile failed";
    return act_status;
  }

  // Read ethernet modules from configuration folder
  act_status = this->InitEthernetModules();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial ethernet module failed";
    return act_status;
  }

  // Read SFP modules from configuration folder
  act_status = this->InitSFPModules();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial SFP module failed";
    return act_status;
  }

  // Read power modules from configuration folder
  act_status = this->InitPowerModules();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial power module failed";
    return act_status;
  }

  act_status = this->InitGeneralProfiles();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial general profile failed";
    return act_status;
  }

  // Set project config
  QSet<ActProject> project_set;
  qint64 last_assigned_project_id = -1;
  act_status = act::database::project::RetrieveData(project_set, last_assigned_project_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial project config failed";
    return act_status;
  }
  QSet<ActProject> new_project_set;
  for (ActProject project : project_set.values()) {
    qint64 project_id = project.GetId();

    act_status = this->CheckProject(project, false);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Check project failed";
      return act_status;
    }

    // [feat:2495] Support fake MAC address
    QSet<ActDevice> device_set = project.GetDevices();
    QSet<ActDevice> new_device_set;
    for (ActDevice device : device_set) {
      quint32 ip_num = 0;
      ActIpv4::AddressStrToNumber(device.GetIpv4().GetIpAddress(), ip_num);
      project.used_ip_addresses_.insert(ip_num);

      project.used_coordinates_.insert(device.GetCoordinate());

      new_device_set.insert(device);
    }

    project.SetDevices(new_device_set);

    // [feat:955] Undo/Redo - Initiate the project operation history list
    undo_operation_history[project_id] = QStack<ActProject>();
    redo_operation_history[project_id] = QStack<ActProject>();

    // Initiate the project activate deploy flag
    deploy_available[project.GetId()] = this->CheckDeployAvailableByDeviceConfig(project);

    // [bug:2832] move the project status out of the project configuration
    this->project_status_list[project_id] = ActProjectStatusEnum::kIdle;

    new_project_set.insert(project);
  }

  this->SetProjectSet(new_project_set);
  this->last_assigned_project_id_ = last_assigned_project_id;

  // Set firmware config
  QSet<ActFirmware> firmware_set;
  qint64 last_assigned_firmware_id = -1;
  act_status = act::database::firmware::RetrieveData(firmware_set, last_assigned_firmware_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial firmware config failed";
    return act_status;
  }
  this->SetFirmwareSet(firmware_set);
  this->last_assigned_firmware_id_ = last_assigned_firmware_id;

  // [feat:2241] Retrieve topology configuration
  QSet<ActTopology> topology_set;
  qint64 last_assigned_topology_id = -1;
  act_status = act::database::topology::RetrieveData(topology_set, last_assigned_topology_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial topology config failed";
    return act_status;
  }
  this->SetTopologySet(topology_set);
  this->last_assigned_topology_id_ = last_assigned_topology_id;

  // Retrieve Design Baseline
  QSet<ActNetworkBaseline> design_baseline_set;
  qint64 last_assigned_design_baseline_id = -1;
  act_status = act::database::networkbaseline::RetrieveData(ActBaselineModeEnum::kDesign, design_baseline_set,
                                                            last_assigned_design_baseline_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial DesignBaseline config failed";
    return act_status;
  }
  this->SetDesignBaselineSet(design_baseline_set);
  this->last_assigned_design_baseline_id_ = last_assigned_design_baseline_id;

  // Retrieve Operation Baseline
  QSet<ActNetworkBaseline> operation_baseline_set;
  qint64 last_assigned_operation_baseline_id = -1;
  act_status = act::database::networkbaseline::RetrieveData(ActBaselineModeEnum::kOperation, operation_baseline_set,
                                                            last_assigned_operation_baseline_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB(): Initial OperationBaseline config failed";
    return act_status;
  }
  this->SetOperationBaselineSet(operation_baseline_set);

  return act_status;
}

ACT_STATUS ActCore::StopWSJob(qint64 &project_id, const qint64 &ws_listener_id, const ActWSCommandEnum &op_code_enum) {
  ACT_STATUS_INIT();

  act_status = RemoveWSJob(project_id);
  if (!IsActStatusSuccess(act_status)) {
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp = ActWSResponseErrorTransfer(op_code_enum, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return act_status;
  }

  // Update Project status
  this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

  return act_status;
}

ACT_STATUS ActCore::RemoveWSJob(qint64 &project_id) {
  ACT_STATUS_INIT();

  if (!ws_thread_handler_pools.contains(project_id)) {
    QString error_msg = QString("Thread not exist");
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Fetch the promise instance of the thread
  std::pair<std::shared_ptr<std::promise<void>>, std::shared_ptr<std::thread>> thread_handler =
      ws_thread_handler_pools.value(project_id);
  std::shared_ptr<std::promise<void>> signal_sender = thread_handler.first;
  std::shared_ptr<std::thread> thread_ptr = thread_handler.second;

  // Set the value in promise to stop the thread

  if (signal_sender) {
    // qDebug() << "Set the value in promise to stop the thread";
    // [bugfix:3218] Auto Scan過久導致Chamberlain異常
    try {
      signal_sender->set_value();
    } catch (const std::future_error &e) {
      qWarning() << __func__ << "Future error: " << e.what() << "\n";
      if (e.code() != std::make_error_code(std::future_errc::promise_already_satisfied)) {
        QString error_msg = QString("Project thread job cannot stop");
        qCritical() << __func__ << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }
  }

  // Waiting for the thread close
  if ((thread_ptr != nullptr) && (thread_ptr->joinable())) {
    thread_ptr->join();
  }

  // qDebug() << "Thread handler is removed from pool";
  ws_thread_handler_pools.remove(project_id);

  return act_status;
}

ACT_STATUS ActCore::StopWSJobSystem(const qint64 &ws_listener_id) {
  ACT_STATUS_INIT();

  std::shared_ptr<std::promise<void>> signal_sender = system_promise_thread_pair_.first;
  std::shared_ptr<std::thread> thread_ptr = system_promise_thread_pair_.second;

  if (thread_ptr == nullptr) {
    QString error_msg = "System thread job not exist";
    qCritical() << __func__ << error_msg;

    ActBadRequest act_bad_status(error_msg);
    this->SendMessageToWSListener(ws_listener_id, act_bad_status.ToString().toStdString().c_str());
    return act_status;
  }

  // Set the value in promise to stop the thread
  if (signal_sender) {
    qDebug() << "Set the value in promise to stop the system thread job";
    // [bugfix:3218] Auto Scan過久導致Chamberlain異常
    try {
      signal_sender->set_value();
    } catch (const std::future_error &e) {
      qCritical() << __func__ << "Future error: " << e.what() << "\n";

      QString error_msg = QString("System thread job cannot stop");
      qCritical() << __func__ << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Waiting for the thread close
  if ((thread_ptr != nullptr) && (thread_ptr->joinable())) {
    thread_ptr->join();
  }

  return act_status;
}

ACT_STATUS ActCore::DestroyWSJob() {
  ACT_STATUS_INIT();

  // Get project by id
  QSet<ActProject> projects = this->GetProjectSet();

  for (ActProject project : projects) {
    qint64 project_id = project.GetId();
    // Check the Websocket thread pool
    if (ws_thread_handler_pools.contains(project_id)) {
      std::pair<std::shared_ptr<std::promise<void>>, std::shared_ptr<std::thread>> thread_handler =
          ws_thread_handler_pools.value(project_id);
      std::shared_ptr<std::promise<void>> signal_sender = thread_handler.first;
      std::shared_ptr<std::thread> thread_ptr = thread_handler.second;

      ActProjectStatusEnum project_status = this->project_status_list[project_id];

      // Check the exist thread is reasonable
      if (project_status == ActProjectStatusEnum::kFinished || project_status == ActProjectStatusEnum::kAborted ||
          project_status == ActProjectStatusEnum::kIdle) {
        // Remove the previous thread handler pointer from the pool
        if (thread_ptr != nullptr && thread_ptr->joinable()) {
          thread_ptr->join();
        }
      } else {
        // Set the value in promise to stop the thread
        if (signal_sender) {
          // qDebug() << "Set the value in promise to stop the thread";
          // [bugfix:3218] Auto Scan過久導致Chamberlain異常
          try {
            signal_sender->set_value();
          } catch (const std::future_error &e) {
            qCritical() << __func__ << "Future error: " << e.what() << "\n";

            QString error_msg = QString("Thread job cannot stop");
            qCritical() << __func__ << error_msg.toStdString().c_str();
            return std::make_shared<ActBadRequest>(error_msg);
          }
        }

        // Waiting for the thread close
        if ((thread_ptr != nullptr) && (thread_ptr->joinable())) {
          qDebug() << __func__ << "Waiting for the WS thread close";
          thread_ptr->join();
          qDebug() << __func__ << "WS Thread is closed";
        }
      }

      ws_thread_handler_pools.remove(project_id);
      qDebug() << __func__ << "Project status:" << kActProjectStatusEnumMap.key(project_status);
      qDebug() << __func__ << project.GetProjectName().toStdString().c_str() << "Thread handler is removed from pool";
    }
  }
  return act_status;
}

ACT_STATUS ActCore::DestroyWSJobMonitor() {
  ACT_STATUS_INIT();
  std::shared_ptr<std::promise<void>> signal_sender = monitor_promise_thread_pair_.first;
  std::shared_ptr<std::thread> thread_ptr = monitor_promise_thread_pair_.second;

  if (thread_ptr != nullptr) {
    // Check the exist thread is reasonable
    if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
      if ((thread_ptr != nullptr) && (thread_ptr->joinable())) {
        thread_ptr->join();
      }
    } else {
      // Set the value in promise to stop the thread
      if (signal_sender) {
        qDebug() << "Set the value in promise to stop the thread";
        // [bugfix:3218] Auto Scan過久導致Chamberlain異常
        try {
          signal_sender->set_value();
        } catch (const std::future_error &e) {
          qCritical() << __func__ << "Future error: " << e.what() << "\n";

          QString error_msg = QString("Thread job cannot stop");
          qCritical() << __func__ << error_msg.toStdString().c_str();
          return std::make_shared<ActBadRequest>(error_msg);
        }
      }

      // Waiting for the thread close
      if ((thread_ptr != nullptr) && (thread_ptr->joinable())) {
        thread_ptr->join();
      }
    }
  }

  return act_status;
}

ACT_STATUS ActCore::CheckWSJobCanStart(const ActProjectStatusEnum &new_job_status, qint64 &project_id,
                                       QString &result_project_name) {
  ACT_STATUS_INIT();
  if (project_id == ACT_SYSTEM_WS_PROJECT_ID) {
    result_project_name = ACT_SYSTEM_WS_PROJECT_NAME;
  } else {
    ActProject project;
    act_status = this->GetProject(project_id, project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Get project failed with project id:" << project_id;

      return std::make_shared<ActBadRequest>("Get project failed");
    }
    result_project_name = project.GetProjectName();
  }

  // Check is the monitor concurrent job
  if ((this->GetSystemStatus() == ActSystemStatusEnum::kMonitoring) &&
      (!ActMonitorConcurrentJobStatus.contains(new_job_status))) {
    // Only device config job can be run concurrently with the monitor job
    QString error_msg = "The monitor process is already running";
    qDebug() << __func__ << result_project_name + ":" + error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check system thread status
  if ((this->GetSystemStatus() != ActSystemStatusEnum::kIdle) &&
      (!ActMonitorConcurrentJobStatus.contains(new_job_status))) {
    QString error_msg = "Please wait, the system is still busy";
    qCritical() << __func__ << result_project_name + ":" + error_msg;

    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check the Websocket thread pool
  if (ws_thread_handler_pools.contains(project_id)) {
    ActProjectStatusEnum project_status = this->project_status_list[project_id];
    qDebug() << __func__ << "Project status:" << kActProjectStatusEnumMap.key(project_status);

    // Check the exist thread is reasonable
    bool idle_status = project_status == ActProjectStatusEnum::kFinished ||
                       project_status == ActProjectStatusEnum::kAborted ||
                       project_status == ActProjectStatusEnum::kIdle;
    bool monitor_concurrent_job_status =
        (project_status == ActProjectStatusEnum::kMonitoring) && ActMonitorConcurrentJobStatus.contains(new_job_status);
    if (idle_status || monitor_concurrent_job_status) {
      // Remove the previous thread handler pointer from the pool
      qDebug() << __func__ << result_project_name << "Thread handler is removed from pool";
      std::pair<std::shared_ptr<std::promise<void>>, std::shared_ptr<std::thread>> thread_handler =
          ws_thread_handler_pools.value(project_id);
      std::shared_ptr<std::promise<void>> signal_sender = thread_handler.first;
      std::shared_ptr<std::thread> thread_ptr = thread_handler.second;

      if (thread_ptr != nullptr && thread_ptr->joinable()) {
        thread_ptr->join();
      }
      ws_thread_handler_pools.remove(project_id);
    } else {
      qCritical() << __func__ << result_project_name << "Previous process is still running";
      return std::make_shared<ActBadRequest>("Previous process is still running");
    }
  }

  return act_status;
}

ACT_STATUS ActCore::GetRESTfulTokenMap(QMap<qint64, QString> &restful_token_map) {
  ACT_STATUS_INIT();
  restful_token_map = this->restful_token_map_;
  restful_token_map.detach();
  return act_status;
}

ACT_STATUS ActCore::SetRESTfulTokenMap(const QMap<qint64, QString> &restful_token_map) {
  ACT_STATUS_INIT();

  this->restful_token_map_ = restful_token_map;
  this->restful_token_map_.detach();

  return act_status;
}

ACT_STATUS ActCore::GetDeviceRESTfulTokenMap(const qint64 &device_id, QString &token) {
  ACT_STATUS_INIT();
  if (this->restful_token_map_.contains(device_id)) {
    token = this->restful_token_map_[device_id];
  } else {
    token = "";
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateDeviceRESTfulTokenMap(const qint64 &device_id, const QString &token) {
  ACT_STATUS_INIT();

  this->restful_token_map_[device_id] = token;
  this->restful_token_map_.detach();

  return act_status;
}

ACT_STATUS ActCore::GetMacHostMap(QMap<QString, QString> &mac_host_map) {
  ACT_STATUS_INIT();
  mac_host_map = this->mac_host_map_;
  mac_host_map.detach();
  return act_status;
}

ACT_STATUS ActCore::SetMacHostMap(const QMap<QString, QString> &mac_host_map) {
  ACT_STATUS_INIT();

  this->mac_host_map_ = mac_host_map;
  this->mac_host_map_.detach();

  return act_status;
}

ACT_STATUS ActCore::GetProjectDevIpConnCfgMap(const qint64 &project_id,
                                              QMap<qint64, ActDeviceIpConnectConfig> &dev_ip_conn_cfg_map) {
  // [bugfix:1780] Broadcast Search finished not connecting to the Auto-Scan page
  ACT_STATUS_INIT();

  dev_ip_conn_cfg_map.clear();
  if (!this->project_dev_ip_conn_cfg_map_.contains(project_id)) {
    return act_status;
  }
  dev_ip_conn_cfg_map = this->project_dev_ip_conn_cfg_map_[project_id];
  dev_ip_conn_cfg_map.detach();
  return act_status;
}

ACT_STATUS ActCore::UpdateProjectDevIpConnCfgMap(const qint64 &project_id,
                                                 const QMap<qint64, ActDeviceIpConnectConfig> &dev_ip_conn_cfg_map) {
  // [bugfix:1780] Broadcast Search finished not connecting to the Auto-Scan page
  ACT_STATUS_INIT();

  this->project_dev_ip_conn_cfg_map_[project_id] = dev_ip_conn_cfg_map;
  this->project_dev_ip_conn_cfg_map_[project_id].detach();

  return act_status;
}

ACT_STATUS ActCore::ExportProject(qint64 &project_id, ActExportProject &exported_project) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // [bugfix:2514] AutoScan can not identify device
  // EncryptPassword
  project.EncryptPassword();

  QSet<ActExportDeviceProfileInfo> device_profile_infos;
  QSet<ActDeviceProfile> device_profile_set;
  for (ActDevice device : project.GetDevices()) {
    qint64 device_profile_id = device.GetDeviceProfileId();

    // Find device_profile
    ActDeviceProfile dev_profile;
    act_status = ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device_profile_id, dev_profile);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Device profile id:" << device_profile_id << "not found";
      return act_status;
    }

    QString device_profile_key;
    dev_profile.GetClassKeys(device_profile_key);

    ActExportDeviceProfileInfo device_profile_info;
    device_profile_info.SetId(dev_profile.GetId());
    device_profile_info.SetKey(device_profile_key);

    device_profile_infos.insert(device_profile_info);

    // Skip built-in device profile
    if (dev_profile.GetBuiltIn()) {
      continue;
    }

    device_profile_set.insert(dev_profile);
  }

  // [feat:3854] Include Baselines when exporting the project
  QList<ActNetworkBaseline> design_baseline_list;
  QList<ActNetworkBaseline> operation_baseline_list;

  // Add the Design Baselines (sort by date)
  // Append DB Baselines
  for (auto design_baseline_id : project.GetDesignBaselineIds()) {
    ActNetworkBaseline design_baseline;
    act_status = ActGetItemById<ActNetworkBaseline>(this->GetDesignBaselineSet(), design_baseline_id, design_baseline);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << __func__ << QString("Not found the Design Baseline id %1").arg(design_baseline_id);
    } else {
      design_baseline.EncryptPassword();
      design_baseline_list.append(design_baseline);
    }
  }
  // sort by date (from small to large)
  std::sort(design_baseline_list.begin(), design_baseline_list.end(),
            [](const ActNetworkBaseline &x, const ActNetworkBaseline &y) { return x.GetDate() < y.GetDate(); });

  // Add the Operation Baselines (sort by date)

  // Append DB Baselines
  for (auto operation_baseline_id : project.GetOperationBaselineIds()) {
    ActNetworkBaseline operation_baseline;
    act_status =
        ActGetItemById<ActNetworkBaseline>(this->GetOperationBaselineSet(), operation_baseline_id, operation_baseline);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << __func__ << QString("Not found the Operation Baseline id %1").arg(operation_baseline_id);
    } else {
      operation_baseline.EncryptPassword();
      operation_baseline_list.append(operation_baseline);
    }
  }
  // sort by date (from small to large)
  std::sort(operation_baseline_list.begin(), operation_baseline_list.end(),
            [](const ActNetworkBaseline &x, const ActNetworkBaseline &y) { return x.GetDate() < y.GetDate(); });

  // TODO: The ProjectName field should be removed after UI team is available
  exported_project.SetProjectName(project.GetProjectName());
  exported_project.SetProject(project);
  exported_project.SetDeviceProfiles(device_profile_set);
  exported_project.SetDeviceProfileInfos(device_profile_infos);
  exported_project.SetDesignBaselines(design_baseline_list);
  exported_project.SetOperationBaselines(operation_baseline_list);

  return act_status;
}

ACT_STATUS ActCore::GetDeviceAndInterfacePair(const QSet<ActDevice> &device_set, const ActLink &link,
                                              ActDevice &src_dev, ActDevice &dst_dev, ActInterface &src_intf,
                                              ActInterface &dst_intf) {
  ACT_STATUS_INIT();

  // Get source/destination device by id
  act_status = ActGetItemById<ActDevice>(device_set, link.GetSourceDeviceId(), src_dev);
  if (!IsActStatusSuccess(act_status)) {
    QString message = QString("Source device %1 not found").arg(QString::number(link.GetSourceDeviceId()));
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }
  act_status = ActGetItemById<ActDevice>(device_set, link.GetDestinationDeviceId(), dst_dev);
  if (!IsActStatusSuccess(act_status)) {
    QString message = QString("Destination device %1 not found").arg(QString::number(link.GetDestinationDeviceId()));
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  // Get source interface by id
  QList<ActInterface> src_intf_list = src_dev.GetInterfaces();
  qint64 src_intf_id = link.GetSourceInterfaceId();
  int src_intf_idx = src_intf_list.indexOf(ActInterface(src_intf_id));
  if (src_intf_idx < 0) {
    QString message = QString("Interface %1 in source device %2 not found")
                          .arg(QString::number(src_intf_id))
                          .arg(src_dev.GetIpv4().GetIpAddress());
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }
  src_intf = src_intf_list[src_intf_idx];

  // Get destination interface by id
  QList<ActInterface> dst_intf_list = dst_dev.GetInterfaces();
  qint64 dst_intf_id = link.GetDestinationInterfaceId();
  int dst_intf_idx = dst_intf_list.indexOf(ActInterface(dst_intf_id));
  if (dst_intf_idx < 0) {
    QString message = QString("Interface %1 in destination device %2 not found")
                          .arg(QString::number(dst_intf_id))
                          .arg(dst_dev.GetIpv4().GetIpAddress());
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }
  dst_intf = dst_intf_list[dst_intf_idx];

  return act_status;
}

ACT_STATUS ActCore::GetDeviceAndInterfacePairFromProject(const ActProject &project, const ActLink &link,
                                                         ActDevice &src_dev, ActDevice &dst_dev, ActInterface &src_intf,
                                                         ActInterface &dst_intf) {
  ACT_STATUS_INIT();

  // Get source/destination device by id
  QSet<ActDevice> device_set = project.GetDevices();
  return GetDeviceAndInterfacePair(device_set, link, src_dev, dst_dev, src_intf, dst_intf);
}

ACT_STATUS ActCore::ClearFolder(const QString &path) {
  ACT_STATUS_INIT();

  QDir dir(path);

  // Check folder exist
  if (!dir.exists()) {
    return std::make_shared<ActStatusNotFound>(QString("%1 folder").arg(path));
  }

  // Get files & sub folders
  QStringList entries = dir.entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

  for (const QString &entry : entries) {
    QString entry_path = dir.filePath(entry);
    QFileInfo fileInfo(entry_path);

    if (fileInfo.isDir()) {
      // Recursive remove sub folder
      if (!IsActStatusSuccess(ClearFolder(entry_path))) {  // files of the sub folder
        return act_status;
      }

      // Remove empty sub folder
      QDir sub_dir;
      if (!sub_dir.rmdir(entry_path)) {  // sub folder
        qWarning() << "Failed to remove directory:" << entry_path;
        return std::make_shared<ActStatusInternalError>("ClearFolder");
      }
    } else {
      // Remove files
      if (!QFile::remove(entry_path)) {
        qWarning() << "Failed to remove file:" << entry_path;
        return std::make_shared<ActStatusInternalError>("ClearFolder");
      }
    }
  }
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::OpenBrowser() {
  ACT_STATUS_INIT();

  // Open browser
  QDesktopServices::openUrl(GetMainHttpsUrl());

  return act_status;
}

}  // namespace core
}  // namespace act
