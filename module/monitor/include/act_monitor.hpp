/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_MONITOR_HPP
#define ACT_MONITOR_HPP

#include <QQueue>
#include <QString>
#include <thread>

#include "act_common_feature.hpp"
#include "act_core.hpp"
#include "act_device.hpp"
#include "act_job.hpp"
#include "act_json.hpp"
#include "act_project.hpp"
#include "act_southbound.hpp"
#include "act_status.hpp"

class ActMonitor : public ActCommonFeature {
  Q_GADGET

  ACT_JSON_OBJECT(ActProfiles, profiles, Profiles);  ///< Profiles item
  ACT_JSON_FIELD(bool, fake_mode, FakeMode);         ///< FakeMode item

 public:
  // QQueue<ActDeviceMonitorResult> result_queue_;
  ActSouthbound southbound_;

  /**
   * @brief Construct a new Act Monitor object
   *
   */
  ActMonitor(const ActProfiles &profiles) {
    this->SetFeatureName(__func__);
    profiles_ = profiles;
    southbound_.SetProfiles(profiles);
  }

  /**
   * @brief Monitor device enqueue ErrorHandler object
   *
   * @param error_reason
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS MonitorDeviceEnqueueErrorHandler(const QString &called_func, const QString &error_reason,
                                              const ActDevice &device);

  /**
   * @brief Get the Connect Control By Feature object
   *
   * @param device
   * @param connect_control
   * @return ACT_STATUS
   */
  ACT_STATUS GetConnectControlByFeature(const ActDevice &device, ActDeviceConnectStatusControl &connect_control);

  /**
   * @brief Update device connect status by monitor feature
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceConnectByMonitorFeature(ActDevice &device);

  /**
   * @brief Assign Device ModularConfig And Interfaces
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceModuleAndInterfaces(ActDevice &device);

  /**
   * @brief Get the Basic Status object
   *
   * @param device
   * @param result_basic_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetBasicStatus(const ActDevice &device, ActMonitorBasicStatus &result_basic_status);

  /**
   * @brief Get the Device IP Configuration object
   *
   * @param device
   * @param result_network_setting
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceIPConfiguration(const ActDevice &device, ActNetworkSettingTable &result_network_setting);

  /**
   * @brief Get the Device User Account object
   *
   * @param device
   * @param result_user_account
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceUserAccount(const ActDevice &device, ActUserAccountTable &result_user_account);

  /**
   * @brief Get the Device Login Policy object
   *
   * @param device
   * @param result_login_policy
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceLoginPolicy(const ActDevice &device, ActLoginPolicyTable &result_login_policy);

  /**
   * @brief Get the Device Snmp Trap Setting object
   *
   * @param device
   * @param result_snmp_trap_setting
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceSnmpTrapSetting(const ActDevice &device, ActSnmpTrapSettingTable &result_snmp_trap_setting);

  /**
   * @brief Get the Device Syslog Setting object
   *
   * @param device
   * @param result_syslog_setting
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceSyslogSetting(const ActDevice &device, ActSyslogSettingTable &result_syslog_setting);

  /**
   * @brief Get the Device Time Setting object
   *
   * @param device
   * @param result_time_setting
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceTimeSetting(const ActDevice &device, ActTimeSettingTable &result_time_setting);

  /**
   * @brief Get the Device Port Setting object
   *
   * @param device
   * @param result_port_setting
   * @return ACT_STATUS
   */
  ACT_STATUS GetDevicePortSetting(const ActDevice &device, ActPortSettingTable &result_port_setting);
  /**
   * @brief Get the Device Loop Protection object
   *
   * @param device
   * @param result_loop_protection
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceLoopProtection(const ActDevice &device, ActLoopProtectionTable &result_loop_protection);

  /**
   * @brief Get the Device Information Setting object
   *
   * @param device
   * @param result_info_setting
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceInformationSetting(const ActDevice &device, ActInformationSettingTable &result_info_setting);

  /**
   * @brief Get the Device Management Interface object
   *
   * @param device
   * @param result_mgmt_interface
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceManagementInterface(const ActDevice &device, ActManagementInterfaceTable &result_mgmt_interface);

  /**
   * @brief Get the Device RSTP Setting object
   *
   * @param device
   * @param result_rstp
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceRstpSetting(const ActDevice &device, ActRstpTable &result_rstp);

  /**
   * @brief Get the VLAN configuration object
   *
   * @param device
   * @param result_vlan_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetVlan(const ActDevice &device, ActVlanTable &result_vlan_table);

  /**
   * @brief Get the Port Default PCP object
   *
   * @param device
   * @param result_pcp_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetPortDefaultPCP(const ActDevice &device, ActDefaultPriorityTable &result_pcp_table);

  /**
   * @brief Get the Stream Priority Ingress object
   *
   * @param device
   * @param result_stad_port_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetStreamPriorityIngress(const ActDevice &device, ActStadPortTable &result_stad_port_table);

  /**
   * @brief Get the Stream Priority Egress object
   *
   * @param device
   * @param result_stad_config_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetStreamPriorityEgress(const ActDevice &device, ActStadConfigTable &result_stad_config_table);

  /**
   * @brief Get the Device Unicast Static Forward object
   *
   * @param device
   * @param static_forward_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceUnicastStatic(const ActDevice &device, ActStaticForwardTable &static_forward_table);

  /**
   * @brief Get the Device Multicast Static Forward object
   *
   * @param device
   * @param static_forward_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceMulticastStatic(const ActDevice &device, ActStaticForwardTable &static_forward_table);

  /**
   * @brief Get the Time Sync Setting object
   *
   * @param device
   * @param result_time_sync_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetTimeSyncSetting(const ActDevice &device, ActTimeSyncTable &result_time_sync_table);

  /**
   * @brief Get the Device TimeAwareShaper object
   *
   * @param device
   * @param result_gcl_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceTimeAwareShaper(const ActDevice &device, ActGclTable &result_gcl_table);

  /**
   * @brief Get the RSTP status object
   *
   * @param device
   * @param result_rstp_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetRSTPStatus(const ActDevice &device, ActMonitorRstpStatus &result_rstp_status);

  /**
   * @brief Get the Traffic object
   *
   * @param device
   * @param result_traffic
   * @return ACT_STATUS
   */
  ACT_STATUS GetTraffic(const ActDevice &device, ActDeviceMonitorTraffic &result_traffic);

  /**
   * @brief Get the Time Synchronization object
   *
   * @param device
   * @param result_time_synchronization
   * @return ACT_STATUS
   */
  ACT_STATUS GetTimeSynchronization(const ActDevice &device, ActMonitorTimeStatus &result_time_synchronization);

  /**
   * @brief Ping the device
   *
   * @param device
   * @param ping_device
   * @return ACT_STATUS
   */
  ACT_STATUS PingDevice(const ActPingJob &ping_job, ActPingDevice &ping_device);

  void MutiPingDeviceTask(const ActPingJob &ping_job, ActPingDevice &ping_device);

  /**
   * @brief Identify the new Device
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS IdentifyNewDevice(ActDevice &device);

  /**
   * @brief Assign Device's ScanData(LLDP data, MAC table)
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceScanLinkData(ActDevice &device);

  /**
   * @brief Generate Device's links
   *
   * @param device
   * @param project_devices
   * @param scan_link_result
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateDeviceLinks(const ActDevice &device, const QSet<ActDevice> &project_devices,
                                 ActScanLinksResult &scan_link_result);

  /**
   * @brief Keep the RESTful connection
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS KeepRestfulConnection(const ActDevice &device);
  void MultiKeepRestfulConnectionTask(const ActDevice &device);

  /**
   * @brief Find source device (management endpoint)
   *
   * @param project_devices
   * @param result_src_device
   * @return ACT_STATUS
   */
  ACT_STATUS FindSourceDevice(const QSet<ActDevice> &project_devices, ActSourceDevice &result_src_device);

  /**
   * @brief Check the device is source device
   *
   * @param device
   * @param result_src_device
   * @return ACT_STATUS
   */
  ACT_STATUS CheckDeviceAsManagementEndpoint(const ActDevice &device, ActSourceDevice &result_src_device);

  /**
   * @brief Start Monitor thread object
   *
   * @param project
   * @return ACT_STATUS
   */
  // ACT_STATUS Start(ActProject &project);

  /**
   * @brief Stop the Monitor thread
   *
   * @return ACT_STATUS
   */
  // ACT_STATUS Stop();
};

#endif /* ACT_MONITOR_HPP */