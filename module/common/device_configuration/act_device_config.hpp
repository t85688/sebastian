/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include "act_json.hpp"
#include "deploy_entry/act_deploy_table.hpp"
/**
 * @brief The ACT Device Config class
 *
 */
class ActDeviceConfig : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // TopologyMapping IP setting
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActMappingDeviceIpSettingTable, mapping_device_ip_setting_tables,
                           MappingDeviceIpSettingTables);

  // NetworkSetting
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActNetworkSettingTable, network_setting_tables, NetworkSettingTables);

  // UserAccount
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActUserAccountTable, user_account_tables, UserAccountTables);

  // TimeSync
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActTimeSyncTable, time_sync_tables, TimeSyncTables);

  // LoginPolicy
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActLoginPolicyTable, login_policy_tables, LoginPolicyTables);

  // LoopProtection
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActLoopProtectionTable, loop_protection_tables, LoopProtectionTables);

  // SNMP Trap server setting
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActSnmpTrapSettingTable, snmp_trap_setting_tables, SnmpTrapSettingTables);

  // Syslog server setting
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActSyslogSettingTable, syslog_setting_tables, SyslogSettingTables);

  // Time setting
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActTimeSettingTable, time_setting_tables, TimeSettingTables);

  // InformationSetting
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActInformationSettingTable, information_setting_tables,
                           InformationSettingTables);

  // Management Interface
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActManagementInterfaceTable, management_interface_tables,
                           ManagementInterfaceTables);

  // PortSetting
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActPortSettingTable, port_setting_tables, PortSettingTables);

  // VLAN (VlanStatic & PortType & PVID)
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActVlanTable, vlan_tables, VlanTables);

  // StaticForward(Unicast & Multicast)
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActStaticForwardTable, unicast_static_forward_tables,
                           UnicastStaticForwardTables);

  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActStaticForwardTable, multicast_static_forward_tables,
                           MulticastStaticForwardTables);

  // Port Default PCP
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActDefaultPriorityTable, port_default_pcp_tables, PortDefaultPCPTables);

  // StreamPriority(Ingress & Egress)
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActStadPortTable, stream_priority_ingress_tables, StreamPriorityIngressTables);

  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActStadConfigTable, stream_priority_egress_tables, StreamPriorityEgressTables);
  // GCL
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActGclTable, gcl_tables, GCLTables);

  // CB
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActCbTable, cb_tables, CbTables);

  // RSTP
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActRstpTable, rstp_tables, RstpTables);

 public:
  ActDeviceConfig() {}

  ACT_STATUS HidePassword() {
    ACT_STATUS_INIT();
    for (auto key : this->user_account_tables_.keys()) {
      this->user_account_tables_[key].HidePassword();
    }
    return act_status;
  }

  ACT_STATUS EncryptPassword() {
    ACT_STATUS_INIT();
    for (auto key : this->user_account_tables_.keys()) {
      this->user_account_tables_[key].EncryptPassword();
    }
    return act_status;
  }

  ACT_STATUS DecryptPassword() {
    ACT_STATUS_INIT();
    for (auto key : this->user_account_tables_.keys()) {
      this->user_account_tables_[key].DecryptPassword();
    }
    return act_status;
  }
};
