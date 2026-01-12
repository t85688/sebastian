/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#include "act_device_event_log.hpp"
#include "act_feature_profile.hpp"
#include "act_monitor_data.hpp"
#include "act_status.hpp"
#include "agents/act_moxa_iei_client_agent.hpp"
#include "client/act_moxa_iei_client.hpp"
#include "deploy_entry/act_deploy_table.hpp"
#include "topology/act_device.hpp"

#ifndef ACT_RESTFUL_CLIENT_HANDLER_H
#define ACT_RESTFUL_CLIENT_HANDLER_H
#define ACT_SERVICE_UNAVAILABLE_SLEEP_TIME (2000)

// #define ACT_RESTFUL_CLIENT_HTTP_PORT 80  /// < The restful client

/**
 * @brief Redundant Protocol Mapping map
 *
 */
static const QMap<QString, QString> RedundantProtocolMappingMap = {
    {"dualhoming", "Dual Homing"}, {"turboringv2", "Turbo Ring V2"}, {"turbochain", "Turbo Chain"},
    {"iec62439_2", "MRP"},         {"stprstp", "STP/RSTP"},          {"mstp", "MSTP"},
};

class ActRestfulClientHandler {
 private:
  QString token_;

  /**
   * @brief Login device and update the token to core_map object
   *
   * @param client_agent
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS LoginAndUpdateCoreToken(ActMoxaIEIClientAgent &client_agent, const ActDevice &device);

  /**
   * @brief Convert ClientTimeDate to ActTimeDay
   *
   * @param interval_date
   * @param result_time_date
   */
  void ConvertClientTimeDateToActTimeDay(const ActClientTimeDate &client_time_date, ActTimeDay &act_time_day);

 public:
  /**
   * @brief Construct a new Act Restful Client Handler object
   *
   */
  ActRestfulClientHandler();

  /**
   * @brief Destroy the Act Restful Client Handler object
   *
   */
  ~ActRestfulClientHandler();

  /**
   * @brief Get the Device Configuration Sync Status object
   *
   * @param device
   * @param check_result
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceConfigurationSyncStatus(const ActDevice &device, bool &check_result);

  /**
   * @brief Check MoxaIEI device connect
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @return ACT_STATUS
   */
  ACT_STATUS CheckConnect(const ActDevice &device, const QString &action_key,
                          const ActFeatureMethodProtocol &protocol_elem);

  /**
   * @brief Set the MoxaIEI device SnmpService object
   *
   * @param device
   * @param items
   * @param snmp_service_config
   * @return ACT_STATUS
   */
  ACT_STATUS SetSnmpService(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem,
                            const ActClientSnmpService &snmp_service_config);

  /**
   * @brief Set the MoxaIEI device Layer2Redundancy object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param active
   * @return ACT_STATUS
   */
  ACT_STATUS SetLayer2Redundancy(const ActDevice &device, const QString &action_key,
                                 const ActFeatureMethodProtocol &protocol_elem, const bool &active);

  /**
   * @brief Set the MoxaIEI device SpanningTree object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetSpanningTree(const ActDevice &device, const QString &action_key,
                             const ActFeatureMethodProtocol &protocol_elem, const ActRstpTable &rstp_table);

  /**
   * @brief Set the Stream Adapter Ingress object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param stad_port_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetStreamAdapterIngress(const ActDevice &device, const QString &action_key,
                                     const ActFeatureMethodProtocol &protocol_elem,
                                     const ActStadPortTable &stad_port_table);

  /**
   * @brief Set the Stream Adapter Egress object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param stad_config_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetStreamAdapterEgress(const ActDevice &device, const QString &action_key,
                                    const ActFeatureMethodProtocol &protocol_elem,
                                    const ActStadConfigTable &stad_config_table);

  /**
   * @brief Get the Stream Adapter Ingress object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param stad_port_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetStreamAdapterIngress(const ActDevice &device, const QString &action_key,
                                     const ActFeatureMethodProtocol &protocol_elem, ActStadPortTable &stad_port_table);

  /**
   * @brief Get the Stream Adapter Egress object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param stad_config_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetStreamAdapterEgress(const ActDevice &device, const QString &action_key,
                                    const ActFeatureMethodProtocol &protocol_elem,
                                    ActStadConfigTable &stad_config_table);

  /**
   * @brief Get the Time Sync Base Config object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param time_sync_base_config
   * @return ACT_STATUS
   */
  ACT_STATUS GetTimeSyncBaseConfig(const ActDevice &device, const QString &action_key,
                                   const ActFeatureMethodProtocol &protocol_elem,
                                   ActTimeSyncBaseConfig &time_sync_base_config);

  /**
   * @brief Get the Time Sync 802Dot1AS Config object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param time_sync_config
   * @return ACT_STATUS
   */
  ACT_STATUS GetTimeSync802Dot1ASConfig(const ActDevice &device, const QString &action_key,
                                        const ActFeatureMethodProtocol &protocol_elem,
                                        ActTimeSync802Dot1ASConfig &time_sync_config);
  /**
   * @brief Get the Time Sync 1588 Config object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param time_sync_config
   * @return ACT_STATUS
   */
  ACT_STATUS GetTimeSync1588Config(const ActDevice &device, const QString &action_key,
                                   const ActFeatureMethodProtocol &protocol_elem,
                                   ActTimeSync1588Config &time_sync_config);

  /**
   * @brief Get the Time Sync Iec 61850 Config object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param time_sync_config
   * @return ACT_STATUS
   */
  ACT_STATUS GetTimeSyncIec61850Config(const ActDevice &device, const QString &action_key,
                                       const ActFeatureMethodProtocol &protocol_elem,
                                       ActTimeSyncIec61850Config &time_sync_config);
  /**
   * @brief Get the Time Sync C37Dot238 Config object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param time_sync_config
   * @return ACT_STATUS
   */
  ACT_STATUS GetTimeSyncC37Dot238Config(const ActDevice &device, const QString &action_key,
                                        const ActFeatureMethodProtocol &protocol_elem,
                                        ActTimeSyncC37Dot238Config &time_sync_config);

  /**
   *
   * @brief Reboot MoxaIEI device
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @return ACT_STATUS
   */
  ACT_STATUS Reboot(const ActDevice &device, const QString &action_key, const ActFeatureMethodProtocol &protocol_elem);

  /**
   * @brief FactoryDefault MoxaIEI device
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @return ACT_STATUS
   */
  ACT_STATUS FactoryDefault(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem);

  /**
   * @brief Locator MoxaIEI device
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param duration
   * @return ACT_STATUS
   */
  ACT_STATUS Locator(const ActDevice &device, const QString &action_key, const ActFeatureMethodProtocol &protocol_elem,
                     const quint16 &duration);

  /**
   * @brief Get the Event log object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_event_logs
   * @return ACT_STATUS
   */
  ACT_STATUS GetEventLog(const ActDevice &device, const QString &action_key,
                         const ActFeatureMethodProtocol &protocol_elem, ActDeviceEventLog &result_event_log);

  /**
   * @brief Get the String Request Use Token Moxa object
   *
   * @param device
   * @param protocol_elem
   * @param result_action_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetStringRequest(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                              QMap<QString, QString> &result_action_map);

  /**
   * @brief Get the Serial Number object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_serial_number
   * @return ACT_STATUS
   */
  ACT_STATUS GetSerialNumber(const ActDevice &device, const QString &action_key,
                             const ActFeatureMethodProtocol &protocol_elem, QString &result_serial_number);

  /**
   * @brief Get the System Uptime object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_uptime
   * @return ACT_STATUS
   */
  ACT_STATUS GetSystemUptime(const ActDevice &device, const QString &action_key,
                             const ActFeatureMethodProtocol &protocol_elem, QString &result_uptime);

  /**
   * @brief Get the Product Revision object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_product_revision
   * @return ACT_STATUS
   */
  ACT_STATUS GetProductRevision(const ActDevice &device, const QString &action_key,
                                const ActFeatureMethodProtocol &protocol_elem, QString &result_product_revision);

  /**
   * @brief Get the Redundant Protocol object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_redundant_protocol
   * @return ACT_STATUS
   */
  ACT_STATUS GetRedundantProtocol(const ActDevice &device, const QString &action_key,
                                  const ActFeatureMethodProtocol &protocol_elem, QString &result_redundant_protocol);

  /**
   * @brief Get the Device Name object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_device_name
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceName(const ActDevice &device, const QString &action_key,
                           const ActFeatureMethodProtocol &protocol_elem, QString &result_device_name);
  /**
   * @brief Get the Device Location object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_device_location
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceLocation(const ActDevice &device, const QString &action_key,
                               const ActFeatureMethodProtocol &protocol_elem, QString &result_device_location);
  /**
   * @brief Get the Device Description object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_device_description
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceDescription(const ActDevice &device, const QString &action_key,
                                  const ActFeatureMethodProtocol &protocol_elem, QString &result_device_description);
  /**
   * @brief Get the Contact Information object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_contact_info
   * @return ACT_STATUS
   */
  ACT_STATUS GetContactInformation(const ActDevice &device, const QString &action_key,
                                   const ActFeatureMethodProtocol &protocol_elem, QString &result_contact_info);

  /**
   * @brief Get the IP Configuration object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_ipv4
   * @return ACT_STATUS
   */
  ACT_STATUS GetIPConfiguration(const ActDevice &device, const QString &action_key,
                                const ActFeatureMethodProtocol &protocol_elem, ActIpv4 &result_ipv4);

  /**
   * @brief Get the L3 IP Configuration object
   *
   * @param device
   * @param action_keys
   * @param protocol_elem
   * @param result_ipv4
   * @return ACT_STATUS
   */
  ACT_STATUS GetL3IPConfiguration(const ActDevice &device, const QSet<QString> &action_keys,
                                  const ActFeatureMethodProtocol &protocol_elem, ActIpv4 &result_ipv4);

  /**
   * @brief Get the ModularInfo object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_modular_info
   * @return ACT_STATUS
   */
  ACT_STATUS GetModularInfo(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem, ActDeviceModularInfo &result_modular_info);

  /**
   * @brief Pre-start the import process for MoxaIEI device
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @return ACT_STATUS
   */
  ACT_STATUS PreStartImport(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem);

  // /**
  //  * @brief Get the System Information object
  //  *
  //  * @param device
  //  * @param action_key
  //  * @param protocol_elem
  //  * @param result_system_information
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS GetSystemInformation(const ActDevice &device, const QString &action_key,
  //                                 const ActFeatureMethodProtocol &protocol_elem,
  //                                 ActMonitorSystemInformation &result_system_information);

  /**
   * @brief Get the System Utilization object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_system_utilization
   * @return ACT_STATUS
   */
  ACT_STATUS GetSystemUtilization(const ActDevice &device, const QString &action_key,
                                  const ActFeatureMethodProtocol &protocol_elem,
                                  ActMonitorSystemUtilization &result_system_utilization);

  /**
   * @brief Set the Information Setting object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param info_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetInformationSetting(const ActDevice &device, const QString &action_key,
                                   const ActFeatureMethodProtocol &protocol_elem,
                                   const ActInformationSettingTable &info_setting_table);

  /**
   * @brief Get the Information Setting object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_info_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetInformationSetting(const ActDevice &device, const QString &action_key,
                                   const ActFeatureMethodProtocol &protocol_elem,
                                   ActInformationSettingTable &result_info_setting_table);

  /**
   * @brief Set the Management Interface object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param mgmt_interface_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetManagementInterface(const ActDevice &device, const QString &action_key,
                                    const ActFeatureMethodProtocol &protocol_elem,
                                    const ActManagementInterfaceTable &mgmt_interface_table);

  /**
   * @brief Get the Management Interface object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_mgmt_interface_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetManagementInterface(const ActDevice &device, const QString &action_key,
                                    const ActFeatureMethodProtocol &protocol_elem,
                                    ActManagementInterfaceTable &result_mgmt_interface_table);

  /**
   * @brief Set the User Account object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param user_account_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetUserAccount(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem,
                            const ActUserAccountTable &user_account_table);

  /**
   * @brief Get the User Account object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_user_account_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetUserAccount(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem,
                            ActUserAccountTable &result_user_account_table);

  /**
   * @brief Set the Login Policy object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param login_policy_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetLoginPolicy(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem,
                            const ActLoginPolicyTable &login_policy_table);

  /**
   * @brief Get the Login Policy object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_login_policy_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetLoginPolicy(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem,
                            ActLoginPolicyTable &result_login_policy_table);

  /**
   * @brief Set the Snmp Trap Setting object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param snmp_trap_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetSnmpTrapSetting(const ActDevice &device, const QString &action_key,
                                const ActFeatureMethodProtocol &protocol_elem,
                                const ActSnmpTrapSettingTable &snmp_trap_table);

  /**
   * @brief Get the Snmp Trap Setting object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_snmp_trap_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetSnmpTrapSetting(const ActDevice &device, const QString &action_key,
                                const ActFeatureMethodProtocol &protocol_elem,
                                ActSnmpTrapSettingTable &result_snmp_trap_table);

  /**
   * @brief Set the Syslog Setting object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param syslog_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetSyslogSetting(const ActDevice &device, const QString &action_key,
                              const ActFeatureMethodProtocol &protocol_elem, const ActSyslogSettingTable &syslog_table);

  /**
   * @brief Get the Syslog Setting object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_syslog_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetSyslogSetting(const ActDevice &device, const QString &action_key,
                              const ActFeatureMethodProtocol &protocol_elem,
                              ActSyslogSettingTable &result_syslog_table);

  /**
   * @brief Set the TSN Device Time Setting object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param time_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetTsnDeviceTimeSetting(const ActDevice &device, const QString &action_key,
                                     const ActFeatureMethodProtocol &protocol_elem,
                                     const ActTimeSettingTable &time_table);

  /**
   * @brief Set the NOS Device Time Setting object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param time_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetNosDeviceTimeSetting(const ActDevice &device, const QString &action_key,
                                     const ActFeatureMethodProtocol &protocol_elem,
                                     const ActTimeSettingTable &time_table);
  /**
   * @brief Get the TSN Device Time setting object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_time_table
   * @return ACT_STATUS
   */
  ACT_STATUS
  GetTsnDeviceTimeSetting(const ActDevice &device, const QString &action_key,
                          const ActFeatureMethodProtocol &protocol_elem, ActTimeSettingTable &result_time_table);

  /**
   * @brief Get the NOS Device Time setting  object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_time_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetNosDeviceTimeSetting(const ActDevice &device, const QString &action_key,
                                     const ActFeatureMethodProtocol &protocol_elem,
                                     ActTimeSettingTable &result_time_table);

  /**
   * @brief Get the Time Status object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_time_data
   * @return ACT_STATUS
   */
  ACT_STATUS GetTimeStatus(const ActDevice &device, const QString &action_key,
                           const ActFeatureMethodProtocol &protocol_elem, ActTimeDate &result_time_data);

  /**
   * @brief Set the Loop Protection object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param lp_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetLoopProtection(const ActDevice &device, const QString &action_key,
                               const ActFeatureMethodProtocol &protocol_elem, const ActLoopProtectionTable &lp_table);

  /**
   * @brief Get the Loop Protection object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_lp_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetLoopProtection(const ActDevice &device, const QString &action_key,
                               const ActFeatureMethodProtocol &protocol_elem, ActLoopProtectionTable &result_lp_table);

  /**
   * @brief Get the Port Admin Status object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_port_admin_status_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetPortSettingAdminStatus(const ActDevice &device, const QString &action_key,
                                       const ActFeatureMethodProtocol &protocol_elem,
                                       QMap<qint64, bool> &result_port_admin_status_map);
  /**
   * @brief Set the Port Settings object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param port_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetPortSetting(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem,
                            const ActPortSettingTable &port_setting_table);

  /**
   * @brief Get the Port Info object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_port_info_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetPortInfo(const ActDevice &device, const QString &action_key,
                         const ActFeatureMethodProtocol &protocol_elem,
                         QMap<qint64, ActDevicePortInfoEntry> &result_port_info_map);

  /**
   * @brief Get the Port Status object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_port_status_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetPortStatus(const ActDevice &device, const QString &action_key,
                           const ActFeatureMethodProtocol &protocol_elem,
                           QMap<qint64, ActMonitorPortStatusEntry> &result_port_status_map);
  /**
   * @brief Get the Fiber Check Status object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_port_fiber_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetFiberCheckStatus(const ActDevice &device, const QString &action_key,
                                 const ActFeatureMethodProtocol &protocol_elem,
                                 QMap<qint64, ActMonitorFiberCheckEntry> &result_port_fiber_map);

  /**
   * @brief Get the Traffic Statistics object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_port_traffic_statistics_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetTrafficStatistics(const ActDevice &device, const QString &action_key,
                                  const ActFeatureMethodProtocol &protocol_elem,
                                  QMap<qint64, ActMonitorTrafficStatisticsEntry> &result_port_traffic_statistics_map);

  /**
   * @brief  Get the TimeSync 1588 DefaultInfo object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_time_sync_status
   * @return ACT_STATUS
   */
  ACT_STATUS Get1588DefaultInfo(const ActDevice &device, const QString &action_key,
                                const ActFeatureMethodProtocol &protocol_elem,
                                ActMonitorTimeSyncStatus &result_time_sync_status);

  /**
   * @brief Get the TimeSync Dot1as Info object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_time_sync_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetDot1asInfo(const ActDevice &device, const QString &action_key,
                           const ActFeatureMethodProtocol &protocol_elem,
                           ActMonitorTimeSyncStatus &result_time_sync_status);

  /**
   * @brief Get the RSTP status object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_rstp_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetRstpStatus(const ActDevice &device, const QString &action_key,
                           const ActFeatureMethodProtocol &protocol_elem, ActMonitorRstpStatus &result_rstp_status);

  /**
   * @brief Get the Spanning Tree object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetSpanningTree(const ActDevice &device, const QString &action_key,
                             const ActFeatureMethodProtocol &protocol_elem, ActRstpTable &rstp_table);

  /**
   * @brief Get the StdVlan Table object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_vlan_static_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetStdVlanTable(const ActDevice &device, const QString &action_key,
                             const ActFeatureMethodProtocol &protocol_elem,
                             ActVlanStaticTable &result_vlan_static_table);

  /**
   * @brief Delete the StdVlan member
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param delete_vlan_list
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteStdVlanMember(const ActDevice &device, const QString &action_key,
                                 const ActFeatureMethodProtocol &protocol_elem, const QList<qint32> &delete_vlan_list);

  /**
   * @brief Add the StdVlan member
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param add_vlan_list
   * @return ACT_STATUS
   */
  ACT_STATUS AddStdVlanMember(const ActDevice &device, const QString &action_key,
                              const ActFeatureMethodProtocol &protocol_elem, const QList<qint32> &add_vlan_list);

  /**
   * @brief Set the StdVlan Table object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param vlan_static_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetStdVlanTable(const ActDevice &device, const QString &action_key,
                             const ActFeatureMethodProtocol &protocol_elem,
                             const ActVlanStaticTable &vlan_static_table);

  /**
   * @brief Get the StdVlan PVID object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_port_vlan_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetStdVlanPVID(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem, ActPortVlanTable &result_port_vlan_table);

  /**
   * @brief Get the MXVLAN VlanPortType object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_vlan_port_type_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetVlanPortType(const ActDevice &device, const QString &action_key,
                             const ActFeatureMethodProtocol &protocol_elem,
                             ActVlanPortTypeTable &result_vlan_port_type_table);
  /**
   * @brief Set the Vlan Port Type object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param vlan_port_type_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetVlanPortType(const ActDevice &device, const QString &action_key,
                             const ActFeatureMethodProtocol &protocol_elem,
                             const ActVlanPortTypeTable &vlan_port_type_table);

  /**
   * @brief Set the StdVlan PVID object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param port_vlan_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetStdVlanPVID(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem, const ActPortVlanTable &port_vlan_table);

  /**
   * @brief Get the Qos Default Priority object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_pcp_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetQosDefaultPriority(const ActDevice &device, const QString &action_key,
                                   const ActFeatureMethodProtocol &protocol_elem,
                                   ActDefaultPriorityTable &result_pcp_table);

  /**
   * @brief Set the Qos Default Priority object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param default_priority_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetQosDefaultPriority(const ActDevice &device, const QString &action_key,
                                   const ActFeatureMethodProtocol &protocol_elem,
                                   const ActDefaultPriorityTable &default_priority_table);

  /**
   * @brief Get the Management Vlan object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param result_mgmt_vlan
   * @return ACT_STATUS
   */
  ACT_STATUS GetManagementVlan(const ActDevice &device, const QString &action_key,
                               const ActFeatureMethodProtocol &protocol_elem, qint32 &result_mgmt_vlan);
  /**
   * @brief Set the Management Vlan object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param mgmt_vlan
   * @return ACT_STATUS
   */
  ACT_STATUS SetManagementVlan(const ActDevice &device, const QString &action_key,
                               const ActFeatureMethodProtocol &protocol_elem, const qint32 &mgmt_vlan);

  ACT_STATUS ImportConfig(const ActDevice &device, const QString &action_key,
                          const ActFeatureMethodProtocol &protocol_elem, const QString &file_path);

  ACT_STATUS ExportConfig(const ActDevice &device, const QString &action_key,
                          const ActFeatureMethodProtocol &protocol_elem, const QString &file_path);

  // /**
  //  * @brief Set the Network Settings Moxa object
  //  *
  //  * @param device
  //  * @param new_ipv4
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS SetNetworkSettingsMoxa(const ActDevice &device, const ActIpv4 &new_ipv4);

  // /**
  //  * @brief Firmware Upgrade MoxaIEI device
  //  *
  //  * @param device
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS FirmwareUpgradeMoxa(const ActDevice &device);

  // /**
  //  * @brief Get the Model Name Moxa object
  //  *
  //  * @param device
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS GetModelNameMoxa(ActDevice &device);

  // ACT_STATUS GetSerialNumberMoxa(ActDevice &device);

  // template <class TCLIENT>
  // ACT_STATUS CreateRestfulClient(const QString &device_ip, const quint16 &port, std::shared_ptr<TCLIENT> &client);
};

#endif /* ACT_RESTFUL_CLIENT_HANDLER_H */