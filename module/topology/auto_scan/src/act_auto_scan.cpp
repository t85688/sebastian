
#include "act_auto_scan.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QDebug>
#include <QHostAddress>
#include <QNetworkInterface>
#include <sstream>

#include "act_auto_probe.hpp"
#include "act_link.hpp"
#include "act_utilities.hpp"

act::topology::ActAutoScan::~ActAutoScan() {
  if ((scan_topology_thread_ != nullptr) && (scan_topology_thread_->joinable())) {
    qDebug() << __func__ << __LINE__ << "Join ScanTopology thread.";
    scan_topology_thread_->join();
  }
}

ACT_STATUS act::topology::ActAutoScan::GetStatus() {
  if (IsActStatusSuccess(scan_topology_act_status_) && (progress_ == 100)) {
    scan_topology_act_status_->SetStatus(ActStatusType::kFinished);
  }

  if ((!IsActStatusRunning(scan_topology_act_status_)) && (!IsActStatusFinished(scan_topology_act_status_))) {
    // failed
    return scan_topology_act_status_;
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*scan_topology_act_status_), progress_);
}

ACT_STATUS act::topology::ActAutoScan::UpdateProgress(quint8 progress) {
  ACT_STATUS_INIT();
  progress_ = progress;
  qDebug() << __func__ << QString("Progress: %1%.").arg(GetProgress()).toStdString().c_str();
  return act_status;
}

ACT_STATUS act::topology::ActAutoScan::Stop() {
  // Checking has the thread is running
  if (IsActStatusRunning(scan_topology_act_status_)) {
    qDebug() << "Stop ScanTopology thread.";

    southbound_.SetStopFlag(true);

    // Send the stop signal to the ScanTopology and wait for the thread to finish.
    stop_flag_ = true;
    if ((scan_topology_thread_ != nullptr) && (scan_topology_thread_->joinable())) {
      qDebug() << "Join ScanTopology thread...";
      try {
        scan_topology_thread_->join();  // wait thread finished
        qDebug() << "ScanTopology thread joined successfully.";
      } catch (const std::exception &e) {
        qCritical() << "Exception caught while joining ScanTopology thread:" << e.what();
      } catch (...) {
        qCritical() << "Unknown exception caught while joining ScanTopology thread.";
      }
    }
  } else {
    qDebug() << __func__ << "The ScanTopology thread not running.";
  }

  qDebug() << "ScanTopology thread is finish.";
  return std::make_shared<ActProgressStatus>(ActStatusBase(*scan_topology_act_status_), progress_);
}

ACT_STATUS act::topology::ActAutoScan::Start(QList<ActScanIpRangeEntry> scan_ip_ranges, const bool &auto_probe_license,
                                             ActAutoScanResult &auto_scan_result) {
  // Checking has the thread is running
  if (IsActStatusRunning(scan_topology_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("ScanTopology");
  }

  // init ScanTopology status
  progress_ = 0;
  stop_flag_ = false;
  scan_topology_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to triggered the ScanTopology
  try {
    // check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((scan_topology_thread_ != nullptr) && (scan_topology_thread_->joinable())) {
      qDebug() << __func__ << __LINE__ << "Join ScanTopology thread.";
      scan_topology_thread_->join();
    }

    scan_topology_act_status_->SetStatus(ActStatusType::kRunning);
    scan_topology_thread_ =
        std::make_unique<std::thread>(&act::topology::ActAutoScan::TriggeredScanTopologyForThread, this, scan_ip_ranges,
                                      std::cref(auto_probe_license), std::ref(auto_scan_result));
  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(ScanTopology) failed. Error:" << e.what();
    scan_topology_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("ScanTopology");
  }

  qDebug() << "Start ScanTopology thread.";
  return std::make_shared<ActProgressStatus>(ActStatusBase(*scan_topology_act_status_), progress_);
}

void act::topology::ActAutoScan::TriggeredScanTopologyForThread(QList<ActScanIpRangeEntry> scan_ip_ranges,
                                                                const bool &auto_probe_license,
                                                                ActAutoScanResult &auto_scan_result) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the ScanTopology and wait for the return, and update scan_topology_act_status_.
  try {
    scan_topology_act_status_ = ScanTopology(scan_ip_ranges, auto_probe_license, auto_scan_result);
  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(interrupt) failed. Error:" << e.what();
    scan_topology_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kAutoScanFailed, ActSeverity::kCritical);
  }
}

ACT_STATUS act::topology::ActAutoScan::ScanTopology(QList<ActScanIpRangeEntry> &scan_ip_ranges,
                                                    const bool &auto_probe_license,
                                                    ActAutoScanResult &auto_scan_result) {
  ACT_STATUS_INIT();

  // qDebug() << __func__ << "ScanIpRanges size:" << scan_ip_ranges.size();

  qDebug() << __func__ << "Start ScanTopology.";

  qDebug() << "ScanDevicesByScanIpRange";
  // Scan devices
  act_status = southbound_.ScanDevicesByScanIpRange(scan_ip_ranges, alive_devices_);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanDevicesByScanIpRange() failed.";
    return act_status;
  }

  qDebug() << "FeatureAssignDevicesMoxaVendorByBroadcastSearch";
  // Assign devices Moxa vendor by BroadcastSearch
  act_status = southbound_.FeatureAssignDevicesMoxaVendorByBroadcastSearch(alive_devices_);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "FeatureAssignDevicesMoxaVendorByBroadcastSearch() failed.";
    return act_status;
  }

  qDebug() << "GetIpMacTable";
  // Get IP-MAC table(arp_table & adapter_table)
  act_status = southbound_.GetIpMacTable(ip_mac_table_);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetIpMacTable() failed.";
    return act_status;
  }
  UpdateProgress(30);

  qDebug() << "IdentifyDevices";
  // [feat:2396] Refactor - AutoScan performance enhance
  // Identify Devices (DeviceProfileId, FirmwareFeatureProfileId, FirmwareVersion, DeviceType, DeviceName)
  act_status = IdentifyDevices();
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "IdentifyDevices() failed.";
    return act_status;
  }

  // [feat:2604] Auto Scan - Get the unknown device need to execute the auto probe
  // [bugfix: 2657] AutoScan contact AutoProbe needs to add the license control
  // Probe Devices
  if (auto_probe_license) {
    act_status = ProbeDevices();
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "ProbeDevices() failed.";
      return act_status;
    }
  }

  UpdateProgress(50);

  qDebug() << "AssignDevicesConfiguration";
  // Assign Device Configuration (MACAddress, lldp_chassis_id) & (Interfaces's InterfaceName) & (Disable
  // SnmpEnableService)
  act_status = AssignDevicesConfiguration();
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "AssignDevicesConfiguration() failed.";
    return act_status;
  }

  UpdateProgress(70);

  qDebug() << "ScanLinks";
  // Scan Links
  act_status = ScanLinks(alive_devices_, alive_links_);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanLinks() failed.";
    return act_status;
  }

  UpdateProgress(90);

  // // Update unknown interfaces device(ICMP & Unknown & Moxa device)
  // // If link's interface not in the device's interface
  // act_status = UpdateUnknownInterfacesDeviceInterface();
  // if (!IsActStatusSuccess(act_status)) {
  //   qCritical() << __func__ << "UpdateUnknownInterfacesDeviceInterface() failed.";
  //   return act_status;
  // }

  auto_scan_result.SetDevices(alive_devices_);
  auto_scan_result.SetLinks(alive_links_);
  auto_scan_result.SetDeviceConfig(alive_device_config_);

  qDebug() << __func__ << "Result device size:" << alive_devices_.size();
  for (auto dev : alive_devices_) {
    qDebug() << __func__
             << QString("Result device: %1(%2), MAC: %3")
                    .arg(dev.GetIpv4().GetIpAddress())
                    .arg(dev.GetId())
                    .arg(dev.GetMacAddress())
                    .toStdString()
                    .c_str();
  }

  qDebug() << __func__ << "Result link size:" << alive_links_.size();
  for (auto link : alive_links_) {
    qDebug() << __func__
             << QString("Result link: %1(IF_ID:%2) to %3(IF_ID:%4)")
                    .arg(link.GetSourceDeviceId())
                    .arg(link.GetSourceInterfaceId())
                    .arg(link.GetDestinationDeviceId())
                    .arg(link.GetDestinationInterfaceId())
                    .toStdString()
                    .c_str();
  }

  UpdateProgress(100);

  return act_status;
}

ACT_STATUS act::topology::ActAutoScan::IdentifyDevices() {
  ACT_STATUS_INIT();
  QSet<ActDevice> new_device_set;

  for (auto device : alive_devices_) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    auto identify_status = southbound_.IdentifyDevice(device);
    if (!IsActStatusSuccess(identify_status)) {
      continue;
    }

    // Insert
    new_device_set.insert(device);
  }
  alive_devices_ = new_device_set;

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS act::topology::ActAutoScan::ProbeDevices() {
  ACT_STATUS_INIT();
  QSet<ActDevice> new_device_set;
  qint64 new_dev_profile_id = 10000;

  for (auto device : alive_devices_) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    ActAutoProbeWarning probe_warning_result;
    ActDeviceProfile probe_dev_profile;

    // Skip not AutoProbe device
    if (!device.auto_probe_) {
      new_device_set.insert(device);
      continue;
    }

    // Only probe Unknown Device & Moxa Device
    if (device.GetDeviceType() != ActDeviceTypeEnum::kUnknown && device.GetDeviceType() != ActDeviceTypeEnum::kMoxa) {
      new_device_set.insert(device);
      continue;
    }

    act::auto_probe::ActAutoProbe prober(profiles_);
    auto act_status_probe = prober.AutoProbe(device, probe_warning_result, probe_dev_profile);
    // Check Success
    if (!IsActStatusSuccess(act_status_probe)) {
      qDebug() << __func__
               << QString("AutoProbe Device(%1) failed. ProbeFeature Warning Report: %2")
                      .arg(device.GetIpv4().GetIpAddress())
                      .arg(probe_warning_result.ToString())
                      .toStdString()
                      .c_str();

      new_device_set.insert(device);
      continue;
    }

    // Check ModelName is empty (Uncertified-Device)
    if (probe_dev_profile.GetModelName().isEmpty()) {
      new_device_set.insert(device);
      continue;
    }

    if (profiles_.GetDeviceProfiles().contains(probe_dev_profile)) {  // duplicated
      // Check the probe_dev_profile duplicated with newly generated new_device_profiles
      if (new_device_profiles_.contains(probe_dev_profile)) {
        // Set Device DeviceProfileId
        device.SetDeviceProfileId(probe_dev_profile.GetId());
      }

      new_device_set.insert(device);
      continue;
    }

    // Assign temporarily DeviceProfile ID
    // Get Unique ID
    while (true) {
      ActDeviceProfile target_dev_profile;
      if (!IsActStatusSuccess(ActGetItemById<ActDeviceProfile>(profiles_.GetDeviceProfiles(), new_dev_profile_id,
                                                               target_dev_profile))) {
        break;
      }
      new_dev_profile_id += 1;
    }
    probe_dev_profile.SetId(new_dev_profile_id);
    profiles_.GetDeviceProfiles().insert(probe_dev_profile);
    new_device_profiles_.insert(probe_dev_profile);
    probe_device_profiles_queue_.enqueue(probe_dev_profile);

    // Set Device DeviceProfileId
    device.SetDeviceProfileId(probe_dev_profile.GetId());
    new_device_set.insert(device);
  }
  alive_devices_ = new_device_set;

  return act_status;
}

ACT_STATUS act::topology::ActAutoScan::AssignDeviceInformations(ActDevice &device) {
  // Device Name, Serial Number,  Modular Info
  ACT_STATUS_INIT();

  southbound_.AssignDeviceIPv4(device);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  southbound_.AssignDeviceName(device);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  southbound_.AssignDeviceSerialNumber(device);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  southbound_.AssignDeviceSystemUptime(device);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  southbound_.AssignDeviceProductRevision(device);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  southbound_.AssignDeviceRedundantProtocol(device);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  southbound_.AssignDeviceLocation(device);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  southbound_.AssignDeviceModularConfiguration(device);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  southbound_.AssignInterfacesAndBuiltinPowerByModular(device);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // // Fiber Check
  // QMap<qint64, ActMonitorFiberCheckEntry> port_fiber_map;
  // southbound_.ScanFiberCheck(device, port_fiber_map);
  // if (stop_flag_) {
  //   return ACT_STATUS_STOP;
  // }

  // // Port Info
  // QMap<qint64, ActDevicePortInfoEntry> port_info_map;
  // southbound_.ScanPortInfo(device, port_info_map);
  // if (stop_flag_) {
  //   return ACT_STATUS_STOP;
  // }

  // // Assign SFP to Module info
  // if (!port_fiber_map.isEmpty()) {
  //   southbound_.AssignSFPtoModule(device, port_fiber_map, port_info_map);
  // }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS act::topology::ActAutoScan::AssignDevicesConfiguration() {
  ACT_STATUS_INIT();
  QSet<ActDevice> new_device_set;

  for (auto device : alive_devices_) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    qDebug() << __func__ << "Device:" << device.GetIpv4().GetIpAddress();
    // Update Device Connect by AutoScan feature
    southbound_.UpdateDeviceConnectByScanFeature(device);
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    // Assign Device DeviceConfigs
    southbound_.AssignDeviceConfigs(device, alive_device_config_);
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    qDebug() << "Assign Device LLDP data";
    // Assign Device LLDP data
    southbound_.AssignDeviceLldpData(device);
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    qDebug() << "Assign Device MAC table";
    // Assign Device MAC table
    southbound_.AssignDeviceMacTable(device);
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    // Set MAC address & MAC int
    QString mac_addr = this->ip_mac_table_[device.GetIpv4().GetIpAddress()];  //  by ACT's ip_mac_table
    qint64 mac_int = 0;
    MacAddressToQInt64(mac_addr, mac_int);
    // If MAC is "00-00-00-00-00-00" would try to replace by chassis_id
    if (mac_int == 0) {
      // Try to replace the MAC by lldp loc_chassis_id
      QString new_mac_addr = "";
      auto transfer_status = TransferChassisIdToMacFormat(device.lldp_data_.GetLocChassisId(), new_mac_addr);
      if (IsActStatusSuccess(transfer_status)) {
        // Update mac_addr & mac_int
        mac_addr = new_mac_addr;
        MacAddressToQInt64(new_mac_addr, mac_int);
      }
    }
    device.SetMacAddress(mac_addr);
    device.mac_address_int = mac_int;

    qDebug() << "Assign Device Informations";
    // Assign Device Informations(Device Name, Serial Number,  Modular Info)
    AssignDeviceInformations(device);
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    qDebug() << "Assign Interfaces";
    // Assign Interfaces
    southbound_.AssignDeviceInterfacesInfo(device);
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    // Insert
    new_device_set.insert(device);
  }
  alive_devices_ = new_device_set;

  // qDebug() << "AutoScan device_config:" << alive_device_config_.ToString().toStdString().c_str();

  return act_status;
}

// ACT_STATUS act::topology::ActAutoScan::UpdateDeviceConnectByAutoScanFeature(ActDevice &device) {
//   ACT_STATUS_INIT();

//   ActFeatureProfile feature_profile;
//   act_status = GetDeviceFeature(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
//                                 ActFeatureEnum::kAutoScan, feature_profile);
//   if (IsActStatusSuccess(act_status)) {
//     // Find Feature used connect protocols
//     ActDeviceConnectStatusControl connect_status_control;
//     GetFeaturesUsedConnectProtocol(feature_profile, connect_status_control);
//     // qDebug() << __func__
//     //          << QString("Device(%1) AutoScan features used Connect: %2")
//     //                 .arg(device.GetIpv4().GetIpAddress())
//     //                 .arg(connect_status_control.ToString())
//     //                 .toStdString()
//     //                 .c_str();

//     // Update Device connect
//     act_status = southbound_.FeatureAssignDeviceStatus(true, device, connect_status_control);
//     if (!IsActStatusSuccess(act_status)) {
//       // qDebug() << __func__
//       //          << QString("Device(%1) AssignDeviceStatus failed.")
//       //                 .arg(device.GetIpv4().GetIpAddress())
//       //                 .toStdString()
//       //                 .c_str();
//     }
//   }

//   return ACT_STATUS_SUCCESS;
// }

ACT_STATUS act::topology::ActAutoScan::UpdateUnknownInterfacesDeviceInterface() {
  ACT_STATUS_INIT();

  // UnknownInterfacesDevice: ICMP, Unknown, Moxa
  QList<ActDeviceTypeEnum> know_if_dev_types;
  know_if_dev_types.append(ActDeviceTypeEnum::kICMP);
  know_if_dev_types.append(ActDeviceTypeEnum::kUnknown);
  know_if_dev_types.append(ActDeviceTypeEnum::kMoxa);

  for (auto link : alive_links_) {
    ActDevice src_device, dst_device;
    act_status = ActGetItemById<ActDevice>(alive_devices_, link.GetSourceDeviceId(), src_device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical()
          << __func__
          << QString("Alive Check: The device(%1) not found").arg(link.GetSourceDeviceId()).toStdString().c_str();

      return std::make_shared<ActStatusInternalError>("ScanTopology");
    }

    act_status = ActGetItemById<ActDevice>(alive_devices_, link.GetDestinationDeviceId(), dst_device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical()
          << __func__
          << QString("Alive Check: The device(%1) not found").arg(link.GetDestinationDeviceId()).toStdString().c_str();

      return std::make_shared<ActStatusInternalError>("ScanTopology");
    }

    // Handle interface of the src_device
    if (know_if_dev_types.contains(src_device.GetDeviceType())) {
      auto interface_list = src_device.GetInterfaces();
      qint64 src_if_id = link.GetSourceInterfaceId();

      ActInterface target_interface(src_if_id);
      auto interface_index = interface_list.indexOf(target_interface);
      if (interface_index == -1) {  // not contains
        target_interface.SetDeviceId(src_device.GetId());
        target_interface.SetInterfaceName(QString::number(src_if_id));
        target_interface.SetSupportSpeeds({link.GetSpeed()});
        interface_list.append(target_interface);
      } else {
        target_interface = interface_list.at(interface_index);
        target_interface.SetSupportSpeeds({link.GetSpeed()});
        interface_list[interface_index] = target_interface;
      }
      src_device.SetInterfaces(interface_list);
      alive_devices_.remove(src_device);
      alive_devices_.insert(src_device);
    }

    // Handle interface of the dst_device
    if (know_if_dev_types.contains(dst_device.GetDeviceType())) {
      auto interface_list = dst_device.GetInterfaces();
      qint64 dst_if_id = link.GetDestinationInterfaceId();

      ActInterface target_interface(dst_if_id);
      auto interface_index = interface_list.indexOf(target_interface);
      if (interface_index == -1) {  // not contains

        target_interface.SetDeviceId(dst_device.GetId());
        target_interface.SetInterfaceName(QString::number(dst_if_id));
        target_interface.SetSupportSpeeds({link.GetSpeed()});
        interface_list.append(target_interface);
      } else {
        target_interface = interface_list.at(interface_index);
        target_interface.SetSupportSpeeds({link.GetSpeed()});
        interface_list[interface_index] = target_interface;
      }
      dst_device.SetInterfaces(interface_list);
      alive_devices_.remove(dst_device);
      alive_devices_.insert(dst_device);
    }
  }

  return act_status;
}

ACT_STATUS act::topology::ActAutoScan::FindManagementEndpoint(
    const QMap<qint64, QMap<qint64, QString>> &device_remote_port_mac_map,
    ActManagementInterface &result_management_endpoint) {
  ACT_STATUS_INIT();

  // Get Host adapters mac
  QMap<QString, QString> adapter_table;
  act_status = southbound_.GetLocalhostAdapterTable(adapter_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetLocalhostAdapterTable() failed.";
    return act_status;
  }

  QList<QString> adapters_mac_list(adapter_table.values());
  QSet<QString> adapters_mac_set(adapters_mac_list.begin(), adapters_mac_list.end());

  foreach (auto dev_id, device_remote_port_mac_map.keys()) {
    for (auto port_id : device_remote_port_mac_map[dev_id].keys()) {
      auto rem_mac = device_remote_port_mac_map[dev_id][port_id];
      // Find adapter by remote_mac
      if (adapters_mac_set.find(rem_mac) != adapters_mac_set.end()) {  // found
        result_management_endpoint.SetDeviceId(dev_id);
        result_management_endpoint.SetInterfaces({port_id});
        return act_status;
      }
    }
  }

  return std::make_shared<ActStatusNotFound>("ManagementEndpoint");
}

ACT_STATUS act::topology::ActAutoScan::ScanLinks(QSet<ActDevice> &alive_devices, QSet<ActLink> &result_alive_links) {
  ACT_STATUS_INIT();
  result_alive_links.clear();

  for (auto device : alive_devices) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    ActScanLinksResult scan_link_result;
    auto create_status = southbound_.CreateDeviceLink(ip_mac_table_, device, alive_devices, scan_link_result);
    if (!IsActStatusSuccess(create_status)) {
      qDebug() << __func__ << "CreateDeviceLink() failed. Device:" << device.GetIpv4().GetIpAddress();
      continue;
    }

    // Success would update device
    for (auto update_device : scan_link_result.GetUpdateDevices()) {
      alive_devices.remove(update_device);
      alive_devices.insert(update_device);
    }

    // Re-assign the link ID & Insert to result_alive_links
    for (auto create_link : scan_link_result.GetScanLinks()) {
      create_link.SetId(result_alive_links.size() + 1);

      // Insert to alive_links
      result_alive_links.insert(create_link);
    }
  }
  return ACT_STATUS_SUCCESS;
}
