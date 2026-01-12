#include "act_topology_mapping.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QDebug>
#include <QQueue>

#include "act_auto_scan.hpp"
#include "act_core.hpp"

using namespace act::topology;

ActTopologyMapping::~ActTopologyMapping() {
  if ((topology_mapping_thread_ != nullptr) && (topology_mapping_thread_->joinable())) {
    topology_mapping_thread_->join();
  }
}

ACT_STATUS ActTopologyMapping::UpdateProgress(quint8 progress) {
  ACT_STATUS_INIT();
  progress_ = progress;
  qDebug() << __func__ << QString("Progress: %1%.").arg(GetProgress()).toStdString().c_str();
  return act_status;
}

ACT_STATUS ActTopologyMapping::GetStatus() {
  if (IsActStatusSuccess(topology_mapping_act_status_) && (progress_ == 100)) {
    topology_mapping_act_status_->SetStatus(ActStatusType::kFinished);
  }

  if ((!IsActStatusRunning(topology_mapping_act_status_)) && (!IsActStatusFinished(topology_mapping_act_status_))) {
    // failed
    return topology_mapping_act_status_;
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*topology_mapping_act_status_), progress_);
}

ACT_STATUS ActTopologyMapping::Stop() {
  // Checking has the thread is running
  if (IsActStatusRunning(topology_mapping_act_status_)) {
    qDebug() << "Stop TopologyMapping's running thread.";

    southbound_.SetStopFlag(true);

    // Send the stop signal and wait for the thread to finish.
    stop_flag_ = true;
    if ((topology_mapping_thread_ != nullptr) && (topology_mapping_thread_->joinable())) {
      topology_mapping_thread_->join();  // wait thread finished
    }
  } else {
    qDebug() << __func__ << "The TopologyMapping's thread not running.";
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*topology_mapping_act_status_), progress_);
}

ACT_STATUS ActTopologyMapping::Start(ActProject &project, ActTopologyMappingResult &mapping_result) {
  // Checking has the thread is running
  if (IsActStatusRunning(topology_mapping_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("TopologyMapping");
  }

  // init ActTopologyMapping status
  progress_ = 0;
  stop_flag_ = false;
  topology_mapping_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((topology_mapping_thread_ != nullptr) && (topology_mapping_thread_->joinable())) {
      topology_mapping_thread_->join();
    }
    topology_mapping_act_status_->SetStatus(ActStatusType::kRunning);
    topology_mapping_thread_ = std::make_unique<std::thread>(&ActTopologyMapping::TriggerMapperForThread, this,
                                                             std::ref(project), std::ref(mapping_result));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggerMapperForThread";
    HRESULT hr = SetThreadDescription(topology_mapping_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    qDebug() << "Start Mapper thread.";

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(Mapper) failed. Error:" << e.what();
    topology_mapping_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("Mapper");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*topology_mapping_act_status_), progress_);
}

void ActTopologyMapping::TriggerMapperForThread(ActProject &project, ActTopologyMappingResult &mapping_result) {
  // Triggered the Mapper and wait for the return, and update topology_mapping_act_status_.
  try {
    topology_mapping_act_status_ = Mapper(project, mapping_result);

  } catch (std::exception &e) {
    // no effect on the program's behavior
    static_cast<void>(e);
    topology_mapping_act_status_ = std::make_shared<ActStatusInternalError>("TopologyMapping");
  }
}

// ACT_STATUS ActTopologyMapping::ExecuteSyncModule(ActProject &project) {
//   ACT_STATUS_INIT();

//   UpdateProgress(10);

//   ActSync sync(profiles_);
//   act_status = sync.Start(project);

//   while (true) {
//     SLEEP_MS(1000);
//     act_status = sync.GetStatus();
//     if (act_status->GetStatus() != ActStatusType::kRunning) {
//       break;
//     }
//     if (stop_flag_) {
//       sync.Stop();
//       return ACT_STATUS_STOP;
//     }
//   }
//   if (act_status->GetStatus() != ActStatusType::kFinished) {
//     qCritical() << __func__ << "Sync(Check EndStation) failed.";
//     return act_status;
//   }

//   sync_device_error_map_ = sync.GetDeviceErrorMap();

//   return ACT_STATUS_SUCCESS;
// }

ACT_STATUS ActTopologyMapping::Mapper(ActProject &project, ActTopologyMappingResult &mapping_result) {
  ACT_STATUS_INIT();

  UpdateProgress(20);

  // Progress(20~80)
  // Scan physical topology
  ActMapTopology online_topology;
  act_status = ScanPhysicalTopology(project, online_topology);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << "ScanPhysicalTopology() failed.";
  }
  UpdateProgress(80);

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Topology mapping
  ActMapTopology offline_topology(project.GetDevices().values(), project.GetLinks().values());

  // Find candidates for the offline source device.
  QList<qint64> source_device_candidates;
  act_status = FindOfflineSourceDeviceCandidates(offline_topology, online_topology, source_device_candidates);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << "FindOfflineSourceDeviceCandidates() failed.";
  }
  qDebug() << "source_device_candidates size:" << source_device_candidates.size();

  if (source_device_candidates.size() == 0) {  // for generate the mapping_result report
    MappingTopology(offline_topology, online_topology, mapping_result);
  } else {
    // Use the candidates to Mapping topology
    MappingTopologyByOfflineSources(offline_topology, online_topology, source_device_candidates, mapping_result);
  }

  // Can's Deploy print log.
  if (!mapping_result.GetDeploy()) {
    // qInfo() << __func__ << "OnlineTopology:" << online_topology.ToString().toStdString().c_str();
    qInfo() << __func__ << "MappingResult:" << mapping_result.ToString().toStdString().c_str();
  } else {
    // Update the MappingDeviceIpSettingTable for deploy to change IP
    UpdateIpSettingMapByMappingResult(project, mapping_result);

    // Update Project Device's MAC address & mac_int & connect config (SNMP, RESTful, NETCONF)
    UpdateProjectDevicesConfiguration(project, online_topology.GetDevices(), mapping_result);

    // Update Online Topology for project
    project.online_topology_ = ActOnlineTopology(online_topology, mapping_result);
  }

  UpdateProgress(100);
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActTopologyMapping::UpdateProjectDevicesConfiguration(ActProject &project,
                                                                 const QList<ActDevice> &online_devices,
                                                                 const ActTopologyMappingResult &mapping_result) {
  ACT_STATUS_INIT();

  // Check can deploy
  if (!mapping_result.GetDeploy()) {
    return act_status;
  }

  auto new_devices = project.GetDevices();
  for (auto device_result : mapping_result.GetMappingReport()) {
    // Check device mapping status is Success or Warning
    if (device_result.GetStatus() == ActDeviceMapStatus::kSuccess ||
        device_result.GetStatus() == ActDeviceMapStatus::kWarning) {
      // Check Online IP address not empty
      if (device_result.GetOnlineIpAddress().isEmpty()) {
        continue;
      }

      ActDevice offline_device;
      act_status = project.GetDeviceById(offline_device, device_result.GetOfflineDeviceId());
      if (IsActStatusNotFound(act_status)) {
        // qDebug() << __func__
        //          << QString("Device(%1) not found in project, so skip update it MAC")
        //                 .arg(device_result.GetOfflineDeviceId());
        continue;
      }

      offline_device.SetMacAddress(device_result.GetOnlineMacAddress());
      qint64 mac_int = 0;
      act_status = MacAddressToQInt64(offline_device.GetMacAddress(), mac_int);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << "Transfer MAC address" << offline_device.GetMacAddress() << "failed";
        continue;
      }
      offline_device.mac_address_int = mac_int;

      // Connect config (SNMP, RESTful, NETCONF)
      auto online_device_index = online_devices.indexOf(ActDevice(device_result.GetOnlineDeviceId()));
      auto online_device = online_devices.at(online_device_index);
      offline_device.SetAccount(online_device.GetAccount());
      offline_device.SetNetconfConfiguration(online_device.GetNetconfConfiguration());
      offline_device.SetRestfulConfiguration(online_device.GetRestfulConfiguration());
      offline_device.SetSnmpConfiguration(online_device.GetSnmpConfiguration());
      offline_device.SetEnableSnmpSetting(online_device.GetEnableSnmpSetting());

      new_devices.remove(offline_device);
      new_devices.insert(offline_device);
    }
  }

  project.SetDevices(new_devices);
  return act_status;
}

ACT_STATUS ActTopologyMapping::UpdateIpSettingMapByMappingResult(ActProject &project,
                                                                 const ActTopologyMappingResult &mapping_result) {
  ACT_STATUS_INIT();

  // Check can deploy
  if (!mapping_result.GetDeploy()) {
    return act_status;
  }

  for (auto device_result : mapping_result.GetMappingReport()) {
    // Check device mapping status is Success or Warning
    if (device_result.GetStatus() == ActDeviceMapStatus::kSuccess ||
        device_result.GetStatus() == ActDeviceMapStatus::kWarning) {
      // Check Online IP address not empty
      if (device_result.GetOnlineIpAddress().isEmpty()) {
        continue;
      }

      // Check IP are different
      if (device_result.GetOfflineIpAddress() == device_result.GetOnlineIpAddress()) {
        continue;
      }

      // Get Project Device by OfflineDeviceId
      ActDevice offline_device;
      project.GetDeviceById(offline_device, device_result.GetOfflineDeviceId());

      // Set ip_setting_table
      ActMappingDeviceIpSettingTable ip_setting_table(
          device_result.GetOfflineDeviceId(), device_result.GetOfflineIpAddress(), device_result.GetOnlineIpAddress(),
          device_result.GetOnlineMacAddress());

      if (offline_device.GetIpv4().GetSubnetMask().isEmpty()) {
        ip_setting_table.SetSubnetMask("255.255.255.0");
      } else {
        ip_setting_table.SetSubnetMask(offline_device.GetIpv4().GetSubnetMask());
      }
      ip_setting_table.SetGateway(offline_device.GetIpv4().GetGateway());
      ip_setting_table.SetDNS1(offline_device.GetIpv4().GetDNS1());
      ip_setting_table.SetDNS2(offline_device.GetIpv4().GetDNS2());

      project.GetDeviceConfig().GetMappingDeviceIpSettingTables()[device_result.GetOfflineDeviceId()] =
          ip_setting_table;
    }
  }

  return act_status;
}

ACT_STATUS ActTopologyMapping::GenerateDeviceLinksMap(const ActMapTopology &map_topology,
                                                      QMap<qint64, QSet<ActLink>> &result_device_links_map) {
  ACT_STATUS_INIT();

  result_device_links_map.clear();

  for (auto link : map_topology.GetLinks()) {
    // Insert Destination Device ID
    if (!result_device_links_map.contains(link.GetDestinationDeviceId())) {
      result_device_links_map[link.GetDestinationDeviceId()] = {link};
    } else {
      result_device_links_map[link.GetDestinationDeviceId()].insert(link);
    }

    // Destination Source ID
    if (!result_device_links_map.contains(link.GetSourceDeviceId())) {
      result_device_links_map[link.GetSourceDeviceId()] = {link};
    } else {
      result_device_links_map[link.GetSourceDeviceId()].insert(link);
    }
  }

  return act_status;
}

ACT_STATUS ActTopologyMapping::GenerateLeaveInterfaceOppositeDeviceMap(const bool &check_moxa_vendor,
                                                                       const qint64 device_id,
                                                                       const QSet<ActLink> &device_links,
                                                                       const QSet<qint64> moxa_vendor_devices_id,
                                                                       QMap<qint64, qint64> &result_map) {
  ACT_STATUS_INIT();

  result_map.clear();

  for (auto neighbor_link : device_links) {
    if (neighbor_link.GetDestinationDeviceId() == device_id) {  // destination
      if (check_moxa_vendor) {
        if (moxa_vendor_devices_id.contains(neighbor_link.GetSourceDeviceId())) {
          result_map[neighbor_link.GetDestinationInterfaceId()] = neighbor_link.GetSourceDeviceId();
          continue;
        }
      } else {
        result_map[neighbor_link.GetDestinationInterfaceId()] = neighbor_link.GetSourceDeviceId();
        continue;
      }
    }
    if (neighbor_link.GetSourceDeviceId() == device_id) {  // source
      if (check_moxa_vendor) {
        if (moxa_vendor_devices_id.contains(neighbor_link.GetDestinationDeviceId())) {
          result_map[neighbor_link.GetSourceInterfaceId()] = neighbor_link.GetDestinationDeviceId();
          continue;
        }
      } else {
        result_map[neighbor_link.GetSourceInterfaceId()] = neighbor_link.GetDestinationDeviceId();
        continue;
      }
    }
  }

  return act_status;
}

ACT_STATUS ActTopologyMapping::GenerateOfflineMappingDeviceSet(const ActMapTopology &map_topology,
                                                               QSet<qint64> &result_device_set) {
  ACT_STATUS_INIT();

  result_device_set.clear();

  for (auto device : map_topology.GetDevices()) {
    // [bugfix:2854] Topology mapping - Moxa end station should be skipped
    // Skip EndStation
    if (device.GetDeviceType() == ActDeviceTypeEnum::kEndStation) {
      continue;
    }

    if (device.GetDeviceProperty().GetVendor() == ACT_VENDOR_MOXA ||
        device.GetDeviceProperty().GetVendor() == ACT_VENDOR_ID_MOXA) {
      result_device_set.insert(device.GetId());
    }
  }

  return act_status;
}

ACT_STATUS ActTopologyMapping::GenerateTopologyMappingResult(
    const QMap<qint64, ActMapDeviceResultItem> &offline_devices_result, const QSet<qint64> &found_online_devices,
    const QSet<qint64> &offline_end_station_devices, const ActMapTopology &online_topology,
    ActTopologyMappingResult &result) {
  ACT_STATUS_INIT();

  QSet<QString> mapping_offline_ip_set;
  QSet<QString> offline_end_station_checked_ip_set;
  bool offline_devices_can_deploy = true;

  // Generate result(offline devices)
  for (auto device_result_item : offline_devices_result) {
    qint64 item_id = result.GetMappingReport().size() + 1;
    device_result_item.SetId(item_id);
    if (device_result_item.GetStatus() == ActDeviceMapStatus::kSuccess ||
        device_result_item.GetStatus() == ActDeviceMapStatus::kWarning) {
      if (!offline_end_station_devices.contains(device_result_item.GetOfflineDeviceId())) {
        mapping_offline_ip_set.insert(device_result_item.GetOfflineIpAddress());
      }
    }

    // Add EndStation checked offline device
    if (device_result_item.GetStatus() == ActDeviceMapStatus::kChecked) {
      offline_end_station_checked_ip_set.insert(device_result_item.GetOfflineIpAddress());
    }

    result.GetMappingReport().append(device_result_item);

    // Check offline devices can deploy
    if (device_result_item.GetStatus() == ActDeviceMapStatus::kFailed ||
        device_result_item.GetStatus() == ActDeviceMapStatus::kNotFound) {
      offline_devices_can_deploy = false;
    }
  }

  // Append online not be found device to result
  for (auto online_device : online_topology.GetDevices()) {  // online
    if (!found_online_devices.contains(online_device.GetId())) {
      qint64 item_id = result.GetMappingReport().size() + 1;

      ActMapDeviceResultItem device_result_item;
      device_result_item.SetId(item_id);
      device_result_item.SetOnlineDeviceId(online_device.GetId());
      device_result_item.SetOnlineIpAddress(online_device.GetIpv4().GetIpAddress());
      device_result_item.SetOnlineModelName(online_device.GetDeviceProperty().GetModelName());
      device_result_item.SetOnlineMacAddress(online_device.GetMacAddress());

      // When offline devices can deploy, would check ip duplicated with offline devices
      if (offline_devices_can_deploy && mapping_offline_ip_set.contains(device_result_item.GetOnlineIpAddress())) {
        device_result_item.SetStatus(ActDeviceMapStatus::kFailed);
        device_result_item.SetErrorMessage("The IP duplicated with the offline design device");
      } else {
        device_result_item.SetStatus(ActDeviceMapStatus::kSkip);
      }

      // Skip Checked EndStation
      if (device_result_item.GetStatus() == ActDeviceMapStatus::kSkip &&
          offline_end_station_checked_ip_set.contains(device_result_item.GetOnlineIpAddress())) {
        continue;
      }

      // Assign to result_offline_devices
      result.GetMappingReport().append(device_result_item);
    }
  }

  // sorting result
  auto mapping_report = result.GetMappingReport();
  std::sort(mapping_report.begin(), mapping_report.end());
  result.SetMappingReport(mapping_report);

  // Check can deploy
  bool can_deploy = true;
  for (auto device_result_item : result.GetMappingReport()) {
    if (device_result_item.GetStatus() == ActDeviceMapStatus::kFailed ||
        device_result_item.GetStatus() == ActDeviceMapStatus::kNotFound) {
      can_deploy = false;
      break;
    }
  }

  result.SetDeploy(can_deploy);

  return act_status;
}

ACT_STATUS ActTopologyMapping::CheckDevice(const ActDevice &offline_device, const ActDevice &online_device,
                                           const bool &built_in_power,
                                           QMap<qint64, ActMapDeviceResultItem> &offline_devices_result,
                                           QSet<qint64> &offline_device_check_failed_id_set) {
  ACT_STATUS_INIT();

  // Avoid duplicated check
  if (!offline_devices_result.contains(offline_device.GetId())) {
    bool check_result = true;

    if (offline_device.GetDeviceProperty().GetModelName() != online_device.GetDeviceProperty().GetModelName()) {
      QString error_msg = "Check Model Name failed";
      offline_devices_result[offline_device.GetId()] =
          ActMapDeviceResultItem(offline_device, online_device, built_in_power, ActDeviceMapStatus::kFailed, error_msg);

      offline_device_check_failed_id_set.insert(offline_device.GetId());

      return std::make_shared<ActBadRequest>(error_msg);
    }

    // Check Module(Power)
    if (!built_in_power) {
      if (offline_device.GetModularConfiguration().GetPower() != online_device.GetModularConfiguration().GetPower()) {
        QString error_msg = "Check Power Module failed";
        offline_devices_result[offline_device.GetId()] = ActMapDeviceResultItem(
            offline_device, online_device, built_in_power, ActDeviceMapStatus::kFailed, error_msg);

        offline_device_check_failed_id_set.insert(offline_device.GetId());

        return std::make_shared<ActBadRequest>(error_msg);
      }
    }

    // Check Module(Ethernet)
    if (offline_device.GetModularConfiguration().GetEthernet() !=
        online_device.GetModularConfiguration().GetEthernet()) {
      QString error_msg = "Check Line Module failed";
      offline_devices_result[offline_device.GetId()] =
          ActMapDeviceResultItem(offline_device, online_device, built_in_power, ActDeviceMapStatus::kFailed, error_msg);

      offline_device_check_failed_id_set.insert(offline_device.GetId());

      return std::make_shared<ActBadRequest>(error_msg);
    }

    offline_devices_result[offline_device.GetId()] =
        ActMapDeviceResultItem(offline_device, online_device, built_in_power, ActDeviceMapStatus::kSuccess, "");
  }

  return act_status;
}

ACT_STATUS ActTopologyMapping::MappingTopology(const ActMapTopology &offline_topology,
                                               const ActMapTopology &online_topology,
                                               ActTopologyMappingResult &result) {
  ACT_STATUS_INIT();
  result = ActTopologyMappingResult();

  // Result
  QMap<qint64, ActMapDeviceResultItem> offline_devices_result;
  QSet<qint64> found_online_device_id_set;
  QSet<qint64> online_mapped_device_id_set;
  QSet<qint64> offline_not_found_port_device_id_set;
  QSet<qint64> offline_device_check_failed_id_set;

  // Prepare data
  QMap<qint64, QSet<ActLink>> offline_device_links_map;  // QMap<DeviceID, QSet<LinkID>>
  QMap<qint64, QSet<ActLink>> online_device_links_map;   // QMap<DeviceID, QSet<LinkID>>
  GenerateDeviceLinksMap(offline_topology, offline_device_links_map);
  GenerateDeviceLinksMap(online_topology, online_device_links_map);
  QSet<qint64> offline_mapping_device_id_set;
  GenerateOfflineMappingDeviceSet(offline_topology, offline_mapping_device_id_set);

  QQueue<qint64> offline_device_id_queue;
  qint64 online_device_id = online_topology.GetSourceDeviceId();

  // Check Source Device is MOXA device
  if (offline_mapping_device_id_set.contains(offline_topology.GetSourceDeviceId())) {
    offline_device_id_queue.enqueue(offline_topology.GetSourceDeviceId());
  }

  while (!offline_device_id_queue.isEmpty() && (!online_topology.GetDevices().isEmpty())) {
    auto offline_device_id = offline_device_id_queue.dequeue();

    if (offline_device_id != offline_topology.GetSourceDeviceId()) {  // not first
      // Get online_device_id
      online_device_id = -1;
      if (!offline_devices_result.contains(
              offline_device_id)) {  // next offline_device not found at the offline_devices_result
        qInfo() << __func__
                << QString("Offline Device not in offline_devices_result. DeviceID: %1, skip it")
                       .arg(offline_device_id)
                       .toStdString()
                       .c_str();
        continue;
      }
      online_device_id = offline_devices_result[offline_device_id].GetOnlineDeviceId();
      if (online_device_id == -1) {
        continue;
      }

      // [bugfix:2885] Topology mapping - mapping result show 1 online IP twice
      if (online_mapped_device_id_set.contains(online_device_id)) {
        continue;
      }
    }
    // qDebug() << __func__
    //          << QString("OfflineDevice(%1),  OnlineDevice(%2)")
    //                 .arg(offline_device_id)
    //                 .arg(online_device_id)
    //                 .toStdString()
    //                 .c_str();

    // Find Offline Device
    auto device_index = offline_topology.GetDevices().indexOf(ActDevice(offline_device_id));
    if (device_index == -1) {
      qCritical() << __func__ << "Offline device not found. DeviceID:" << offline_device_id;
      return std::make_shared<ActStatusNotFound>(QString("Device(%1) in offline topology").arg(offline_device_id));
    }
    auto offline_device = offline_topology.GetDevices().at(device_index);

    // Check Online device
    // Find Online Device
    device_index = online_topology.GetDevices().indexOf(ActDevice(online_device_id));
    if (device_index == -1) {
      qCritical() << __func__ << "Online device not found. DeviceID:" << online_device_id;
      return std::make_shared<ActStatusNotFound>(QString("Device(%1) in online topology").arg(online_device_id));
    }
    auto online_device = online_topology.GetDevices().at(device_index);
    found_online_device_id_set.insert(online_device_id);
    online_mapped_device_id_set.insert(online_device_id);

    // Get Profile
    ActDeviceProfile online_device_profile;
    act_status = ActGetItemById<ActDeviceProfile>(profiles_.GetDeviceProfiles(), online_device.GetDeviceProfileId(),
                                                  online_device_profile);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__
                  << QString("The device profile not found. DeviceProfile:%1")
                         .arg(online_device.GetDeviceProfileId())
                         .toStdString()
                         .c_str();
      return std::make_shared<ActStatusInternalError>("TopologyMapping");
    }
    bool &built_in_power = online_device_profile.GetBuiltInPower();

    // Avoid duplicated check
    if (!offline_devices_result.contains(offline_device_id)) {
      CheckDevice(offline_device, online_device, built_in_power, offline_devices_result,
                  offline_device_check_failed_id_set);
    }

    // Enqueue neighbor device by link
    for (auto neighbor_link : offline_device_links_map[offline_device_id]) {
      // Enqueue device(only uncheck device && not in the queue && is moxa device)
      auto next_device_id = neighbor_link.GetDestinationDeviceId();
      if ((!offline_devices_result.contains(next_device_id)) &&
          (!offline_device_id_queue.contains(next_device_id))) {  // destination
        if (offline_mapping_device_id_set.contains(next_device_id)) {
          offline_device_id_queue.enqueue(next_device_id);
        }
      }

      next_device_id = neighbor_link.GetSourceDeviceId();
      if ((!offline_devices_result.contains(next_device_id)) &&
          (!offline_device_id_queue.contains(next_device_id))) {  // source
        if (offline_mapping_device_id_set.contains(next_device_id)) {
          offline_device_id_queue.enqueue(next_device_id);
        }
      }
    }

    // Get offline leave_if_id
    QMap<qint64, qint64> offline_leave_if_and_opposite_dev_map;  // <leave_if_id, opposite_id>
    GenerateLeaveInterfaceOppositeDeviceMap(true, offline_device_id, offline_device_links_map[offline_device_id],
                                            offline_mapping_device_id_set, offline_leave_if_and_opposite_dev_map);

    // Get online leave_if_id
    QMap<qint64, qint64> online_leave_if_and_opposite_dev_map;  // <leave_if_id, opposite_id>
    GenerateLeaveInterfaceOppositeDeviceMap(false, online_device_id, online_device_links_map[online_device_id],
                                            offline_mapping_device_id_set, online_leave_if_and_opposite_dev_map);

    QMap<qint64, QString> offline_device_interfaces_name_map;
    offline_device.GetInterfacesIdNameMap(offline_device_interfaces_name_map);

    // Check offline one-hop device & link
    for (auto leave_if_id : offline_leave_if_and_opposite_dev_map.keys()) {
      // Get leave_if_name
      QString leave_if_name = offline_device_interfaces_name_map.contains(leave_if_id)
                                  ? offline_device_interfaces_name_map[leave_if_id]
                                  : QString::number(leave_if_id);

      if (!online_leave_if_and_opposite_dev_map.contains(leave_if_id)) {  // not found leave if

        if (!offline_devices_result.contains(offline_device_id)) {
          offline_devices_result[offline_device_id] =
              ActMapDeviceResultItem(offline_device, online_device, built_in_power, ActDeviceMapStatus::kFailed,
                                     QString("Port (%1) not found").arg(leave_if_name));
          offline_not_found_port_device_id_set.insert(offline_device_id);

        } else {
          if (offline_devices_result[offline_device_id].GetErrorMessage().isEmpty()) {
            offline_devices_result[offline_device_id] =
                ActMapDeviceResultItem(offline_device, online_device, built_in_power, ActDeviceMapStatus::kFailed,
                                       QString("Port (%1) not found").arg(leave_if_name));
            offline_not_found_port_device_id_set.insert(offline_device_id);
          } else {
            offline_devices_result[offline_device_id].GetErrorMessage() +=
                QString(", Port (%1) not found").arg(leave_if_name);
          }
        }
      } else {  // has leave if
        // Check Opposite Device

        // Find offline_opposite_device
        auto offline_opposite_device_id = offline_leave_if_and_opposite_dev_map[leave_if_id];
        // Avoid duplicated check
        if (offline_devices_result.contains(offline_opposite_device_id)) {
          continue;
        }

        auto device_index = offline_topology.GetDevices().indexOf(ActDevice(offline_opposite_device_id));
        if (device_index == -1) {
          qCritical() << __func__ << "Offline device not found. Device:" << offline_opposite_device_id;
          return std::make_shared<ActStatusNotFound>(
              QString("Offline device(%1) in OfflineTopology").arg(offline_opposite_device_id));
        }
        auto offline_opposite_device = offline_topology.GetDevices().at(device_index);

        // Find online_opposite_device
        auto online_opposite_device_id = online_leave_if_and_opposite_dev_map[leave_if_id];
        device_index = online_topology.GetDevices().indexOf(ActDevice(online_opposite_device_id));
        if (device_index == -1) {
          qCritical() << __func__ << "Online device not found. DeviceID:" << online_opposite_device_id;
          return std::make_shared<ActStatusNotFound>(
              QString("Online Device(%1) in OnlineTopology").arg(online_opposite_device_id));
        }
        auto online_opposite_device = online_topology.GetDevices().at(device_index);
        found_online_device_id_set.insert(online_opposite_device_id);

        // [bugfix:2885] Topology mapping - mapping result show 1 online IP twice
        if (online_mapped_device_id_set.contains(online_opposite_device_id)) {
          continue;
        }

        // Device Check
        ActDeviceProfile online_opposite_device_profile;
        act_status = ActGetItemById<ActDeviceProfile>(
            profiles_.GetDeviceProfiles(), online_opposite_device.GetDeviceProfileId(), online_opposite_device_profile);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__
                      << QString("The device profile not found. DeviceProfile:%1")
                             .arg(online_opposite_device.GetDeviceProfileId())
                             .toStdString()
                             .c_str();
          return std::make_shared<ActStatusInternalError>("TopologyMapping");
        }
        bool &built_in_power = online_opposite_device_profile.GetBuiltInPower();
        auto act_check_status = CheckDevice(offline_opposite_device, online_opposite_device, built_in_power,
                                            offline_devices_result, offline_device_check_failed_id_set);
        if (!IsActStatusSuccess(act_check_status)) {
          continue;
        }
      }
    }
  }

  // Update warning status
  for (auto &offline_device_result : offline_devices_result) {
    if (offline_device_result.GetStatus() == ActDeviceMapStatus::kFailed) {
      auto offline_device_id = offline_device_result.GetOfflineDeviceId();
      // Get not found port device
      if (offline_not_found_port_device_id_set.contains(offline_device_id)) {
        // Find offline link
        for (auto device_link : offline_device_links_map[offline_device_id]) {
          // Get opposite device id
          auto opposite_offline_device_id = device_link.GetSourceDeviceId() == offline_device_id
                                                ? device_link.GetDestinationDeviceId()
                                                : device_link.GetSourceDeviceId();
          // Check opposite device also not found port
          if (offline_not_found_port_device_id_set.contains(opposite_offline_device_id)) {
            offline_device_result.SetStatus(ActDeviceMapStatus::kWarning);
          }
        }
      }
    }
  }

  // Append others device to Result (Offline)
  QSet<qint64> offline_end_station_id_set;
  for (auto offline_device : offline_topology.GetDevices()) {
    if (!offline_devices_result.contains(offline_device.GetId())) {
      // If is the MOXA device would set SKIP.
      ActMapDeviceResultItem device_result_item;
      device_result_item.SetOfflineDeviceId(offline_device.GetId());
      device_result_item.SetOfflineIpAddress(offline_device.GetIpv4().GetIpAddress());
      device_result_item.SetOfflineModelName(offline_device.GetDeviceProperty().GetModelName());

      if (!offline_mapping_device_id_set.contains(offline_device.GetId())) {
        device_result_item.SetStatus(ActDeviceMapStatus::kSkip);
      } else {
        device_result_item.SetStatus(ActDeviceMapStatus::kNotFound);
        device_result_item.SetErrorMessage("Online device not found");
      }
      // Assign to offline_devices_result
      offline_devices_result[offline_device.GetId()] = device_result_item;
    }
  }

  GenerateTopologyMappingResult(offline_devices_result, found_online_device_id_set, offline_end_station_id_set,
                                online_topology, result);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActTopologyMapping::ScanPhysicalTopology(const ActProject &project,
                                                    ActMapTopology &result_physical_topology) {
  ACT_STATUS_INIT();

  QList<ActDevice> result_devices;
  QSet<ActLink> link_set;

  // Scan physical topology

  // Scan Devices by BroadcastSearch
  ActFeatureSubItem feature_sub_item;
  act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kAutoScan,
                                               "BroadcastSearch", "Basic", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetFeatureProfileFeatureSubItem() failed";
    return act_status;
  }

  QList<ActDevice> south_scan_devices;
  // Send BroadCast Search & Update mac_host_map
  act_status = southbound_.ActionBroadcastSearchDevices(feature_sub_item, south_scan_devices, mac_host_map_);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ActionBroadcastSearchDevices() failed.";
    return act_status;
  }

  UpdateProgress(20);

  //  Progress(20 ~ 70);
  // For each devices to set configuration
  quint8 dev_progress_slot = 50;
  if (south_scan_devices.size() > 0) {
    dev_progress_slot = dev_progress_slot / south_scan_devices.size();
  }
  const quint8 dev_progress_update_times = 2;
  for (auto &device : south_scan_devices) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    qDebug() << __func__
             << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

    // Set ArpTable
    act_status = southbound_.SetLocalhostArpTable(device, mac_host_map_[device.GetMacAddress()]);
    if (!IsActStatusSuccess(act_status)) {
      return std::make_shared<ActStatusSouthboundFailed>("Add localhost Arp entry failed.");
    }

    auto assign_cfg_act_status = AssignDeviceConfiguration(project, device);
    // Update progress
    quint8 new_progress = progress_ + (dev_progress_slot / dev_progress_update_times);
    UpdateProgress(new_progress);

    // Delete ArpTable's mac
    act_status = southbound_.DeleteLocalhostArpEntry(device);
    if (!IsActStatusSuccess(act_status)) {
      return std::make_shared<ActStatusSouthboundFailed>("Delete localhost Arp entry failed.");
    }

    if (!IsActStatusSuccess(assign_cfg_act_status)) {
      DeviceErrorLogHandler(__func__, "AssignDeviceConfiguration() Failed", device);
      continue;
    }

    // Update progress(20~70)
    new_progress = progress_ + (dev_progress_slot / dev_progress_update_times);
    if (new_progress >= 70) {
      UpdateProgress(70);
    } else {
      UpdateProgress(new_progress);
    }
  }

  // For each devices to create Link
  for (auto &device : south_scan_devices) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    QList<ActLink> south_link_list;
    auto device_link_act_status = CreateDeviceLink(device, south_scan_devices, south_link_list);
    if (!IsActStatusSuccess(device_link_act_status)) {
      DeviceErrorLogHandler(__func__, "CreateDeviceLink() Failed", device);
      continue;
    }

    // qDebug() << __func__
    //          << QString("Device(%1), South link size(%2)")
    //                 .arg(device.GetIpv4().GetIpAddress())
    //                 .arg(south_link_list.size())
    //                 .toStdString()
    //                 .c_str();

    // Assign link to link Set(remove duplicated link (forward & opposite))
    for (auto link : south_link_list) {
      ActLink new_link(link);
      new_link.SetId(link_set.size() + 1);
      link_set.insert(new_link);
    }
  }

  // Append to result device list
  result_devices = south_scan_devices;

  // Get SourceDevice(LLDP or MAC table)
  ActSourceDevice source_device;
  act_status = southbound_.FindSourceDeviceByLLDPAndMacTable(result_devices, source_device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Find online source Device failed.";
    return act_status;
  }

  // Set the result_physical_topology
  result_physical_topology.SetDevices(result_devices);
  result_physical_topology.SetLinks(link_set.values());  // QList to QSet
  result_physical_topology.SetSourceDeviceId(source_device.GetDeviceId());

  UpdateProgress(75);

  return act_status;
}

ACT_STATUS ActTopologyMapping::MappingTopologyByOfflineSources(const ActMapTopology &offline_topology,
                                                               const ActMapTopology &online_topology,
                                                               const QList<qint64> &offline_source_devices,
                                                               ActTopologyMappingResult &mapping_result) {
  ACT_STATUS_INIT();

  // For each candidates to MappingTopology try to get best result;
  QList<ActTopologyMappingResultCandidate> mapping_result_candidates;
  for (auto ofl_src_dev_id : offline_source_devices) {
    ActTopologyMappingResult topology_mapping_result;

    // Create candidate offline_topology with ofl_src_dev_id
    ActMapTopology candidate_offline_topology(offline_topology);
    candidate_offline_topology.SetSourceDeviceId(ofl_src_dev_id);

    // Start mapping algorithm
    MappingTopology(candidate_offline_topology, online_topology, topology_mapping_result);
    ActTopologyMappingResultCandidate candidate(topology_mapping_result, candidate_offline_topology);
    if (topology_mapping_result.GetDeploy() && (candidate.GetWarningItemNum() == 0) &&
        (candidate.GetFailedItemNum() == 0)) {
      // Directly get result
      mapping_result = topology_mapping_result;
      // qDebug() << __func__
      //          << QString("Candidate src(%1) directly as the mapping_result(no failed & no warning)")
      //                 .arg(ofl_src_dev_id)
      //                 .toStdString()
      //                 .c_str();
      return ACT_STATUS_SUCCESS;
    } else {
      mapping_result_candidates.append(candidate);
      // qDebug() << __func__
      //          << QString("mapping_result_candidates: candidate src(%1): %2")
      //                 .arg(ofl_src_dev_id)
      //                 .arg(candidate.ToString({"WarningItemNum", "FailedItemNum", "TopologyMappingResult"}))
      //                 .toStdString()
      //                 .c_str();
    }
  }

  // Select the mapping_result, when the mapping_result not deploy (not directly get result)
  if (!mapping_result.GetDeploy()) {
    // Sort mapping_result candidates
    // 1. Deploy success at front
    //    - Compare Warning items (from small to large)
    //    -- IP address (from small to large)
    // 2. Deploy failed
    //    - Compare Failed items (from small to large)
    //    -- IP address (from small to large)
    std::sort(mapping_result_candidates.begin(), mapping_result_candidates.end(),
              [](const ActTopologyMappingResultCandidate &x, const ActTopologyMappingResultCandidate &y) {
                // 1.Deploy success (x success & y success)
                if (x.GetTopologyMappingResult().GetDeploy() && y.GetTopologyMappingResult().GetDeploy()) {
                  // Compare Warning items (from small to large)
                  return x.GetWarningItemNum() < y.GetWarningItemNum() ||
                         (x.GetWarningItemNum() == y.GetWarningItemNum() &&
                          x.GetOfflineSourceIpNumber() < y.GetOfflineSourceIpNumber());
                }

                // 2. Deploy failed (x failed & y failed)
                if ((!x.GetTopologyMappingResult().GetDeploy()) && (!y.GetTopologyMappingResult().GetDeploy())) {
                  // Compare Failed items (from small to large)
                  return x.GetFailedItemNum() < y.GetFailedItemNum() ||
                         (x.GetFailedItemNum() == y.GetFailedItemNum() &&
                          x.GetOfflineSourceIpNumber() < y.GetOfflineSourceIpNumber());
                }

                // x success && y failed (Remove x to front)
                if (x.GetTopologyMappingResult().GetDeploy() && (!y.GetTopologyMappingResult().GetDeploy())) {
                  return true;  // x << y
                }

                // x failed & y success
                return false;
              });

    // Get the first as the mapping_result
    if (mapping_result_candidates.size() != 0) {
      auto mapping_result_candidate = ActTopologyMappingResultCandidate(*mapping_result_candidates.begin());
      mapping_result = mapping_result_candidate.GetTopologyMappingResult();
      auto ofl_src_id = mapping_result_candidate.GetOfflineTopology().GetSourceDeviceId();
      auto ofl_src_ip_num = mapping_result_candidate.GetOfflineSourceIpNumber();
      QString ip_str;
      ActIpv4::AddressNumberToStr(ofl_src_ip_num, ip_str);  // get ip string
      // qDebug() << __func__
      //          << QString("Selected mapping_result offline source device: %1(%2)")
      //                 .arg(ip_str)
      //                 .arg(ofl_src_id)
      //                 .toStdString()
      //                 .c_str();
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActTopologyMapping::FindOfflineSourceDeviceCandidates(const ActMapTopology &offline_topology,
                                                                 const ActMapTopology &online_topology,
                                                                 QList<qint64> &result_source_device_candidates) {
  ACT_STATUS_INIT();

  result_source_device_candidates.clear();

  QList<ActSourceDeviceCandidate> dev_candidates;

  // Get online source device
  auto onl_src_dev_id = online_topology.GetSourceDeviceId();
  auto onl_src_dev_idx = online_topology.GetDevices().indexOf(ActDevice(onl_src_dev_id));
  if (onl_src_dev_idx == -1) {
    qCritical() << __func__ << "Online source device not found. Device:" << onl_src_dev_id;
    return std::make_shared<ActStatusNotFound>(QString("Online device(%1) in OnlineTopology").arg(onl_src_dev_id));
  }
  auto onl_src_dev = online_topology.GetDevices().at(onl_src_dev_idx);

  // Get online links Interface that connect to onl_source_device
  QList<qint64> onl_src_up_link_ifs;  // QList<IntefaceID>
  for (auto onl_link : online_topology.GetLinks()) {
    if (onl_src_dev_id == onl_link.GetDestinationDeviceId()) {
      onl_src_up_link_ifs.append(onl_link.GetDestinationInterfaceId());
      continue;
    }

    if (onl_src_dev_id == onl_link.GetSourceDeviceId()) {
      onl_src_up_link_ifs.append(onl_link.GetSourceInterfaceId());
      continue;
    }
  }

  QList<ActDevice> ofl_same_model_devs;
  // Get offline source device candidates
  for (auto ofl_dev : offline_topology.GetDevices()) {
    // Check Model Name
    if (ofl_dev.GetDeviceProperty().GetModelName() != onl_src_dev.GetDeviceProperty().GetModelName()) {
      // qDebug() << __func__
      //          << QString(
      //                 "OfflineDevice(%1) not as Candidate. OfflineDevice ModelName(%2) != OnlineSourceDevice(%3(%4))
      //                 " "ModelName(%5)") .arg(ofl_dev.GetIpv4().GetIpAddress())
      //                 .arg(ofl_dev.GetDeviceProperty().GetModelName())
      //                 .arg(onl_src_dev.GetIpv4().GetIpAddress())
      //                 .arg(onl_src_dev.GetMacAddress())
      //                 .arg(onl_src_dev.GetDeviceProperty().GetModelName())
      //                 .toStdString()
      //                 .c_str();
      continue;
    }
    ofl_same_model_devs.append(ofl_dev);

    // Get links Interface that connect to this device
    QList<qint64> ofl_dev_up_link_ifs;  // QList<IntefaceID>
    for (auto ofl_link : offline_topology.GetLinks()) {
      if (ofl_dev.GetId() == ofl_link.GetDestinationDeviceId()) {
        ofl_dev_up_link_ifs.append(ofl_link.GetDestinationInterfaceId());
        continue;
      }

      if (ofl_dev.GetId() == ofl_link.GetSourceDeviceId()) {
        ofl_dev_up_link_ifs.append(ofl_link.GetSourceInterfaceId());
        continue;
      }
    }

    // - Check number of link
    // offline device link >= online device link (avoid loop, need to disconnect a link)
    if (ofl_dev_up_link_ifs.size() < onl_src_up_link_ifs.size()) {
      // qDebug() << __func__
      //          << QString(
      //                 "OfflineDevice(%1) not as Candidate. OfflineDevice NumOfLinkUp(%2) < OnlineSourceDevice(%3(%4))
      //                 " "NumOfLinkUp(%5)") .arg(ofl_dev.GetIpv4().GetIpAddress()) .arg(ofl_dev_up_link_ifs.size())
      //                 .arg(onl_src_dev.GetIpv4().GetIpAddress())
      //                 .arg(onl_src_dev.GetMacAddress())
      //                 .arg(onl_src_up_link_ifs.size())
      //                 .toStdString()
      //                 .c_str();
      continue;
    }

    // - Check Interface ID
    // onl_src_up_link_ifs's interface in the ofl_dev_up_link_ifs
    bool check_interface_id = true;
    for (auto onl_if_id : onl_src_up_link_ifs) {
      if (!ofl_dev_up_link_ifs.contains(onl_if_id)) {
        check_interface_id = false;
        break;
      }
    }
    if (check_interface_id == false) {
      QString ofl_dev_up_link_ifs_str;
      QString onl_src_up_link_ifs_str;
      for (auto ofl_id : ofl_dev_up_link_ifs) {
        ofl_dev_up_link_ifs_str.append(QString("%1 ").arg(ofl_id));
      }
      for (auto onl_id : onl_src_up_link_ifs) {
        onl_src_up_link_ifs_str.append(QString("%1 ").arg(onl_id));
      }

      // qDebug() << __func__
      //          << QString(
      //                 "OfflineDevice(%1)(LinkUpID: %2) not as Candidate. Linkup ID failed compared to "
      //                 "OnlineSourceDevice(%3(%4))(LinkUpID: %5)")
      //                 .arg(ofl_dev.GetIpv4().GetIpAddress())
      //                 .arg(ofl_dev_up_link_ifs_str)
      //                 .arg(onl_src_dev.GetIpv4().GetIpAddress())
      //                 .arg(onl_src_dev.GetMacAddress())
      //                 .arg(onl_src_up_link_ifs_str)
      //                 .toStdString()
      //                 .c_str();

      continue;
    }

    // Append to result
    quint32 ip_num;
    ofl_dev.GetIpv4().GetIpAddressNumber(ip_num);
    dev_candidates.append(ActSourceDeviceCandidate(ofl_dev.GetId(), ip_num, ofl_dev_up_link_ifs));
  }

  // If offline source device candidates is empty, would try to find same IP & ModelName as candidate
  if (dev_candidates.isEmpty()) {
    for (auto ofl_same_model_dev : ofl_same_model_devs) {
      if (ofl_same_model_dev.GetIpv4().GetIpAddress() == onl_src_dev.GetIpv4().GetIpAddress()) {
        // Append to result
        quint32 ip_num;
        ofl_same_model_dev.GetIpv4().GetIpAddressNumber(ip_num);
        dev_candidates.append(ActSourceDeviceCandidate(ofl_same_model_dev.GetId(), ip_num));
      }
    }
  }

  // sort candidate device
  // 1. Same IP with online_source_device (TBD)
  // 2. Number of interface  (from large to small)
  // 3. IP address (from small to large)
  quint32 onl_src_dev_ip_num;
  onl_src_dev.GetIpv4().GetIpAddressNumber(onl_src_dev_ip_num);
  std::sort(dev_candidates.begin(), dev_candidates.end(),
            [onl_src_dev_ip_num](const ActSourceDeviceCandidate &x, const ActSourceDeviceCandidate &y) {
              // 1. Same IP with online_source_device(move to front)
              if (x.GetIpNumber() == onl_src_dev_ip_num && y.GetIpNumber() != onl_src_dev_ip_num) {
                return true;  // a << b
              }
              if (x.GetIpNumber() != onl_src_dev_ip_num && y.GetIpNumber() == onl_src_dev_ip_num) {
                return false;  // b << a
              }

              // 2. Number of interface  (from large to small)
              // 3. IP address (from small to large)
              return y.GetUpLinkInterfacesId().size() < x.GetUpLinkInterfacesId().size() ||
                     (x.GetUpLinkInterfacesId().size() == y.GetUpLinkInterfacesId().size() &&
                      x.GetIpNumber() < y.GetIpNumber());
            });

  // Append to result
  for (auto dev_candidate : dev_candidates) {
    QString ip_str;
    ActIpv4::AddressNumberToStr(dev_candidate.GetIpNumber(), ip_str);  // get ip string
    // qDebug() << __func__
    //          << QString("Selected Offline Source Device Candidate: %1(%2)")
    //                 .arg(ip_str)
    //                 .arg(dev_candidate.GetId())
    //                 .toStdString()
    //                 .c_str();
    result_source_device_candidates.append(dev_candidate.GetId());
  }

  return act_status;
}

ACT_STATUS ActTopologyMapping::CreateDeviceLink(const ActDevice &device, const QList<ActDevice> &alive_device_list,
                                                QList<ActLink> &result_link_list) {
  ACT_STATUS_INIT();

  // Get device's links (use lldp & mac_table)
  QSet<ActLink> south_links;

  // Use LLDP data to generate links

  // - Access links by LLDP
  ActScanLinksResult south_result;
  act_status = southbound_.GenerateLinkSetBylldpInfo(device, alive_device_list, south_result);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GenerateLinkSetBylldpInfo() failed.";
    return act_status;
  }
  south_links = south_result.GetScanLinks();

  // For each link
  for (auto south_link : south_links) {
    // Check link hasn't same Source & Destination.
    if (south_link.GetSourceDeviceId() == south_link.GetDestinationDeviceId()) {
      qCritical() << __func__
                  << QString("Device IP %1 is duplicated.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

      return std::make_shared<ActDuplicatedError>(QString("Device IP %1").arg(device.GetIpv4().GetIpAddress()));
    }

    // Append to result list
    result_link_list.append(south_link);
  }

  return act_status;
}

ACT_STATUS ActTopologyMapping::AssignDeviceConfiguration(const ActProject &project, ActDevice &device) {
  ACT_STATUS_INIT();

  // Find offline device by IP
  qint64 offline_device_id = -1;
  act_status = project.GetDeviceIdByIp(offline_device_id, device.GetIpv4().GetIpAddress());
  if (!IsActStatusNotFound(act_status)) {
    ActDevice offline_device;
    project.GetDeviceById(offline_device, offline_device_id);
    act::core::g_core.CopyConnectionConfigField(device, offline_device);
    device.SetEnableSnmpSetting(offline_device.GetEnableSnmpSetting());

  } else {
    // Set device's connect configuration, If not found offline device
    act::core::g_core.CopyConnectionConfigField(device, project.GetProjectSetting());
    device.SetEnableSnmpSetting(true);
  }

  // Assign the Vendor to Device -> DeviceProperty -> Vendor
  auto device_property = device.GetDeviceProperty();
  device_property.SetVendor(ACT_VENDOR_MOXA);
  device.SetDeviceProperty(device_property);

  // // Force Enable SNMP
  // southbound_.FeatureEnableDeviceSnmp(true, device, false);

  // Update SNMP connect status by southbound
  ActDeviceConnectStatusControl control(true, true, false,
                                        false);  // (RESTful(true), SNMP(true), NETCONF(false), NewMOXACommand(false))
  act_status = southbound_.FeatureAssignDeviceStatus(true, device, control);
  if (!IsActStatusSuccess(act_status)) {
    DeviceErrorLogHandler(__func__, "FeatureAssignDeviceStatus() Failed", device);
    return act_status;
  }

  // Identify Device
  act_status = southbound_.IdentifyDevice(device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "IdentifyDevice() failed.";
    return act_status;
  }

  // Assign Lldp Data & Interfaces & MAC Table
  act_status = southbound_.AssignDeviceLldpData(device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "AssignDeviceLldpData() failed.";
    return act_status;
  }

  act_status = southbound_.AssignDeviceMacTable(device);
  if (!IsActStatusSuccess(act_status)) {
    qWarning() << __func__ << "AssignDeviceMacTable() failed.";
  }

  act_status = southbound_.AssignDeviceSerialNumber(device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "AssignDeviceSerialNumber() failed.";
    return act_status;
  }

  act_status = southbound_.AssignDeviceModularConfiguration(device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "AssignDeviceModularConfiguration() failed.";
    return act_status;
  }

  act_status = southbound_.AssignInterfacesAndBuiltinPowerByModular(device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "AssignInterfacesAndBuiltinPowerByModular() failed.";
    return act_status;
  }

  act_status = southbound_.AssignDeviceInterfacesInfo(device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "AssignDeviceInterfacesInfo() failed.";
    return act_status;
  }

  return act_status;
}

void ActTopologyMapping::DeviceErrorLogHandler(const QString &error_fun, const QString &error_reason,
                                               const ActDevice &device) {
  qCritical() << error_fun.toStdString().c_str()
              << QString("%1. Device: %2(%3)")
                     .arg(error_reason)
                     .arg(device.GetIpv4().GetIpAddress())
                     .arg(device.GetId())
                     .toStdString()
                     .c_str();
}
