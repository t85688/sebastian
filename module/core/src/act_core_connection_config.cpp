#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::CheckDeviceAccount(const ActDeviceAccount &account, QString item_name) {
  ACT_STATUS_INIT();

  // The length should be 4 ~ 32 characters.
  if (account.GetUsername().length() < ACT_USERNAME_LENGTH_MIN ||
      account.GetUsername().length() > ACT_USERNAME_LENGTH_MAX) {
    QString error_msg = QString("%1 - The length of Account username should be %2 ~ %3 characters")
                            .arg(item_name)
                            .arg(ACT_USERNAME_LENGTH_MIN)
                            .arg(ACT_USERNAME_LENGTH_MAX);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // The length should be 4 ~ 63 characters.
  if (account.GetPassword().length() < ACT_PASSWORD_LENGTH_MIN ||
      account.GetPassword().length() > ACT_PASSWORD_LENGTH_MAX) {
    QString error_msg = QString("%1 - The length of Account password should be %2 ~ %3 characters")
                            .arg(item_name)
                            .arg(ACT_PASSWORD_LENGTH_MIN)
                            .arg(ACT_PASSWORD_LENGTH_MAX);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

ACT_STATUS ActCore::CheckNetconfConfiguration(const ActNetconfConfiguration &netconf_config, QString item_name) {
  ACT_STATUS_INIT();

  // Check port number
  if (netconf_config.GetNetconfOverSSH().GetSSHPort() != ACT_DEFAULT_NETCONF_SSH_PORT &&
      (netconf_config.GetNetconfOverSSH().GetSSHPort() < ACT_NETCONF_SSH_PORT_MIN ||
       netconf_config.GetNetconfOverSSH().GetSSHPort() > ACT_NETCONF_SSH_PORT_MAX)) {
    QString error_msg = QString("%1 - The ssh port number of NETCONF should be %2 or located in %3 ~ %4")
                            .arg(item_name)
                            .arg(ACT_DEFAULT_NETCONF_SSH_PORT)
                            .arg(ACT_NETCONF_SSH_PORT_MIN)
                            .arg(ACT_NETCONF_SSH_PORT_MAX);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

ACT_STATUS ActCore::CheckRestfulConfiguration(const ActRestfulConfiguration &restful_config, QString item_name) {
  ACT_STATUS_INIT();

  // Check port number
  if ((restful_config.GetPort() != ACT_DEFAULT_RESTFUL_HTTP_PORT) &&
      (restful_config.GetPort() != ACT_DEFAULT_RESTFUL_HTTPS_PORT) &&
      (restful_config.GetPort() < ACT_RESTFUL_PORT_MIN || restful_config.GetPort() > ACT_RESTFUL_PORT_MAX)) {
    QString error_msg = QString("%1 - The port number of RESTful should be %2 or %3 or located in %4 ~ %5")
                            .arg(item_name)
                            .arg(ACT_DEFAULT_RESTFUL_HTTP_PORT)
                            .arg(ACT_DEFAULT_RESTFUL_HTTPS_PORT)
                            .arg(ACT_RESTFUL_PORT_MIN)
                            .arg(ACT_RESTFUL_PORT_MAX);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

ACT_STATUS ActCore::CheckSnmpConfiguration(const ActSnmpConfiguration &snmp_config, QString item_name) {
  ACT_STATUS_INIT();

  // Check port number
  if (snmp_config.GetPort() != ACT_DEFAULT_SNMP_PORT &&
      (snmp_config.GetPort() < ACT_SNMP_PORT_MIN || snmp_config.GetPort() > ACT_SNMP_PORT_MAX)) {
    QString error_msg = QString("%1 - The port number of SNMP should be %2 or located in %3 ~ %4")
                            .arg(item_name)
                            .arg(ACT_DEFAULT_SNMP_PORT)
                            .arg(ACT_SNMP_PORT_MIN)
                            .arg(ACT_SNMP_PORT_MAX);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check SNMPv1, SNMPv2c configuration
  if (snmp_config.GetVersion() == ActSnmpVersionEnum::kV2c || snmp_config.GetVersion() == ActSnmpVersionEnum::kV1) {
    // ReadCommunity length should be 1 ~ 32 characters.
    if (snmp_config.GetReadCommunity().length() < ACT_SNMP_COMMUNITY_MIN ||
        snmp_config.GetReadCommunity().length() > ACT_SNMP_COMMUNITY_MAX) {
      QString error_msg = QString("%1 - The length of SNMP read community should be %2 ~ %3 characters")
                              .arg(item_name)
                              .arg(ACT_SNMP_COMMUNITY_MIN)
                              .arg(ACT_SNMP_COMMUNITY_MAX);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    // WriteCommunity length should be 1 ~ 32 characters.
    if (snmp_config.GetWriteCommunity().length() < ACT_SNMP_COMMUNITY_MIN ||
        snmp_config.GetWriteCommunity().length() > ACT_SNMP_COMMUNITY_MAX) {
      QString error_msg = QString("%1 - The length of SNMP write community should be %2 ~ %3 characters")
                              .arg(item_name)
                              .arg(ACT_SNMP_COMMUNITY_MIN)
                              .arg(ACT_SNMP_COMMUNITY_MAX);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Check SNMPv3 configuration
  if (snmp_config.GetVersion() == ActSnmpVersionEnum::kV3) {
    // Username length should be 4 ~ 32 characters.
    if (snmp_config.GetUsername().length() < ACT_USERNAME_LENGTH_MIN ||
        snmp_config.GetUsername().length() > ACT_USERNAME_LENGTH_MAX) {
      QString error_msg = QString("%1 - The length of SNMP username should be %2 ~ %3 characters")
                              .arg(item_name)
                              .arg(ACT_USERNAME_LENGTH_MIN)
                              .arg(ACT_USERNAME_LENGTH_MAX);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    // Check AuthenticationPassword
    if (snmp_config.GetAuthenticationType() != ActSnmpAuthenticationTypeEnum::kNone) {
      // The length should be 8 ~ 64 characters.
      if (snmp_config.GetAuthenticationPassword().length() < ACT_SNMP_PASSWORD_LENGTH_MIN ||
          snmp_config.GetAuthenticationPassword().length() > ACT_SNMP_PASSWORD_LENGTH_MAX) {
        QString error_msg = QString("%1 - The length of SNMP authentication password should be %2 ~ %3 characters")
                                .arg(item_name)
                                .arg(ACT_SNMP_PASSWORD_LENGTH_MIN)
                                .arg(ACT_SNMP_PASSWORD_LENGTH_MAX);
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }

    // Check DataEncryptionKey
    if (snmp_config.GetDataEncryptionType() != ActSnmpDataEncryptionTypeEnum::kNone) {
      // The length should be 8 ~ 64 characters.
      if (snmp_config.GetDataEncryptionKey().length() < ACT_SNMP_PASSWORD_LENGTH_MIN ||
          snmp_config.GetDataEncryptionKey().length() > ACT_SNMP_PASSWORD_LENGTH_MAX) {
        QString error_msg = QString("%1 - The length of SNMP data encryption key should be %2 ~ %3 characters")
                                .arg(item_name)
                                .arg(ACT_SNMP_PASSWORD_LENGTH_MIN)
                                .arg(ACT_SNMP_PASSWORD_LENGTH_MAX);
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }
  }

  return act_status;
}

ACT_STATUS ActCore::CheckSnmpTrapConfiguration(const ActSnmpTrapConfiguration &snmp_trap_config, QString item_name) {
  ACT_STATUS_INIT();

  // Check port number
  if (snmp_trap_config.GetPort() != ACT_DEFAULT_SNMP_TRAP_PORT &&
      (snmp_trap_config.GetPort() < ACT_SNMP_PORT_MIN || snmp_trap_config.GetPort() > ACT_SNMP_PORT_MAX)) {
    QString error_msg = QString("%1 - The port number of SNMP should be %2 or located in %3 ~ %4")
                            .arg(item_name)
                            .arg(ACT_DEFAULT_SNMP_TRAP_PORT)
                            .arg(ACT_SNMP_PORT_MIN)
                            .arg(ACT_SNMP_PORT_MAX);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check SNMPv1, SNMPv2c configuration
  if (snmp_trap_config.GetVersion() == ActSnmpVersionEnum::kV2c ||
      snmp_trap_config.GetVersion() == ActSnmpVersionEnum::kV1) {
    // TrapCommunity length should be 1 ~ 32 characters.
    if (snmp_trap_config.GetTrapCommunity().length() < ACT_SNMP_COMMUNITY_MIN ||
        snmp_trap_config.GetTrapCommunity().length() > ACT_SNMP_COMMUNITY_MAX) {
      QString error_msg = QString("%1 - The length of SNMP trap community should be %2 ~ %3 characters")
                              .arg(item_name)
                              .arg(ACT_SNMP_COMMUNITY_MIN)
                              .arg(ACT_SNMP_COMMUNITY_MAX);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Check SNMPv3 configuration
  if (snmp_trap_config.GetVersion() == ActSnmpVersionEnum::kV3) {
    // Username length should be 4 ~ 32 characters.
    if (snmp_trap_config.GetUsername().length() < ACT_USERNAME_LENGTH_MIN ||
        snmp_trap_config.GetUsername().length() > ACT_USERNAME_LENGTH_MAX) {
      QString error_msg = QString("%1 - The length of SNMP username should be %2 ~ %3 characters")
                              .arg(item_name)
                              .arg(ACT_USERNAME_LENGTH_MIN)
                              .arg(ACT_USERNAME_LENGTH_MAX);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    // Check AuthenticationPassword
    if (snmp_trap_config.GetAuthenticationType() != ActSnmpAuthenticationTypeEnum::kNone) {
      // The length should be 8 ~ 64 characters.
      if (snmp_trap_config.GetAuthenticationPassword().length() < ACT_SNMP_PASSWORD_LENGTH_MIN ||
          snmp_trap_config.GetAuthenticationPassword().length() > ACT_SNMP_PASSWORD_LENGTH_MAX) {
        QString error_msg = QString("%1 - The length of SNMP authentication password should be %2 ~ %3 characters")
                                .arg(item_name)
                                .arg(ACT_SNMP_PASSWORD_LENGTH_MIN)
                                .arg(ACT_SNMP_PASSWORD_LENGTH_MAX);
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }

    // Check DataEncryptionKey
    if (snmp_trap_config.GetDataEncryptionType() != ActSnmpDataEncryptionTypeEnum::kNone) {
      // The length should be 8 ~ 64 characters.
      if (snmp_trap_config.GetDataEncryptionKey().length() < ACT_SNMP_PASSWORD_LENGTH_MIN ||
          snmp_trap_config.GetDataEncryptionKey().length() > ACT_SNMP_PASSWORD_LENGTH_MAX) {
        QString error_msg = QString("%1 - The length of SNMP data encryption key should be %2 ~ %3 characters")
                                .arg(item_name)
                                .arg(ACT_SNMP_PASSWORD_LENGTH_MIN)
                                .arg(ACT_SNMP_PASSWORD_LENGTH_MAX);
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }
  }

  return act_status;
}

ACT_STATUS ActCore::HandleDeviceAccount(ActDeviceAccount &target_account, const ActDeviceAccount &old_account,
                                        const ActDeviceAccount &default_account) {
  ACT_STATUS_INIT();

  if (target_account.GetDefaultSetting()) {
    // Use project_setting
    ActDeviceAccount new_config(default_account);
    new_config.SetDefaultSetting(true);
    target_account = new_config;
  } else {
    // Check Password
    if (target_account.GetPassword().length() == 0) {
      // Keep old_setting
      target_account = old_account;
    }
  }

  return act_status;
}

ACT_STATUS ActCore::HandleNetconfConfiguration(ActNetconfConfiguration &target_netconf_config,
                                               const ActNetconfConfiguration &default_netconf_config) {
  ACT_STATUS_INIT();

  if (target_netconf_config.GetDefaultSetting()) {
    // Use project_setting
    ActNetconfConfiguration new_config(default_netconf_config);
    new_config.SetDefaultSetting(true);
    target_netconf_config = new_config;
  }

  return act_status;
}

ACT_STATUS ActCore::HandleRestfulConfiguration(ActRestfulConfiguration &target_restful_config,
                                               const ActRestfulConfiguration &default_restful_config) {
  ACT_STATUS_INIT();

  if (target_restful_config.GetDefaultSetting()) {
    // Use project_setting
    ActRestfulConfiguration new_config(default_restful_config);
    new_config.SetDefaultSetting(true);
    target_restful_config = new_config;
  }

  return act_status;
}

ACT_STATUS ActCore::HandleSnmpConfiguration(ActSnmpConfiguration &target_snmp_config,
                                            const ActSnmpConfiguration &old_snmp_config,
                                            const ActSnmpConfiguration &default_snmp_config) {
  ACT_STATUS_INIT();

  // SNMPv1 & SNMPv2c
  if (target_snmp_config.GetVersion() == ActSnmpVersionEnum::kV2c ||
      target_snmp_config.GetVersion() == ActSnmpVersionEnum::kV1) {
    if (target_snmp_config.GetDefaultSetting()) {
      // Use project_setting
      ActSnmpConfiguration new_config(default_snmp_config);
      new_config.SetDefaultSetting(true);
      target_snmp_config = new_config;
    } else {
      // Check Password
      if (target_snmp_config.GetReadCommunity().length() == 0) {
        // Keep old_setting
        target_snmp_config = old_snmp_config;
      }
    }
  }

  // SNMPv3
  if (target_snmp_config.GetVersion() == ActSnmpVersionEnum::kV3) {
    if (target_snmp_config.GetDefaultSetting()) {
      // Use project_setting
      ActSnmpConfiguration new_config(default_snmp_config);
      new_config.SetDefaultSetting(true);
      target_snmp_config = new_config;
    } else {
      // Check AuthenticationPassword when the AuthenticationType not as none
      if (target_snmp_config.GetAuthenticationType() != ActSnmpAuthenticationTypeEnum::kNone) {
        if (target_snmp_config.GetAuthenticationPassword().length() == 0) {
          // Keep old_setting
          target_snmp_config = old_snmp_config;
        }
      }
    }
  }

  return act_status;
}

ACT_STATUS ActCore::SyncDeviceAccountDefault(const ActDeviceAccount &account, ActProject &project) {
  ACT_STATUS_INIT();

  // Devices
  QSet<ActDevice> new_devices;
  for (auto dev : project.GetDevices()) {
    if (dev.GetAccount().GetDefaultSetting()) {
      dev.SetAccount(account);
      dev.GetAccount().SetDefaultSetting(true);

      // Send device update msg to temp
      InsertDeviceMsgToNotificationTmp(
          ActDevicePatchUpdateMsg(ActPatchUpdateActionEnum::kUpdate, project.GetId(), dev, true));
    }
    new_devices.insert(dev);
  }
  project.SetDevices(new_devices);

  // ComputeResult > Devices
  QSet<ActDevice> new_result_devices;
  for (auto dev : project.GetComputedResult().GetDevices()) {
    if (dev.GetAccount().GetDefaultSetting()) {
      dev.SetAccount(account);
      dev.GetAccount().SetDefaultSetting(true);
    }
    new_result_devices.insert(dev);
  }
  project.GetComputedResult().SetDevices(new_result_devices);

  // ProjectSetting > ScanIpRange
  QList<ActScanIpRangeEntry> new_scan_ip_range_entries;
  for (auto scan_ip_range_entry : project.GetProjectSetting().GetScanIpRanges()) {
    if (scan_ip_range_entry.GetAccount().GetDefaultSetting()) {
      scan_ip_range_entry.SetAccount(account);
      scan_ip_range_entry.GetAccount().SetDefaultSetting(true);
    }
    new_scan_ip_range_entries.append(scan_ip_range_entry);
  }
  project.GetProjectSetting().SetScanIpRanges(new_scan_ip_range_entries);

  return act_status;
}

ACT_STATUS ActCore::SyncNetconfDefaultConfiguration(const ActNetconfConfiguration &netconf_config,
                                                    ActProject &project) {
  ACT_STATUS_INIT();

  // Devices
  QSet<ActDevice> new_devices;
  for (auto dev : project.GetDevices()) {
    if (dev.GetNetconfConfiguration().GetDefaultSetting()) {
      dev.SetNetconfConfiguration(netconf_config);
      dev.GetNetconfConfiguration().SetDefaultSetting(true);

      // Send device update msg to temp
      InsertDeviceMsgToNotificationTmp(
          ActDevicePatchUpdateMsg(ActPatchUpdateActionEnum::kUpdate, project.GetId(), dev, true));
    }
    new_devices.insert(dev);
  }
  project.SetDevices(new_devices);

  // ComputeResult > Devices
  QSet<ActDevice> new_result_devices;
  for (auto dev : project.GetComputedResult().GetDevices()) {
    if (dev.GetNetconfConfiguration().GetDefaultSetting()) {
      dev.SetNetconfConfiguration(netconf_config);
      dev.GetNetconfConfiguration().SetDefaultSetting(true);
    }
    new_result_devices.insert(dev);
  }
  project.GetComputedResult().SetDevices(new_result_devices);

  // ProjectSetting > ScanIpRange
  QList<ActScanIpRangeEntry> new_scan_ip_range_entries;
  for (auto scan_ip_range_entry : project.GetProjectSetting().GetScanIpRanges()) {
    if (scan_ip_range_entry.GetNetconfConfiguration().GetDefaultSetting()) {
      scan_ip_range_entry.SetNetconfConfiguration(netconf_config);
      scan_ip_range_entry.GetNetconfConfiguration().SetDefaultSetting(true);
    }
    new_scan_ip_range_entries.append(scan_ip_range_entry);
  }
  project.GetProjectSetting().SetScanIpRanges(new_scan_ip_range_entries);

  return act_status;
}

ACT_STATUS ActCore::SyncRestfulDefaultConfiguration(const ActRestfulConfiguration &restful_config,
                                                    ActProject &project) {
  ACT_STATUS_INIT();

  // Devices
  QSet<ActDevice> new_devices;
  for (auto dev : project.GetDevices()) {
    if (dev.GetRestfulConfiguration().GetDefaultSetting()) {
      dev.SetRestfulConfiguration(restful_config);
      dev.GetRestfulConfiguration().SetDefaultSetting(true);

      // Send device update msg to temp
      InsertDeviceMsgToNotificationTmp(
          ActDevicePatchUpdateMsg(ActPatchUpdateActionEnum::kUpdate, project.GetId(), dev, true));
    }
    new_devices.insert(dev);
  }
  project.SetDevices(new_devices);

  // ComputeResult > Devices
  QSet<ActDevice> new_result_devices;
  for (auto dev : project.GetComputedResult().GetDevices()) {
    if (dev.GetRestfulConfiguration().GetDefaultSetting()) {
      dev.SetRestfulConfiguration(restful_config);
      dev.GetRestfulConfiguration().SetDefaultSetting(true);
    }
    new_result_devices.insert(dev);
  }
  project.GetComputedResult().SetDevices(new_result_devices);

  // ProjectSetting > ScanIpRange
  QList<ActScanIpRangeEntry> new_scan_ip_range_entries;
  for (auto scan_ip_range_entry : project.GetProjectSetting().GetScanIpRanges()) {
    if (scan_ip_range_entry.GetRestfulConfiguration().GetDefaultSetting()) {
      scan_ip_range_entry.SetRestfulConfiguration(restful_config);
      scan_ip_range_entry.GetRestfulConfiguration().SetDefaultSetting(true);
    }
    new_scan_ip_range_entries.append(scan_ip_range_entry);
  }
  project.GetProjectSetting().SetScanIpRanges(new_scan_ip_range_entries);

  return act_status;
}

ACT_STATUS ActCore::SyncSnmpDefaultConfiguration(const ActSnmpConfiguration &snmp_config, ActProject &project) {
  ACT_STATUS_INIT();

  // Devices
  QSet<ActDevice> new_devices;
  for (auto dev : project.GetDevices()) {
    if (dev.GetSnmpConfiguration().GetDefaultSetting()) {
      dev.SetSnmpConfiguration(snmp_config);
      dev.GetSnmpConfiguration().SetDefaultSetting(true);

      // Send device update msg to temp
      InsertDeviceMsgToNotificationTmp(
          ActDevicePatchUpdateMsg(ActPatchUpdateActionEnum::kUpdate, project.GetId(), dev, true));
    }
    new_devices.insert(dev);
  }
  project.SetDevices(new_devices);

  // ComputeResult > Devices
  QSet<ActDevice> new_result_devices;
  for (auto dev : project.GetComputedResult().GetDevices()) {
    if (dev.GetSnmpConfiguration().GetDefaultSetting()) {
      dev.SetSnmpConfiguration(snmp_config);
      dev.GetSnmpConfiguration().SetDefaultSetting(true);
    }
    new_result_devices.insert(dev);
  }
  project.GetComputedResult().SetDevices(new_result_devices);

  // ProjectSetting > ScanIpRange
  QList<ActScanIpRangeEntry> new_scan_ip_range_entries;
  for (auto scan_ip_range_entry : project.GetProjectSetting().GetScanIpRanges()) {
    if (scan_ip_range_entry.GetSnmpConfiguration().GetDefaultSetting()) {
      scan_ip_range_entry.SetSnmpConfiguration(snmp_config);
      scan_ip_range_entry.GetSnmpConfiguration().SetDefaultSetting(true);
    }
    new_scan_ip_range_entries.append(scan_ip_range_entry);
  }
  project.GetProjectSetting().SetScanIpRanges(new_scan_ip_range_entries);

  return act_status;
}

}  // namespace core
}  // namespace act