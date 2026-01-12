/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include <qmath.h>

#include <QMutex>
#include <QUuid>

#include "act_algorithm_configuration.hpp"
#include "act_class_based.hpp"
#include "act_computed_result.hpp"
#include "act_device_profile.hpp"
#include "act_group.hpp"
#include "act_json.hpp"
#include "act_monitor_data.hpp"
#include "act_scan_ip_range.hpp"
#include "act_service_profile.hpp"
#include "act_snmp_trap_configuration.hpp"
#include "act_status.hpp"
#include "act_system.hpp"
#include "act_utilities.hpp"
#include "act_vlan_view.hpp"
#include "deploy_entry/act_manufacture_result.hpp"
#include "device_configuration/act_device_config.hpp"
#include "routing/act_host_pair.hpp"
#include "simplecrypt.h"
#include "stream/act_stream.hpp"
#include "stream/act_traffic.hpp"
#include "topology/act_device.hpp"
#include "topology/act_link.hpp"
#include "topology/act_topology_mapping_result.hpp"

enum class ActProjectModeEnum { kDesign = 0, kOperation = 1, kManufacture = 2 };
static const QMap<QString, ActProjectModeEnum> kActProjectModeEnumMap = {
    {"Design", ActProjectModeEnum::kDesign},
    {"Operation", ActProjectModeEnum::kOperation},
    {"Manufacture", ActProjectModeEnum::kManufacture}};

enum class ActQuestionnaireModeEnum { kNone = 0, kVerified = 1, kUnVerified = 2 };
static const QMap<QString, ActQuestionnaireModeEnum> kActQuestionnaireModeEnumMap = {
    {"", ActQuestionnaireModeEnum::kNone},
    {"Verified", ActQuestionnaireModeEnum::kVerified},
    {"UnVerified", ActQuestionnaireModeEnum::kUnVerified}};

/**
 * @brief The type class enum class for IP configuring sequence TSN configuration wizard
 *
 */
enum class ActIpConfiguringSequenceTypeEnum { kFromFarToNear = 1, kFromNearToFar = 2 };

/**
 * @brief The QMap for ActIpConfiguringSequenceTypeEnum class enum mapping
 *
 */
static const QMap<QString, ActIpConfiguringSequenceTypeEnum> kActIpConfiguringSequenceTypeEnumMap = {
    {"FromFarToNear", ActIpConfiguringSequenceTypeEnum::kFromFarToNear},
    {"FromNearToFar", ActIpConfiguringSequenceTypeEnum::kFromNearToFar}};

/**
 * @brief The type class enum class for IP assigning sequence TSN configuration wizard
 *
 */
enum class ActIpAssigningSequenceTypeEnum {
  kByMacDescending = 1,
  kByMacAscending = 2,
  kFromFarToNear = 3,
  kFromNearToFar = 4
};

/**
 * @brief The QMap for ActIpAssigningSequenceTypeEnum class enum mapping
 *
 */
static const QMap<QString, ActIpAssigningSequenceTypeEnum> kActIpAssigningSequenceTypeEnumMap = {
    {"ByMacDescending", ActIpAssigningSequenceTypeEnum::kByMacDescending},
    {"ByMacAscending", ActIpAssigningSequenceTypeEnum::kByMacAscending},
    {"FromFarToNear", ActIpAssigningSequenceTypeEnum::kFromFarToNear},
    {"FromNearToFar", ActIpAssigningSequenceTypeEnum::kFromNearToFar}};

/**
 * @brief The type class enum class for IP assignment TSN configuration wizard
 *
 */
enum class ActDefineDeviceToBeSetTypeEnum {
  kAllDevicesUponSearch = 1,
  kDevicesOfSpecificIPs = 2,
  kDevicesExcludingSpecificIPs = 3
};

/**
 * @brief The QMap for ActDefineDeviceToBeSetTypeEnum class enum mapping
 *
 */
static const QMap<QString, ActDefineDeviceToBeSetTypeEnum> kActDefineDeviceToBeSetTypeEnumMap = {
    {"AllDevicesUponSearch", ActDefineDeviceToBeSetTypeEnum::kAllDevicesUponSearch},
    {"DevicesOfSpecificIPs", ActDefineDeviceToBeSetTypeEnum::kDevicesOfSpecificIPs},
    {"DevicesExcludingSpecificIPs", ActDefineDeviceToBeSetTypeEnum::kDevicesExcludingSpecificIPs}};

/**
 * @brief The type class enum class for IP assignment type TSN configuration wizard
 *
 */
enum class ActDefineIpAssignmentTypeEnum { kDefineIpAssigningRule = 1, kDefineIpAssignmentRange = 2 };

/**
 * @brief The QMap for ActDefineIpAssignmentTypeEnum class enum mapping
 *
 */
static const QMap<QString, ActDefineIpAssignmentTypeEnum> kActDefineIpAssignmentTypeEnumMap = {
    {"DefineIpAssigningRule", ActDefineIpAssignmentTypeEnum::kDefineIpAssigningRule},
    {"DefineIpAssignmentRange", ActDefineIpAssignmentTypeEnum::kDefineIpAssignmentRange}};

/**
 * @brief The project status enum class
 *
 */
enum class ActProjectStatusEnum {
  kIdle,
  kComputing,
  kComparing,
  kDeploying,
  kSyncing,
  kScanning,
  kMonitoring,
  kTopologyMapping,
  kBroadcastSearching,
  kDeviceConfiguring,
  kIntelligentRequestSending,
  kIntelligentUploadSending,
  kIntelligentDownloadSending,
  kAborted,
  kFinished
};

/**
 * @brief The QMap for project status enum mapping
 *
 */
static const QMap<QString, ActProjectStatusEnum> kActProjectStatusEnumMap = {
    {"Idle", ActProjectStatusEnum::kIdle},
    {"Computing", ActProjectStatusEnum::kComputing},
    {"Comparing", ActProjectStatusEnum::kComparing},
    {"Deploying", ActProjectStatusEnum::kDeploying},
    {"Syncing", ActProjectStatusEnum::kSyncing},
    {"Scanning", ActProjectStatusEnum::kScanning},
    {"Monitoring", ActProjectStatusEnum::kMonitoring},
    {"TopologyMapping", ActProjectStatusEnum::kTopologyMapping},
    {"BroadcastSearching", ActProjectStatusEnum::kBroadcastSearching},
    {"DeviceConfiguring", ActProjectStatusEnum::kDeviceConfiguring},
    {"IntelligentRequestSending", ActProjectStatusEnum::kIntelligentRequestSending},
    {"IntelligentUploadSending", ActProjectStatusEnum::kIntelligentUploadSending},
    {"IntelligentDownloadSending", ActProjectStatusEnum::kIntelligentDownloadSending},
    {"Aborted", ActProjectStatusEnum::kAborted},
    {"Finished", ActProjectStatusEnum::kFinished}};

static const QList<ActProjectStatusEnum> ActMonitorConcurrentJobStatus = {
    ActProjectStatusEnum::kComputing, ActProjectStatusEnum::kIntelligentRequestSending,
    ActProjectStatusEnum::kIntelligentUploadSending, ActProjectStatusEnum::kIntelligentDownloadSending,
    ActProjectStatusEnum::kDeviceConfiguring};
/**
 * @brief The project setting class enum class
 *
 */
enum class ActProjectSettingMember {
  kProjectName,
  kAlgorithmConfiguration,
  kVlanRange,
  kAccount,
  kCfgWizardSetting,
  kNetconfConfiguration,
  kSnmpConfiguration,
  kRestfulConfiguration,
  kTrafficTypeToPriorityCodePointMapping,
  kPriorityCodePointToQueueMapping,
  kScanIpRanges,
  kProjectStartIp,
  kSnmpTrapConfiguration,
  kMonitorConfiguration
};

/**
 * @brief The QMap for project setting class enum mapping
 *
 */
static const QMap<QString, ActProjectSettingMember> kActProjectSettingMemberMap = {
    {"ProjectName", ActProjectSettingMember::kProjectName},
    {"AlgorithmConfiguration", ActProjectSettingMember::kAlgorithmConfiguration},
    {"VlanRange", ActProjectSettingMember::kVlanRange},
    {"CfgWizardSetting", ActProjectSettingMember::kCfgWizardSetting},
    {"Account", ActProjectSettingMember::kAccount},
    {"NetconfConfiguration", ActProjectSettingMember::kNetconfConfiguration},
    {"SnmpConfiguration", ActProjectSettingMember::kSnmpConfiguration},
    {"RestfulConfiguration", ActProjectSettingMember::kRestfulConfiguration},
    {"TrafficTypeToPriorityCodePointMapping", ActProjectSettingMember::kTrafficTypeToPriorityCodePointMapping},
    {"PriorityCodePointToQueueMapping", ActProjectSettingMember::kPriorityCodePointToQueueMapping},
    {"ScanIpRanges", ActProjectSettingMember::kScanIpRanges},
    {"ProjectStartIp", ActProjectSettingMember::kProjectStartIp},
    {"SnmpTrapConfiguration", ActProjectSettingMember::kSnmpTrapConfiguration},
    {"MonitorConfiguration", ActProjectSettingMember::kMonitorConfiguration}

};

/**
 * @brief The IP range class
 *
 */
class ActIpRange : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, start_ip, StartIp);
  ACT_JSON_FIELD(QString, end_ip, EndIp);

 public:
  ActIpRange() {}
};

/**
 * @brief The Vlan Range class
 *
 */
class ActVlanRange : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, min, Min);
  ACT_JSON_FIELD(quint16, max, Max);

 public:
  /**
   * @brief Construct a new Vlan Range object
   *
   */
  ActVlanRange() : min_(ACT_VLAN_MIN), max_(ACT_VLAN_MAX) {}

  /**
   * @brief Construct a new Vlan Range object
   *
   * @param min
   * @param max
   */
  ActVlanRange(const quint16 &min, const quint16 &max) : min_(min), max_(max) {}

  /**
   * @brief Check input vlan in the vlan range
   *
   * @param vlan
   * @return true
   * @return false
   */
  bool InVlanRange(const quint16 &vlan) const { return (min_ <= vlan) && (vlan < max_); }
};

/**
 * @brief The TrafficType to PriorityCodePoint Mapping class
 *
 */
class ActTrafficTypeToPriorityCodePointMapping : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET(quint8, na, NA);
  ACT_JSON_QT_SET(quint8, best_effort, BestEffort);
  ACT_JSON_QT_SET(quint8, cyclic, Cyclic);
  ACT_JSON_QT_SET(quint8, time_sync, TimeSync);

 public:
  ActTrafficTypeToPriorityCodePointMapping() {
    this->SetBestEffort({0, 1});
    this->SetTimeSync({6});
    this->SetCyclic({2, 3, 4, 5, 7});
  }
};

/**
 * @brief The class of configuration of define device type in the TSN configuration wizard
 *
 */
class ActDefineDeviceType : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, skip, Skip);  ///< Specifies the config is skip in the configuration wizard
  ACT_JSON_FIELD(bool, moxa_industrial_ethernet_product,
                 MoxaIndustrialEthernetProduct);  ///< Specifies the Ethernet Switches/Secure Routers/DSL Extenders
                                                  ///< serial products are active in the configuration wizard
  ACT_JSON_FIELD(bool, moxa_industrial_wireless_product,
                 MoxaIndustrialWirelessProduct);  ///< Specifies the Wireless serial products are active in the
                                                  ///< configuration wizard

 public:
  ActDefineDeviceType() {
    this->skip_ = true;
    this->moxa_industrial_ethernet_product_ = true;
    this->moxa_industrial_wireless_product_ = false;
  }
};

/**
 * @brief The class of configuration of define network interface in the TSN configuration wizard
 *
 */
class ActDefineNetworkInterface : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, skip, Skip);  ///< Specifies the config is skip in the configuration wizard
  ACT_JSON_QT_SET(qint64, define_network_interfaces,
                  DefineNetworkInterfaces);  ///< Specifies the Wireless serial products are active in the
                                             ///< configuration wizard

 public:
  ActDefineNetworkInterface() { this->skip_ = true; }
};

/**
 * @brief The class of configuration of unlock devices in the TSN configuration wizard
 *
 */
class ActUnlockDevices : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, skip, Skip);  ///< Specifies the config is skip in the configuration wizard

 public:
  ActUnlockDevices() { this->skip_ = true; }
};

/**
 * @brief The class of configuration of define network interface in the TSN configuration wizard
 *
 */
class ActDefineDeviceToBeSet : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, skip, Skip);  ///< Specifies the config is skip in the configuration wizard
  ACT_JSON_ENUM(ActDefineDeviceToBeSetTypeEnum, define_device_to_be_set_type,
                DefineDeviceToBeSetType);                          ///< The project type enum
  ACT_JSON_COLLECTION(QList, QString, specific_ips, SpecificIps);  ///< The specific ip list

 public:
  ActDefineDeviceToBeSet() {
    this->skip_ = true;
    this->define_device_to_be_set_type_ = ActDefineDeviceToBeSetTypeEnum::kAllDevicesUponSearch;
  }
};

/**
 * @brief The class of configuration of define ip assignment type in the TSN configuration wizard
 *
 */
class ActDefineIpAssigningRule : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, start_ip, StartIp);
  ACT_JSON_FIELD(quint8, incremental_gap, IncrementalGap);

 public:
  ActDefineIpAssigningRule() { this->incremental_gap_ = 1; }
};

/**
 * @brief The class of configuration of define ip assignment in the TSN configuration wizard
 *
 */
class ActDefineIpAssignment : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, skip, Skip);  ///< Specifies the config is skip in the configuration wizard
  ACT_JSON_ENUM(ActDefineIpAssignmentTypeEnum, define_ip_assignment,
                DefineIpAssignment);  ///< The project type enum

  ACT_JSON_OBJECT(ActDefineIpAssigningRule, define_ip_assigning_rule, ActDefineIpAssigningRule);  ///< The
  ACT_JSON_OBJECT(ActIpRange, define_ip_assigning_range, ActDefineIpAssigningRange);

 public:
  ActDefineIpAssignment() {
    this->skip_ = true;
    this->define_ip_assignment_ = ActDefineIpAssignmentTypeEnum::kDefineIpAssigningRule;
  }
};

/**
 * @brief The class of configuration of define ip assigning sequence in the TSN configuration wizard
 *
 */
class ActDefineIpAssigningSequence : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, skip, Skip);  ///< Specifies the config is skip in the configuration wizard
  ACT_JSON_ENUM(ActIpAssigningSequenceTypeEnum, ip_assigning_sequence_type,
                IpAssigningSequenceType);  ///< The sequence of IP assignment

 public:
  ActDefineIpAssigningSequence() {
    this->skip_ = true;
    this->ip_assigning_sequence_type_ = ActIpAssigningSequenceTypeEnum::kFromFarToNear;
  }
};

/**
 * @brief The class of configuration of define ip configuring sequence in the TSN configuration wizard
 *
 */
class ActDefineIpConfiguringSequence : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, skip, Skip);  ///< Specifies the config is skip in the configuration wizard
  ACT_JSON_ENUM(ActIpConfiguringSequenceTypeEnum, ip_configuring_sequence_type,
                IpConfiguringSequenceType);  ///< The sequence of IP assignment

 public:
  ActDefineIpConfiguringSequence() {
    this->skip_ = true;
    this->ip_configuring_sequence_type_ = ActIpConfiguringSequenceTypeEnum::kFromFarToNear;
  }
};

/**
 * @brief The class of configuration of define topology scan in the TSN configuration wizard
 *
 */
class ActDefineTopologyScan : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, skip, Skip);  ///< Specifies the config is skip in the configuration wizard

 public:
  ActDefineTopologyScan() { this->skip_ = true; }
};

/**
 * @brief The class of TSN configuration wizard setting
 *
 */
class ActCfgWizardSetting : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDefineDeviceType, define_device_type,
                  DefineDeviceType);  // The configuration of define device type in the TSN configuration wizard
  ACT_JSON_OBJECT(
      ActDefineNetworkInterface, define_network_interface,
      DefineNetworkInterface);  // The configuration of define network interface in the TSN configuration wizard

  ACT_JSON_OBJECT(ActUnlockDevices, unlock_devices,
                  UnlockDevices);  // The configuration of unlock devices in the TSN configuration wizard

  ACT_JSON_OBJECT(ActDefineDeviceToBeSet, define_device_to_be_set,
                  DefineDeviceToBeSet);  // The configuration of define device to be set in the TSN configuration wizard

  ACT_JSON_OBJECT(ActDefineIpAssignment, define_ip_assignment,
                  DefineIpAssignment);  // The configuration of ip assignment in the TSN
                                        // configuration wizard

  ACT_JSON_OBJECT(ActDefineIpAssigningSequence, define_ip_assigning_sequence,
                  DefineIpAssigningSequence);  // The configuration of ip assigning sequence in the TSN
                                               // configuration wizard

  ACT_JSON_OBJECT(ActDefineIpConfiguringSequence, define_ip_configuring_sequence,
                  DefineIpConfiguringSequence);  // The configuration of ip configuring sequence in the TSN
                                                 // configuration wizard

  ACT_JSON_OBJECT(ActDefineTopologyScan, define_topology_scan,
                  DefineTopologyScan);  // The configuration of topology scan in the TSN
                                        // configuration wizard
};

class ActManagementInterface : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint64, device_id, DeviceId);
  ACT_JSON_QT_SET(qint64, interfaces, Interfaces);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act Management Interface object
   *
   */
  ActManagementInterface() {
    this->device_id_ = -1;
    this->key_order_.append(QList<QString>({QString("DeviceId"), QString("Interfaces")}));
  }

  /**
   * @brief Construct a new Act Management Interface object
   *
   * @param device_id
   */
  ActManagementInterface(const quint64 device_id) : ActManagementInterface() { this->device_id_ = device_id; }

  /**
   * @brief Construct a new Act Management Interface object
   *
   * @param device_id
   * @param interfaces
   */
  ActManagementInterface(const quint64 device_id, const QSet<qint64> &interfaces) : ActManagementInterface() {
    this->device_id_ = device_id;
    this->interfaces_ = interfaces;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActManagementInterface &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
    return 0;
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActManagementInterface &x, const ActManagementInterface &y) {
    return x.device_id_ == y.device_id_;
  }
};

/**
 * @brief The RSTP class
 *
 */
class ActRSTP : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(qint64, hello_time, HelloTime);
  ACT_JSON_FIELD(qint64, root_device, RootDevice);
  ACT_JSON_FIELD(qint64, backup_root_device, BackupRootDevice);
  ACT_JSON_QT_SET(qint64, devices, Devices);

 public:
  QList<QString> key_order_;

  ActRSTP() {
    this->id_ = -1;
    this->hello_time_ = 1;
    this->root_device_ = -1;
    this->backup_root_device_ = -1;
    this->key_order_.append(QList<QString>(
        {QString("Id"), QString("HelloTime"), QString("RootDevice"), QString("BackupRootDevice"), QString("Devices")}));
  }

  /**
   * @brief Construct a new Act Project object
   *
   * @param id
   */
  ActRSTP(const qint64 &id) : ActRSTP() { this->id_ = id; }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActRSTP &x) {
    return qHash(x.id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActRSTP &x, const ActRSTP &y) { return (x.id_ == y.id_); }
};

class ActSwiftCandidate : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, root_device, RootDevice);
  ACT_JSON_FIELD(qint64, backup_root_device, BackupRootDevice);

 public:
  ActSwiftCandidate() {
    this->root_device_ = -1;
    this->backup_root_device_ = -1;
  }

  ActSwiftCandidate(const qint64 &root_device, const qint64 &backup_root_device) {
    this->root_device_ = root_device;
    this->backup_root_device_ = backup_root_device;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActSwiftCandidate &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
    return 0;
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActSwiftCandidate &x, const ActSwiftCandidate &y) {
    return (x.root_device_ == y.root_device_ && x.backup_root_device_ == y.backup_root_device_) ||
           (x.root_device_ == y.backup_root_device_ && x.backup_root_device_ == y.root_device_);
  }
};

class ActSwiftCandidates : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActSwiftCandidate, candidates, Candidates);
};

class ActSwift : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, active, Active);
  ACT_JSON_FIELD(qint64, root_device, RootDevice);
  ACT_JSON_FIELD(qint64, backup_root_device, BackupRootDevice);
  ACT_JSON_QT_DICT(QMap, qint64, qint16, device_tier_map, DeviceTierMap);
  ACT_JSON_QT_SET(qint64, links, Links);

 public:
  QList<QString> key_order_;

  ActSwift() {
    this->active_ = false;
    this->root_device_ = -1;
    this->backup_root_device_ = -1;
    this->key_order_.append(QList<QString>({QString("Active"), QString("RootDevice"), QString("BackupRootDevice"),
                                            QString("DeviceTierMap"), QString("Links")}));
  }

  ActSwift(const qint64 &root_device, const qint64 &backup_root_device) : ActSwift() {
    this->root_device_ = root_device;
    this->backup_root_device_ = backup_root_device;
  }
};

/**
 * @brief The redundant group class
 *
 */
class ActRedundantGroup : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActRSTP, rstp, RSTP);
  ACT_JSON_OBJECT(ActSwift, swift, Swift);

 public:
  QList<QString> key_order_;

  ActRedundantGroup() { this->key_order_.append(QList<QString>({QString("RSTP"), QString("Swift")})); }
};

/**
 * @brief The ACT topology setting class
 *
 */
class ActTopologySetting : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActIntelligentVlan, vlan_groups, IntelligentVlanGroup);
  ACT_JSON_OBJECT(ActRedundantGroup, redundant_group, RedundantGroup);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActManagementInterface, management_interfaces, ManagementInterfaces);
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActGroup, groups, Groups);

 public:
  QList<QString> key_order_;

 public:
  ActTopologySetting() {
    this->key_order_.append(QList<QString>({QString("IntelligentVlanGroup"), QString("RedundantGroup"),
                                            QString("ManagementInterfaces"), QString("Group")}));
  }
};

/**
 * @brief The ACT monitor setting class
 *
 */
class ActMonitorConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, polling_interval, PollingInterval);
  ACT_JSON_FIELD(bool, from_offline_project, FromOfflineProject);
  ACT_JSON_FIELD(bool, from_ip_scan_list, FromIpScanList);

 public:
  QList<QString> key_order_;

 public:
  ActMonitorConfiguration() {
    this->polling_interval_ = 15;
    this->from_offline_project_ = true;
    this->from_ip_scan_list_ = true;
    this->key_order_.append(
        QList<QString>({QString("PollingInterval"), QString("FromOfflineProject"), QString("FromIpScanList")}));
  }
};

/**
 * @brief The ACT project setting class
 *
 */
class ActProjectSetting : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_FIELD(QString, project_name, ProjectName);
  ACT_JSON_OBJECT(ActAlgorithmConfiguration, algorithm_configuration, AlgorithmConfiguration);
  ACT_JSON_OBJECT(ActVlanRange, vlan_range, VlanRange);

  ACT_JSON_OBJECT(ActCfgWizardSetting, cfg_wizard_setting, CfgWizardSetting);

  ACT_JSON_OBJECT(ActDeviceAccount, account, Account);
  ACT_JSON_OBJECT(ActNetconfConfiguration, netconf_configuration, NetconfConfiguration);
  ACT_JSON_OBJECT(ActSnmpConfiguration, snmp_configuration, SnmpConfiguration);
  ACT_JSON_OBJECT(ActRestfulConfiguration, restful_configuration, RestfulConfiguration);

  ACT_JSON_OBJECT(ActTrafficTypeToPriorityCodePointMapping, traffic_type_to_priority_code_point_mapping,
                  TrafficTypeToPriorityCodePointMapping);
  ACT_JSON_QT_DICT_SET(quint8, QSet<quint8>, priority_code_point_to_queue_mapping, PriorityCodePointToQueueMapping);

  ACT_JSON_COLLECTION_OBJECTS(QList, ActScanIpRangeEntry, scan_ip_ranges, ScanIpRanges);

  ACT_JSON_OBJECT(ActIpv4, project_start_ip, ProjectStartIp);

  ACT_JSON_OBJECT(ActSnmpTrapConfiguration, snmp_trap_configuration, SnmpTrapConfiguration);

  ACT_JSON_OBJECT(ActMonitorConfiguration, monitor_configuration, MonitorConfiguration);

 public:
  QList<QString> key_order_;

 private:
 public:
  ActProjectSetting() {
    this->project_start_ip_.SetIpAddress(ACT_DEFAULT_PROJECT_START_IP);
    this->project_start_ip_.SetSubnetMask("255.255.255.0");
    this->project_start_ip_.AutoAssignIPSubnetMask(ACT_DEFAULT_PROJECT_START_IP);

    this->key_order_.append(QList<QString>(
        {QString("ProjectName"), QString("AlgorithmConfiguration"), QString("VlanRange"), QString("CfgWizardSetting"),
         QString("Account"), QString("NetconfConfiguration"), QString("SnmpConfiguration"),
         QString("RestfulConfiguration"), QString("TrafficTypeToPriorityCodePointMapping"),
         QString("PriorityCodePointToQueueMapping"), QString("ScanIpRanges"), QString("ProjectStartIp"),
         QString("SnmpTrapConfiguration"), QString("MonitorConfiguration")}));

    QMap<quint8, QSet<quint8>> pcp_to_queue_map;
    for (quint8 i = 0; i < 8; i++) {
      QSet<quint8> queue_set;
      queue_set.insert(i);
      pcp_to_queue_map.insert(i, queue_set);
    }
    this->SetPriorityCodePointToQueueMapping(pcp_to_queue_map);
  }

  /**
   * @brief [feat:1662] Remove the password related fields in the project
   *
   * @return ACT_STATUS
   */
  ACT_STATUS HidePassword() {
    ACT_STATUS_INIT();

    // Hide scan_ip_range password
    QList<ActScanIpRangeEntry> new_scan_ip_ranges;
    for (auto scan_ip_range : this->GetScanIpRanges()) {
      scan_ip_range.HidePassword();
      new_scan_ip_ranges.append(scan_ip_range);
    }
    this->SetScanIpRanges(new_scan_ip_ranges);

    // Hide password in the project setting
    this->account_.HidePassword();
    this->snmp_configuration_.HidePassword();
    this->snmp_trap_configuration_.HidePassword();

    return act_status;
  }

  /**
   * @brief Encrypt the password field
   *
   * @return QString
   */
  ACT_STATUS EncryptPassword() {
    ACT_STATUS_INIT();

    // Encrypt scan_ip_range password
    QList<ActScanIpRangeEntry> new_scan_ip_ranges;
    for (auto scan_ip_range : this->GetScanIpRanges()) {
      scan_ip_range.EncryptPassword();
      new_scan_ip_ranges.append(scan_ip_range);
    }
    this->SetScanIpRanges(new_scan_ip_ranges);

    // Encrypt password in the project setting
    this->account_.EncryptPassword();
    this->snmp_configuration_.EncryptPassword();
    this->snmp_trap_configuration_.EncryptPassword();

    return act_status;
  }

  /**
   * @brief Decrypt the encrypted password
   *
   * @return QString
   */
  ACT_STATUS DecryptPassword() {
    ACT_STATUS_INIT();

    // Decrypt scan_ip_range password
    QList<ActScanIpRangeEntry> new_scan_ip_ranges;
    for (auto scan_ip_range : this->GetScanIpRanges()) {
      scan_ip_range.DecryptPassword();
      new_scan_ip_ranges.append(scan_ip_range);
    }
    this->SetScanIpRanges(new_scan_ip_ranges);

    // Decrypt password in the project setting
    this->account_.DecryptPassword();
    this->snmp_configuration_.DecryptPassword();
    this->snmp_trap_configuration_.DecryptPassword();

    return act_status;
  }
};

/**
 * @brief The ACT project class
 *
 */
class ActOnlineTopology : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActMapTopology, topology, Topology);
  ACT_JSON_OBJECT(ActTopologyMappingResult, mapping_result, MappingResult);

 public:
  /**
   * @brief Construct a new Act Online Topology object
   *
   */
  ActOnlineTopology() {}

  /**
   * @brief Construct a new Act Online Topology object
   *
   * @param topology
   * @param mapping_result
   */
  ActOnlineTopology(const ActMapTopology &topology, const ActTopologyMappingResult &mapping_result)
      : ActOnlineTopology() {
    this->SetTopology(topology);
    this->SetMappingResult(mapping_result);
  }
};

class ActSFPPair : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActMonitorFiberCheckEntry, source, Source);
  ACT_JSON_OBJECT(ActMonitorFiberCheckEntry, target, Target);
};

class ActSFPList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActSFPPair, SFP_list, SFPList);
};

/**
 * @brief The ACT project class
 *
 */
class ActProject : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, act_version, ActVersion);
  ACT_JSON_FIELD(QString, data_version, DataVersion);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_ENUM(ActProjectModeEnum, project_mode, ProjectMode);
  ACT_JSON_ENUM(ActQuestionnaireModeEnum, questionnaire_mode, QuestionnaireMode);
  ACT_JSON_ENUM(ActServiceProfileForLicenseEnum, profile, Profile);
  ACT_JSON_FIELD(QString, uuid, UUID);
  ACT_JSON_FIELD(qint64, user_id, UserId);  // Created user id
  ACT_JSON_FIELD(QString, organization_id,
                 OrganizationId);  ///< The organization id is the same as user id in the current version
  ACT_JSON_FIELD(quint64, created_time, CreatedTime);
  ACT_JSON_FIELD(quint64, last_modified_time, LastModifiedTime);
  ACT_JSON_QT_SET_OBJECTS(ActDevice, devices, Devices);
  ACT_JSON_QT_SET_OBJECTS(ActLink, links, Links);
  ACT_JSON_QT_SET_OBJECTS(ActStream, streams, Streams);
  ACT_JSON_OBJECT(ActCycleSetting, cycle_setting, CycleSetting);
  ACT_JSON_OBJECT(ActComputedResult, computed_result, ComputedResult);
  ACT_JSON_OBJECT(ActProjectSetting, project_setting, ProjectSetting);
  ACT_JSON_OBJECT(ActTopologySetting, topology_setting, TopologySetting);
  ACT_JSON_OBJECT(ActDeviceConfig, device_config, DeviceConfig);
  ACT_JSON_OBJECT(ActTrafficDesign, traffic_design, TrafficDesign);
  ACT_JSON_OBJECT(ActManufactureResult, manufacture_result, ManufactureResult);
  ACT_JSON_QT_DICT(QMap, QString, quint32, sfp_counts, SFPCounts);
  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActSkuQuantity, sku_quantities_map, SkuQuantitiesMap);
  ACT_JSON_FIELD(qint64, platform_project_id, PlatformProjectId);
  ACT_JSON_FIELD(qint64, activate_baseline_id, ActivateBaselineId);
  ACT_JSON_QT_SET(qint64, design_baseline_ids, DesignBaselineIds);
  ACT_JSON_QT_SET(qint64, operation_baseline_ids, OperationBaselineIds);

 public:
  qint64 last_assigned_device_id_;
  qint64 last_assigned_stream_id_;
  qint64 last_assigned_link_id_;
  quint16 last_assigned_vlan_id_;
  qint64 last_assigned_scan_ip_range_id_;
  qint64 last_assigned_rstp_id_;
  qint64 last_assigned_traffic_time_slot_id_;
  qint64 last_assigned_traffic_per_stream_priority_id_;
  qint64 last_assigned_traffic_application_id_;
  qint64 last_assigned_traffic_stream_id_;
  QList<ActDevice> broadcast_search_devices_;
  QSet<ActRoutingResult> routing_results_;
  QList<QString> key_order_;
  quint64 last_assigned_mac_addresses_;  // [feat:2495] last assigned MAC addresses
  QSet<quint32> used_ip_addresses_;      // Already used IP addresses
  quint32 last_assigned_ip_;
  quint32 last_start_ip_;
  ActOnlineTopology online_topology_;

  QSet<ActCoordinate> used_coordinates_;

  // For monitor
  ActSFPList sfp_list_;
  QSet<ActMonitorFiberCheckEntry> fiber_check_entries_;
  ActSourceDevice monitor_endpoint_;
  QSet<qint64> keep_connect_status_devices_;

  /**
   * @brief Construct a new Act Project object
   *
   */
  ActProject() {
    this->id_ = -1;
    this->project_mode_ = ActProjectModeEnum::kDesign;
    this->questionnaire_mode_ = ActQuestionnaireModeEnum::kNone;
    this->profile_ = ActServiceProfileForLicenseEnum::kSelfPlanning;
    this->data_version_ = ACT_PROJECT_DATA_VERSION;
    this->last_assigned_device_id_ = 0;
    this->last_assigned_stream_id_ = 0;
    this->last_assigned_vlan_id_ = 0;
    this->last_assigned_link_id_ = 0;
    this->last_assigned_scan_ip_range_id_ = 0;
    this->last_assigned_rstp_id_ = 0;
    this->last_assigned_traffic_time_slot_id_ = 0;
    this->last_assigned_traffic_per_stream_priority_id_ = 0;
    this->last_assigned_traffic_application_id_ = 0;
    this->last_assigned_traffic_stream_id_ = 0;
    this->created_time_ = QDateTime::currentMSecsSinceEpoch();
    this->last_modified_time_ = QDateTime::currentMSecsSinceEpoch();

    this->uuid_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
    this->last_assigned_mac_addresses_ = 0;
    this->last_start_ip_ = 0;
    this->last_assigned_ip_ = 0;
    this->last_assigned_vlan_id_ = this->GetVlanRange().GetMin();
    this->platform_project_id_ = -1;
    this->activate_baseline_id_ = -1;

    this->key_order_.append(QList<QString>({QString("Id"),
                                            QString("ActVersion"),
                                            QString("DataVersion"),
                                            QString("Description"),
                                            QString("ProjectMode"),
                                            QString("QuestionnaireMode"),
                                            QString("Profile"),
                                            QString("UUID"),
                                            QString("UserId"),
                                            QString("OrganizationId"),
                                            QString("CreatedTime"),
                                            QString("LastModifiedTime"),
                                            QString("ProjectSetting"),
                                            QString("Devices"),
                                            QString("Links"),
                                            QString("Streams"),
                                            QString("DeviceConfig"),
                                            QString("TopologySetting"),
                                            QString("CycleSetting"),
                                            QString("ComputedResult"),
                                            QString("TrafficDesign"),
                                            QString("SFPCounts"),
                                            QString("ManufactureResult"),
                                            QString("SkuQuantitiesMap"),
                                            QString("PlatformProjectId"),
                                            QString("ActivateBaselineId"),
                                            QString("DesignBaselineIds"),
                                            QString("OperationBaselineIds")}));
  }

  /**
   * @brief Construct a new Act Project object
   *
   * @param id
   */
  ActProject(const qint64 &id) : ActProject() { this->id_ = id; }

  // /**
  //  * @brief Increase the data version on the project, which means the data has been changed
  //  *
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS IncreaseDataVersion();

  /**
   * @brief Set the Project Name object
   *
   * @param project_name
   * @return ACT_STATUS
   */
  ACT_STATUS SetProjectName(const QString &project_name);

  /**
   * @brief Get the Project Name object
   *
   * @return QString
   */
  QString GetProjectName() const;

  /**
   * @brief Set the Algorithm Configuration object
   *
   * @param algorithm_configuration
   * @return ACT_STATUS
   */
  ACT_STATUS SetAlgorithmConfiguration(ActAlgorithmConfiguration &algorithm_configuration);

  /**
   * @brief Get the Algorithm Configuration object
   *
   * @return QString
   */
  ActAlgorithmConfiguration GetAlgorithmConfiguration() const;

  /**
   * @brief Set the Vlan Range object
   *
   * @param vlan_range
   * @return ACT_STATUS
   */
  ACT_STATUS SetVlanRange(ActVlanRange &vlan_range);

  /**
   * @brief Get the Vlan Range object
   *
   * @return ActVlanRange
   */
  ActVlanRange GetVlanRange() const;

  /**
   * @brief Set the Cfg Wizard Setting object
   *
   * @param cfg_wizard_setting
   * @return ACT_STATUS
   */
  ACT_STATUS SetCfgWizardSetting(ActCfgWizardSetting &cfg_wizard_setting);

  /**
   * @brief Get the Cfg Wizard Setting object
   *
   * @return ActCfgWizardSetting
   */
  ActCfgWizardSetting GetCfgWizardSetting() const;

  /**
   * @brief Set the Device account object
   *
   * @param account
   * @return ACT_STATUS
   */
  ACT_STATUS SetAccount(ActDeviceAccount &account);

  /**
   * @brief Get Device account object
   *
   * @return ActDeviceAccount
   */
  ActDeviceAccount GetAccount() const;

  /**
   * @brief Set the Netconf Configuration object
   *
   * @param netconf_configuration
   * @return ACT_STATUS
   */
  ACT_STATUS SetNetconfConfiguration(ActNetconfConfiguration &netconf_configuration);

  /**
   * @brief Get the Netconf Configuration object
   *
   * @return ActNetconfConfiguration
   */
  ActNetconfConfiguration GetNetconfConfiguration() const;

  /**
   * @brief Set the Snmp Configuration object
   *
   * @param snmp_configuration
   * @return ACT_STATUS
   */
  ACT_STATUS SetSnmpConfiguration(ActSnmpConfiguration &snmp_configuration);

  /**
   * @brief Get the Snmp Configuration object
   *
   * @return ActSnmpConfiguration
   */
  ActSnmpConfiguration GetSnmpConfiguration() const;

  /**
   * @brief Set the Restful Configuration object
   *
   * @param restful_configuration
   * @return ACT_STATUS
   */
  ACT_STATUS SetRestfulConfiguration(ActRestfulConfiguration &restful_configuration);

  /**
   * @brief Get the Restful Configuration object
   *
   * @return ActRestfulConfiguration
   */
  ActRestfulConfiguration GetRestfulConfiguration() const;

  /**
   * @brief Set the Traffic Type To Priority Code Point Mapping object
   *
   * @param traffic_type_to_priority_code_point_mapping
   * @return ACT_STATUS
   */
  ACT_STATUS SetTrafficTypeToPriorityCodePointMapping(
      ActTrafficTypeToPriorityCodePointMapping &traffic_type_to_priority_code_point_mapping);

  /**
   * @brief Get the Traffic Type To Priority Code Point Mapping object
   *
   * @return ActTrafficTypeToPriorityCodePointMapping
   */
  ActTrafficTypeToPriorityCodePointMapping GetTrafficTypeToPriorityCodePointMapping() const;

  /**
   * @brief Set the Priority Code Point To Queue Mapping object
   *
   * @param priority_code_point_to_queue_mapping
   * @return ACT_STATUS
   */
  ACT_STATUS SetPriorityCodePointToQueueMapping(QMap<quint8, QSet<quint8>> &priority_code_point_to_queue_mapping);

  /**
   * @brief Get the Priority Code Point To Queue Mapping object
   *
   * @return QMap<quint8, QSet<quint8>>
   */
  QMap<quint8, QSet<quint8>> GetPriorityCodePointToQueueMapping() const;

  /**
   * @brief Set the Scan Ranges object
   *
   * @param scan_ip_ranges
   * @return ACT_STATUS
   */
  ACT_STATUS SetScanIpRanges(QList<ActScanIpRangeEntry> &scan_ip_ranges);

  /**
   * @brief Get the Scan Ranges object
   *
   * @return QList<ActScanIpRangeEntry>
   */
  QList<ActScanIpRangeEntry> GetScanIpRanges() const;

  /**
   * @brief Set the project start IP
   *
   * @param start_ip
   * @return ACT_STATUS
   */
  ACT_STATUS SetProjectStartIp(QString &start_ip);

  /**
   * @brief Get project start IP
   *
   * @return ActDeviceAccount
   */
  ActIpv4 GetProjectStartIp() const;

  /**
   * @brief Get project device IP
   *
   * @return device IP
   */
  QString GetDeviceIp(qint64 &device_id) const;

  /**
   * @brief Set the Snmp Trap Configuration object
   *
   * @param snmp_trap_configuration
   * @return ACT_STATUS
   */
  ACT_STATUS SetSnmpTrapConfiguration(ActSnmpTrapConfiguration &snmp_trap_configuration);

  /**
   * @brief Get the Snmp Trap Configuration object
   *
   * @return ActSnmpConfiguration
   */
  ActSnmpTrapConfiguration GetSnmpTrapConfiguration() const;

  /**
   * @brief Set the Monitor Configuration object
   *
   * @param monitor_configuration
   * @return ACT_STATUS
   */
  ACT_STATUS SetMonitorConfiguration(ActMonitorConfiguration &monitor_configuration);

  /**
   * @brief Get the Monitor Configuration object
   *
   * @return ActSnmpConfiguration
   */
  ActMonitorConfiguration GetMonitorConfiguration() const;

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActProject &obj) {
    return qHash(obj.id_, 0);  // arbitrary value is 0
  }

  /**
   * @brief The equal operator
   *
   * @param source The copied source
   */
  friend bool operator==(const ActProject &x, const ActProject &y) { return x.id_ == y.id_; }

  /**
   * @brief [feat:1662] Remove the password related fields in the project
   *
   * @return ACT_STATUS
   */
  ACT_STATUS HidePassword();

  /**
   * @brief Encrypt the password field
   *
   * @return QString
   */
  ACT_STATUS EncryptPassword();

  /**
   * @brief Decrypt the encrypted password
   *
   * @return QString
   */
  ACT_STATUS DecryptPassword();

  /**
   * @brief Get the Host Pair list From End Station Interfaces object
   *
   * host_pairs:
   *  <t1, l1>
   *  <t1, l2>
   *  <t1, l3>
   *  <t2, l2>
   *
   * @param host_pairs
   * @param stream_id
   * @return ACT_STATUS
   */
  void GetHostPairsFromEndStationInterfaces(QList<ActHostPair> &host_pairs, const ActStream &stream) const;

  /**
   * @brief Get the Device By Ip value
   *
   * @param device
   * @param ip
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceByIp(const QString &ip, ActDevice &device) const;

  /**
   * @brief Get the Device Id By Ip value (traverse all devices)
   *
   * @param device_id
   * @param ip
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceIdByIp(qint64 &device_id, const QString &ip) const;

  /**
   * @brief Get the Device object By Id value
   *
   * @param device
   * @param device_id
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceById(ActDevice &device, const qint64 &device_id) const;

  /**
   * @brief Get the Stream object By Id value
   *
   * @param stream
   * @param stream_id
   * @return ACT_STATUS
   */
  ACT_STATUS GetStreamById(ActStream &stream, const qint64 &stream_id) const;

  ACT_STATUS GetStreamsByTalkerInterface(QSet<ActStream> &streams, const qint64 &device_id, const qint64 &intf_id);

  ACT_STATUS GetStreamsByListenerInterface(QSet<ActStream> &streams, const qint64 &device_id, const qint64 &intf_id);

  // ACT_STATUS RemoveStreamsByListenerInterface(QSet<ActStream>& streams, const qint64& device_id,
  //                                             const qint64& intf_id) {
  //   ACT_STATUS_INIT();
  //   for (ActStream stream : this->GetStreams()) {
  //     QList<ActListener> listeners = stream.GetListeners();
  //     bool updated = false;
  //     for (QList<ActListener>::iterator listener_iterator = listeners.begin(); listener_iterator != listeners.end();)
  //     {
  //       ActListener listener = *listener_iterator;
  //       const qint64 listener_device_id = listener.GetEndStationInterface().GetDeviceId();
  //       const qint64 listener_intf_id = listener.GetEndStationInterface().GetInterfaceId();

  //       if (listener_device_id == device_id && listener_intf_id == intf_id) {
  //         listener_iterator = listeners.erase(listener_iterator);
  //         updated = true;
  //       } else {
  //         listener_iterator++;
  //       }
  //     }

  //     if (updated) {
  //       stream.SetListeners(listeners);
  //       streams.insert(stream);
  //     }
  //   }

  //   return act_status;
  // }

  /**
   * @brief Get the Link object By Id value
   *
   * @param link
   * @param link_id
   * @return ACT_STATUS
   */
  ACT_STATUS GetLinkById(ActLink &link, const qint64 &link_id) const;

  /**
   * @brief Get the Link By Device Ids list (traverse all links)
   *
   * @param act_link
   * @param ids
   * @return ACT_STATUS
   */
  ACT_STATUS GetLinkByDeviceIds(ActLink &act_link, const QPair<qint64, qint64> &ids) const;

  /**
   * @brief Get the Link By Device Ids list (traverse all links)
   *
   * @param act_link
   * @param ids
   * @return ACT_STATUS
   */
  ACT_STATUS GetLinkByInterfaceId(ActLink &act_link, const qint64 &device_id, const qint64 &interface_id) const;

  /**
   * @brief Get the Available Priority Code Points For a Traffic Type (Stream)
   *
   * @param priority_code_point_set
   * @param traffic_type
   * @return ACT_STATUS
   */
  ACT_STATUS GetAvailablePriorityCodePointsForTrafficType(QSet<quint8> &priority_code_point_set,
                                                          const ActStreamTrafficTypeEnum &traffic_type) const;

  /**
   * @brief Get the Available Priority Code Points For a Traffic Type (Time Slot)
   *
   * @param priority_code_point_set
   * @param traffic_type
   * @return ACT_STATUS
   */
  ACT_STATUS GetAvailablePriorityCodePointsForTrafficType(QSet<quint8> &priority_code_point_set,
                                                          const ActTimeSlotTrafficTypeEnum &traffic_type) const;

  ACT_STATUS GetAvailableQueuesForPriorityCodePoints(QSet<quint8> &available_queues,
                                                     const QSet<quint8> &priority_code_point_set) const;

  /**
   * @brief Get the Available Queues For Traffic Type object (Stream)
   *
   * @param available_queues
   * @param traffic_type
   * @return ACT_STATUS
   */
  ACT_STATUS GetAvailableQueuesForTrafficType(QSet<quint8> &available_queues,
                                              const ActStreamTrafficTypeEnum &traffic_type) const;

  /**
   * @brief Get the Available Queues For Traffic Type object (Time Slot)
   *
   * @param available_queues
   * @param traffic_type
   * @return ACT_STATUS
   */
  ACT_STATUS GetAvailableQueuesForTrafficType(QSet<quint8> &available_queues,
                                              const ActTimeSlotTrafficTypeEnum &traffic_type) const;

  void SetUsedInterface(const qint64 &device_id, const qint64 &interface_id);

  /**
   * @brief Auto assign IP Address
   *
   * @tparam T
   * @param item
   * @return ACT_STATUS
   */
  template <class T>
  ACT_STATUS AutoAssignIP(T &item) {
    ACT_STATUS_INIT();

    // Get the start ip from project setting
    ActIpv4 start_ip = this->GetProjectSetting().GetProjectStartIp();
    quint32 start_ip_int = 0;
    act_status = ActIpv4::AddressStrToNumber(start_ip.GetIpAddress(), start_ip_int);

    quint32 new_ip_int = start_ip_int;

    // Check if new_ip_int already exists in used_ip_addresses_ or is a .255 address
    while (used_ip_addresses_.contains(new_ip_int) || (new_ip_int & 0xFF) == 0xFF || new_ip_int == 0) {
      new_ip_int++;
    }

    // Convert the new IP address integer to string
    QString new_ip_string;
    ActIpv4::AddressNumberToStr(new_ip_int, new_ip_string);

    // Update last_assigned_ip_ and used_ip_addresses_
    last_assigned_ip_ = new_ip_int;

    // Don't add the new IP address to used_ip_addresses_
    // because the used ip list will be update after the device created/updated
    // used_ip_addresses_.insert(new_ip_int);

    // Assign the results to the passed reference parameters
    ActIpv4 ip;
    ip.AutoAssignIPSubnetMask(new_ip_string);
    item.SetIpv4(ip);

    return act_status;
  }

  ACT_STATUS UpdateSFPList(QMap<qint64, bool> monitor_link_status);
};

class ActSimpleProject : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, project_name, ProjectName);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_ENUM(ActProjectModeEnum, project_mode, ProjectMode);
  ACT_JSON_ENUM(ActQuestionnaireModeEnum, questionnaire_mode, QuestionnaireMode);
  ACT_JSON_ENUM(ActServiceProfileForLicenseEnum, profile, Profile);
  ACT_JSON_FIELD(QString, uuid, UUID);
  ACT_JSON_FIELD(qint64, user_id, UserId);  // Created user id
  ACT_JSON_FIELD(QString, organization_id,
                 OrganizationId);  ///< The organization id is the same as user id in the current version

  ACT_JSON_FIELD(quint64, created_time, CreatedTime);
  ACT_JSON_FIELD(quint64, last_modified_time, LastModifiedTime);

  ACT_JSON_ENUM(ActProjectStatusEnum, project_status, ProjectStatus);

 public:
  /**
   * @brief Construct a new Act Simple Project object
   *
   */
  ActSimpleProject() {}

  /**
   * @brief Construct a new Act Simple Project object
   *
   * @param project
   */
  ActSimpleProject(ActProject &project) {
    this->id_ = project.GetId();
    this->project_name_ = project.GetProjectName();
    this->created_time_ = project.GetCreatedTime();
    this->description_ = project.GetDescription();
    this->last_modified_time_ = project.GetLastModifiedTime();
    this->project_mode_ = project.GetProjectMode();
    this->profile_ = project.GetProfile();
    this->questionnaire_mode_ = project.GetQuestionnaireMode();

    this->uuid_ = project.GetUUID();
    this->user_id_ = project.GetUserId();

    this->project_status_ = ActProjectStatusEnum::kIdle;
  }

  /**
   * @brief Construct a new Act Simple Project object
   *
   * @param id
   */
  ActSimpleProject(const qint64 &id) : id_(id) {}

  /**
   * @brief The equal operator
   *
   * @param source The copied source
   */
  friend bool operator==(const ActSimpleProject &x, const ActSimpleProject &y) { return x.id_ == y.id_; }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActSimpleProject &obj) {
    return qHash(obj.id_, 0);  // arbitrary value is 0
  }
};

class ActSimpleProjectSet : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActSimpleProject, simple_project_set, SimpleProjectSet);
};

class ActProjectCreateParam : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(QString, project_name, ProjectName);
  ACT_JSON_ENUM(ActProjectModeEnum, project_mode, ProjectMode);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_FIELD(qint64, user_id, UserId);
  ACT_JSON_ENUM(ActServiceProfileForLicenseEnum, profile, Profile);

 public:
  ActProjectCreateParam() {
    this->profile_ = ActServiceProfileForLicenseEnum::kSelfPlanning;
    this->user_id_ = -1;
  }
};

class ActProjectCopyParam : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(QString, project_name, ProjectName);

 public:
  ActProjectCopyParam() {}
};
