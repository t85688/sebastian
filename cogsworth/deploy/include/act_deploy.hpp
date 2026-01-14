/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_DEPLOY_HPP
#define ACT_DEPLOY_HPP

#include <QDebug>
#include <QQueue>
#include <QString>
#include <thread>

#include "act_compare.hpp"
#include "act_deploy_parameter.hpp"
#include "act_deploy_result.hpp"
#include "act_device_profile.hpp"
#include "act_json.hpp"
#include "act_profiles.hpp"
#include "act_project.hpp"
#include "act_southbound.hpp"
#include "act_southbound_result.hpp"
#include "act_status.hpp"
#include "act_utilities.hpp"
#include "deploy_entry/act_deploy_table.hpp"
#include "deploy_entry/act_gate_parameters.hpp"
#include "gcl/act_gcl_result.hpp"
#include "routing/act_routing_result.hpp"
#include "stream/act_stream.hpp"
#include "stream/act_stream_id.hpp"
#include "topology/act_device.hpp"
#include "topology/act_link.hpp"

namespace act {
namespace deploy {

/**
 * @brief The Deploy's DeployControl class
 *
 */
class ActDeployControl : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, reboot, Reboot);
  ACT_JSON_FIELD(bool, factory_default, FactoryDefault);
  ACT_JSON_FIELD(bool, firmware_upgrade, FirmwareUpgrade);

  ACT_JSON_FIELD(bool, network_setting, NetworkSetting);
  ACT_JSON_FIELD(bool, login_policy, LoginPolicy);
  ACT_JSON_FIELD(bool, snmp_trap_setting, SnmpTrapSetting);
  ACT_JSON_FIELD(bool, syslog_setting, SyslogSetting);
  ACT_JSON_FIELD(bool, time_setting, TimeSetting);
  ACT_JSON_FIELD(bool, information_setting, InformationSetting);
  ACT_JSON_FIELD(bool, management_interface, ManagementInterface);
  ACT_JSON_FIELD(bool, port_setting, PortSetting);
  ACT_JSON_FIELD(bool, loop_protection, LoopProtection);
  ACT_JSON_FIELD(bool, vlan, Vlan);
  ACT_JSON_FIELD(bool, port_default_pcp, PortDefaultPcp);
  ACT_JSON_FIELD(bool, unicast_static_forward, UnicastStaticForward);
  ACT_JSON_FIELD(bool, multicast_static_forward, MulticastStaticForward);
  ACT_JSON_FIELD(bool, stream_priority_ingress, StreamPriorityIngress);
  ACT_JSON_FIELD(bool, stream_priority_egress, StreamPriorityEgress);
  ACT_JSON_FIELD(bool, gcl, Gcl);
  ACT_JSON_FIELD(bool, te_mstid, TeMstid);
  ACT_JSON_FIELD(bool, ieee802_dot1cb, Ieee802Dot1Cb);

  ACT_JSON_FIELD(bool, from_broadcast_search, FromBroadcastSearch);

  ACT_JSON_FIELD(bool, spanning_tree, SpanningTree);

 public:
  /**
   * @brief Construct a new Deploy Control object
   *
   */
  ActDeployControl()
      : reboot_(false),
        factory_default_(false),
        firmware_upgrade_(false),
        network_setting_(false),
        login_policy_(false),
        snmp_trap_setting_(false),
        syslog_setting_(false),
        time_setting_(false),
        information_setting_(false),
        management_interface_(false),
        port_setting_(false),
        loop_protection_(false),
        vlan_(false),
        port_default_pcp_(false),
        unicast_static_forward_(false),
        multicast_static_forward_(false),
        stream_priority_ingress_(false),
        stream_priority_egress_(false),
        gcl_(false),
        te_mstid_(false),
        ieee802_dot1cb_(false),
        from_broadcast_search_(false),
        spanning_tree_(false) {}
};

class ActDeploy {
  Q_GADGET

  ACT_JSON_FIELD(bool, deployer_stop_flag, DeployerStopFlag);
  ACT_JSON_FIELD(quint8, progress, Progress);
  ACT_JSON_QT_SET(qint64, network_setting_success_devices, NetworkSettingSuccessDevices);
  ACT_JSON_OBJECT(ActProfiles, profiles, Profiles);

  ACT_STATUS deployer_act_status_;
  std::unique_ptr<std::thread> deployer_thread_;

 public:
  QQueue<ActDeviceConfigureResult> result_queue_;
  QQueue<QPair<qint64, ActDeviceAccount>> update_account_queue_;  // QPair<DeviceID, ActDeviceAccount>

  // QMap<qint64, ActDeviceAccount> update_account_map_;  // QMap<DeviceID, ActDeviceAccount>

  QSet<qint64> failed_device_id_set_;

 private:
  QMap<QString, QString> mac_host_map_;
  ActSouthbound southbound_;

  /**
   * @brief Triggered the deployer for the thread
   *
   * @param project
   * @param dev_id_list
   * @param deploy_control
   * @param parameter_base
   */
  void TriggeredDeployerForThread(const ActProject &project, const QList<qint64> &dev_id_list,
                                  const ActDeployControl &deploy_control, const bool &skip_mapping_dev,
                                  ActDeployParameterBase *parameter_base);

  /**
   * @brief Triggered the ini deployer for the thread
   *
   * @param project
   * @param dev_id_list
   */
  void TriggeredIniDeployerForThread(const ActProject &project, const QList<qint64> &dev_id_list,
                                     const bool &skip_mapping_dev);

  /**
   * @brief Deploy error handler
   *
   * @param called_func
   * @param error_reason
   * @param error_detail
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS DeployErrorHandler(QString called_func, const QString &error_reason, const QString &error_detail,
                                const ActDevice &device);

  /**
   * @brief
   *
   * @param called_func
   * @param error_reason
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS DeployIniErrorHandler(QString called_func, const QString &error_reason, const ActDevice &device);

  /**
   * @brief Check feature error handler
   *
   * @param called_func
   * @param check_item
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS CheckFeatureErrorHandler(QString called_func, const QString &check_item, const ActDevice &device);

  /**
   * @brief Check device alive by icmp
   *
   * @param device
   * @param from_broadcast_search
   * @return ACT_STATUS
   */
  ACT_STATUS CheckDeviceAlive(const ActDevice &device, bool from_broadcast_search);

  /**
   * @brief Get the Ini Config FileName object
   *
   * @param dev
   * @param file_name
   * @return ACT_STATUS
   */
  ACT_STATUS GetIniConfigFileName(const ActDevice &dev, QString &file_name);

  /**
   * @brief Check TSN-Switch configuration synchronized
   *
   * @param dev
   * @param dev_config
   * @param result
   * @return ACT_STATUS
   */
  ACT_STATUS CheckTSNSwitchConfigurationSynchronized(const ActDevice &dev, ActDeviceConfig &dev_config, bool &result);

  /**
   * @brief Check GCL's AdminControlListLength
   *
   * @param device
   * @param gate_control_table
   * @return ACT_STATUS
   */
  ACT_STATUS CheckGclAdminControlListLength(const ActDevice &device, const ActGclTable &gate_control_table);

  /**
   * @brief Mapping Device firmware(Update FeatureGroup and FirmwareFeatureProfileId)
   *
   * @param device
   * @param device_config
   * @return ACT_STATUS
   */
  ACT_STATUS MappingDeviceFirmware(ActDevice &device, const ActDeviceConfig &device_config,
                                   const bool &skip_mapping_dev);

  /**
   * @brief Check device feature
   *
   * @param device
   * @param device_config
   * @return ACT_STATUS
   */
  ACT_STATUS CheckDeviceFeature(const ActDevice &device, const ActDeviceConfig &device_config);

  /**
   * @brief Reboot device
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS Reboot(const ActDevice &device);

  /**
   * @brief Factory Default
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS FactoryDefault(const ActDevice &device);

  /**
   * @brief Firmware Upgrade
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS FirmwareUpgrade(const ActDevice &device, const QString &firmware_name);

  ACT_STATUS DeployNetworkSetting(const ActDevice &device, const ActNetworkSettingTable &network_setting_table,
                                  bool from_broadcast_search);

  ACT_STATUS DeployMappingDeviceIpSetting(const ActDevice &device,
                                          const ActMappingDeviceIpSettingTable &ip_setting_table);

  /**
   * @brief Configure device NETCONF configuration
   *
   * @param device
   * @param deploy_control
   * @param device_config
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureNetconf(const ActDevice &device, const ActDeployControl &deploy_control,
                              const ActDeviceConfig &device_config);

  /**
   * @brief  Update progress
   *
   * @param progress
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProgress(quint8 progress);

 public:
  /**
   * @brief Construct a new Act Deploy object
   *
   */
  ActDeploy(const ActProfiles &profiles, const QMap<QString, QString> &mac_host_map) {
    profiles_ = profiles;
    progress_ = 0;
    failed_device_id_set_.clear();
    deployer_stop_flag_ = false;
    deployer_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);
    deployer_thread_ = nullptr;
    mac_host_map_ = mac_host_map;
    southbound_.SetProfiles(profiles);
  }

  /**
   * @brief Destroy the Act Deploy object
   *
   */
  ~ActDeploy();

  /**
   * @brief Deployer the class and stream base
   *
   * @param project
   * @param dev_id_list
   * @param deploy_control
   * @param parameter_base
   * @return ACT_STATUS
   */
  ACT_STATUS Deployer(const ActProject &project, const QList<qint64> &dev_id_list,
                      const ActDeployControl &deploy_control, const bool &skip_mapping_dev,
                      ActDeployParameterBase *parameter_base);

  /**
   * @brief Ini Deployer the class and stream base
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS IniDeployer(const ActProject &project, const QList<qint64> &dev_id_list, const bool &skip_mapping_dev);

  /**
   * @brief Start the deployer by new thread
   *
   * @param project
   * @param dev_id_list
   * @param deploy_control
   * @param parameter_base
   * @return ACT_STATUS
   */
  ACT_STATUS Start(const ActProject &project, const QList<qint64> &dev_id_list, const ActDeployControl &deploy_control,
                   const bool &skip_mapping_dev, ActDeployParameterBase *parameter_base);

  /**
   * @brief Start the ini deployer by new thread
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS StartIniDeployer(const ActProject &project, const QList<qint64> &dev_id_list,
                              const bool &skip_mapping_dev);

  /**
   * @brief Stop the deployer thread
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Stop();

  /**
   * @brief Get the status of the deployer thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetStatus();

  /**
   * @brief Import device config file
   *
   * @param device
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS ImportConfig(const ActDevice &device, const QString &file_path);
};
}  // namespace deploy
}  // namespace act

#endif /* ACT_DEPLOY_HPP */