/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <QDateTime>

#include "act_json.hpp"
#include "deploy_entry/act_default_priority_entry.hpp"
#include "deploy_entry/act_frer_entry.hpp"
#include "deploy_entry/act_gate_parameters.hpp"
#include "deploy_entry/act_management_interface_service.hpp"
#include "deploy_entry/act_port_setting_entry.hpp"
#include "deploy_entry/act_rstp_port_entry.hpp"
#include "deploy_entry/act_snmp_trap_host_entry.hpp"
#include "deploy_entry/act_stad_config_entry.hpp"
#include "deploy_entry/act_stad_port_entry.hpp"
#include "deploy_entry/act_static_forward_entry.hpp"
#include "deploy_entry/act_stream_identity_entry.hpp"
#include "deploy_entry/act_time_sync_config.hpp"
#include "deploy_entry/act_user_account.hpp"
#include "deploy_entry/act_vlan_static_entry.hpp"
#include "topology/act_device.hpp"

class ActDeployTableBase : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);

 public:
  ActDeployTableBase() : device_id_(-1) {}

  ActDeployTableBase(const qint64 &device_id) : device_id_(device_id) {}
  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActDeployTableBase &x) {
    return qHash(x.device_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActDeployTableBase &x, const ActDeployTableBase &y) {
    return x.device_id_ == y.device_id_;
  }
};

enum class ActNetworkSettingModeEnum { kStatic = 0, kDHCP = 1, kBootp = 2 };

static const QMap<QString, ActNetworkSettingModeEnum> kActNetworkSettingModeEnumMap = {
    {"Static", ActNetworkSettingModeEnum::kStatic},
    {"DHCP", ActNetworkSettingModeEnum::kDHCP},
    {"Bootp", ActNetworkSettingModeEnum::kBootp}};

class ActNetworkSettingTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActNetworkSettingModeEnum, network_setting_mode, NetworkSettingMode);
  ACT_JSON_FIELD(QString, ip_address, IpAddress);
  ACT_JSON_FIELD(QString, subnet_mask, SubnetMask);
  ACT_JSON_FIELD(QString, gateway, Gateway);
  ACT_JSON_FIELD(QString, dns1, DNS1);
  ACT_JSON_FIELD(QString, dns2, DNS2);

 public:
  ActNetworkSettingTable() {
    network_setting_mode_ = ActNetworkSettingModeEnum::kStatic;
    SetDeviceId(-1);
    ip_address_ = "";
    subnet_mask_ = "255.255.255.0";
    gateway_ = "";
    dns1_ = "";
    dns2_ = "";
  }
  ActNetworkSettingTable(const qint64 &device_id) : ActNetworkSettingTable() { SetDeviceId(device_id); }

  ActNetworkSettingTable(const qint64 &device_id, const QString &ip_address) : ActNetworkSettingTable() {
    SetDeviceId(device_id);
    this->ip_address_ = ip_address;
  }

  ActNetworkSettingTable(const ActDevice &device) : ActNetworkSettingTable() {
    SetDeviceId(device.GetId());
    ip_address_ = device.GetIpv4().GetIpAddress();
    if (!device.GetIpv4().GetSubnetMask().isEmpty()) {
      subnet_mask_ = device.GetIpv4().GetSubnetMask();
    }
    gateway_ = device.GetIpv4().GetGateway();
    dns1_ = device.GetIpv4().GetDNS1();
    dns2_ = device.GetIpv4().GetDNS2();
  }
};

class ActMappingDeviceIpSettingTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, mac_address, MacAddress);
  ACT_JSON_FIELD(QString, offline_ip, OfflineIP);
  ACT_JSON_FIELD(QString, online_ip, OnlineIP);
  ACT_JSON_FIELD(QString, subnet_mask, SubnetMask);
  ACT_JSON_FIELD(QString, gateway, Gateway);
  ACT_JSON_FIELD(QString, dns1, DNS1);
  ACT_JSON_FIELD(QString, dns2, DNS2);

 public:
  ActMappingDeviceIpSettingTable() {
    mac_address_ = "";
    offline_ip_ = "";
    online_ip_ = "";
    subnet_mask_ = "255.255.255.0";
    gateway_ = "";
    dns1_ = "";
    dns2_ = "";
  }
  ActMappingDeviceIpSettingTable(const qint64 &offline_device_id) : ActMappingDeviceIpSettingTable() {
    SetDeviceId(offline_device_id);
  }

  ActMappingDeviceIpSettingTable(const qint64 &offline_device_id, const QString &offline_ip, const QString &online_ip,
                                 const QString &mac_address)
      : ActMappingDeviceIpSettingTable() {
    SetDeviceId(offline_device_id);
    offline_ip_ = offline_ip;
    online_ip_ = online_ip;
    mac_address_ = mac_address;
  }
};

class ActUserAccountTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, sync_connection_account, SyncConnectionAccount);
  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActUserAccount, accounts, Accounts);

 public:
  ActUserAccountTable() : ActDeployTableBase() { this->sync_connection_account_ = ""; }
  ActUserAccountTable(const qint64 &device_id) : ActUserAccountTable() { SetDeviceId(device_id); }

  ACT_STATUS HidePassword() {
    ACT_STATUS_INIT();
    for (auto account : this->accounts_.keys()) {
      this->accounts_[account].HidePassword();
    }
    return act_status;
  }

  ACT_STATUS EncryptPassword() {
    ACT_STATUS_INIT();
    for (auto account : this->accounts_.keys()) {
      this->accounts_[account].EncryptPassword();
    }
    return act_status;
  }

  ACT_STATUS DecryptPassword() {
    ACT_STATUS_INIT();

    for (auto account : this->accounts_.keys()) {
      this->accounts_[account].DecryptPassword();
    }

    return act_status;
  }
};

class ActLoginPolicyTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, login_message, LoginMessage);
  ACT_JSON_FIELD(QString, login_authentication_failure_message, LoginAuthenticationFailureMessage);
  ACT_JSON_FIELD(bool, login_failure_lockout, LoginFailureLockout);
  ACT_JSON_FIELD(qint32, retry_failure_threshold, RetryFailureThreshold);
  ACT_JSON_FIELD(qint32, lockout_duration, LockoutDuration);
  ACT_JSON_FIELD(qint32, auto_logout_after, AutoLogoutAfter);

 public:
  ActLoginPolicyTable() : ActDeployTableBase() {
    this->login_message_ = "";
    this->login_authentication_failure_message_ = "";
    this->login_failure_lockout_ = false;
    this->retry_failure_threshold_ = 5;
    this->lockout_duration_ = 5;
    this->auto_logout_after_ = 5;
  }

  ActLoginPolicyTable(const qint64 &device_id) : ActLoginPolicyTable() { SetDeviceId(device_id); }

  ActLoginPolicyTable(const ActDevice &device) : ActLoginPolicyTable() { SetDeviceId(device.GetId()); }
};

class ActSnmpTrapSettingTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActSnmpTrapHostEntry, host_list, HostList);

 public:
  ActSnmpTrapSettingTable() : ActDeployTableBase() {}

  ActSnmpTrapSettingTable(const qint64 &device_id) : ActSnmpTrapSettingTable() { SetDeviceId(device_id); }

  ActSnmpTrapSettingTable(const ActDevice &device) : ActSnmpTrapSettingTable() { SetDeviceId(device.GetId()); }
};

class ActSyslogSettingTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enabled, Enabled);
  ACT_JSON_FIELD(bool, syslog_server_1, SyslogServer1);
  ACT_JSON_FIELD(QString, address_1, Address1);
  ACT_JSON_FIELD(quint16, port_1, Port1);
  ACT_JSON_FIELD(bool, syslog_server_2, SyslogServer2);
  ACT_JSON_FIELD(QString, address_2, Address2);
  ACT_JSON_FIELD(quint16, port_2, Port2);
  ACT_JSON_FIELD(bool, syslog_server_3, SyslogServer3);
  ACT_JSON_FIELD(QString, address_3, Address3);
  ACT_JSON_FIELD(quint16, port_3, Port3);

 public:
  ActSyslogSettingTable() : ActDeployTableBase() {
    this->enabled_ = false;
    this->syslog_server_1_ = false;
    this->address_1_ = "";
    this->port_1_ = 514;
    this->syslog_server_2_ = false;
    this->address_2_ = "";
    this->port_2_ = 514;
    this->syslog_server_3_ = false;
    this->address_3_ = "";
    this->port_3_ = 514;
  }

  ActSyslogSettingTable(const qint64 &device_id) : ActSyslogSettingTable() { SetDeviceId(device_id); }

  ActSyslogSettingTable(const ActDevice &device) : ActSyslogSettingTable() { SetDeviceId(device.GetId()); }
};

class ActTimeDate : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, year, Year);
  ACT_JSON_FIELD(quint16, month, Month);
  ACT_JSON_FIELD(quint16, date, Date);
  ACT_JSON_FIELD(quint16, hour, Hour);
  ACT_JSON_FIELD(quint16, minute, Minute);

 public:
  ActTimeDate() {
    QDateTime current_date_time = QDateTime::currentDateTime();
    this->year_ = current_date_time.date().year();
    this->month_ = 1;
    this->date_ = 1;
    this->hour_ = 0;
    this->minute_ = 0;
  };
};

class ActTimeDay : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, month, Month);
  ACT_JSON_FIELD(quint16, week, Week);
  ACT_JSON_FIELD(quint16, day, Day);
  ACT_JSON_FIELD(quint16, hour, Hour);
  ACT_JSON_FIELD(quint16, minute, Minute);

 public:
  ActTimeDay() {
    this->month_ = 1;
    this->week_ = 1;
    this->day_ = 1;
    this->hour_ = 0;
    this->minute_ = 0;
  };
};

enum class ActClockSourceEnum { kLocal = 1, kSNTP = 2, kNTP = 3, kPTP = 4 };

static const QMap<QString, ActClockSourceEnum> kActClockSourceEnumMap = {{"Local", ActClockSourceEnum::kLocal},
                                                                         {"SNTP", ActClockSourceEnum::kSNTP},
                                                                         {"NTP", ActClockSourceEnum::kNTP},
                                                                         {"PTP", ActClockSourceEnum::kPTP}};

enum class ActTimeZoneEnum {
  kIDLW = 1,
  kSST,
  kHST,
  kMIT,
  kAKST,
  kPST,
  kMST,
  kCST,
  kEST,
  kAST,
  kNST,
  kBRT,
  kFNT,
  kCVT,
  kGMT,
  kCET,
  kEET,
  kMSK,
  kIRST,
  kGST,
  kAFT,
  kPKT,
  kIST,
  kNPT,
  kBHT,
  kMMT,
  kICT,
  kBJT,
  kJST,
  kACST,
  kAEST,
  kLHST,
  kVUT,
  kNZST,
  kCHAST,
  kPHOT,
  kLINT
};

static const QMap<QString, ActTimeZoneEnum> kActTimeZoneEnumMap = {
    {"UTC-12:00", ActTimeZoneEnum::kIDLW},  {"UTC-11:00", ActTimeZoneEnum::kSST},
    {"UTC-10:00", ActTimeZoneEnum::kHST},   {"UTC-09:30", ActTimeZoneEnum::kMIT},
    {"UTC-09:00", ActTimeZoneEnum::kAKST},  {"UTC-08:00", ActTimeZoneEnum::kPST},
    {"UTC-07:00", ActTimeZoneEnum::kMST},   {"UTC-06:00", ActTimeZoneEnum::kCST},
    {"UTC-05:00", ActTimeZoneEnum::kEST},   {"UTC-04:00", ActTimeZoneEnum::kAST},
    {"UTC-03:30", ActTimeZoneEnum::kNST},   {"UTC-03:00", ActTimeZoneEnum::kBRT},
    {"UTC-02:00", ActTimeZoneEnum::kFNT},   {"UTC-01:00", ActTimeZoneEnum::kCVT},
    {"UTC+00:00", ActTimeZoneEnum::kGMT},   {"UTC+01:00", ActTimeZoneEnum::kCET},
    {"UTC+02:00", ActTimeZoneEnum::kEET},   {"UTC+03:00", ActTimeZoneEnum::kMSK},
    {"UTC+03:30", ActTimeZoneEnum::kIRST},  {"UTC+04:00", ActTimeZoneEnum::kGST},
    {"UTC+04:30", ActTimeZoneEnum::kAFT},   {"UTC+05:00", ActTimeZoneEnum::kPKT},
    {"UTC+05:30", ActTimeZoneEnum::kIST},   {"UTC+05:45", ActTimeZoneEnum::kNPT},
    {"UTC+06:00", ActTimeZoneEnum::kBHT},   {"UTC+06:30", ActTimeZoneEnum::kMMT},
    {"UTC+07:00", ActTimeZoneEnum::kICT},   {"UTC+08:00", ActTimeZoneEnum::kBJT},
    {"UTC+09:00", ActTimeZoneEnum::kJST},   {"UTC+09:30", ActTimeZoneEnum::kACST},
    {"UTC+10:00", ActTimeZoneEnum::kAEST},  {"UTC+10:30", ActTimeZoneEnum::kLHST},
    {"UTC+11:00", ActTimeZoneEnum::kVUT},   {"UTC+12:00", ActTimeZoneEnum::kNZST},
    {"UTC+12:45", ActTimeZoneEnum::kCHAST}, {"UTC+13:00", ActTimeZoneEnum::kPHOT},
    {"UTC+14:00", ActTimeZoneEnum::kLINT}};

class ActTimeSettingTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  // Time
  ACT_JSON_ENUM(ActClockSourceEnum, clock_source, ClockSource);
  ACT_JSON_FIELD(QString, ntp_time_server_1, NTPTimeServer1);
  ACT_JSON_FIELD(QString, ntp_time_server_2, NTPTimeServer2);
  ACT_JSON_FIELD(QString, sntp_time_server_1, SNTPTimeServer1);
  ACT_JSON_FIELD(QString, sntp_time_server_2, SNTPTimeServer2);

  // TimeZone
  ACT_JSON_ENUM(ActTimeZoneEnum, time_zone, TimeZone);
  ACT_JSON_FIELD(bool, daylight_saving_time, DaylightSavingTime);
  ACT_JSON_FIELD(QString, offset, Offset);  // HH:MM
  ACT_JSON_OBJECT(ActTimeDay, start, Start);
  ACT_JSON_OBJECT(ActTimeDay, end, End);

 public:
  ActTimeSettingTable() : ActDeployTableBase() {
    this->clock_source_ = ActClockSourceEnum::kLocal;
    this->time_zone_ = ActTimeZoneEnum::kGMT;
    this->daylight_saving_time_ = false;
    this->offset_ = "01:00";
    this->ntp_time_server_1_ = "time.nist.gov";
    this->ntp_time_server_2_ = "";
    this->sntp_time_server_1_ = "time.nist.gov";
    this->sntp_time_server_2_ = "";
  }

  ActTimeSettingTable(const qint64 &device_id) : ActTimeSettingTable() { SetDeviceId(device_id); }

  ActTimeSettingTable(const ActDevice &device) : ActTimeSettingTable() { SetDeviceId(device.GetId()); }
};

class ActLoopProtectionTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, network_loop_protection, NetworkLoopProtection);
  ACT_JSON_FIELD(qint32, detect_interval, DetectInterval);

 public:
  ActLoopProtectionTable() : ActDeployTableBase() {
    this->network_loop_protection_ = false;
    this->detect_interval_ = 10;
  }

  ActLoopProtectionTable(const qint64 &device_id) : ActLoopProtectionTable() { SetDeviceId(device_id); }

  ActLoopProtectionTable(const ActDevice &device) : ActLoopProtectionTable() { SetDeviceId(device.GetId()); }
};

class ActManagementInterfaceTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActMgmtEncryptedMoxaService, encrypted_moxa_service, EncryptedMoxaService);
  ACT_JSON_OBJECT(ActMgmtHttpService, http_service, HttpService);
  ACT_JSON_OBJECT(ActMgmtHttpsService, https_service, HttpsService);
  ACT_JSON_OBJECT(ActMgmtSnmpService, snmp_service, SnmpService);
  ACT_JSON_OBJECT(ActMgmtSshService, ssh_service, SSHService);
  ACT_JSON_OBJECT(ActMgmtTelnetService, telnet_service, TelnetService);

  ACT_JSON_FIELD(quint16, http_max_login_sessions, HttpMaxLoginSessions);
  ACT_JSON_FIELD(quint16, terminal_max_login_sessions, TerminalMaxLoginSessions);

 public:
  ActManagementInterfaceTable() : ActDeployTableBase() {
    this->http_max_login_sessions_ = 5;
    this->terminal_max_login_sessions_ = 1;
  }

  ActManagementInterfaceTable(const qint64 &device_id) : ActDeployTableBase(device_id) {}
};

class ActInformationSettingTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, device_name, DeviceName);
  ACT_JSON_FIELD(QString, location, Location);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_FIELD(QString, contact_information, ContactInformation);

 public:
  ActInformationSettingTable() : ActDeployTableBase() {
    this->device_name_ = "";
    this->location_ = "";
    this->description_ = "";
    this->contact_information_ = "";
  }

  ActInformationSettingTable(const qint64 &device_id) : ActDeployTableBase(device_id) {}

  ActInformationSettingTable(const qint64 &device_id, const QString &device_name, const QString &location,
                             const QString &description, const QString &contact_information)
      : ActDeployTableBase(device_id) {
    this->device_name_ = device_name;
    this->location_ = location;
    this->description_ = description;
    this->contact_information_ = contact_information;
  }

  ActInformationSettingTable(const ActDevice &device) : ActInformationSettingTable() { SetDeviceId(device.GetId()); }
};

class ActGclTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActInterfaceGateParameters, interfaces_gate_parameters, InterfacesGateParameters);

 public:
  ActGclTable() {}
  ActGclTable(const qint64 &device_id) { SetDeviceId(device_id); }
  ActGclTable(const ActDevice &device) {
    SetDeviceId(device.GetId());

    // Add the init port_retry by device's interface
    for (ActInterface intf : device.GetInterfaces()) {
      this->interfaces_gate_parameters_.insert(ActInterfaceGateParameters(intf.GetInterfaceId()));
    }
  }
  ActGclTable(const qint64 &device_id, const QSet<ActInterfaceGateParameters> &interfaces_gate_parameters) {
    SetDeviceId(device_id);
    interfaces_gate_parameters_ = interfaces_gate_parameters;
  }
};

class ActTeMstidTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE
  // ACT_JSON_QT_SET_OBJECTS(Ieee802Dot1qBridge, ieee802_dot1q_bridges, Ieee802Dot1qBridges);
};

class ActCbTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActStreamIdentityEntry, stream_identity_list, StreamIdentityList);
  ACT_JSON_OBJECT(ActFREREntry, frer, FRER);

 private:
  quint32 stream_identity_index_;

 public:
  ActCbTable() { stream_identity_index_ = 0; }
  ActCbTable(const qint64 &device_id) {
    stream_identity_index_ = 0;
    SetDeviceId(device_id);
  }

  quint32 GetStreamIdentityIndex() { return stream_identity_index_++; };
};

enum class ActTimeSyncProfileEnum {
  kIEEE_802Dot1AS_2011 = 1,
  kIEEE_1588_2008 = 3,
  kIEC_61850_2016 = 4,
  kIEEE_C37Dot238_2017 = 5
};

static const QMap<QString, ActTimeSyncProfileEnum> kActTimeSyncProfileEnumMap = {
    {"IEEE_802Dot1AS_2011", ActTimeSyncProfileEnum::kIEEE_802Dot1AS_2011},
    {"IEEE_1588_2008", ActTimeSyncProfileEnum::kIEEE_1588_2008},
    {"IEC_61850_2016", ActTimeSyncProfileEnum::kIEC_61850_2016},
    {"IEEE_C37Dot238_2017", ActTimeSyncProfileEnum::kIEEE_C37Dot238_2017}};

class ActTimeSyncTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  // mxptp
  ACT_JSON_FIELD(bool, enabled, Enabled);
  ACT_JSON_ENUM(ActTimeSyncProfileEnum, profile, Profile);

  // profile config
  ACT_JSON_OBJECT(ActTimeSync802Dot1ASConfig, ieee_802dot1as_2011, IEEE_802Dot1AS_2011);
  ACT_JSON_OBJECT(ActTimeSync1588Config, ieee_1588_2008, IEEE_1588_2008);
  ACT_JSON_OBJECT(ActTimeSyncIec61850Config, iec_61850_2016, IEC_61850_2016);
  ACT_JSON_OBJECT(ActTimeSyncC37Dot238Config, ieee_c37dot238_2017, IEEE_C37Dot238_2017);

 public:
  ActTimeSyncTable() {
    this->enabled_ = true;
    this->profile_ = ActTimeSyncProfileEnum::kIEEE_1588_2008;
  }
  ActTimeSyncTable(const qint64 &device_id) : ActTimeSyncTable() { SetDeviceId(device_id); }

  ActTimeSyncTable(const ActDevice &device) : ActTimeSyncTable() {
    SetDeviceId(device.GetId());
    this->ieee_802dot1as_2011_ = ActTimeSync802Dot1ASConfig(device.GetInterfaces());
    this->ieee_1588_2008_ = ActTimeSync1588Config(device.GetInterfaces());
    this->iec_61850_2016_ = ActTimeSyncIec61850Config(device.GetInterfaces());
    this->ieee_c37dot238_2017_ = ActTimeSyncC37Dot238Config(device.GetInterfaces());
  }
};

class ActTimeSyncBaseConfig : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // mxptp
  ACT_JSON_FIELD(bool, enabled, Enabled);
  ACT_JSON_ENUM(ActTimeSyncProfileEnum, profile, Profile);

 public:
  ActTimeSyncBaseConfig() {
    this->enabled_ = false;
    this->profile_ = ActTimeSyncProfileEnum::kIEEE_1588_2008;
  }
};

// IEEE8021-SPANNING-TREE-MIB > ieee8021SpanningTreeVersion
enum class ActSpanningTreeVersionEnum { kSTP = 0, kRSTP = 2, kNotSupport = 3 };

static const QMap<QString, ActSpanningTreeVersionEnum> kActSpanningTreeVersionEnumMap = {
    {"STP", ActSpanningTreeVersionEnum::kSTP},
    {"RSTP", ActSpanningTreeVersionEnum::kRSTP},
    {"NotSupport", ActSpanningTreeVersionEnum::kNotSupport}};

class ActRstpTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, active, Active);

  // std1w1ap
  ACT_JSON_ENUM(ActSpanningTreeVersionEnum, spanningTreeVersion,
                SpanningTreeVersion);  ///< The spanningTreeVersion(2) of the device
  ACT_JSON_FIELD(qint64, priority, Priority);
  ACT_JSON_FIELD(qint64, forward_delay,
                 ForwardDelay);  ///< The forwardDelay(sec) of the device
  ACT_JSON_FIELD(qint64, hello_time,
                 HelloTime);  ///< The hello time(sec) of the device
  ACT_JSON_FIELD(qint64, max_age,
                 MaxAge);  ///< The maxAge(sec) of the device
  // mxrstp
  ACT_JSON_FIELD(qint64, rstp_error_recovery_time,
                 RstpErrorRecoveryTime);  ///< The rstpErrorRecoveryTime(sec) of the device

  // mxrstp > swift
  ACT_JSON_FIELD(bool, rstp_config_swift, RstpConfigSwift);
  ACT_JSON_FIELD(bool, rstp_config_revert, RstpConfigRevert);

  ACT_JSON_QT_SET_OBJECTS(ActRstpPortEntry, rstp_port_entries, RstpPortEntries);

  // ACT_JSON_FIELD(qint64, rstp_tx_hold_count,
  //                RstpTxHoldCount);  ///< The rstpTxHoldCount of the device
 public:
  ActRstpTable() {
    this->active_ = true;

    // std1w1ap
    this->spanningTreeVersion_ = ActSpanningTreeVersionEnum::kRSTP;
    this->priority_ = 32768;
    this->max_age_ = 20;
    this->hello_time_ = 2;
    this->forward_delay_ = 15;
    // this->rstp_tx_hold_count_ = 6;

    // mxrstp
    this->rstp_error_recovery_time_ = 300;

    // swift
    this->rstp_config_swift_ = false;
    this->rstp_config_revert_ = false;
  }
  ActRstpTable(const qint64 &device_id) : ActRstpTable() { SetDeviceId(device_id); }

  ActRstpTable(const ActDevice &device) : ActRstpTable() {
    SetDeviceId(device.GetId());

    // Add the init port_retry by device's interface
    for (ActInterface intf : device.GetInterfaces()) {
      this->rstp_port_entries_.insert(ActRstpPortEntry(intf.GetInterfaceId()));
    }
  }

  ActRstpTable(const qint64 &device_id, const QSet<ActRstpPortEntry> &rstp_port_entries) : ActRstpTable() {
    SetDeviceId(device_id);
    this->rstp_port_entries_ = rstp_port_entries;
  }
};

class ActStadPortTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_QT_SET_OBJECTS(ActInterfaceStadPortEntry, interface_stad_port_entries, InterfaceStadPortEntries);

 public:
  ActStadPortTable() {}
  ActStadPortTable(const qint64 &device_id) { SetDeviceId(device_id); }

  ActStadPortTable(const ActDevice &device) {
    SetDeviceId(device.GetId());

    // Add the init port_retry by device's interface
    for (ActInterface intf : device.GetInterfaces()) {
      this->interface_stad_port_entries_.insert(ActInterfaceStadPortEntry(intf.GetInterfaceId()));
    }
  }

  ActStadPortTable(const qint64 &device_id, const QSet<ActInterfaceStadPortEntry> interface_stad_port_entries) {
    SetDeviceId(device_id);
    interface_stad_port_entries_ = interface_stad_port_entries;
  }

  // Check used V2 config
  bool IsPerStreamPriorityV2() {
    for (auto if_stad_port_entry : interface_stad_port_entries_) {
      for (auto stad_port_entry : if_stad_port_entry.GetStadPortEntries()) {
        if (stad_port_entry.GetType().contains(ActStreamPriorityTypeEnum::kTcp) ||
            stad_port_entry.GetType().contains(ActStreamPriorityTypeEnum::kUdp)) {
          return true;
        }
      }
    }
    return false;
  }
};

class ActStadConfigTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActStadConfigEntry, stad_config_entries, StadConfigEntries);

 public:
  ActStadConfigTable() {}
  ActStadConfigTable(const qint64 &device_id) { SetDeviceId(device_id); }
};

class ActPortVlanTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_QT_SET_OBJECTS(ActPortVlanEntry, port_vlan_entries, PortVlanEntries);

 public:
  ActPortVlanTable() {}
  ActPortVlanTable(const qint64 &device_id) { SetDeviceId(device_id); }
  ActPortVlanTable(const qint64 &device_id, const QSet<ActPortVlanEntry> &port_vlan_entries) {
    SetDeviceId(device_id);
    port_vlan_entries_ = port_vlan_entries;
  }
};

class ActDefaultPriorityTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_QT_SET_OBJECTS(ActDefaultPriorityEntry, default_priority_entries, DefaultPriorityEntries);

 public:
  ActDefaultPriorityTable() {}
  ActDefaultPriorityTable(const qint64 &device_id) { SetDeviceId(device_id); }
  ActDefaultPriorityTable(const qint64 &device_id, const QSet<ActDefaultPriorityEntry> &default_priority_entries) {
    SetDeviceId(device_id);
    default_priority_entries_ = default_priority_entries;
  }
};

class ActVlanPortTypeTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_QT_SET_OBJECTS(ActVlanPortTypeEntry, vlan_port_type_entries, VlanPortTypeEntries);

 public:
  ActVlanPortTypeTable() {}
  ActVlanPortTypeTable(const qint64 &device_id) { SetDeviceId(device_id); }
  ActVlanPortTypeTable(const qint64 &device_id, const QSet<ActVlanPortTypeEntry> &vlan_port_type_entries) {
    SetDeviceId(device_id);
    vlan_port_type_entries_ = vlan_port_type_entries;
  }
};

class ActVlanStaticTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_QT_SET_OBJECTS(ActVlanStaticEntry, vlan_static_entries, VlanStaticEntries);

 public:
  ActVlanStaticTable() {}
  ActVlanStaticTable(const qint64 &device_id) { SetDeviceId(device_id); }
  ActVlanStaticTable(const qint64 &device_id, const QSet<ActVlanStaticEntry> &vlan_static_entries) {
    SetDeviceId(device_id);
    this->vlan_static_entries_ = vlan_static_entries;
  }
};

class ActVlanTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, management_vlan, ManagementVlan);
  ACT_JSON_QT_SET_OBJECTS(ActVlanStaticEntry, vlan_static_entries, VlanStaticEntries);
  ACT_JSON_QT_SET_OBJECTS(ActPortVlanEntry, port_vlan_entries,
                          PortVlanEntries);  ///< SNMP's PortVlanEntries(PVID) set
  ACT_JSON_QT_SET_OBJECTS(ActVlanPortTypeEntry, vlan_port_type_entries, VlanPortTypeEntries);

 public:
  QList<QString> key_order_;

  ActVlanTable() { this->management_vlan_ = ACT_VLAN_INIT_PVID; }
  ActVlanTable(const qint64 &device_id) : ActVlanTable() { SetDeviceId(device_id); }

  ActVlanTable(const ActDevice &device) : ActVlanTable() { SetDeviceId(device.GetId()); }
};

class ActStaticForwardTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_QT_SET_OBJECTS(ActStaticForwardEntry, static_forward_entries, StaticForwardEntries);

 public:
  ActStaticForwardTable() {}
  ActStaticForwardTable(const qint64 &device_id) { SetDeviceId(device_id); }
  ActStaticForwardTable(const qint64 &device_id, const QSet<ActStaticForwardEntry> &static_forward_entries) {
    SetDeviceId(device_id);
    static_forward_entries_ = static_forward_entries;
  }
};

class ActPortSettingTable : public ActDeployTableBase {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_QT_SET_OBJECTS(ActPortSettingEntry, port_setting_entries,
                          PortSettingEntries);  ///< PortSettingEntries set

 public:
  ActPortSettingTable() {}
  ActPortSettingTable(const qint64 &device_id) { SetDeviceId(device_id); }

  ActPortSettingTable(const ActDevice &device) : ActPortSettingTable() {
    SetDeviceId(device.GetId());

    // Add the init port_retry by device's interface
    for (ActInterface intf : device.GetInterfaces()) {
      this->port_setting_entries_.insert(ActPortSettingEntry(intf.GetInterfaceId()));
    }
  }

  ActPortSettingTable(const qint64 &device_id, const QSet<ActPortSettingEntry> port_setting_entries) {
    SetDeviceId(device_id);
    port_setting_entries_ = port_setting_entries;
  }
};

// /**
//  * @brief The Add&Remove VlanStatic tables object
//  *
//  */
// class ActAddRemoveVlanStaticTable : public QSerializer {
//   Q_GADGET
//   QS_SERIALIZABLE

//   ACT_JSON_OBJECT(ActVlanStaticTable, add_vlan_static_table, AddVlanStaticTable);
//   ACT_JSON_OBJECT(ActVlanStaticTable, remove_vlan_static_table, RemoveVlanStaticTable);
// };

/**
 * @brief The Edit VlanStatic tables object
 *
 */
class ActEditVlanStaticTable : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint32, add_vlan_id, AddVlanId);
  ACT_JSON_COLLECTION(QList, qint32, delete_vlan_id, DeleteVlanId);
  ACT_JSON_OBJECT(ActVlanStaticTable, set_vlan_static_table, SetVlanStaticTable);
};

/**
 * @brief The Add&Remove StaticForward tables object
 *
 */
class ActAddRemoveStaticForwardTable : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStaticForwardTable, add_static_forward_table, AddStaticForwardTable);
  ACT_JSON_OBJECT(ActStaticForwardTable, remove_static_forward_table, RemoveStaticForwardTable);
};

/**
 * @brief The Add&Remove StreamPriority Port tables object
 *
 */
class ActAddRemoveStadPortTable : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStadPortTable, add_stad_port_table, AddStadPortTable);
  ACT_JSON_OBJECT(ActStadPortTable, remove_stad_port_table, RemoveStadPortTable);
};

class ActActionMethodValue : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, key, Key);
  ACT_JSON_FIELD(QString, value, Value);

 public:
  ActActionMethodValue() {};
  ActActionMethodValue(QString key, QString value) : key_(key), value_(value) {};
};

class ActActionMethodConfig : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActActionMethodValue, action_method_list, ActionMethodList);

 public:
  ActActionMethodConfig() {};
};