#include <QJsonObject>

#include "act_offline_config.hpp"

ACT_STATUS act::offline_config::GenL2NetworkSetting(const ActDeviceConfig &device_config, const qint64 &device_id,
                                                    QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto network_setting_tables_ = device_config.GetNetworkSettingTables();
  // no network settings -> skip
  if (network_setting_tables_.isEmpty()) {
    return act_status;
  }

  ActNetworkSettingTable network_setting_;
  QMap<qint64, ActNetworkSettingTable>::iterator it = network_setting_tables_.find(device_id);
  if (it == network_setting_tables_.end()) {
    // no specified device network setting -> skip
    return act_status;
  } else {
    network_setting_ = it.value();
  }

  QList<QJsonObject> elements;

  if (network_setting_.GetNetworkSettingMode() == ActNetworkSettingModeEnum::kStatic) {
    if (network_setting_.GetIpAddress() != "") {
      AddSingleValueElementToList(elements, "IP Address", network_setting_.GetIpAddress());
      AddSingleValueElementToList(elements, "Subnet Mask", network_setting_.GetSubnetMask());
      AddSingleValueElementToList(elements, "Gateway", network_setting_.GetGateway());
    }

    if (network_setting_.GetDNS1() != "") {
      AddSingleValueElementToList(elements, "DNS1", network_setting_.GetDNS1());
    }

    if (network_setting_.GetDNS2() != "") {
      AddSingleValueElementToList(elements, "DNS2", network_setting_.GetDNS2());
    }
  } else if (network_setting_.GetNetworkSettingMode() == ActNetworkSettingModeEnum::kDHCP) {
    AddSingleValueElementToList(elements, "Auto IP Configuration", QString("DHCP"));
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("IP Configuration", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenLoginPolicy(const ActDeviceConfig &device_config, const qint64 &device_id,
                                               QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto login_policy_tables_ = device_config.GetLoginPolicyTables();
  // no login policy settings -> skip
  if (login_policy_tables_.isEmpty()) {
    return act_status;
  }

  ActLoginPolicyTable login_policy_;
  QMap<qint64, ActLoginPolicyTable>::iterator it = login_policy_tables_.find(device_id);
  if (it == login_policy_tables_.end()) {
    // no specified device login policy setting -> skip
    return act_status;
  } else {
    login_policy_ = it.value();
  }

  QList<QJsonObject> elements;

  if (login_policy_.GetLoginMessage() != "") {
    AddSingleValueElementToList(elements, "Login Message", login_policy_.GetLoginMessage());
  }

  if (login_policy_.GetLoginAuthenticationFailureMessage() != "") {
    AddSingleValueElementToList(elements, "Login Authentication Failure Message",
                                login_policy_.GetLoginAuthenticationFailureMessage());
  }

  if (login_policy_.GetLoginFailureLockout() != false) {
    AddSingleValueElementToList(elements, "Account Login Failure Lockout", login_policy_.GetLoginFailureLockout());
  }

  if (login_policy_.GetRetryFailureThreshold() != 5) {
    AddSingleValueElementToList(elements, "Retry Failure Threshold", login_policy_.GetRetryFailureThreshold());
  }

  if (login_policy_.GetLockoutDuration() != 5) {
    AddSingleValueElementToList(elements, "Lockout Duration", login_policy_.GetLockoutDuration());
  }

  if (login_policy_.GetAutoLogoutAfter() != 5) {
    AddSingleValueElementToList(elements, "Auto Logout After", login_policy_.GetAutoLogoutAfter());
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Login Policy", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenLoopProtection(const ActDeviceConfig &device_config, const qint64 &device_id,
                                                  QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto loop_protection_tables_ = device_config.GetLoopProtectionTables();
  // no loop protection settings -> skip
  if (loop_protection_tables_.isEmpty()) {
    return act_status;
  }

  ActLoopProtectionTable loop_protection_setting_;
  QMap<qint64, ActLoopProtectionTable>::iterator it = loop_protection_tables_.find(device_id);
  if (it == loop_protection_tables_.end()) {
    // no specified device loop protection setting -> skip
    return act_status;
  } else {
    loop_protection_setting_ = it.value();
  }

  QList<QJsonObject> elements;

  if (loop_protection_setting_.GetNetworkLoopProtection() != false) {
    AddSingleValueElementToList(elements, "Active Loop Protection",
                                loop_protection_setting_.GetNetworkLoopProtection());
  }

  if (loop_protection_setting_.GetDetectInterval() != 10) {
    AddSingleValueElementToList(elements, "Detect Interval", loop_protection_setting_.GetDetectInterval());
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Loop Protection", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenSNMPTrapSetting(const ActDeviceConfig &device_config, const qint64 &device_id,
                                                   QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto snmp_trap_setting_tables_ = device_config.GetSnmpTrapSettingTables();
  // no SNMP trap settings -> skip
  if (snmp_trap_setting_tables_.isEmpty()) {
    return act_status;
  }

  ActSnmpTrapSettingTable snmp_trap_setting_;
  QMap<qint64, ActSnmpTrapSettingTable>::iterator it = snmp_trap_setting_tables_.find(device_id);
  if (it == snmp_trap_setting_tables_.end()) {
    // no specified device SNMP trap setting -> skip
    return act_status;
  } else {
    snmp_trap_setting_ = it.value();
  }

  QList<ActSnmpTrapHostEntry> hosts = snmp_trap_setting_.GetHostList();
  // no SNMP trap entry setting -> skip
  if (hosts.isEmpty()) {
    return act_status;
  }

  QJsonArray host_data_list;
  QJsonArray mode_data_list;
  QJsonArray community_data_list;

  for (auto host : hosts) {
    if (host.GetHostName() != "") {
      QJsonObject host_data;
      QJsonArray host_value = {host.GetHostName()};
      host_data["value"] = host_value;

      host_data_list.append(host_data);

      QJsonObject mode_data;
      QJsonArray mode_value = {kSnmpTrapModeConfigStringEnum.value(host.GetMode())};
      mode_data["value"] = mode_value;

      mode_data_list.append(mode_data);

      QJsonObject community_data;
      QJsonArray community_value = {host.GetTrapCommunity()};
      community_data["value"] = community_value;

      community_data_list.append(community_data);
    }
  }

  QList<QJsonObject> elements;
  if (!host_data_list.isEmpty()) {
    AddElementToList(elements, "Receipt IP or Name", host_data_list);
    AddElementToList(elements, "Mode", mode_data_list);
    AddElementToList(elements, "Community", community_data_list);
  }
  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("SNMP Trap", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenSyslogSetting(const ActDeviceConfig &device_config, const qint64 &device_id,
                                                 QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto syslog_setting_tables_ = device_config.GetSyslogSettingTables();
  // no syslog settings -> skip
  if (syslog_setting_tables_.isEmpty()) {
    return act_status;
  }

  ActSyslogSettingTable syslog_setting_;
  QMap<qint64, ActSyslogSettingTable>::iterator it = syslog_setting_tables_.find(device_id);
  if (it == syslog_setting_tables_.end()) {
    // no specified syslog setting -> skip
    return act_status;
  } else {
    syslog_setting_ = it.value();
  }

  QList<QJsonObject> elements;

  if (syslog_setting_.GetEnabled() != false) {
    AddSingleValueElementToList(elements, "Active Syslog", syslog_setting_.GetEnabled());
  }

  // if syslog server 1/2/3 is disable still can set syslog configuration to device
  if (syslog_setting_.GetSyslogServer1() == false) {
    AddSingleValueElementToList(elements, "Disable Syslog Server 1", QString(""));
  }
  if (syslog_setting_.GetAddress1() != "") {
    QString ip = QString("%1 %2").arg("ipv4").arg(syslog_setting_.GetAddress1());
    AddSingleValueElementToList(elements, "IP/Domain Name of Server 1", ip);
    AddSingleValueElementToList(elements, "Port of Server 1", syslog_setting_.GetPort1());
  }

  if (syslog_setting_.GetSyslogServer2() == false) {
    AddSingleValueElementToList(elements, "Disable Syslog Server 2", QString(""));
  }
  if (syslog_setting_.GetAddress2() != "") {
    QString ip = QString("%1 %2").arg("ipv4").arg(syslog_setting_.GetAddress2());
    AddSingleValueElementToList(elements, "IP/Domain Name of Server 2", ip);
    AddSingleValueElementToList(elements, "Port of Server 2", syslog_setting_.GetPort2());
  }

  if (syslog_setting_.GetSyslogServer3() == false) {
    AddSingleValueElementToList(elements, "Disable Syslog Server 3", QString(""));
  }
  if (syslog_setting_.GetAddress3() != "") {
    QString ip = QString("%1 %2").arg("ipv4").arg(syslog_setting_.GetAddress3());
    AddSingleValueElementToList(elements, "IP/Domain Name of Server 3", ip);
    AddSingleValueElementToList(elements, "Port of Server 3", syslog_setting_.GetPort3());
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Syslog", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenNosTimeSetting(const ActDeviceConfig &device_config, const qint64 &device_id,
                                                  QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto time_setting_tables_ = device_config.GetTimeSettingTables();
  // no time settings -> skip
  if (time_setting_tables_.isEmpty()) {
    return act_status;
  }

  ActTimeSettingTable time_setting_;
  QMap<qint64, ActTimeSettingTable>::iterator it = time_setting_tables_.find(device_id);
  if (it == time_setting_tables_.end()) {
    // no specified device time setting -> skip
    return act_status;
  } else {
    time_setting_ = it.value();
  }

  QList<QJsonObject> elements;

  const QString clock_source = kClockSourceConfigStringEnum.value(time_setting_.GetClockSource());
  AddSingleValueElementToList(elements, "Time Source", clock_source);

  if (time_setting_.GetNTPTimeServer1() != "time.nist.gov") {
    AddSingleValueElementToList(elements, "Remote NTP Server 1", time_setting_.GetNTPTimeServer1());
  }

  if (time_setting_.GetNTPTimeServer2() != "") {
    AddSingleValueElementToList(elements, "Remote NTP Server 2", time_setting_.GetNTPTimeServer2());
  }

  if (time_setting_.GetSNTPTimeServer1() != "time.nist.gov") {
    AddSingleValueElementToList(elements, "Remote SNTP Server 1", time_setting_.GetSNTPTimeServer1());
  }

  if (time_setting_.GetSNTPTimeServer2() != "") {
    AddSingleValueElementToList(elements, "Remote SNTP Server 2", time_setting_.GetSNTPTimeServer2());
  }

  const QString time_zone = kTimeZoneConfigStringEnum.value(time_setting_.GetTimeZone());
  AddSingleValueElementToList(elements, "Time Zone", time_zone);

  if (time_setting_.GetDaylightSavingTime() != false) {
    AddSingleValueElementToList(elements, "Active Daylight Saving", time_setting_.GetDaylightSavingTime());

    auto start_date_time = time_setting_.GetStart();
    QString start_month = ToQString(start_date_time.GetMonth());
    QString start_week = kTimeWeekConfigStringEnum.value(start_date_time.GetWeek());
    QString start_day = ToQString(start_date_time.GetDay());
    QString start_time = QString("%1:%2").arg(start_date_time.GetHour()).arg(start_date_time.GetMinute());

    auto end_date_time = time_setting_.GetEnd();
    QString end_month = ToQString(end_date_time.GetMonth());
    QString end_week = kTimeWeekConfigStringEnum.value(end_date_time.GetWeek());
    QString end_day = ToQString(end_date_time.GetDay());
    QString end_time = QString("%1:%2").arg(end_date_time.GetHour()).arg(end_date_time.GetMinute());

    // check input
    if (time_setting_.GetOffset() == "00:00") {
      QString error_msg = QString("daylight saving offset is out of range: %1 (valid range: %2-%3)")
                              .arg(time_setting_.GetOffset())
                              .arg("00:30")
                              .arg("23:00");
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActStatusInternalError>(error_msg);
    }

    QString offset = time_setting_.GetOffset();

    // format: start_month week day hh:mm end_month week day hh:mm offset
    QString time_range = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9")
                             .arg(start_month)
                             .arg(start_week)
                             .arg(start_day)
                             .arg(start_time)
                             .arg(end_month)
                             .arg(end_week)
                             .arg(end_day)
                             .arg(end_time)
                             .arg(offset);
    AddSingleValueElementToList(elements, "Daylight Saving Time Range", time_range);
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Device Time Setting", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenTsnTimeSetting(const ActDeviceConfig &device_config, const qint64 &device_id,
                                                  QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto time_setting_tables_ = device_config.GetTimeSettingTables();
  // no TSN time settings -> skip
  if (time_setting_tables_.isEmpty()) {
    return act_status;
  }

  ActTimeSettingTable time_setting_;
  QMap<qint64, ActTimeSettingTable>::iterator it = time_setting_tables_.find(device_id);
  if (it == time_setting_tables_.end()) {
    // no specified device TSN time setting -> skip
    return act_status;
  } else {
    time_setting_ = it.value();
  }

  QList<QJsonObject> elements;

  const QString clock_source = kClockSourceConfigStringEnum.value(time_setting_.GetClockSource());
  AddSingleValueElementToList(elements, "Time Source", clock_source);

  if (time_setting_.GetNTPTimeServer1() != "time.nist.gov") {
    AddSingleValueElementToList(elements, "Remote NTP Server 1", time_setting_.GetNTPTimeServer1());
  }

  if (time_setting_.GetNTPTimeServer2() != "") {
    AddSingleValueElementToList(elements, "Remote NTP Server 2", time_setting_.GetNTPTimeServer2());
  }

  if (time_setting_.GetSNTPTimeServer1() != "time.nist.gov") {
    AddSingleValueElementToList(elements, "Remote SNTP Server 1", time_setting_.GetSNTPTimeServer1());
  }

  if (time_setting_.GetSNTPTimeServer2() != "") {
    AddSingleValueElementToList(elements, "Remote SNTP Server 2", time_setting_.GetSNTPTimeServer2());
  }

  const QString time_zone = kTimeZoneConfigStringEnum.value(time_setting_.GetTimeZone());
  AddSingleValueElementToList(elements, "Time Zone", time_zone);

  if (time_setting_.GetDaylightSavingTime() != false) {
    AddSingleValueElementToList(elements, "Active Daylight Saving", time_setting_.GetDaylightSavingTime());

    ActTimeDay start_date_time = time_setting_.GetStart();
    ActTimeDay end_date_time = time_setting_.GetEnd();
    ActClientTimeDate tsn_start_date_time;
    ActClientTimeDate tsn_end_date_time;
    // fake agent, just for calling the function (ConvertActIntervalDayToClientDurationDate())
    ActMoxaIEIClientAgent client_agent("0.0.0.0", ActRestfulProtocolEnum::kHTTPS, 80);
    client_agent.ConvertActIntervalDayToClientDurationDate(start_date_time, end_date_time, tsn_start_date_time,
                                                           tsn_end_date_time);

    QString start_month = ToQString(tsn_start_date_time.Getmonth());
    QString start_date = ToQString(tsn_start_date_time.Getdate());
    QString start_year = ToQString(tsn_start_date_time.Getyear());
    QString start_hour = ToQString(tsn_start_date_time.Gethour());
    QString start_minute = ToQString(tsn_start_date_time.Getminute());

    // format: month date year hour minute
    QString start_time =
        QString("%1 %2 %3 %4 %5").arg(start_month).arg(start_date).arg(start_year).arg(start_hour).arg(start_minute);
    AddSingleValueElementToList(elements, "Daylight Saving Start Time", start_time);

    QString end_month = ToQString(tsn_end_date_time.Getmonth());
    QString end_date = ToQString(tsn_end_date_time.Getdate());
    QString end_year = ToQString(tsn_end_date_time.Getyear());
    QString end_hour = ToQString(tsn_end_date_time.Gethour());
    QString end_minute = ToQString(tsn_end_date_time.Getminute());

    // format: month date year hour minute
    QString end_time =
        QString("%1 %2 %3 %4 %5").arg(end_month).arg(end_date).arg(end_year).arg(end_hour).arg(end_minute);
    AddSingleValueElementToList(elements, "Daylight Saving End Time", end_time);

    // check input
    if (time_setting_.GetOffset() == "00:00") {
      QString error_msg = QString("daylight saving offset is out of range: %1 (valid range: %2-%3)")
                              .arg(time_setting_.GetOffset())
                              .arg("00:30")
                              .arg("23:00");
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActStatusInternalError>(error_msg);
    }

    QString offset = time_setting_.GetOffset().replace(":", " ");
    AddSingleValueElementToList(elements, "Daylight Saving Offset", offset);
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Device Time Setting", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenInformationSetting(const ActDeviceConfig &device_config, const qint64 &device_id,
                                                      QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto info_setting_tables_ = device_config.GetInformationSettingTables();
  // no information settings -> skip
  if (info_setting_tables_.isEmpty()) {
    return act_status;
  }

  ActInformationSettingTable info_setting_;
  QMap<qint64, ActInformationSettingTable>::iterator it = info_setting_tables_.find(device_id);
  if (it == info_setting_tables_.end()) {
    // no specified device information setting -> skip
    return act_status;
  } else {
    info_setting_ = it.value();
  }

  QList<QJsonObject> elements;

  if (info_setting_.GetDeviceName() != "") {
    AddSingleValueElementToList(elements, "Device Name", info_setting_.GetDeviceName());
  }

  if (info_setting_.GetLocation() != "") {
    AddSingleValueElementToList(elements, "Location", info_setting_.GetLocation());
  }

  if (info_setting_.GetDescription() != "") {
    AddSingleValueElementToList(elements, "Description", info_setting_.GetDescription());
  }

  if (info_setting_.GetContactInformation() != "") {
    AddSingleValueElementToList(elements, "Contact Information", info_setting_.GetContactInformation());
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Information Settings", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenPortSetting(const ActDeviceConfig &device_config, const qint64 &device_id,
                                               const QMap<qint64, QString> &interface_names, QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto port_setting_tables_ = device_config.GetPortSettingTables();
  // no port settings -> skip
  if (port_setting_tables_.isEmpty()) {
    return act_status;
  }

  ActPortSettingTable port_setting_;
  QMap<qint64, ActPortSettingTable>::iterator it = port_setting_tables_.find(device_id);
  if (it == port_setting_tables_.end()) {
    // no specified device port setting -> skip
    return act_status;
  } else {
    port_setting_ = it.value();
  }

  auto entries = port_setting_.GetPortSettingEntries();
  // no port entry setting -> skip
  if (entries.isEmpty()) {
    return act_status;
  }

  QJsonArray element_data_list;

  for (auto entry : entries) {
    // only disable the port need to set config
    if (entry.GetAdminStatus() != true) {
      QJsonObject element_data;
      QJsonArray value = {"disable"};
      element_data["value"] = value;

      QString port_id = interface_names[entry.GetPortId()];
      element_data["portID"] = port_id;

      element_data_list.append(element_data);
    }
  }

  QList<QJsonObject> elements;
  if (!element_data_list.isEmpty()) {
    AddElementToList(elements, "Active Port", element_data_list);
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Port Setting", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenNosVlanSetting(const ActDeviceConfig &device_config, const qint64 &device_id,
                                                  const QMap<qint64, QString> &interface_names,
                                                  QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto vlan_setting_tables = device_config.GetVlanTables();
  // no VLAN settings -> skip
  if (vlan_setting_tables.isEmpty()) {
    return act_status;
  }

  ActVlanTable vlan_setting_table;
  QMap<qint64, ActVlanTable>::iterator it = vlan_setting_tables.find(device_id);
  if (it == vlan_setting_tables.end()) {
    // no specified device VLAN setting -> skip
    return act_status;
  } else {
    vlan_setting_table = it.value();
  }

  QList<QJsonObject> elements;

  AddSingleValueElementToList(elements, "Management VLAN ID", vlan_setting_table.GetManagementVlan());

  // static entry
  QSet<ActVlanStaticEntry> vlan_static_entries = vlan_setting_table.GetVlanStaticEntries();
  // vlan static entry has value
  if (!vlan_static_entries.isEmpty()) {
    QJsonArray active_vlan_element_data_list;
    QJsonArray vlan_name_element_data_list;
    QJsonArray no_untagged_port_element_data_list;
    QJsonArray untagged_port_element_data_list;
    QJsonArray egress_port_element_data_list;
    QJsonArray temstid_element_data_list;

    // sort vlan static entry by vlan id
    QList<ActVlanStaticEntry> vlan_static_entries_list = vlan_static_entries.values();
    std::sort(vlan_static_entries_list.begin(), vlan_static_entries_list.end(),
              [](const ActVlanStaticEntry &a, const ActVlanStaticEntry &b) { return a.GetVlanId() < b.GetVlanId(); });

    for (auto vlan_static_entry : vlan_static_entries_list) {
      QString vlan_id = ToQString(vlan_static_entry.GetVlanId());

      // vlan id
      QJsonObject active_vlan_element_data;
      QJsonArray active_vlan_values = {"enable"};

      active_vlan_element_data["value"] = active_vlan_values;
      active_vlan_element_data["vlanID"] = vlan_id;

      active_vlan_element_data_list.append(active_vlan_element_data);

      // vlan name
      if (vlan_static_entry.GetName() != "") {
        QJsonObject vlan_name_element_data;
        QJsonArray vlan_name_values = {vlan_static_entry.GetName()};

        vlan_name_element_data["value"] = vlan_name_values;
        vlan_name_element_data["vlanID"] = vlan_id;

        vlan_name_element_data_list.append(vlan_name_element_data);
      } else {
        QJsonObject vlan_name_element_data;
        QJsonArray vlan_name_values = {"\"\""};

        vlan_name_element_data["value"] = vlan_name_values;
        vlan_name_element_data["vlanID"] = vlan_id;

        vlan_name_element_data_list.append(vlan_name_element_data);
      }

      QSet<qint64> egress_ports_set = vlan_static_entry.GetEgressPorts();
      QSet<qint64> untagged_ports_set = vlan_static_entry.GetUntaggedPorts();
      // no untagged ports = tagged ports
      QSet<qint64> tagged_ports_set = egress_ports_set - untagged_ports_set;

      // tagged ports
      if (!tagged_ports_set.isEmpty()) {
        // sort untagged_ports_set
        QList<qint64> tagged_ports_list = tagged_ports_set.values();
        std::sort(tagged_ports_list.begin(), tagged_ports_list.end());

        QString port_list_str;
        for (auto port : tagged_ports_list) {
          QString port_id = interface_names[port];
          port_list_str = port_list_str + "," + port_id;
        }
        // remove first ","
        port_list_str.remove(0, 1);

        QJsonArray tagged_port_values = QJsonArray() << QJsonValue(port_list_str);

        QJsonObject no_untagged_port_element_data;
        no_untagged_port_element_data["value"] = tagged_port_values;
        no_untagged_port_element_data["vlanID"] = vlan_id;

        no_untagged_port_element_data_list.append(no_untagged_port_element_data);
      }

      // untagged ports
      if (!untagged_ports_set.isEmpty()) {
        // sort untagged_ports_set
        QList<qint64> untagged_ports_list = untagged_ports_set.values();
        std::sort(untagged_ports_list.begin(), untagged_ports_list.end());

        QString port_list_str;
        for (auto port : untagged_ports_list) {
          QString port_id = interface_names[port];
          port_list_str = port_list_str + "," + port_id;
        }
        // remove first ","
        port_list_str.remove(0, 1);

        QJsonArray untagged_port_values = QJsonArray() << QJsonValue(port_list_str);

        QJsonObject untagged_port_element_data;
        untagged_port_element_data["value"] = untagged_port_values;
        untagged_port_element_data["vlanID"] = vlan_id;

        untagged_port_element_data_list.append(untagged_port_element_data);
      }

      // egress ports/member ports
      if (!egress_ports_set.isEmpty()) {
        // sort egress_ports_set
        QList<qint64> egress_ports_list = egress_ports_set.values();
        std::sort(egress_ports_list.begin(), egress_ports_list.end());

        QString port_list_str;
        for (auto port : egress_ports_list) {
          QString port_id = interface_names[port];
          port_list_str = port_list_str + "," + port_id;
        }
        // remove first ","
        port_list_str.remove(0, 1);

        QJsonArray egress_port_values = QJsonArray() << QJsonValue(port_list_str);

        QJsonObject egress_port_element_data;
        egress_port_element_data["value"] = egress_port_values;
        egress_port_element_data["vlanID"] = vlan_id;

        egress_port_element_data_list.append(egress_port_element_data);
      } else if ((egress_ports_set.isEmpty()) && (vlan_id == "1")) {
        // all interfaces need to add into no egress port values
        QString port_list_str;
        for (auto &key : interface_names.keys()) {
          auto port_id = interface_names.value(key);
          port_list_str = port_list_str + "," + port_id;
        }
        // remove first ","
        port_list_str.remove(0, 1);

        QJsonArray no_egress_port_values = QJsonArray() << QJsonValue(port_list_str);

        QJsonObject no_egress_port_element_data;
        no_egress_port_element_data["value"] = no_egress_port_values;
        no_egress_port_element_data["vlanID"] = vlan_id;

        QJsonArray no_egress_port_element_data_list = QJsonArray() << no_egress_port_element_data;

        AddElementToList(elements, "VLAN No Member Port", no_egress_port_element_data_list);
      }

      // te-mstid
      if (vlan_static_entry.GetTeMstid()) {
        QJsonObject temstid_element_data;
        QJsonArray temstid_values = {"enable"};

        temstid_element_data["value"] = temstid_values;
        temstid_element_data["vlanID"] = vlan_id;

        temstid_element_data_list.append(temstid_element_data);
      }
    }

    if (!active_vlan_element_data_list.isEmpty()) {
      AddElementToList(elements, "VLAN Active", active_vlan_element_data_list);
    }
    if (!vlan_name_element_data_list.isEmpty()) {
      AddElementToList(elements, "VLAN Name", vlan_name_element_data_list);
    }
    if (!no_untagged_port_element_data_list.isEmpty()) {
      AddElementToList(elements, "VLAN Port TAGGED", no_untagged_port_element_data_list);
    }
    if (!untagged_port_element_data_list.isEmpty()) {
      AddElementToList(elements, "VLAN Port UNTAGGED", untagged_port_element_data_list);
    }
    if (!egress_port_element_data_list.isEmpty()) {
      AddElementToList(elements, "VLAN Member Port", egress_port_element_data_list);
    }
    if (!temstid_element_data_list.isEmpty()) {
      AddElementToList(elements, "Active te-mstid", temstid_element_data_list);
    }
  }

  // port entry
  QSet<ActPortVlanEntry> vlan_port_entries = vlan_setting_table.GetPortVlanEntries();
  // vlan port entry has value
  if (!vlan_port_entries.isEmpty()) {
    // PVID
    QJsonArray pvid_element_data_list;
    for (auto vlan_port_entry : vlan_port_entries) {
      QJsonObject element_data;

      QJsonArray values = {ToQString(vlan_port_entry.GetPVID())};
      element_data["value"] = values;

      QString port_id = interface_names[vlan_port_entry.GetPortId()];
      element_data["portID"] = port_id;

      pvid_element_data_list.append(element_data);
    }

    AddElementToList(elements, "VLAN Port PVID", pvid_element_data_list);
  }

  // port type entry
  QSet<ActVlanPortTypeEntry> vlan_port_type_entries = vlan_setting_table.GetVlanPortTypeEntries();
  // vlan port type entry has value
  if (!vlan_port_type_entries.isEmpty()) {
    QJsonArray port_type_data_list;
    QJsonArray acc_frame_type_data_list;

    for (auto vlan_port_type_entry : vlan_port_type_entries) {
      // port type
      QJsonObject port_type_element_data;

      QString port_type_str = kVlanPortTypeEnum.value(vlan_port_type_entry.GetVlanPortType());
      QJsonArray values = {port_type_str};
      port_type_element_data["value"] = values;

      QString port_id = interface_names[vlan_port_type_entry.GetPortId()];
      port_type_element_data["portID"] = port_id;

      port_type_data_list.append(port_type_element_data);

      // acceptable-frame-type all, only hybrid & trunk need
      if ((port_type_str == "hybrid") || (port_type_str == "trunk")) {
        QJsonObject acc_frame_type_element_data;
        QJsonArray values = {"all"};

        acc_frame_type_element_data["value"] = values;
        acc_frame_type_element_data["portID"] = port_id;

        acc_frame_type_data_list.append(acc_frame_type_element_data);
      }
    }

    AddElementToList(elements, "VLAN Port Mode", port_type_data_list);
    if (!acc_frame_type_data_list.isEmpty()) {
      AddElementToList(elements, "VLAN Port Acceptable Frame Type", acc_frame_type_data_list);
    }
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("VLAN Settings", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenTsnVlanSetting(const bool supportHybrid, const ActDeviceConfig &device_config,
                                                  const qint64 &device_id, const QMap<qint64, QString> &interface_names,
                                                  QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto vlan_setting_tables = device_config.GetVlanTables();
  // no VLAN settings -> skip
  if (vlan_setting_tables.isEmpty()) {
    return act_status;
  }

  ActVlanTable vlan_setting_table;
  QMap<qint64, ActVlanTable>::iterator it = vlan_setting_tables.find(device_id);
  if (it == vlan_setting_tables.end()) {
    // no specified device VLAN setting -> skip
    return act_status;
  } else {
    vlan_setting_table = it.value();
  }

  QList<QJsonObject> elements;

  AddSingleValueElementToList(elements, "Management VLAN ID", vlan_setting_table.GetManagementVlan());

  // static entry
  QSet<ActVlanStaticEntry> vlan_static_entries = vlan_setting_table.GetVlanStaticEntries();
  // vlan static entry has value
  if (!vlan_static_entries.isEmpty()) {
    QJsonArray active_vlan_element_data_list;
    QJsonArray vlan_name_element_data_list;
    QJsonArray no_untagged_port_element_data_list;
    QJsonArray untagged_port_element_data_list;
    QJsonArray egress_port_element_data_list;
    QJsonArray temstid_element_data_list;

    // sort vlan static entry by vlan id
    QList<ActVlanStaticEntry> vlan_static_entries_list = vlan_static_entries.values();
    std::sort(vlan_static_entries_list.begin(), vlan_static_entries_list.end(),
              [](const ActVlanStaticEntry &a, const ActVlanStaticEntry &b) { return a.GetVlanId() < b.GetVlanId(); });

    for (auto vlan_static_entry : vlan_static_entries_list) {
      QString vlan_id = ToQString(vlan_static_entry.GetVlanId());

      // vlan id
      QJsonObject active_vlan_element_data;
      QJsonArray active_vlan_values = {"enable"};

      active_vlan_element_data["value"] = active_vlan_values;
      active_vlan_element_data["vlanID"] = vlan_id;

      active_vlan_element_data_list.append(active_vlan_element_data);

      // vlan name
      if (vlan_static_entry.GetName() != "") {
        QJsonObject vlan_name_element_data;
        QJsonArray vlan_name_values = {vlan_static_entry.GetName()};

        vlan_name_element_data["value"] = vlan_name_values;
        vlan_name_element_data["vlanID"] = vlan_id;

        vlan_name_element_data_list.append(vlan_name_element_data);
      } else {
        QJsonObject vlan_name_element_data;
        QJsonArray vlan_name_values = {"\"\""};

        vlan_name_element_data["value"] = vlan_name_values;
        vlan_name_element_data["vlanID"] = vlan_id;

        vlan_name_element_data_list.append(vlan_name_element_data);
      }

      QSet<qint64> egress_ports_set = vlan_static_entry.GetEgressPorts();
      QSet<qint64> untagged_ports_set = vlan_static_entry.GetUntaggedPorts();
      // no untagged ports = tagged ports
      QSet<qint64> tagged_ports_set = egress_ports_set - untagged_ports_set;

      // tagged ports
      if (!tagged_ports_set.isEmpty()) {
        // sort untagged_ports_set
        QList<qint64> tagged_ports_list = tagged_ports_set.values();
        std::sort(tagged_ports_list.begin(), tagged_ports_list.end());

        QString port_list_str;
        for (auto port : tagged_ports_list) {
          QString port_id = interface_names[port];
          port_list_str = port_list_str + "," + port_id;
        }
        // remove first ","
        port_list_str.remove(0, 1);

        QJsonArray tagged_port_values = QJsonArray() << QJsonValue(port_list_str);

        QJsonObject no_untagged_port_element_data;
        no_untagged_port_element_data["value"] = tagged_port_values;
        no_untagged_port_element_data["vlanID"] = vlan_id;

        no_untagged_port_element_data_list.append(no_untagged_port_element_data);
      }

      // untagged ports
      if (!untagged_ports_set.isEmpty()) {
        // sort untagged_ports_set
        QList<qint64> untagged_ports_list = untagged_ports_set.values();
        std::sort(untagged_ports_list.begin(), untagged_ports_list.end());

        QString port_list_str;
        for (auto port : untagged_ports_list) {
          QString port_id = interface_names[port];
          port_list_str = port_list_str + "," + port_id;
        }
        // remove first ","
        port_list_str.remove(0, 1);

        QJsonArray untagged_port_values = QJsonArray() << QJsonValue(port_list_str);

        QJsonObject untagged_port_element_data;
        untagged_port_element_data["value"] = untagged_port_values;
        untagged_port_element_data["vlanID"] = vlan_id;

        untagged_port_element_data_list.append(untagged_port_element_data);
      }

      // egress ports/member ports
      if (!egress_ports_set.isEmpty()) {
        // sort egress_ports_set
        QList<qint64> egress_ports_list = egress_ports_set.values();
        std::sort(egress_ports_list.begin(), egress_ports_list.end());

        QString port_list_str;
        for (auto port : egress_ports_list) {
          QString port_id = interface_names[port];
          port_list_str = port_list_str + "," + port_id;
        }
        // remove first ","
        port_list_str.remove(0, 1);

        QJsonArray egress_port_values = QJsonArray() << QJsonValue(port_list_str);

        QJsonObject egress_port_element_data;
        egress_port_element_data["value"] = egress_port_values;
        egress_port_element_data["vlanID"] = vlan_id;

        egress_port_element_data_list.append(egress_port_element_data);
      } else if ((egress_ports_set.isEmpty()) && (vlan_id == "1")) {
        // all interfaces need to add into no egress port values
        QString port_list_str;
        for (auto &key : interface_names.keys()) {
          auto port_id = interface_names.value(key);
          port_list_str = port_list_str + "," + port_id;
        }
        // remove first ","
        port_list_str.remove(0, 1);

        QJsonArray no_egress_port_values = QJsonArray() << QJsonValue(port_list_str);

        QJsonObject no_egress_port_element_data;
        no_egress_port_element_data["value"] = no_egress_port_values;
        no_egress_port_element_data["vlanID"] = vlan_id;

        QJsonArray no_egress_port_element_data_list = QJsonArray() << no_egress_port_element_data;

        AddElementToList(elements, "VLAN No Member Port", no_egress_port_element_data_list);
      }

      // te-mstid
      if (vlan_static_entry.GetTeMstid()) {
        QJsonObject temstid_element_data;
        QJsonArray temstid_values = {"enable"};

        temstid_element_data["value"] = temstid_values;
        temstid_element_data["vlanID"] = vlan_id;

        temstid_element_data_list.append(temstid_element_data);
      }
    }

    if (!vlan_name_element_data_list.isEmpty()) {
      AddElementToList(elements, "VLAN Name", vlan_name_element_data_list);
    }
    if (!no_untagged_port_element_data_list.isEmpty()) {
      AddElementToList(elements, "VLAN Port TAGGED", no_untagged_port_element_data_list);
    }
    if (!untagged_port_element_data_list.isEmpty()) {
      AddElementToList(elements, "VLAN Port UNTAGGED", untagged_port_element_data_list);
    }
    if (!egress_port_element_data_list.isEmpty()) {
      AddElementToList(elements, "VLAN Member Port", egress_port_element_data_list);
    }
    if (!temstid_element_data_list.isEmpty()) {
      AddElementToList(elements, "Active te-mstid", temstid_element_data_list);
    }
  }

  // port entry
  QSet<ActPortVlanEntry> vlan_port_entries = vlan_setting_table.GetPortVlanEntries();
  // vlan port entry has value
  if (!vlan_port_entries.isEmpty()) {
    // PVID
    QJsonArray pvid_element_data_list;
    for (auto vlan_port_entry : vlan_port_entries) {
      QJsonObject element_data;

      QJsonArray values = {ToQString(vlan_port_entry.GetPVID())};
      element_data["value"] = values;

      QString port_id = interface_names[vlan_port_entry.GetPortId()];
      element_data["portID"] = port_id;

      pvid_element_data_list.append(element_data);
    }

    AddElementToList(elements, "VLAN Port PVID", pvid_element_data_list);
  }

  // port type entry
  QSet<ActVlanPortTypeEntry> vlan_port_type_entries = vlan_setting_table.GetVlanPortTypeEntries();
  // vlan port type entry has value
  if (!vlan_port_type_entries.isEmpty()) {
    // port type
    QJsonArray port_type_data_list;
    QJsonArray acc_frame_type_data_list;

    for (auto vlan_port_type_entry : vlan_port_type_entries) {
      // port type
      QJsonObject port_type_element_data;

      QString port_type_str = kVlanPortTypeEnum.value(vlan_port_type_entry.GetVlanPortType());
      QJsonArray values = {port_type_str};
      port_type_element_data["value"] = values;

      QString port_id = interface_names[vlan_port_type_entry.GetPortId()];
      port_type_element_data["portID"] = port_id;

      port_type_data_list.append(port_type_element_data);

      if (supportHybrid) {
        if (port_type_str == "hybrid" || (port_type_str == "trunk")) {
          QJsonObject acc_frame_type_element_data;
          QJsonArray values = {QString("all")};

          acc_frame_type_element_data["value"] = values;
          acc_frame_type_element_data["portID"] = port_id;

          acc_frame_type_data_list.append(acc_frame_type_element_data);
        }
      }
    }

    AddElementToList(elements, "VLAN Port Mode", port_type_data_list);
    if (!acc_frame_type_data_list.isEmpty()) {
      AddElementToList(elements, "VLAN Port Acceptable Frame Type", acc_frame_type_data_list);
    }
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("VLAN Settings", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenStaticForwardSetting(const ActDeviceConfig &device_config, const qint64 &device_id,
                                                        const QMap<qint64, QString> &interface_names,
                                                        QJsonArray *root_array) {
  ACT_STATUS_INIT();

  // unicast & multicast static forward table
  auto uni_forwarding_tables_ = device_config.GetUnicastStaticForwardTables();
  auto multi_forwarding_tables_ = device_config.GetMulticastStaticForwardTables();

  QSet<ActStaticForwardEntry> uni_entries;
  QSet<ActStaticForwardEntry> multi_entries;

  ActStaticForwardTable uni_forwarding_setting;
  ActStaticForwardTable multi_forwarding_setting;

  if (uni_forwarding_tables_.isEmpty() && multi_forwarding_tables_.isEmpty()) {
    // no static forwarding settings -> skip
    return act_status;
  }
  if (!uni_forwarding_tables_.isEmpty()) {
    // get the unicast forwarding table
    QMap<qint64, ActStaticForwardTable>::iterator uni_it = uni_forwarding_tables_.find(device_id);
    // found device unicast forwarding table
    if (uni_it != uni_forwarding_tables_.end()) {
      uni_forwarding_setting = uni_it.value();
      uni_entries = uni_forwarding_setting.GetStaticForwardEntries();
    }
  }
  if (!multi_forwarding_tables_.isEmpty()) {
    // get the multicast forwarding table
    QMap<qint64, ActStaticForwardTable>::iterator multi_it = multi_forwarding_tables_.find(device_id);
    if (multi_it != multi_forwarding_tables_.end()) {
      // found device multicast forwarding table
      multi_forwarding_setting = multi_it.value();
      multi_entries = multi_forwarding_setting.GetStaticForwardEntries();
    }
  }

  QList<QJsonObject> elements;
  QJsonArray uni_element_data_list;
  QJsonArray multi_element_data_list;

  // unicast
  if (!uni_entries.isEmpty()) {
    for (auto entry : uni_entries) {
      QSet<qint64> egress_port_set = entry.GetEgressPorts();

      // sort egress port
      QList<qint64> egress_port_list = egress_port_set.values();
      std::sort(egress_port_list.begin(), egress_port_list.end());

      QString port_id_list;
      for (auto egress_port : egress_port_list) {
        port_id_list = port_id_list + "," + interface_names[egress_port];
      }
      // remove first ","
      port_id_list.remove(0, 1);

      QString unicast_mac = entry.GetMAC().replace("-", ":");

      QJsonArray values = ToQJsonArray(unicast_mac, entry.GetVlanId(), port_id_list);

      QJsonObject element_data;
      element_data["value"] = values;

      uni_element_data_list.append(element_data);
    }
    AddElementToList(elements, "Static Unicast Forwarding", uni_element_data_list);
  }

  // multicast
  if (!multi_entries.isEmpty()) {
    for (auto entry : multi_entries) {
      QSet<qint64> egress_port_set = entry.GetEgressPorts();

      // sort egress port
      QList<qint64> egress_port_list = egress_port_set.values();
      std::sort(egress_port_list.begin(), egress_port_list.end());

      QString port_id_list;
      for (auto egress_port : egress_port_list) {
        port_id_list = port_id_list + "," + interface_names[egress_port];
      }
      // remove first ","
      port_id_list.remove(0, 1);

      QString multicast_mac = entry.GetMAC().replace("-", ":");

      QJsonArray values = ToQJsonArray(multicast_mac, entry.GetVlanId(), port_id_list);

      QJsonObject element_data;
      element_data["value"] = values;

      multi_element_data_list.append(element_data);
    }
    AddElementToList(elements, "Static Multicast Forwarding", multi_element_data_list);
  }

  // unicast or multicast is not empty
  if ((!uni_element_data_list.isEmpty()) || (!multi_element_data_list.isEmpty())) {
    QJsonObject feature = CreateFeature("Static Forward", elements);
    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenPortDefaultPriority(const ActDeviceConfig &device_config, const qint64 &device_id,
                                                       const QMap<qint64, QString> &interface_names,
                                                       QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto default_priority_tables_ = device_config.GetPortDefaultPCPTables();
  // no default priority settings -> skip
  if (default_priority_tables_.isEmpty()) {
    return act_status;
  }

  ActDefaultPriorityTable default_priority_;
  QMap<qint64, ActDefaultPriorityTable>::iterator it = default_priority_tables_.find(device_id);
  if (it == default_priority_tables_.end()) {
    // no specified device default priority setting -> skip
    return act_status;
  } else {
    default_priority_ = it.value();
  }

  auto entries = default_priority_.GetDefaultPriorityEntries();
  // default priority entry is empty -> skip
  if (entries.isEmpty()) {
    return act_status;
  }

  QJsonArray element_data_list;

  for (auto entry : entries) {
    QJsonObject element_data;

    QJsonArray values = QJsonArray() << ToQString(entry.GetDefaultPCP());

    element_data["value"] = values;
    element_data["portID"] = interface_names[entry.GetPortId()];

    element_data_list.append(element_data);
  }

  QList<QJsonObject> elements;
  if (!element_data_list.isEmpty()) {
    AddElementToList(elements, "Port Default Priority", element_data_list);
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Default Priority", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenStreamAdapterV1(const ActDeviceConfig &device_config, const qint64 &device_id,
                                                   const QMap<qint64, QString> &interface_names,
                                                   QJsonArray *root_array) {
  ACT_STATUS_INIT();

  // ingress
  auto stream_priority_tables_ = device_config.GetStreamPriorityIngressTables();
  // egress
  auto egress_untag_tables_ = device_config.GetStreamPriorityEgressTables();

  QSet<ActInterfaceStadPortEntry> stream_priority_entries;
  QSet<ActStadConfigEntry> egress_untag_entries;

  if (stream_priority_tables_.isEmpty() && egress_untag_tables_.isEmpty()) {
    // no stream priority and egress untag settings -> skip
    return act_status;
  } else {
    if (!stream_priority_tables_.isEmpty()) {
      // get the stream priority table
      ActStadPortTable stream_priority_;
      QMap<qint64, ActStadPortTable>::iterator it = stream_priority_tables_.find(device_id);

      if (it != stream_priority_tables_.end()) {
        // get specified device stream priority table
        ActStadPortTable stream_priority_ = it.value();
        stream_priority_entries = stream_priority_.GetInterfaceStadPortEntries();
      }
    }
    if (!egress_untag_tables_.isEmpty()) {
      // get the egress untag table
      QMap<qint64, ActStadConfigTable>::iterator it = egress_untag_tables_.find(device_id);

      if (it != egress_untag_tables_.end()) {
        // get specified device egress untag table
        ActStadConfigTable egress_untag_ = it.value();
        egress_untag_entries = egress_untag_.GetStadConfigEntries();
      }
    }
  }

  QList<QJsonObject> elements;

  // handle stream priority
  if (!stream_priority_entries.isEmpty()) {
    QJsonArray sub_type_element_data_list;
    QJsonArray ethertype_only_element_data_list;
    QJsonArray disable_data_list;

    for (auto interface_entry : stream_priority_entries) {
      QSet<ActStadPortEntry> port_entries = interface_entry.GetStadPortEntries();

      QString port_id = interface_names[interface_entry.GetInterfaceId()];

      // clear all per stream priority entries at each ports
      // max is 10 entries for each port; index in CLI command is from 0 to 9
      for (int i = 0; i <= 9; i++) {
        QString index = QString::number(i);

        QJsonArray values = {index};
        QJsonObject element_data;

        element_data["value"] = values;
        element_data["portID"] = port_id;

        disable_data_list.append(element_data);
      }

      if (!port_entries.isEmpty()) {
        for (auto entry : port_entries) {
          auto type = *(entry.GetType().begin());
          QString port_id = interface_names[entry.GetPortId()];

          if (type == ActStreamPriorityTypeEnum::kEthertype) {
            // enable subtype
            if (entry.GetSubtypeEnable() == 1) {
              QJsonArray values = ToQJsonArray(entry.GetIngressIndex(), entry.GetEthertypeValue(),
                                               entry.GetSubtypeValue(), entry.GetVlanId(), entry.GetVlanPcp());
              QJsonObject element_data;
              element_data["value"] = values;
              element_data["portID"] = port_id;

              sub_type_element_data_list.append(element_data);
            } else {
              // ethertype only, no subtype
              QJsonArray values = ToQJsonArray(entry.GetIngressIndex(), entry.GetEthertypeValue(), entry.GetVlanId(),
                                               entry.GetVlanPcp());
              QJsonObject element_data;
              element_data["value"] = values;
              element_data["portID"] = port_id;

              ethertype_only_element_data_list.append(element_data);
            }
          }
        }
      }
    }

    if (!sub_type_element_data_list.isEmpty()) {
      AddElementToList(elements, "Per-stream Priority With Subtype", sub_type_element_data_list);
    }
    if (!ethertype_only_element_data_list.isEmpty()) {
      AddElementToList(elements, "Per-stream Priority", ethertype_only_element_data_list);
    }
    // no per-stream priority config
    if (!disable_data_list.isEmpty()) {
      AddElementToList(elements, "Inactive Per-stream Priority", disable_data_list);
    }
  }

  // handle egress untag
  if (!egress_untag_entries.isEmpty()) {
    QJsonArray element_data_list;

    for (auto port_entry : egress_untag_entries) {
      QJsonObject element_data;
      QJsonArray enable_values = {"enable"};
      QJsonArray disable_values = {"disable"};

      // enable egress untag
      if (port_entry.GetEgressUntag() == 1) {
        element_data["value"] = enable_values;
      } else {
        element_data["value"] = disable_values;
      }

      element_data["portID"] = interface_names[port_entry.GetPortId()];

      element_data_list.append(element_data);
    }

    if (!element_data_list.isEmpty()) {
      AddElementToList(elements, "Active Egress Untag", element_data_list);
    }
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Stream Adapter", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenStreamAdapterV2(const ActDeviceConfig &device_config, const qint64 &device_id,
                                                   const QMap<qint64, QString> &interface_names,
                                                   QJsonArray *root_array) {
  ACT_STATUS_INIT();

  // ingress
  auto stream_priority_tables_ = device_config.GetStreamPriorityIngressTables();
  // egress
  auto egress_untag_tables_ = device_config.GetStreamPriorityEgressTables();

  QSet<ActInterfaceStadPortEntry> stream_priority_entries;
  QSet<ActStadConfigEntry> egress_untag_entries;

  if (stream_priority_tables_.isEmpty() && egress_untag_tables_.isEmpty()) {
    // no stream priority and egress untag settings -> skip
    return act_status;
  } else {
    if (!stream_priority_tables_.isEmpty()) {
      // get the stream priority table
      ActStadPortTable stream_priority_;
      QMap<qint64, ActStadPortTable>::iterator it = stream_priority_tables_.find(device_id);

      if (it != stream_priority_tables_.end()) {
        // get specified device stream priority table
        ActStadPortTable stream_priority_ = it.value();
        stream_priority_entries = stream_priority_.GetInterfaceStadPortEntries();
      }
    }
    if (!egress_untag_tables_.isEmpty()) {
      // get the egress untag table
      QMap<qint64, ActStadConfigTable>::iterator it = egress_untag_tables_.find(device_id);

      if (it != egress_untag_tables_.end()) {
        // get specified device egress untag table
        ActStadConfigTable egress_untag_ = it.value();
        egress_untag_entries = egress_untag_.GetStadConfigEntries();
      }
    }
  }

  QList<QJsonObject> elements;

  // handle stream priority
  if (!stream_priority_entries.isEmpty()) {
    QJsonArray active_element_data_list;

    QJsonArray sub_type_element_data_list;
    QJsonArray ethertype_only_element_data_list;
    QJsonArray udp_type_element_data_list;
    QJsonArray tcp_type_element_data_list;
    QJsonArray disable_data_list;

    for (auto interface_entry : stream_priority_entries) {
      QSet<ActStadPortEntry> port_entries = interface_entry.GetStadPortEntries();

      QString port_id = interface_names[interface_entry.GetInterfaceId()];

      // clear all per stream priority entries at each ports
      // max is 10 entries for each port; index in CLI command is from 0 to 9
      for (int i = 0; i <= 9; i++) {
        QString index = QString::number(i);

        QJsonArray values = {index};
        QJsonObject element_data;

        element_data["value"] = values;
        element_data["portID"] = port_id;

        disable_data_list.append(element_data);
      }

      if (!port_entries.isEmpty()) {
        for (auto entry : port_entries) {
          QJsonObject active_element_data;
          QJsonArray active_values;
          QString index = QString::number(entry.GetIngressIndex());
          ActStreamPriorityTypeEnum type = *(entry.GetType().begin());

          QString type_str = kStreamPriorityTypeEnum[type];

          QString port_id = interface_names[entry.GetPortId()];

          active_values.append(QString("%1 %2").arg(index).arg(type_str));
          active_element_data["value"] = active_values;
          active_element_data["portID"] = port_id;

          active_element_data_list.append(active_element_data);

          if (type == ActStreamPriorityTypeEnum::kEthertype) {
            // enable subtype
            if (entry.GetSubtypeEnable() == 1) {
              QJsonArray values = ToQJsonArray(entry.GetIngressIndex(), entry.GetEthertypeValue(),
                                               entry.GetSubtypeValue(), entry.GetVlanId(), entry.GetVlanPcp());
              QJsonObject element_data;
              element_data["value"] = values;
              element_data["portID"] = port_id;

              sub_type_element_data_list.append(element_data);

            } else {
              // ethertype only, no subtype
              QJsonArray values = ToQJsonArray(entry.GetIngressIndex(), entry.GetEthertypeValue(), entry.GetVlanId(),
                                               entry.GetVlanPcp());
              QJsonObject element_data;
              element_data["value"] = values;
              element_data["portID"] = port_id;

              ethertype_only_element_data_list.append(element_data);
            }
          } else if (type == ActStreamPriorityTypeEnum::kUdp) {
            QJsonArray values =
                ToQJsonArray(entry.GetIngressIndex(), entry.GetUdpPort(), entry.GetVlanId(), entry.GetVlanPcp());
            QJsonObject element_data;
            element_data["value"] = values;
            element_data["portID"] = port_id;

            udp_type_element_data_list.append(element_data);
          } else if (type == ActStreamPriorityTypeEnum::kTcp) {
            QJsonArray values =
                ToQJsonArray(entry.GetIngressIndex(), entry.GetTcpPort(), entry.GetVlanId(), entry.GetVlanPcp());
            QJsonObject element_data;
            element_data["value"] = values;
            element_data["portID"] = port_id;

            tcp_type_element_data_list.append(element_data);
          }
        }
      }
    }

    if (!active_element_data_list.isEmpty()) {
      AddElementToList(elements, "Active Per-stream Priority", active_element_data_list);
    }
    // no per-stream priority config
    if (!disable_data_list.isEmpty()) {
      AddElementToList(elements, "Inactive Per-stream Priority", disable_data_list);
    }

    if (!sub_type_element_data_list.isEmpty()) {
      AddElementToList(elements, "Per-stream Priority With Subtype", sub_type_element_data_list);
    }
    if (!ethertype_only_element_data_list.isEmpty()) {
      AddElementToList(elements, "Per-stream Priority", ethertype_only_element_data_list);
    }
    if (!udp_type_element_data_list.isEmpty()) {
      AddElementToList(elements, "Per-stream Priority With UDP Port", udp_type_element_data_list);
    }
    if (!tcp_type_element_data_list.isEmpty()) {
      AddElementToList(elements, "Per-stream Priority With TCP Port", tcp_type_element_data_list);
    }
  }

  // handle egress untag
  if (!egress_untag_entries.isEmpty()) {
    QJsonArray element_data_list;

    for (auto port_entry : egress_untag_entries) {
      QJsonObject element_data;
      QJsonArray enable_values = {"enable"};
      QJsonArray disable_values = {"disable"};

      // enable egress untag
      if (port_entry.GetEgressUntag() == 1) {
        element_data["value"] = enable_values;
      } else {
        element_data["value"] = disable_values;
      }

      element_data["portID"] = interface_names[port_entry.GetPortId()];

      element_data_list.append(element_data);
    }

    if (!element_data_list.isEmpty()) {
      AddElementToList(elements, "Active Egress Untag", element_data_list);
    }
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Stream Adapter", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenGCL(const ActDeviceConfig &device_config, const qint64 &device_id,
                                       const QMap<qint64, QString> &interface_names, QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto GCL_tables_ = device_config.GetGCLTables();
  // no GCL settings -> skip
  if (GCL_tables_.isEmpty()) {
    return act_status;
  }

  QMap<qint64, ActGclTable>::iterator it = GCL_tables_.find(device_id);
  // no specified device GCL table -> skip
  if (it == GCL_tables_.end()) {
    return act_status;
  }

  ActGclTable GCL_table = it.value();
  QSet<ActInterfaceGateParameters> GCLs = GCL_table.GetInterfacesGateParameters();
  // gate control list is empty in GCL table -> skip
  if (GCLs.isEmpty()) {
    return act_status;
  }

  QJsonArray disable_element_data_list;
  QJsonArray gate_control_element_data_list;
  QJsonArray active_element_data_list;
  QJsonArray cycle_time_element_data_list;

  for (auto GCL : GCLs) {
    QString port_id = interface_names[GCL.GetInterfaceId()];

    // clear gate control list
    QJsonObject element_data;
    QJsonArray values = {"all"};

    element_data["value"] = values;
    element_data["portID"] = port_id;

    disable_element_data_list.append(element_data);

    ActGateParameters gate_params = GCL.GetGateParameters();
    QList<ActAdminControl> admin_controls = gate_params.GetAdminControlList();

    // enable qbv
    if (gate_params.GetGateEnabled()) {
      QJsonObject active_element_data;
      QJsonArray active_values = {"enable"};

      active_element_data["value"] = active_values;
      active_element_data["portID"] = port_id;

      active_element_data_list.append(active_element_data);
    }

    if (!admin_controls.isEmpty()) {
      // gate control list
      for (auto admin_control : admin_controls) {
        QString operation_name = ToQString(admin_control.GetOperationName());

        ActSGSParams sgs_params = admin_control.GetSgsParams();
        QString gate_state_value = ToQString(sgs_params.GetGateStatesValue());
        QString time_interval_value = ToQString(sgs_params.GetTimeIntervalValue());

        QJsonObject gate_control_element_data;
        QJsonArray gate_control_values;
        // Hard-coded because this is the only value we have right now.
        // gate_control_values.append(operation_name);
        gate_control_values.append(QString("sgs"));
        gate_control_values.append(gate_state_value);
        gate_control_values.append(time_interval_value);

        gate_control_element_data["value"] = gate_control_values;
        gate_control_element_data["portID"] = port_id;

        gate_control_element_data_list.append(gate_control_element_data);
      }
      // admin cycle time
      ActAdminCycleTime admin_cycle_time = gate_params.GetAdminCycleTime();
      QString numerator = ToQString(admin_cycle_time.GetNumerator());
      QString denominator = ToQString(admin_cycle_time.GetDenominator());

      QJsonObject cycle_time_element_data;
      QJsonArray cycle_time_values;
      cycle_time_values.append(numerator);
      cycle_time_values.append(denominator);

      cycle_time_element_data["value"] = cycle_time_values;
      cycle_time_element_data["portID"] = port_id;

      cycle_time_element_data_list.append(cycle_time_element_data);
    }
  }

  QList<QJsonObject> elements;

  if (!active_element_data_list.isEmpty()) {
    AddElementToList(elements, "Active Port Time Aware Shaper", active_element_data_list);
  }

  if (!disable_element_data_list.isEmpty()) {
    AddElementToList(elements, "Remove Port Gate Control Setting", disable_element_data_list);
  }
  if (!gate_control_element_data_list.isEmpty()) {
    AddElementToList(elements, "Port Gate Control Setting", gate_control_element_data_list);
  }

  if (!cycle_time_element_data_list.isEmpty()) {
    AddElementToList(elements, "Port Cycle Time", cycle_time_element_data_list);
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Time Aware Shaper", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenCB(const ActDeviceConfig &device_config, const qint64 &device_id,
                                      const QMap<QString, QString> &interface_names, QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto cb_tables = device_config.GetCbTables();
  // no CB settings -> skip
  if (cb_tables.isEmpty()) {
    return act_status;
  }

  ActCbTable cb_table;
  QMap<qint64, ActCbTable>::iterator it = cb_tables.find(device_id);
  if (it == cb_tables.end()) {
    // no specified device CB setting -> skip
    return act_status;
  } else {
    cb_table = it.value();
  }

  // stream identity
  QList<ActStreamIdentityEntry> stream_identity_list = cb_table.GetStreamIdentityList();

  for (auto stream_identity : stream_identity_list) {
    QList<QJsonObject> elements;
    AddSingleValueElementToList(elements, "Append Stream-id", QString("enable"));

    ActTsnStreamIdEntryGroup tsn_stream_id_entry_group = stream_identity.GetTsnStreamIdEntryGroup();

    // handle id
    AddSingleValueElementToList(elements, "Handle Number", tsn_stream_id_entry_group.GetHandle());

    ActOutFacing out_facing = tsn_stream_id_entry_group.GetOutFacing();

    // out facing input port list
    QSet<QString> input_port_list = out_facing.GetInputPortList();
    QString input_port_string_list;
    if (!input_port_list.isEmpty()) {
      for (QString input_port : input_port_list) {
        input_port_string_list = input_port_string_list + "," + interface_names[input_port];
      }
      // remove first ','
      input_port_string_list.remove(0, 1);

      AddSingleValueElementToList(elements, "Outfacing Input Port List", input_port_string_list);
    }

    // out facing output port list
    QSet<QString> output_port_list = out_facing.GetOutputPortList();
    QString output_port_string_list;
    if (!output_port_list.isEmpty()) {
      for (QString output_port : output_port_list) {
        output_port_string_list = output_port_string_list + "," + interface_names[output_port];
      }
      // remove first ','
      output_port_string_list.remove(0, 1);

      AddSingleValueElementToList(elements, "Outfacing Output Port List", output_port_string_list);
    }

    // identification type
    QString identification_type = kIdentificationTypeEnum.value(tsn_stream_id_entry_group.GetIdentificationType());
    AddSingleValueElementToList(elements, "Type", identification_type);

    // null group
    if (identification_type == "null") {
      ActNullStreamIdentificationGroup null_stream_identification =
          tsn_stream_id_entry_group.GetNullStreamIdentification();

      // format: 11:22:33:44:55:66
      QString destination_mac = null_stream_identification.GetDestinationMac().replace("-", ":");
      if (destination_mac != "") {
        AddSingleValueElementToList(elements, "Destination MAC Address", destination_mac);
      }
      QString vlan = ToQString(null_stream_identification.GetVlan());
      if (vlan != "0") {
        AddSingleValueElementToList(elements, "Vlan ID", vlan);
      }
    }

    // dmac vlan group
    if (identification_type == "dmac-vlan") {
      ActDmacVlanStreamIdentificationGroup dmac_vlan_stream_identification =
          tsn_stream_id_entry_group.GetDmacVlanStreamIdentification();

      // up
      ActUp up = dmac_vlan_stream_identification.GetUp();
      // down - overwrite
      ActDown down = dmac_vlan_stream_identification.GetDown();

      // up - destination MAC
      QString up_destination_mac = up.GetDestinationMac().replace("-", ":");
      if (up_destination_mac != "") {
        AddSingleValueElementToList(elements, "Destination MAC Address", up_destination_mac);
      }
      // up - VLAN
      QString up_vlan = ToQString(up.GetVlan());
      if (up_vlan != "0") {
        AddSingleValueElementToList(elements, "Vlan ID", up_vlan);
      }

      // down - mac
      QString overwrite_dst_mac = down.GetDestinationMac().replace("-", ":");
      if (overwrite_dst_mac != "") {
        AddSingleValueElementToList(elements, "Overwrite Destination MAC Address", overwrite_dst_mac);
      }
      // down - VLAN
      QString down_vlan = ToQString(down.GetVlan());
      if (down_vlan != "0") {
        AddSingleValueElementToList(elements, "Overwrite Vlan ID", down_vlan);
      }
    }

    if (!elements.isEmpty()) {
      AddSingleValueElementToList(elements, "Active Stream-id", QString("enable"));

      QJsonObject feature = CreateFeature("Stream Identity", elements);
      root_array->append(feature);
    }
  }

  // FRER
  ActFREREntry frer = cb_table.GetFRER();
  QList<QJsonObject> seq_id_elements;

  // sequence identification list
  QList<ActSequenceIdentificationList> sequence_identification_lists = frer.GetSequenceIdentificationLists();
  QJsonArray stream_list_element_data_list;
  QJsonArray active_element_data_list;
  QJsonArray encapsulation_element_data_list;

  for (ActSequenceIdentificationList list : sequence_identification_lists) {
    ActSequenceIdentificationEntry entry = list.GetSequenceIdentificationEntry();
    QString port_id = interface_names[entry.GetPort()];

    // stream list
    QSet<qint32> stream_id_set = entry.GetStreamList();
    QList<qint32> stream_id_list = stream_id_set.values();

    // sort stream id
    std::sort(stream_id_list.begin(), stream_id_list.end());

    QString stream_list_str;
    for (auto stream_id : stream_id_list) {
      QString stream_id_str = ToQString(stream_id);
      stream_list_str = stream_list_str + "," + stream_id_str;
    }
    // remove first ','
    stream_list_str.remove(0, 1);

    QJsonObject stream_list_element_data;
    QJsonArray stream_list_values = {stream_list_str};

    stream_list_element_data["value"] = stream_list_values;
    stream_list_element_data["portID"] = port_id;

    stream_list_element_data_list.append(stream_list_element_data);

    // active
    QString active = ToQString(entry.GetActive());

    QJsonObject active_element_data;
    QJsonArray active_values = {active};

    active_element_data["value"] = active_values;
    active_element_data["portID"] = port_id;

    active_element_data_list.append(active_element_data);

    // encapsulation
    QString encapsulation = kActSequenceEncodeDecodeTypesMap.key(entry.GetEncapsulation());

    QJsonObject encapsulation_element_data;
    QJsonArray encapsulation_values = {encapsulation};

    encapsulation_element_data["value"] = encapsulation_values;
    encapsulation_element_data["portID"] = port_id;

    encapsulation_element_data_list.append(encapsulation_element_data);
  }

  QList<QJsonObject> sequence_identification_elements;

  if (!stream_list_element_data_list.isEmpty()) {
    AddElementToList(seq_id_elements, "Sequence Identification - Stream Handle List", stream_list_element_data_list);
  }
  if (!active_element_data_list.isEmpty()) {
    AddElementToList(seq_id_elements, "Sequence Identification - Encode Active", active_element_data_list);
  }
  if (!encapsulation_element_data_list.isEmpty()) {
    AddElementToList(seq_id_elements, "Sequence Identification - Encapsulation", encapsulation_element_data_list);
  }

  if (!seq_id_elements.isEmpty()) {
    QJsonObject feature = CreateFeature("FRER", seq_id_elements);
    root_array->append(feature);
  }

  // sequence generation list
  QList<ActSequenceGenerationList> sequence_generation_lists = frer.GetSequenceGenerationLists();
  for (auto list : sequence_generation_lists) {
    QList<QJsonObject> seq_gen_elements;
    AddSingleValueElementToList(seq_gen_elements, "Sequence Generation - Append Sequence Generation",
                                QString("enable"));

    ActSequenceGenerationEntry entry = list.GetSequenceGenerationEntry();

    // stream list
    QSet<qint32> stream_id_set = entry.GetStreamList();
    QList<qint32> stream_id_list = stream_id_set.values();

    // sort stream id
    std::sort(stream_id_list.begin(), stream_id_list.end());

    QString stream_list_str;
    for (auto stream_id : stream_id_list) {
      QString stream_id_str = ToQString(stream_id);
      stream_list_str = stream_list_str + "," + stream_id_str;
    }
    // remove first ','
    stream_list_str.remove(0, 1);

    AddSingleValueElementToList(seq_gen_elements, "Sequence Generation - Stream Handle List", stream_list_str);

    if (!seq_gen_elements.isEmpty()) {
      QJsonObject feature = CreateFeature("FRER", seq_gen_elements);
      root_array->append(feature);
    }
  }

  // sequence recovery list
  QList<ActSequenceRecoveryList> sequence_recovery_lists = frer.GetSequenceRecoveryLists();
  for (auto list : sequence_recovery_lists) {
    QList<QJsonObject> seq_recv_elements;
    AddSingleValueElementToList(seq_recv_elements, "Sequence Recovery - Append Sequence Recovery", QString("enable"));

    ActSequenceRecoveryEntry entry = list.GetSequenceRecoveryEntry();

    // stream list
    QSet<qint32> stream_id_set = entry.GetStreamList();
    QList<qint32> stream_id_list = stream_id_set.values();

    // sort stream id
    std::sort(stream_id_list.begin(), stream_id_list.end());

    QString stream_list_str;
    for (auto stream_id : stream_id_list) {
      QString stream_id_str = ToQString(stream_id);
      stream_list_str = stream_list_str + "," + stream_id_str;
    }
    // remove first ','
    stream_list_str.remove(0, 1);

    AddSingleValueElementToList(seq_recv_elements, "Sequence Recovery - Stream Handle List", stream_list_str);

    // port list
    QSet<QString> port_list = entry.GetPortList();
    QString port_list_str;
    for (auto port_id : port_list) {
      QString port_id_str = interface_names[port_id];
      port_list_str = port_list_str + "," + port_id_str;
    }
    // remove first ','
    port_list_str.remove(0, 1);

    AddSingleValueElementToList(seq_recv_elements, "Sequence Recovery - Port List", port_list_str);

    // algorithm
    QString algorithm = kActSequenceRecoveryAlgorithmMap.key(entry.GetAlgorithm());
    AddSingleValueElementToList(seq_recv_elements, "Sequence Recovery - Algorithm", algorithm);

    // history length (algorithm=vector)
    if (algorithm == "vector") {
      AddSingleValueElementToList(seq_recv_elements, "Sequence Recovery - History Length", entry.GetHistoryLength());
    }

    // reset timeout
    AddSingleValueElementToList(seq_recv_elements, "Sequence Recovery - Reset Timeout", entry.GetResetTimeout());

    // take no sequence
    AddSingleValueElementToList(seq_recv_elements, "Sequence Recovery - Take No Sequence", entry.GetTakeNoSequence());

    // individual recovery type
    QString recv_type;
    if (entry.GetIndividualRecovery()) {
      recv_type = "individual";
    } else {
      recv_type = "sequence";
    }

    AddSingleValueElementToList(seq_recv_elements, "Sequence Recovery - Individual Recovery Type", recv_type);

    if (!seq_recv_elements.isEmpty()) {
      QJsonObject feature = CreateFeature("FRER", seq_recv_elements);
      root_array->append(feature);
    }
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenNosRstp(const ActDeviceConfig &device_config, const qint64 &device_id,
                                           const QMap<qint64, QString> &interface_names, QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto rstp_tables_ = device_config.GetRstpTables();
  // no RSTP settings -> skip
  if (rstp_tables_.isEmpty()) {
    return act_status;
  }

  ActRstpTable rstp_table_;
  QMap<qint64, ActRstpTable>::iterator it = rstp_tables_.find(device_id);
  if (it == rstp_tables_.end()) {
    // no specified device RSTP setting -> skip
    return act_status;
  } else {
    rstp_table_ = it.value();
  }

  QList<QJsonObject> elements;

  if (rstp_table_.GetActive()) {
    AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Active", QString("enable"));
    if ((rstp_table_.GetSpanningTreeVersion() == ActSpanningTreeVersionEnum::kSTP) ||
        (rstp_table_.GetSpanningTreeVersion() == ActSpanningTreeVersionEnum::kRSTP)) {
      AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Compatibility",
                                  kSpanningTreeVersionEnum.value(rstp_table_.GetSpanningTreeVersion()));
    }
    AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Bridge Priority", rstp_table_.GetPriority());
    AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Forward Delay Time", rstp_table_.GetForwardDelay());
    AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Hello Time", rstp_table_.GetHelloTime());
    AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Max Age", rstp_table_.GetMaxAge());
    AddSingleValueElementToList(elements, "Spanning Tree Error Recovery Time", rstp_table_.GetRstpErrorRecoveryTime());

  } else {
    AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Active", QString("disable"));
  }

  if (rstp_table_.GetRstpConfigSwift()) {
    AddSingleValueElementToList(elements, "Active Swift", rstp_table_.GetRstpConfigSwift());
  }

  if (rstp_table_.GetRstpConfigRevert()) {
    AddSingleValueElementToList(elements, "Active Swift Root Precedence", rstp_table_.GetRstpConfigRevert());
  }

  auto port_entries = rstp_table_.GetRstpPortEntries();
  if (port_entries.isEmpty()) {
    // add global rstp setting only
    QJsonObject feature = CreateFeature("Redundancy Protocol", elements);
    root_array->append(feature);
    return act_status;
  }

  QJsonArray port_active_element_data_list;
  QJsonArray port_edge_element_data_list;
  QJsonArray port_priority_element_data_list;
  QJsonArray port_path_cost_element_data_list;
  QJsonArray port_link_type_element_data_list;
  QJsonArray bpdu_guard_element_data_list;
  QJsonArray root_guard_element_data_list;
  QJsonArray loop_guard_element_data_list;
  QJsonArray bpdu_filter_element_data_list;

  for (ActRstpPortEntry port_entry : port_entries) {
    if (port_entry.GetRstpEnable()) {
      QString port_id = interface_names[port_entry.GetPortId()];

      // port active
      QJsonObject port_active_element_data;
      QJsonArray port_active_values = {"enable"};

      port_active_element_data["value"] = port_active_values;
      port_active_element_data["portID"] = port_id;

      port_active_element_data_list.append(port_active_element_data);

      // edge
      const QString edge = kActRstpEdgeEnumMap.key(port_entry.GetEdge());
      QJsonObject port_edge_element_data;
      QJsonArray port_edge_values = {edge};

      port_edge_element_data["value"] = port_edge_values;
      port_edge_element_data["portID"] = port_id;

      port_edge_element_data_list.append(port_edge_element_data);

      // port priority
      QJsonObject port_priority_element_data;
      QJsonArray port_priority_values = {ToQString(port_entry.GetPortPriority())};

      port_priority_element_data["value"] = port_priority_values;
      port_priority_element_data["portID"] = port_id;

      port_priority_element_data_list.append(port_priority_element_data);

      // path cost
      QJsonObject path_cost_element_data;
      QJsonArray path_cost_values = {ToQString(port_entry.GetPathCost())};

      path_cost_element_data["value"] = path_cost_values;
      path_cost_element_data["portID"] = port_id;

      port_path_cost_element_data_list.append(path_cost_element_data);

      // link type
      const QString link_type = kSpanningTreeLinkTypeEnum.value(port_entry.GetLinkType());
      // if link type is auto, there is no command.
      if (link_type != "auto") {
        QJsonObject link_type_element_data;
        QJsonArray link_type_values = {link_type};

        link_type_element_data["value"] = link_type_values;
        link_type_element_data["portID"] = port_id;

        port_link_type_element_data_list.append(link_type_element_data);
      }

      // bpdu guard
      if (port_entry.GetBpduGuard()) {
        QJsonObject bpdu_guard_element_data;
        QJsonArray bpdu_guard_values = {ToQString(port_entry.GetBpduGuard())};

        bpdu_guard_element_data["value"] = bpdu_guard_values;
        bpdu_guard_element_data["portID"] = port_id;

        bpdu_guard_element_data_list.append(bpdu_guard_element_data);
      }

      // root guard
      if (port_entry.GetRootGuard()) {
        QJsonObject root_guard_element_data;
        QJsonArray root_guard_values = {ToQString(port_entry.GetRootGuard())};

        root_guard_element_data["value"] = root_guard_values;
        root_guard_element_data["portID"] = port_id;

        root_guard_element_data_list.append(root_guard_element_data);
      }

      // loop guard
      if (port_entry.GetLoopGuard()) {
        QJsonObject loop_guard_element_data;
        QJsonArray loop_guard_values = {ToQString(port_entry.GetLoopGuard())};

        loop_guard_element_data["value"] = loop_guard_values;
        loop_guard_element_data["portID"] = port_id;

        loop_guard_element_data_list.append(loop_guard_element_data);
      }

      // bpdu filter
      if (port_entry.GetBpduFilter()) {
        QJsonObject bpdu_filter_element_data;
        QJsonArray bpdu_filter_values = {ToQString(port_entry.GetBpduFilter())};

        bpdu_filter_element_data["value"] = bpdu_filter_values;
        bpdu_filter_element_data["portID"] = port_id;

        bpdu_filter_element_data_list.append(bpdu_filter_element_data);
      }
    } else {
      continue;
    }
  }

  if (!port_active_element_data_list.isEmpty()) {
    AddElementToList(elements, "Spanning Tree STP/RSTP Port Active", port_active_element_data_list);
    AddElementToList(elements, "Spanning Tree STP/RSTP Port Edge", port_edge_element_data_list);
    AddElementToList(elements, "Spanning Tree STP/RSTP Port Priority", port_priority_element_data_list);
    AddElementToList(elements, "Spanning Tree STP/RSTP Port Path Cost", port_path_cost_element_data_list);
  }

  if (!port_link_type_element_data_list.isEmpty()) {
    AddElementToList(elements, "Spanning Tree STP/RSTP Port Link Type", port_link_type_element_data_list);
  }

  if (!bpdu_guard_element_data_list.isEmpty()) {
    AddElementToList(elements, "Spanning Tree STP/RSTP Port BPDU Guard", bpdu_guard_element_data_list);
  }
  if (!root_guard_element_data_list.isEmpty()) {
    AddElementToList(elements, "Spanning Tree STP/RSTP Port Root Guard", root_guard_element_data_list);
  }
  if (!loop_guard_element_data_list.isEmpty()) {
    AddElementToList(elements, "Spanning Tree STP/RSTP Port Loop Guard", loop_guard_element_data_list);
  }
  if (!bpdu_filter_element_data_list.isEmpty()) {
    AddElementToList(elements, "Spanning Tree STP/RSTP Port BPDU Filter", bpdu_filter_element_data_list);
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Redundancy Protocol", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenTsnRstp(const ActDeviceConfig &device_config, const qint64 &device_id,
                                           const QMap<qint64, QString> &interface_names, QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto rstp_tables_ = device_config.GetRstpTables();
  // no RSTP settings -> skip
  if (rstp_tables_.isEmpty()) {
    return act_status;
  }

  ActRstpTable rstp_table_;
  QMap<qint64, ActRstpTable>::iterator it = rstp_tables_.find(device_id);
  if (it == rstp_tables_.end()) {
    // no specified device RSTP setting -> skip
    return act_status;
  } else {
    rstp_table_ = it.value();
  }

  QList<QJsonObject> elements;

  if (rstp_table_.GetActive()) {
    AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Active", QString("enable"));
    if ((rstp_table_.GetSpanningTreeVersion() == ActSpanningTreeVersionEnum::kSTP) ||
        (rstp_table_.GetSpanningTreeVersion() == ActSpanningTreeVersionEnum::kRSTP)) {
      AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Compatibility",
                                  kSpanningTreeVersionEnum.value(rstp_table_.GetSpanningTreeVersion()));
    }
    AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Bridge Priority", rstp_table_.GetPriority());
    AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Forward Delay Time", rstp_table_.GetForwardDelay());
    AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Hello Time", rstp_table_.GetHelloTime());
    AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Max Age", rstp_table_.GetMaxAge());

  } else {
    AddSingleValueElementToList(elements, "Spanning Tree STP/RSTP Active", QString("disable"));
  }

  // no RSTP port settings
  auto port_entries = rstp_table_.GetRstpPortEntries();
  if (port_entries.isEmpty()) {
    QJsonObject feature = CreateFeature("Redundancy Protocol", elements);
    root_array->append(feature);
    return act_status;
  }

  QJsonArray port_edge_element_data_list;
  QJsonArray port_priority_element_data_list;
  QJsonArray port_path_cost_element_data_list;
  QJsonArray port_link_type_element_data_list;
  QJsonArray bpdu_guard_element_data_list;
  QJsonArray root_guard_element_data_list;
  QJsonArray loop_guard_element_data_list;
  QJsonArray bpdu_filter_element_data_list;

  for (ActRstpPortEntry port_entry : port_entries) {
    QString port_id = interface_names[port_entry.GetPortId()];

    // edge
    const QString edge = kActRstpEdgeEnumMap.key(port_entry.GetEdge());
    QJsonObject port_edge_element_data;
    QJsonArray port_edge_values = {edge};

    port_edge_element_data["value"] = port_edge_values;
    port_edge_element_data["portID"] = port_id;

    port_edge_element_data_list.append(port_edge_element_data);

    // port priority
    QJsonObject port_priority_element_data;
    QJsonArray port_priority_values = {ToQString(port_entry.GetPortPriority())};

    port_priority_element_data["value"] = port_priority_values;
    port_priority_element_data["portID"] = port_id;

    port_priority_element_data_list.append(port_priority_element_data);

    // path cost
    QJsonObject path_cost_element_data;
    QJsonArray path_cost_values = {ToQString(port_entry.GetPathCost())};

    path_cost_element_data["value"] = path_cost_values;
    path_cost_element_data["portID"] = port_id;

    port_path_cost_element_data_list.append(path_cost_element_data);

    // link type
    const QString link_type = kSpanningTreeLinkTypeEnum.value(port_entry.GetLinkType());
    // if link type is auto, there is no command.
    if (link_type != "auto") {
      QJsonObject link_type_element_data;
      QJsonArray link_type_values = {link_type};

      link_type_element_data["value"] = link_type_values;
      link_type_element_data["portID"] = port_id;

      port_link_type_element_data_list.append(link_type_element_data);
    }

    // bpdu filter
    if (port_entry.GetBpduFilter()) {
      QJsonObject bpdu_filter_element_data;
      QJsonArray bpdu_filter_values = {ToQString(port_entry.GetBpduFilter())};

      bpdu_filter_element_data["value"] = bpdu_filter_values;
      bpdu_filter_element_data["portID"] = port_id;

      bpdu_filter_element_data_list.append(bpdu_filter_element_data);
    }
  }

  AddElementToList(elements, "Spanning Tree STP/RSTP Port Edge", port_edge_element_data_list);
  AddElementToList(elements, "Spanning Tree STP/RSTP Port Priority", port_priority_element_data_list);
  AddElementToList(elements, "Spanning Tree STP/RSTP Port Path Cost", port_path_cost_element_data_list);

  if (!port_link_type_element_data_list.isEmpty()) {
    AddElementToList(elements, "Spanning Tree STP/RSTP Port Link Type", port_link_type_element_data_list);
  }

  if (!bpdu_filter_element_data_list.isEmpty()) {
    AddElementToList(elements, "Spanning Tree STP/RSTP Port BPDU Filter", bpdu_filter_element_data_list);
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Redundancy Protocol", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenTimeSync(const ActTimeSyncSettingItem &feature_group,
                                            const ActDeviceConfig &device_config, const qint64 &device_id,
                                            const QMap<qint64, QString> &interface_names, QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto time_sync_tables_ = device_config.GetTimeSyncTables();
  // no time sync settings -> skip
  if (time_sync_tables_.isEmpty()) {
    return act_status;
  }

  ActTimeSyncTable time_sync_table;
  QMap<qint64, ActTimeSyncTable>::iterator it = time_sync_tables_.find(device_id);
  if (it == time_sync_tables_.end()) {
    // no specified device time sync setting -> skip
    return act_status;
  } else {
    time_sync_table = it.value();
  }

  QList<QJsonObject> elements;

  auto profile = time_sync_table.GetProfile();

  // enable time sync
  AddSingleValueElementToList(elements, "Active Time Synchronization", ToQString(time_sync_table.GetEnabled()));

  // set every profile configurations
  if (feature_group.GetIEEE802Dot1AS_2011()) {
    ActTimeSync802Dot1ASConfig as = time_sync_table.GetIEEE_802Dot1AS_2011();
    HandleTimeSyncAS(interface_names, profile, as, &elements);
  }
  if (feature_group.GetIEEE1588_2008()) {
    ActTimeSync1588Config ptp = time_sync_table.GetIEEE_1588_2008();
    HandleTimeSyncPTP2008(feature_group, interface_names, profile, ptp, &elements);
  }
  if (feature_group.GetIEC61850_2016()) {
    ActTimeSyncIec61850Config power_profile_61850 = time_sync_table.GetIEC_61850_2016();
    HandleTimeSync61850(interface_names, profile, power_profile_61850, &elements);
  }
  if (feature_group.GetIEEEC37Dot238_2017()) {
    ActTimeSyncC37Dot238Config power_profile_c37 = time_sync_table.GetIEEE_C37Dot238_2017();
    HandleTimeSyncC37(interface_names, profile, power_profile_c37, &elements);
  }

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Time Synchronization", elements);

    root_array->append(feature);
  }

  return act_status;
}

void act::offline_config::HandleTimeSyncAS(const QMap<qint64, QString> &interface_names, ActTimeSyncProfileEnum profile,
                                           ActTimeSync802Dot1ASConfig &as, QList<QJsonObject> *elements) {
  if (as.GetPriority1() != 246) {
    AddSingleValueElementToList(*elements, "AS - Priority 1", ToQString(as.GetPriority1()));
  }
  if (as.GetPriority2() != 248) {
    AddSingleValueElementToList(*elements, "AS - Priority 2", ToQString(as.GetPriority2()));
  }
  if (as.GetAccuracyAlert() != 500) {
    AddSingleValueElementToList(*elements, "AS - Accuracy Alert", ToQString(as.GetAccuracyAlert()));
  }

  QJsonArray port_active_element_data_list;
  QJsonArray announce_interval_element_data_list;
  QJsonArray announce_receipt_timeout_element_data_list;
  QJsonArray sync_interval_element_data_list;
  QJsonArray sync_receipt_timeout_element_data_list;
  QJsonArray pdelay_request_interval_element_data_list;
  QJsonArray neighbor_prop_delay_thresh_element_data_list;

  for (ActTimeSync802Dot1ASPortEntry port_entry : as.GetPortEntries()) {
    QString port_id = interface_names[port_entry.GetPortId()];
    // enable port
    if (profile == ActTimeSyncProfileEnum::kIEEE_802Dot1AS_2011) {
      if (port_entry.GetEnable()) {
        AppendElementData(port_active_element_data_list, port_id, "enable");
      } else {
        AppendElementData(port_active_element_data_list, port_id, "disable");
      }
    }

    // announce interval
    if (port_entry.GetAnnounceInterval() != 0) {
      AppendElementData(announce_interval_element_data_list, port_id, ToQString(port_entry.GetAnnounceInterval()));
    }
    // announce receipt timeout
    if (port_entry.GetAnnounceReceiptTimeout() != 3) {
      AppendElementData(announce_receipt_timeout_element_data_list, port_id,
                        ToQString(port_entry.GetAnnounceReceiptTimeout()));
    }
    // sync interval
    if (port_entry.GetSyncInterval() != -3) {
      AppendElementData(sync_interval_element_data_list, port_id, ToQString(port_entry.GetSyncInterval()));
    }
    // sync receipt timeout
    if (port_entry.GetSyncReceiptTimeout() != 3) {
      AppendElementData(sync_receipt_timeout_element_data_list, port_id, ToQString(port_entry.GetSyncReceiptTimeout()));
    }
    // pdelay request interval
    if (port_entry.GetPdelayReqInterval() != 0) {
      AppendElementData(pdelay_request_interval_element_data_list, port_id,
                        ToQString(port_entry.GetPdelayReqInterval()));
    }
    // neighbor propagation delay threshold
    if (port_entry.GetNeighborPropDelayThresh() != 800) {
      AppendElementData(neighbor_prop_delay_thresh_element_data_list, port_id,
                        ToQString(port_entry.GetNeighborPropDelayThresh()));
    }
  }

  // add to list
  if (!port_active_element_data_list.isEmpty()) {
    AddElementToList(*elements, "AS - Active Port", port_active_element_data_list);
  }
  if (!announce_interval_element_data_list.isEmpty()) {
    AddElementToList(*elements, "AS - Port Announce Interval", announce_interval_element_data_list);
  }
  if (!announce_receipt_timeout_element_data_list.isEmpty()) {
    AddElementToList(*elements, "AS - Port Announce Timeout", announce_receipt_timeout_element_data_list);
  }
  if (!sync_interval_element_data_list.isEmpty()) {
    AddElementToList(*elements, "AS - Port Sync Interval", sync_interval_element_data_list);
  }
  if (!sync_receipt_timeout_element_data_list.isEmpty()) {
    AddElementToList(*elements, "AS - Port Sync Timeout", sync_receipt_timeout_element_data_list);
  }
  if (!pdelay_request_interval_element_data_list.isEmpty()) {
    AddElementToList(*elements, "AS - Port Pdelay_Req Interval", pdelay_request_interval_element_data_list);
  }
  if (!neighbor_prop_delay_thresh_element_data_list.isEmpty()) {
    AddElementToList(*elements, "AS - Port Neighbor Propagation Delay Threshold",
                     neighbor_prop_delay_thresh_element_data_list);
  }
}

void act::offline_config::HandleTimeSyncPTP2008(const ActTimeSyncSettingItem &feature_group,
                                                const QMap<qint64, QString> &interface_names,
                                                ActTimeSyncProfileEnum profile, ActTimeSync1588Config &ptp,
                                                QList<QJsonObject> *elements) {
  // clock type and delay mechanism
  QString clock_type = k1588ClockTypeEnum.value(ptp.GetClockType());
  QString delay_mechanism = k1588DelayMechanismTypeEnum.value(ptp.GetDelayMechanism());
  AddSingleValueElementToList(*elements, "PTP - Delay Mechanism", clock_type, delay_mechanism);
  // transport type
  QString transport_type = k1588TransportTypeEnum.value(ptp.GetTransportType());
  AddSingleValueElementToList(*elements, "PTP - Network Transport Type", transport_type);
  // priority 1
  if (ptp.GetPriority1() != 128) {
    AddSingleValueElementToList(*elements, "PTP - Priority 1", ToQString(ptp.GetPriority1()));
  }
  // priority 2
  if (ptp.GetPriority2() != 128) {
    AddSingleValueElementToList(*elements, "PTP - Priority 2", ToQString(ptp.GetPriority2()));
  }
  // domain number
  if (ptp.GetDomainNumber() != 0) {
    AddSingleValueElementToList(*elements, "PTP - Domain", ToQString(ptp.GetDomainNumber()));
  }
  // clock mode - one step/two step
  bool support_clock_mode = feature_group.GetIEEE1588_2008_ClockMode();
  if (support_clock_mode) {
    if (ptp.GetClockMode() == Act1588ClockModeEnum::kTwoStep) {
      AddSingleValueElementToList(*elements, "PTP - Active Two-step Mode", QString("enable"));
    }
  }
  // accuracy alert
  if (ptp.GetAccuracyAlert() != 1000) {
    AddSingleValueElementToList(*elements, "PTP - Accuracy Alert", ToQString(ptp.GetAccuracyAlert()));
  }
  // max step removed
  bool support_max_step_removed = feature_group.GetIEEE1588_2008_MaximumStepsRemoved();
  if (support_max_step_removed) {
    if (ptp.GetMaximumStepsRemoved() != 255) {
      AddSingleValueElementToList(*elements, "PTP - Max Step Removed", ToQString(ptp.GetMaximumStepsRemoved()));
    }
  }

  QJsonArray port_active_element_data_list;
  QJsonArray announce_interval_element_data_list;
  QJsonArray announce_receipt_timeout_element_data_list;
  QJsonArray sync_interval_element_data_list;
  QJsonArray delay_request_interval_element_data_list;
  QJsonArray pdelay_request_interval_element_data_list;

  for (ActTimeSync1588PortEntry port_entry : ptp.GetPortEntries()) {
    QString port_id = interface_names[port_entry.GetPortId()];
    // enable port
    if (profile == ActTimeSyncProfileEnum::kIEEE_1588_2008) {
      if (port_entry.GetEnable()) {
        AppendElementData(port_active_element_data_list, port_id, "enable");
      } else {
        AppendElementData(port_active_element_data_list, port_id, "disable");
      }
    }

    // announce interval
    if (port_entry.GetAnnounceInterval() != 0) {
      AppendElementData(announce_interval_element_data_list, port_id, ToQString(port_entry.GetAnnounceInterval()));
    }
    // announce receipt timeout
    if (port_entry.GetAnnounceReceiptTimeout() != 3) {
      AppendElementData(announce_receipt_timeout_element_data_list, port_id,
                        ToQString(port_entry.GetAnnounceReceiptTimeout()));
    }
    // sync interval
    if (port_entry.GetSyncInterval() != -3) {
      AppendElementData(sync_interval_element_data_list, port_id, ToQString(port_entry.GetSyncInterval()));
    }
    // delay request interval
    if (port_entry.GetDelayReqInterval() != 0) {
      AppendElementData(delay_request_interval_element_data_list, port_id, ToQString(port_entry.GetDelayReqInterval()));
    }
    // pdelay request interval
    if (port_entry.GetPdelayReqInterval() != 0) {
      AppendElementData(pdelay_request_interval_element_data_list, port_id,
                        ToQString(port_entry.GetPdelayReqInterval()));
    }
  }

  // add to list
  if (!port_active_element_data_list.isEmpty()) {
    AddElementToList(*elements, "PTP - Active Port", port_active_element_data_list);
  }
  if (!announce_interval_element_data_list.isEmpty()) {
    AddElementToList(*elements, "PTP - Port Announce Interval", announce_interval_element_data_list);
  }
  if (!announce_receipt_timeout_element_data_list.isEmpty()) {
    AddElementToList(*elements, "PTP - Port Announce Timeout", announce_receipt_timeout_element_data_list);
  }
  if (!sync_interval_element_data_list.isEmpty()) {
    AddElementToList(*elements, "PTP - Port Sync Interval", sync_interval_element_data_list);
  }
  if (!delay_request_interval_element_data_list.isEmpty()) {
    AddElementToList(*elements, "PTP - Port Delay_Req Interval", delay_request_interval_element_data_list);
  }
  if (!pdelay_request_interval_element_data_list.isEmpty()) {
    AddElementToList(*elements, "PTP - Port Pdelay_Req Interval", pdelay_request_interval_element_data_list);
  }
}

void act::offline_config::HandleTimeSync61850(const QMap<qint64, QString> &interface_names,
                                              ActTimeSyncProfileEnum profile,
                                              ActTimeSyncIec61850Config &power_profile_61850,
                                              QList<QJsonObject> *elements) {
  // clock type and delay mechanism
  QString clock_type = k1588ClockTypeEnum.value(power_profile_61850.GetClockType());
  AddSingleValueElementToList(*elements, "Power Profile (IEC 61850) - Delay Mechanism", clock_type);
  // priority 1
  if (power_profile_61850.GetPriority1() != 128) {
    AddSingleValueElementToList(*elements, "Power Profile (IEC 61850) - Priority 1",
                                ToQString(power_profile_61850.GetPriority1()));
  }
  // priority 2
  if (power_profile_61850.GetPriority2() != 128) {
    AddSingleValueElementToList(*elements, "Power Profile (IEC 61850) - Priority 2",
                                ToQString(power_profile_61850.GetPriority2()));
  }
  // domain number
  if (power_profile_61850.GetDomainNumber() != 0) {
    AddSingleValueElementToList(*elements, "Power Profile (IEC 61850) - Domain",
                                ToQString(power_profile_61850.GetDomainNumber()));
  }
  // accuracy alert
  if (power_profile_61850.GetAccuracyAlert() != 1000) {
    AddSingleValueElementToList(*elements, "Power Profile (IEC 61850) - Accuracy Alert",
                                ToQString(power_profile_61850.GetAccuracyAlert()));
  }
  // max step removed
  if (power_profile_61850.GetMaximumStepsRemoved() != 255) {
    AddSingleValueElementToList(*elements, "Power Profile (IEC 61850) - Max Step Removed",
                                ToQString(power_profile_61850.GetMaximumStepsRemoved()));
  }

  QJsonArray port_active_element_data_list;

  for (ActTimeSyncDefaultPortEntry port_entry : power_profile_61850.GetPortEntries()) {
    QString port_id = interface_names[port_entry.GetPortId()];
    // enable port
    if (profile == ActTimeSyncProfileEnum::kIEC_61850_2016) {
      if (port_entry.GetEnable()) {
        AppendElementData(port_active_element_data_list, port_id, "enable");
      } else {
        AppendElementData(port_active_element_data_list, port_id, "disable");
      }
    }
  }

  // add to list
  if (!port_active_element_data_list.isEmpty()) {
    AddElementToList(*elements, "Power Profile (IEC 61850) - Active Port", port_active_element_data_list);
  }
}

void act::offline_config::HandleTimeSyncC37(const QMap<qint64, QString> &interface_names,
                                            ActTimeSyncProfileEnum profile,
                                            ActTimeSyncC37Dot238Config &power_profile_c37,
                                            QList<QJsonObject> *elements) {
  // clock type and delay mechanism
  QString clock_type = k1588ClockTypeEnum.value(power_profile_c37.GetClockType());
  AddSingleValueElementToList(*elements, "Power Profile (IEEE C37.238) - Delay Mechanism", clock_type);
  // priority 1
  if (power_profile_c37.GetPriority1() != 128) {
    AddSingleValueElementToList(*elements, "Power Profile (IEEE C37.238) - Priority 1",
                                ToQString(power_profile_c37.GetPriority1()));
  }
  // priority 2
  if (power_profile_c37.GetPriority2() != 128) {
    AddSingleValueElementToList(*elements, "Power Profile (IEEE C37.238) - Priority 2",
                                ToQString(power_profile_c37.GetPriority2()));
  }
  // domain number
  if (power_profile_c37.GetDomainNumber() != 0) {
    AddSingleValueElementToList(*elements, "Power Profile (IEEE C37.238) - Domain",
                                ToQString(power_profile_c37.GetDomainNumber()));
  }
  // grandmaster ID
  if (power_profile_c37.GetGrandmasterId() != 255) {
    AddSingleValueElementToList(*elements, "Power Profile (IEEE C37.238) - Grandmaster Id",
                                ToQString(power_profile_c37.GetGrandmasterId()));
  }
  // accuracy alert
  if (power_profile_c37.GetAccuracyAlert() != 1000) {
    AddSingleValueElementToList(*elements, "Power Profile (IEEE C37.238) - Accuracy Alert",
                                ToQString(power_profile_c37.GetAccuracyAlert()));
  }
  // max step removed
  if (power_profile_c37.GetMaximumStepsRemoved() != 255) {
    AddSingleValueElementToList(*elements, "Power Profile (IEEE C37.238) - Max Step Removed",
                                ToQString(power_profile_c37.GetMaximumStepsRemoved()));
  }

  QJsonArray port_active_element_data_list;

  for (ActTimeSyncDefaultPortEntry port_entry : power_profile_c37.GetPortEntries()) {
    QString port_id = interface_names[port_entry.GetPortId()];
    // enable port
    if (profile == ActTimeSyncProfileEnum::kIEC_61850_2016) {
      if (port_entry.GetEnable()) {
        AppendElementData(port_active_element_data_list, port_id, "enable");
      } else {
        AppendElementData(port_active_element_data_list, port_id, "disable");
      }
    }
  }

  // add to list
  if (!port_active_element_data_list.isEmpty()) {
    AddElementToList(*elements, "Power Profile (IEEE C37.238) - Active Port", port_active_element_data_list);
  }
}

ACT_STATUS act::offline_config::GenMgmtInterface(const ActDeviceConfig &device_config, const qint64 &device_id,
                                                 QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto mgmt_interface_tables_ = device_config.GetManagementInterfaceTables();
  // no management interface settings -> skip
  if (mgmt_interface_tables_.isEmpty()) {
    return act_status;
  }

  ActManagementInterfaceTable mgmt_interface_table;
  QMap<qint64, ActManagementInterfaceTable>::iterator it = mgmt_interface_tables_.find(device_id);
  if (it == mgmt_interface_tables_.end()) {
    // no specified device management interface setting -> skip
    return act_status;
  } else {
    mgmt_interface_table = it.value();
  }

  QList<QJsonObject> elements;

  // new moxa command
  AddSingleValueElementToList(elements, "Active Moxa Service",
                              mgmt_interface_table.GetEncryptedMoxaService().GetEnable());
  // HTTP
  AddSingleValueElementToList(elements, "Active HTTP", mgmt_interface_table.GetHttpService().GetEnable());
  AddSingleValueElementToList(elements, "Service Port of HTTP", mgmt_interface_table.GetHttpService().GetPort());
  // HTTPS
  AddSingleValueElementToList(elements, "Active HTTPS", mgmt_interface_table.GetHttpsService().GetEnable());
  AddSingleValueElementToList(elements, "Service Port of HTTPS", mgmt_interface_table.GetHttpsService().GetPort());
  // SNMP
  QString snmp_mode = kSnmpServiceModeEnum.value(mgmt_interface_table.GetSnmpService().GetMode());
  AddSingleValueElementToList(elements, "Active SNMP", snmp_mode);
  AddSingleValueElementToList(elements, "Service Port of SNMP", mgmt_interface_table.GetSnmpService().GetPort());
  // SSH
  AddSingleValueElementToList(elements, "Active SSH", mgmt_interface_table.GetSSHService().GetEnable());
  AddSingleValueElementToList(elements, "Service Port of SSH", mgmt_interface_table.GetSSHService().GetPort());
  // Telnet
  AddSingleValueElementToList(elements, "Active Telnet", mgmt_interface_table.GetTelnetService().GetEnable());
  AddSingleValueElementToList(elements, "Service Port of Telnet", mgmt_interface_table.GetTelnetService().GetPort());

  // max login session
  AddSingleValueElementToList(elements, "Max Login Sessions of HTTP and HTTPS",
                              mgmt_interface_table.GetHttpMaxLoginSessions());
  AddSingleValueElementToList(elements, "Max Login Sessions of Telnet and SSH",
                              mgmt_interface_table.GetTerminalMaxLoginSessions());

  if (!elements.isEmpty()) {
    QJsonObject feature = CreateFeature("Management Interface", elements);

    root_array->append(feature);
  }

  return act_status;
}

ACT_STATUS act::offline_config::GenUserAccount(const ActDeviceConfig &device_config, const qint64 &device_id,
                                               QJsonArray *root_array) {
  ACT_STATUS_INIT();

  auto account_tables_ = device_config.GetUserAccountTables();
  // no user account settings -> skip
  if (account_tables_.isEmpty()) {
    return act_status;
  }

  ActUserAccountTable account_table;
  QMap<qint64, ActUserAccountTable>::iterator it = account_tables_.find(device_id);
  if (it == account_tables_.end()) {
    // no specified device user account setting -> skip
    return act_status;
  } else {
    account_table = it.value();
  }

  QList<QJsonObject> elements;

  // key = account name; value = account setting
  QMap<QString, ActUserAccount> accounts = account_table.GetAccounts();

  for (auto account : accounts.values()) {
    if (account.GetActive()) {
      AddSingleValueElementToList(elements, "Active", QString("true"));
    } else {
      AddSingleValueElementToList(elements, "Active", QString("false"));
    }

    AddSingleValueElementToList(elements, "Account Name", account.GetUsername());
    AddSingleValueElementToList(elements, "New Password", account.GetPassword());

    QString role = kUserAccountRoleEnum.value(account.GetRole());
    AddSingleValueElementToList(elements, "Authority", role);

    AddSingleValueElementToList(elements, "Email", account.GetEmail());

    if (!elements.isEmpty()) {
      QJsonObject feature = CreateFeature("Account Password", elements);

      root_array->append(feature);
    }
  }

  return act_status;
}