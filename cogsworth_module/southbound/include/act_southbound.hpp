/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_SOUTHBOUND_HPP
#define ACT_SOUTHBOUND_HPP

#include <QDebug>
#include <QMutex>
#include <QString>

#include "act_auto_probe_warning.hpp"
#include "act_device.hpp"
#include "act_feature_common.hpp"
#include "act_json.hpp"
#include "act_lldp_data.hpp"
#include "act_monitor_data.hpp"
#include "act_profiles.hpp"
#include "act_project.hpp"
#include "act_scan_ip_range.hpp"
#include "act_snmp_result.hpp"
#include "act_southbound_result.hpp"
#include "act_status.hpp"
#include "deploy_entry/act_deploy_table.hpp"
#include "deploy_entry/act_vlan_static_entry.hpp"
#include "topology/act_auto_scan_result.hpp"

#define ACT_VENDOR_ID_MOXA (8691)    ///< The vendor_id of the moxa
#define ACT_VENDOR_MOXA "MOXA"       ///< The vendor of the moxa
#define ACT_PING_TIMEOUT (1000)      ///< The timeout(ms, 1 second) of the ping
#define ACT_PING_TIMEOUT_SECOND (1)  ///< The timeout of the ping for linux second
#define ACT_PING_REPEAT_TIMES (3)    ///< The repeat times of the ping
#define ACT_ARP_ENTRY_DYNAMIC "3"
#define ACT_ARP_ENTRY_STATIC "4"
#define ACT_AUTOSCAN_ASSIGN_INTERFACE_NAME "Port"  ///< The Interface Name by auto_scan assign

const QSet<QString> kStaticYangRevisions = {
    //   "IEEE802dot1qPreemption20180910",
    //  "IEEE802dot1qBridge20180307"
};

// enum class ActArpEntryTypeEnum { kOther = 1, kInvalidated = 2, kDynamic = 3, kStatic = 4 };

/**
 * @brief The ACT Southbound module class
 *
 */
class ActSouthbound {
  Q_GADGET

  // for thread
  QMutex mutex_;

  ACT_JSON_FIELD(bool, stop_flag, StopFlag);         ///< StopFlag item
  ACT_JSON_OBJECT(ActProfiles, profiles, Profiles);  ///< Profiles item

 private:
  QSet<QString> probe_success_oid_cache_;
  QSet<QString> probe_yang_cache_;

  const QString restful_str_ = kActConnectProtocolTypeEnumMap.key(ActConnectProtocolTypeEnum::kRESTful);
  const QString snmp_str_ = kActConnectProtocolTypeEnumMap.key(ActConnectProtocolTypeEnum::kSNMP);
  const QString netconf_str_ = kActConnectProtocolTypeEnumMap.key(ActConnectProtocolTypeEnum::kNETCONF);
  const QString moxa_command_str_ = kActConnectProtocolTypeEnumMap.key(ActConnectProtocolTypeEnum::kMOXAcommand);

  /**
   * @brief Get the Method Sequence object
   *
   * @param feat_str
   * @param feat_sub_item
   * @param result_method_sequence
   * @return ACT_STATUS
   */
  ACT_STATUS GetMethodSequence(const QString &feat_str, const ActFeatureSubItem &feat_sub_item,
                               QList<QString> &result_method_sequence);

  /**
   * @brief Update device ICMP status thread
   *
   * @param devices
   * @param result_update_icmp_devices
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDeviceIcmpStatusThread(ActDevice device, QList<ActDevice> &result_update_icmp_devices);

  /**
   * @brief Insert the probe success cache
   *
   * @param method
   * @return ACT_STATUS
   */

  ACT_STATUS InsertMethodToCache(const ActFeatureMethod &feat_method);

  /**
   * @brief Check the method has cache
   *
   * @param method
   * @return true
   * @return false
   */
  bool CheckMethodHasCache(const ActFeatureMethod &feat_method);

  /**
   * @brief Check the method protocols status
   *
   * @param device
   * @param feat_method
   * @return ACT_STATUS
   */
  ACT_STATUS CheckMethodProtocolsStatus(const ActDevice &device, const ActFeatureMethod &feat_method);

  /**
   * @brief Format the Uptime from centisecond to string
   *
   * @param uptime_centisecond
   * @return QString
   */
  QString FormatUptime(qint64 uptime_centisecond);

  /**
   * @brief Transfer from VlanStaticEntries to VlanPortTypeEntries
   *
   * @param device
   * @param vlan_static_entries
   * @param vlan_port_type_entries
   * @return ACT_STATUS
   */
  ACT_STATUS FromVlanStaticToVlanPortType(const ActDevice &device, const QSet<ActVlanStaticEntry> &vlan_static_entries,
                                          QSet<ActVlanPortTypeEntry> &vlan_port_type_entries);

  /**
   * @brief Find the Device by LLDP ChassisID
   *
   * @param devices
   * @param chassis_id_subtype
   * @param chassis_id
   * @param result_device
   * @return ACT_STATUS
   */
  ACT_STATUS FindDeviceByLldpChassisId(const QList<ActDevice> &devices, const qint64 &chassis_id_subtype,
                                       const QString &chassis_id, ActDevice &result_device);

  /**
   * @brief Find & Update remote DeviceInterface by MAC
   *
   * @param rem_dev
   * @param update_device_set
   * @param result_rem_interface_id
   * @return ACT_STATUS
   */
  ACT_STATUS FindAndUpdateRemoteDeviceInterface(const ActDevice &rem_dev, QSet<ActDevice> &update_device_set,
                                                qint64 &result_rem_interface_id);

  /**
   * @brief Check Configuration failed ErrorHandler
   *
   * @param called_func
   * @param device
   * @param item
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigurationCheckErrorHandler(QString called_func, const ActDevice &device, const QString &item);

  /**
   * @brief Device connection status false ErrorHandler
   *
   * @param called_func
   * @param device
   * @param protocol_enum
   * @return ACT_STATUS
   */
  ACT_STATUS DeviceConnectionFalseErrorHandler(QString called_func, const ActDevice &device,
                                               const ActConnectProtocolTypeEnum &protocol_enum);

  /**
   * @brief Set the Config Fail ErrorHandler object
   *
   * @param device
   * @param protocol_enum
   * @param item
   * @return ACT_STATUS
   */
  ACT_STATUS SetConfigFailProtocolErrorHandler(QString called_func, const ActDevice &device,
                                               const ActConnectProtocolTypeEnum &protocol_enum, const QString &item);

  /**
   * @brief Get the Data empty Error Handler object
   *
   * @param called_func
   * @param device
   * @param item
   * @return ACT_STATUS
   */
  ACT_STATUS GetDataEmptyFailErrorHandler(QString called_func, const ActDevice &device, const QString &item);

  /**
   * @brief Generate Data fail Error Handler object
   *
   * @param called_func
   * @param device
   * @param item
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateDataFailErrorHandler(QString called_func, const ActDevice &device, const QString &item);

  /**
   * @brief Methods Empty Error Handler object
   *
   * @param called_func
   * @param device
   * @param item
   * @return ACT_STATUS
   */
  ACT_STATUS MethodsEmptyErrorHandler(QString called_func, const ActDevice &device, const QString &item);

  /**
   * @brief SequenceItem not found Error Handler object
   *
   * @param called_func
   * @param sequence_item
   * @param feat
   * @return ACT_STATUS
   */
  ACT_STATUS SequenceItemNotFoundErrorHandler(QString called_func, const QString &sequence_item, const QString &feat);

  /**
   * @brief Error Handler for the method not implemented object
   *
   * @param called_func
   * @param device
   * @param feat
   * @param method
   * @return ACT_STATUS
   */
  ACT_STATUS MethodNotImplementedErrorHandler(QString called_func, const ActDevice &device, const QString &feat,
                                              const QString &method);

  /**
   * @brief Error Handler for the protocol not found at method
   *
   * @param called_func
   * @param device
   * @param feat
   * @param method
   * @param protocol
   * @return ACT_STATUS
   */
  ACT_STATUS NotFoundProtocolErrorHandler(QString called_func, const ActDevice &device, const QString &feat,
                                          const QString &method, const QString &protocol);

  /**
   * @brief Error Handler for the probe failed
   *
   * @param called_func
   * @param device
   * @param feat
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeFailErrorHandler(QString called_func, const ActDevice &device, const QString &feat);

  /**
   * @brief Error Handler for the access failed
   *
   * @param called_func
   * @param device
   * @param feat
   * @param act_status
   * @return ACT_STATUS
   */
  ACT_STATUS AccessFailErrorHandler(QString called_func, const ActDevice &device, const QString &feat,
                                    ACT_STATUS &act_status);

  /**
   * @brief Not found Method ErrorHandler object
   *
   * @param called_func
   * @param method
   * @return ACT_STATUS
   */
  ACT_STATUS NotFoundMethodErrorHandler(QString called_func, ActActionMethod method);

  /*************
   *  Config   *
   *************/

  // /**
  //  * @brief Generate the AddRemoveVlanTable
  //  *
  //  * @param device
  //  * @param vlan_static_table
  //  * @param add_remove_vlan_static_table
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS GenerateAddRemoveVlanTable(const ActDevice &device, const ActVlanStaticTable &vlan_static_table,
  //                                       ActAddRemoveVlanStaticTable &add_remove_vlan_static_table);

  ACT_STATUS GenerateEditVlanTable(const ActDevice &device, const ActVlanStaticTable &vlan_static_table,
                                   ActEditVlanStaticTable &edit_vlan_static_table);

  /**
   * @brief Generate the RemoveStaticForwardTable by the VLAN ID set
   *
   * @param device
   * @param vlan_id_set
   * @param unicast
   * @param remove_static_forward_table
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateRemoveStaticForwardTableByVlans(const ActDevice &device, const QSet<qint32> &vlan_id_set,
                                                     const bool &unicast,
                                                     ActStaticForwardTable &remove_static_forward_table);

  /**
   * @brief Generate the RemoveStaticForwardTable by the VLANTable
   *
   * @param device
   * @param vlan_config_table
   * @param unicast
   * @param remove_static_forward_table
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateRemoveStaticForwardTableByVlanTable(const ActDevice &device, const ActVlanTable &vlan_config_table,
                                                         const bool &unicast,
                                                         ActStaticForwardTable &remove_static_forward_table);

  /**
   * @brief Generate the target config VLANPortTypeTable
   *
   * @param device
   * @param vlan_port_type_table
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateTargetConfigVLANPortTypeTable(const ActDevice &device, ActVlanPortTypeTable &vlan_port_type_table);

  /**
   * @brief Generate the target config PortVlanTable
   *
   * @param device
   * @param port_vlan_table
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateTargetConfigPortVlanTable(const ActDevice &device, ActPortVlanTable &port_vlan_table);

  /**
   * @brief Check device's VlanEntries exist by the VLAN ID set
   *
   * @param device
   * @param vlan_id_set
   * @return ACT_STATUS
   */
  ACT_STATUS CheckDeviceVlanEntriesExistByVlans(const ActDevice &device, const QSet<qint32> &vlan_id_set);

  /**
   * @brief Generate the AddRemoveStaticForwardTable
   *
   * @param device
   * @param static_forward_table
   * @param unicast
   * @param add_remove_static_forward_table
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateAddRemoveStaticForwardTable(const ActDevice &device,
                                                 const ActStaticForwardTable &static_forward_table, const bool &unicast,
                                                 ActAddRemoveStaticForwardTable &add_remove_static_forward_table);

  /**
   * @brief Generate the StreamPriorityIngressTable
   *
   * @param device
   * @param stad_port_table
   * @param add_remove_stad_port_table
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateAddRemoveStreamPriorityIngressTable(const ActDevice &device,
                                                         const ActStadPortTable &stad_port_table,
                                                         ActAddRemoveStadPortTable &add_remove_stad_port_table);

  /**
  Find the idle StreamPriorityPort's port_index
   *
   * @param ingress_index_max
   * @param stad_port_entries
   * @param target_stad_port_entry
   * @param ingress_index
   * @return ACT_STATUS
   */
  ACT_STATUS FindIdleStadPortIndex(const quint16 &ingress_index_max, const QSet<ActStadPortEntry> &stad_port_entries,
                                   const ActStadPortEntry &target_stad_port_entry, qint32 &ingress_index);

 public:
  /**
   * @brief Construct a new Act Southbound object
   *
   */
  /**
   * @brief Construct a new Act Southbound object
   *
   */
  ActSouthbound() { this->stop_flag_ = false; };

  /**
   * @brief Construct a new Act Southbound object
   *
   * @param profiles
   */

  ActSouthbound(const ActProfiles &profiles) : ActSouthbound() { this->profiles_ = profiles; }

  /**
   * @brief Init the SNMP global resource
   *
   * @return ACT_STATUS
   */
  ACT_STATUS InitSnmpResource();

  /**
   * @brief Clear the SNMP global resource
   *
   * @return ACT_STATUS
   */
  ACT_STATUS ClearSnmpResource();

  /**
   * @brief Init probe cache
   *
   * @return ACT_STATUS
   */
  ACT_STATUS InitProbeCache();

  /**
   * @brief Generate the links by lldp informations
   *
   * @param device
   * @param alive_devices
   * @param scan_link_result
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateLinkSetBylldpInfo(const ActDevice &device, const QList<ActDevice> &alive_devices,
                                       ActScanLinksResult &scan_link_result);

  /**
   * @brief Generate the links by MAC table
   *
   * @param device
   * @param alive_devices
   * @param exist_links
   * @param scan_link_result
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateLinkSetByMacTable(const ActDevice &device, const QList<ActDevice> &alive_devices,
                                       const QSet<ActLink> &exist_links, const QMap<QString, QString> &ip_mac_table,
                                       ActScanLinksResult &scan_link_result);

  /**
   * @brief Ping IpAddress
   *
   * @param ip
   * @return ACT_STATUS
   */
  ACT_STATUS PingIpAddress(const QString &ip, const quint8 &times);

  // /**
  //  * @brief Set the Device SysName object
  //  *
  //  * @param device
  //  * @param sys_name
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS SetDeviceName(const ActDevice &device);

  /**
   * @brief Update devices ICMP Status
   *
   * @param devices
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDevicesIcmpStatus(QList<ActDevice> &devices);

  /**
   * @brief Scan Devices by ScanIpRange
   *
   * @param scan_ip_ranges
   * @param result_devices
   * @return ACT_STATUS
   */
  ACT_STATUS ScanDevicesByScanIpRange(const QList<ActScanIpRangeEntry> &scan_ip_ranges,
                                      QSet<ActDevice> &result_devices);

  // /**
  //  * @brief Get the Device Identify Info object
  //  *
  //  * @param device
  //  * @param feature_profile
  //  * @param device_identify_info
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS GetIdentifyDeviceInfo(const ActDevice &device, const ActFeatureProfile &feature_profile,
  //                                  ActIdentifyDeviceInfo &device_identify_info);

  /**
   * @brief Clear the ARP cache
   *
   * @return ACT_STATUS
   */
  ACT_STATUS ClearArpCache();

  /**
   * @brief Delete the Localhost Arp Table entry object
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteLocalhostArpEntry(const ActDevice &device);

  /**
   * @brief Set the Localhost Arp Table object
   *
   * @param device
   * @param localhost_ip
   * @return ACT_STATUS
   */
  ACT_STATUS SetLocalhostArpTable(const ActDevice &device, const QString &localhost_ip);

  // /**
  //  * @brief Get the Device If Operational Status object
  //  *
  //  * @param device
  //  * @param result_port_speed_map
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS GetDeviceIfOperationalStatus(const ActDevice &device, QMap<qint64, quint8> &result_port_speed_map);

  // /**
  //  * @brief Get the Device If Propagation Delays object
  //  *
  //  * @param device
  //  * @param result_port_speed_map
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS GetDeviceIfPropagationDelays(const ActDevice &device, QMap<qint64, quint32>
  // &result_port_prop_delay_map);

  // /**
  //  * @brief Get the Supported List Max object
  //  *
  //  * @param device
  //  * @param port_id_value_set
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS GetSupportedListMax(const ActDevice &device, QSet<ActInterfaceIdValue> &interface_id_value_set);

  /**
   * @brief Get the Ip Mac Table object
   *
   * @param ip_mac_map_result
   * @return ACT_STATUS
   */
  ACT_STATUS GetIpMacTable(QMap<QString, QString> &ip_mac_map_result);

  /**
   * @brief Get the Localhost Arp Table object
   *
   * @param ip_mac_map_result
   * @return ACT_STATUS
   */
  ACT_STATUS GetLocalhostArpTable(const QSet<QString> &arp_entry_types, QMap<QString, QString> &ip_mac_map_result);

  /**
   * @brief Get the Localhost Adapter Table object
   *
   * @param ip_mac_map_result
   * @return ACT_STATUS
   */
  ACT_STATUS GetLocalhostAdapterTable(QMap<QString, QString> &ip_mac_map_result);

  /**
   * @brief Get the Localhost Adapter Ip Device Table object
   *
   * @param ip_dev_map_result
   * @return ACT_STATUS
   */
  ACT_STATUS GetLocalhostAdapterIpDeviceTable(QMap<QString, QString> &ip_dev_map_result);

  /**
   * @brief Add the Arp entry object
   *
   * @param interface_name
   * @param ip
   * @param mac
   * @return ACT_STATUS
   */
  ACT_STATUS AddArpEntry(const QString &interface_name, const QString &ip, const QString &mac);

  /**
   * @brief Delete the Arp entry object
   *
   * @param interface_name
   * @param ip
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteArpEntry(const QString &ip);

  /**
   * @brief Find source device by device's LLDP data & MacTable
   *
   * @param devices
   * @param result_src_device
   * @return ACT_STATUS
   */
  ACT_STATUS FindSourceDeviceByLLDPAndMacTable(const QList<ActDevice> &devices, ActSourceDevice &result_src_device);

  /**
   * @brief Parser the MAC format at the linux base
   *
   * @param mac
   * @param dst
   * @return ACT_STATUS
   */
  ACT_STATUS ParserMacForLinuxBase(const QString &mac, char *dst);

  /**
   * @brief Enable SNMP service action
   *
   * @param check_connect
   * @param device
   * @param feat_sub_item
   * @return ACT_STATUS
   */
  ACT_STATUS ActSouthbound::ActionEnableSnmpService(const bool &check_connect, const ActDevice &device,
                                                    const ActFeatureSubItem &feat_sub_item);

  /**
   * @brief Get ModelName action
   *
   * @param device
   * @param action_str
   * @param action_methods
   * @param result_model_name
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetModelName(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                QString &result_model_name);

  /**
   * @brief Get FirmwareVersion action
   *
   * @param device
   * @param action_str
   * @param action_methods
   * @param result_firmware_version
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetFirmwareVersion(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                      QString &result_firmware_version);

  /**
   * @brief Get DeviceName action
   *
   * @param device
   * @param feat_sub_item
   * @param result_device_name
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetDeviceName(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                 QString &result_device_name);

  /**
   * @brief Get VendorId action
   *
   * @param device
   * @param action_str
   * @param action_methods
   * @param result_vendor_id
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetVendorId(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                               quint32 &result_vendor_id);

  /**
   * @brief Scan links by MAC table
   *
   * @param device
   * @param feat_sub_item
   * @param alive_devices
   * @param exist_links
   * @param ip_mac_table
   * @param result
   * @return ACT_STATUS
   */
  ACT_STATUS ActionScanLinksByMacTable(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                       const QSet<ActDevice> &alive_devices, const QSet<ActLink> &exist_links,
                                       const QMap<QString, QString> &ip_mac_table, ActScanLinksResult &result);

  /**
   * @brief Check source device by MAC table
   *
   * @param device
   * @param feat_sub_item
   * @param adapters_mac_set
   * @param result_port_id
   * @return ACT_STATUS
   */
  ACT_STATUS ActionFindSourceByMacTable(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                        const QSet<QString> &adapters_mac_set, qint64 &result_port_id);

  /**
   * @brief Get LLDP local ChassisID
   *
   * @param device
   * @param feat_sub_item
   * @param result_chassis_id
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetLldpLocChassisID(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                       QString &result_chassis_id);

  /**
   * @brief Get InterfaceName action
   *
   * @param device
   * @param feat_sub_item
   * @param result_if_name_map
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetInterfaceName(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                    QMap<qint64, QString> &result_if_name_map);

  /**
   * @brief Get LLDP local Port ID
   *
   * @param device
   * @param feat_sub_item
   * @param result_if_port_id_map
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetLldpLocPortID(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                    QMap<qint64, QString> &result_if_port_id_map);

  /**
   * @brief Get Interface MAC action
   *
   * @param device
   * @param feat_sub_item
   * @param result_if_mac_map
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetInterfaceMac(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                   QMap<qint64, QString> &result_if_mac_map);

  /**
   * @brief Get Port speed action
   *
   * @param device
   * @param feat_sub_item
   * @param result_port_speed_map
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetPortSpeed(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                QMap<qint64, qint64> &result_port_speed_map);

  /**
   * @brief Get LLDP data
   *
   * @param device
   * @param feat_sub_item
   * @param result_lldp_data
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetLldpData(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                               ActLLDPData &result_lldp_data);

  /**
   * @brief Get MAC table data
   *
   * @param device
   * @param feat_sub_item
   * @param result_port_mac_map
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetSingleEntryMacTable(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                          QMap<qint64, QString> &result_port_mac_map);

  /**
   * @brief Check SNMP connect
   *
   * @param device
   * @param feat_sub_item
   * @return ACT_STATUS
   */
  ACT_STATUS ActionCheckSnmpConnect(const ActDevice &device, const ActFeatureSubItem &feat_sub_item);

  /**
   * @brief Check RESTful connect
   *
   * @param device
   * @param feat_sub_item
   * @return ACT_STATUS
   */
  ACT_STATUS ActionCheckRestfulConnect(const ActDevice &device, const ActFeatureSubItem &feat_sub_item);

  /**
   * @brief Check NETCONF connect
   *
   * @param device
   * @param feat_sub_item
   * @return ACT_STATUS
   */
  // ACT_STATUS ActionCheckNetconfConnect(const ActDevice &device, const ActFeatureSubItem &feat_sub_item);

  /**
   * @brief Check NewMOXACommand connect
   *
   * @param device
   * @param feat_sub_item
   * @return ACT_STATUS
   */
  ACT_STATUS ActionCheckNewMOXACommandConnect(const ActDevice &device, const ActFeatureSubItem &feat_sub_item);

  /**
   * @brief Feature update device SNMP status
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS FeatureUpdateDeviceStatusSnmp(const bool &use_feature_profile, ActDevice &device);

  /**
   * @brief Feature update device NETCONF status
   *
   * @param use_feature_profile
   * @param device
   * @return ACT_STATUS
   */
  // ACT_STATUS FeatureUpdateDeviceStatusNetconf(const bool &use_feature_profile, ActDevice &device);

  /**
   * @brief Feature update device RESTful status
   *
   * @param use_feature_profile
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS FeatureUpdateDeviceStatusRestful(const bool &use_feature_profile, ActDevice &device);

  /**
   * @brief Feature update device NewMOXACommand status
   *
   * @param use_feature_profile
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS FeatureUpdateDeviceStatusNewMOXACommand(const bool &use_feature_profile, ActDevice &device);

  /**
   *
   * @brief Assign the device status at feature level
   *
   * @param use_feature_profile
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS FeatureAssignDeviceStatus(const bool &use_feature_profile, ActDevice &device);

  /**
   * @brief Assign the device status at feature level
   *
   * @param use_feature_profile
   * @param device
   * @param control
   * @return ACT_STATUS
   */
  ACT_STATUS FeatureAssignDeviceStatus(const bool &use_feature_profile, ActDevice &device,
                                       const ActDeviceConnectStatusControl &control);

  /**
   * @brief Enable device SNMP service at feature level
   *
   * @param use_feature_profile
   * @param device
   * @param check_action_method_connect
   * @return ACT_STATUS
   */
  ACT_STATUS FeatureEnableDeviceSnmp(const bool &use_feature_profile, ActDevice &device,
                                     const bool &check_action_method_connect);

  /**
   * @brief Assign devices Moxa vendor by BroadcastSearch at feature level
   *
   * @param devices
   * @return ACT_STATUS
   */
  ACT_STATUS FeatureAssignDevicesMoxaVendorByBroadcastSearch(QSet<ActDevice> &devices);

  /**
   * @brief Get Device IdentifyDeviceInfo
   *
   * @param use_feature_profile
   * @param device
   * @param result_identify_device_info
   * @return ACT_STATUS
   */
  ACT_STATUS FeatureGetIdentifyDeviceInfo(const bool &use_feature_profile, const ActDevice &device,
                                          ActIdentifyDeviceInfo &result_identify_device_info);

  /**
   * @brief Identify device and get profiles(DeviceProfile & FirmwareFeatureProfile)
   *
   * @param use_feature_profile
   * @param device
   * @param profiles
   * @param result_device_profile
   * @param result_firmware_feature_profile
   * @param result_firmware
   * @return ACT_STATUS
   */
  ACT_STATUS FeatureIdentifyDeviceAndGetProfiles(const bool &use_feature_profile, ActDevice &device,
                                                 const ActProfiles &profiles, ActDeviceProfile &result_device_profile,
                                                 ActFirmwareFeatureProfile &result_firmware_feature_profile,
                                                 QString &result_firmware);

  /**
   * @brief Identify device and get profiles(DeviceProfile & FirmwareFeatureProfile) use by AutoProbe
   *
   * @param use_feature_profile
   * @param device
   * @param profiles
   * @param result_device_profile
   * @param result_firmware_feature_profile
   * @return ACT_STATUS
   */
  ACT_STATUS FeatureIdentifyDeviceAndGetProfilesByProbe(const bool &use_feature_profile, ActDevice &device,
                                                        const ActProfiles &profiles,
                                                        ActDeviceProfile &result_device_profile,
                                                        ActFirmwareFeatureProfile &result_firmware_feature_profile);

  /**
   * @brief Get TSN-Switch Configuration sync status
   *
   * @param device
   * @param check_result
   * @return ACT_STATUS
   */
  ACT_STATUS GetTSNSwitchConfigurationSyncStatus(const ActDevice &device, bool &check_result);

  /**
   * @brief Reboot device object
   *
   * @param device
   * @param feat_sub_item
   * @return ACT_STATUS
   */
  ACT_STATUS ActionReboot(const ActDevice &device, const ActFeatureSubItem &feat_sub_item);

  /**
   * @brief FactoryDefault device object
   *
   * @param device
   * @param feat_sub_item
   * @return ACT_STATUS
   */
  ACT_STATUS ActionFactoryDefault(const ActDevice &device, const ActFeatureSubItem &feat_sub_item);

  /**
   * @brief FirmwareUpgrade device object
   *
   * @param device
   * @param feat_sub_item
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS ActionFirmwareUpgrade(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                   const QString &file_path);

  /**
   * @brief Export device config object
   *
   * @param device
   * @param feat_sub_item
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS ActionExportConfig(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                const QString &file_path);

  /**
   * @brief Import device config object
   *
   * @param device
   * @param feat_sub_item
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS ActionImportConfig(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                const QString &file_path);

  /**
   * @brief Locator device object
   *
   * @param device
   * @param feat_sub_item
   * @param duration
   * @return ACT_STATUS
   */
  ACT_STATUS ActionLocator(const ActDevice &device, const ActFeatureSubItem &feat_sub_item, const quint16 &duration);

  /**
   * @brief Get device EventLog object
   *
   * @param device
   * @param feat_sub_item
   * @param result_event_log
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetEventLog(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                               ActDeviceEventLog &result_event_log);

  /**
   * @brief  BroadcastSearch devices object
   *
   * @param feat_sub_item
   * @param result_device_list
   * @param result_mac_host_map
   * @return ACT_STATUS
   */
  ACT_STATUS ActionBroadcastSearchDevices(const ActFeatureSubItem &feat_sub_item, QList<ActDevice> &result_device_list,
                                          QMap<QString, QString> &result_mac_host_map);

  /**
   * @brief NetworkSetting object
   *
   * @param device
   * @param feat_sub_item
   * @param network_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetNetworkSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                     const ActNetworkSettingTable &network_setting_table);

  /**
   * @brief Delete VLAN member
   *
   * @param device
   * @param feat_sub_item
   * @param delete_vlan_list
   * @return ACT_STATUS
   */
  ACT_STATUS ActionDeleteVLANMember(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                    const QList<qint32> &delete_vlan_list);
  /**
   * @brief Aff VLAN member
   *
   * @param device
   * @param feat_sub_item
   * @param add_vlan_list
   * @return ACT_STATUS
   */
  ACT_STATUS ActionAddVLANMember(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                 const QList<qint32> &add_vlan_list);

  /**
   * @brief Set VLAN static
   *
   * @param device
   * @param feat_sub_item
   * @param vlan_static_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetVLAN(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                           const ActVlanStaticTable &vlan_static_table);

  /**
   * @brief Get VLAN static
   *
   * @param device
   * @param feat_sub_item
   * @param result_vlan_static_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetVLAN(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                           ActVlanStaticTable &result_vlan_static_table);

  /**
   * @brief Set VLAN Port type action
   *
   * @param device
   * @param feat_sub_item
   * @param vlan_port_type_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetVLANPortType(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                   const ActVlanPortTypeTable &vlan_port_type_table);

  /**
   * @brief Get VLAN Port type action
   *
   * @param device
   * @param feat_sub_item
   * @param result_vlan_port_type_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetVLANPortType(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                   ActVlanPortTypeTable &result_vlan_port_type_table);

  /**
   * @brief Set Port PVID action
   *
   * @param device
   * @param feat_sub_item
   * @param port_vlan_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetPortPVID(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                               const ActPortVlanTable &port_vlan_table);

  /**
   * @brief Get Port PVID action
   *
   * @param device
   * @param feat_sub_item
   * @param result_port_vlan_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetPortPVID(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                               ActPortVlanTable &result_port_vlan_table);

  /**
   * @brief Set Port default PCP action
   *
   * @param device
   * @param feat_sub_item
   * @param default_priority_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetPortDefaultPCP(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                     const ActDefaultPriorityTable &default_priority_table);

  /**
   * @brief Get Port default PCP action
   *
   * @param device
   * @param feat_sub_item
   * @param result_default_priority_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetPortDefaultPCP(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                     ActDefaultPriorityTable &result_default_priority_table);

  /**
   * @brief Set Management VLAN action
   *
   * @param device
   * @param feat_sub_item
   * @param mgmt_vlan
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetManagementVlan(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                     const qint32 &mgmt_vlan);

  /**
   * @brief Get Management VLAN action
   *
   * @param device
   * @param feat_sub_item
   * @param result_mgmt_vlan
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetManagementVlan(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                     qint32 &result_mgmt_vlan);
  /**
   * @brief Get StreamPriority Ingress action
   *
   * @param device
   * @param feat_sub_item
   * @param result_stad_port_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetStreamPriorityIngress(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            ActStadPortTable &result_stad_port_table);

  /**
   * @brief Set StreamPriority Ingress action
   *
   * @param device
   * @param feat_sub_item
   * @param stad_port_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetStreamPriorityIngress(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            const ActStadPortTable &stad_port_table);

  /**
   * @brief Get StreamPriority Egress action
   *
   * @param device
   * @param feat_sub_item
   * @param result_stad_config_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetStreamPriorityEgress(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                           ActStadConfigTable &result_stad_config_table);

  /**
   * @brief Set StreamPriority Egress action
   *
   * @param device
   * @param feat_sub_item
   * @param stad_config_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetStreamPriorityEgress(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                           const ActStadConfigTable &stad_config_table);

  /**
   * @brief Probe VLAN static(VLAN Method)
   *
   * @param device
   * @param feat_sub_item
   * @param static_forward_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetStaticUnicast(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                    const ActStaticForwardTable &static_forward_table);

  /**
   * @brief Get Static Forward(Unicast)
   *
   * @param device
   * @param feat_sub_item
   * @param result_static_forward_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetStaticUnicast(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                    ActStaticForwardTable &result_static_forward_table);

  /**
   * @brief Set Static Forward(Multicast)
   *
   * @param device
   * @param feat_sub_item
   * @param static_forward_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetStaticMulticast(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                      const ActStaticForwardTable &static_forward_table);

  /**
   * @brief Get Static Forward(Multicast)
   *
   * @param device
   * @param feat_sub_item
   * @param result_static_forward_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetStaticMulticast(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                      ActStaticForwardTable &result_static_forward_table);

  /**
   * @brief Set SpanningTreeMethod(RSTP Method) action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreeMethod(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                         const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree action
   *
   * @param device
   * @param feat_sub_item
   * @param result_rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetSpanningTree(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                   ActRstpTable &result_rstp_table);

  /**
   * @brief Set SpanningTree Basic RSTP action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreeBasicRstp(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree HelloTime action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreeHelloTime(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            const ActRstpTable &rstp_table);

  /**
   * @brief Get IP Configuration action
   *
   * @param device
   * @param feat_sub_item
   * @param result_ipv4
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetIPConfiguration(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                      ActIpv4 &result_ipv4);

  /**
   * @brief Get Serial Number action
   *
   * @param device
   * @param feat_sub_item
   * @param result_serial_number
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetSerialNumber(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                   QString &result_serial_number);

  /**
   * @brief Get System Uptime action
   *
   * @param device
   * @param feat_sub_item
   * @param result_uptime
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetSystemUptime(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                   QString &result_uptime);

  /**
   * @brief Get Product Revision action
   *
   * @param device
   * @param feat_sub_item
   * @param result_product_revision
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetProductRevision(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                      QString &result_product_revision);

  /**
   * @brief Get Redundant Protocol action
   *
   * @param device
   * @param feat_sub_item
   * @param result_redundant_protocol
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetRedundantProtocol(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                        QString &result_redundant_protocol);

  /**
   * @brief Get Location action
   *
   * @param device
   * @param feat_sub_item
   * @param result_device_location
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetLocation(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                               QString &result_device_location);

  /**
   * @brief Get Description action
   *
   * @param device
   * @param feat_sub_item
   * @param result_device_description
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetDescription(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                  QString &result_device_description);

  /**
   * @brief Get Contact Information action
   *
   * @param device
   * @param feat_sub_item
   * @param result_contact_info
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetContactInformation(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                         QString &result_contact_info);
  /**
   * @brief Get Modular Info action
   *
   * @param device
   * @param feat_sub_item
   * @param result_modular_info
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetModularInfo(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                  ActDeviceModularInfo &result_modular_info);

  // /**
  //  * @brief Get System Information action
  //  *
  //  * @param device
  //  * @param feat_sub_item
  //  * @param result_system_information
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS ActionGetSystemInformation(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
  //                                       ActMonitorSystemInformation &result_system_information);

  /**
   * @brief Get System Utilization action
   *
   * @param device
   * @param feat_sub_item
   * @param result_system_utilization
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetSystemUtilization(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                        ActMonitorSystemUtilization &result_system_utilization);

  /**
   * @brief Get Port info action
   *
   * @param device
   * @param feat_sub_item
   * @param result_port_info_map
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetPortInfo(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                               QMap<qint64, ActDevicePortInfoEntry> &result_port_info_map);

  /**
   * @brief Get Port status action
   *
   * @param device
   * @param feat_sub_item
   * @param result_port_status_map
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetPortStatus(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                 QMap<qint64, ActMonitorPortStatusEntry> &result_port_status_map);

  /**
   * @brief Get Fiber check action
   *
   * @param device
   * @param feat_sub_item
   * @param result_port_fiber_map
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetFiberCheck(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                 QMap<qint64, ActMonitorFiberCheckEntry> &result_port_fiber_map);

  /**
   * @brief Get Tx Total Octets action
   *
   * @param device
   * @param feat_sub_item
   * @param result_port_tx_total_octets_map
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetTxTotalOctets(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                    QMap<qint64, quint64> &result_port_tx_total_octets_map);

  /**
   * @brief Get Tx Total Packets action
   *
   * @param device
   * @param feat_sub_item
   * @param result_port_tx_total_packets_map
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetTxTotalPackets(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                     QMap<qint64, quint64> &result_port_tx_total_packets_map);

  /**
   * @brief Get Traffic Utilization action
   *
   * @param device
   * @param feat_sub_item
   * @param result_port_traffic_utilization_map
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetTrafficUtilization(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                         QMap<qint64, qreal> &result_port_traffic_utilization_map);

  /**
   * @brief Get 1588_2008 Time Sync status action
   *
   * @param device
   * @param feat_sub_item
   * @param result_time_sync_status
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGet1588TimeSyncStatus(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                         ActMonitorTimeSyncStatus &result_time_sync_status);

  /**
   * @brief Get Dot1AS_2011 Time Sync status action

   *
   * @param device
   * @param feat_sub_item
   * @param result_time_sync_status
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetDot1ASTimeSyncStatus(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                           ActMonitorTimeSyncStatus &result_time_sync_status);

  /**
   * @brief Get RSTP status action
   *
   * @param device
   * @param feat_sub_item
   * @param result_rstp_status
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetRstpStatus(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                 ActMonitorRstpStatus &result_rstp_status);

  /**
   * @brief Probe SpanningTree HelloTime action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionSpanningTreeHelloTime(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              ActFeatureSubItem &result_feat_sub_item,
                                              ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Set SpanningTree Priority action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreePriority(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                           const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree MaxAge action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreeMaxAge(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                         const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree ForwardDelay action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreeForwardDelay(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                               const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree ErrorRecoveryTime action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreeErrorRecoveryTime(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                    const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree Swift action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreeSwift(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                        const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree PortRSTPEnable action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreePortRSTPEnable(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                 const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree PortPriority action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreePortPriority(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                               const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree Edge action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreeEdge(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                       const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree PathCost action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreePathCost(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                           const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree LinkType action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreeLinkType(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                           const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree BPDUGuard action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreeBPDUGuard(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree RootGuard action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreeRootGuard(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree LoopGuard action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreeLoopGuard(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            const ActRstpTable &rstp_table);

  /**
   * @brief Set SpanningTree BPDUFilter action
   *
   * @param device
   * @param feat_sub_item
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSpanningTreeBPDUFilter(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                             const ActRstpTable &rstp_table);

  /**
   * @brief  Set UserAccount action
   *
   * @param device
   * @param feat_sub_item
   * @param user_account_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetUserAccount(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                  const ActUserAccountTable &user_account_table);

  /**
   * @brief  Get UserAccount action
   *
   * @param device
   * @param feat_sub_item
   * @param result_user_account_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetUserAccount(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                  ActUserAccountTable &result_user_account_table);

  /**
   * @brief  Set LoginPolicy action
   *
   * @param device
   * @param feat_sub_item
   * @param login_policy_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetLoginPolicy(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                  const ActLoginPolicyTable &login_policy_table);

  /**
   * @brief  Get LoginPolicy action
   *
   * @param device
   * @param feat_sub_item
   * @param result_login_policy_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetLoginPolicy(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                  ActLoginPolicyTable &result_login_policy_table);

  /**
   * @brief  Set SnmpTrap setting action
   *
   * @param device
   * @param feat_sub_item
   * @param snmp_trap_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSnmpTrapSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                      const ActSnmpTrapSettingTable &snmp_trap_table);

  /**
   * @brief  Get SnmpTrap setting action
   *
   * @param device
   * @param feat_sub_item
   * @param result_snmp_trap_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetSnmpTrapSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                      ActSnmpTrapSettingTable &result_snmp_trap_table);

  /**
   * @brief Set Syslog setting action
   *
   * @param device
   * @param feat_sub_item
   * @param syslog_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetSyslogSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                    const ActSyslogSettingTable &syslog_table);

  /**
   * @brief Get Syslog setting action
   *
   * @param device
   * @param feat_sub_item
   * @param result_syslog_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetSyslogSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                    ActSyslogSettingTable &result_syslog_table);

  /**
   * @brief Set Time setting action
   *
   * @param device
   * @param feat_sub_item
   * @param time_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetTimeSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                  const ActTimeSettingTable &time_table);

  /**
   * @brief Get Time setting action
   *
   * @param device
   * @param feat_sub_item
   * @param result_time_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetTimeSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                  ActTimeSettingTable &result_time_table);

  /**
   * @brief Set LoopProtection action
   *
   * @param device
   * @param feat_sub_item
   * @param lp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetLoopProtection(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                     const ActLoopProtectionTable &lp_table);
  /**
   * @brief Get LoopProtection action
   *
   * @param device
   * @param feat_sub_item
   * @param result_lp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetLoopProtection(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                     ActLoopProtectionTable &result_lp_table);

  /**
   * @brief Set Information Setting action
   *
   * @param device
   * @param feat_sub_item
   * @param info_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetInformationSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                         const ActInformationSettingTable &info_setting_table);

  /**
   * @brief Get Information Setting action
   *
   * @param device
   * @param feat_sub_item
   * @param result_info_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetInformationSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                         ActInformationSettingTable &result_info_setting_table);

  /**
   * @brief Set Management Interface action
   *
   * @param device
   * @param feat_sub_item
   * @param mgmt_interface_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetManagementInterface(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                          const ActManagementInterfaceTable &mgmt_interface_table);

  /**
   * @brief Get Management Interface action
   *
   * @param device
   * @param feat_sub_item
   * @param result_info_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetManagementInterface(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                          ActManagementInterfaceTable &result_mgmt_interface_table);

  /**
   * @brief Set PortSetting AdminStatus action
   *
   * @param device
   * @param feat_sub_item
   * @param port_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSetPortSettingAdminStatus(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                             const ActPortSettingTable &port_setting_table);

  /**
   * @brief Get PortSetting AdminStatus action
   *
   * @param device
   * @param feat_sub_item
   * @param result_port_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetPortSettingAdminStatus(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                             ActPortSettingTable &result_port_setting_table);

  /**
   * @brief Set 802.1qbv(GCL) action
   *
   * @param device
   * @param feat_sub_item
   * @param gcl_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSet802Dot1Qbv(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                 const ActGclTable &gcl_table);

  /**
   * @brief Get 802.1qbv(GCL) action
   *
   * @param device
   * @param feat_sub_item
   * @param result_gcl_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGet802Dot1Qbv(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                 ActGclTable &result_gcl_table);

  /**
   * @brief Set 802.1CB action
   *
   * @param device
   * @param feat_sub_item
   * @param ieee_802_1cb_table
   * @return ACT_STATUS
   */
  ACT_STATUS ActionSet802Dot1CB(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                const ActCbTable &ieee_802_1cb_table);

  /**
   * @brief Get TimeSync Base Config action
   *
   * @param device
   * @param feat_sub_item
   * @param result_base_config
   * @return * ACT_STATUS
   */
  ACT_STATUS ActionGetTimeSyncBaseConfig(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                         ActTimeSyncBaseConfig &result_base_config);

  /**
   * @brief Get TimeSync 802Dot1AS Config action
   *
   * @param device
   * @param feat_sub_item
   * @param resutl_time_sync_config
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetTimeSync802Dot1ASConfig(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              ActTimeSync802Dot1ASConfig &resutl_time_sync_config);

  /**
   * @brief Get TimeSync 1588 Config action
   *
   * @param device
   * @param feat_sub_item
   * @param resutl_time_sync_config
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetTimeSync1588Config(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                         ActTimeSync1588Config &resutl_time_sync_config);

  /**
   * @brief Get TimeSync Iec61850 Config action
   *
   * @param device
   * @param feat_sub_item
   * @param resutl_time_sync_config
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetTimeSyncIec61850Config(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                             ActTimeSyncIec61850Config &resutl_time_sync_config);

  /**
   * @brief Get TimeSync C37.238 Config action
   *
   * @param device
   * @param feat_sub_item
   * @param resutl_time_sync_config
   * @return ACT_STATUS
   */
  ACT_STATUS ActionGetTimeSyncC37Dot238Config(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              ActTimeSyncC37Dot238Config &resutl_time_sync_config);

  /**
   * @brief Assign device partial infos object
   *
   * @param device
   * @param feat_sub_item
   * @return ACT_STATUS
   */
  ACT_STATUS ActionAssignDevicePartialInfos(ActDevice &device, const ActFeatureSubItem &feat_sub_item);

  // Probe Functions
  /**
   * @brief Probe Netconf YANG Library action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  // ACT_STATUS ProbeActionNetconfYangLibrary(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
  //                                          ActFeatureSubItem &result_feat_sub_item,
  //                                          ActFeatureSubItemWarning &result_warning);
  /**
   * @brief Probe Enable SNMP service action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ActSouthbound::ProbeActionEnableSnmpService(const ActDevice &device,
                                                         const ActFeatureSubItem &feat_sub_item,
                                                         ActFeatureSubItem &result_feat_sub_item,
                                                         ActFeatureSubItemWarning &result_warning);
  /**
   * @brief Probe ModelName action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionModelName(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                  ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe FirmwareVersion action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionFirmwareVersion(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                        ActFeatureSubItem &result_feat_sub_item,
                                        ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe DeviceName action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionDeviceName(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                   ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe VendorId action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionVendorId(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                 ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe MAC table action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionMacTable(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                 ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe InterfaceName action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionInterfaceName(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                      ActFeatureSubItem &result_feat_sub_item,
                                      ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe  Interface MAC action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionInterfaceMac(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                     ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe Port speed action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionPortSpeed(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                  ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe the LLDP action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionLLDP(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                             ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe the check SNMP connect action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionCheckSnmpConnect(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                         ActFeatureSubItem &result_feat_sub_item,
                                         ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe the check RESTful connect action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionCheckRestfulConnect(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            ActFeatureSubItem &result_feat_sub_item,
                                            ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe the check NETCONF connect action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  // ACT_STATUS ProbeActionCheckNetconfConnect(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
  //                                           ActFeatureSubItem &result_feat_sub_item,
  //                                           ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe reboot device object
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionReboot(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                               ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe FactoryDefault device object
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionFactoryDefault(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                       ActFeatureSubItem &result_feat_sub_item,
                                       ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe FirmwareUpgrade device object
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionFirmwareUpgrade(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                        ActFeatureSubItem &result_feat_sub_item,
                                        ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe BroadcastSearch object
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionBroadcastSearch(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                        ActFeatureSubItem &result_feat_sub_item,
                                        ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe NetworkSetting object
   *
   * @param device
   * @param feat_sub_item
   * @param network_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionNetworkSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                       ActFeatureSubItem &result_feat_sub_item,
                                       ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Set VLAN static
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionVLAN(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                             ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe VLAN Port type action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionVLANPortType(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                     ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe TE-MSTID action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionTEMSTID(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe Port PVID action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionPortPVID(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                 ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  /**
     * @brief Probe Port default PCP action

   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionPortDefaultPCP(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                       ActFeatureSubItem &result_feat_sub_item,
                                       ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe StreamPriority action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionStreamPriority(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                       ActFeatureSubItem &result_feat_sub_item,
                                       ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe Static Forward(Unicast)
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionStaticUnicast(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                      ActFeatureSubItem &result_feat_sub_item,
                                      ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe Static Forward(Multicast)
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionStaticMulticast(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                        ActFeatureSubItem &result_feat_sub_item,
                                        ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe SpanningTreeMethod(RSTP Method) action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionSpanningTreeMethod(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                           ActFeatureSubItem &result_feat_sub_item,
                                           ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe SpanningTree Priority action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionSpanningTreePriority(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                             ActFeatureSubItem &result_feat_sub_item,
                                             ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe SpanningTree RootGuard action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeActionSpanningTreeRootGuard(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              ActFeatureSubItem &result_feat_sub_item,
                                              ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe 802.1qbv(GCL) action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeAction802Dot1Qbv(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                   ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  /**
   * @brief Probe 802.1CB action
   *
   * @param device
   * @param feat_sub_item
   * @param result_feat_sub_item
   * @param result_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeAction802Dot1CB(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                  ActFeatureSubItem &result_feat_sub_item, ActFeatureSubItemWarning &result_warning);

  // ACT_STATUS NetconfAction(const ActDevice &device, const QSet<QString> &sub_item_key_set,
  //                          const ActDeviceConfig &device_config);

  /*************
   *  Scanner  *
   *************/

  ACT_STATUS UpdateDeviceConnectByScanFeature(ActDevice &device);

  /**
   * @brief Assign Device's Interfaces
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceInterfacesInfo(ActDevice &device);

  /**
   * @brief Assign device configs
   *
   * @param device
   * @param device_config
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceConfigs(ActDevice &device, ActDeviceConfig &device_config);

  /**
   * @brief Assign the Device LLDP Data
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceLldpData(ActDevice &device);

  /**
   * @brief Assign the Device SingleEntry MAC table Data
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceMacTable(ActDevice &device);

  // /**
  //  * @brief Assign SFP info to module info
  //  *
  //  * @param device
  //  * @param port_fiber_map
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS AssignSFPtoModule(ActDevice &device, const QMap<qint64, ActMonitorFiberCheckEntry> &port_fiber_map,
  //                              const QMap<qint64, ActDevicePortInfoEntry> &port_info_map);

  /**
   * @brief  Assign the Device IPv4 data
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceIPv4(ActDevice &device);

  /**
   * @brief  Assign the Device Name data
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceName(ActDevice &device);

  /**
   * @brief  Assign the Device SerialNumber data
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceSerialNumber(ActDevice &device);

  /**
   * @brief  Assign the Device SystemUptime data
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceSystemUptime(ActDevice &device);

  /**
   * @brief  Assign the Device ProductRevision data
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceProductRevision(ActDevice &device);

  /**
   * @brief  Assign the Device RedundantProtocol data
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceRedundantProtocol(ActDevice &device);

  /**
   * @brief  Assign the Device Location data
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceLocation(ActDevice &device);

  /**
   * @brief  Assign the Device ModularConfiguration data
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceModularConfiguration(ActDevice &device);

  /**
   * @brief  Assign the Interfaces & BuiltinPower by the ModularConfiguration
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignInterfacesAndBuiltinPowerByModular(ActDevice &device);

  // /**
  //  * @brief  Assign the Device ModularInfo data
  //  *
  //  * @param device
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS AssignDeviceModularInfo(ActDevice &device);

  /**
   * @brief Scan the FiberCheck map
   *
   * @param device
   * @param port_fiber_map
   * @return ACT_STATUS
   */
  ACT_STATUS ScanFiberCheck(const ActDevice &device, QMap<qint64, ActMonitorFiberCheckEntry> &port_fiber_map);

  /**
   * @brief Scan the PortInfo map
   *
   * @param device
   * @param port_info_map
   * @return ACT_STATUS
   */
  ACT_STATUS ScanPortInfo(const ActDevice &device, QMap<qint64, ActDevicePortInfoEntry> &port_info_map);

  /**
   * @brief Scan the NetworkSetting Table
   *
   * @param device
   * @param result_network_setting
   * @return ACT_STATUS
   */
  ACT_STATUS ScanNetworkSettingTable(const ActDevice &device, ActNetworkSettingTable &result_network_setting);

  /**
   * @brief Scan the UserAccount Table
   *
   * @param device
   * @param result_user_account
   * @return ACT_STATUS
   */
  ACT_STATUS ScanUserAccountTable(const ActDevice &device, ActUserAccountTable &result_user_account);

  /**
   * @brief Scan the LoginPolicy Table
   *
   * @param device
   * @param result_login_policy
   * @return ACT_STATUS
   */
  ACT_STATUS ScanLoginPolicyTable(const ActDevice &device, ActLoginPolicyTable &result_login_policy);

  /**
   * @brief Scan the ManagementInterface Table
   *
   * @param device
   * @param result_mgmt_interface
   * @return ACT_STATUS
   */
  ACT_STATUS ScanManagementInterfaceTable(const ActDevice &device, ActManagementInterfaceTable &result_mgmt_interface);

  /**
   * @brief Scan the SnmpTrapSetting Table
   *
   * @param device
   * @param result_snmp_trap_setting
   * @return ACT_STATUS
   */
  ACT_STATUS ScanSnmpTrapSettingTable(const ActDevice &device, ActSnmpTrapSettingTable &result_snmp_trap_setting);

  /**
   * @brief Scan the SyslogSetting Table
   *
   * @param device
   * @param result_syslog_setting
   * @return ACT_STATUS
   */
  ACT_STATUS ScanSyslogSettingTable(const ActDevice &device, ActSyslogSettingTable &result_syslog_setting);

  /**
   * @brief Scan the LoopProtection Table
   *
   * @param device
   * @param result_loop_protection
   * @return ACT_STATUS
   */
  ACT_STATUS ScanLoopProtectionTable(const ActDevice &device, ActLoopProtectionTable &result_loop_protection);

  /**
   * @brief Scan the InformationSetting Table
   *
   * @param device
   * @param result_info_setting
   * @return ACT_STATUS
   */
  ACT_STATUS ScanInformationSettingTable(const ActDevice &device, ActInformationSettingTable &result_info_setting);

  /**
   * @brief Scan the TimeSetting Table
   *
   * @param device
   * @param result_time_setting
   * @return ACT_STATUS
   */
  ACT_STATUS ScanTimeSettingTable(const ActDevice &device, ActTimeSettingTable &result_time_setting);

  /**
   * @brief Scan the PortSetting Table
   *
   * @param device
   * @param result_port_setting
   * @return ACT_STATUS
   */
  ACT_STATUS ScanPortSettingTable(const ActDevice &device, ActPortSettingTable &result_port_setting);

  /**
   * @brief Scan the VLAN Table
   *
   * @param device
   * @param result_vlan_table
   * @return ACT_STATUS
   */
  ACT_STATUS ScanVlanTable(const ActDevice &device, ActVlanTable &result_vlan_table);

  /**
   * @brief Scan the PCP Table
   *
   * @param device
   * @param result_default_priority_table
   * @return ACT_STATUS
   */
  ACT_STATUS ScanPortDefaultPCPTable(const ActDevice &device, ActDefaultPriorityTable &result_default_priority_table);

  /**
   * @brief Scan the StreamPriority Ingress Table
   *
   * @param device
   * @param result_stad_port_table
   * @return ACT_STATUS
   */
  ACT_STATUS ScanStreamPriorityIngressTable(const ActDevice &device, ActStadPortTable &result_stad_port_table);

  /**
   * @brief Scan the StreamPriority Egress Table
   *
   * @param device
   * @param result_stad_config_table
   * @return ACT_STATUS
   */
  ACT_STATUS ScanStreamPriorityEgressTable(const ActDevice &device, ActStadConfigTable &result_stad_config_table);

  /**
   * @brief Scan the StaticForward Unicast Table
   *
   * @param device
   * @param result_static_forward
   * @return ACT_STATUS
   */
  ACT_STATUS ScanUnicastStaticTable(const ActDevice &device, ActStaticForwardTable &result_static_forward);

  /**
   * @brief Scan the StaticForward Multicast Table
   *
   * @param device
   * @param result_static_forward
   * @return ACT_STATUS
   */
  ACT_STATUS ScanMulticastStaticTable(const ActDevice &device, ActStaticForwardTable &result_static_forward);

  /**
   * @brief Scan the TimeAwareShaper Table
   *
   * @param device
   * @param result_gcl_table
   * @return ACT_STATUS
   */
  ACT_STATUS ScanTimeAwareShaperTable(const ActDevice &device, ActGclTable &result_gcl_table);

  /**
   * @brief Scan the RSTP Table
   *
   * @param device
   * @param result_rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS ScanRstpTable(const ActDevice &device, ActRstpTable &result_rstp_table);

  /**
   * @brief Scan the TimeSync setting Table
   *
   * @param device
   * @param result_time_sync_table
   * @return ACT_STATUS
   */
  ACT_STATUS ScanTimeSyncTable(const ActDevice &device, ActTimeSyncTable &result_time_sync_table);

  /**
   * @brief Identify a Device
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS IdentifyDevice(ActDevice &device);

  /**
   * @brief Create a Device Link object
   *
   * @param ip_mac_table
   * @param device
   * @param alive_devices
   * @param scan_link_result
   * @return ACT_STATUS
   */
  ACT_STATUS CreateDeviceLink(const QMap<QString, QString> &ip_mac_table, const ActDevice &device,
                              const QSet<ActDevice> &alive_devices, ActScanLinksResult &scan_link_result);

  /*************
   *  Config   *
   *************/

  /**
   * @brief Configure device NetworkSetting
   *
   * @param device
   * @param network_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureNetworkSetting(const ActDevice &device, const ActNetworkSettingTable &network_setting_table);

  /**
   * @brief Configure device VLAN_config
   *
   * @param device
   * @param vlan_config_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureVlan(const ActDevice &device, const ActVlanTable &vlan_config_table);

  /**
   * @brief Configure device StaticForward
   *
   * @param device
   * @param static_forward_table
   * @param unicast
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureStaticForward(const ActDevice &device, const ActStaticForwardTable &static_forward_table,
                                    const bool &unicast);

  /**
   * @brief Configure device StreamPriorityIngress
   *
   * @param device
   * @param stad_port_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureStreamPriorityIngress(const ActDevice &device, const ActStadPortTable &stad_port_table);

  /**
   * @brief Configure device StreamPriorityEgress
   *
   * @param device
   * @param stad_config_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureStreamPriorityEgress(const ActDevice &device, const ActStadConfigTable &stad_config_table);

  /**
   * @brief Configure device PortDefaultPCP
   *
   * @param device
   * @param default_priority_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigurePortDefaultPCP(const ActDevice &device, const ActDefaultPriorityTable &default_priority_table);

  /**
   * @brief Configure device GateControlList
   *
   * @param device
   * @param gcl_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureGCL(const ActDevice &device, const ActGclTable &gcl_table);

  /**
   * @brief Configure device FRER
   *
   * @param device
   * @param ieee_802_1cb_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureFRER(const ActDevice &device, const ActCbTable &ieee_802_1cb_table);

  /**
   * @brief Configure device Spanning Tree
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureSpanningTree(const ActDevice &device, const ActRstpTable &rstp_table);

  /**
   * @brief Configure device User Account
   *
   * @param device
   * @param user_account_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureUserAccount(const ActDevice &device, const ActUserAccountTable &user_account_table);

  /**
   * @brief Configure device Login Policy
   *
   * @param device
   * @param login_policy_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureLoginPolicy(const ActDevice &device, const ActLoginPolicyTable &login_policy_table);

  /**
   * @brief Configure device SnmpTrapSetting
   *
   * @param device
   * @param snmp_trap_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureSnmpTrapSetting(const ActDevice &device, const ActSnmpTrapSettingTable &snmp_trap_setting_table);

  /**
   * @brief Configure device SyslogSetting
   *
   * @param device
   * @param syslog_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureSyslogSetting(const ActDevice &device, const ActSyslogSettingTable &syslog_setting_table);

  /**
   * @brief Configure device TimeSetting
   *
   * @param device
   * @param time_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureTimeSetting(const ActDevice &device, const ActTimeSettingTable &time_setting_table);

  /**
   * @brief Configure device InformationSetting
   *
   * @param device
   * @param info_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureInformationSetting(const ActDevice &device, const ActInformationSettingTable &info_setting_table);

  /**
   * @brief Configure device PortSetting
   *
   * @param device
   * @param port_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigurePortSetting(const ActDevice &device, const ActPortSettingTable &port_setting_table);

  /**
   * @brief Configure device LoopProtection
   *
   * @param device
   * @param loop_protection_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureLoopProtection(const ActDevice &device, const ActLoopProtectionTable &loop_protection_table);

  /**
   * @brief Configure device ManagementInterface
   *
   * @param device
   * @param mgmt_interface_table
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigureManagementInterface(const ActDevice &device,
                                          const ActManagementInterfaceTable &mgmt_interface_table);

  /**
   * @brief Check the device VLAN configuration
   *
   * @param device
   * @param vlan_config_table
   * @param result
   * @return ACT_STATUS
   */
  ACT_STATUS CheckDeviceVLANConfiguration(const ActDevice &device, const ActVlanTable &vlan_config_table, bool &result);

  // /**
  //  * @brief Configure device PortPVID
  //  *
  //  * @param device
  //  * @param port_vlan_table
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS ConfigurePortPVID(const ActDevice &device, const ActPortVlanTable &port_vlan_table);
};
#endif  // ACT_SOUTHBOUND_HPP