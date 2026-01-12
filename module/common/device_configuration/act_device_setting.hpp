/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include "act_deploy_table.hpp"
#include "act_device.hpp"
#include "act_json.hpp"

class ActDeviceBasic : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);
  ACT_JSON_FIELD(QString, device_alias, DeviceAlias);
  ACT_JSON_FIELD(bool, support, Support);

 public:
  /**
   * @brief Construct a new device information object
   *
   */
  ActDeviceBasic() {
    device_id_ = -1;
    support_ = false;
  }

  ActDeviceBasic(const ActDevice &device, const bool &support) {
    device_id_ = device.GetId();
    device_ip_ = device.GetIpv4().GetIpAddress();
    device_alias_ = device.GetDeviceAlias();
    support_ = support;
  }
};

class ActDeviceInformation : public ActDeviceBasic {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, device_name, DeviceName);
  ACT_JSON_FIELD(QString, location, Location);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_FIELD(QString, contact_information, ContactInformation);

 public:
  /**
   * @brief Construct a new device information object
   *
   */
  ActDeviceInformation() {}
  ActDeviceInformation(ActDevice &device)
      : ActDeviceBasic(device,
                       device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetInformationSetting()) {}
};

class ActDeviceInformationList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceInformation, device_information_list, DeviceInformationList);
};

class ActDeviceLoginPolicy : public ActDeviceBasic {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, login_message, LoginMessage);
  ACT_JSON_FIELD(QString, login_authentication_failure_message, LoginAuthenticationFailureMessage);
  ACT_JSON_FIELD(bool, account_login_failure_lockout, AccountLoginFailureLockout);
  ACT_JSON_FIELD(qint32, retry_failure_threshold, RetryFailureThreshold);
  ACT_JSON_FIELD(qint32, lockout_duration, LockoutDuration);
  ACT_JSON_FIELD(qint32, auto_logout_after, AutoLogoutAfter);

 public:
  /**
   * @brief Construct a new device login policy object
   *
   */
  ActDeviceLoginPolicy() {
    account_login_failure_lockout_ = false;
    retry_failure_threshold_ = 5;
    lockout_duration_ = 5;
    auto_logout_after_ = 5;
  }

  ActDeviceLoginPolicy(const ActDevice &device)
      : ActDeviceBasic(device, device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetLoginPolicy()) {
    account_login_failure_lockout_ = false;
    retry_failure_threshold_ = 5;
    lockout_duration_ = 5;
    auto_logout_after_ = 5;
  }
};

class ActDeviceLoginPolicyList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceLoginPolicy, device_login_policy_list, DeviceLoginPolicyList);
};

class ActDeviceLoopProtection : public ActDeviceBasic {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, network_loop_protection, NetworkLoopProtection);
  ACT_JSON_FIELD(qint32, detect_interval, DetectInterval);

 public:
  /**
   * @brief Construct a new device loop protection object
   *
   */
  ActDeviceLoopProtection() {}

  ActDeviceLoopProtection(const ActDevice &device)
      : ActDeviceBasic(device, device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetLoopProtection()) {}
};

class ActDeviceLoopProtectionList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceLoopProtection, device_loop_protection_list, DeviceLoopProtectionList);
};

class ActDeviceSyslogSetting : public ActDeviceBasic {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enabled, Enabled);
  ACT_JSON_FIELD(bool, syslog_server_1, SyslogServer1);
  ACT_JSON_FIELD(QString, address_1, Address1);
  ACT_JSON_FIELD(qint32, udp_port_1, UDPPort1);
  ACT_JSON_FIELD(bool, syslog_server_2, SyslogServer2);
  ACT_JSON_FIELD(QString, address_2, Address2);
  ACT_JSON_FIELD(qint32, udp_port_2, UDPPort2);
  ACT_JSON_FIELD(bool, syslog_server_3, SyslogServer3);
  ACT_JSON_FIELD(QString, address_3, Address3);
  ACT_JSON_FIELD(qint32, udp_port_3, UDPPort3);

 public:
  /**
   * @brief Construct a new device syslog server object
   *
   */
  ActDeviceSyslogSetting() {
    enabled_ = false;
    syslog_server_1_ = false;
    udp_port_1_ = 514;
    syslog_server_2_ = false;
    udp_port_2_ = 514;
    syslog_server_3_ = false;
    udp_port_3_ = 514;
  }

  ActDeviceSyslogSetting(const ActDevice &device)
      : ActDeviceBasic(device, device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSyslogSetting()) {
    enabled_ = false;
    syslog_server_1_ = false;
    udp_port_1_ = 514;
    syslog_server_2_ = false;
    udp_port_2_ = 514;
    syslog_server_3_ = false;
    udp_port_3_ = 514;
  }
};

class ActDeviceSyslogSettingList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceSyslogSetting, device_syslog_setting_list, DeviceSyslogSettingList);
};

class ActDeviceSnmpTrapSetting : public ActDeviceBasic {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActSnmpTrapHostEntry, host_list, HostList);

 public:
  /**
   * @brief Construct a new device snmp trap setting object
   *
   */
  ActDeviceSnmpTrapSetting() {}
  ActDeviceSnmpTrapSetting(const ActDevice &device)
      : ActDeviceBasic(device, device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSNMPTrapSetting()) {}
};

class ActDeviceSnmpTrapSettingList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceSnmpTrapSetting, device_snmp_trap_setting_list,
                              DeviceSnmpTrapSettingList);
};

class ActDeviceBackupSetting : public ActDeviceBasic {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(QString, firmware_version, FirmwareVersion);
  ACT_JSON_FIELD(QString, mac_address, MacAddress);
  ACT_JSON_FIELD(QString, serial_number, SerialNumber);
  ACT_JSON_FIELD(QString, location, Location);

 public:
  /**
   * @brief Construct a new device backup setting object
   *
   */
  ActDeviceBackupSetting() {}
  ActDeviceBackupSetting(const ActDevice &device)
      : ActDeviceBasic(device, device.GetDeviceProperty().GetFeatureGroup().GetOperation().GetImportExport()) {}
};

class ActDeviceBackupSettingList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceBackupSetting, device_backup_setting_list, DeviceBackupSettingList);
};

class ActDeviceBackupFile : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, file_name, FileName);
  ACT_JSON_FIELD(QString, backup_file, BackupFile);
};

class ActDevicePort : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, port_id, PortId);
  ACT_JSON_FIELD(QString, port_name, PortName);
  ACT_JSON_FIELD(bool, active, Active);
  ACT_JSON_ENUM(ActVlanPortTypeEnum, port_type, PortType);
  ACT_JSON_FIELD(qint32, pvid, Pvid);
  ACT_JSON_FIELD(quint8, default_pcp, DefaultPCP);
  ACT_JSON_QT_SET(qint32, untagged_vlan, UntaggedVlan);
  ACT_JSON_QT_SET(qint32, tagged_vlan, TaggedVlan);

 public:
  ActDevicePort() {
    this->port_id_ = 0;
    this->active_ = false;
    this->port_type_ = ActVlanPortTypeEnum::kAccess;
    this->pvid_ = 1;
    this->default_pcp_ = ACT_INIT_DEFAULT_PCP;
  }
  ActDevicePort(const qint64 &port_id) : ActDevicePort() { this->port_id_ = port_id; }
  ActDevicePort(const bool &active, const qint64 &port_id, const QString &port_name) : ActDevicePort() {
    this->active_ = active;
    this->port_id_ = port_id;
    this->port_name_ = port_name;
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActDevicePort &x, const ActDevicePort &y) { return x.port_id_ == y.port_id_; }
};

class ActDeviceVlan : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, vlan_id, VlanId);
  ACT_JSON_FIELD(QString, vlan_name, VlanName);
  ACT_JSON_FIELD(bool, te_mstid, TeMstid);
  ACT_JSON_QT_SET(qint64, member_port, MemberPort);

 public:
  ActDeviceVlan() {
    vlan_id_ = 0;
    te_mstid_ = false;
  }

  ActDeviceVlan(qint32 vlan_id) : ActDeviceVlan() { this->vlan_id_ = vlan_id; }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActDeviceVlan &x, const ActDeviceVlan &y) { return x.vlan_id_ == y.vlan_id_; }
};

class ActDeviceVlanSetting : public ActDeviceBasic {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, management_vlan, ManagementVlan);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceVlan, vlan_list, VlanList);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActDevicePort, port_list, PortList);
  ACT_JSON_QT_SET(qint32, reserved_vlan, ReservedVlan);

 public:
  ActDeviceVlanSetting() { management_vlan_ = 1; }
  ActDeviceVlanSetting(ActDevice &device)
      : ActDeviceBasic(
            device,
            device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().CheckSupportAnyOne()) {
    management_vlan_ = 1;
  }
};

class ActDeviceVlanSettingList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceVlanSetting, device_vlan_setting_list, DeviceVlanSettingList);
};

class ActDeviceTimeSetting : public ActDeviceBasic {
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
  ActDeviceTimeSetting() {
    this->clock_source_ = ActClockSourceEnum::kLocal;
    this->time_zone_ = ActTimeZoneEnum::kGMT;
    this->daylight_saving_time_ = false;
    this->offset_ = "00:00";
    this->ntp_time_server_1_ = "time.nist.gov";
    this->sntp_time_server_1_ = "time.nist.gov";
  }

  ActDeviceTimeSetting(ActDevice &device)
      : ActDeviceBasic(
            device, device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSetting().GetSystemTime()) {
    ActDeviceTimeSetting();
  }
};

class ActDeviceTimeSettingList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceTimeSetting, device_time_setting_list, DeviceTimeSettingList);
};

class ActDevicePortStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, port_id, PortId);
  ACT_JSON_FIELD(QString, port_name, PortName);
  ACT_JSON_FIELD(bool, active, Active);
  ACT_JSON_FIELD(bool, admin_status, AdminStatus);

 public:
  ActDevicePortStatus() {
    this->port_id_ = -1;
    this->active_ = false;
    this->admin_status_ = true;
  };
  ActDevicePortStatus(const qint32 port_id) : ActDevicePortStatus() { this->port_id_ = port_id; }
  ActDevicePortStatus(const bool &active, const qint32 &port_id, const QString &port_name) : ActDevicePortStatus() {
    this->active_ = active;
    this->port_id_ = port_id;
    this->port_name_ = port_name;
  }
  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActDevicePortStatus &x, const ActDevicePortStatus &y) {
    return x.port_id_ == y.port_id_;
  }
};

class ActDevicePortSetting : public ActDeviceBasic {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDevicePortStatus, port_list, PortList);

 public:
  ActDevicePortSetting() {}

  ActDevicePortSetting(ActDevice &device)
      : ActDeviceBasic(
            device, device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetPortSetting().GetAdminStatus()) {
  }
};

class ActDevicePortSettingList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDevicePortSetting, device_port_setting_list, DevicePortSettingList);
};

class ActDeployDevice : public ActDeviceBasic {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(QString, firmware_version, FirmwareVersion);
  ACT_JSON_FIELD(QString, mac_address, MacAddress);
  ACT_JSON_FIELD(QString, serial_number, SerialNumber);
  ACT_JSON_FIELD(QString, location, Location);

 public:
  quint32 address_number_;

  ActDeployDevice() {}
  ActDeployDevice(ActDevice &device) : ActDeviceBasic(device, device.CheckCanDeploy()) {}

  friend bool operator<(const ActDeployDevice &x, const ActDeployDevice &y) {
    return x.address_number_ < y.address_number_;
  }
};

class ActDeployDeviceList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeployDevice, device_list, DeviceList);
};

class ActDeviceIpSetting : public ActDeviceBasic {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, subnet_mask, SubnetMask);
  ACT_JSON_FIELD(QString, gateway, Gateway);
  ACT_JSON_FIELD(QString, dns1, DNS1);
  ACT_JSON_FIELD(QString, dns2, DNS2);

 public:
  ActDeviceIpSetting() {}

  ActDeviceIpSetting(ActDevice &device)
      : ActDeviceBasic(device, device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetNetworkSetting()) {}
};

class ActDeviceIpSettingList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceIpSetting, device_ip_setting_list, DeviceIpSettingList);
};

class ActDeviceRstpPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, port_id, PortId);
  ACT_JSON_FIELD(QString, port_name, PortName);
  ACT_JSON_FIELD(bool, active, Active);

  // std1w1ap
  ACT_JSON_FIELD(bool, rstp_enable, RstpEnable);
  // Edge:
  ACT_JSON_ENUM(ActRstpEdgeEnum, edge, Edge);

  ACT_JSON_FIELD(qint64, port_priority, PortPriority);
  ACT_JSON_FIELD(qint64, path_cost, PathCost);

  // std1d1ap
  ACT_JSON_FIELD(bool, link_type_support, LinkTypeSupport);
  ACT_JSON_ENUM(ActRstpLinkTypeEnum, link_type, LinkType);

  // mxrstp
  ACT_JSON_FIELD(bool, bpdu_guard_support, BpduGuardSupport);
  ACT_JSON_FIELD(bool, root_guard_support, RootGuardSupport);
  ACT_JSON_FIELD(bool, loop_guard_support, LoopGuardSupport);
  ACT_JSON_FIELD(bool, bpdu_filter_support, BpduFilterSupport);

  ACT_JSON_FIELD(bool, bpdu_guard, BpduGuard);
  ACT_JSON_FIELD(bool, root_guard, RootGuard);
  ACT_JSON_FIELD(bool, loop_guard, LoopGuard);
  ACT_JSON_FIELD(bool, bpdu_filter, BpduFilter);

 public:
  /**
   * @brief Construct a new RSTP Port Entry object
   *
   */
  ActDeviceRstpPortEntry() {
    this->port_id_ = -1;

    // std1w1ap
    this->port_priority_ = 128;
    this->rstp_enable_ = true;
    this->path_cost_ = 0;
    // this->force_edge_ = false;

    this->edge_ = ActRstpEdgeEnum::kAuto;

    // std1d1ap
    this->link_type_support_ = false;
    this->link_type_ = ActRstpLinkTypeEnum::kAuto;

    // mxrstp
    this->bpdu_guard_support_ = false;
    this->root_guard_support_ = false;
    this->loop_guard_support_ = false;
    this->bpdu_filter_support_ = false;
    this->bpdu_guard_ = false;
    this->root_guard_ = false;
    this->loop_guard_ = false;
    this->bpdu_filter_ = false;
  }

  /**
   * @brief Construct a new RSTP Port Entry object
   *
   * @param port_id
   */
  ActDeviceRstpPortEntry(const qint32 &port_id) : ActDeviceRstpPortEntry() { this->port_id_ = port_id; }

  /**
   * @brief Construct a new RSTP Port Entry object
   *
   * @param port_id
   */
  ActDeviceRstpPortEntry(const qint32 &port_id, const ActDevice &device) : ActDeviceRstpPortEntry() {
    this->port_id_ = port_id;
    this->link_type_support_ =
        device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetLinkType();
    this->bpdu_guard_support_ =
        device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetBPDUGuard();
    this->root_guard_support_ =
        device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetRootGuard();
    this->loop_guard_support_ =
        device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetLoopGuard();
    this->bpdu_filter_support_ =
        device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetBPDUFilter();
    for (ActInterface intf : device.GetInterfaces()) {
      if (intf.GetInterfaceId() == port_id) {
        this->port_name_ = intf.GetInterfaceName();
        this->active_ = intf.GetActive();
        break;
      }
    }
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActDeviceRstpPortEntry &x) {
    return qHash(x.port_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActDeviceRstpPortEntry &x, const ActDeviceRstpPortEntry &y) {
    return x.port_id_ == y.port_id_;
  }
};

class ActDeviceRstpSetting : public ActDeviceBasic {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, stp_rstp, StpRstp);

  // std1w1ap
  ACT_JSON_ENUM(ActSpanningTreeVersionEnum, compatibility, Compatibility);
  ACT_JSON_FIELD(qint64, priority, Priority);
  ACT_JSON_FIELD(qint64, forward_delay, ForwardDelay);
  ACT_JSON_FIELD(qint64, hello_time, HelloTime);
  ACT_JSON_FIELD(qint64, max_age, MaxAge);
  // mxrstp
  ACT_JSON_FIELD(bool, rstp_error_recovery_time_support, RstpErrorRecoveryTimeSupport);
  ACT_JSON_FIELD(qint64, rstp_error_recovery_time,
                 RstpErrorRecoveryTime);  ///< The rstpErrorRecoveryTime(sec) of the device

  // mxrstp > swift
  ACT_JSON_FIELD(bool, rstp_config_swift_support, RstpConfigSwiftSupport);
  ACT_JSON_FIELD(bool, rstp_config_swift, RstpConfigSwift);
  ACT_JSON_FIELD(bool, rstp_config_revert, RstpConfigRevert);

  ACT_JSON_FIELD(bool, rstp_port_entry_support, RstpPortEntrySupport);
  ACT_JSON_QT_SET_OBJECTS(ActDeviceRstpPortEntry, rstp_port_entries, RstpPortEntries);

  // ACT_JSON_FIELD(qint64, rstp_tx_hold_count,
  //                RstpTxHoldCount);  ///< The rstpTxHoldCount of the device
 public:
  ActDeviceRstpSetting() {
    this->stp_rstp_ = true;

    // std1w1ap
    this->compatibility_ = ActSpanningTreeVersionEnum::kRSTP;
    this->priority_ = 32768;
    this->max_age_ = 20;
    this->hello_time_ = 2;
    this->forward_delay_ = 15;

    // mxrstp
    this->rstp_error_recovery_time_support_ = false;
    this->rstp_error_recovery_time_ = 300;

    // swift
    this->rstp_config_swift_support_ = false;
    this->rstp_config_swift_ = false;
    this->rstp_config_revert_ = false;

    // port entry
    this->rstp_port_entry_support_ = false;
  }
  ActDeviceRstpSetting(ActDevice &device)
      : ActDeviceBasic(device, device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetRSTP()) {
    this->rstp_error_recovery_time_support_ =
        device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetErrorRecoveryTime();
    this->rstp_config_swift_support_ =
        device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetSwift();
    this->rstp_port_entry_support_ =
        device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetPortRSTPEnable();
  }
};

class ActDeviceRstpSettingList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceRstpSetting, device_rstp_setting_list, DeviceRstpSettingList);
};

// per-stream priority

class ActDevicePerStreamPriorityPortListEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, port_id, PortId);
  ACT_JSON_FIELD(QString, port_name, PortName);
  ACT_JSON_FIELD(bool, active, Active);
  ACT_JSON_COLLECTION(QList, qint32, available_vlans, AvailableVlans);
};
class ActDevicePerStreamPriorityEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, port_id, PortId);
  ACT_JSON_FIELD(QString, port_name, PortName);

  ACT_JSON_ENUM(ActStreamPriorityTypeEnum, type, Type);  ///< The Type

  // L2: ethertype, subtype
  ACT_JSON_FIELD(qint32, ether_type, EtherType);
  ACT_JSON_FIELD(bool, enable_sub_type, EnableSubType);
  ACT_JSON_FIELD(qint32, sub_type, SubType);

  // L3: tcpPort, udpPort
  ACT_JSON_FIELD(qint32, udp_port, UdpPort);  ///< The UdpPort
  ACT_JSON_FIELD(qint32, tcp_port, TcpPort);  ///< The TcpPort

  ACT_JSON_FIELD(qint32, vlan_id, VlanId);
  ACT_JSON_FIELD(qint32, priority_code_point, PriorityCodePoint);

 public:
  QList<QString> key_order_;

  ActDevicePerStreamPriorityEntry() {
    this->key_order_.append(QList<QString>(
        {QString("PortId"), QString("PortName"), QString("Type"), QString("EtherType"), QString("EnableSubType"),
         QString("SubType"), QString("UdpPort"), QString("TcpPort"), QString("VlanId"), QString("PriorityCodePoint")}));

    this->port_id_ = 0;
    this->port_name_ = "0";

    this->type_ = {ActStreamPriorityTypeEnum::kEthertype};

    this->ether_type_ = 0;
    this->enable_sub_type_ = false;
    this->sub_type_ = 0;

    this->udp_port_ = 1;
    this->tcp_port_ = 1;

    this->vlan_id_ = 0;
    this->priority_code_point_ = 0;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActDevicePerStreamPriorityEntry &obj) {
    uint seed = 0;
    uint h = qHash(obj.port_id_, seed);
    h = qHash(obj.type_, h);

    switch (obj.type_) {
      case ActStreamPriorityTypeEnum::kEthertype:
        h = qHash(obj.ether_type_, h);
        h = qHash(obj.enable_sub_type_, h);
        h = qHash(obj.sub_type_, h);
        break;
      case ActStreamPriorityTypeEnum::kTcp:
        h = qHash(obj.tcp_port_, h);
        break;
      case ActStreamPriorityTypeEnum::kUdp:
        h = qHash(obj.udp_port_, h);
        break;
      case ActStreamPriorityTypeEnum::kInactive:
        break;
      default:
        break;
    }
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   */
  friend bool operator==(const ActDevicePerStreamPriorityEntry &x, const ActDevicePerStreamPriorityEntry &y) {
    // check port id, type
    if (x.port_id_ != y.port_id_ || x.type_ != y.type_) {
      return false;
    }

    // check type setting according type
    switch (x.type_) {
      case ActStreamPriorityTypeEnum::kEthertype:
        return x.ether_type_ == y.ether_type_ && x.enable_sub_type_ == y.enable_sub_type_ && x.sub_type_ == y.sub_type_;
      case ActStreamPriorityTypeEnum::kTcp:
        return x.tcp_port_ == y.tcp_port_;
      case ActStreamPriorityTypeEnum::kUdp:
        return x.udp_port_ == y.udp_port_;
      case ActStreamPriorityTypeEnum::kInactive:
        return true;
      default:
        return false;
    }
  }
};

class ActDevicePerStreamPrioritySetting : public ActDeviceBasic {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, udp_tcp_support, UdpTcpSupport);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActDevicePerStreamPriorityPortListEntry, port_list, PortList);

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDevicePerStreamPriorityEntry, per_stream_priority_setting,
                              PerStreamPrioritySetting);

 public:
  ActDevicePerStreamPrioritySetting() { this->udp_tcp_support_ = false; }
  ActDevicePerStreamPrioritySetting(ActDevice &device)
      : ActDeviceBasic(
            device,
            (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetPerStreamPriority()) ||
                (device.GetDeviceProperty()
                     .GetFeatureGroup()
                     .GetConfiguration()
                     .GetVLANSetting()
                     .GetPerStreamPriorityV2())) {
    QList<ActDevicePerStreamPriorityPortListEntry> port_list;

    for (ActInterface intf : device.GetInterfaces()) {
      ActDevicePerStreamPriorityPortListEntry entry;
      entry.SetPortId(intf.GetInterfaceId());
      entry.SetPortName(intf.GetInterfaceName());
      entry.SetActive(intf.GetActive());

      port_list.append(entry);
    }

    this->port_list_ = port_list;

    this->udp_tcp_support_ =
        device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetPerStreamPriorityV2();
  }
};

/**
 * @brief The simple per-stream priority list class
 *
 */
class ActDevicePerStreamPrioritySettingList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDevicePerStreamPrioritySetting, device_per_stream_priority_setting_list,
                              DevicePerStreamPrioritySettingList);

 public:
  ActDevicePerStreamPrioritySettingList() {}
};

// GCL//

/**
 * @brief The simple time slot setting class
 *
 */
class ActDeviceTimeSlot : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, slot_id, SlotId);
  ACT_JSON_FIELD(qreal, interval, Interval);
  ACT_JSON_COLLECTION(QList, quint8, queue_set, QueueSet);

 public:
  QList<QString> key_order_;

  ActDeviceTimeSlot() {
    this->key_order_.append(QList<QString>({QString("SlotId"), QString("Interval"), QString("QueueSet")}));

    this->slot_id_ = -1;
    this->interval_ = 1000000;
  }

  ActDeviceTimeSlot(qint64 id) : ActDeviceTimeSlot() { this->slot_id_ = id; }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActDeviceTimeSlot &obj) {
    return qHash(obj.slot_id_, 0);  // arbitrary value is 0
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   */
  friend bool operator==(const ActDeviceTimeSlot &x, const ActDeviceTimeSlot &y) { return x.slot_id_ == y.slot_id_; }
};

/**
 * @brief The simple time slot setting list class
 *
 */
class ActDeviceTimeSlotPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, port_id, PortId);
  ACT_JSON_FIELD(QString, port_name, PortName);

  ACT_JSON_FIELD(bool, active, Active);
  ACT_JSON_FIELD(qreal, cycle_time, CycleTime);

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceTimeSlot, gate_control_list, GateControlList);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new time slot port entry
   */
  ActDeviceTimeSlotPortEntry() {
    this->key_order_.append(
        QList<QString>({QString("PortId"), QString("PortName"), QString("CycleTime"), QString("GateControlList")}));

    this->port_id_ = 0;
    this->port_name_ = "0";
    this->active_ = false;
    this->cycle_time_ = 0;
  }

  /**
   * @brief Construct a new time slot port entry
   * @param id
   */
  ActDeviceTimeSlotPortEntry(qint64 port_id) : ActDeviceTimeSlotPortEntry() { this->port_id_ = port_id; }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActDeviceTimeSlotPortEntry &obj) {
    return qHash(obj.port_id_, 0);  // arbitrary value is 0
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   */
  friend bool operator==(const ActDeviceTimeSlotPortEntry &x, const ActDeviceTimeSlotPortEntry &y) {
    return x.port_id_ == y.port_id_;
  }
};

class ActDeviceTimeSlotSetting : public ActDeviceBasic {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceTimeSlotPortEntry, port_list, PortList);

 public:
  ActDeviceTimeSlotSetting() {}
  ActDeviceTimeSlotSetting(ActDevice &device)
      : ActDeviceBasic(device,
                       device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTSN().GetIEEE802Dot1Qbv()) {}
};

class ActDeviceTimeSlotSettingList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceTimeSlotSetting, time_slot_setting_list, DeviceTimeSlotSettingList);
};