#include <QtEndian>

#include "act_new_moxa_command_handler.h"
#include "act_restful_client_handler.h"
#include "act_snmp_handler.h"
#include "act_southbound.hpp"
#include "act_system.hpp"
#include "act_utilities.hpp"

ACT_STATUS ActSouthbound::FeatureUpdateDeviceStatusSnmp(const bool &use_feature_profile, ActDevice &device) {
  ACT_STATUS_INIT();

  bool connect_status = false;
  ActFeatureSubItem sub_item;
  QString action_str = "";

  // Get feature_sub_item
  if (use_feature_profile) {
    // FeatureProfiles
    act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kBase,
                                                 "CheckConnection", "SNMP", sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetFeatureProfileFeatureSubItem() failed";
      return act_status;
    }
  } else {
    // Device's FeatureCapability
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kBase, "CheckConnection", "SNMP", sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
      return act_status;
    }
  }

  // Start Check connect
  auto connect_act_status = ActionCheckSnmpConnect(device, sub_item);
  if (IsActStatusSuccess(connect_act_status)) {
    connect_status = true;
  }

  // qDebug() << QString("Device: %1(%2) connect_status: %3")
  //                 .arg(device.GetIpv4().GetIpAddress())
  //                 .arg(device.GetMacAddress())
  //                 .arg(connect_status)
  //                 .toStdString()
  //                 .c_str();

  mutex_.lock();
  // Set device status
  auto dev_status = device.GetDeviceStatus();
  dev_status.SetSNMPStatus(connect_status);
  device.SetDeviceStatus(dev_status);
  mutex_.unlock();

  return act_status;
}

ACT_STATUS ActSouthbound::FeatureUpdateDeviceStatusRestful(const bool &use_feature_profile, ActDevice &device) {
  ACT_STATUS_INIT();

  bool connect_status = false;
  ActFeatureSubItem sub_item;
  QString action_str = "";

  if (use_feature_profile) {
    // FeatureProfiles
    act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kBase,
                                                 "CheckConnection", "RESTful", sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetFeatureProfileFeatureSubItem() failed";
      return act_status;
    }

  } else {
    // Device's FeatureCapability
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kBase, "CheckConnection", "RESTful", sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
      return act_status;
    }
  }

  // Start Check connect
  auto connect_act_status = ActionCheckRestfulConnect(device, sub_item);
  if (IsActStatusSuccess(connect_act_status)) {
    connect_status = true;
  }

  mutex_.lock();
  // Set device status
  auto dev_status = device.GetDeviceStatus();
  dev_status.SetRESTfulStatus(connect_status);
  device.SetDeviceStatus(dev_status);
  mutex_.unlock();

  return act_status;
}

ACT_STATUS ActSouthbound::FeatureUpdateDeviceStatusNewMOXACommand(const bool &use_feature_profile, ActDevice &device) {
  ACT_STATUS_INIT();

  bool connect_status = false;
  ActFeatureSubItem sub_item;
  QString action_str = "";

  if (use_feature_profile) {
    // FeatureProfiles
    act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kBase,
                                                 "CheckConnection", "NewMOXACommand", sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetFeatureProfileFeatureSubItem() failed";
      return act_status;
    }

  } else {
    // Device's FeatureCapability
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kBase, "CheckConnection", "NewMOXACommand", sub_item);

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetDeviceProfileFeatureSubItem() failed";
      return act_status;
    }
  }

  // Start Check connect
  auto connect_act_status = ActionCheckNewMOXACommandConnect(device, sub_item);
  if (IsActStatusSuccess(connect_act_status)) {
    connect_status = true;
  }

  mutex_.lock();
  // Set device status
  auto dev_status = device.GetDeviceStatus();
  dev_status.SetNewMOXACommandStatus(connect_status);
  device.SetDeviceStatus(dev_status);
  mutex_.unlock();

  return act_status;
}

ACT_STATUS ActSouthbound::FeatureEnableDeviceSnmp(const bool &use_feature_profile, ActDevice &device,
                                                  const bool &check_connect) {
  ACT_STATUS_INIT();
  ActFeatureSubItem feature_sub_item;

  if (use_feature_profile) {
    act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kOperation,
                                                 "EnableSNMPService", "Basic", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetFeatureProfileFeatureSubItem() failed";
      return act_status;
    }

  } else {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kOperation, "EnableSNMPService", "Basic", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
      return act_status;
    }
  }

  // Start Enable SNMP service
  act_status = ActionEnableSnmpService(check_connect, device, feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__
                << QString("ActionEnableSnmpService() failed. Device: %1")
                       .arg(device.GetIpv4().GetIpAddress())
                       .toStdString()
                       .c_str();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSouthbound::FeatureAssignDeviceStatus(const bool &use_feature_profile, ActDevice &device) {
  ActDeviceConnectStatusControl control(true, true, true, true);
  return FeatureAssignDeviceStatus(use_feature_profile, device, control);
}

ACT_STATUS ActSouthbound::FeatureAssignDeviceStatus(const bool &use_feature_profile, ActDevice &device,
                                                    const ActDeviceConnectStatusControl &control) {
  ACT_STATUS_INIT();

  // Use multiple thread to check each connection status
  quint32 thread_count = 0;
  quint32 max_thread = 10;
  std::unique_ptr<std::thread[]> threads = std::make_unique<std::thread[]>(max_thread);
  // qDebug() << __func__
  //          << QString("Update Device(%1) Status Control: %2")
  //                 .arg(device.GetIpv4().GetIpAddress())
  //                 .arg(control.ToString())
  //                 .toStdString()
  //                 .c_str();

  // RESTful
  if (control.GetRESTful()) {
    threads[thread_count] = std::thread(&ActSouthbound::FeatureUpdateDeviceStatusRestful, this,
                                        std::cref(use_feature_profile), std::ref(device));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"FeatureUpdateDeviceStatusRestful";
    HRESULT hr = SetThreadDescription(threads[thread_count].native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    thread_count++;
  }

  // SNMP
  if (control.GetSNMP()) {
    threads[thread_count] = std::thread(&ActSouthbound::FeatureUpdateDeviceStatusSnmp, this,
                                        std::cref(use_feature_profile), std::ref(device));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"FeatureUpdateDeviceStatusSnmp";
    HRESULT hr = SetThreadDescription(threads[thread_count].native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    thread_count++;
  }

  // NewMOXACommand
  if (control.GetNewMOXACommand()) {
    threads[thread_count] = std::thread(&ActSouthbound::FeatureUpdateDeviceStatusNewMOXACommand, this,
                                        std::cref(use_feature_profile), std::ref(device));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"FeatureUpdateDeviceStatusNewMOXACommand";
    HRESULT hr = SetThreadDescription(threads[thread_count].native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    thread_count++;
  }

  // Wait thread done and remove delete it
  for (quint32 i = 0; i < thread_count; i++) {
    if (threads[i].joinable()) {
      threads[i].join();
    }
  }

  // qDebug() << __func__
  //          << QString("Update Device(%1)(%2) Device Status: %3")
  //                 .arg(device.GetMacAddress())
  //                 .arg(device.GetIpv4().GetIpAddress())
  //                 .arg(device.GetDeviceStatus().ToString())
  //                 .toStdString()
  //                 .c_str();

  // Enable SNMP
  // Conditions:
  // control.GetSNMP = true
  // && Device SNMP status = false
  // && Device Enable SNMP setting = true
  // && Device Vendor ("MOXA" or 8691) || NEW_MOXA_COMMAND status = true
  if (control.GetSNMP() && (!device.GetDeviceStatus().GetSNMPStatus()) && device.GetEnableSnmpSetting()) {
    bool moxa_vendor = (device.GetDeviceProperty().GetVendor() == ACT_VENDOR_MOXA ||
                        device.GetDeviceProperty().GetVendor() == ACT_VENDOR_ID_MOXA);

    // Get NEW_MOXA_COMMAND status
    bool new_moxa_command_status = false;
    if (!moxa_vendor) {
      if (control.GetNewMOXACommand()) {
        new_moxa_command_status = device.GetDeviceStatus().GetNewMOXACommandStatus();
      } else {
        auto tmp_device = device;
        FeatureUpdateDeviceStatusNewMOXACommand(use_feature_profile, tmp_device);
        new_moxa_command_status = tmp_device.GetDeviceStatus().GetNewMOXACommandStatus();
      }
    }

    if (moxa_vendor || new_moxa_command_status) {
      // If concurrent RESTful status can use this result to enable SNMP
      if (control.GetRESTful()) {
        act_status = FeatureEnableDeviceSnmp(use_feature_profile, device, true);
      } else {
        act_status = FeatureEnableDeviceSnmp(use_feature_profile, device, false);
      }

      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__
                    << QString("FeatureEnableDeviceSnmp() failed. Device:%1")
                           .arg(device.GetIpv4().GetIpAddress())
                           .toStdString()
                           .c_str();
      } else {
        // Re Update SNMP connect status
        FeatureUpdateDeviceStatusSnmp(use_feature_profile, device);
      }
    }
  }

  // qDebug() << __func__ << "Device Status:" << device.GetDeviceStatus().ToString().toStdString().c_str();
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::FeatureAssignDevicesMoxaVendorByBroadcastSearch(QSet<ActDevice> &devices) {
  ACT_STATUS_INIT();

  QSet<ActDevice> new_devices;

  ActFeatureSubItem search_sub_item;
  act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kAutoScan,
                                               "BroadcastSearch", "Basic", search_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Start SearchDevices
  QList<ActDevice> search_devices;
  QMap<QString, QString> mac_host_map;
  act_status = ActionBroadcastSearchDevices(search_sub_item, search_devices, mac_host_map);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ActionBroadcastSearchDevices() failed.";
    return act_status;
  }

  // Assign the Vendor to Device -> DeviceProperty -> Vendor
  QSet<ActDevice> search_devices_set(search_devices.begin(), search_devices.end());

  for (auto device : devices) {
    auto dev_ip = device.GetIpv4().GetIpAddress();
    ActDevice target_device(dev_ip);
    auto target_device_it = search_devices_set.find(target_device);
    if (target_device_it != search_devices_set.end()) {  // find
      auto dev_property = device.GetDeviceProperty();
      dev_property.SetVendor(ACT_VENDOR_MOXA);
      device.SetDeviceProperty(dev_property);
      new_devices.insert(device);
    } else {
      new_devices.insert(device);
    }
  }

  devices = new_devices;

  // for (auto device : devices) {
  //   qCritical() << __func__
  //               << QString("Device(%1) Vendor:%2")
  //                      .arg(device.GetIpv4().GetIpAddress())
  //                      .arg(device.GetDeviceProperty().GetVendor())
  //                      .toStdString()
  //                      .c_str();
  // }

  return act_status;
}

ACT_STATUS ActSouthbound::FeatureGetIdentifyDeviceInfo(const bool &use_feature_profile, const ActDevice &device,
                                                       ActIdentifyDeviceInfo &result_identify_device_info) {
  ACT_STATUS_INIT();

  QString model_name = "";
  quint32 vendor_id = 0;
  QString firmware_version = "";

  ActFeatureSubItem model_name_sub_item;
  ActFeatureSubItem vendor_id_sub_item;
  ActFeatureSubItem firmware_id_sub_item;

  if (use_feature_profile) {
    // ModelName
    act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kAutoScan, "Identify",
                                                 "ModelName", model_name_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetFeatureProfileFeatureSubItem() failed";
      return act_status;
    }

    // VendorID
    act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kAutoScan, "Identify",
                                                 "VendorID", vendor_id_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetFeatureProfileFeatureSubItem() failed";
      return act_status;
    }

    // FirmwareVersion
    act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), ActFeatureEnum::kAutoScan, "Identify",
                                                 "FirmwareVersion", firmware_id_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetFeatureProfileFeatureSubItem() failed";
      return act_status;
    }

  } else {  // use device config feature capability
    // ModelName
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "Identify", "ModelName", model_name_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
      return act_status;
    }

    // VendorID
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "Identify", "VendorID", vendor_id_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
      return act_status;
    }

    // FirmwareVersion
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kAutoScan, "Identify", "FirmwareVersion", firmware_id_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
      return act_status;
    }
  }
  // Get
  ActionGetModelName(device, model_name_sub_item, model_name);
  ActionGetVendorId(device, vendor_id_sub_item, vendor_id);
  ActionGetFirmwareVersion(device, firmware_id_sub_item, firmware_version);

  // Set result
  result_identify_device_info.SetModelName(model_name);
  result_identify_device_info.SetVendorId(vendor_id);
  result_identify_device_info.SetFirmwareVersion(firmware_version);

  return act_status;
}

ACT_STATUS ActSouthbound::FeatureIdentifyDeviceAndGetProfiles(
    const bool &use_feature_profile, ActDevice &device, const ActProfiles &profiles,
    ActDeviceProfile &result_device_profile, ActFirmwareFeatureProfile &result_firmware_feature_profile,
    QString &result_firmware) {
  ACT_STATUS_INIT();

  ActIdentifyDeviceInfo identify_device_info;

  // [feat:2396] Refactor - AutoScan performance enhance
  // Start update SNMP connect status (Include Enable SNMP)
  ActDeviceConnectStatusControl control(false, true, false,
                                        false);  // (RESTful(false), SNMP(true), NETCONF(false), NewMOXACommand(false))
  FeatureAssignDeviceStatus(use_feature_profile, device, control);

  // Check SNMP connect status
  if (!device.GetDeviceStatus().GetSNMPStatus()) {
    // Return ICMP DeviceProfile
    auto icmp_device_profile_it = profiles.GetDeviceProfiles().find(ActDeviceProfile(ACT_ICMP_DEVICE_PROFILE_ID));
    if (icmp_device_profile_it == profiles.GetDeviceProfiles().end()) {  //  not found ICMP DeviceProfile
      qDebug() << __func__
               << QString("ICMP DeviceProfile(%1) not found").arg(ACT_ICMP_DEVICE_PROFILE_ID).toStdString().c_str();
      return std::make_shared<ActStatusNotFound>(QString("ICMP DeviceProfile(%1)").arg(ACT_ICMP_DEVICE_PROFILE_ID));
    }
    result_device_profile = *icmp_device_profile_it;

    return act_status;
  }

  // Get IdentifyDeviceInfo (ModelName & VendorId & FirmwareVersion)
  FeatureGetIdentifyDeviceInfo(use_feature_profile, device, identify_device_info);
  // qDebug() << __func__
  //          << QString("Identify Device(%1) info: %2")
  //                 .arg(device.GetIpv4().GetIpAddress())
  //                 .arg(identify_device_info.ToString())
  //                 .toStdString()
  //                 .c_str();

  auto model_name = identify_device_info.GetModelName();
  auto firmware_version = identify_device_info.GetFirmwareVersion();

  // Set result Firmware Version
  result_firmware = firmware_version;

  // Find DeviceProfile at profiles->DeviceProfiles
  // First use the ModelName to find
  // -  If found DeviceProfile would return
  auto find_status = ActDeviceProfile::FindDeviceProfileByPhysicalModelName(profiles_.GetDeviceProfiles(), model_name,
                                                                            result_device_profile);
  if (IsActStatusSuccess(find_status)) {  //  found
    // [bugfix:4026] EDS-4000 price is N/A
    // Check ModelName & PhysicalModelName are consistent
    // If inconsistent would get the device's module info to match the device_profile.
    // EX: EDS series
    if (result_device_profile.GetModelName() != result_device_profile.GetPhysicalModelName()) {
      // Set Device DeviceProfileId
      ActDevice tmp_device = device;
      tmp_device.GetDeviceStatus().SetAllConnectStatus(true);  // only for get module info
      tmp_device.SetDeviceProfileId(result_device_profile.GetId());
      ActFeatureSubItem sub_item;
      auto status =
          GetDeviceFeatureSubItem(tmp_device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                  ActFeatureEnum::kAutoScan, "DeviceInformation", "ModularInfo", sub_item);
      ActDeviceModularInfo modular_info;
      if (IsActStatusSuccess(status)) {
        status = ActionGetModularInfo(tmp_device, sub_item, modular_info);
        if (IsActStatusSuccess(status)) {
          // qDebug() << "modular_info(" << tmp_device.GetIpv4().GetIpAddress()
          //          << "):" << modular_info.ToString().toStdString().c_str();
          QString build_in_power = modular_info.GetPower()[1].GetModuleName();
          qDebug() << "Build in Power(" << tmp_device.GetIpv4().GetIpAddress() << "):" << build_in_power;
          find_status = ActDeviceProfile::FindDeviceProfileByPhysicalModelNameAndBuildInPower(
              profiles_.GetDeviceProfiles(), model_name, build_in_power, result_device_profile);
          if (IsActStatusSuccess(find_status)) {
            // Find FirmwareFeatureProfile (ModelName & FirmwareVersion)
            ActFirmwareFeatureProfile fw_feat_profile;
            find_status = ActFirmwareFeatureProfile::GetFirmwareFeatureProfile(
                profiles_.GetFirmwareFeatureProfiles(), model_name, firmware_version, fw_feat_profile);
            if (IsActStatusSuccess(find_status)) {
              result_firmware_feature_profile = fw_feat_profile;
            }

            return act_status;
          }
        } else {
          qDebug() << __func__ << "ActionGetModularInfo() failed.";
        }
      }
    } else {
      // Find FirmwareFeatureProfile (ModelName & FirmwareVersion)
      ActFirmwareFeatureProfile fw_feat_profile;
      find_status = ActFirmwareFeatureProfile::GetFirmwareFeatureProfile(profiles_.GetFirmwareFeatureProfiles(),
                                                                         model_name, firmware_version, fw_feat_profile);
      if (IsActStatusSuccess(find_status)) {
        result_firmware_feature_profile = fw_feat_profile;
      }

      return act_status;
    }
  }

  // - If not found would found the internal DeviceProfile
  // Try to assign by Vendor
  if (identify_device_info.GetVendorId() == ACT_VENDOR_ID_MOXA) {  // moxa(8691)
    auto device_profile_it = profiles.GetDeviceProfiles().find(ActDeviceProfile(ACT_MOXA_DEVICE_PROFILE_ID));
    if (device_profile_it != profiles.GetDeviceProfiles().end()) {  // found
      result_device_profile = *device_profile_it;
      return act_status;
    }
  }

  // Assign Unknown
  auto device_profile_it = profiles.GetDeviceProfiles().find(ActDeviceProfile(ACT_UNKNOW_DEVICE_PROFILE_ID));
  if (device_profile_it == profiles.GetDeviceProfiles().end()) {  // not found
    qDebug() << __func__
             << QString("DeviceProfile(%1) not found").arg(ACT_UNKNOW_DEVICE_PROFILE_ID).toStdString().c_str();
    return std::make_shared<ActStatusNotFound>(QString("DeviceProfile(%1)").arg(ACT_UNKNOW_DEVICE_PROFILE_ID));
  }

  result_device_profile = *device_profile_it;

  return act_status;
}

ACT_STATUS ActSouthbound::FeatureIdentifyDeviceAndGetProfilesByProbe(
    const bool &use_feature_profile, ActDevice &device, const ActProfiles &profiles,
    ActDeviceProfile &result_device_profile, ActFirmwareFeatureProfile &result_firmware_feature_profile) {
  ACT_STATUS_INIT();

  ActIdentifyDeviceInfo identify_device_info;

  // [feat:2396] Refactor - AutoScan performance enhance
  // Start update SNMP connect status (Include Enable SNMP)
  ActDeviceConnectStatusControl control(false, true, false,
                                        false);  // (RESTful(false), SNMP(true), NETCONF(false), NewMOXACommand(false))
  FeatureAssignDeviceStatus(use_feature_profile, device, control);

  // Check SNMP connect status
  if (!device.GetDeviceStatus().GetSNMPStatus()) {
    // Return ICMP DeviceProfile
    auto icmp_device_profile_it = profiles.GetDeviceProfiles().find(ActDeviceProfile(ACT_ICMP_DEVICE_PROFILE_ID));
    if (icmp_device_profile_it == profiles.GetDeviceProfiles().end()) {  //  not found ICMP DeviceProfile
      qCritical() << __func__
                  << QString("ICMP DeviceProfile(%1) not found").arg(ACT_ICMP_DEVICE_PROFILE_ID).toStdString().c_str();
      return std::make_shared<ActStatusNotFound>(QString("ICMP DeviceProfile(%1)").arg(ACT_ICMP_DEVICE_PROFILE_ID));
    }
    result_device_profile = *icmp_device_profile_it;

    return act_status;
  }

  // Get IdentifyDeviceInfo (ModelName & VendorId & FirmwareVersion)
  FeatureGetIdentifyDeviceInfo(use_feature_profile, device, identify_device_info);
  qDebug() << __func__
           << QString("Identify Device(%1) info: %2")
                  .arg(device.GetIpv4().GetIpAddress())
                  .arg(identify_device_info.ToString())
                  .toStdString()
                  .c_str();

  auto model_name = identify_device_info.GetModelName();
  auto firmware_version = identify_device_info.GetFirmwareVersion();
  // Find DeviceProfile at profiles->DeviceProfiles
  // First use the ModelName to find
  // -  If found DeviceProfile would return
  auto device_profile_it = profiles.GetDeviceProfiles().find(ActDeviceProfile(model_name));
  if (device_profile_it != profiles.GetDeviceProfiles().end()) {  //  found
    result_device_profile = *device_profile_it;

    // Find FirmwareFeatureProfile (ModelName & FirmwareVersion)
    auto firmware_feature_it =
        std::find_if(profiles.GetFirmwareFeatureProfiles().begin(), profiles.GetFirmwareFeatureProfiles().end(),
                     [&model_name, &firmware_version](const ActFirmwareFeatureProfile &fw_feat_profile) {
                       return (fw_feat_profile.GetModelName() == model_name) &&
                              fw_feat_profile.GetFirmwareVersions().contains(firmware_version);
                     });
    if (firmware_feature_it != profiles.GetFirmwareFeatureProfiles().end()) {  // find
      result_firmware_feature_profile = *firmware_feature_it;
    }
    return act_status;
  }

  // - If not found would use AutoProbe to find
  // AutoProbe: Use default_profiles
  // Would find DefaultDeviceProfile and generate DeviceProfile(From DefaultDeviceProfile)

  // If not found ModelName would return failed
  if (model_name.isEmpty()) {
    qCritical() << __func__
                << QString("Device(%1) ModelName is empty").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();
    return std::make_shared<ActStatusGetDeviceDataFailed>("ModelName", device.GetIpv4().GetIpAddress());
  }

  ActDeviceProfile default_device_profile;
  QString vendor_id_str = QString::number(identify_device_info.GetVendorId());
  // Find key = model_name
  auto default_device_profile_it = profiles.GetDefaultDeviceProfiles().find(ActDeviceProfile(model_name));
  if (default_device_profile_it == profiles.GetDefaultDeviceProfiles().end()) {  // not found

    // Try to found default by VendorId.
    // Use VenderId to find ModelName as the VendorId DeviceProfile
    auto found_vendor_profile_bool = false;
    default_device_profile_it = profiles.GetDefaultDeviceProfiles().find(ActDeviceProfile(vendor_id_str));
    if (default_device_profile_it != profiles.GetDefaultDeviceProfiles().end()) {  // found
      // Check DeviceProfile's Vendor same as VendorId
      if (default_device_profile_it->GetVendor() == vendor_id_str) {
        found_vendor_profile_bool = true;
      }
    }

    // Assign Default (Not match any vendor)
    if (!found_vendor_profile_bool) {
      default_device_profile_it =
          profiles.GetDefaultDeviceProfiles().find(ActDeviceProfile(ACT_MOXA_DEVICE_PROFILE_ID));
      if (default_device_profile_it == profiles.GetDefaultDeviceProfiles().end()) {  // not found
        qCritical()
            << __func__
            << QString("DefaultDeviceProfile(%1) not found").arg(ACT_MOXA_DEVICE_PROFILE_ID).toStdString().c_str();
        return std::make_shared<ActStatusNotFound>(
            QString("DefaultDeviceProfile(%1).").arg(ACT_MOXA_DEVICE_PROFILE_ID));
      }
    }
  }
  default_device_profile = *default_device_profile_it;
  qDebug() << __func__
           << QString("Found DefaultDeviceProfile(%1) by ModelName(%2)")
                  .arg(default_device_profile.GetId())
                  .arg(default_device_profile.GetModelName())
                  .toStdString()
                  .c_str();

  default_device_profile.SetModelName(model_name);

  // Set Vendor ID & ICON Name
  if (identify_device_info.GetVendorId() == ACT_VENDOR_ID_MOXA) {  // MOXA vendor
    default_device_profile.SetVendor(ACT_VENDOR_MOXA);

    // Set moxa icon when it use the default device profile
    if (default_device_profile.GetId() == ACT_MOXA_DEVICE_PROFILE_ID) {
      default_device_profile.SetIconName(ACT_MOXA_DEVICE_ICON_NAME);
    }
  } else {  // other use the vendor id
    default_device_profile.SetVendor(vendor_id_str);
  }
  qDebug()
      << __func__
      << QString("Set default DeviceProfile Vendor: %1").arg(default_device_profile.GetVendor()).toStdString().c_str();

  default_device_profile.SetId(-1);
  result_device_profile = default_device_profile;

  return act_status;
}
