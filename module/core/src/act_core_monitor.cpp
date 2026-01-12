#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QHostInfo>
#include <QNetworkInterface>
#include <QSet>
#include <thread>

#include "act_core.hpp"
#include "act_monitor.hpp"

namespace act {
namespace core {

extern QSet<QString> g_busy_device_set;  // Used to prevent multiple ping jobs in the same cycle
extern QMutex g_busy_device_set_mutex;

extern QMap<qint64, ActMonitorDeviceStatus> g_monitor_device_status;  // <device_id, device_status>
extern QSet<ActMonitorDeviceStatusData> g_device_status_ws_data_set;
extern QMap<qint64, bool> g_monitor_link_status;  // <link_id, link_status>
extern QSet<ActMonitorLinkStatusData> g_link_status_ws_data_set;
extern QSet<ActMonitorDeviceSystemStatus> g_device_system_status_ws_data_set;
extern QMap<qint64, ActDeviceMonitorTraffic> g_monitor_device_traffic;  // <device_id, device_traffic>
extern QMap<qint64, ActLinkMonitorTraffic> g_monitor_link_traffic;      // <link_id, link_traffic>
extern QMap<qint64, ActMonitorTimeStatus> g_monitor_time_status;        // <device_id, time_status>
extern QMap<qint64, ActMonitorBasicStatus> g_monitor_basic_status;      // <device_id, basic_status>
extern QMap<qint64, ActMonitorRstpStatus> g_monitor_rstp_status;        // <device_id, rstp_status>

extern QList<QString> g_baseline_device_ip_list;  // offline device IP list
extern QList<qint64> g_baseline_device_id_list;   // offline device id list
extern QList<qint64> g_baseline_link_id_list;     // offline link id list

void ActCore::StartMonitorThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                                 const ActStartMonitorWSCommand &command) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  quint64 last_time = 0;
  quint64 polling_interval_ms;
  quint64 last_heartbeat_time = 0;
  QSet<ActDevice> baseline_device_set;
  QSet<ActDevice> monitor_device_set;
  QList<ActScanIpRangeEntry> scan_ip_range_list;
  bool from_scan_list;
  bool from_offline;
  qint64 monitor_project_id;
  const int batch_size = 10;

  this->fake_monitor_mode_ = command.GetFakeMode();

  // Init the operation project
  act_status = this->GetProject(project_id, monitor_project_);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartMonitor, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    // Send finished status reply to client
    ActBaseResponse ws_sys_resp(ActWSCommandEnum::kStartMonitor, ActStatusType::kFinished);
    // qDebug() << __func__ << ws_sys_resp.ToString(ws_sys_resp.key_order_).toStdString().c_str();
    this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_sys_resp);
    return;
  }

  qDebug() << "Monitor device size:" << monitor_project_.GetDevices().size();
  qDebug() << "Monitor link size:" << monitor_project_.GetLinks().size();

  // TODO: Baseline topology for offline check
  baseline_project_ = monitor_project_;

  // Get design project by UUID
  g_baseline_device_ip_list.clear();
  g_baseline_device_id_list.clear();
  g_baseline_link_id_list.clear();
  monitor_process_queue_.clear();
  g_busy_device_set.clear();

  for (ActDevice device : baseline_project_.GetDevices()) {
    g_baseline_device_ip_list.append(device.GetIpv4().GetIpAddress());
    g_baseline_device_id_list.append(device.GetId());
  }

  for (ActLink link : baseline_project_.GetLinks()) {
    g_baseline_link_id_list.append(link.GetId());
  }

  baseline_device_set = baseline_project_.GetDevices();

  act_status = StartMonitorProcessEngine(project_id, ws_listener_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Start operation process engine failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartMonitor, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    // Send finished status reply to client
    ActBaseResponse ws_sys_resp(ActWSCommandEnum::kStartMonitor, ActStatusType::kFinished);
    // qDebug() << __func__ << ws_sys_resp.ToString(ws_sys_resp.key_order_).toStdString().c_str();
    this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_sys_resp);

    return;
  }

  // Wait operation thread ready
  SLEEP_MS(1000);

  this->project_status_list[project_id] = ActProjectStatusEnum::kMonitoring;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    // Get ping interval from project setting

    // Fetch the host IP of this computer and skip
    QList<QHostAddress> all_addresses = QNetworkInterface::allAddresses();
    QList<QString> host_ip_list;

    for (const QHostAddress &address : all_addresses) {
      if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost) {
        host_ip_list.append(address.toString());
      }
    }

    // Update the operation project setting
    monitor_mutex_.lock();
    monitor_project_id = monitor_project_.GetId();
    polling_interval_ms = monitor_project_.GetProjectSetting().GetMonitorConfiguration().GetPollingInterval() * 1000;
    scan_ip_range_list = monitor_project_.GetProjectSetting().GetScanIpRanges();
    from_scan_list = monitor_project_.GetProjectSetting().GetMonitorConfiguration().GetFromIpScanList();
    from_offline = monitor_project_.GetProjectSetting().GetMonitorConfiguration().GetFromOfflineProject();
    monitor_device_set = monitor_project_.GetDevices();

    qint64 current_time = QDateTime::currentMSecsSinceEpoch();

    // Add this block to execute a new task every 30 seconds
    if (current_time - last_heartbeat_time >= 30000) {
      last_heartbeat_time = current_time;

      QList<ActJob> job_list;
      QList<ActHeartbeatJob> heartbeat_job_list;
      for (ActDevice device : monitor_device_set) {
        if (g_monitor_device_status.contains(device.GetId()) && !g_monitor_device_status[device.GetId()].GetAlive()) {
          continue;
        }
        ActHeartbeatJob heartbeat_job(device);
        heartbeat_job_list.push_back(heartbeat_job);
      }

      ActJob job;
      job.AssignJob<QList<ActHeartbeatJob>>(monitor_project_id, ActJobTypeEnum::kMultipleHeartbeat, heartbeat_job_list);
      job_list.push_back(job);
      this->DistributeWorkerJobs(job_list);
    }

    if (current_time - last_time < polling_interval_ms) {
      SLEEP_MS(1000);
      monitor_mutex_.unlock();
      continue;
    }

    last_time = current_time;

    QList<ActJob> job_list;
    QList<ActPingJob> ping_job_list;

    // Add the baseline device to ping list
    for (ActDevice device : baseline_device_set) {
      if (host_ip_list.contains(device.GetIpv4().GetIpAddress())) {
        continue;
      }

      // Skip the device if the device has already been pinged
      QMutexLocker locker(&g_busy_device_set_mutex);
      if (g_busy_device_set.contains(device.GetIpv4().GetIpAddress())) {
        continue;
      }

      ActPingJob ping_job(device);
      ping_job.SetIp(device.GetIpv4().GetIpAddress());
      ping_job_list.push_back(ping_job);
      g_busy_device_set.insert(device.GetIpv4().GetIpAddress());
    }

    // Add the operation device to ping list
    for (ActDevice device : monitor_device_set) {
      if (host_ip_list.contains(device.GetIpv4().GetIpAddress())) {
        continue;
      }

      // Skip the device if the device has already been pinged
      QMutexLocker locker(&g_busy_device_set_mutex);
      if (g_busy_device_set.contains(device.GetIpv4().GetIpAddress())) {
        continue;
      }

      ActPingJob ping_job(device);
      ping_job.SetIp(device.GetIpv4().GetIpAddress());
      ping_job_list.push_back(ping_job);
      g_busy_device_set.insert(device.GetIpv4().GetIpAddress());
    }

    // Translate the scan_ip_range to ping entry and add to the ping list
    if (from_scan_list) {
      for (ActScanIpRangeEntry scan_ip_range : scan_ip_range_list) {
        // In the scan ip range, foreach ip address, add to the ping list
        // From start_ip to end_ip
        quint32 start_ip_num = 0;
        ActIpv4::AddressStrToNumber(scan_ip_range.GetStartIp(), start_ip_num);
        quint32 end_ip_num = 0;
        ActIpv4::AddressStrToNumber(scan_ip_range.GetEndIp(), end_ip_num);

        for (quint32 ip_num = start_ip_num; ip_num <= end_ip_num; ip_num++) {
          // Copy the connection parameters from the scan ip range to the ping job
          ActPingJob ping_job(scan_ip_range);

          QString ip_str;
          ActIpv4::AddressNumberToStr(ip_num, ip_str);
          ping_job.SetIp(ip_str);

          if (host_ip_list.contains(ip_str)) {
            continue;
          }

          QMutexLocker locker(&g_busy_device_set_mutex);
          if (g_busy_device_set.contains(ip_str)) {
            // Skip the device if the device has already been pinged
            continue;
          }

          ping_job_list.push_back(ping_job);
          g_busy_device_set.insert(ip_str);
        }
      }
    }

    monitor_mutex_.unlock();

    for (int i = 0; i < ping_job_list.size(); i += batch_size) {
      QList<ActPingJob> batch = ping_job_list.mid(i, batch_size);
      ActJob job;
      job.AssignJob<QList<ActPingJob>>(monitor_project_id, ActJobTypeEnum::kMultiplePing, batch);
      job_list.push_back(job);
    }

    this->DistributeWorkerJobs(job_list);
  }

  qDebug() << monitor_project_.GetProjectName() << "Thread is going to close";

  {
    // Clear job queue
    QMutexLocker locker(&job_queue_mutex_);
    job_queue_.clear();
  }

  StopMonitorProcessEngine();

  // Send Websocket update msg from NotificationTmp
  // this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartMonitor, ActStatusType::kFinished);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_resp);

  return;
}

ACT_STATUS ActCore::StartMonitor(qint64 project_id, const qint64 ws_listener_id,
                                 const ActStartMonitorWSCommand &command) {
  ACT_STATUS_INIT();

  // If the license is not available, return the error
  /* if (!this->GetLicense().GetFeature().GetStage().GetOperation()) {
    QString error_msg = "The license does not support operation";
    qCritical() << __func__ << error_msg;

    return std::make_shared<ActLicenseNotActiveFailed>("Operation");
  } */

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kMonitoring, project_id, project_name);

  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartMonitor, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << project_name << "Spawn operation thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::StartMonitorThread, this, std::cref(ws_listener_id),
                                             std::move(signal_receiver), project_id, command);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartMonitorThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Set thread handler to monitor_ws
  qDebug() << project_name << "Set thread handler to monitor_ws";
  monitor_promise_thread_pair_ = make_pair(signal_sender, thread_ptr);

  // Insert thread handler to pools
  qDebug() << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, monitor_promise_thread_pair_);

  return act_status;
}

ACT_STATUS ActCore::StopMonitor(qint64 project_id, const qint64 ws_listener_id, const ActWSCommandEnum &op_code_enum) {
  ACT_STATUS_INIT();
  act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, ActWSCommandEnum::kStopMonitor);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Stop device discovery failed";
  }

  monitor_project_ = ActProject();

  return act_status;
}

ACT_STATUS ActCore::GetMonitorSFPList(const qint64 &project_id, ActSFPList &sfp_list) {
  ACT_STATUS_INIT();

  // If the license is not available, return the error
  /* if (!this->GetLicense().GetFeature().GetStage().GetOperation()) {
    QString error_msg = "The license does not support operation";
    qCritical() << __func__ << error_msg;

    return std::make_shared<ActLicenseNotActiveFailed>("Operation");
  } */

  if (project_id != monitor_project_.GetId()) {
    QString error_msg = "The project is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  sfp_list = monitor_project_.sfp_list_;

  return act_status;
}

ACT_STATUS ActCore::GetDeviceBasicStatus(const qint64 &project_id, const qint64 &device_id,
                                         ActMonitorBasicStatus &basic_status) {
  ACT_STATUS_INIT();

  // If the license is not available, return the error
  /* if (!this->GetLicense().GetFeature().GetStage().GetOperation()) {
    QString error_msg = "The license does not support operation";
    qCritical() << __func__ << error_msg;

    return std::make_shared<ActLicenseNotActiveFailed>("Operation");
  } */

  if (project_id != monitor_project_.GetId()) {
    QString error_msg = "The project is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (g_monitor_basic_status.contains(device_id)) {
    basic_status = g_monitor_basic_status[device_id];
  } else {
    QString error_msg = "The device is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

ACT_STATUS ActCore::GetDeviceSFPStatus(const qint64 &project_id, const qint64 &device_id,
                                       ActMonitorDeviceSFPStatus &sfp_status) {
  ACT_STATUS_INIT();

  // If the license is not available, return the error
  /* if (!this->GetLicense().GetFeature().GetStage().GetOperation()) {
    QString error_msg = "The license does not support operation";
    qCritical() << __func__ << error_msg;

    return std::make_shared<ActLicenseNotActiveFailed>("Operation");
  } */

  if (project_id != monitor_project_.GetId()) {
    QString error_msg = "The project is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (!g_monitor_basic_status.contains(device_id)) {
    QString error_msg = "The device is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  ActMonitorBasicStatus basic_status = g_monitor_basic_status[device_id];

  QMap<QString, ActMonitorSFPStatusEntry> sfp_status_map;
  for (ActMonitorFiberCheckEntry fiber_check : basic_status.GetFiberCheck()) {
    if (fiber_check.GetExist()) {
      sfp_status_map[fiber_check.GetInterfaceName()] = ActMonitorSFPStatusEntry(fiber_check);
    }
  }
  sfp_status.SetSFPStatus(sfp_status_map);

  return act_status;
}

ACT_STATUS ActCore::GetMonitorDeviceBasicInfo(const QString &device_ip, ActMonitorDeviceBasicInfo &basic_info) {
  ACT_STATUS_INIT();

  // If the license is not available, return the error
  /* if (!this->GetLicense().GetFeature().GetStage().GetOperation()) {
    QString error_msg = "The license does not support operation";
    qCritical() << __func__ << error_msg;

    return std::make_shared<ActLicenseNotActiveFailed>("Operation");
  } */

  if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
    QString error_msg = "The system is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Get device ip by device IP
  qint64 device_id = 0;
  ActDevice device;
  act_status = monitor_project_.GetDeviceByIp(device_ip, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get device id by ip failed with ip:" << device_ip;
    return act_status;
  }

  device_id = device.GetId();

  if (!g_monitor_basic_status.contains(device_id)) {
    QString error_msg = "The device is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  ActMonitorBasicStatus basic_status = g_monitor_basic_status[device_id];
  basic_info = ActMonitorDeviceBasicInfo(basic_status);

  return act_status;
}

ACT_STATUS ActCore::GetMonitorDeviceSFPStatus(const QString &device_ip, ActMonitorDeviceSFPStatus &sfp_status) {
  ACT_STATUS_INIT();

  // If the license is not available, return the error
  /* if (!this->GetLicense().GetFeature().GetStage().GetOperation()) {
    QString error_msg = "The license does not support operation";
    qCritical() << __func__ << error_msg;

    return std::make_shared<ActLicenseNotActiveFailed>("Operation");
  } */

  if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
    QString error_msg = "The system is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Get device ip by device IP
  qint64 device_id = 0;
  ActDevice device;
  act_status = monitor_project_.GetDeviceByIp(device_ip, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get device id by ip failed with ip:" << device_ip;
    return act_status;
  }

  device_id = device.GetId();

  if (!g_monitor_basic_status.contains(device_id)) {
    QString error_msg = "The device is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  ActMonitorBasicStatus basic_status = g_monitor_basic_status[device_id];

  QMap<QString, ActMonitorSFPStatusEntry> sfp_status_map;
  for (ActMonitorFiberCheckEntry fiber_check : basic_status.GetFiberCheck()) {
    if (fiber_check.GetExist()) {
      sfp_status_map[fiber_check.GetInterfaceName()] = ActMonitorSFPStatusEntry(fiber_check);
    }
  }
  sfp_status.SetSFPStatus(sfp_status_map);

  return act_status;
}

ACT_STATUS ActCore::GetDevicePortStatus(const qint64 &project_id, const qint64 &device_id,
                                        ActMonitorDevicePortStatus &port_status) {
  ACT_STATUS_INIT();

  // If the license is not available, return the error
  /* if (!this->GetLicense().GetFeature().GetStage().GetOperation()) {
    QString error_msg = "The license does not support operation";
    qCritical() << __func__ << error_msg;

    return std::make_shared<ActLicenseNotActiveFailed>("Operation");
  } */

  if (project_id != monitor_project_.GetId()) {
    QString error_msg = "The project is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (!g_monitor_basic_status.contains(device_id)) {
    QString error_msg = "The device is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  ActMonitorBasicStatus basic_status = g_monitor_basic_status[device_id];

  ActDevice device;
  act_status = act::core::g_core.GetDevice(project_id, device_id, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get device failed with device id:" << device_id;
    return act_status;
  }

  QMap<QString, ActMonitorPortStatusEntry> port_status_map;
  QList<ActInterface> interface_set = device.GetInterfaces();

  for (ActInterface intf : interface_set) {
    if (!intf.GetActive()) {
      continue;
    }
    port_status_map[intf.GetInterfaceName()] = basic_status.GetPortStatus().value(intf.GetInterfaceId());
  }

  port_status.SetPortStatus(port_status_map);

  return act_status;
}

ACT_STATUS ActCore::GetMonitorDevicePortStatus(const QString &device_ip, ActMonitorDevicePortStatus &port_status) {
  ACT_STATUS_INIT();

  // If the license is not available, return the error
  /* if (!this->GetLicense().GetFeature().GetStage().GetOperation()) {
    QString error_msg = "The license does not support operation";
    qCritical() << __func__ << error_msg;

    return std::make_shared<ActLicenseNotActiveFailed>("Operation");
  } */

  if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
    QString error_msg = "The system is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Get device ip by device IP
  qint64 device_id = 0;
  ActDevice device;
  act_status = monitor_project_.GetDeviceByIp(device_ip, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get device id by ip failed with ip:" << device_ip;
    return act_status;
  }

  device_id = device.GetId();

  if (!g_monitor_basic_status.contains(device_id)) {
    QString error_msg = "The device is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  ActMonitorBasicStatus basic_status = g_monitor_basic_status[device_id];

  QMap<QString, ActMonitorPortStatusEntry> port_status_map;
  QList<ActInterface> interface_set = device.GetInterfaces();

  for (ActInterface intf : interface_set) {
    if (!intf.GetActive()) {
      continue;
    }
    port_status_map[intf.GetInterfaceName()] = basic_status.GetPortStatus().value(intf.GetInterfaceId());
  }

  port_status.SetPortStatus(port_status_map);

  return act_status;
}

ACT_STATUS ActCore::GetDeviceTrafficStatus(const qint64 &project_id, const qint64 &device_id,
                                           ActMonitorDeviceTrafficStatus &traffic_status) {
  ACT_STATUS_INIT();

  // If the license is not available, return the error
  /* if (!this->GetLicense().GetFeature().GetStage().GetOperation()) {
    QString error_msg = "The license does not support operation";
    qCritical() << __func__ << error_msg;

    return std::make_shared<ActLicenseNotActiveFailed>("Operation");
  } */

  if (project_id != monitor_project_.GetId()) {
    QString error_msg = "The project is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (!g_monitor_device_traffic.contains(device_id)) {
    QString error_msg = "The device is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  ActDeviceMonitorTraffic device_traffic_status = g_monitor_device_traffic[device_id];

  ActDevice device;
  act_status = act::core::g_core.GetDevice(project_id, device_id, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get device failed with device id:" << device_id;
    return act_status;
  }

  QMap<QString, ActMonitorTrafficStatisticsEntry> traffic_status_map;
  QList<ActInterface> interface_set = device.GetInterfaces();
  for (ActInterface intf : interface_set) {
    if (!intf.GetActive()) {
      continue;
    }
    ActMonitorTrafficStatisticsEntry traffic_entry;
    traffic_entry.SetTxTotalOctets(
        device_traffic_status.GetTrafficMap().value(intf.GetInterfaceId()).GetTxTotalOctets());
    traffic_entry.SetTxTotalPackets(
        device_traffic_status.GetTrafficMap().value(intf.GetInterfaceId()).GetTxTotalPackets());
    traffic_entry.SetTrafficUtilization(
        device_traffic_status.GetTrafficMap().value(intf.GetInterfaceId()).GetTrafficUtilization());
    traffic_status_map[intf.GetInterfaceName()] = traffic_entry;
  }

  traffic_status.SetTrafficMap(traffic_status_map);

  return act_status;
}

ACT_STATUS ActCore::GetMonitorDeviceTrafficStatus(const QString &device_ip,
                                                  ActMonitorDeviceTrafficStatus &traffic_status) {
  ACT_STATUS_INIT();

  // If the license is not available, return the error
  /* if (!this->GetLicense().GetFeature().GetStage().GetOperation()) {
    QString error_msg = "The license does not support operation";
    qCritical() << __func__ << error_msg;

    return std::make_shared<ActLicenseNotActiveFailed>("Operation");
  } */

  if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
    QString error_msg = "The system is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Get device ip by device IP
  qint64 device_id = 0;
  ActDevice device;
  act_status = monitor_project_.GetDeviceByIp(device_ip, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get device id by ip failed with ip:" << device_ip;
    return act_status;
  }

  device_id = device.GetId();

  if (!g_monitor_device_traffic.contains(device_id)) {
    QString error_msg = "The device is not under operating";
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  ActDeviceMonitorTraffic device_traffic_status = g_monitor_device_traffic[device_id];

  QMap<QString, ActMonitorTrafficStatisticsEntry> traffic_status_map;
  QList<ActInterface> interface_set = device.GetInterfaces();
  for (ActInterface intf : interface_set) {
    if (!intf.GetActive()) {
      continue;
    }
    ActMonitorTrafficStatisticsEntry traffic_entry;
    traffic_entry.SetTxTotalOctets(
        device_traffic_status.GetTrafficMap().value(intf.GetInterfaceId()).GetTxTotalOctets());
    traffic_entry.SetTxTotalPackets(
        device_traffic_status.GetTrafficMap().value(intf.GetInterfaceId()).GetTxTotalPackets());
    traffic_entry.SetTrafficUtilization(
        device_traffic_status.GetTrafficMap().value(intf.GetInterfaceId()).GetTrafficUtilization());
    traffic_status_map[intf.GetInterfaceName()] = traffic_entry;
  }

  traffic_status.SetTrafficMap(traffic_status_map);

  return act_status;
}

}  // namespace core
}  // namespace act
