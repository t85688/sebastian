#include "act_broadcast_search.hpp"

#include "act_auto_scan.hpp"
#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QDebug>

#include "act_algorithm.hpp"
#include "act_core.hpp"

act::topology::ActBroadcastSearch::~ActBroadcastSearch() {
  if ((broadcast_search_thread_ != nullptr) && (broadcast_search_thread_->joinable())) {
    broadcast_search_thread_->join();
  }
}

ACT_STATUS act::topology::ActBroadcastSearch::GetStatus() {
  if (IsActStatusSuccess(broadcast_search_act_status_) && (progress_ == 100)) {
    broadcast_search_act_status_->SetStatus(ActStatusType::kFinished);
  }

  if ((!IsActStatusRunning(broadcast_search_act_status_)) && (!IsActStatusFinished(broadcast_search_act_status_))) {
    // failed
    return broadcast_search_act_status_;
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*broadcast_search_act_status_), progress_);
}

ACT_STATUS act::topology::ActBroadcastSearch::Stop() {
  // Checking has the thread is running
  if (IsActStatusRunning(broadcast_search_act_status_)) {
    qDebug() << "Stop BroadcastSearch's running thread.";

    southbound_.SetStopFlag(true);

    // Send the stop signal and wait for the thread to finish.
    stop_flag_ = true;
    if ((broadcast_search_thread_ != nullptr) && (broadcast_search_thread_->joinable())) {
      broadcast_search_thread_->join();  // wait thread finished
    }
  } else {
    qDebug() << __func__ << "The BroadcastSearch's thread not running.";
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*broadcast_search_act_status_), progress_);
}

ACT_STATUS act::topology::ActBroadcastSearch::StartDeviceDiscovery(ActDeviceDiscoveryConfig dev_discovery_cfg,
                                                                   QList<ActDevice> &result_devices) {
  // Checking has the thread is running
  if (IsActStatusRunning(broadcast_search_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("BroadcastSearch");
  }

  // init ActBroadcastSearch status
  progress_ = 0;
  stop_flag_ = false;
  broadcast_search_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to start DeviceDiscoveryBroadcastSearch
  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((broadcast_search_thread_ != nullptr) && (broadcast_search_thread_->joinable())) {
      broadcast_search_thread_->join();
    }
    broadcast_search_act_status_->SetStatus(ActStatusType::kRunning);
    broadcast_search_thread_ = std::make_unique<std::thread>(
        &act::topology::ActBroadcastSearch::TriggerDeviceDiscoveryBroadcastSearchForThread, this, dev_discovery_cfg,
        std::ref(result_devices));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggerDeviceDiscoveryBroadcastSearchForThread";
    HRESULT hr = SetThreadDescription(broadcast_search_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    qDebug() << "Start DeviceDiscoveryBroadcastSearch thread.";

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(DeviceDiscoveryBroadcastSearch) failed. Error:" << e.what();
    broadcast_search_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("BroadcastSearch");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*broadcast_search_act_status_), progress_);
}

void act::topology::ActBroadcastSearch::TriggerDeviceDiscoveryBroadcastSearchForThread(
    ActDeviceDiscoveryConfig dev_discovery_cfg, QList<ActDevice> &result_devices) {
  // Triggered the DeviceDiscoveryBroadcastSearch and wait for the return, and update broadcast_search_act_status_.
  try {
    broadcast_search_act_status_ = DeviceDiscoveryBroadcastSearch(dev_discovery_cfg, result_devices);

  } catch (std::exception &e) {
    // no effect on the program's behavior
    static_cast<void>(e);
    broadcast_search_act_status_ = std::make_shared<ActStatusInternalError>("BroadcastSearch");
  }
}

ACT_STATUS act::topology::ActBroadcastSearch::StartRetryConnect(ActRetryConnectConfig retry_connect_cfg,
                                                                QList<ActDevice> &result_devices) {
  // Checking has the thread is running
  if (IsActStatusRunning(broadcast_search_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("BroadcastSearch");
  }

  // init ActBroadcastSearch status
  progress_ = 0;
  stop_flag_ = false;
  broadcast_search_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to start RetryConnect
  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((broadcast_search_thread_ != nullptr) && (broadcast_search_thread_->joinable())) {
      broadcast_search_thread_->join();
    }

    broadcast_search_act_status_->SetStatus(ActStatusType::kRunning);
    broadcast_search_thread_ =
        std::make_unique<std::thread>(&act::topology::ActBroadcastSearch::TriggerRetryConnectForThread, this,
                                      retry_connect_cfg, std::ref(result_devices));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggerRetryConnectForThread";
    HRESULT hr = SetThreadDescription(broadcast_search_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    qDebug() << "Start RetryConnect thread.";

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(RetryConnect) failed. Error:" << e.what();
    broadcast_search_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("BroadcastSearch");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*broadcast_search_act_status_), progress_);
}

void act::topology::ActBroadcastSearch::TriggerRetryConnectForThread(ActRetryConnectConfig retry_connect_cfg,
                                                                     QList<ActDevice> &result_devices) {
  // Triggered the RetryConnect and wait for the return, and update broadcast_search_act_status_.
  try {
    broadcast_search_act_status_ = RetryConnect(retry_connect_cfg, result_devices);
  } catch (std::exception &e) {
    // no effect on the program's behavior
    static_cast<void>(e);
    broadcast_search_act_status_ = std::make_shared<ActStatusInternalError>("BroadcastSearch");
  }
}

ACT_STATUS act::topology::ActBroadcastSearch::StartLinkDistanceDetect(
    bool from_broadcast_search, QList<qint64> select_dev_id_list,
    QList<ActDeviceDistanceEntry> &result_distance_entry_list) {
  // Checking has the thread is running
  if (IsActStatusRunning(broadcast_search_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("BroadcastSearch");
  }

  // init ActBroadcastSearch status
  progress_ = 0;
  stop_flag_ = false;
  broadcast_search_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to start LinkDistanceDetect
  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((broadcast_search_thread_ != nullptr) && (broadcast_search_thread_->joinable())) {
      broadcast_search_thread_->join();
    }
    broadcast_search_act_status_->SetStatus(ActStatusType::kRunning);
    broadcast_search_thread_ =
        std::make_unique<std::thread>(&act::topology::ActBroadcastSearch::TriggerLinkDistanceDetectForThread, this,
                                      from_broadcast_search, select_dev_id_list, std::ref(result_distance_entry_list));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggerLinkDistanceDetectForThread";
    HRESULT hr = SetThreadDescription(broadcast_search_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(LinkDistanceDetect) failed. Error:" << e.what();
    broadcast_search_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("BroadcastSearch");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*broadcast_search_act_status_), progress_);
}

void act::topology::ActBroadcastSearch::TriggerLinkDistanceDetectForThread(
    bool from_broadcast_search, QList<qint64> select_dev_id_list,
    QList<ActDeviceDistanceEntry> &result_distance_entry_list) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the LinkDistanceDetect and wait for the return, and update broadcast_search_act_status_.
  try {
    broadcast_search_act_status_ =
        LinkDistanceDetect(from_broadcast_search, select_dev_id_list, result_distance_entry_list);
  } catch (std::exception &e) {
    // no effect on the program's behavior
    static_cast<void>(e);
    broadcast_search_act_status_ = std::make_shared<ActStatusInternalError>("BroadcastSearch");
  }
}

ACT_STATUS act::topology::ActBroadcastSearch::UpdateProgress(quint8 progress) {
  ACT_STATUS_INIT();
  progress_ = progress;
  qDebug() << __func__ << QString("Progress: %1%.").arg(GetProgress()).toStdString().c_str();
  return act_status;
}

ACT_STATUS act::topology::ActBroadcastSearch::CreateDevicesPathLinks(const bool &config_arp_table) {
  ACT_STATUS_INIT();
  QSet<ActLink> link_set;

  // Get Sub-Item(LLDP > Basic)
  ActFeatureSubItem lldp_sub_item;
  act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kAutoScan, "LLDP",
                                               "Basic", lldp_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetFeatureProfileFeatureSubItem() failed";
    return act_status;
  }
  // Get Sub-Item(DeviceInformation > MACTable)
  ActFeatureSubItem mac_table_sub_item;
  act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kAutoScan,
                                               "DeviceInformation", "MACTable", mac_table_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetFeatureProfileFeatureSubItem() failed";
    return act_status;
  }

  // Assign all devices LLDP Data & MAC table
  for (auto &dev : devices_) {
    if (stop_flag_) {  // stop flag
      return ACT_STATUS_STOP;
    }

    // Set device's mac to arp table
    if (config_arp_table) {
      act_status = southbound_.SetLocalhostArpTable(dev, mac_host_map_[dev.GetMacAddress()]);
      if (!IsActStatusSuccess(act_status)) {
        return BroadcastInternalErrorHandler(__func__, "SetLocalhostArpTable() Failed", dev);
      }
    }

    // Get LLDP data
    ActLLDPData lldp_data;
    auto get_lldp_status = southbound_.ActionGetLldpData(dev, lldp_sub_item, lldp_data);
    if (IsActStatusSuccess(get_lldp_status)) {
      dev.lldp_data_ = lldp_data;
    }

    // Get MAC table
    QMap<qint64, QString> mac_table;  // <PortID, MAC> Port_MAC_map
    auto get_mac_table_status = southbound_.ActionGetSingleEntryMacTable(dev, mac_table_sub_item, mac_table);
    if (IsActStatusSuccess(get_mac_table_status)) {
      dev.single_entry_mac_table_ = mac_table;
    }

    if (config_arp_table) {
      // Delete device's mac to arp table
      act_status = southbound_.DeleteLocalhostArpEntry(dev);
      if (!IsActStatusSuccess(act_status)) {
        return BroadcastInternalErrorHandler(__func__, "DeleteLocalhostArpEntry() Failed", dev);
      }
    }
  }

  for (auto dev : devices_) {
    if (stop_flag_) {  // stop flag
      return ACT_STATUS_STOP;
    }

    // Get Device Links by LLDP
    ActScanLinksResult south_result;
    act_status = southbound_.GenerateLinkSetBylldpInfo(dev, devices_, south_result);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenerateLinkSetBylldpInfo() Failed.";
      continue;
    }

    // Insert link
    for (auto link : south_result.GetScanLinks()) {
      ActLink new_link(link);
      new_link.SetId(link_set.size() + 1);
      links_.append(new_link);

      // qDebug() << __func__
      //          << QString("Device(ID:%1, IP:%2, MAC:%3) scan link(src(%4)-port(%5) <-> dst(%6)-port(%7))")
      //                 .arg(dev.GetId())
      //                 .arg(dev.GetIpv4().GetIpAddress())
      //                 .arg(dev.GetMacAddress())
      //                 .arg(new_link.GetSourceDeviceId())
      //                 .arg(new_link.GetSourceInterfaceId())
      //                 .arg(new_link.GetDestinationDeviceId())
      //                 .arg(new_link.GetDestinationInterfaceId())
      //                 .toStdString()
      //                 .c_str();
    }

    UpdateProgress(progress_ + (50 / devices_.size()));
  }

  // qDebug() << __func__ << "Topology Scan Links:";
  // for (auto link : links_) {
  //   qDebug() << __func__
  //            << QString("scan link(src(%4)-port(%5) <-> dst(%6)-port(%7))")
  //                   .arg(link.GetSourceDeviceId())
  //                   .arg(link.GetSourceInterfaceId())
  //                   .arg(link.GetDestinationDeviceId())
  //                   .arg(link.GetDestinationInterfaceId())
  //                   .toStdString()
  //                   .c_str();
  // }

  return act_status;
}

ACT_STATUS act::topology::ActBroadcastSearch::LinkDistanceDetectFilterDevices() {
  ACT_STATUS_INIT();

  QList<ActDevice> new_devices;
  for (auto dev : devices_) {
    // Check device can be deploy
    if (!ActDevice::CheckDeviceCanBeDeploy(dev)) {
      // qDebug() << __func__
      //          << QString("Skip the %1 device(%2(%3))")
      //                 .arg(kActDeviceTypeEnumMap.key(dev.GetDeviceType()))
      //                 .arg(dev.GetIpv4().GetIpAddress())
      //                 .arg(dev.GetId())
      //                 .toStdString()
      //                 .c_str();
      continue;
    }
    new_devices.append(dev);
  }
  devices_ = new_devices;

  return act_status;
}

ACT_STATUS act::topology::ActBroadcastSearch::LinkDistanceDetectSplitNotAliveDevices(
    QList<ActDevice> &not_alive_devices) {
  ACT_STATUS_INIT();

  not_alive_devices.clear();
  QList<ActDevice> new_devices;

  southbound_.UpdateDevicesIcmpStatus(devices_);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  for (auto dev : devices_) {
    if (dev.GetDeviceStatus().GetICMPStatus()) {
      new_devices.append(dev);  // alive
    } else {
      not_alive_devices.append(dev);  // not alive
    }
  }
  devices_ = new_devices;

  return act_status;
}

ACT_STATUS act::topology::ActBroadcastSearch::UpdateDevicesSnmpStatusAndEnable() {
  ACT_STATUS_INIT();

  QList<ActDevice> new_devices;

  for (auto dev : devices_) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    // Start update SNMP connect status by southbound
    ActDeviceConnectStatusControl control(
        false, true, false, false);  // (RESTful(false), SNMP(true), NETCONF(false), NewMOXACommand(false))
    act_status = southbound_.FeatureAssignDeviceStatus(true, dev, control);
    if (!IsActStatusSuccess(act_status)) {
      DeviceErrorLogHandler(__func__, "FeatureAssignDeviceStatus() Failed", dev);
      new_devices.append(dev);
      continue;
    }

    new_devices.append(dev);
  }
  devices_ = new_devices;
  return act_status;
}

ACT_STATUS act::topology::ActBroadcastSearch::AssignDevicesMacAddressByArpTable() {
  ACT_STATUS_INIT();

  QMap<QString, QString> ip_mac_table;
  QList<ActDevice> new_dev_list;

  // Get IP-MAC table(arp_table & adapter_table)
  act_status = southbound_.GetIpMacTable(ip_mac_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetIpMacTable() failed.";
    return act_status;
  }
  for (auto dev : devices_) {
    QString mac_addr = ip_mac_table[dev.GetIpv4().GetIpAddress()];
    dev.SetMacAddress(mac_addr);

    // Set MAC int
    qint64 mac_int = 0;
    MacAddressToQInt64(dev.GetMacAddress(), mac_int);
    dev.mac_address_int = mac_int;

    new_dev_list.append(dev);
  }
  devices_ = new_dev_list;

  return act_status;
}

ACT_STATUS act::topology::ActBroadcastSearch::ComputeDeviceDistance(QList<ActDevice> &device_list,
                                                                    const QList<ActLink> &link_list,
                                                                    const qint64 &src_device_id) {
  ACT_STATUS_INIT();

  // Compute device tier
  QSet<qint64> visited;
  QMap<quint32, QQueue<qint64>> BFS_queue;
  quint32 distance = 0;
  int idx = device_list.indexOf(ActDevice(src_device_id));
  if (idx == -1) {
    return std::make_shared<ActStatusNotFound>(QString("source device id %1 in device list").arg(src_device_id));
  }
  ActDevice &source_device = device_list[idx];
  source_device.SetDistance(distance);

  BFS_queue[distance].enqueue(src_device_id);
  visited.insert(src_device_id);

  while (!BFS_queue[distance].isEmpty()) {
    qint64 device_id = BFS_queue[distance].dequeue();

    for (const ActLink &link : link_list) {
      qint64 neighbor_id = (link.GetSourceDeviceId() == device_id)        ? link.GetDestinationDeviceId()
                           : (link.GetDestinationDeviceId() == device_id) ? link.GetSourceDeviceId()
                                                                          : -1;
      if (neighbor_id == -1 || visited.contains(neighbor_id)) {
        continue;
      }

      idx = device_list.indexOf(ActDevice(neighbor_id));
      if (idx == -1) {
        continue;
      }

      ActDevice &neighbor = device_list[idx];
      neighbor.SetDistance(distance + 1);

      BFS_queue[distance + 1].enqueue(neighbor_id);
      visited.insert(neighbor_id);
    }

    if (BFS_queue[distance].isEmpty()) {
      distance++;
    }
  }

  for (ActDevice &device : device_list) {
    if (visited.contains(device.GetId())) {
      continue;
    }
    device.SetDistance(-1);
  }

  return act_status;
}

ACT_STATUS act::topology::ActBroadcastSearch::LinkDistanceDetect(
    bool from_broadcast_search, QList<qint64> select_dev_id_list,
    QList<ActDeviceDistanceEntry> &result_distance_entry_list) {
  ACT_STATUS_INIT();
  result_distance_entry_list.clear();
  QList<ActDevice> not_alive_devices;

  // If not from broadcast_search, would filter the device by device_type
  if (!from_broadcast_search) {
    LinkDistanceDetectFilterDevices();
  }

  // qDebug() << __func__ << "Devices size:" << devices_.size();
  // for (auto dev : devices_) {
  //   qDebug() << __func__
  //            << QString("Device: ID:%1, IP:%2, MAC:%3")
  //                   .arg(dev.GetId())
  //                   .arg(dev.GetIpv4().GetIpAddress())
  //                   .arg(dev.GetMacAddress())
  //                   .toStdString()
  //                   .c_str();
  // }

  // If not from broadcast_search need to enable SNMP
  if (!from_broadcast_search) {
    act_status = LinkDistanceDetectSplitNotAliveDevices(not_alive_devices);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "LinkDistanceDetectSplitNotAliveDevices() failed.";
      return act_status;
    }
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    auto enable_act_status = UpdateDevicesSnmpStatusAndEnable();
    if (!IsActStatusSuccess(enable_act_status)) {
      qCritical() << __func__ << "UpdateDevicesSnmpStatusAndEnable() failed.";
    }
  }
  UpdateProgress(20);

  // If not from broadcast_search need to get device MAC
  if (!from_broadcast_search) {
    act_status = AssignDevicesMacAddressByArpTable();
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "AssignDevicesMacAddressByArpTable() failed.";
      return act_status;
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }
  UpdateProgress(40);

  // Create Devices path links (Two switches only create a link, and don't distinguish the interface.)
  act_status = CreateDevicesPathLinks(from_broadcast_search);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "CreateDevicesPathLinks() failed.";
    return act_status;
  }
  UpdateProgress(70);

  // Find the source device by device's LLDP data & MacTable
  ActSourceDevice source_dev;
  auto find_source_status = southbound_.FindSourceDeviceByLLDPAndMacTable(devices_, source_dev);
  qDebug() << __func__ << QString("Source Device's ID: %1").arg(source_dev.GetDeviceId()).toStdString().c_str();
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }
  UpdateProgress(90);

  // Sort
  if (IsActStatusSuccess(find_source_status)) {
    // Sort by distance
    auto compute_status = ComputeDeviceDistance(devices_, links_, source_dev.GetDeviceId());
    if (!IsActStatusSuccess(compute_status)) {
      qCritical() << __func__ << "ComputeDeviceDistance() failed.";
    }

    // Append ActDeviceDistanceEntry to result
    for (auto dev : devices_) {
      // Filter by select
      if (select_dev_id_list.isEmpty()) {  // input all
        result_distance_entry_list.append(ActDeviceDistanceEntry(dev.GetId(), dev.GetDistance(), dev.mac_address_int));
      } else {
        if (select_dev_id_list.contains(dev.GetId())) {
          result_distance_entry_list.append(
              ActDeviceDistanceEntry(dev.GetId(), dev.GetDistance(), dev.mac_address_int));
        } else {
          continue;
        }
      }
      // qDebug() << __func__
      //          << QString("Device:%1(MAC:%2, MAC_int:%3), Distance:%4")
      //                 .arg(dev.GetId())
      //                 .arg(dev.GetMacAddress())
      //                 .arg(dev.mac_address_int)
      //                 .arg(dev.GetDistance())
      //                 .toStdString()
      //                 .c_str();
    }

    // Sort result by distance from far to near
    std::sort(result_distance_entry_list.begin(), result_distance_entry_list.end());
    for (auto entry : result_distance_entry_list) {
      qInfo() << __func__
              << QString("Device:%1(MAC_int:%2), Distance:%3")
                     .arg(entry.GetId())
                     .arg(entry.GetMacAddressInt())
                     .arg(entry.GetDistance())
                     .toStdString()
                     .c_str();
    }

  } else {
    // Sort by IP num (from small to large)
    quint32 ip_num;
    for (auto dev : devices_) {
      if (select_dev_id_list.isEmpty()) {  // input all
        dev.GetIpv4().GetIpAddressNumber(ip_num);
        result_distance_entry_list.append(ActDeviceDistanceEntry(dev.GetId(), ip_num, dev.mac_address_int));
      } else {
        if (select_dev_id_list.contains(dev.GetId())) {
          dev.GetIpv4().GetIpAddressNumber(ip_num);
          result_distance_entry_list.append(ActDeviceDistanceEntry(dev.GetId(), ip_num, dev.mac_address_int));
        }
      }
    }
    // rsort
    std::sort(result_distance_entry_list.rbegin(), result_distance_entry_list.rend());
    // Remove tmp distance
    for (auto &distance_entry : result_distance_entry_list) {
      distance_entry.SetDistance(0);
    }
  }

  // Re Append not_alive_devices to entry_list last
  for (auto dev : not_alive_devices) {
    // Filter by select
    if (select_dev_id_list.isEmpty()) {  // input all
      result_distance_entry_list.append(ActDeviceDistanceEntry(dev.GetId(), 0, dev.mac_address_int));
    } else {
      if (select_dev_id_list.contains(dev.GetId())) {
        result_distance_entry_list.append(ActDeviceDistanceEntry(dev.GetId(), 0, dev.mac_address_int));
      } else {
        continue;
      }
    }
    // qDebug() << __func__
    //          << QString("Device: %1, Not alive would append to last)").arg(dev.GetId()).toStdString().c_str();
  }

  UpdateProgress(100);

  return act_status;
}

ACT_STATUS act::topology::ActBroadcastSearch::BroadcastInternalErrorHandler(const QString &error_fun,
                                                                            const QString &error_reason,
                                                                            const ActDevice &device) {
  qCritical() << __func__
              << QString("%1 > %2. Device: %3(%4)")
                     .arg(error_fun)
                     .arg(error_reason)
                     .arg(device.GetIpv4().GetIpAddress())
                     .arg(device.GetId())
                     .toStdString()
                     .c_str();

  return std::make_shared<ActStatusInternalError>("BroadcastSearch");
}

void act::topology::ActBroadcastSearch::DeviceErrorLogHandler(const QString &error_fun, const QString &error_reason,
                                                              const ActDevice &device) {
  qCritical() << error_fun.toStdString().c_str()
              << QString("%1. Device: %2(%3)")
                     .arg(error_reason)
                     .arg(device.GetIpv4().GetIpAddress())
                     .arg(device.GetId())
                     .toStdString()
                     .c_str();
}

ACT_STATUS act::topology::ActBroadcastSearch::DeviceDiscoveryBroadcastSearch(
    const ActDeviceDiscoveryConfig &dev_discovery_cfg, QList<ActDevice> &result_devices) {
  ACT_STATUS_INIT();

  ActDeviceDiscoveryConfig discovery_cfg(dev_discovery_cfg);

  if (discovery_cfg.GetDefineDeviceType().GetMoxaIndustrialEthernetProduct() &&
      discovery_cfg.GetDefineNetworkInterface().GetDefineNetworkInterfaces().isEmpty()) {
    // Search devices
    QList<ActDevice> south_dev_list;
    act_status = SearchDevicesHandle(south_dev_list);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "BroadcastSearchDevices() Failed.";
      return act_status;
    }
    UpdateProgress(30);

    // Update g_core -> mac_host_map
    act::core::g_core.SetMacHostMap(mac_host_map_);

    // Update device
    for (auto dev : south_dev_list) {
      if (stop_flag_) {  // stop flag
        return ACT_STATUS_STOP;
      }

      // Set device's connect configuration
      act::core::g_core.CopyConnectionConfigField(dev, discovery_cfg);
      dev.SetEnableSnmpSetting(discovery_cfg.GetEnableSnmpSetting());

      // Assign the Vendor to Device -> DeviceProperty -> Vendor
      // [feat:2396] Refactor - AutoScan performance enhance
      auto dev_property = dev.GetDeviceProperty();
      dev_property.SetVendor(ACT_VENDOR_MOXA);
      dev.SetDeviceProperty(dev_property);

      // Update device
      UpdateDevice(dev);

      // Append device to result_devices & private devices_

      mutex_.lock();

      devices_.append(dev);
      result_devices.append(dev);

      mutex_.unlock();

      UpdateProgress(progress_ + (60 / south_dev_list.size()));  // update progress(30~90/100)
    }
    UpdateProgress(95);

  } else {
    qCritical() << __func__
                << "Currently DeviceDiscovery only supports the Moxa Industrial Ethernet & All NetworkInterfaces.";
    return std::make_shared<ActStatusInternalError>("BroadcastSearch");
  }

  UpdateProgress(100);
  return act_status;
}

ACT_STATUS act::topology::ActBroadcastSearch::SearchDevicesHandle(QList<ActDevice> &devices) {
  ACT_STATUS_INIT();
  devices.clear();

  ActFeatureSubItem search_sub_item;
  act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kAutoScan,
                                               "BroadcastSearch", "Basic", search_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Start SearchDevices
  QList<ActDevice> search_devices;
  QMap<QString, QString> mac_host_map;
  return southbound_.ActionBroadcastSearchDevices(search_sub_item, devices, mac_host_map_);
}

ACT_STATUS act::topology::ActBroadcastSearch::UpdateDevice(ActDevice &device) {
  ACT_STATUS_INIT();

  // act_status = southbound_.ClearArpCache();
  // if (!IsActStatusSuccess(act_status)) {
  //   return BroadcastInternalErrorHandler(__func__, "ClearArpCache() Failed", device);
  // }

  // Set device's mac to arp table
  act_status = southbound_.SetLocalhostArpTable(device, mac_host_map_[device.GetMacAddress()]);
  if (!IsActStatusSuccess(act_status)) {
    return BroadcastInternalErrorHandler(__func__, "SetLocalhostArpTable() Failed", device);
  }

  // Set connect status as false
  device.GetDeviceStatus().SetAllConnectStatus(false);

  // Update SNMP connect status by southbound
  ActDeviceConnectStatusControl control(false, true, false,
                                        false);  // (RESTful(false), SNMP(true),NETCONF(false),NewMOXACommand(false))
  southbound_.FeatureAssignDeviceStatus(true, device, control);

  // Update another connect status(skip assign the NETCONF)
  ActDeviceConnectStatusControl control2(true, false, false,
                                         true);  // (RESTful(true), SNMP(false), NETCONF(true), NewMOXACommand(true))
  southbound_.FeatureAssignDeviceStatus(true, device, control2);

  // Assign the NETCONF status as true
  device.GetDeviceStatus().SetNETCONFStatus(true);

  // Update DevicePartial Info
  UpdateDevicePartialInfos(device);

  // Delete device's mac to arp table
  act_status = southbound_.DeleteLocalhostArpEntry(device);
  if (!IsActStatusSuccess(act_status)) {
    return BroadcastInternalErrorHandler(__func__, "DeleteLocalhostArpEntry() Failed", device);
  }

  return act_status;
}

ACT_STATUS act::topology::ActBroadcastSearch::UpdateDevicePartialInfos(ActDevice &device) {
  ACT_STATUS_INIT();

  // Serial Number
  ActFeatureSubItem feature_sub_item;
  act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kAutoScan,
                                               "DeviceInformation", "SerialNumber", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }
  QString serial_number;
  act_status = southbound_.ActionGetSerialNumber(device, feature_sub_item, serial_number);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  device.GetDeviceInfo().SetSerialNumber(serial_number);

  // Ipv4
  GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kAutoScan, "DeviceInformation",
                                  "IPConfiguration", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }
  ActIpv4 ipv4;
  act_status = southbound_.ActionGetIPConfiguration(device, feature_sub_item, ipv4);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  device.GetIpv4().SetGateway(ipv4.GetGateway());
  device.GetIpv4().SetSubnetMask(ipv4.GetSubnetMask());
  device.GetIpv4().SetDNS1(ipv4.GetDNS1());
  device.GetIpv4().SetDNS2(ipv4.GetDNS2());

  return act_status;
}

ACT_STATUS act::topology::ActBroadcastSearch::RetryConnect(const ActRetryConnectConfig &retry_connect_cfg,
                                                           QList<ActDevice> &result_devices) {
  ACT_STATUS_INIT();
  ActRetryConnectConfig connect_cfg(retry_connect_cfg);

  UpdateProgress(10);

  auto devices_id = connect_cfg.GetId();
  for (auto dev_id : devices_id) {
    if (stop_flag_) {  // stop flag
      return ACT_STATUS_STOP;
    }

    // Find device from private devices_
    qint32 dev_index = -1;
    act_status = ActGetListItemIndexById<ActDevice>(devices_, dev_id, dev_index);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Not found target device in the private devices_. Device:" << dev_id;
      return std::make_shared<ActStatusInternalError>("BroadcastSearch");
    }
    ActDevice dev = devices_.at(dev_index);

    // Set device's connect configuration
    act::core::g_core.CopyConnectionConfigField(dev, connect_cfg);
    dev.SetEnableSnmpSetting(connect_cfg.GetEnableSnmpSetting());

    // Update device
    UpdateDevice(dev);

    // Append & Replace device

    mutex_.lock();

    devices_.replace(dev_index, dev);
    result_devices.append(dev);

    mutex_.unlock();

    UpdateProgress(progress_ + (80 / devices_id.size()));  // update progress(10~90/100)
  }

  // QMap<QString, QString> mac_host_map;
  // act::core::g_core.GetMacHostMap(mac_host_map);
  // foreach (auto mac, mac_host_map.keys()) {  // mac: 38-F3-AB-E2-69-67
  //   qDebug() << __func__ << QString("mac: %1, host:%2").arg(mac).arg(mac_host_map[mac]).toStdString().c_str();
  // }

  UpdateProgress(100);
  return act_status;
}