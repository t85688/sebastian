#include "act_deploy.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

ACT_STATUS act::deploy::ActDeploy::Reboot(const ActDevice &device) {
  ACT_STATUS_INIT();

  // Get Sub-item
  ActFeatureSubItem feature_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kOperation, "Reboot", "Basic", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
    return act_status;
  }

  act_status = southbound_.ActionReboot(device, feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Reboot failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::deploy::ActDeploy::FactoryDefault(const ActDevice &device) {
  ACT_STATUS_INIT();

  // Get Sub-item
  ActFeatureSubItem feature_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kOperation, "FactoryDefault", "Basic", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
    return act_status;
  }

  act_status = southbound_.ActionFactoryDefault(device, feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "FactoryDefault failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::deploy::ActDeploy::ImportConfig(const ActDevice &device, const QString &file_path) {
  ACT_STATUS_INIT();

  // Get Sub-item
  ActFeatureSubItem feature_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kOperation, "ImportExport", "Basic", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
    return act_status;
  }

  act_status = southbound_.ActionImportConfig(device, feature_sub_item, file_path);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ImportConfig failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::deploy::ActDeploy::FirmwareUpgrade(const ActDevice &device, const QString &firmware_name) {
  ACT_STATUS_INIT();

  // Get Sub-item
  ActFeatureSubItem feature_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kOperation, "FirmwareUpgrade", "Basic", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
    return act_status;
  }

  // FirmwareUpgrade device
  auto file_path = QString("%1/%2").arg(ACT_FIRMWARE_FILE_FOLDER).arg(firmware_name);
  act_status = southbound_.ActionFirmwareUpgrade(device, feature_sub_item, file_path);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "FirmwareUpgrade failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::deploy::ActDeploy::ConfigureNetconf(const ActDevice &device, const ActDeployControl &deploy_control,
                                                    const ActDeviceConfig &device_config) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  QSet<QString> sub_item_key_set;
  // CB
  if (deploy_control.GetIeee802Dot1Cb()) {
    auto &ieee_802_1cb_table = device_config.GetCbTables();
    if (ieee_802_1cb_table.contains(device.GetId())) {
      // feature group disable would skip (skip default configuration)
      if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTSN().GetIEEE802Dot1CB()) {
        ActFeatureSubItem feature_sub_item;
        act_status =
            GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                    ActFeatureEnum::kConfiguration, "TSN", "IEEE802Dot1CB", feature_sub_item);
        if (IsActStatusSuccess(act_status)) {
          // action_list.append(feature_sub_item);
          sub_item_key_set.insert("IEEE802Dot1CB");
        }

      } else {  // skip
        qDebug() << __func__
                 << QString("Skip config the CB. Device: %1(%2)")
                        .arg(device.GetIpv4().GetIpAddress())
                        .arg(device.GetId())
                        .toStdString()
                        .c_str();
      }
    }
  }

  // Qbv
  if (deploy_control.GetGcl()) {
    auto &gcl_tables = device_config.GetGCLTables();
    if (gcl_tables.contains(device.GetId())) {
      // feature group disable would skip (skip default configuration)
      if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTSN().GetIEEE802Dot1Qbv()) {
        // Check AdminControlListLength
        act_status = CheckGclAdminControlListLength(device, gcl_tables[device.GetId()]);
        if (IsActStatusSuccess(act_status)) {
          ActFeatureSubItem feature_sub_item;
          act_status =
              GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                      ActFeatureEnum::kConfiguration, "TSN", "IEEE802Dot1Qbv", feature_sub_item);
          if (IsActStatusSuccess(act_status)) {
            // action_list.append(feature_sub_item);
            sub_item_key_set.insert("IEEE802Dot1Qbv");
          }
        }

      } else {  // skip
        qDebug() << __func__
                 << QString("Skip config the Qbv. Device: %1(%2)")
                        .arg(device.GetIpv4().GetIpAddress())
                        .arg(device.GetId())
                        .toStdString()
                        .c_str();
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS act::deploy::ActDeploy::DeployNetworkSetting(const ActDevice &device,
                                                        const ActNetworkSettingTable &network_setting_table,
                                                        bool from_broadcast_search) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  // Set ArpTable
  if (from_broadcast_search) {  // has mac
    // Check Map has device's mac
    if (!mac_host_map_.contains(device.GetMacAddress())) {  // hasn't
      qCritical()
          << __func__
          << QString("Device's(%1) MAC(%2) not found in mac_host_map").arg(device.GetId()).arg(device.GetMacAddress());

      return std::make_shared<ActStatusNotFound>(QString("MAC(%1) in mac_host_map").arg(device.GetMacAddress()));
    }

    QString host = mac_host_map_[device.GetMacAddress()];
    auto msg = QString("SetLocalhostArpTable: device: %1(%2, %3), host: %4")
                   .arg(device.GetId())
                   .arg(device.GetIpv4().GetIpAddress())
                   .arg(device.GetMacAddress())
                   .arg(host);
    qDebug() << __func__ << msg.toStdString().c_str();
    act_status = southbound_.SetLocalhostArpTable(device, host);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set NetworkSetting
  // Get Sub-item
  ActFeatureSubItem feature_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "NetworkSetting", "Basic", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
    return act_status;
  }

  act_status = southbound_.ActionSetNetworkSetting(device, feature_sub_item, network_setting_table);
  // Delete Arp entry
  if (from_broadcast_search) {
    auto act_status_arp = southbound_.DeleteLocalhostArpEntry(device);
    if (!IsActStatusSuccess(act_status_arp)) {
      qCritical() << __func__ << "DeleteLocalhostArpEntry() failed.";
    }
  }

  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "NetworkSetting failed.";
    return act_status;
  }

  act_status = southbound_.ClearArpCache();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ClearArpCache() failed.";
    return act_status;
  }

  if (deployer_stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Check icmp status
  SLEEP_MS(1000);
  act_status = southbound_.PingIpAddress(network_setting_table.GetIpAddress(), ACT_PING_REPEAT_TIMES);
  if (!IsActStatusSuccess(act_status)) {  // not alive
    qCritical() << __func__ << "Check the New NetworkSetting connect failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::deploy::ActDeploy::DeployMappingDeviceIpSetting(
    const ActDevice &device, const ActMappingDeviceIpSettingTable &ip_setting_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  // Generate new device for online_device(IP & MAC)
  ActDevice online_device(device);
  online_device.GetIpv4().SetIpAddress(ip_setting_table.GetOnlineIP());
  online_device.SetMacAddress(ip_setting_table.GetMacAddress());
  online_device.GetDeviceStatus().SetAllConnectStatus(true);

  // Generate NetworkSettingTable
  ActNetworkSettingTable network_setting_table(online_device.GetId());
  network_setting_table.SetIpAddress(ip_setting_table.GetOfflineIP());
  network_setting_table.SetGateway(ip_setting_table.GetGateway());
  network_setting_table.SetSubnetMask(ip_setting_table.GetSubnetMask());
  network_setting_table.SetDNS1(ip_setting_table.GetDNS1());
  network_setting_table.SetDNS2(ip_setting_table.GetDNS2());

  // Set ArpTable
  // Check Map has device's mac
  if (!mac_host_map_.contains(online_device.GetMacAddress())) {  // hasn't
    qCritical() << __func__
                << QString("Device's(%1) MAC(%2) not found in mac_host_map")
                       .arg(online_device.GetId())
                       .arg(online_device.GetMacAddress());

    return std::make_shared<ActStatusNotFound>(QString("MAC(%1) in mac_host_map").arg(online_device.GetMacAddress()));
  }

  QString host = mac_host_map_[online_device.GetMacAddress()];
  act_status = southbound_.SetLocalhostArpTable(online_device, host);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Set NetworkSetting
  // Get Sub-item
  ActFeatureSubItem feature_sub_item;
  act_status =
      GetDeviceFeatureSubItem(online_device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                              ActFeatureEnum::kConfiguration, "NetworkSetting", "Basic", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
    auto act_status_arp = southbound_.DeleteLocalhostArpEntry(online_device);
    if (!IsActStatusSuccess(act_status_arp)) {
      qCritical() << __func__ << "DeleteLocalhostArpEntry() failed.";
    }

    return act_status;
  }

  act_status = southbound_.ActionSetNetworkSetting(online_device, feature_sub_item, network_setting_table);

  // Delete Arp entry
  auto act_status_arp = southbound_.DeleteLocalhostArpEntry(online_device);
  if (!IsActStatusSuccess(act_status_arp)) {
    qCritical() << __func__ << "DeleteLocalhostArpEntry() failed.";
  }

  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "NetworkSetting failed.";
    return act_status;
  }

  return act_status;
}
