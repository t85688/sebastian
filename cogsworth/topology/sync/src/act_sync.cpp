#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QDebug>

#include "act_sync.hpp"

act::topology::ActSync::~ActSync() {
  if ((sync_thread_ != nullptr) && (sync_thread_->joinable())) {
    sync_thread_->join();
  }
}

ACT_STATUS act::topology::ActSync::GetStatus() {
  if (IsActStatusSuccess(sync_act_status_) && (progress_ == 100)) {
    sync_act_status_->SetStatus(ActStatusType::kFinished);
  }

  if ((!IsActStatusRunning(sync_act_status_)) && (!IsActStatusFinished(sync_act_status_))) {
    // failed
    return sync_act_status_;
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*sync_act_status_), progress_);
}

ACT_STATUS act::topology::ActSync::UpdateProgress(quint8 progress) {
  ACT_STATUS_INIT();
  progress_ = progress;
  qDebug() << __func__ << QString("Progress: %1%.").arg(GetProgress()).toStdString().c_str();
  return act_status;
}

ACT_STATUS act::topology::ActSync::Stop() {
  // Checking has the thread is running
  if (IsActStatusRunning(sync_act_status_)) {
    qDebug() << "Stop SyncMac thread.";

    southbound_.SetStopFlag(true);

    // Send the stop signal to the SyncMac and wait for the thread to finish.
    stop_flag_ = true;
    if ((sync_thread_ != nullptr) && (sync_thread_->joinable())) {
      sync_thread_->join();  // wait thread finished
    }
  } else {
    qDebug() << __func__ << "The SyncMac thread not running.";
  }
  return std::make_shared<ActProgressStatus>(ActStatusBase(*sync_act_status_), progress_);
}

ACT_STATUS act::topology::ActSync::Start(ActProject &project, const QList<qint64> &dev_id_list) {
  // Checking has the thread is running
  if (IsActStatusRunning(sync_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("SyncMac");
  }

  // init SyncMac status
  progress_ = 0;
  stop_flag_ = false;
  sync_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to triggered the SyncMac
  try {
    // check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((sync_thread_ != nullptr) && (sync_thread_->joinable())) {
      sync_thread_->join();
    }

    sync_act_status_->SetStatus(ActStatusType::kRunning);
    sync_thread_ = std::make_unique<std::thread>(&act::topology::ActSync::TriggeredSyncForThread, this,
                                                 std::ref(project), std::ref(dev_id_list));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggeredSyncForThread";
    HRESULT hr = SetThreadDescription(sync_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(SyncMac) failed. Error:" << e.what();
    sync_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("SyncMac");
  }

  qDebug() << "Start SyncMac thread.";
  return std::make_shared<ActProgressStatus>(ActStatusBase(*sync_act_status_), progress_);
}

void act::topology::ActSync::TriggeredSyncForThread(ActProject &project, const QList<qint64> &dev_id_list) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the Sync and wait for the return, and update sync_act_status_.
  try {
    sync_act_status_ = SyncDevices(project, dev_id_list);
  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(interrupt) failed. Error:" << e.what();
    sync_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kSyncFailed, ActSeverity::kCritical);
  }
}

ACT_STATUS act::topology::ActSync::SyncErrorHandler(QString called_func, const QString &error_reason,
                                                    const ActDevice &device) {
  qCritical() << called_func.toStdString().c_str()
              << QString("Device: %1(%2): %3")
                     .arg(device.GetIpv4().GetIpAddress())
                     .arg(device.GetId())
                     .arg(error_reason)
                     .toStdString()
                     .c_str();

  result_queue_.enqueue(ActDeviceConfigureResult(device.GetId(), progress_, ActStatusType::kFailed, error_reason));

  failed_device_id_set_.insert(device.GetId());
  return ACT_STATUS_SUCCESS;
}
ACT_STATUS act::topology::ActSync::SyncFirmwareVersion(ActDevice &dev) {
  ACT_STATUS_INIT();
  // FirmwareVersion
  ActFeatureSubItem feature_sub_item;
  if (dev.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetIdentify().GetFirmwareVersion()) {
    act_status = GetDeviceFeatureSubItem(dev, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "Identify", "FirmwareVersion", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QString firmware_version;
      act_status = southbound_.ActionGetFirmwareVersion(dev, feature_sub_item, firmware_version);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetFirmwareVersion() failed.";
        return act_status;
      } else {
        // If different would update firmware_version & firmware_feature_profile & feature_group
        if (dev.GetFirmwareVersion() != firmware_version) {
          dev.SetFirmwareVersion(firmware_version);

          // Try to find firmware_feature_profile
          ActFirmwareFeatureProfile fw_feat_profile;
          auto find_status = ActFirmwareFeatureProfile::GetFirmwareFeatureProfile(
              profiles_.GetFirmwareFeatureProfiles(), dev.GetDeviceProperty().GetModelName(), firmware_version,
              fw_feat_profile);
          if (IsActStatusSuccess(find_status)) {
            dev.SetFirmwareFeatureProfileId(fw_feat_profile.GetId());
            dev.GetDeviceProperty().SetFeatureGroup(fw_feat_profile.GetFeatureGroup());
          } else {
            dev.SetFirmwareFeatureProfileId(-1);

            // Get DeviceProfile & Update FeatureGroup
            ActDeviceProfile dev_profile;
            act_status =
                ActGetItemById<ActDeviceProfile>(profiles_.GetDeviceProfiles(), dev.GetDeviceProfileId(), dev_profile);
            if (!IsActStatusSuccess(act_status)) {
              QString not_found_elem = QString("DeviceProfile(%1)").arg(dev.GetDeviceProfileId());
              qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
              return std::make_shared<ActStatusNotFound>(not_found_elem);
            }
            dev.GetDeviceProperty().SetFeatureGroup(dev_profile.GetFeatureGroup());
          }
        }
      }
    }
  }
  return act_status;
}

ACT_STATUS act::topology::ActSync::SyncDevices(ActProject &project, const QList<qint64> &dev_id_list) {
  ACT_STATUS_INIT();
  QList<ActDevice> dev_list;

  // Create device list
  for (auto &dev_id : dev_id_list) {
    ActDevice dev;
    act_status = project.GetDeviceById(dev, dev_id);
    if (!IsActStatusSuccess(act_status)) {
      SyncErrorHandler(__func__, "Device not found in the project", dev);
      continue;
    }
    dev_list.append(dev);
  }

  // Update Devices ICMP status to check alive
  southbound_.UpdateDevicesIcmpStatus(dev_list);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  UpdateProgress(10);

  for (auto &dev : dev_list) {
    // Check ICMP status
    if (!dev.GetDeviceStatus().GetICMPStatus()) {
      SyncErrorHandler(__func__, "ICMP status is false(not alive)", dev);
      continue;
    }

    // Update connect status to true for southbound
    southbound_.UpdateDeviceConnectByScanFeature(dev);

    // Sync firmware
    act_status = SyncFirmwareVersion(dev);
    if (!IsActStatusSuccess(act_status)) {
      SyncErrorHandler(__func__, "Sync Firmware version failed", dev);
      continue;
    }
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    // Sync DeviceConfig
    southbound_.AssignDeviceConfigs(dev, project.GetDeviceConfig());
    if (!IsActStatusSuccess(act_status)) {
      QString err_msg = QString("Sync Device configs failed(%1)").arg(act_status->GetErrorMessage());
      SyncErrorHandler(__func__, err_msg, dev);
      continue;
    }
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    // Update dev to project
    auto proj_dev_iter = project.GetDevices().find(dev);
    if (proj_dev_iter != project.GetDevices().end()) {  // found
      project.GetDevices().erase(proj_dev_iter);
      project.GetDevices().insert(dev);
    }

    // Add success result to result_queue_
    result_queue_.enqueue(ActDeviceConfigureResult(dev.GetId(), progress_, ActStatusType::kSuccess));

    // Update progress(10~90/100)
    quint8 new_progress = progress_ + (80 / dev_id_list.size());
    if (new_progress >= 90) {
      UpdateProgress(90);
    } else {
      UpdateProgress(new_progress);
    }
  }

  SLEEP_MS(100);
  UpdateProgress(100);
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS act::topology::ActSync::Sync(ActProject &project) {
  ACT_STATUS_INIT();

  // TODO: remove
  // [feat:3661] Remove fake MAC

  // [feat:2615] Add 'Check End Station' button to the Deploy module

  // // Sync Project's Devices & Interfaces MAC address
  // QMap<QString, QString> sync_mac_mapping_map;  // <old_mac, new_mac>
  // act_status = SyncDevicesMac(project, sync_mac_mapping_map);
  // if (!IsActStatusSuccess(act_status)) {
  //   qCritical() << __func__ << "SyncDevicesMac() failed";

  //   return act_status;
  // }
  // UpdateProgress(50);

  // // Update Compute Result
  // UpdateComputeResult(project, sync_mac_mapping_map);

  // // Update Stream Listeners & Talker & StreamId
  // UpdateStreams(project, sync_mac_mapping_map);

  // // Update Device Config
  // UpdateDeviceConfig(project, sync_mac_mapping_map);

  UpdateProgress(100);

  return act_status;
}

// ACT_STATUS act::topology::ActSync::UpdateComputeResult(ActProject &project,
//                                                        QMap<QString, QString> &sync_mac_mapping_map) {
//   ACT_STATUS_INIT();

//   // ComputeResult > Devices
//   QSet<ActDevice> new_result_devices;
//   for (auto cmp_dev : project.GetComputedResult().GetDevices()) {
//     auto iter = sync_mac_mapping_map.find(cmp_dev.GetMacAddress());
//     if (iter != sync_mac_mapping_map.end()) {  // found

//       // Update device MAC address
//       cmp_dev.SetMacAddress(iter.value());
//       // Set MAC int
//       qint64 mac_int = 0;
//       MacAddressToQInt64(cmp_dev.GetMacAddress(), mac_int);
//       cmp_dev.mac_address_int = mac_int;
//     }

//     // Update interfaces MAC address
//     for (auto &cmp_dev_interface : cmp_dev.GetInterfaces()) {
//       auto mac_iter = sync_mac_mapping_map.find(cmp_dev_interface.GetMacAddress());
//       if (mac_iter != sync_mac_mapping_map.end()) {  // found
//         cmp_dev_interface.SetMacAddress(mac_iter.value());
//       }
//     }
//     new_result_devices.insert(cmp_dev);
//   }
//   project.GetComputedResult().SetDevices(new_result_devices);

//   return act_status;
// }

// ACT_STATUS act::topology::ActSync::UpdateStreams(ActProject &project, QMap<QString, QString> &sync_mac_mapping_map) {
//   ACT_STATUS_INIT();

//   QSet<ActStream> new_streams;
//   for (auto stream : project.GetStreams()) {
//     // Update StreamId > MacAddress
//     auto iter = sync_mac_mapping_map.find(stream.GetStreamId().GetMacAddress());
//     if (iter != sync_mac_mapping_map.end()) {  // found
//       stream.GetStreamId().SetMacAddress(iter.value());
//     }

//     // Update Talker > EndStationInterface > MacAddress
//     iter = sync_mac_mapping_map.find(stream.GetTalker().GetEndStationInterface().GetMacAddress());
//     if (iter != sync_mac_mapping_map.end()) {  // found
//       stream.GetTalker().GetEndStationInterface().SetMacAddress(iter.value());
//     }

//     // Update Talker > DataFrameSpecification > Ieee802MacAddresses > SourceMacAddress & DestinationMacAddress
//     for (auto &data_frame_spec : stream.GetTalker().GetDataFrameSpecifications()) {
//       if (data_frame_spec.GetType() == ActFieldTypeEnum::kIeee802MacAddresses) {
//         // SourceMacAddress
//         iter = sync_mac_mapping_map.find(data_frame_spec.GetIeee802MacAddresses().GetSourceMacAddress());
//         if (iter != sync_mac_mapping_map.end()) {  // found
//           data_frame_spec.GetIeee802MacAddresses().SetSourceMacAddress(iter.value());
//         }

//         // DestinationMacAddress(Unicast)
//         if (!stream.GetMulticast()) {
//           iter = sync_mac_mapping_map.find(data_frame_spec.GetIeee802MacAddresses().GetDestinationMacAddress());
//           if (iter != sync_mac_mapping_map.end()) {  // found
//             data_frame_spec.GetIeee802MacAddresses().SetDestinationMacAddress(iter.value());
//           }
//         }
//       }
//     }

//     // Update Listeners
//     for (auto &listener : stream.GetListeners()) {
//       // Update Listener > EndStationInterface > MacAddress
//       iter = sync_mac_mapping_map.find(listener.GetEndStationInterface().GetMacAddress());
//       if (iter != sync_mac_mapping_map.end()) {  // found
//         listener.GetEndStationInterface().SetMacAddress(iter.value());
//       }
//     }

//     new_streams.insert(stream);
//   }
//   project.SetStreams(new_streams);

//   return act_status;
// }

// ACT_STATUS act::topology::ActSync::UpdateDeviceConfig(ActProject &project,
//                                                       QMap<QString, QString> &sync_mac_mapping_map) {
//   ACT_STATUS_INIT();

//   // Sync UnicastStaticForwardTable
//   for (auto static_forward_table : project.GetDeviceConfig().GetUnicastStaticForwardTables()) {
//     QSet<ActStaticForwardEntry> new_static_forward_entries;
//     for (auto static_forward_entry : static_forward_table.GetStaticForwardEntries()) {
//       // Check mapping map has this entry MAC
//       auto iter = sync_mac_mapping_map.find(static_forward_entry.GetMAC());
//       if (iter != sync_mac_mapping_map.end()) {  // found
//         // Update MAC
//         static_forward_entry.SetMAC(iter.value());
//       }
//       new_static_forward_entries.insert(static_forward_entry);
//     }

//     // Update entries to table
//     static_forward_table.SetStaticForwardEntries(new_static_forward_entries);
//     // Update table to DeviceConfig > UnicastStaticForwardTables
//     project.GetDeviceConfig().GetUnicastStaticForwardTables()[static_forward_table.GetDeviceId()] =
//         static_forward_table;
//   }

//   // Sync CB stream identity
//   for (auto cb_table : project.GetDeviceConfig().GetCbTables()) {
//     for (auto &stream_id_entry : cb_table.GetStreamIdentityList()) {
//       // DmacVlanStreamIdentification > Down
//       if (!stream_id_entry.GetTsnStreamIdEntryGroup()
//                .GetDmacVlanStreamIdentification()
//                .GetDown()
//                .GetDestinationMac()
//                .isEmpty()) {
//         auto iter = sync_mac_mapping_map.find(
//             stream_id_entry.GetTsnStreamIdEntryGroup().GetDmacVlanStreamIdentification().GetDown().GetDestinationMac());
//         if (iter != sync_mac_mapping_map.end()) {  // found
//           // Update MAC
//           stream_id_entry.GetTsnStreamIdEntryGroup().GetDmacVlanStreamIdentification().GetDown().SetDestinationMac(
//               iter.value());
//         }
//       }

//       // DmacVlanStreamIdentification > Up
//       if (!stream_id_entry.GetTsnStreamIdEntryGroup()
//                .GetDmacVlanStreamIdentification()
//                .GetUp()
//                .GetDestinationMac()
//                .isEmpty()) {
//         auto iter = sync_mac_mapping_map.find(
//             stream_id_entry.GetTsnStreamIdEntryGroup().GetDmacVlanStreamIdentification().GetUp().GetDestinationMac());
//         if (iter != sync_mac_mapping_map.end()) {  // found
//           // Update MAC
//           stream_id_entry.GetTsnStreamIdEntryGroup().GetDmacVlanStreamIdentification().GetUp().SetDestinationMac(
//               iter.value());
//         }
//       }

//       // IpStreamIdentification
//       if (!stream_id_entry.GetTsnStreamIdEntryGroup().GetIpStreamIdentification().GetDestinationMac().isEmpty()) {
//         auto iter = sync_mac_mapping_map.find(
//             stream_id_entry.GetTsnStreamIdEntryGroup().GetIpStreamIdentification().GetDestinationMac());
//         if (iter != sync_mac_mapping_map.end()) {  // found
//           // Update MAC
//           stream_id_entry.GetTsnStreamIdEntryGroup().GetIpStreamIdentification().SetDestinationMac(iter.value());
//         }
//       }

//       // NullStreamIdentification
//       if (!stream_id_entry.GetTsnStreamIdEntryGroup().GetNullStreamIdentification().GetDestinationMac().isEmpty()) {
//         auto iter = sync_mac_mapping_map.find(
//             stream_id_entry.GetTsnStreamIdEntryGroup().GetNullStreamIdentification().GetDestinationMac());
//         if (iter != sync_mac_mapping_map.end()) {  // found
//           // Update MAC
//           stream_id_entry.GetTsnStreamIdEntryGroup().GetNullStreamIdentification().SetDestinationMac(iter.value());
//         }
//       }

//       // SmacVlanStreamIdentification
//       if (!stream_id_entry.GetTsnStreamIdEntryGroup().GetSmacVlanStreamIdentification().GetSourceMac().isEmpty()) {
//         auto iter = sync_mac_mapping_map.find(
//             stream_id_entry.GetTsnStreamIdEntryGroup().GetSmacVlanStreamIdentification().GetSourceMac());
//         if (iter != sync_mac_mapping_map.end()) {  // found
//           // Update MAC
//           stream_id_entry.GetTsnStreamIdEntryGroup().GetSmacVlanStreamIdentification().SetSourceMac(iter.value());
//         }
//       }
//     }

//     // Update table to DeviceConfig > CbTables
//     project.GetDeviceConfig().GetCbTables()[cb_table.GetDeviceId()] = cb_table;
//   }

//   return act_status;
// }

// ACT_STATUS act::topology::ActSync::SyncDevicesMac(ActProject &project, QMap<QString, QString> &sync_mac_mapping_map)
// {
//   // sync_mac_mapping_map<old_mac, new_mac>

//   ACT_STATUS_INIT();

//   QSet<ActDevice> new_devices;
//   auto old_device_set = project.GetDevices();
//   for (auto dev : old_device_set) {
//     if (stop_flag_) {
//       return ACT_STATUS_STOP;
//     }

//     // [feat:2615] Add 'Check End Station' button to the Deploy module
//     // Only sync EndStation
//     if (dev.GetDeviceType() != ActDeviceTypeEnum::kEndStation) {
//       new_devices.insert(dev);
//       continue;
//     }

//     // Ping device
//     act_status = southbound_.PingIpAddress(dev.GetIpv4().GetIpAddress(), ACT_PING_REPEAT_TIMES);
//     if (!IsActStatusSuccess(act_status)) {  // not reply
//       // [feat:2615] Add 'Check End Station' button to the Deploy module
//       QString error_msg = QString("Device(%1) is not alive").arg(dev.GetIpv4().GetIpAddress());
//       qCritical() << __func__ << error_msg.toStdString().c_str();
//       device_error_map_[dev.GetId()] = "Device not alive";
//       continue;
//     }

//     // Get IP-MAC table(arp_table & adapter_table)
//     QMap<QString, QString> ip_mac_table;
//     act_status = southbound_.GetIpMacTable(ip_mac_table);
//     if (!IsActStatusSuccess(act_status)) {
//       qCritical() << __func__ << "GetIpMacTable() failed.";

//       return act_status;
//     }

//     // Sync Device MAC address by ACT's ip_mac_table
//     QString new_mac = ip_mac_table[dev.GetIpv4().GetIpAddress()];
//     // Check MAC address format
//     act_status = CheckMacAddress(new_mac);
//     if (!IsActStatusSuccess(act_status)) {
//       // [feat:2615] Add 'Check End Station' button to the Deploy module
//       QString error_msg =
//           QString("Device (%1) - %2").arg(dev.GetIpv4().GetIpAddress()).arg(act_status->GetErrorMessage());
//       qCritical() << __func__ << error_msg.toStdString().c_str();
//       device_error_map_[dev.GetId()] = "Check access MAC failed";

//       continue;
//     }

//     // Update Device MAC address
//     if (new_mac != dev.GetMacAddress()) {
//       sync_mac_mapping_map[dev.GetMacAddress()] = new_mac;
//       dev.SetMacAddress(new_mac);
//       // Set MAC int
//       qint64 mac_int = 0;
//       MacAddressToQInt64(dev.GetMacAddress(), mac_int);
//       dev.mac_address_int = mac_int;
//     }

//     // Get InterfaceMac
//     // Access Device
//     // Update connect status to true for southbound
//     ActDevice south_dev(dev);
//     south_dev.GetDeviceStatus().SetAllConnectStatus(true);

//     ActFeatureSubItem if_mac_sub_item;
//     act_status =
//         GetDeviceFeatureSubItem(dev, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
//                                 ActFeatureEnum::kAutoScan, "DeviceInformation", "InterfaceMAC", if_mac_sub_item);
//     if (!IsActStatusSuccess(act_status)) {
//       qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
//       return std::make_shared<ActBadRequest>(act_status->GetErrorMessage());
//     }

//     QMap<qint64, QString> interface_mac_map;  // <InterfaceId, InterfaceMacAddress>
//     act_status = southbound_.ActionGetInterfaceMac(south_dev, if_mac_sub_item, interface_mac_map);

//     if (!IsActStatusSuccess(act_status)) {
//       // [feat:2615] Add 'Check End Station' button to the Deploy module
//       QString error_msg = QString("Device(%1) get the interfaces MAC address
//       failed").arg(dev.GetIpv4().GetIpAddress()); qCritical() << __func__ << error_msg.toStdString().c_str();
//       device_error_map_[dev.GetId()] = "Access interfaces MAC failed";

//       continue;
//     }

//     // Sync Interface MAC address
//     QList<ActInterface> new_interface_list;
//     for (auto new_interface : dev.GetInterfaces()) {
//       // Update MAC
//       if (interface_mac_map.contains(new_interface.GetInterfaceId())) {
//         QString new_mac = interface_mac_map[new_interface.GetInterfaceId()];
//         // Check MAC address format
//         act_status = CheckMacAddress(new_mac);

//         if (!IsActStatusSuccess(act_status)) {
//           // [feat:2615] Add 'Check End Station' button to the Deploy module
//           QString error_msg = QString("%1 Interface(%2) - %3")
//                                   .arg(dev.GetIpv4().GetIpAddress())
//                                   .arg(new_interface.GetInterfaceId())
//                                   .arg(act_status->GetErrorMessage());
//           qCritical() << __func__ << error_msg.toStdString().c_str();
//           return std::make_shared<ActBadRequest>(error_msg);
//         }
//         // Skip the same with the DB MAC address
//         if (new_mac == new_interface.GetMacAddress()) {
//           new_interface_list.append(new_interface);
//           continue;
//         }

//         // Replace the DB MAC address
//         sync_mac_mapping_map[new_interface.GetMacAddress()] = new_mac;
//         new_interface.SetMacAddress(new_mac);
//       }

//       new_interface_list.append(new_interface);
//     }
//     dev.SetInterfaces(new_interface_list);
//     new_devices.insert(dev);
//   }

//   // Replace devices
//   for (auto new_device : new_devices) {
//     project.GetDevices().remove(new_device);
//     project.GetDevices().insert(new_device);
//   }

//   return ACT_STATUS_SUCCESS;
// }
