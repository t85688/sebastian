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
#include "act_broadcast_search.hpp"
#include "act_core.hpp"

using namespace act::topology;

ACT_STATUS ActTopologyMapping::UpdateManufactureReadyDevices(ActProject &project,
                                                             const ActTopologyMappingResult &mapping_result) {
  ACT_STATUS_INIT();

  // Check can deploy
  if (!mapping_result.GetDeploy()) {
    return act_status;
  }

  for (auto map_dev : mapping_result.GetMappingReport()) {
    ActDevice dev;
    project.GetDeviceById(dev, map_dev.GetOfflineDeviceId());

    ActManufactureResultDevice manufacture_dev(-1, dev);
    manufacture_dev.SetSerialNumber(map_dev.GetOnlineSerialNumber());
    manufacture_dev.SetEthernetModule(map_dev.GetOnlineEthernetModule());
    manufacture_dev.SetPowerModule(map_dev.GetOnlinePowerModule());

    ActIpv4::AddressStrToNumber(map_dev.GetOfflineIpAddress(), manufacture_dev.ip_num_);

    project.GetManufactureResult().GetReady().GetDevices().append(manufacture_dev);
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActTopologyMapping::StartScanMapper(ActProject &project, ActTopologyMappingResult &mapping_result) {
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
    topology_mapping_thread_ = std::make_unique<std::thread>(&ActTopologyMapping::TriggerScanMapperForThread, this,
                                                             std::ref(project), std::ref(mapping_result));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggerScanMapperForThread";
    HRESULT hr = SetThreadDescription(topology_mapping_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    qDebug() << "Start Scan Mapper thread.";

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(ScanMapper) failed. Error:" << e.what();
    topology_mapping_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("ScanMapper");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*topology_mapping_act_status_), progress_);
}

void ActTopologyMapping::TriggerScanMapperForThread(ActProject &project, ActTopologyMappingResult &mapping_result) {
  // Triggered the Mapper and wait for the return, and update topology_mapping_act_status_.
  try {
    topology_mapping_act_status_ = ScanMapper(project, mapping_result);

  } catch (std::exception &e) {
    // no effect on the program's behavior
    static_cast<void>(e);
    topology_mapping_act_status_ = std::make_shared<ActStatusInternalError>("ScanMapping");
  }
}

ACT_STATUS ActTopologyMapping::ScanMapper(ActProject &project, ActTopologyMappingResult &mapping_result) {
  ACT_STATUS_INIT();
  // auto &ip_setting_tables = project.GetDeviceConfig().GetMappingDeviceIpSettingTables();
  ActMapTopology offline_topology(project.GetDevices().values(), project.GetLinks().values());
  SortMapTopologyDevices(offline_topology);  // sort by IP
  QSet<qint64> remain_dev_id_set;

  // Init manufacture_result_ when first execute mapper
  if (project.GetManufactureResult().CheckIsFirstTime()) {
    // offline_topology
    act::core::g_core.InitManufactureResult(project);
  } else {
    // Clear manufacture_deploy_result ready devices
    project.GetManufactureResult().GetReady().GetDevices().clear();
  }

  for (auto manufacture_dev : project.GetManufactureResult().GetRemain().GetDevices()) {
    remain_dev_id_set.insert(manufacture_dev.GetDeviceId());
  }

  UpdateProgress(20);

  // Progress(20~80)
  // Scan physical topology
  ActMapTopology online_topology;
  act_status = ScanPhysicalTopology(project, online_topology);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << "ScanPhysicalTopology() failed.";
  }
  SortMapTopologyDevices(online_topology);

  UpdateProgress(80);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Mapping not deployed device
  bool can_deploy = true;

  for (auto online_dev : online_topology.GetDevices()) {
    // Find device_profile
    ActDeviceProfile online_dev_profile;
    act_status = ActGetItemById<ActDeviceProfile>(profiles_.GetDeviceProfiles(), online_dev.GetDeviceProfileId(),
                                                  online_dev_profile);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__
                  << QString("The device profile not found. DeviceProfile:%1")
                         .arg(online_dev.GetDeviceProfileId())
                         .toStdString()
                         .c_str();
      return std::make_shared<ActStatusInternalError>("ScanMapping");
    }
    bool &built_in_power = online_dev_profile.GetBuiltInPower();

    QPair<ActScanMappingLevel, QString> fail_pair(ActScanMappingLevel::kNone, "");  // QPair<mapping_level, error_msg>
    ActDevice target_offline_dev;
    for (auto offline_dev : offline_topology.GetDevices()) {
      // Check deployed
      if (!remain_dev_id_set.contains(offline_dev.GetId())) {
        if (fail_pair.first <= ActScanMappingLevel::kRemain) {
          fail_pair.first = ActScanMappingLevel::kRemain;
          fail_pair.second = QString("No remaining devices");
        }
        continue;
      }

      if (offline_dev.GetDeviceProperty().GetModelName() != online_dev.GetDeviceProperty().GetModelName()) {
        if (fail_pair.first <= ActScanMappingLevel::kModelName) {
          fail_pair.first = ActScanMappingLevel::kModelName;
          fail_pair.second = QString("Remaining devices have different Model Name");
        }
        continue;
      }

      if (!built_in_power) {
        if (offline_dev.GetModularConfiguration().GetPower() != online_dev.GetModularConfiguration().GetPower()) {
          if (fail_pair.first <= ActScanMappingLevel::kPowerModule) {
            fail_pair.first = ActScanMappingLevel::kPowerModule;
            fail_pair.second = QString("Remaining devices have different Power Module");
          }
          continue;
        }
      }

      if (offline_dev.GetModularConfiguration().GetEthernet() != online_dev.GetModularConfiguration().GetEthernet()) {
        if (fail_pair.first <= ActScanMappingLevel::kLineModule) {
          fail_pair.first = ActScanMappingLevel::kLineModule;
          fail_pair.second = QString("Remaining devices have different Ethernet Module");
        }
        continue;
      }

      target_offline_dev = offline_dev;
      break;
    }

    ActMapDeviceResultItem map_dev_result_item;
    qint64 item_id = mapping_result.GetMappingReport().size() + 1;
    if (target_offline_dev.GetId() != -1) {
      remain_dev_id_set.remove(target_offline_dev.GetId());

      map_dev_result_item =
          ActMapDeviceResultItem(item_id, target_offline_dev, online_dev, built_in_power, ActDeviceMapStatus::kSuccess);
    } else {  // not found
      map_dev_result_item =
          ActMapDeviceResultItem(item_id, online_dev, built_in_power, ActDeviceMapStatus::kNotFound, fail_pair.second);
      can_deploy = false;
    }
    mapping_result.GetMappingReport().append(map_dev_result_item);
  }
  mapping_result.SetDeploy(can_deploy);

  // Can's Deploy print log.
  if (!can_deploy) {
    // qInfo() << __func__ << "OnlineTopology:" << online_topology.ToString().toStdString().c_str();
    qInfo() << __func__ << "MappingResult:" << mapping_result.ToString().toStdString().c_str();
  } else {
    // Update the MappingDeviceIpSettingTable for deploy to change IP
    UpdateIpSettingMapByMappingResult(project, mapping_result);

    // Update Project Device's MAC address & mac_int & connect config (SNMP, RESTful, NETCONF)
    UpdateProjectDevicesConfiguration(project, online_topology.GetDevices(), mapping_result);

    // Update the manufacture_result's Ready
    UpdateManufactureReadyDevices(project, mapping_result);

    // Update Online Topology for project
    project.online_topology_ = ActOnlineTopology(online_topology, mapping_result);
  }

  UpdateProgress(100);
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActTopologyMapping::SortMapTopologyDevices(ActMapTopology &map_topology) {
  ACT_STATUS_INIT();
  QList<ActDeviceDistanceEntry> distance_entry_list;

  if (map_topology.GetSourceDeviceId() == -1) {
    // Sort by IP num (from small to large)
    quint32 ip_num;
    for (auto dev : map_topology.GetDevices()) {
      dev.GetIpv4().GetIpAddressNumber(ip_num);
      distance_entry_list.append(ActDeviceDistanceEntry(dev.GetId(), ip_num, dev.mac_address_int));
    }
    // rsort
    std::sort(distance_entry_list.rbegin(), distance_entry_list.rend());
  } else {
    act::topology::ActBroadcastSearch broadcast(profiles_);
    act_status = broadcast.ComputeDeviceDistance(map_topology.GetDevices(), map_topology.GetLinks(),
                                                 map_topology.GetSourceDeviceId());
    if (IsActStatusSuccess(act_status)) {
      // Append ActDeviceDistanceEntry
      for (auto dev : map_topology.GetDevices()) {
        distance_entry_list.append(ActDeviceDistanceEntry(dev.GetId(), dev.GetDistance(), dev.mac_address_int));
      }
      // Sort result by distance from far to near
      std::sort(distance_entry_list.begin(), distance_entry_list.end());
    } else {
      // Sort by IP num (from small to large)
      quint32 ip_num;
      for (auto dev : map_topology.GetDevices()) {
        dev.GetIpv4().GetIpAddressNumber(ip_num);
        distance_entry_list.append(ActDeviceDistanceEntry(dev.GetId(), ip_num, dev.mac_address_int));
      }
      // rsort
      std::sort(distance_entry_list.rbegin(), distance_entry_list.rend());
    }
  }

  // Generate sorted Device list
  QList<ActDevice> new_dev_list;
  for (auto dist_entry : distance_entry_list) {
    auto index = map_topology.GetDevices().indexOf(ActDevice(dist_entry.GetId()));
    new_dev_list.append(map_topology.GetDevices().at(index));
  }

  map_topology.SetDevices(new_dev_list);

  return ACT_STATUS_SUCCESS;
}
