#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QHostAddress>
#include <QSet>
#include <algorithm>
#include <cmath>
#include <thread>

#include "act_auto_scan.hpp"
#include "act_core.hpp"

namespace act {
namespace core {

ACT_STATUS CheckInput(const ActProject &copy_project, const QList<ActScanIpRangeEntry> &scan_ip_ranges,
                      QList<ActScanIpRangeEntry> &new_scan_ip_ranges) {
  ACT_STATUS_INIT();

  if (scan_ip_ranges.size() == 0) {
    QString error_msg = QString("The scan IP range is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  act_status = g_core.CheckScanIpRanges(scan_ip_ranges);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << QString("Check the scan_ip_ranges failed.");
    return act_status;
  }

  for (auto scan_ip_range : scan_ip_ranges) {
    g_core.HandleConnectionConfigField(scan_ip_range, copy_project.GetProjectSetting(),
                                       copy_project.GetProjectSetting());
    new_scan_ip_ranges.push_back(scan_ip_range);
  }

  return act_status;
}

ACT_STATUS UpdateScanResultDeviceProfileId(act::topology::ActAutoScanResult &scan_result,
                                           QMap<qint64, qint64> old_new_device_profile_id_map) {
  ACT_STATUS_INIT();

  // Update DeviceProfile's ID of the device
  QSet<ActDevice> result_device_set;
  for (auto device : scan_result.GetDevices()) {
    // If use the old_device_profile id would update to new id
    if (old_new_device_profile_id_map.contains(device.GetDeviceProfileId())) {
      device.SetDeviceProfileId(old_new_device_profile_id_map[device.GetDeviceProfileId()]);
    }
    result_device_set.insert(device);
  }

  scan_result.SetDevices(result_device_set);

  return act_status;
}

ACT_STATUS CreateNewTopology(act::topology::ActAutoScanResult &scan_result, ActProject &project,
                             QSet<ActDevice> &success_update_devices) {
  // [feat:2257] UI Feature Redesign - New Auto Scan Topology
  ACT_STATUS_INIT();

  // [bugfix:3237] It should not be that a single device error causes the topology display failed
  QSet<qint64> failed_devices;
  QSet<qint64> failed_links;

  // Clear devices from project
  auto devs = project.GetDevices();
  for (ActDevice dev : devs) {
    qint64 devId(dev.GetId());
    act_status = g_core.DeleteDevice(project, devId);
    if (!IsActStatusSuccess(act_status)) {
      dev.HidePassword();
      qCritical() << __func__ << QString("Delete device failed. Device: %1").arg(dev.ToString()).toStdString().c_str();
      failed_devices.insert(dev.GetId());
      continue;
    }
  }

  // Fill the scan result to project - devices
  for (auto device : scan_result.GetDevices()) {
    // Modify the coordinate to (0, 0) for auto-scan location
    // [bugfix:2616] AutoLayout is ineffective when using NewTopology in AutoScan
    device.SetCoordinate(ActCoordinate(0, 0));
    // Create device
    const bool from_bag = false;
    act_status = g_core.CreateDevice(project, device, from_bag);
    if (!IsActStatusSuccess(act_status)) {
      device.HidePassword();
      qWarning() << __func__
                 << QString("Create device failed. Device: %1").arg(device.ToString()).toStdString().c_str();
      failed_devices.insert(device.GetId());
      continue;
    }
    success_update_devices.insert(device);
  }

  // Fill the scan result to project - links
  for (auto link : scan_result.GetLinks()) {
    act_status = g_core.CreateLink(project, link);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << __func__ << QString("Create link failed. Link: %1").arg(link.ToString()).toStdString().c_str();
      failed_links.insert(link.GetId());
      continue;
    }
  }

  // Return failed status or not
  // [bugfix:3237] It should not be that a single device error causes the topology display failed
  if ((!failed_devices.isEmpty()) || (!failed_links.isEmpty())) {
    return std::make_shared<ActStatusUpdateProjectTopologyFailed>(failed_devices, failed_links);
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS UpdateScanTopology(const act::topology::ActAutoScanResult &scan_result, ActProject &project,
                              QSet<ActDevice> &success_update_devices) {
  // [feat:2257] UI Feature Redesign - New Auto Scan Topology
  ACT_STATUS_INIT();

  // [bugfix:3237] It should not be that a single device error causes the topology display failed
  QSet<qint64> failed_devices;
  QSet<qint64> failed_links;

  // Add New Device to Project
  QList<ActDevice> update_devices_list;
  for (auto device : scan_result.GetDevices()) {
    // Skip exists device
    if (project.GetDevices().contains(device)) {
      qDebug() << __func__
               << QString("Skip device(%1(%2))")
                      .arg(device.GetId())
                      .arg(device.GetIpv4().GetIpAddress())
                      .toStdString()
                      .c_str();
      continue;
    }
    update_devices_list.append(device);
  }

  if (update_devices_list.size() != 0) {
    // [feat:2637] Auto Layout - add device to empty space
    qint32 update_devices_num = update_devices_list.size();
    double radius = update_devices_num == 1   ? 0
                    : update_devices_num == 2 ? 150
                                              : 150 / sin(M_PI * 2 / update_devices_num);

    ActCoordinate scan_coordinate_center(0, UINT_MAX);
    for (auto device : project.GetDevices()) {
      // find maximum X in topology
      qint64 X = device.GetCoordinate().GetX() + radius + 150;
      if (scan_coordinate_center.GetX() < X) {
        scan_coordinate_center.SetX(X);
      }
      // find minimum Y in topology
      qint64 Y = device.GetCoordinate().GetY() + radius;
      if (scan_coordinate_center.GetY() > Y) {
        scan_coordinate_center.SetY(Y);
      }
    }

    // Create devices
    for (int idx = 0; idx < update_devices_list.size(); idx++) {
      ActDevice device = update_devices_list[idx];
      device.GetCoordinate().SetX(scan_coordinate_center.GetX() + radius * cos(M_PI * 2 / update_devices_num * idx));
      device.GetCoordinate().SetY(scan_coordinate_center.GetY() + radius * sin(M_PI * 2 / update_devices_num * idx));

      const bool from_bag = false;
      act_status = g_core.CreateDevice(project, device, from_bag);
      if (!IsActStatusSuccess(act_status)) {
        device.HidePassword();
        qWarning() << __func__
                   << QString("Create device failed. Device: %1").arg(device.ToString()).toStdString().c_str();

        failed_devices.insert(device.GetId());
        continue;
      }
      success_update_devices.insert(device);
    }
  }

  // [feat:2734] Auto Scan - Update topology
  // Add links to Project
  for (auto link : scan_result.GetLinks()) {
    ActLink old_link;

    // Check Src interface
    auto found_link_status =
        project.GetLinkByInterfaceId(old_link, link.GetDestinationDeviceId(), link.GetDestinationInterfaceId());

    if (!IsActStatusNotFound(found_link_status)) {
      // remove old_link;
      act_status = g_core.DeleteLink(project, old_link.GetId());
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__
                   << QString("Delete link failed. Link: %1").arg(old_link.ToString()).toStdString().c_str();
        failed_links.insert(old_link.GetId());
      }
    }

    // Check Dst interface
    found_link_status = project.GetLinkByInterfaceId(old_link, link.GetSourceDeviceId(), link.GetSourceInterfaceId());
    if (!IsActStatusNotFound(found_link_status)) {
      // remove old_link;
      act_status = g_core.DeleteLink(project, old_link.GetId());
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__
                   << QString("Delete link failed. Link: %1").arg(old_link.ToString()).toStdString().c_str();

        failed_links.insert(old_link.GetId());
      }
    }

    // Create link
    link.SetId(-1);
    act_status = g_core.CreateLink(project, link);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << __func__ << QString("Create link failed. Link: %1").arg(link.ToString()).toStdString().c_str();

      failed_links.insert(link.GetId());
    }
  }

  // Return failed status or not
  // [bugfix:3237] It should not be that a single device error causes the topology display failed
  if ((!failed_devices.isEmpty()) || (!failed_links.isEmpty())) {
    return std::make_shared<ActStatusUpdateProjectTopologyFailed>(failed_devices, failed_links);
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ReplaceManagementEndpoint(act::topology::ActAutoScanResult &scan_result, ActProject &project) {
  ACT_STATUS_INIT();

  if (scan_result.GetManagementEndpoint().GetDeviceId() == -1) {  // not found ManagementInterface

    // clear old ManagementInterface
    for (auto mgmt_interface : project.GetTopologySetting().GetManagementInterfaces()) {
      act_status = g_core.DeleteManagementInterface(project, mgmt_interface.GetDeviceId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "Delete management_interface failed with device id:" << mgmt_interface.GetDeviceId();
        return act_status;
      }
    }
  } else {  // replace ManagementInterface
    act_status = g_core.UpdateManagementInterface(project, scan_result.GetManagementEndpoint());
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Update ManagementInterface failed";
      return act_status;
    }
  }

  return act_status;
}

void ActCore::OpcUaStartScanTopologyThread(qint64 project_id, ActScanIpRange scan_ip_ranges, bool new_topology,
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
  // set scan ip range to project setting
  QList<ActScanIpRangeEntry> scan_ip_range_entries = scan_ip_ranges.GetScanIpRangeEntries();
  for (ActScanIpRangeEntry &scan_ip_range_entry : scan_ip_range_entries) {
    g_core.HandleConnectionConfigField(scan_ip_range_entry, project.GetProjectSetting(), project.GetProjectSetting());
  }
  project.GetProjectSetting().SetScanIpRanges(scan_ip_range_entries);

  // Trigger compare procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::topology::ActAutoScan scanner(profiles);
  scanner.SetLatestNewDevProfileId(this->last_assigned_device_profile_id_);
  act::topology::ActAutoScanResult scan_result;

  // [feat:1662] Hide password
  // If the input is empty, which means it should use default setting
  act_status =
      scanner.Start(project.GetScanIpRanges(), true /* this->GetLicense().GetFeature().GetAutoProbe() */, scan_result);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start scan topology failed";
    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kScanning;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  QMap<qint64, qint64> old_new_device_profile_id_map;  // <Old DeviceProfileId, New DeviceProfileId>
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = scanner.GetStatus();

    // [feat:2604] Auto Scan - Get the unknown device need to execute the auto probe
    // upload probe device_profiles
    while (!scanner.probe_device_profiles_queue_.isEmpty()) {
      auto probe_device_profile = scanner.probe_device_profiles_queue_.dequeue();
      auto old_device_profile_id = probe_device_profile.GetId();
      auto act_status_upload = act::core::g_core.UploadDeviceProfile(probe_device_profile);
      if (!IsActStatusSuccess(act_status_upload)) {  // failed
        qCritical() << __func__
                    << QString("Upload new generated DeviceProfile failed. DeviceProfile:")
                           .arg(probe_device_profile.ToString())
                           .toStdString()
                           .c_str();
        cb_func(act_status_upload, arg);
        break;
      }

      // Update old_new_device_profile_id map
      old_new_device_profile_id_map[old_device_profile_id] = probe_device_profile.GetId();
    }

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
    scanner.Stop();
    qCritical() << project.GetProjectName() << "Abort scan topology";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  UpdateScanResultDeviceProfileId(scan_result, old_new_device_profile_id_map);

  QSet<ActDevice> success_update_devices;
  // NewTopology
  if (new_topology) {
    auto update_topology_status = CreateNewTopology(scan_result, project, success_update_devices);
    if (!IsActStatusSuccess(update_topology_status)) {
      auto log_scan_result = scan_result;
      log_scan_result.HidePassword();
      qWarning() << update_topology_status->ToString().toStdString().c_str();
      qWarning() << "Create Project new topology failed. ScanResult:"
                 << log_scan_result.ToString().toStdString().c_str();
    }
  } else {
    // Update Topology
    auto update_topology_status = UpdateScanTopology(scan_result, project, success_update_devices);
    if (!IsActStatusSuccess(update_topology_status)) {
      auto log_scan_result = scan_result;
      log_scan_result.HidePassword();
      qWarning() << update_topology_status->ToString().toStdString().c_str();
      qWarning() << "Update Project topology failed. ScanResult:" << log_scan_result.ToString().toStdString().c_str();
    }
  }

  // Write back the project status to the core memory
  act_status = this->UpdateProject(project, true);
  if (!IsActStatusSuccess(act_status)) {
    auto log_project = project;
    log_project.HidePassword();
    qCritical() << log_project.GetProjectName()
                << "Cannot update the project. Project:" << log_project.ToString().toStdString().c_str();
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
  act_status = scanner.GetStatus();
  cb_func(act_status, arg);

  return;
}

ACT_STATUS ActCore::OpcUaStartScanTopology(qint64 &project_id, ActScanIpRange &scan_ip_ranges, bool new_topology,
                                           std::future<void> signal_receiver,
                                           void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kScanning, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << project_name << "Spawn OPC UA scan topology thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartScanTopologyThread, this, project_id,
                                             scan_ip_ranges, new_topology, std::move(signal_receiver), cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartScanTopologyThread";
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

void ActCore::StartScanTopologyThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                      qint64 project_id, bool new_topology) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  QSet<ActDevice> success_update_devices;

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartScanTopology, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }
  auto backup_project = project;
  // Trigger compare procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::topology::ActAutoScan scanner(profiles);
  scanner.SetLatestNewDevProfileId(this->last_assigned_device_profile_id_);
  act::topology::ActAutoScanResult scan_result;

  // for (auto scan_ip_range_entry : project.GetScanIpRanges()) {
  //   ActScanIpRangeEntry hide_pwd_scan_ip_range_entry(scan_ip_range_entry);
  //   hide_pwd_scan_ip_range_entry.HidePassword();
  //   qDebug() << __func__ << hide_pwd_scan_ip_range_entry.ToString().toStdString().c_str();
  // }

  // [feat:1662] Hide password
  // If the input is empty, which means it should use default setting
  act_status =
      scanner.Start(project.GetScanIpRanges(), true /* this->GetLicense().GetFeature().GetAutoProbe() */, scan_result);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start scan topology failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartScanTopology, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kScanning;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  quint8 prev_progress = 0;
  QMap<qint64, qint64> old_new_device_profile_id_map;  // <Old DeviceProfileId, New DeviceProfileId>
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = scanner.GetStatus();

    // [feat:2604] Auto Scan - Get the unknown device need to execute the auto probe
    // upload probe device_profiles
    while (!scanner.probe_device_profiles_queue_.isEmpty()) {
      auto probe_device_profile = scanner.probe_device_profiles_queue_.dequeue();
      auto old_device_profile_id = probe_device_profile.GetId();
      auto act_status_upload = act::core::g_core.UploadDeviceProfile(probe_device_profile);
      if (!IsActStatusSuccess(act_status_upload)) {  // failed
        qCritical() << __func__
                    << QString("Upload new generated DeviceProfile failed. DeviceProfile:")
                           .arg(probe_device_profile.ToString())
                           .toStdString()
                           .c_str();

        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartScanTopology, *act_status_upload);
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
        break;
      }

      // Update old_new_device_profile_id map
      old_new_device_profile_id_map[old_device_profile_id] = probe_device_profile.GetId();
    }

    if (act_status->GetStatus() != ActStatusType::kFinished) {
      if (act_status->GetStatus() != ActStatusType::kRunning) {  // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartScanTopology, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      } else {
        quint8 progress = dynamic_cast<ActProgressStatus &>(*act_status).GetProgress();
        if (progress != prev_progress) {
          prev_progress = progress;
          ActProgressWSResponse ws_resp(ActWSCommandEnum::kStartScanTopology,
                                        dynamic_cast<ActProgressStatus &>(*act_status));
          qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
          this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
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
    scanner.Stop();
    qCritical() << project.GetProjectName() << "Abort scan topology";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  UpdateScanResultDeviceProfileId(scan_result, old_new_device_profile_id_map);

  // NewTopology
  if (new_topology) {
    auto update_topology_status = CreateNewTopology(scan_result, project, success_update_devices);
    if (!IsActStatusSuccess(update_topology_status)) {
      auto log_scan_result = scan_result;
      log_scan_result.HidePassword();
      qWarning() << update_topology_status->ToString().toStdString().c_str();
      qWarning() << "Create Project new topology failed. ScanResult:"
                 << log_scan_result.ToString().toStdString().c_str();
    }
  } else {
    // Update Topology
    auto update_topology_status = UpdateScanTopology(scan_result, project, success_update_devices);
    if (!IsActStatusSuccess(update_topology_status)) {
      auto log_scan_result = scan_result;
      log_scan_result.HidePassword();
      qWarning() << update_topology_status->ToString().toStdString().c_str();
      qWarning() << "Update Project topology failed. ScanResult:" << log_scan_result.ToString().toStdString().c_str();
    }
  }

  // Update DeviceConfig
  for (auto device : scan_result.GetDevices()) {
    UpdateDeviceDeviceConfig(project, scan_result.GetDeviceConfig(), device.GetId());
  }

  // Write back the project status to the core memory & topology
  act_status = this->UpdateProject(project, true);
  if (!IsActStatusSuccess(act_status)) {
    auto log_project = project;
    log_project.HidePassword();

    qCritical() << log_project.GetProjectName()
                << "Cannot update the project. Project:" << log_project.ToString().toStdString().c_str();
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartScanTopology, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << project.GetProjectName() << "Cannot update the backup project";
    }
    return;
  }

  // Use the specific format to reply the result
  QList<ActAutoScanResultItem> ws_result_list;
  for (auto device : success_update_devices) {
    ActAutoScanResultItem ws_result(device);
    ws_result_list.append(ws_result);
  }

  // Sort the list using qSort and a lambda function
  std::sort(ws_result_list.begin(), ws_result_list.end(),
            [](const ActAutoScanResultItem &a, const ActAutoScanResultItem &b) {
              return QHostAddress(a.GetIp()).toIPv4Address() < QHostAddress(b.GetIp()).toIPv4Address();
            });

  ActScanResultWSResponse ws_resp(ActWSCommandEnum::kStartScanTopology, ws_result_list);
  qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartScanTopology(qint64 &project_id, const qint64 &ws_listener_id, bool new_topology) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kScanning, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartScanTopology, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << project_name << "Spawn scan topology thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartScanTopologyThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, new_topology);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartScanTopologyThread";
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
