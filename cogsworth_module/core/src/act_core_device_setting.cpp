#include "act_core.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::GetDeviceInformation(qint64 &project_id, qint64 &device_id,
                                         ActDeviceInformation &device_information, bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActInformationSettingTable> &information_setting_tables =
      project.GetDeviceConfig().GetInformationSettingTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetId() != device_id) {
      continue;
    } else if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      QString error_msg = QString("Device %1 is not a switch").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(error_msg);
    }

    ActInformationSettingTable information_setting(device);
    if (information_setting_tables.contains(device.GetId())) {
      information_setting = information_setting_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetInformationSettingTables().isEmpty()) {
        information_setting = device_profile.GetDefaultDeviceConfig().GetInformationSettingTables().first();
      }
      information_setting_tables[device.GetId()] = information_setting;  // insert config to tables
    }

    device_information = ActDeviceInformation(device);
    device_information.SetDeviceName(information_setting.GetDeviceName());
    device_information.SetLocation(information_setting.GetLocation());
    device_information.SetDescription(information_setting.GetDescription());
    device_information.SetContactInformation(information_setting.GetContactInformation());
    return ACT_STATUS_SUCCESS;
  }
  return std::make_shared<ActStatusNotFound>(QString("Device %1 in project is not found").arg(device_id));
}

ACT_STATUS ActCore::GetDeviceInformations(qint64 &project_id, ActDeviceInformationList &device_information_list,
                                          bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActDeviceInformation> &device_information_list_ = device_information_list.GetDeviceInformationList();
  QMap<qint64, ActInformationSettingTable> &information_setting_tables =
      project.GetDeviceConfig().GetInformationSettingTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }

    ActInformationSettingTable information_setting(device);
    if (information_setting_tables.contains(device.GetId())) {
      information_setting = information_setting_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetInformationSettingTables().isEmpty()) {
        information_setting = device_profile.GetDefaultDeviceConfig().GetInformationSettingTables().first();
      }
      information_setting_tables[device.GetId()] = information_setting;  // insert config to tables
    }

    ActDeviceInformation device_information(device);
    device_information.SetDeviceName(information_setting.GetDeviceName());
    device_information.SetLocation(information_setting.GetLocation());
    device_information.SetDescription(information_setting.GetDescription());
    device_information.SetContactInformation(information_setting.GetContactInformation());
    device_information_list_.append(device_information);
  }

  // // Update the project in core memory
  // act_status = this->UpdateProject(project, false, is_operation);
  // if (!IsActStatusSuccess(act_status)) {
  //   qCritical() << "Cannot update the project id:" << project.GetId();
  //   return act_status;
  // }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateDeviceInformations(qint64 &project_id, ActDeviceInformationList &device_information_list,
                                             bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActDeviceConfig &device_config = project.GetDeviceConfig();

  QMap<qint64, ActInformationSettingTable> &information_setting_tables = device_config.GetInformationSettingTables();

  // Check device information list is empty
  auto information_list = device_information_list.GetDeviceInformationList();
  if (information_list.isEmpty()) {
    QString error_msg = QString("Device information list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (ActDeviceInformation device_information : information_list) {
    ActInformationSettingTable &information_setting = information_setting_tables[device_information.GetDeviceId()];
    information_setting.SetDeviceName(device_information.GetDeviceName());
    information_setting.SetLocation(device_information.GetLocation());
    information_setting.SetDescription(device_information.GetDescription());
    information_setting.SetContactInformation(device_information.GetContactInformation());

    ActDevice device;
    act_status = project.GetDeviceById(device, device_information.GetDeviceId());
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Get device failed with device id:" << device_information.GetDeviceId();
      return act_status;
    }

    device.SetDeviceName(device_information.GetDeviceName());
    device.GetDeviceInfo().SetLocation(device_information.GetLocation());
    device.GetDeviceProperty().SetDescription(device_information.GetDescription());

    act_status = this->UpdateDevice(project, device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Update device failed with device id:" << device.GetId();
      return act_status;
    }
    // Notify the user that the device be update
    ActDevicePatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, project_id, device, true);
    this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project_id);
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetDeviceLoginPolicy(qint64 &project_id, qint64 &device_id,
                                         ActDeviceLoginPolicy &device_login_policy, bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActLoginPolicyTable> &login_policy_tables = project.GetDeviceConfig().GetLoginPolicyTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetId() != device_id) {
      continue;
    } else if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      QString error_msg = QString("Device %1 is not a switch").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(error_msg);
    }

    ActLoginPolicyTable login_policy(device);
    if (login_policy_tables.contains(device.GetId())) {
      login_policy = login_policy_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetLoginPolicyTables().isEmpty()) {
        login_policy = device_profile.GetDefaultDeviceConfig().GetLoginPolicyTables().first();
      }
      login_policy_tables[device.GetId()] = login_policy;  // insert config to tables
    }

    device_login_policy = ActDeviceLoginPolicy(device);
    device_login_policy.SetLoginMessage(login_policy.GetLoginMessage());
    device_login_policy.SetLoginAuthenticationFailureMessage(login_policy.GetLoginAuthenticationFailureMessage());
    device_login_policy.SetAccountLoginFailureLockout(login_policy.GetLoginFailureLockout());
    device_login_policy.SetRetryFailureThreshold(login_policy.GetRetryFailureThreshold());
    device_login_policy.SetLockoutDuration(login_policy.GetLockoutDuration());
    device_login_policy.SetAutoLogoutAfter(login_policy.GetAutoLogoutAfter());
    return ACT_STATUS_SUCCESS;
  }
  return std::make_shared<ActStatusNotFound>(QString("Device %1 in project is not found").arg(device_id));
}

ACT_STATUS ActCore::GetDeviceLoginPolicies(qint64 &project_id, ActDeviceLoginPolicyList &device_login_policy_list,
                                           bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActDeviceLoginPolicy> &device_login_policy_list_ = device_login_policy_list.GetDeviceLoginPolicyList();
  QMap<qint64, ActLoginPolicyTable> &login_policy_tables = project.GetDeviceConfig().GetLoginPolicyTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }

    ActLoginPolicyTable login_policy(device);
    if (login_policy_tables.contains(device.GetId())) {
      login_policy = login_policy_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetLoginPolicyTables().isEmpty()) {
        login_policy = device_profile.GetDefaultDeviceConfig().GetLoginPolicyTables().first();
      }
      login_policy_tables[device.GetId()] = login_policy;  // insert config to tables
    }

    ActDeviceLoginPolicy device_login_policy(device);
    device_login_policy.SetLoginMessage(login_policy.GetLoginMessage());
    device_login_policy.SetLoginAuthenticationFailureMessage(login_policy.GetLoginAuthenticationFailureMessage());
    device_login_policy.SetAccountLoginFailureLockout(login_policy.GetLoginFailureLockout());
    device_login_policy.SetRetryFailureThreshold(login_policy.GetRetryFailureThreshold());
    device_login_policy.SetLockoutDuration(login_policy.GetLockoutDuration());
    device_login_policy.SetAutoLogoutAfter(login_policy.GetAutoLogoutAfter());
    device_login_policy_list_.append(device_login_policy);
  }
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateDeviceLoginPolicies(qint64 &project_id, ActDeviceLoginPolicyList &device_login_policy_list,
                                              bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActDeviceConfig &device_config = project.GetDeviceConfig();

  QMap<qint64, ActLoginPolicyTable> &login_policy_tables = device_config.GetLoginPolicyTables();

  // Check device login policy list is empty
  auto login_policy_list = device_login_policy_list.GetDeviceLoginPolicyList();
  if (login_policy_list.isEmpty()) {
    QString error_msg = QString("Device login policy list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (ActDeviceLoginPolicy device_login_policy : login_policy_list) {
    ActLoginPolicyTable &login_policy = login_policy_tables[device_login_policy.GetDeviceId()];
    login_policy.SetLoginMessage(device_login_policy.GetLoginMessage());
    login_policy.SetLoginAuthenticationFailureMessage(device_login_policy.GetLoginAuthenticationFailureMessage());
    login_policy.SetLoginFailureLockout(device_login_policy.GetAccountLoginFailureLockout());
    login_policy.SetRetryFailureThreshold(device_login_policy.GetRetryFailureThreshold());
    login_policy.SetLockoutDuration(device_login_policy.GetLockoutDuration());
    login_policy.SetAutoLogoutAfter(device_login_policy.GetAutoLogoutAfter());
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetDeviceLoopProtection(qint64 &project_id, qint64 &device_id,
                                            ActDeviceLoopProtection &device_loop_protection, bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActLoopProtectionTable> &loop_protection_tables = project.GetDeviceConfig().GetLoopProtectionTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetId() != device_id) {
      continue;
    } else if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      QString error_msg = QString("Device %1 is not a switch").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(error_msg);
    }

    ActLoopProtectionTable loop_protection(device);
    if (loop_protection_tables.contains(device.GetId())) {
      loop_protection = loop_protection_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetLoopProtectionTables().isEmpty()) {
        loop_protection = device_profile.GetDefaultDeviceConfig().GetLoopProtectionTables().first();
      }
      loop_protection_tables[device.GetId()] = loop_protection;  // insert config to tables
    }

    device_loop_protection = ActDeviceLoopProtection(device);
    device_loop_protection.SetNetworkLoopProtection(loop_protection.GetNetworkLoopProtection());
    device_loop_protection.SetDetectInterval(loop_protection.GetDetectInterval());
    return ACT_STATUS_SUCCESS;
  }
  return std::make_shared<ActStatusNotFound>(QString("Device %1 in project is not found").arg(device_id));
}

ACT_STATUS ActCore::GetDeviceLoopProtections(qint64 &project_id,
                                             ActDeviceLoopProtectionList &device_loop_protection_list,
                                             bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActDeviceLoopProtection> &device_loop_protection_list_ =
      device_loop_protection_list.GetDeviceLoopProtectionList();
  QMap<qint64, ActLoopProtectionTable> &loop_protection_tables = project.GetDeviceConfig().GetLoopProtectionTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }

    ActLoopProtectionTable loop_protection(device);
    if (loop_protection_tables.contains(device.GetId())) {
      loop_protection = loop_protection_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetLoopProtectionTables().isEmpty()) {
        loop_protection = device_profile.GetDefaultDeviceConfig().GetLoopProtectionTables().first();
      }
      loop_protection_tables[device.GetId()] = loop_protection;  // insert config to tables
    }

    ActDeviceLoopProtection device_loop_protection(device);
    device_loop_protection.SetNetworkLoopProtection(loop_protection.GetNetworkLoopProtection());
    device_loop_protection.SetDetectInterval(loop_protection.GetDetectInterval());
    device_loop_protection_list_.append(device_loop_protection);
  }
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateDeviceLoopProtections(qint64 &project_id,
                                                ActDeviceLoopProtectionList &device_loop_protection_list,
                                                bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActDeviceConfig &device_config = project.GetDeviceConfig();

  QMap<qint64, ActLoopProtectionTable> &loop_protection_tables = device_config.GetLoopProtectionTables();

  // Check device loop protection list is empty
  auto loop_protection_list = device_loop_protection_list.GetDeviceLoopProtectionList();
  if (loop_protection_list.isEmpty()) {
    QString error_msg = QString("Device loop protection list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (ActDeviceLoopProtection device_loop_protection : loop_protection_list) {
    ActLoopProtectionTable &loop_protection = loop_protection_tables[device_loop_protection.GetDeviceId()];
    loop_protection.SetNetworkLoopProtection(device_loop_protection.GetNetworkLoopProtection());
    loop_protection.SetDetectInterval(device_loop_protection.GetDetectInterval());
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetDeviceSyslogSetting(qint64 &project_id, qint64 &device_id,
                                           ActDeviceSyslogSetting &device_syslog_setting, bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActSyslogSettingTable> &syslog_setting_tables = project.GetDeviceConfig().GetSyslogSettingTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetId() != device_id) {
      continue;
    } else if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      QString error_msg = QString("Device %1 is not a switch").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(error_msg);
    }

    ActSyslogSettingTable syslog_setting(device);
    if (syslog_setting_tables.contains(device.GetId())) {
      syslog_setting = syslog_setting_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetSyslogSettingTables().isEmpty()) {
        syslog_setting = device_profile.GetDefaultDeviceConfig().GetSyslogSettingTables().first();
      }
      syslog_setting_tables[device.GetId()] = syslog_setting;  // insert config to tables
    }

    device_syslog_setting = ActDeviceSyslogSetting(device);
    device_syslog_setting.SetEnabled(syslog_setting.GetEnabled());
    device_syslog_setting.SetSyslogServer1(syslog_setting.GetSyslogServer1());
    device_syslog_setting.SetAddress1(syslog_setting.GetAddress1());
    device_syslog_setting.SetUDPPort1(syslog_setting.GetPort1());
    device_syslog_setting.SetSyslogServer2(syslog_setting.GetSyslogServer2());
    device_syslog_setting.SetAddress2(syslog_setting.GetAddress2());
    device_syslog_setting.SetUDPPort2(syslog_setting.GetPort2());
    device_syslog_setting.SetSyslogServer3(syslog_setting.GetSyslogServer3());
    device_syslog_setting.SetAddress3(syslog_setting.GetAddress3());
    device_syslog_setting.SetUDPPort3(syslog_setting.GetPort3());
    return ACT_STATUS_SUCCESS;
  }
  return std::make_shared<ActStatusNotFound>(QString("Device %1 in project is not found").arg(device_id));
}

ACT_STATUS ActCore::GetDeviceSyslogSettings(qint64 &project_id, ActDeviceSyslogSettingList &device_syslog_setting_list,
                                            bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActDeviceSyslogSetting> &device_syslog_setting_list_ = device_syslog_setting_list.GetDeviceSyslogSettingList();
  QMap<qint64, ActSyslogSettingTable> &syslog_setting_tables = project.GetDeviceConfig().GetSyslogSettingTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }

    ActSyslogSettingTable syslog_setting(device);
    if (syslog_setting_tables.contains(device.GetId())) {
      syslog_setting = syslog_setting_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetSyslogSettingTables().isEmpty()) {
        syslog_setting = device_profile.GetDefaultDeviceConfig().GetSyslogSettingTables().first();
      }
      syslog_setting_tables[device.GetId()] = syslog_setting;  // insert config to tables
    }

    ActDeviceSyslogSetting device_syslog_setting(device);
    device_syslog_setting.SetEnabled(syslog_setting.GetEnabled());
    device_syslog_setting.SetSyslogServer1(syslog_setting.GetSyslogServer1());
    device_syslog_setting.SetAddress1(syslog_setting.GetAddress1());
    device_syslog_setting.SetUDPPort1(syslog_setting.GetPort1());
    device_syslog_setting.SetSyslogServer2(syslog_setting.GetSyslogServer2());
    device_syslog_setting.SetAddress2(syslog_setting.GetAddress2());
    device_syslog_setting.SetUDPPort2(syslog_setting.GetPort2());
    device_syslog_setting.SetSyslogServer3(syslog_setting.GetSyslogServer3());
    device_syslog_setting.SetAddress3(syslog_setting.GetAddress3());
    device_syslog_setting.SetUDPPort3(syslog_setting.GetPort3());
    device_syslog_setting_list_.append(device_syslog_setting);
  }
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateDeviceSyslogSettings(qint64 &project_id,
                                               ActDeviceSyslogSettingList &device_syslog_setting_list,
                                               bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActDeviceConfig &device_config = project.GetDeviceConfig();

  QMap<qint64, ActSyslogSettingTable> &syslog_setting_tables = device_config.GetSyslogSettingTables();

  // Check device syslog setting list is empty
  auto syslog_setting_list = device_syslog_setting_list.GetDeviceSyslogSettingList();
  if (syslog_setting_list.isEmpty()) {
    QString error_msg = QString("Device syslog setting list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (ActDeviceSyslogSetting device_syslog_setting : syslog_setting_list) {
    if ((device_syslog_setting.GetAddress1() == device_syslog_setting.GetAddress2() &&
         !device_syslog_setting.GetAddress1().isEmpty()) ||
        (device_syslog_setting.GetAddress2() == device_syslog_setting.GetAddress3() &&
         !device_syslog_setting.GetAddress2().isEmpty()) ||
        (device_syslog_setting.GetAddress3() == device_syslog_setting.GetAddress1() &&
         !device_syslog_setting.GetAddress3().isEmpty())) {
      QString error_msg = QString("The server addresses are duplicated.");
      return std::make_shared<ActBadRequest>(error_msg);
    }

    ActSyslogSettingTable &syslog_setting = syslog_setting_tables[device_syslog_setting.GetDeviceId()];
    syslog_setting.SetEnabled(device_syslog_setting.GetEnabled());
    syslog_setting.SetSyslogServer1(device_syslog_setting.GetSyslogServer1());
    syslog_setting.SetAddress1(device_syslog_setting.GetAddress1());
    syslog_setting.SetPort1(device_syslog_setting.GetUDPPort1());
    syslog_setting.SetSyslogServer2(device_syslog_setting.GetSyslogServer2());
    syslog_setting.SetAddress2(device_syslog_setting.GetAddress2());
    syslog_setting.SetPort2(device_syslog_setting.GetUDPPort2());
    syslog_setting.SetSyslogServer3(device_syslog_setting.GetSyslogServer3());
    syslog_setting.SetAddress3(device_syslog_setting.GetAddress3());
    syslog_setting.SetPort3(device_syslog_setting.GetUDPPort3());
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetDeviceSnmpTrapSetting(qint64 &project_id, qint64 &device_id,
                                             ActDeviceSnmpTrapSetting &device_snmp_trap_setting, bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActSnmpTrapSettingTable> &snmp_trap_setting_tables =
      project.GetDeviceConfig().GetSnmpTrapSettingTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetId() != device_id) {
      continue;
    } else if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      QString error_msg = QString("Device %1 is not a switch").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(error_msg);
    }

    ActSnmpTrapSettingTable snmp_trap_setting(device);
    if (snmp_trap_setting_tables.contains(device.GetId())) {
      snmp_trap_setting = snmp_trap_setting_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetSnmpTrapSettingTables().isEmpty()) {
        snmp_trap_setting = device_profile.GetDefaultDeviceConfig().GetSnmpTrapSettingTables().first();
      }
      snmp_trap_setting_tables[device.GetId()] = snmp_trap_setting;  // insert config to tables
    }

    device_snmp_trap_setting = ActDeviceSnmpTrapSetting(device);
    device_snmp_trap_setting.SetHostList(snmp_trap_setting.GetHostList());
    return ACT_STATUS_SUCCESS;
  }
  return std::make_shared<ActStatusNotFound>(QString("Device %1 in project is not found").arg(device_id));
}

ACT_STATUS ActCore::GetDeviceSnmpTrapSettings(qint64 &project_id,
                                              ActDeviceSnmpTrapSettingList &device_snmp_trap_setting_list,
                                              bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActDeviceSnmpTrapSetting> &device_snmp_trap_setting_list_ =
      device_snmp_trap_setting_list.GetDeviceSnmpTrapSettingList();
  QMap<qint64, ActSnmpTrapSettingTable> &snmp_trap_setting_tables =
      project.GetDeviceConfig().GetSnmpTrapSettingTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }

    ActSnmpTrapSettingTable snmp_trap_setting(device);
    if (snmp_trap_setting_tables.contains(device.GetId())) {
      snmp_trap_setting = snmp_trap_setting_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetSnmpTrapSettingTables().isEmpty()) {
        snmp_trap_setting = device_profile.GetDefaultDeviceConfig().GetSnmpTrapSettingTables().first();
      }
      snmp_trap_setting_tables[device.GetId()] = snmp_trap_setting;  // insert config to tables
    }

    ActDeviceSnmpTrapSetting device_snmp_trap_setting(device);
    device_snmp_trap_setting.SetHostList(snmp_trap_setting.GetHostList());
    device_snmp_trap_setting_list_.append(device_snmp_trap_setting);
  }
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateDeviceSnmpTrapSettings(qint64 &project_id,
                                                 ActDeviceSnmpTrapSettingList &device_snmp_trap_setting_list,
                                                 bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActDeviceConfig &device_config = project.GetDeviceConfig();

  QMap<qint64, ActSnmpTrapSettingTable> &snmp_trap_setting_tables = device_config.GetSnmpTrapSettingTables();

  // Check device SNMP trap setting list is empty
  auto snmp_trap_setting_list = device_snmp_trap_setting_list.GetDeviceSnmpTrapSettingList();
  if (snmp_trap_setting_list.isEmpty()) {
    QString error_msg = QString("Device SNMP trap setting list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (ActDeviceSnmpTrapSetting device_snmp_trap_setting : snmp_trap_setting_list) {
    ActSnmpTrapSettingTable &snmp_trap_setting = snmp_trap_setting_tables[device_snmp_trap_setting.GetDeviceId()];
    snmp_trap_setting.SetHostList(device_snmp_trap_setting.GetHostList());
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetDeviceBackups(qint64 &project_id, ActDeviceBackupSettingList &device_backup_setting_list,
                                     bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActDeviceBackupSetting> &device_backup_setting_list_ = device_backup_setting_list.GetDeviceBackupSettingList();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }
    ActDeviceBackupSetting device_backup_setting(device);
    device_backup_setting.SetModelName(device.GetDeviceProperty().GetModelName());
    device_backup_setting.SetFirmwareVersion(device.GetFirmwareVersion());
    device_backup_setting.SetMacAddress(device.GetMacAddress());
    device_backup_setting.SetSerialNumber(device.GetDeviceInfo().GetSerialNumber());
    device_backup_setting.SetLocation(device.GetDeviceInfo().GetLocation());
    device_backup_setting_list_.append(device_backup_setting);
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::ExportDeviceBackupFile(qint64 &project_id, ActDeviceBackupFile &device_backup_file,
                                           bool is_operation) {
  ACT_STATUS_INIT();

  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QString zip_file_name(QString(MAF_EXPORT_CONFIG_FILE_NAME));
  // kene+
  /*
  QString zip_file_path(QString("%1/%2").arg(ACT_DEVICE_CONFIG_FILE_FOLDER).arg(zip_file_name));
  act_status = this->CompressFolder(ACT_DEVICE_CONFIG_FILE_FOLDER, zip_file_path);
  */
  QString deviceConfigFilePath = GetDeviceConfigFilePath();
  QString zip_file_path(QString("%1/%2").arg(deviceConfigFilePath).arg(zip_file_name));
  act_status = this->CompressFolder(deviceConfigFilePath, zip_file_path);
  // kene-
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compress folder failed:" << act_status->GetErrorMessage().toStdString().c_str();
    return act_status;
  }

  QByteArray file_content;
  act_status = this->ReadFileContent(zip_file_path, file_content);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Read zip file failed:" << act_status->GetErrorMessage().toStdString().c_str();
    return act_status;
  }

  device_backup_file.SetFileName(zip_file_name);
  device_backup_file.SetBackupFile(file_content.toBase64());

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::ImportDeviceBackupFile(qint64 &project_id, qint64 &device_id,
                                           ActDeviceBackupFile &device_backup_file, bool is_operation) {
  ACT_STATUS_INIT();

  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActDevice device;
  act_status = project.GetDeviceById(device, device_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get device failed with device id:" << device_id;
    return act_status;
  }

  QFile file(QString("%1/%2_%3.ini")
                 // kene+
                 /*
                 .arg(ACT_DEVICE_CONFIG_FILE_FOLDER)
                 */
                 .arg(GetDeviceConfigFilePath())
                 // kene-
                 .arg(project.GetProjectName())
                 .arg(device.GetIpv4().GetIpAddress()));
  if (!file.open(QIODevice::WriteOnly)) {
    qDebug() << "Open device config file failed:" << file.fileName();
    return std::make_shared<ActStatusInternalError>("ImportDeviceBackupFile");
  }

  file.write(device_backup_file.GetBackupFile().toStdString().c_str());
  file.close();

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetDeviceVlanSetting(qint64 &project_id, qint64 &device_id,
                                         ActDeviceVlanSetting &device_vlan_setting, bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActVlanTable> &vlan_tables = project.GetDeviceConfig().GetVlanTables();
  QMap<qint64, ActDefaultPriorityTable> &port_priority_tables = project.GetDeviceConfig().GetPortDefaultPCPTables();

  for (ActDevice device : project.GetDevices()) {
    if (device.GetId() != device_id) {
      continue;
    } else if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      QString error_msg = QString("Device %1 is not a switch").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(error_msg);
    }

    ActVlanTable vlan_table(device);
    if (vlan_tables.contains(device.GetId())) {
      vlan_table = vlan_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetVlanTables().isEmpty()) {
        vlan_table = device_profile.GetDefaultDeviceConfig().GetVlanTables().first();
      }
      vlan_tables[device.GetId()] = vlan_table;  // insert config to tables
    }

    device_vlan_setting = ActDeviceVlanSetting(device);
    device_vlan_setting.SetReservedVlan(device.GetDeviceProperty().GetReservedVlan());
    device_vlan_setting.SetManagementVlan(vlan_table.GetManagementVlan());

    QList<ActDeviceVlan> &vlan_list = device_vlan_setting.GetVlanList();
    QMap<qint64, ActDevicePort> port_map;
    for (ActInterface &interface : device.GetInterfaces()) {
      port_map.insert(interface.GetInterfaceId(),
                      ActDevicePort(interface.GetActive(), interface.GetInterfaceId(), interface.GetInterfaceName()));
    }

    for (ActVlanStaticEntry vlan_static_entry : vlan_table.GetVlanStaticEntries()) {
      ActDeviceVlan device_vlan;
      device_vlan.SetVlanId(vlan_static_entry.GetVlanId());
      device_vlan.SetVlanName(vlan_static_entry.GetName());
      device_vlan.SetTeMstid(vlan_static_entry.GetTeMstid());
      device_vlan.SetMemberPort(vlan_static_entry.GetEgressPorts());
      vlan_list.append(device_vlan);

      for (qint64 port_id : vlan_static_entry.GetEgressPorts()) {
        ActDevicePort &device_port = port_map[port_id];
        if (vlan_static_entry.GetUntaggedPorts().contains(port_id)) {
          device_port.GetUntaggedVlan().insert(vlan_static_entry.GetVlanId());
        } else {
          device_port.GetTaggedVlan().insert(vlan_static_entry.GetVlanId());
        }
      }
    }

    for (ActPortVlanEntry port_vlan_entry : vlan_table.GetPortVlanEntries()) {
      ActDevicePort &device_port = port_map[port_vlan_entry.GetPortId()];
      device_port.SetPvid(port_vlan_entry.GetPVID());
    }

    for (ActVlanPortTypeEntry vlan_port_type_entry : vlan_table.GetVlanPortTypeEntries()) {
      ActDevicePort &device_port = port_map[vlan_port_type_entry.GetPortId()];
      device_port.SetPortType(vlan_port_type_entry.GetVlanPortType());
    }

    ActDefaultPriorityTable &priority_table = port_priority_tables[device.GetId()];
    for (ActDefaultPriorityEntry default_priority_entry : priority_table.GetDefaultPriorityEntries()) {
      ActDevicePort &device_port = port_map[default_priority_entry.GetPortId()];
      device_port.SetDefaultPCP(default_priority_entry.GetDefaultPCP());
    }

    device_vlan_setting.SetPortList(port_map.values());
    return ACT_STATUS_SUCCESS;
  }
  return std::make_shared<ActStatusNotFound>(QString("Device %1 in project is not found").arg(device_id));
}

ACT_STATUS ActCore::GetDeviceVlanSettings(qint64 &project_id, ActDeviceVlanSettingList &device_vlan_setting_list,
                                          bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActVlanTable> &vlan_tables = project.GetDeviceConfig().GetVlanTables();
  QMap<qint64, ActDefaultPriorityTable> &port_priority_tables = project.GetDeviceConfig().GetPortDefaultPCPTables();

  QList<ActDeviceVlanSetting> &device_vlan_setting_list_ = device_vlan_setting_list.GetDeviceVlanSettingList();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }

    ActVlanTable vlan_table(device);
    if (vlan_tables.contains(device.GetId())) {
      vlan_table = vlan_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetVlanTables().isEmpty()) {
        vlan_table = device_profile.GetDefaultDeviceConfig().GetVlanTables().first();
      }
      vlan_tables[device.GetId()] = vlan_table;  // insert config to tables
    }

    ActDeviceVlanSetting device_vlan_setting(device);
    device_vlan_setting.SetReservedVlan(device.GetDeviceProperty().GetReservedVlan());
    device_vlan_setting.SetManagementVlan(vlan_table.GetManagementVlan());

    QList<ActDeviceVlan> &vlan_list = device_vlan_setting.GetVlanList();
    QMap<qint64, ActDevicePort> port_map;
    for (ActInterface &interface : device.GetInterfaces()) {
      port_map.insert(interface.GetInterfaceId(),
                      ActDevicePort(interface.GetActive(), interface.GetInterfaceId(), interface.GetInterfaceName()));
    }

    for (ActVlanStaticEntry vlan_static_entry : vlan_table.GetVlanStaticEntries()) {
      ActDeviceVlan device_vlan;
      device_vlan.SetVlanId(vlan_static_entry.GetVlanId());
      device_vlan.SetVlanName(vlan_static_entry.GetName());
      device_vlan.SetTeMstid(vlan_static_entry.GetTeMstid());
      device_vlan.SetMemberPort(vlan_static_entry.GetEgressPorts());
      vlan_list.append(device_vlan);

      for (qint64 port_id : vlan_static_entry.GetEgressPorts()) {
        ActDevicePort &device_port = port_map[port_id];
        if (vlan_static_entry.GetUntaggedPorts().contains(port_id)) {
          device_port.GetUntaggedVlan().insert(vlan_static_entry.GetVlanId());
        } else {
          device_port.GetTaggedVlan().insert(vlan_static_entry.GetVlanId());
        }
      }
    }

    for (ActPortVlanEntry port_vlan_entry : vlan_table.GetPortVlanEntries()) {
      ActDevicePort &device_port = port_map[port_vlan_entry.GetPortId()];
      device_port.SetPvid(port_vlan_entry.GetPVID());
    }

    for (ActVlanPortTypeEntry vlan_port_type_entry : vlan_table.GetVlanPortTypeEntries()) {
      ActDevicePort &device_port = port_map[vlan_port_type_entry.GetPortId()];
      device_port.SetPortType(vlan_port_type_entry.GetVlanPortType());
    }

    ActDefaultPriorityTable &priority_table = port_priority_tables[device.GetId()];
    for (ActDefaultPriorityEntry default_priority_entry : priority_table.GetDefaultPriorityEntries()) {
      ActDevicePort &device_port = port_map[default_priority_entry.GetPortId()];
      device_port.SetDefaultPCP(default_priority_entry.GetDefaultPCP());
    }

    device_vlan_setting.SetPortList(port_map.values());
    device_vlan_setting_list_.append(device_vlan_setting);
  }
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateDeviceVlanSettings(qint64 &project_id, ActDeviceVlanSettingList &device_vlan_setting_list,
                                             bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActVlanTable> &vlan_tables = project.GetDeviceConfig().GetVlanTables();
  QMap<qint64, ActDefaultPriorityTable> &port_priority_tables = project.GetDeviceConfig().GetPortDefaultPCPTables();

  // Check device SNMP trap setting list is empty
  auto vlan_setting_list = device_vlan_setting_list.GetDeviceVlanSettingList();
  if (vlan_setting_list.isEmpty()) {
    QString error_msg = QString("Device VLAN setting list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (ActDeviceVlanSetting device_vlan_setting : vlan_setting_list) {
    qint64 device_id = device_vlan_setting.GetDeviceId();
    ActDevice device;
    act_status = project.GetDeviceById(device, device_id);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    // Check has the reserved VLANs
    for (auto reserved_vlan_id : device.GetDeviceProperty().GetReservedVlan()) {
      if (!device_vlan_setting.GetVlanList().contains(ActDeviceVlan(reserved_vlan_id))) {
        QString error_msg = QString("Device(%1) reserved VLAN(%2) does not exist")
                                .arg(device.GetIpv4().GetIpAddress())
                                .arg(reserved_vlan_id);
        qWarning() << __func__ << error_msg;
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }

    ActDefaultPriorityTable &port_priority_table = port_priority_tables[device_id];
    QSet<ActDefaultPriorityEntry> &default_priority_entry_set = port_priority_table.GetDefaultPriorityEntries();
    QSet<ActDefaultPriorityEntry>::iterator default_priority_entry_iter;

    ActVlanTable &vlan_table = vlan_tables[device_id];
    vlan_table.SetManagementVlan(device_vlan_setting.GetManagementVlan());

    QSet<ActVlanStaticEntry> vlan_static_entry_set;
    QSet<ActPortVlanEntry> &port_vlan_entry_set = vlan_table.GetPortVlanEntries();
    QSet<ActVlanPortTypeEntry> &vlan_port_type_entry_set = vlan_table.GetVlanPortTypeEntries();
    QSet<ActVlanStaticEntry>::iterator vlan_static_entry_iter;
    QSet<ActPortVlanEntry>::iterator port_vlan_entry_iter;
    QSet<ActVlanPortTypeEntry>::iterator vlan_port_type_entry_iter;

    for (ActDeviceVlan device_vlan : device_vlan_setting.GetVlanList()) {
      // update vlan static entry
      ActVlanStaticEntry vlan_static_entry(device_vlan.GetVlanId());

      vlan_static_entry_iter = vlan_table.GetVlanStaticEntries().find(vlan_static_entry);
      if (vlan_static_entry_iter != vlan_table.GetVlanStaticEntries().end()) {
        vlan_static_entry = *vlan_static_entry_iter;
      }

      vlan_static_entry.SetName(device_vlan.GetVlanName());
      vlan_static_entry.SetTeMstid(device_vlan.GetTeMstid());
      for (ActDevicePort device_port : device_vlan_setting.GetPortList()) {
        qint64 port_id = device_port.GetPortId();
        vlan_static_entry.GetEgressPorts().remove(port_id);
        vlan_static_entry.GetUntaggedPorts().remove(port_id);
      }

      vlan_static_entry_set.insert(vlan_static_entry);
    }

    for (ActDevicePort device_port : device_vlan_setting.GetPortList()) {
      qint64 port_id = device_port.GetPortId();
      if (port_id <= 0 || port_id > device.GetInterfaces().size()) {
        continue;
      }
      // update port vlan entry
      ActPortVlanEntry port_vlan_entry(port_id);
      port_vlan_entry.SetPVID(device_port.GetPvid());

      port_vlan_entry_iter = port_vlan_entry_set.find(port_vlan_entry);
      if (port_vlan_entry_iter != port_vlan_entry_set.end()) {
        port_vlan_entry_set.erase(port_vlan_entry_iter);
      }
      port_vlan_entry_set.insert(port_vlan_entry);

      // update vlan port type entry
      ActVlanPortTypeEntry vlan_port_type_entry(port_id);
      vlan_port_type_entry.SetVlanPortType(device_port.GetPortType());

      vlan_port_type_entry_iter = vlan_port_type_entry_set.find(vlan_port_type_entry);
      if (vlan_port_type_entry_iter != vlan_port_type_entry_set.end()) {
        vlan_port_type_entry_set.erase(vlan_port_type_entry_iter);
      }
      vlan_port_type_entry_set.insert(vlan_port_type_entry);

      // update untagged vlan
      for (qint32 untagged_vlan : device_port.GetUntaggedVlan()) {
        ActVlanStaticEntry untagged_vlan_static_entry(untagged_vlan);
        vlan_static_entry_iter = vlan_static_entry_set.find(untagged_vlan_static_entry);
        if (vlan_static_entry_iter != vlan_static_entry_set.end()) {
          untagged_vlan_static_entry = *vlan_static_entry_iter;
          vlan_static_entry_set.erase(vlan_static_entry_iter);
        } else {
          QString error_msg = QString("Untagged VLAN %1 doesn't exist in VLAN table").arg(untagged_vlan);
          qWarning() << __func__ << error_msg;
          return std::make_shared<ActBadRequest>(error_msg);
        }
        untagged_vlan_static_entry.GetUntaggedPorts().insert(port_id);
        untagged_vlan_static_entry.GetEgressPorts().insert(port_id);
        vlan_static_entry_set.insert(untagged_vlan_static_entry);
      }

      // update tagged vlan
      for (qint32 tagged_vlan : device_port.GetTaggedVlan()) {
        ActVlanStaticEntry tagged_vlan_static_entry(tagged_vlan);
        vlan_static_entry_iter = vlan_static_entry_set.find(tagged_vlan_static_entry);
        if (vlan_static_entry_iter != vlan_static_entry_set.end()) {
          tagged_vlan_static_entry = *vlan_static_entry_iter;
          vlan_static_entry_set.erase(vlan_static_entry_iter);
        } else {
          QString error_msg = QString("Tagged VLAN %1 doesn't exist in VLAN table").arg(tagged_vlan);
          qWarning() << __func__ << error_msg;
          return std::make_shared<ActBadRequest>(error_msg);
        }
        tagged_vlan_static_entry.GetUntaggedPorts().remove(port_id);
        tagged_vlan_static_entry.GetEgressPorts().insert(port_id);
        vlan_static_entry_set.insert(tagged_vlan_static_entry);
      }

      ActDefaultPriorityEntry default_priority_entry(port_id);
      default_priority_entry.SetDefaultPCP(device_port.GetDefaultPCP());

      // update default priority entry
      default_priority_entry_iter = default_priority_entry_set.find(default_priority_entry);
      if (default_priority_entry_iter != default_priority_entry_set.end()) {
        default_priority_entry_set.erase(default_priority_entry_iter);
      }
      default_priority_entry_set.insert(default_priority_entry);
    }

    vlan_table.SetVlanStaticEntries(vlan_static_entry_set);
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetDeviceTimeSetting(qint64 &project_id, qint64 &device_id,
                                         ActDeviceTimeSetting &device_time_setting, bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActTimeSettingTable> &time_setting_tables = project.GetDeviceConfig().GetTimeSettingTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetId() != device_id) {
      continue;
    } else if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      QString error_msg = QString("Device %1 is not a switch").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(error_msg);
    }

    ActTimeSettingTable time_setting_table(device);
    if (time_setting_tables.contains(device.GetId())) {
      time_setting_table = time_setting_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetTimeSettingTables().isEmpty()) {
        time_setting_table = device_profile.GetDefaultDeviceConfig().GetTimeSettingTables().first();
      }
      time_setting_tables[device.GetId()] = time_setting_table;  // insert config to tables
    }

    device_time_setting = ActDeviceTimeSetting(device);
    device_time_setting.SetClockSource(time_setting_table.GetClockSource());
    device_time_setting.SetNTPTimeServer1(time_setting_table.GetNTPTimeServer1());
    device_time_setting.SetNTPTimeServer2(time_setting_table.GetNTPTimeServer2());
    device_time_setting.SetSNTPTimeServer1(time_setting_table.GetSNTPTimeServer1());
    device_time_setting.SetSNTPTimeServer2(time_setting_table.GetSNTPTimeServer2());
    device_time_setting.SetTimeZone(time_setting_table.GetTimeZone());
    device_time_setting.SetDaylightSavingTime(time_setting_table.GetDaylightSavingTime());
    device_time_setting.SetOffset(time_setting_table.GetOffset());
    device_time_setting.SetStart(time_setting_table.GetStart());
    device_time_setting.SetEnd(time_setting_table.GetEnd());
    return ACT_STATUS_SUCCESS;
  }
  return std::make_shared<ActStatusNotFound>(QString("Device %1 in project is not found").arg(device_id));
}

ACT_STATUS ActCore::GetDeviceTimeSettings(qint64 &project_id, ActDeviceTimeSettingList &device_time_setting_list,
                                          bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActDeviceTimeSetting> &device_time_setting_list_ = device_time_setting_list.GetDeviceTimeSettingList();
  QMap<qint64, ActTimeSettingTable> &time_setting_tables = project.GetDeviceConfig().GetTimeSettingTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }

    ActTimeSettingTable time_setting_table(device);
    if (time_setting_tables.contains(device.GetId())) {
      time_setting_table = time_setting_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetTimeSettingTables().isEmpty()) {
        time_setting_table = device_profile.GetDefaultDeviceConfig().GetTimeSettingTables().first();
      }
      time_setting_tables[device.GetId()] = time_setting_table;  // insert config to tables
    }

    ActDeviceTimeSetting device_time_setting(device);
    device_time_setting.SetClockSource(time_setting_table.GetClockSource());
    device_time_setting.SetNTPTimeServer1(time_setting_table.GetNTPTimeServer1());
    device_time_setting.SetNTPTimeServer2(time_setting_table.GetNTPTimeServer2());
    device_time_setting.SetSNTPTimeServer1(time_setting_table.GetSNTPTimeServer1());
    device_time_setting.SetSNTPTimeServer2(time_setting_table.GetSNTPTimeServer2());
    device_time_setting.SetTimeZone(time_setting_table.GetTimeZone());
    device_time_setting.SetDaylightSavingTime(time_setting_table.GetDaylightSavingTime());
    device_time_setting.SetOffset(time_setting_table.GetOffset());
    device_time_setting.SetStart(time_setting_table.GetStart());
    device_time_setting.SetEnd(time_setting_table.GetEnd());
    device_time_setting_list_.append(device_time_setting);
  }
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateDeviceTimeSettings(qint64 &project_id, ActDeviceTimeSettingList &device_time_setting_list,
                                             bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActDeviceConfig &device_config = project.GetDeviceConfig();

  QMap<qint64, ActTimeSettingTable> &time_setting_tables = device_config.GetTimeSettingTables();

  // Check device time setting list is empty
  auto time_setting_list = device_time_setting_list.GetDeviceTimeSettingList();
  if (time_setting_list.isEmpty()) {
    QString error_msg = QString("Device time setting list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (ActDeviceTimeSetting device_time_setting : time_setting_list) {
    ActDevice device;
    act_status = project.GetDeviceById(device, device_time_setting.GetDeviceId());
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Get device failed with device id:" << device_time_setting.GetDeviceId();
      return act_status;
    }

    // Check input
    if (device_time_setting.GetOffset() == "00:00") {
      QString error_msg = QString("Offset of device id %1 is out of range: %2 (valid range: %3-%4)")
                              .arg(device_time_setting.GetDeviceId())
                              .arg(device_time_setting.GetOffset())
                              .arg("00:30")
                              .arg("23:00");
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    ActTimeSettingTable &time_setting_table = time_setting_tables[device_time_setting.GetDeviceId()];
    time_setting_table.SetClockSource(device_time_setting.GetClockSource());
    time_setting_table.SetNTPTimeServer1(device_time_setting.GetNTPTimeServer1());
    time_setting_table.SetNTPTimeServer2(device_time_setting.GetNTPTimeServer2());
    time_setting_table.SetSNTPTimeServer1(device_time_setting.GetSNTPTimeServer1());
    time_setting_table.SetSNTPTimeServer2(device_time_setting.GetSNTPTimeServer2());
    time_setting_table.SetTimeZone(device_time_setting.GetTimeZone());
    time_setting_table.SetDaylightSavingTime(device_time_setting.GetDaylightSavingTime());
    time_setting_table.SetOffset(device_time_setting.GetOffset());
    time_setting_table.SetStart(device_time_setting.GetStart());
    time_setting_table.SetEnd(device_time_setting.GetEnd());
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetDevicePortSetting(qint64 &project_id, qint64 &device_id,
                                         ActDevicePortSetting &device_port_setting, bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActPortSettingTable> &port_setting_tables = project.GetDeviceConfig().GetPortSettingTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetId() != device_id) {
      continue;
    } else if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      QString error_msg = QString("Device %1 is not a switch").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(error_msg);
    }
    QMap<qint64, ActDevicePortStatus> port_status_map;
    for (ActInterface interface : device.GetInterfaces()) {
      port_status_map.insert(
          interface.GetInterfaceId(),
          ActDevicePortStatus(interface.GetActive(), interface.GetInterfaceId(), interface.GetInterfaceName()));
    }

    ActPortSettingTable port_setting_table(device);
    if (port_setting_tables.contains(device.GetId())) {
      port_setting_table = port_setting_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetPortSettingTables().isEmpty()) {
        port_setting_table = device_profile.GetDefaultDeviceConfig().GetPortSettingTables().first();
      }
      port_setting_tables[device.GetId()] = port_setting_table;  // insert config to tables
    }

    device_port_setting = ActDevicePortSetting(device);
    for (ActPortSettingEntry port_setting_entry : port_setting_table.GetPortSettingEntries()) {
      ActDevicePortStatus &port_status = port_status_map[port_setting_entry.GetPortId()];
      port_status.SetAdminStatus(port_setting_entry.GetAdminStatus());
      device_port_setting.GetPortList().append(port_status);
    }
    return ACT_STATUS_SUCCESS;
  }
  return std::make_shared<ActStatusNotFound>(QString("Device %1 in project is not found").arg(device_id));
}

ACT_STATUS ActCore::GetDevicePortSettings(qint64 &project_id, ActDevicePortSettingList &device_port_setting_list,
                                          bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActDevicePortSetting> &device_port_setting_list_ = device_port_setting_list.GetDevicePortSettingList();
  QMap<qint64, ActPortSettingTable> &port_setting_tables = project.GetDeviceConfig().GetPortSettingTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }
    QMap<qint64, ActDevicePortStatus> port_map;
    for (ActInterface interface : device.GetInterfaces()) {
      port_map.insert(interface.GetInterfaceId(), ActDevicePortStatus(interface.GetActive(), interface.GetInterfaceId(),
                                                                      interface.GetInterfaceName()));
    }

    ActPortSettingTable port_setting_table(device);
    if (port_setting_tables.contains(device.GetId())) {
      port_setting_table = port_setting_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetPortSettingTables().isEmpty()) {
        port_setting_table = device_profile.GetDefaultDeviceConfig().GetPortSettingTables().first();
      }
      port_setting_tables[device.GetId()] = port_setting_table;  // insert config to tables
    }

    ActDevicePortSetting device_port_setting(device);
    for (ActPortSettingEntry port_setting_entry : port_setting_table.GetPortSettingEntries()) {
      ActDevicePortStatus &port_status = port_map[port_setting_entry.GetPortId()];
      port_status.SetAdminStatus(port_setting_entry.GetAdminStatus());
      device_port_setting.GetPortList().append(port_status);
    }
    device_port_setting_list_.append(device_port_setting);
  }
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateDevicePortSettings(qint64 &project_id, ActDevicePortSettingList &device_port_setting_list,
                                             bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActDeviceConfig &device_config = project.GetDeviceConfig();

  QMap<qint64, ActPortSettingTable> &port_setting_tables = device_config.GetPortSettingTables();

  // Check device port setting list is empty
  auto port_setting_list = device_port_setting_list.GetDevicePortSettingList();
  if (port_setting_list.isEmpty()) {
    QString error_msg = QString("Device port setting list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (ActDevicePortSetting device_port_setting : port_setting_list) {
    qint64 device_id = device_port_setting.GetDeviceId();
    ActDevice device;
    act_status = project.GetDeviceById(device, device_id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Get device failed with device id:" << device_id;
      return act_status;
    }
    ActPortSettingTable &port_setting_table = port_setting_tables[device_port_setting.GetDeviceId()];
    QSet<ActPortSettingEntry> &port_setting_entries = port_setting_table.GetPortSettingEntries();
    QSet<ActPortSettingEntry>::iterator port_setting_entry_iter;
    for (ActDevicePortStatus port_status : device_port_setting.GetPortList()) {
      if (port_status.GetPortId() <= 0 || port_status.GetPortId() > device.GetInterfaces().size()) {
        continue;
      }
      ActPortSettingEntry port_setting_entry(port_status.GetPortId(), port_status.GetAdminStatus());
      port_setting_entry_iter = port_setting_entries.find(port_setting_entry);
      if (port_setting_entry_iter != port_setting_entries.end()) {
        port_setting_entries.erase(port_setting_entry_iter);
      }
      port_setting_entries.insert(port_setting_entry);
    }
    port_setting_table.SetPortSettingEntries(port_setting_entries);
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetDeviceIpSetting(qint64 &project_id, qint64 &device_id, ActDeviceIpSetting &device_ip_setting,
                                       bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActNetworkSettingTable> &network_setting_tables = project.GetDeviceConfig().GetNetworkSettingTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetId() != device_id) {
      continue;
    } else if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      QString error_msg = QString("Device %1 is not a switch").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(error_msg);
    }

    ActNetworkSettingTable network_setting_table(device);
    if (network_setting_tables.contains(device.GetId())) {
      network_setting_table = network_setting_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetNetworkSettingTables().isEmpty()) {
        network_setting_table = device_profile.GetDefaultDeviceConfig().GetNetworkSettingTables().first();
      }
      network_setting_tables[device.GetId()] = network_setting_table;  // insert config to tables
    }

    device_ip_setting = ActDeviceIpSetting(device);
    device_ip_setting.SetSubnetMask(network_setting_table.GetSubnetMask());
    device_ip_setting.SetGateway(network_setting_table.GetGateway());
    device_ip_setting.SetDNS1(network_setting_table.GetDNS1());
    device_ip_setting.SetDNS2(network_setting_table.GetDNS2());
    return ACT_STATUS_SUCCESS;
  }
  return std::make_shared<ActStatusNotFound>(QString("Device %1 in project is not found").arg(device_id));
}

ACT_STATUS ActCore::GetDeviceIpSettings(qint64 &project_id, ActDeviceIpSettingList &device_ip_setting_list,
                                        bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActDeviceIpSetting> &device_ip_setting_list_ = device_ip_setting_list.GetDeviceIpSettingList();
  QMap<qint64, ActNetworkSettingTable> &network_setting_tables = project.GetDeviceConfig().GetNetworkSettingTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }

    ActNetworkSettingTable network_setting_table(device);
    if (network_setting_tables.contains(device.GetId())) {
      network_setting_table = network_setting_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetNetworkSettingTables().isEmpty()) {
        network_setting_table = device_profile.GetDefaultDeviceConfig().GetNetworkSettingTables().first();
      }
      network_setting_tables[device.GetId()] = network_setting_table;  // insert config to tables
    }

    ActDeviceIpSetting device_ip_setting(device);
    device_ip_setting.SetSubnetMask(network_setting_table.GetSubnetMask());
    device_ip_setting.SetGateway(network_setting_table.GetGateway());
    device_ip_setting.SetDNS1(network_setting_table.GetDNS1());
    device_ip_setting.SetDNS2(network_setting_table.GetDNS2());
    device_ip_setting_list_.append(device_ip_setting);
  }
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateDeviceIpSettings(qint64 &project_id, ActDeviceIpSettingList &device_ip_setting_list,
                                           bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActDeviceConfig &device_config = project.GetDeviceConfig();

  QMap<qint64, ActNetworkSettingTable> &network_setting_tables = device_config.GetNetworkSettingTables();

  // Check device IP setting list is empty
  auto ip_setting_list = device_ip_setting_list.GetDeviceIpSettingList();
  if (ip_setting_list.isEmpty()) {
    QString error_msg = QString("Device IP setting list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (ActDeviceIpSetting device_ip_setting : ip_setting_list) {
    ActNetworkSettingTable &network_setting_table = network_setting_tables[device_ip_setting.GetDeviceId()];
    network_setting_table.SetIpAddress(device_ip_setting.GetDeviceIp());
    network_setting_table.SetSubnetMask(device_ip_setting.GetSubnetMask());
    network_setting_table.SetGateway(device_ip_setting.GetGateway());
    network_setting_table.SetDNS1(device_ip_setting.GetDNS1());
    network_setting_table.SetDNS2(device_ip_setting.GetDNS2());

    if (project.GetProjectMode() != ActProjectModeEnum::kOperation) {
      ActDevice device;
      act_status = project.GetDeviceById(device, device_ip_setting.GetDeviceId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Get device failed with device id:" << device_ip_setting.GetDeviceId();
        return act_status;
      }

      ActIpv4 ipv4;
      ipv4.SetIpAddress(device_ip_setting.GetDeviceIp());
      ipv4.SetSubnetMask(device_ip_setting.GetSubnetMask());
      ipv4.SetGateway(device_ip_setting.GetGateway());
      ipv4.SetDNS1(device_ip_setting.GetDNS1());
      ipv4.SetDNS2(device_ip_setting.GetDNS2());
      device.SetIpv4(ipv4);

      act_status = this->UpdateDevice(project, device);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Update device failed with device id:" << device.GetId();
        return act_status;
      }
      // Notify the user that the device be update
      ActDevicePatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, project_id, device, true);
      this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project_id);
    }
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetDeviceRstpSetting(qint64 &project_id, qint64 &device_id,
                                         ActDeviceRstpSetting &device_rstp_setting, bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActRstpTable> &rstp_tables = project.GetDeviceConfig().GetRstpTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetId() != device_id) {
      continue;
    } else if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      QString error_msg = QString("Device %1 is not a switch").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(error_msg);
    }

    ActRstpTable rstp_table(device);
    if (rstp_tables.contains(device.GetId())) {
      rstp_table = rstp_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetRstpTables().isEmpty()) {
        rstp_table = device_profile.GetDefaultDeviceConfig().GetRstpTables().first();
      }
      rstp_tables[device.GetId()] = rstp_table;  // insert config to tables
    }

    device_rstp_setting = ActDeviceRstpSetting(device);
    device_rstp_setting.SetStpRstp(rstp_table.GetActive());
    device_rstp_setting.SetCompatibility(rstp_table.GetSpanningTreeVersion());
    device_rstp_setting.SetPriority(rstp_table.GetPriority());
    device_rstp_setting.SetForwardDelay(rstp_table.GetForwardDelay());
    device_rstp_setting.SetHelloTime(rstp_table.GetHelloTime());
    device_rstp_setting.SetMaxAge(rstp_table.GetMaxAge());
    device_rstp_setting.SetRstpErrorRecoveryTime(rstp_table.GetRstpErrorRecoveryTime());
    device_rstp_setting.SetRstpConfigSwift(rstp_table.GetRstpConfigSwift());
    device_rstp_setting.SetRstpConfigRevert(rstp_table.GetRstpConfigRevert());

    for (ActRstpPortEntry rstp_port_entry : rstp_table.GetRstpPortEntries()) {
      ActDeviceRstpPortEntry device_rstp_port_entry(rstp_port_entry.GetPortId(), device);
      device_rstp_port_entry.SetRstpEnable(rstp_port_entry.GetRstpEnable());
      device_rstp_port_entry.SetEdge(rstp_port_entry.GetEdge());
      device_rstp_port_entry.SetPortPriority(rstp_port_entry.GetPortPriority());
      device_rstp_port_entry.SetPathCost(rstp_port_entry.GetPathCost());
      device_rstp_port_entry.SetLinkType(rstp_port_entry.GetLinkType());
      device_rstp_port_entry.SetBpduGuard(rstp_port_entry.GetBpduGuard());
      device_rstp_port_entry.SetRootGuard(rstp_port_entry.GetRootGuard());
      device_rstp_port_entry.SetLoopGuard(rstp_port_entry.GetLoopGuard());
      device_rstp_port_entry.SetBpduFilter(rstp_port_entry.GetBpduFilter());
      device_rstp_setting.GetRstpPortEntries().insert(device_rstp_port_entry);
    }

    return ACT_STATUS_SUCCESS;
  }
  return std::make_shared<ActStatusNotFound>(QString("Device %1 in project is not found").arg(device_id));
}

ACT_STATUS ActCore::GetDeviceRstpSettings(qint64 &project_id, ActDeviceRstpSettingList &device_rstp_setting_list,
                                          bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActDeviceRstpSetting> &device_rstp_setting_list_ = device_rstp_setting_list.GetDeviceRstpSettingList();
  QMap<qint64, ActRstpTable> &rstp_tables = project.GetDeviceConfig().GetRstpTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }
    // if (!rstp_tables.contains(device.GetId())) {
    //   rstp_tables.insert(device.GetId(), ActRstpTable(device));
    // }

    ActRstpTable rstp_table(device);
    if (rstp_tables.contains(device.GetId())) {
      rstp_table = rstp_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetRstpTables().isEmpty()) {
        rstp_table = device_profile.GetDefaultDeviceConfig().GetRstpTables().first();
      }
      rstp_tables[device.GetId()] = rstp_table;  // insert config to tables
    }

    ActDeviceRstpSetting device_rstp_setting(device);
    device_rstp_setting.SetStpRstp(rstp_table.GetActive());
    device_rstp_setting.SetCompatibility(rstp_table.GetSpanningTreeVersion());
    device_rstp_setting.SetPriority(rstp_table.GetPriority());
    device_rstp_setting.SetForwardDelay(rstp_table.GetForwardDelay());
    device_rstp_setting.SetHelloTime(rstp_table.GetHelloTime());
    device_rstp_setting.SetMaxAge(rstp_table.GetMaxAge());
    device_rstp_setting.SetRstpErrorRecoveryTime(rstp_table.GetRstpErrorRecoveryTime());
    device_rstp_setting.SetRstpConfigSwift(rstp_table.GetRstpConfigSwift());
    device_rstp_setting.SetRstpConfigRevert(rstp_table.GetRstpConfigRevert());

    for (ActRstpPortEntry rstp_port_entry : rstp_table.GetRstpPortEntries()) {
      ActDeviceRstpPortEntry device_rstp_port_entry(rstp_port_entry.GetPortId(), device);
      device_rstp_port_entry.SetRstpEnable(rstp_port_entry.GetRstpEnable());
      device_rstp_port_entry.SetEdge(rstp_port_entry.GetEdge());
      device_rstp_port_entry.SetPortPriority(rstp_port_entry.GetPortPriority());
      device_rstp_port_entry.SetPathCost(rstp_port_entry.GetPathCost());
      device_rstp_port_entry.SetLinkType(rstp_port_entry.GetLinkType());
      device_rstp_port_entry.SetBpduGuard(rstp_port_entry.GetBpduGuard());
      device_rstp_port_entry.SetRootGuard(rstp_port_entry.GetRootGuard());
      device_rstp_port_entry.SetLoopGuard(rstp_port_entry.GetLoopGuard());
      device_rstp_port_entry.SetBpduFilter(rstp_port_entry.GetBpduFilter());
      device_rstp_setting.GetRstpPortEntries().insert(device_rstp_port_entry);
    }

    device_rstp_setting_list_.append(device_rstp_setting);
  }
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateDeviceRstpSettings(qint64 &project_id, ActDeviceRstpSettingList &device_rstp_setting_list,
                                             bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActDeviceConfig &device_config = project.GetDeviceConfig();

  QMap<qint64, ActRstpTable> &rstp_tables = device_config.GetRstpTables();

  // Check device RSTP setting list is empty
  auto rstp_setting_list = device_rstp_setting_list.GetDeviceRstpSettingList();
  if (rstp_setting_list.isEmpty()) {
    QString error_msg = QString("Device RSTP setting list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (ActDeviceRstpSetting device_rstp_setting : rstp_setting_list) {
    if ((device_rstp_setting.GetForwardDelay() - 1) * 2 < device_rstp_setting.GetMaxAge() ||
        device_rstp_setting.GetMaxAge() < device_rstp_setting.GetHelloTime() * 2) {
      QString error_msg =
          QString("Should follow this protocol rule: 2*(Forward time - 1) >= Max Age time >= 2*(Hello time)");
      return std::make_shared<ActBadRequest>(error_msg);
    }
    ActDevice device;
    act_status = project.GetDeviceById(device, device_rstp_setting.GetDeviceId());
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Get device failed with device id:" << device_rstp_setting.GetDeviceId();
      return act_status;
    }
    if (!rstp_tables.contains(device_rstp_setting.GetDeviceId())) {
      rstp_tables.insert(device_rstp_setting.GetDeviceId(), ActRstpTable(device));
    }
    ActRstpTable &rstp_table = rstp_tables[device_rstp_setting.GetDeviceId()];
    rstp_table.SetActive(device_rstp_setting.GetStpRstp());
    rstp_table.SetSpanningTreeVersion(device_rstp_setting.GetCompatibility());
    rstp_table.SetPriority(device_rstp_setting.GetPriority());
    rstp_table.SetForwardDelay(device_rstp_setting.GetForwardDelay());
    rstp_table.SetHelloTime(device_rstp_setting.GetHelloTime());
    rstp_table.SetMaxAge(device_rstp_setting.GetMaxAge());
    rstp_table.SetRstpErrorRecoveryTime(device_rstp_setting.GetRstpErrorRecoveryTime());
    if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetSwift()) {
      rstp_table.SetRstpConfigSwift(device_rstp_setting.GetRstpConfigSwift());
      rstp_table.SetRstpConfigRevert(device_rstp_setting.GetRstpConfigRevert());
    }
    QSet<ActRstpPortEntry> &rstp_port_entry_set = rstp_table.GetRstpPortEntries();
    QSet<ActRstpPortEntry>::iterator rstp_port_entry_iter;
    for (ActDeviceRstpPortEntry device_rstp_port_entry : device_rstp_setting.GetRstpPortEntries()) {
      if (device_rstp_port_entry.GetPortId() <= 0 ||
          device_rstp_port_entry.GetPortId() > device.GetInterfaces().size()) {
        continue;
      }
      if (device_rstp_port_entry.GetRootGuard() && device_rstp_port_entry.GetLoopGuard()) {
        return std::make_shared<ActBadRequest>(
            QString("Device(%1)-Port(%2): Root Guard & Loop Guard cannot enable at the same time.")
                .arg(device_rstp_setting.GetDeviceIp())
                .arg(device_rstp_port_entry.GetPortId()));
      }
      ActRstpPortEntry rstp_port_entry(device_rstp_port_entry.GetPortId());
      rstp_port_entry.SetEdge(device_rstp_port_entry.GetEdge());
      rstp_port_entry.SetPortPriority(device_rstp_port_entry.GetPortPriority());
      rstp_port_entry.SetPathCost(device_rstp_port_entry.GetPathCost());
      rstp_port_entry.SetRstpEnable(device_rstp_port_entry.GetRstpEnable());
      rstp_port_entry.SetLinkType(device_rstp_port_entry.GetLinkType());
      rstp_port_entry.SetBpduGuard(device_rstp_port_entry.GetBpduGuard());
      rstp_port_entry.SetRootGuard(device_rstp_port_entry.GetRootGuard());
      rstp_port_entry.SetLoopGuard(device_rstp_port_entry.GetLoopGuard());
      rstp_port_entry.SetBpduFilter(device_rstp_port_entry.GetBpduFilter());

      rstp_port_entry_iter = rstp_port_entry_set.find(rstp_port_entry);
      if (rstp_port_entry_iter != rstp_port_entry_set.end()) {
        rstp_port_entry_set.erase(rstp_port_entry_iter);
      }
      rstp_port_entry_set.insert(rstp_port_entry);
    }
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetDevicePerStreamPrioritySetting(qint64 &project_id, qint64 &device_id,
                                                      ActDevicePerStreamPrioritySetting &device_setting,
                                                      bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActStadPortTable> &setting_tables = project.GetDeviceConfig().GetStreamPriorityIngressTables();

  for (ActDevice device : project.GetDevices()) {
    if (device.GetId() != device_id) {
      continue;
    } else if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      QString error_msg = QString("Device %1 is not a switch").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(error_msg);
    }

    auto device_id = device.GetId();
    ActStadPortTable setting_table(device);

    if (setting_tables.contains(device_id)) {
      setting_table = setting_tables[device_id];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetStreamPriorityIngressTables().isEmpty()) {
        setting_table = device_profile.GetDefaultDeviceConfig().GetStreamPriorityIngressTables().first();
      }
      setting_tables[device_id] = setting_table;  // insert config to tables
    }

    device_setting = ActDevicePerStreamPrioritySetting(device);

    for (ActInterfaceStadPortEntry entry : setting_table.GetInterfaceStadPortEntries()) {
      for (ActStadPortEntry port_entry : entry.GetStadPortEntries()) {
        ActDevicePerStreamPriorityEntry device_entry;

        auto port_id = port_entry.GetPortId();
        QString port_name = "";
        for (ActInterface intf : device.GetInterfaces()) {
          if (intf.GetInterfaceId() == port_id) {
            port_name = intf.GetInterfaceName();
          }
        }

        device_entry.SetPortId(port_id);
        device_entry.SetPortName(port_name);

        // if index enable = false
        if (port_entry.GetIndexEnable() == 2) {
          device_entry.SetType(ActStreamPriorityTypeEnum::kInactive);
        } else {
          QSet<ActStreamPriorityTypeEnum> port_entries = port_entry.GetType();
          if (!port_entries.isEmpty()) {
            device_entry.SetType(*port_entries.begin());
          }
        }

        device_entry.SetEtherType(port_entry.GetEthertypeValue());
        if (port_entry.GetSubtypeEnable() == 1) {
          device_entry.SetEnableSubType(true);
        } else {
          device_entry.SetEnableSubType(false);
        }

        device_entry.SetSubType(port_entry.GetSubtypeValue());

        device_entry.SetUdpPort(port_entry.GetUdpPort());
        device_entry.SetTcpPort(port_entry.GetTcpPort());

        device_entry.SetVlanId(port_entry.GetVlanId());
        device_entry.SetPriorityCodePoint(port_entry.GetVlanPcp());

        device_setting.GetPerStreamPrioritySetting().append(device_entry);
      }
    }
    return ACT_STATUS_SUCCESS;
  }
  // update (device) not found
  return std::make_shared<ActStatusNotFound>(QString("Device %1 in project is not found").arg(device_id));
}

ACT_STATUS ActCore::GetDevicePerStreamPrioritySettings(qint64 &project_id,
                                                       ActDevicePerStreamPrioritySettingList &setting_list,
                                                       bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActDevicePerStreamPrioritySetting> &setting_list_ = setting_list.GetDevicePerStreamPrioritySettingList();

  QMap<qint64, ActStadPortTable> &setting_tables = project.GetDeviceConfig().GetStreamPriorityIngressTables();
  QMap<qint64, ActVlanTable> vlan_setting_tables = project.GetDeviceConfig().GetVlanTables();

  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }

    auto device_id = device.GetId();
    ActStadPortTable setting_table(device);

    if (setting_tables.contains(device_id)) {
      setting_table = setting_tables[device_id];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetStreamPriorityIngressTables().isEmpty()) {
        setting_table = device_profile.GetDefaultDeviceConfig().GetStreamPriorityIngressTables().first();
      }
      setting_tables[device_id] = setting_table;  // insert config to tables
    }

    ActDevicePerStreamPrioritySetting device_setting(device);

    for (ActInterfaceStadPortEntry entry : setting_table.GetInterfaceStadPortEntries()) {
      for (ActStadPortEntry port_entry : entry.GetStadPortEntries()) {
        ActDevicePerStreamPriorityEntry device_entry;

        auto port_id = port_entry.GetPortId();
        QString port_name = "";
        for (ActInterface intf : device.GetInterfaces()) {
          if (intf.GetInterfaceId() == port_id) {
            port_name = intf.GetInterfaceName();
          }
        }

        device_entry.SetPortId(port_id);
        device_entry.SetPortName(port_name);

        // if index enable = false
        if (port_entry.GetIndexEnable() == 2) {
          device_entry.SetType(ActStreamPriorityTypeEnum::kInactive);
        } else {
          QSet<ActStreamPriorityTypeEnum> port_entries = port_entry.GetType();
          if (!port_entries.isEmpty()) {
            device_entry.SetType(*port_entries.begin());
          }
        }

        device_entry.SetEtherType(port_entry.GetEthertypeValue());
        if (port_entry.GetSubtypeEnable() == 1) {
          device_entry.SetEnableSubType(true);
        } else {
          device_entry.SetEnableSubType(false);
        }

        device_entry.SetSubType(port_entry.GetSubtypeValue());

        device_entry.SetUdpPort(port_entry.GetUdpPort());
        device_entry.SetTcpPort(port_entry.GetTcpPort());

        device_entry.SetVlanId(port_entry.GetVlanId());
        device_entry.SetPriorityCodePoint(port_entry.GetVlanPcp());

        device_setting.GetPerStreamPrioritySetting().append(device_entry);
      }
    }

    // fill available VLANs in port list
    auto vlan_setting_table = vlan_setting_tables[device_id];
    QSet<ActVlanStaticEntry> static_entry_set = vlan_setting_table.GetVlanStaticEntries();

    QList<ActDevicePerStreamPriorityPortListEntry> &port_list = device_setting.GetPortList();

    // static_entries turn to list to order vlan
    QList<ActVlanStaticEntry> static_entry_list = static_entry_set.values();
    std::sort(static_entry_list.begin(), static_entry_list.end(),
              [](ActVlanStaticEntry &a, ActVlanStaticEntry &b) { return a.GetVlanId() < b.GetVlanId(); });

    for (ActVlanStaticEntry static_entry : static_entry_list) {
      qint32 vid = static_entry.GetVlanId();
      QSet<qint64> egress_port_set = static_entry.GetEgressPorts();

      for (auto &port_entry : port_list) {
        QList<qint32> &available_vlans = port_entry.GetAvailableVlans();
        if (egress_port_set.contains(port_entry.GetPortId())) {
          available_vlans.append(vid);
        }
      }
    }

    setting_list_.append(device_setting);
  }
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateDevicePerStreamPrioritySettings(qint64 &project_id,
                                                          ActDevicePerStreamPrioritySettingList &setting_list,
                                                          bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Get per stream priority device config
  ActDeviceConfig &device_config = project.GetDeviceConfig();
  QMap<qint64, ActStadPortTable> &setting_tables = device_config.GetStreamPriorityIngressTables();

  // Check device per-stream priority setting list is empty
  auto setting_list_ = setting_list.GetDevicePerStreamPrioritySettingList();
  if (setting_list_.isEmpty()) {
    QString error_msg = QString("Device per-stream priority setting list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // one device setting in setting list
  for (ActDevicePerStreamPrioritySetting setting : setting_list_) {
    // Get device id & device
    auto device_id = setting.GetDeviceId();
    ActDevice device;
    act_status = project.GetDeviceById(device, device_id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Get device failed with device id:" << device_id;
      return act_status;
    }

    // map[port_id, entries of the port]
    QMap<qint64, QList<ActStadPortEntry>> port_entry_map;

    // input_port_entry_map[portId, input entries of the port]
    QMap<qint64, QList<ActDevicePerStreamPriorityEntry>> input_port_entry_map;

    // each entry input check
    for (ActDevicePerStreamPriorityEntry device_setting_entry : setting.GetPerStreamPrioritySetting()) {
      // check input
      qint32 ethertype = device_setting_entry.GetEtherType();
      qint32 subtype = device_setting_entry.GetSubType();
      qint32 tcp_port = device_setting_entry.GetTcpPort();
      qint32 udp_port = device_setting_entry.GetUdpPort();
      qint32 vid = device_setting_entry.GetVlanId();
      qint32 pcp = device_setting_entry.GetPriorityCodePoint();

      // ethertype: 0~65535 (0xFFFF)
      if (ethertype < 0 || ethertype > 65535) {
        QString error_msg = QString("Ethertype of device id %1 out of range: %2").arg(device_id).arg(ethertype);
        return std::make_shared<ActBadRequest>(error_msg);
      }
      // subtype: 0~255 (0xFF)
      if (subtype < 0 || subtype > 255) {
        QString error_msg = QString("Subtype of device id %1 out of range: %2").arg(device_id).arg(subtype);
        return std::make_shared<ActBadRequest>(error_msg);
      }
      // tcp port: 1~65535
      if (tcp_port < 1 || tcp_port > 65535) {
        QString error_msg = QString("TCP port of device id %1 out of range: %2").arg(device_id).arg(tcp_port);
        return std::make_shared<ActBadRequest>(error_msg);
      }
      // udp port: 1~65535
      if (udp_port < 1 || udp_port > 65535) {
        QString error_msg = QString("UDP port of device id %1 out of range: %2").arg(device_id).arg(udp_port);
        return std::make_shared<ActBadRequest>(error_msg);
      }
      // VLAN ID: 1~4094
      if (vid < 1 || vid > 4094) {
        QString error_msg = QString("VLAN ID of device id %1 out of range: %2").arg(device_id).arg(vid);
        return std::make_shared<ActBadRequest>(error_msg);
      }
      // PCP: 0~7
      if (pcp < 0 || pcp > 7) {
        QString error_msg = QString("PCP of device id %1 out of range: %2").arg(device_id).arg(pcp);
        return std::make_shared<ActBadRequest>(error_msg);
      }

      qint64 port_id = device_setting_entry.GetPortId();
      input_port_entry_map[port_id].append(device_setting_entry);
    }

    qint32 total_entry_count = 0;
    for (auto it = input_port_entry_map.constBegin(); it != input_port_entry_map.constEnd(); ++it) {
      qint64 port_id = it.key();
      QList<ActDevicePerStreamPriorityEntry> input_entries = it.value();

      total_entry_count = total_entry_count + input_entries.size();

      // check input size (per port)
      if (input_entries.size() > 10) {
        QString error_msg = QString("Per-stream priority setting of device %1 on port %2 out of range: %3")
                                .arg(device_id)
                                .arg(port_id)
                                .arg(input_entries.size());
        return std::make_shared<ActBadRequest>(error_msg);
      }

      // check entry unique
      for (int i = 0; i < input_entries.size(); ++i) {
        for (int j = i + 1; j < input_entries.size(); ++j) {
          if (input_entries[i] == input_entries[j]) {
            QString type = kActStreamPriorityTypeEnumMap.key(input_entries[i].GetType());
            QString error_msg =
                QStringLiteral("Per-stream priority duplicate found at port %1 (type %2)").arg(port_id).arg(type);
            return std::make_shared<ActBadRequest>(error_msg);
          }
        }
      }

      qint32 index = 0;

      for (ActDevicePerStreamPriorityEntry input_entry : input_entries) {
        ActStadPortEntry entry(port_id, index);
        index++;

        if (input_entry.GetType() == ActStreamPriorityTypeEnum::kInactive) {
          // (INTEGER(true(1), false(2))) (enable)
          entry.SetIndexEnable(2);
        } else {
          entry.SetIndexEnable(1);
        }

        entry.SetVlanId(input_entry.GetVlanId());
        entry.SetVlanPcp(input_entry.GetPriorityCodePoint());
        entry.SetEthertypeValue(input_entry.GetEtherType());

        if (input_entry.GetEnableSubType()) {
          // (INTEGER(true(1), false(2))) (enable)
          entry.SetSubtypeEnable(1);
        } else {
          entry.SetSubtypeEnable(2);
        }

        QSet<ActStreamPriorityTypeEnum> type_set;
        type_set.insert(input_entry.GetType());
        entry.SetType(type_set);

        entry.SetUdpPort(input_entry.GetUdpPort());
        entry.SetTcpPort(input_entry.GetTcpPort());

        port_entry_map[port_id].append(entry);
      }
    }

    // check input size (per device)
    if (total_entry_count > 40) {
      QString error_msg =
          QString("Per-stream priority setting of device %1 out of range: %2)").arg(device_id).arg(total_entry_count);
      return std::make_shared<ActBadRequest>(error_msg);
    }

    // handle ActInterfaceStadPortEntry
    QSet<ActInterfaceStadPortEntry> interface_port_entries;

    for (auto it = port_entry_map.constBegin(); it != port_entry_map.constEnd(); ++it) {
      qint64 port_id = it.key();
      QList<ActStadPortEntry> entry_list = it.value();

      QSet<ActStadPortEntry> entry_set;
      for (ActStadPortEntry entry : entry_list) {
        entry_set.insert(entry);
      }

      ActInterfaceStadPortEntry port_entry;
      port_entry.SetInterfaceId(port_id);
      port_entry.SetStadPortEntries(entry_set);

      interface_port_entries.insert(port_entry);
    }

    // fill into device config
    if (!setting_tables.contains(device_id)) {
      setting_tables[device_id] = ActStadPortTable(device_id);
    }
    ActStadPortTable &setting_table = setting_tables[device_id];

    setting_table.SetInterfaceStadPortEntries(interface_port_entries);
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::GetDeviceTimeSlotSetting(qint64 &project_id, qint64 &device_id,
                                             ActDeviceTimeSlotSetting &device_setting, bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QMap<qint64, ActGclTable> &time_slot_setting_tables = project.GetDeviceConfig().GetGCLTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetId() != device_id) {
      continue;
    } else if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
               device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      QString error_msg = QString("Device %1 is not a switch").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(error_msg);
    }

    ActGclTable time_slot_setting_table(device);
    if (time_slot_setting_tables.contains(device.GetId())) {
      time_slot_setting_table = time_slot_setting_tables[device.GetId()];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetGCLTables().isEmpty()) {
        time_slot_setting_table = device_profile.GetDefaultDeviceConfig().GetGCLTables().first();
      }
      time_slot_setting_tables[device.GetId()] = time_slot_setting_table;  // insert config to tables
    }

    device_setting = ActDeviceTimeSlotSetting(device);
    QList<ActDeviceTimeSlotPortEntry> device_port_list;
    QSet<ActInterfaceGateParameters> port_entry_set = time_slot_setting_table.GetInterfacesGateParameters();
    // QSet -> QList & sort by interface id
    QList<ActInterfaceGateParameters> port_entries = port_entry_set.values();
    std::sort(port_entries.begin(), port_entries.end(),
              [](const ActInterfaceGateParameters &a, const ActInterfaceGateParameters &b) {
                return a.GetInterfaceId() < b.GetInterfaceId();
              });

    for (ActInterfaceGateParameters port_entry : port_entries) {
      // get port id and port name
      qint64 port_id = port_entry.GetInterfaceId();
      QString port_name = "";
      for (ActInterface intf : device.GetInterfaces()) {
        if (intf.GetInterfaceId() == port_id) {
          port_name = intf.GetInterfaceName();
        }
      }
      ActDeviceTimeSlotPortEntry device_port_entry;
      device_port_entry.SetPortId(port_id);
      device_port_entry.SetPortName(port_name);

      ActGateParameters gate_params = port_entry.GetGateParameters();

      ActAdminCycleTime admin_cycle_time = gate_params.GetAdminCycleTime();
      QString cycle_time_str = QString::number(admin_cycle_time.ToMicroseconds(), 'f', 3);
      device_port_entry.SetCycleTime(cycle_time_str.toDouble());

      device_port_entry.SetActive(gate_params.GetGateEnabled());

      QList<ActDeviceTimeSlot> device_time_list;
      QList<ActAdminControl> gcl = gate_params.GetAdminControlList();
      for (ActAdminControl time_slot : gcl) {
        ActDeviceTimeSlot device_time_slot;
        device_time_slot.SetSlotId(time_slot.GetIndex());

        ActSGSParams params = time_slot.GetSgsParams();
        QString interval_str = QString::number(params.GetTimeIntervalValue() / 1000.0, 'f', 3);
        device_time_slot.SetInterval(interval_str.toDouble());
        device_time_slot.SetQueueSet(params.GetQueueSetByGateStateValue(params.GetGateStatesValue()));

        device_time_list.append(device_time_slot);
      }
      device_port_entry.SetGateControlList(device_time_list);

      device_port_list.append(device_port_entry);
    }
    device_setting.SetPortList(device_port_list);

    return ACT_STATUS_SUCCESS;
  }
  return std::make_shared<ActStatusNotFound>(QString("Device %1 in project is not found").arg(device_id));
}

ACT_STATUS ActCore::GetDeviceTimeSlotSettings(qint64 &project_id, ActDeviceTimeSlotSettingList &setting_list,
                                              bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActDeviceTimeSlotSetting> &setting_list_ = setting_list.GetDeviceTimeSlotSettingList();
  QMap<qint64, ActGclTable> &setting_tables = project.GetDeviceConfig().GetGCLTables();
  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }

    auto device_id = device.GetId();
    ActGclTable setting_table(device);

    if (setting_tables.contains(device_id)) {
      setting_table = setting_tables[device_id];
    } else {
      // Get Default Config in the DeviceProfile
      ActDeviceProfile device_profile;
      act_status =
          ActGetItemById<ActDeviceProfile>(this->GetDeviceProfileSet(), device.GetDeviceProfileId(), device_profile);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      if (!device_profile.GetDefaultDeviceConfig().GetGCLTables().isEmpty()) {
        setting_table = device_profile.GetDefaultDeviceConfig().GetGCLTables().first();
      }
      setting_tables[device_id] = setting_table;  // insert config to tables
    }

    ActDeviceTimeSlotSetting device_setting(device);
    QList<ActDeviceTimeSlotPortEntry> device_port_list;
    QSet<ActInterfaceGateParameters> port_entries = setting_table.GetInterfacesGateParameters();
    for (ActInterfaceGateParameters port_entry : port_entries) {
      // get port id and port name
      qint64 port_id = port_entry.GetInterfaceId();
      QString port_name = "";
      for (ActInterface intf : device.GetInterfaces()) {
        if (intf.GetInterfaceId() == port_id) {
          port_name = intf.GetInterfaceName();
        }
      }
      ActDeviceTimeSlotPortEntry device_port_entry;
      device_port_entry.SetPortId(port_id);
      device_port_entry.SetPortName(port_name);

      ActGateParameters gate_params = port_entry.GetGateParameters();

      ActAdminCycleTime admin_cycle_time = gate_params.GetAdminCycleTime();
      device_port_entry.SetCycleTime(admin_cycle_time.ToMicroseconds());

      device_port_entry.SetActive(gate_params.GetGateEnabled());

      QList<ActDeviceTimeSlot> device_time_list;
      QList<ActAdminControl> gcl = gate_params.GetAdminControlList();
      for (ActAdminControl time_slot : gcl) {
        ActDeviceTimeSlot device_time_slot;
        device_time_slot.SetSlotId(time_slot.GetIndex());

        ActSGSParams params = time_slot.GetSgsParams();
        QString interval_str = QString::number(params.GetTimeIntervalValue() / 1000.0, 'f', 3);
        device_time_slot.SetInterval(interval_str.toDouble());
        device_time_slot.SetQueueSet(params.GetQueueSetByGateStateValue(params.GetGateStatesValue()));

        device_time_list.append(device_time_slot);
      }
      device_port_entry.SetGateControlList(device_time_list);

      device_port_list.append(device_port_entry);
    }
    device_setting.SetPortList(device_port_list);

    setting_list_.append(device_setting);
  }
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateDeviceTimeSlotSettings(qint64 &project_id, ActDeviceTimeSlotSettingList &setting_list,
                                                 bool is_operation) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActDeviceConfig &device_config = project.GetDeviceConfig();

  QMap<qint64, ActGclTable> &setting_tables = device_config.GetGCLTables();

  // Check device GCL setting list is empty
  auto setting_list_ = setting_list.GetDeviceTimeSlotSettingList();
  if (setting_list_.isEmpty()) {
    QString error_msg = QString("Device GCL setting list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // one device setting in setting list
  for (ActDeviceTimeSlotSetting setting : setting_list_) {
    // Get device id & device
    qint64 device_id = setting.GetDeviceId();
    ActDevice device;
    act_status = project.GetDeviceById(device, device_id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Get device failed with device id:" << device_id;
      return act_status;
    }

    QList<ActDeviceTimeSlotPortEntry> port_list = setting.GetPortList();
    QList<ActInterfaceGateParameters> interfaces_gate_parameters;
    for (ActDeviceTimeSlotPortEntry port_entry : port_list) {
      qint64 port_id = port_entry.GetPortId();
      qreal cycle_time = port_entry.GetCycleTime();

      // cycle time & interval has same restriction
      qreal MAX_CYCLE_TIME = 999999.999;

      // input check: cycle time: 0.001~ 999999.999
      if (cycle_time > MAX_CYCLE_TIME) {
        QString error_msg = QString("cycle time of device id %1 out of range: %2").arg(device_id).arg(cycle_time);
        return std::make_shared<ActBadRequest>(error_msg);
      }

      ActGateParameters gate_params;
      quint32 cycle_time_int = cycle_time * 1000;
      ActAdminCycleTime admin_cycle_time(static_cast<quint32>(cycle_time_int), static_cast<quint32>(1000000000));
      gate_params.SetAdminCycleTime(admin_cycle_time);
      gate_params.SetAdminControlListLength(port_entry.GetGateControlList().size());

      gate_params.SetGateEnabled(port_entry.GetActive());

      QList<ActDeviceTimeSlot> gate_control_list = port_entry.GetGateControlList();
      QList<ActAdminControl> admin_control_list;
      for (ActDeviceTimeSlot time_slot : gate_control_list) {
        // input check: interval: 0.001~ 999999.999
        if (time_slot.GetInterval() > MAX_CYCLE_TIME) {
          QString error_msg =
              QString("interval of device id %1 out of range: %2").arg(device_id).arg(time_slot.GetInterval());
          return std::make_shared<ActBadRequest>(error_msg);
        }

        ActAdminControl admin_control;
        admin_control.SetIndex(time_slot.GetSlotId());
        admin_control.SetOperationName("set-gate-states");

        ActSGSParams sgs_params;
        sgs_params.SetGateStatesValue(sgs_params.GetGateStateValueByQueueSet(time_slot.GetQueueSet()));
        sgs_params.SetTimeIntervalValue(time_slot.GetInterval() * 1'000);

        admin_control.SetSgsParams(sgs_params);

        admin_control_list.append(admin_control);
      }

      gate_params.SetAdminControlList(admin_control_list);

      ActInterfaceGateParameters params(port_id, gate_params);
      interfaces_gate_parameters.append(params);
    }
    // fill into device config
    if (!setting_tables.contains(device_id)) {
      setting_tables[device_id] = ActGclTable(device_id);
    }
    ActGclTable &setting_table = setting_tables[device_id];

    QSet<ActInterfaceGateParameters> gate_param_set(interfaces_gate_parameters.begin(),
                                                    interfaces_gate_parameters.end());
    setting_table.SetInterfacesGateParameters(gate_param_set);
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}
}  // namespace core
}  // namespace act