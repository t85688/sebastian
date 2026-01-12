#include "act_compare.hpp"

#include "act_auto_scan.hpp"
#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QDebug>
act::topology::ActCompare::~ActCompare() {
  if ((compare_topology_thread_ != nullptr) && (compare_topology_thread_->joinable())) {
    compare_topology_thread_->join();
  }
}

void act::topology::ActCompare::SetSouthboundProfiles(const ActProfiles &profiles) {
  southbound_.SetProfiles(profiles);
}

ACT_STATUS act::topology::ActCompare::GetStatus() {
  if (IsActStatusSuccess(compare_topology_act_status_) && (progress_ == 100)) {
    compare_topology_act_status_->SetStatus(ActStatusType::kFinished);
  }

  if ((!IsActStatusRunning(compare_topology_act_status_)) && (!IsActStatusFinished(compare_topology_act_status_))) {
    // failed
    return compare_topology_act_status_;
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*compare_topology_act_status_), progress_);
}

ACT_STATUS act::topology::ActCompare::UpdateProgress(quint8 progress) {
  ACT_STATUS_INIT();
  progress_ = progress;
  qDebug() << __func__ << QString("Progress: %1%.").arg(GetProgress()).toStdString().c_str();
  return act_status;
}

ACT_STATUS act::topology::ActCompare::Stop() {
  // qDebug() << __func__ << compare_topology_act_status_->ToString().toStdString().c_str();
  // Checking has the thread is running
  if (IsActStatusRunning(compare_topology_act_status_)) {
    qDebug() << "Stop CompareTopology thread.";

    southbound_.SetStopFlag(true);

    // Send the stop signal to the CompareTopology and wait for the thread to finish.
    stop_flag_ = true;

    if ((compare_topology_thread_ != nullptr) && (compare_topology_thread_->joinable())) {
      compare_topology_thread_->join();  // wait thread finished
    }
  } else {
    qDebug() << __func__ << "The CompareTopology thread not running.";
  }
  return std::make_shared<ActProgressStatus>(ActStatusBase(*compare_topology_act_status_), progress_);
}

void act::topology::ActCompare::DeviceErrorLogHandler(QString called_func, const QString &error_reason,
                                                      const ActDevice &device) {
  qCritical() << called_func.toStdString().c_str()
              << QString("%1. Device: %2(%3)")
                     .arg(error_reason)
                     .arg(device.GetIpv4().GetIpAddress())
                     .arg(device.GetId())
                     .toStdString()
                     .c_str();
}

ACT_STATUS act::topology::ActCompare::Start(const ActProject &project, const QList<qint64> &dev_id_list,
                                            const ActCompareControl &compare_control) {
  // Checking has the thread is running
  if (IsActStatusRunning(compare_topology_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("Compare");
  }

  // init CompareTopology status
  progress_ = 0;
  stop_flag_ = false;
  compare_topology_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to triggered the CompareTopology
  try {
    // check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((compare_topology_thread_ != nullptr) && (compare_topology_thread_->joinable())) {
      compare_topology_thread_->join();
    }
    compare_topology_act_status_->SetStatus(ActStatusType::kRunning);
    compare_topology_thread_ =
        std::make_unique<std::thread>(&act::topology::ActCompare::TriggeredCompareTopologyForThread, this,
                                      std::cref(project), std::cref(dev_id_list), std::cref(compare_control));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggeredCompareTopologyForThread";
    HRESULT hr = SetThreadDescription(compare_topology_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(CompareTopology) failed. Error:" << e.what();
    compare_topology_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("Compare");
  }

  qDebug() << "Start CompareTopology thread.";
  return std::make_shared<ActProgressStatus>(ActStatusBase(*compare_topology_act_status_), progress_);
}

void act::topology::ActCompare::TriggeredCompareTopologyForThread(const ActProject &project,
                                                                  const QList<qint64> &dev_id_list,
                                                                  const ActCompareControl &compare_control) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the CompareTopology and wait for the return, and update compare_topology_act_status_.
  try {
    compare_topology_act_status_ = CompareTopology(project, dev_id_list, compare_control);
  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(interrupt) failed. Error:" << e.what();
    compare_topology_act_status_ = std::make_shared<ActStatusInternalError>("Compare");
  }
}

ACT_STATUS act::topology::ActCompare::CompareTopology(const ActProject &project, const QList<qint64> &dev_id_list,
                                                      const ActCompareControl &compare_control) {
  ACT_STATUS_INIT();
  project_ = project;

  qDebug() << __func__ << QString("Compare control: %1").arg(compare_control.ToString()).toStdString().c_str();

  // Generate compare devices & links
  act_status = GenerateCompareDevicesAndLinks(dev_id_list);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GenerateCompareDevicesAndLinks() failed.";
    return act_status;
  }

  // Check VlanHybrid capable
  if (compare_control.GetVlanHybridCapableConsistent()) {
    act_status = CheckVlanHybridCapableConsistent();
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "CheckVlanHybridCapableConsistent() failed.";
      return act_status;
    }
  }

  // Check Device Config
  if (compare_control.GetDeviceConfig()) {
    act_status = CheckDeviceConfig();
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "CheckDeviceConfig() failed.";
      return act_status;
    }
  }
  UpdateProgress(10);

  // Check Alive(TSN Switch)
  if (compare_control.GetAlive()) {
    act_status = CheckAlive();
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "CheckAlive() failed.";
      return act_status;
    }
  }
  UpdateProgress(20);

  // Enable device's SNMP
  act_status = UpdateDevicesConnectStatusAndEnableSnmp();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "UpdateDevicesConnectStatusAndEnableSnmp() failed.";
    return act_status;
  }

  UpdateProgress(30);

  // Check Topology consistent (TSN Switch's Link)
  if (compare_control.GetTopologyConsistent()) {
    act_status = CheckTopologyConsistent();
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "CheckTopologyConsistent() failed.";
      return act_status;
    }
  }
  UpdateProgress(50);

  // Check ModelName
  if (compare_control.GetModelName()) {
    act_status = CheckModelName();
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "CheckModelName() failed.";
      return act_status;
    }
  }
  UpdateProgress(70);

  // Check LinkSpeed
  if (compare_control.GetSpeed()) {
    act_status = CheckLinkSpeed();
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "CheckLinkSpeed() failed.";
      return act_status;
    }
  }
  UpdateProgress(80);

  UpdateProgress(100);

  return act_status;
}

ACT_STATUS act::topology::ActCompare::GenerateCompareDevicesAndLinks(const QList<qint64> &dev_id_list) {
  ACT_STATUS_INIT();

  for (auto dev_id : dev_id_list) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    // Get project device by dev_id
    ActDevice dev;
    act_status = ActGetItemById<ActDevice>(project_.GetDevices(), dev_id, dev);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    // Check compare device not as the EndStation or Unknown
    if (!ActDevice::CheckDeviceCanBeDeploy(dev)) {
      continue;
    }

    compare_devices_[dev_id] = dev;
  }

  for (auto link : project_.GetLinks()) {
    if (!CheckLinkDevicesAreComparedDevice(link)) {
      continue;
    }
    compare_links_[link.GetId()] = link;
  }

  return act_status;
}

ACT_STATUS act::topology::ActCompare::CheckVlanHybridCapableConsistent() {
  ACT_STATUS_INIT();
  bool fail_flag = false;
  QSet<QString> fail_devs;

  QSet<ActDevice>::iterator iter;
  auto first_dev_hybrid_capable = compare_devices_.begin()
                                      ->GetDeviceProperty()
                                      .GetFeatureGroup()
                                      .GetConfiguration()
                                      .GetVLANSetting()
                                      .GetHybridMode();
  for (auto dev : compare_devices_) {
    if (first_dev_hybrid_capable !=
        dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) {
      fail_flag = true;
      fail_devs.insert(QString("%1").arg(dev.GetIpv4().GetIpAddress()));
      qCritical() << __func__
                  << QString("Check Devices VLAN hybrid capable inconsistent. Device: %1(%2).")
                         .arg(dev.GetIpv4().GetIpAddress())
                         .arg(dev.GetId())
                         .toStdString()
                         .c_str();
    }
  }

  if (fail_flag) {
    return std::make_shared<ActStatusCompareFailed>("Device VLAN hybrid capable inconsistent", "Device", fail_devs);
  }

  return act_status;
}

ACT_STATUS act::topology::ActCompare::CheckDeviceConfig() {
  ACT_STATUS_INIT();
  bool fail_flag = false;
  QSet<QString> fail_devices;

  // Check Unicast StaticForward
  for (auto static_forward_table : project_.GetDeviceConfig().GetUnicastStaticForwardTables()) {
    // Check each entries
    for (auto static_forward_entry : static_forward_table.GetStaticForwardEntries()) {
      // Check MAC

      auto check_mac_status = CheckMacAddress(static_forward_entry.GetMAC());
      if (!IsActStatusSuccess(check_mac_status)) {
        // Get device
        ActDevice device;
        project_.GetDeviceById(device, static_forward_table.GetDeviceId());

        fail_flag = true;
        fail_devices.insert(device.GetIpv4().GetIpAddress());
        qCritical() << __func__
                    << QString("UnicastStaticForward config - %1. Device: %2(%3).")
                           .arg(check_mac_status->GetErrorMessage())
                           .arg(device.GetIpv4().GetIpAddress())
                           .arg(device.GetId())
                           .toStdString()
                           .c_str();

        continue;
      }
    }
  }

  if (fail_flag) {
    return std::make_shared<ActStatusCompareFailed>("Unicast Static Forward MAC", "Device", fail_devices);
  }

  return act_status;
}

ACT_STATUS act::topology::ActCompare::CheckAlive() {
  ACT_STATUS_INIT();

  // Check device is alive
  bool fail_flag = false;
  QSet<QString> fail_devs;
  for (auto dev : compare_devices_) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    act_status = southbound_.PingIpAddress(dev.GetIpv4().GetIpAddress(), ACT_PING_REPEAT_TIMES);
    if (!IsActStatusSuccess(act_status)) {
      fail_flag = true;
      fail_devs.insert(QString("%1").arg(dev.GetIpv4().GetIpAddress()));

      qCritical() << __func__
                  << QString("Device not found. Device: %1(%2)")
                         .arg(dev.GetIpv4().GetIpAddress())
                         .arg(dev.GetId())
                         .toStdString()
                         .c_str();
    }
  }
  if (fail_flag) {
    return std::make_shared<ActStatusCompareFailed>("device alive", "Device", fail_devs);
  }
  return act_status;
}

ACT_STATUS act::topology::ActCompare::UpdateDevicesConnectStatusAndEnableSnmp() {
  ACT_STATUS_INIT();

  // QList<ActDevice> new_cmp_devices;
  for (auto dev : compare_devices_) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    // Start update connect status by southbound
    act_status = southbound_.FeatureAssignDeviceStatus(false, dev);
    if (!IsActStatusSuccess(act_status)) {
      DeviceErrorLogHandler(__func__, "FeatureAssignDeviceStatus() Failed", dev);
      return act_status;
    }

    compare_devices_[dev.GetId()] = dev;
  }

  return act_status;
}

bool act::topology::ActCompare::CheckLinkDevicesAreComparedDevice(const ActLink &link) {
  // Check Src Device
  if (!compare_devices_.contains(link.GetSourceDeviceId())) {
    return false;
  }

  // Check Dst Device
  if (!compare_devices_.contains(link.GetDestinationDeviceId())) {
    return false;
  }

  return true;
}

// ACT_STATUS act::topology::ActCompare::CheckLinkDevicesAreTsnSwitch(const ActLink &link) {
//   ACT_STATUS_INIT();

//   // Check src & dst is TsnSwitch
//   ActDevice src_dev, dst_dev;
//   // Find SourceDevice
//   act_status = ActGetItemById<ActDevice>(compare_devices_, link.GetSourceDeviceId(), src_dev);
//   if (!IsActStatusSuccess(act_status)) {
//     qCritical() << __func__
//                 << QString(": Link(%1) SourceDevice(%2) in devices not found.")
//                        .arg(link.GetId())
//                        .arg(link.GetSourceDeviceId())
//                        .toStdString()
//                        .c_str();
//     return act_status;
//   }
//   // Find DestinationDevice
//   act_status = ActGetItemById<ActDevice>(compare_devices_, link.GetDestinationDeviceId(), dst_dev);
//   if (!IsActStatusSuccess(act_status)) {
//     qCritical() << __func__
//                 << QString(": Link(%1) DestinationDevice(%2) in devices not found.")
//                        .arg(link.GetId())
//                        .arg(link.GetDestinationDeviceId())
//                        .toStdString()
//                        .c_str();
//     return act_status;
//   }

//   // Skip none tsnSwitch link.
//   if (src_dev.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch ||
//       dst_dev.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch) {
//     return std::make_shared<ActStatusInternalError>("Compare");
//   }

//   return act_status;
// }

ACT_STATUS act::topology::ActCompare::CheckTopologyConsistent() {
  ACT_STATUS_INIT();
  QSet<ActLink> inconsistent_links;

  // Get Ip-Mac Table
  QMap<QString, QString> ip_mac_table;
  act_status = southbound_.GetIpMacTable(ip_mac_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetIpMacTable() failed.";
    return act_status;
  }

  // Assign MAC address by ACT's ip_mac_table(For Scan links)
  for (auto &dev : compare_devices_) {
    dev.SetMacAddress(ip_mac_table[dev.GetIpv4().GetIpAddress()]);

    // Set MAC int
    qint64 mac_int = 0;
    MacAddressToQInt64(dev.GetMacAddress(), mac_int);
    dev.mac_address_int = mac_int;

    // Assign Lldp Data & Interfaces
    act_status = southbound_.AssignDeviceLldpData(dev);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "AssignDeviceLldpData() failed.";
      return act_status;
    }
  }

  // Use LLDP data to generate links
  QSet<ActLink> south_links;
  for (auto &dev : compare_devices_) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    QSet<ActLink> south_lldp_links;
    ActScanLinksResult south_result;
    QList<ActDevice> compare_device_list = compare_devices_.values();
    act_status = southbound_.GenerateLinkSetBylldpInfo(dev, compare_device_list, south_result);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenerateLinkSetBylldpInfo() failed.";
      return act_status;
    }

    for (auto link : south_result.GetScanLinks()) {
      // Check src & dst is compared device
      if (!CheckLinkDevicesAreComparedDevice(link)) {
        continue;
      }

      link.SetId(south_links.size() + 1);  // re-assign id
      // Insert to alive_links
      south_links.insert(link);
    }
  }
  qCritical() << __func__ << "Actual topology Device connect links:";
  for (auto link : south_links) {
    qDebug() << __func__
             << QString("Device connect Link: SrcDevice:%1, SrcInterface:%2 <--> DstDevice:%3, DstInterface:%4")
                    .arg(link.GetSourceDeviceId())
                    .arg(link.GetSourceInterfaceId())
                    .arg(link.GetDestinationDeviceId())
                    .arg(link.GetDestinationInterfaceId())
                    .toStdString()
                    .c_str();
  }

  for (auto link : compare_links_) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    // Find south_link
    ActLink target_south_link(link);
    target_south_link.SetId(-1);
    auto south_link_iter = south_links.find(target_south_link);
    if (south_link_iter == south_links.end()) {  // not found
      inconsistent_links.insert(link);
      qCritical() << __func__
                  << QString("Link(%1) not found in southbound links.").arg(link.GetId()).toStdString().c_str();
      continue;
    }

    // Find
    // Check src_dev, src_inteface, dst_dev, dst_inteface
    // First find link's dst_dev side at south_link. To check link direction.
    bool link_check_pass = false;
    if (link.GetSourceDeviceId() == south_link_iter->GetSourceDeviceId()) {  // forward
      if ((link.GetSourceInterfaceId() == south_link_iter->GetSourceInterfaceId()) &&
          (link.GetDestinationDeviceId() == south_link_iter->GetDestinationDeviceId()) &&
          (link.GetDestinationInterfaceId() == south_link_iter->GetDestinationInterfaceId())) {
        // check pass
        link_check_pass = true;
      }
    } else {  // opposite
      if ((link.GetSourceDeviceId() == south_link_iter->GetDestinationDeviceId()) &&
          (link.GetSourceInterfaceId() == south_link_iter->GetDestinationInterfaceId()) &&
          (link.GetDestinationDeviceId() == south_link_iter->GetSourceDeviceId()) &&
          (link.GetDestinationInterfaceId() == south_link_iter->GetSourceInterfaceId())) {
        // check pass
        link_check_pass = true;
      }
    }

    // Handle check link result
    if (link_check_pass) {
      south_links.erase(south_link_iter);
    } else {
      inconsistent_links.insert(link);
    }
  }

  if ((!south_links.isEmpty()) ||
      (!inconsistent_links.isEmpty())) {  // if south_links or inconsistent_links not empty -> fail
    QSet<QString> extra_links;
    QSet<QString> not_found_links;

    // Southbound topology links
    qCritical() << __func__ << "Actual topology inconsistent links:";
    for (auto link : south_links) {
      QString link_src_dev_ip = compare_devices_[link.GetSourceDeviceId()].GetIpv4().GetIpAddress();
      QString link_dst_dev_ip = compare_devices_[link.GetDestinationDeviceId()].GetIpv4().GetIpAddress();

      qDebug()
          << __func__
          << QString(
                 "Actual inconsistent Link: SrcDevice:%1(%2), SrcInterface:%3 <--> DstDevice:%4(%5), DstInterface:%6")
                 .arg(link.GetSourceDeviceId())
                 .arg(link_src_dev_ip)
                 .arg(link.GetSourceInterfaceId())
                 .arg(link.GetDestinationDeviceId())
                 .arg(link_dst_dev_ip)
                 .arg(link.GetDestinationInterfaceId())
                 .toStdString()
                 .c_str();

      QString link_str;
      CreateLinkString(link, link_str);
      extra_links.insert(link_str);
    }
    // ACT topology links
    qCritical() << __func__ << "ACT topology inconsistent links:";
    for (auto link : inconsistent_links) {
      QString link_src_dev_ip = compare_devices_[link.GetSourceDeviceId()].GetIpv4().GetIpAddress();
      QString link_dst_dev_ip = compare_devices_[link.GetDestinationDeviceId()].GetIpv4().GetIpAddress();

      qDebug()
          << __func__
          << QString(
                 "ACT inconsistent Link(%1): SrcDevice:%2(%3), SrcInterface:%4 <--> DstDevice:%5(%6), DstInterface:%7")
                 .arg(link.GetId())
                 .arg(link.GetSourceDeviceId())
                 .arg(link_src_dev_ip)
                 .arg(link.GetSourceInterfaceId())
                 .arg(link.GetDestinationDeviceId())
                 .arg(link_dst_dev_ip)
                 .arg(link.GetDestinationInterfaceId())
                 .toStdString()
                 .c_str();

      QString link_str;
      CreateLinkString(link, link_str);
      not_found_links.insert(link_str);
    }

    return std::make_shared<ActStatusCompareTopologyFailed>(not_found_links, extra_links);
  }

  return act_status;
}

ACT_STATUS act::topology::ActCompare::GetDeviceFeatureYangRevision(const ActDevice &device,
                                                                   QSet<QString> &result_yang_revisions) {
  ACT_STATUS_INIT();
  result_yang_revisions.clear();
  const QString netconf_str = kActConnectProtocolTypeEnumMap.key(ActConnectProtocolTypeEnum::kNETCONF);

  // Get device's DeviceProfile
  ActDeviceProfile device_profile;
  act_status =
      ActGetItemById<ActDeviceProfile>(profiles_.GetDeviceProfiles(), device.GetDeviceProfileId(), device_profile);
  if (!IsActStatusSuccess(act_status)) {
    QString not_found_elem = QString("DeviceProfile(%1)").arg(device.GetDeviceProfileId());
    qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
    return std::make_shared<ActStatusNotFound>(not_found_elem);
  }

  // iterator each Feature
  for (auto feature_profile : device_profile.GetFeatureCapability()) {
    // iterator each Items
    for (auto item_key : feature_profile.GetItems().keys()) {
      // iterator each Sub-item
      for (auto sub_item_key : feature_profile.GetItems()[item_key].GetSubItems().keys()) {
        // iterator each Method
        for (auto method_key : feature_profile.GetItems()[item_key].GetSubItems()[sub_item_key].GetMethods().keys()) {
          auto method = feature_profile.GetItems()[item_key].GetSubItems()[sub_item_key].GetMethods()[method_key];

          // NETCONF
          if (method.GetProtocols().contains(netconf_str)) {
            // iterator each Action
            for (auto action_key : method.GetProtocols()[netconf_str].GetActions().keys()) {
              auto yang_revision = method.GetProtocols()[netconf_str].GetActions()[action_key].GetSource();
              if (!yang_revision.isEmpty()) {
                result_yang_revisions.insert(yang_revision);
              }
            }
          }
        }
      }
    }
  }

  return act_status;
}

ACT_STATUS act::topology::ActCompare::CheckModelName() {
  ACT_STATUS_INIT();

  bool fail_flag = false;
  QSet<QString> fail_devs;
  for (auto dev : compare_devices_) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    ActFeatureSubItem feature_sub_item;
    act_status = GetDeviceFeatureSubItem(dev, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "Identify", "ModelName", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetFeatureProfileFeatureSubItem() failed";
      continue;
    }

    QString model_name;
    act_status = southbound_.ActionGetModelName(dev, feature_sub_item, model_name);
    if (!IsActStatusSuccess(act_status)) {
      fail_flag = true;
      fail_devs.insert(QString("%1").arg(dev.GetIpv4().GetIpAddress()));
      qCritical() << __func__ << "GetModelName() failed.";
      continue;
    }

    if (model_name != dev.GetDeviceProperty().GetModelName()) {
      fail_flag = true;
      fail_devs.insert(QString("%1").arg(dev.GetIpv4().GetIpAddress()));
      qCritical() << __func__
                  << QString(
                         "Check Device ModelName failed. Device: %1(%2). South ModelName(%2) not in DeviceProfile's "
                         "ModelName(%3)")
                         .arg(dev.GetIpv4().GetIpAddress())
                         .arg(dev.GetId())
                         .arg(model_name)
                         .arg(dev.GetDeviceProperty().GetModelName())
                         .toStdString()
                         .c_str();
      continue;
    }
  }

  if (fail_flag) {
    return std::make_shared<ActStatusCompareFailed>("device Model Name", "Device", fail_devs);
  }

  return act_status;
}

ACT_STATUS act::topology::ActCompare::CheckLinkSpeed() {
  ACT_STATUS_INIT();

  bool fail_flag = false;
  QSet<QString> fail_links;

  using InterfaceSpeedMap = QMap<qint64, qint64>;  // <InterfaceId, Speed>
  QMap<qint64, InterfaceSpeedMap> dev_speed_map;   // <DeviceId, InterfaceSpeedMap>

  // Get Speed
  for (auto dev : compare_devices_) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    // - Get Sub-Item (DeviceInformation  > PortSpeed)
    ActFeatureSubItem port_speed_sub_item;
    act_status =
        GetDeviceFeatureSubItem(dev, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kAutoScan, "DeviceInformation", "PortSpeed", port_speed_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    // Get port speed
    InterfaceSpeedMap interface_speed_map;
    act_status = southbound_.ActionGetPortSpeed(dev, port_speed_sub_item, interface_speed_map);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "ActionGetPortSpeed() failed.";
      return act_status;
    }
    dev_speed_map.insert(dev.GetId(), interface_speed_map);
  }

  // Check Speed. South Speed should be faster.
  for (auto link : compare_links_) {
    // qDebug() << "Check Link Speed:" << link.ToString().toStdString().c_str();
    QString link_str;
    CreateLinkString(link, link_str);

    // Source
    if (dev_speed_map.contains(link.GetSourceDeviceId())) {  //  found
      // Get south's interface speed
      InterfaceSpeedMap interface_speed_map = dev_speed_map.value(link.GetSourceDeviceId());
      quint64 south_speed = interface_speed_map.value(link.GetSourceInterfaceId(), 0);
      // qDebug() << "GetSpeed:" << link.GetSpeed() << ", south_speed:" << south_speed;

      // If south_speed == 0 would skip compare (pass)
      if ((south_speed != 0) && (link.GetSpeed() > south_speed)) {  // fail
        fail_flag = true;
        fail_links.insert(link_str);
        qCritical() << __func__
                    << QString(
                           "Check Link Speed failed. Link: %1(%2). South SourceInterface Speed: %3, Link Speed: %4 ")
                           .arg(link.GetId())
                           .arg(link_str)
                           .arg(south_speed)
                           .arg(link.GetSpeed())
                           .toStdString()
                           .c_str();
      }
    } else {  // not found
      qDebug() << __func__
               << QString("Link(%1(%2)) SourceDevice(%3) not TSNSwitch, so skip check it side Speed")
                      .arg(link.GetId())
                      .arg(link.GetSourceDeviceId())
                      .toStdString()
                      .c_str();
    }

    // Destination
    if (dev_speed_map.contains(link.GetDestinationDeviceId())) {  //  found
      // Get south's interface speed
      InterfaceSpeedMap interface_speed_map = dev_speed_map.value(link.GetDestinationDeviceId());
      quint16 south_speed = interface_speed_map.value(link.GetDestinationInterfaceId(), 0);

      // If south_speed == 0 would skip compare (pass)
      if ((south_speed != 0) && (link.GetSpeed() > south_speed)) {  // fail
        fail_flag = true;
        fail_links.insert(link_str);
        qCritical()
            << __func__
            << QString("Check Link Speed failed. Link: %1(%2). South DestinationInterface Speed: %3, Link Speed: %4 ")
                   .arg(link.GetId())
                   .arg(link_str)
                   .arg(south_speed)
                   .arg(link.GetSpeed())
                   .toStdString()
                   .c_str();
      }
    } else {  // not found
      qDebug() << __func__
               << QString("Link(%1(%2)) DestinationDevice(%3) not TSNSwitch, so skip check it side Speed")
                      .arg(link.GetId())
                      .arg(link_str)
                      .arg(link.GetDestinationDeviceId())
                      .toStdString()
                      .c_str();
    }
  }

  // // Print
  // QMapIterator<qint64, InterfaceSpeedMap> map_it(dev_speed_map);
  // while (map_it.hasNext()) {
  //   map_it.next();
  //   qDebug() << "Device:" << map_it.key() << ", SpeedMap:";
  //   QMapIterator<qint64, quint64> interface_speed_map_it(map_it.value());
  //   while (interface_speed_map_it.hasNext()) {
  //     interface_speed_map_it.next();
  //     qDebug() << "Interface:" << interface_speed_map_it.key() << ", Speed:" << interface_speed_map_it.value();
  //   }
  // }

  if (fail_flag) {
    return std::make_shared<ActStatusCompareFailed>("Link Speed", "Link", fail_links);
  }

  return act_status;
}

ACT_STATUS act::topology::ActCompare::CreateLinkString(const ActLink link, QString &link_str) {
  ACT_STATUS_INIT();

  link_str = QString("%1(%2)-%3(%4)")
                 .arg(compare_devices_[link.GetSourceDeviceId()].GetIpv4().GetIpAddress())
                 .arg(link.GetSourceInterfaceId())
                 .arg(compare_devices_[link.GetDestinationDeviceId()].GetIpv4().GetIpAddress())
                 .arg(link.GetDestinationInterfaceId());

  return act_status;
}

// ACT_STATUS act::topology::ActCompare::CheckInterfaceStatus() {
//   ACT_STATUS_INIT();

//   bool fail_flag = false;
//   QSet<QString> fail_links;

//   using InterfaceStatusMap = QMap<qint64, quint8>;     // <InterfaceId, Status>
//   QMap<qint64, InterfaceStatusMap> dev_if_status_map;  // <DeviceId, InterfaceStatusMap>

//   // Get InterfaceStatus
//   for (auto dev : compare_devices_) {
//     if (stop_flag_) {
//       return ACT_STATUS_STOP;
//     }

//     InterfaceStatusMap if_status_map;
//     act_status = southbound_.GetDeviceIfOperationalStatus(dev, if_status_map);
//     if (!IsActStatusSuccess(act_status)) {
//       qCritical() << __func__ << "GetDeviceIfOperationalStatus() failed.";
//       return act_status;
//     }
//     dev_if_status_map.insert(dev.GetId(), if_status_map);
//   }

//   // Check Status. South Status should be faster.
//   for (auto link : compare_links_) {
//     QString link_str;
//     CreateLinkString(link, link_str);

//     // Source
//     if (dev_if_status_map.contains(link.GetSourceDeviceId())) {  //  found
//       // Get south's interface status
//       InterfaceStatusMap if_status_map = dev_if_status_map.value(link.GetSourceDeviceId());
//       quint8 south_if_status = if_status_map.value(link.GetSourceInterfaceId(), 0);
//       if (south_if_status != ACT_INTERFACE_STATUS_UP) {  // fail
//         fail_flag = true;
//         fail_links.insert(link_str);
//         qCritical() << __func__
//                     << QString("Check Link Interface Status failed. Link: %1(%2). South SourceInterface(%3) Status:
//                     %4")
//                            .arg(link.GetId())
//                            .arg(link_str)
//                            .arg(link.GetSourceInterfaceId())
//                            .arg(south_if_status)
//                            .toStdString()
//                            .c_str();
//       }
//     } else {  // not found
//       qDebug() << __func__
//                << QString("Link(%1(%2)) SourceDevice(%3) not TSNSwitch, so skip check it side Interface Status")
//                       .arg(link.GetId())
//                       .arg(link_str)
//                       .arg(link.GetSourceDeviceId())
//                       .toStdString()
//                       .c_str();
//     }

//     // Destination
//     if (dev_if_status_map.contains(link.GetDestinationDeviceId())) {  //  found
//       // Get south's interface speed
//       InterfaceStatusMap if_status_map = dev_if_status_map.value(link.GetDestinationDeviceId());
//       quint8 south_if_status = if_status_map.value(link.GetDestinationInterfaceId(), 0);
//       if (south_if_status != ACT_INTERFACE_STATUS_UP) {  // fail
//         fail_flag = true;
//         fail_links.insert(link_str);
//         qCritical()
//             << __func__
//             << QString("Check Link Interface Status failed. Link: %1(%2). South DestinationInterface(%3) Status: %4")
//                    .arg(link.GetId())
//                    .arg(link_str)
//                    .arg(link.GetDestinationInterfaceId())
//                    .arg(south_if_status)
//                    .toStdString()
//                    .c_str();
//       }
//     } else {  // not found
//       qDebug() << __func__
//                << QString("Link(%1(%2)) DestinationDevice(%3) not TSNSwitch, so skip check it side Interface Status")
//                       .arg(link.GetId())
//                       .arg(link_str)
//                       .arg(link.GetDestinationDeviceId())
//                       .toStdString()
//                       .c_str();
//     }
//   }

//   // // Print
//   // QMapIterator<qint64, InterfaceStatusMap> map_it(dev_if_status_map);
//   // while (map_it.hasNext()) {
//   //   map_it.next();
//   //   qDebug() << "Device:" << map_it.key() << ", StatusMap:";
//   //   QMapIterator<qint64, quint8> if_status_map_it(map_it.value());
//   //   while (if_status_map_it.hasNext()) {
//   //     if_status_map_it.next();
//   //     qDebug() << "Interface:" << if_status_map_it.key() << ", Status:" << if_status_map_it.value();
//   //   }
//   // }

//   if (fail_flag) {
//     return std::make_shared<ActStatusCompareFailed>("Link Interface status", "Link", fail_links);
//   }

//   return act_status;
// }

// ACT_STATUS act::topology::ActCompare::CheckPropagationDelay() {
//   ACT_STATUS_INIT();

//   bool fail_flag = false;
//   QSet<QString> fail_links;

//   using InterfacePropDelayMap = QMap<qint64, quint32>;     // <InterfaceId, PropDelay>
//   QMap<qint64, InterfacePropDelayMap> dev_prop_delay_map;  // <DeviceId, InterfacePropDelayMap>

//   // Get PropagationDelay
//   for (auto dev : compare_devices_) {
//     if (stop_flag_) {
//       return ACT_STATUS_STOP;
//     }

//     InterfacePropDelayMap interface_prop_delay_map;
//     act_status = southbound_.GetDeviceIfPropagationDelays(dev, interface_prop_delay_map);
//     if (!IsActStatusSuccess(act_status)) {
//       qCritical() << __func__ << "GetDeviceIfPropagationDelays() failed.";
//       return act_status;
//     }
//     dev_prop_delay_map.insert(dev.GetId(), interface_prop_delay_map);
//   }

//   // Check PropagationDelay. South PropagationDelay should be smaller.
//   for (auto link : compare_links_) {
//     QString link_str;
//     CreateLinkString(link, link_str);

//     // Source
//     if (dev_prop_delay_map.contains(link.GetSourceDeviceId())) {  //  found
//       // Get south's interface PropagationDelay
//       InterfacePropDelayMap interface_prop_delay_map = dev_prop_delay_map.value(link.GetSourceDeviceId());
//       quint64 south_prop_delay = interface_prop_delay_map.value(link.GetSourceInterfaceId(), 0);
//       // qDebug() << "[Source] GetPropagationDelay:" << link.GetPropagationDelay()
//       //          << ", south_prop_delay:" << south_prop_delay;
//       if (link.GetPropagationDelay() < south_prop_delay) {  // fail
//         fail_flag = true;
//         fail_links.insert(link_str);
//         qCritical()
//             << __func__
//             << QString(
//                    "Check Link PropagationDelay failed. Link: %1(%2). South SourceInterface PropagationDelay: %3, "
//                    "Link PropagationDelay: %4 ")
//                    .arg(link.GetId())
//                    .arg(link_str)
//                    .arg(south_prop_delay)
//                    .arg(link.GetPropagationDelay())
//                    .toStdString()
//                    .c_str();
//       }
//     } else {  // not found
//       qDebug() << __func__
//                << QString("Link(%1(%2)) SourceDevice(%3) not TSNSwitch, so skip check it side PropagationDelay")
//                       .arg(link.GetId())
//                       .arg(link_str)
//                       .arg(link.GetSourceDeviceId())
//                       .toStdString()
//                       .c_str();
//     }

//     // Destination
//     if (dev_prop_delay_map.contains(link.GetDestinationDeviceId())) {  //  found
//       // Get south's interface PropagationDelay
//       InterfacePropDelayMap interface_prop_delay_map = dev_prop_delay_map.value(link.GetDestinationDeviceId());
//       quint64 south_prop_delay = interface_prop_delay_map.value(link.GetDestinationInterfaceId(), 0);
//       // qDebug() << "[Destination] GetPropagationDelay:" << link.GetPropagationDelay()
//       //          << ", south_prop_delay:" << south_prop_delay;

//       if (link.GetPropagationDelay() < south_prop_delay) {  // fail
//         fail_flag = true;
//         fail_links.insert(link_str);
//         qCritical()
//             << __func__
//             << QString(
//                    "Check Link PropagationDelay failed. Link: %1(%2). South DestinationInterface PropagationDelay: "
//                    "%3, Link PropagationDelay: %4 ")
//                    .arg(link.GetId())
//                    .arg(link_str)
//                    .arg(south_prop_delay)
//                    .arg(link.GetPropagationDelay())
//                    .toStdString()
//                    .c_str();
//       }
//     } else {  // not found
//       qDebug() << __func__
//                << QString("Link(%1(%2)) DestinationDevice(%3) not TSNSwitch, so skip check it side PropagationDelay")
//                       .arg(link.GetId())
//                       .arg(link_str)
//                       .arg(link.GetDestinationDeviceId())
//                       .toStdString()
//                       .c_str();
//     }
//   }

//   // // Print
//   // QMapIterator<qint64, InterfacePropDelayMap> map_it(dev_prop_delay_map);
//   // while (map_it.hasNext()) {
//   //   map_it.next();
//   //   qDebug() << "Device:" << map_it.key() << ", PropagationDelayMap:";
//   //   QMapIterator<qint64, quint32> interface_prop_delay_map_it(map_it.value());
//   //   while (interface_prop_delay_map_it.hasNext()) {
//   //     interface_prop_delay_map_it.next();
//   //     qDebug() << "Interface:" << interface_prop_delay_map_it.key() << ", PropagationDelay:" <<
//   //     interface_prop_delay_map_it.value();
//   //   }
//   // }

//   if (fail_flag) {
//     return std::make_shared<ActStatusInternalError>("Compare");
//   }

//   return act_status;
// }
