/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#include <QString>

#include "act_feature_profile.hpp"
#include "act_monitor_data.hpp"
#include "act_snmp_result.hpp"
#include "act_snmp_set_entry.hpp"
#include "act_status.hpp"
#include "act_utilities.hpp"
#include "deploy_entry/act_deploy_table.hpp"
#include "topology/act_device.hpp"

#ifndef ACT_SNMP_HANDLER_H
#define ACT_SNMP_HANDLER_H

/**
 *
 * @brief The ACT SnmpHandler module class
 *
 */
class ActSnmpHandler {
 private:
  // SNMPv2-MIB (RFC1213-MIB , MIB-II)
  const QString kIfOperStatusOid = "1.3.6.1.2.1.2.2.1.8";  // interface status (up(1), down(2))

  /**
   * @brief Transfer PortsId to PortsIndex
   *
   * @param ports_id
   * @return QSet<QString>
   */
  QSet<QString> PortsIdToPortsIndex(const QSet<QString> &ports_id);

  /**
   * @brief Transfer PortsIndex to PortsId
   *
   * @param ports_index
   * @return QSet<QString>
   */
  QSet<QString> PortsIndexToPortsId(const QSet<QString> &ports_index);

  /**
   * @brief Convert Mac dec string to hex string
   *
   * @param mac_hex
   * @return QString
   */
  QString MacDecToHex(const QString &mac_dec);

  /**
   * @brief Convert Mac hex string to dec string
   *
   * @param mac_hex
   * @return QString
   */
  QString MacHexToDec(const QString &mac_hex);

  /**
   * @brief Convert Ports set to bin string
   * ["1", "3"] -> 1010
   * ["1", "3", "6"] -> 10100100
   *
   * @param ports
   * @return QString
   */
  QString PortsToBin(const QSet<qint64> &ports);

  /**
   * @brief Convert Bin string to hex string
   *
   * @param bin
   * @return QString
   */
  QString BinToHex(const QString &bin);

  /**
   * @brief Convert Bin string to Ports set
   *
   * @param bin
   * @return QSet<QString>
   */
  QSet<qint64> BinToPorts(const QString &bin);

  /**
   * @brief Convert Hex string to Ports set
   *
   * @param hex
   * @return QString
   */
  QSet<qint64> HexToPorts(const QString &hex);

  /**
   * @brief Not found return message error handler
   *
   * @param called_func
   * @param item
   * @param oid
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS NotFoundReturnMessageErrorHandler(const QString &called_func, const QString &item, const QString &oid,
                                               const ActDevice &device);

  /**
   * @brief Get the Snmp Sub Tree Error Handler object
   *
   * @param called_func
   * @param status
   * @param item
   * @param oid
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS GetSnmpSubTreeErrorHandler(const QString &called_func, ACT_STATUS status, const QString &item,
                                        const QString &oid, const ActDevice &device);

  /**
   * @brief Send the request and insert to SNMP result map
   *
   * @param device
   * @param action_str
   * @param action_oid
   * @param snmp_message_result_map
   * @return ACT_STATUS
   */
  ACT_STATUS SendRequestAndInsertResultMap(const ActDevice &device, const QString &action_str,
                                           const QString &action_oid, QMap<QString, QString> &snmp_message_result_map);

  /**
   * @brief Send the request and insert to SNMP result map by bulk request
   *
   * @param device
   * @param action_str
   * @param action_oid
   * @param snmp_message_result_map
   * @return ACT_STATUS
   */
  ACT_STATUS SendRequestAndInsertResultMapByBulk(const ActDevice &device, const QString &action_str,
                                                 const QString &action_oid,
                                                 QMap<QString, QString> &snmp_message_result_map);

  /**
   * @brief Transfer StadInedexEnable SnmpMessageMap to SNMP Request list
   *
   * @param item_oid_map
   * @param snmp_message_result_map
   * @param enabled_index_snmp_message_map
   * @param request_oid_list
   * @return ACT_STATUS
   */
  ACT_STATUS StadInedexEnableSnmpMessageMapToSnmpRequestList(const QMap<QString, QString> &item_oid_map,
                                                             const QMap<QString, QString> &snmp_message_result_map,
                                                             QMap<QString, QString> &enabled_index_snmp_message_map,
                                                             QList<QString> &request_oid_list);

  /**
   * @brief SnmpMessageMap convert to StadPortEntries
   *
   * @param item_oid_map
   * @param snmp_message_map
   * @param interface_stad_port_entries
   * @return ACT_STATUS
   */
  ACT_STATUS SnmpMessageMapToInterfaceStadPort(const QMap<QString, QString> &item_oid_map,
                                               const QMap<QString, QString> &snmp_message_map,
                                               QSet<ActInterfaceStadPortEntry> &interface_stad_port_entries);

  /**
   * @brief Transfer response LldpRemOid to Port
   *
   * @param oid
   * @param response_oid
   * @param port
   * @return ACT_STATUS
   */
  ACT_STATUS LldpRemOidToPort(const QString &oid, const QString &response_oid, qint64 &port);

  //////// Refactor start

  /**
   * @brief Create a Action Oid Map object
   *
   * @param protocol_elem
   * @param use_action_key_str_set
   * @param result_action_oid_map
   * @return ACT_STATUS
   */
  ACT_STATUS CreateActionOidMap(const ActFeatureMethodProtocol &protocol_elem,
                                const QSet<QString> &use_action_key_str_set,
                                QMap<QString, QString> &result_action_oid_map);

  //////// Refactor end

 public:
  //////// Refactor start

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
   * @brief Get the String Value object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param str_value
   * @return ACT_STATUS
   */
  ACT_STATUS GetStrValue(const ActDevice &device, const QString &action_key,
                         const ActFeatureMethodProtocol &protocol_elem, QString &str_value);

  /**
   * @brief Get the Int Value object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param int_value
   * @return ACT_STATUS
   */
  ACT_STATUS GetIntValue(const ActDevice &device, const QString &action_key,
                         const ActFeatureMethodProtocol &protocol_elem, qint64 &int_value);

  /**
   * @brief Get the SysObjectId
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param sys_object_id
   * @return ACT_STATUS
   */
  ACT_STATUS GetSysObjectId(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem, quint32 &sys_object_id);

  /**
   * @brief Get the LLDP Port-Mac Map object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param port_mac_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetLldpRemPortMacMap(const ActDevice &device, const QString &action_key,
                                  const ActFeatureMethodProtocol &protocol_elem, QMap<qint64, QString> &port_mac_map);

  /**
   * @brief Get the LLDP Port-Int Map object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param port_int_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetLldpRemPortIntMap(const ActDevice &device, const QString &action_key,
                                  const ActFeatureMethodProtocol &protocol_elem, QMap<qint64, qint64> &port_int_map);

  /**
   * @brief Get the LLDP Port-String Map object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param port_str_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetLldpRemPortStrMap(const ActDevice &device, const QString &action_key,
                                  const ActFeatureMethodProtocol &protocol_elem, QMap<qint64, QString> &port_str_map);

  /**
   * @brief Get the Port Str Map object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param port_str_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetPortStrMap(const ActDevice &device, const QString &action_key,
                           const ActFeatureMethodProtocol &protocol_elem, QMap<qint64, QString> &port_str_map);

  /**
   * @brief Get the Port MAC Map object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param port_mac_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetPortMacMap(const ActDevice &device, const QString &action_key,
                           const ActFeatureMethodProtocol &protocol_elem, QMap<qint64, QString> &port_mac_map);

  /**
   * @brief Get the Port Int Map object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param port_int_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetPortIntMap(const ActDevice &device, const QString &action_key,
                           const ActFeatureMethodProtocol &protocol_elem, QMap<qint64, qint64> &port_int_map);

  /**
   * @brief Get the Port Uint Map object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param port_uint_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetPortUintMap(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem, QMap<qint64, quint64> &port_uint_map);

  /**
   * @brief Get the Port Macs Map object(MAC table)
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param port_macs_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetPortMacsMap(const ActDevice &device, const QString &action_key,
                            const ActFeatureMethodProtocol &protocol_elem, QMap<qint64, QSet<QString>> &port_macs_map);

  /**
   * @brief Get the Port FiberCheck Map object
   *
   * @param device
   * @param protocol_elem
   * @param result_port_fiber_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetPortFiberCheckMap(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                  QMap<qint64, ActMonitorFiberCheckEntry> &result_port_fiber_map);

  /**
   * @brief Get the If Out Pkts Map object
   *
   * @param device
   * @param protocol_elem
   * @param result_port_packets_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetIfOutPktsMap(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                             QMap<qint64, quint64> &result_port_packets_map);

  /**
   * @brief Get the If HC Out Pkts Map object
   *
   * @param device
   * @param protocol_elem
   * @param result_port_packets_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetIfHCOutPktsMap(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                               QMap<qint64, quint64> &result_port_packets_map);

  /**
   * @brief Get the 1588 PTP BaseClock TimeSync data object(1588-2008(v2))
   *
   * @param device
   * @param protocol_elem
   * @param result_time_sync_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetPTPBaseClock(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                             ActMonitorTimeSyncStatus &result_time_sync_status);

  /**
   * @brief Get the IEEE802.1AS TimeSync data object(802.1AS-2011)
   *
   * @param device
   * @param protocol_elem
   * @param result_time_sync_status
   * @return ACT_STATUS
   */
  ACT_STATUS GetIEEE8021AS(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                           ActMonitorTimeSyncStatus &result_time_sync_status);

  /**
   * @brief Get the Dot1qVlanStatic object
   *
   * @param device
   * @param protocol_elem
   * @param vlan_static_entries
   * @return ACT_STATUS
   */
  ACT_STATUS GetDot1qVlanStatic(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                QSet<ActVlanStaticEntry> &vlan_static_entries);
  /**
   * @brief Set the Dot1qVlanStatic object
   *
   * @param device
   * @param protocol_elem
   * @param vlan_static_entries
   * @return ACT_STATUS
   */
  ACT_STATUS SetDot1qVlanStatic(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                const QSet<ActVlanStaticEntry> &vlan_static_entries);

  /**
   * @brief Get the VLAN PortType object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param vlan_port_type_entries
   * @return ACT_STATUS
   */
  ACT_STATUS GetVlanPortType(const ActDevice &device, const QString &action_key,
                             const ActFeatureMethodProtocol &protocol_elem,
                             QSet<ActVlanPortTypeEntry> &vlan_port_type_entries);

  /**
   * @brief Get the Port VLAN Type object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param vlan_port_type_entries
   * @return ACT_STATUS
   */
  ACT_STATUS GetPortVlanType(const ActDevice &device, const QString &action_key,
                             const ActFeatureMethodProtocol &protocol_elem,
                             QSet<ActVlanPortTypeEntry> &vlan_port_type_entries);

  /**
   * @brief Set the VLAN PortType object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param vlan_port_type_entries
   * @return ACT_STATUS
   */
  ACT_STATUS SetVlanPortType(const ActDevice &device, const QString &action_key,
                             const ActFeatureMethodProtocol &protocol_elem,
                             const QSet<ActVlanPortTypeEntry> &vlan_port_type_entries);

  /**
   * @brief Set the Port Vlan Type object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param vlan_port_type_entries
   * @return ACT_STATUS
   */
  ACT_STATUS SetPortVlanType(const ActDevice &device, const QString &action_key,
                             const ActFeatureMethodProtocol &protocol_elem,
                             const QSet<ActVlanPortTypeEntry> &vlan_port_type_entries);

  /**
   * @brief Get the Port default PVID object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param port_vlan_entries
   * @return ACT_STATUS
   */
  ACT_STATUS GetPortPVID(const ActDevice &device, const QString &action_key,
                         const ActFeatureMethodProtocol &protocol_elem, QSet<ActPortVlanEntry> &port_vlan_entries);

  /**
   * @brief Set the Port default PVID object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param port_vlan_entries
   * @return ACT_STATUS
   */
  ACT_STATUS SetPortPVID(const ActDevice &device, const QString &action_key,
                         const ActFeatureMethodProtocol &protocol_elem,
                         const QSet<ActPortVlanEntry> &port_vlan_entries);

  /**
   * @brief Get the Port default PCP object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param default_priority_entries
   * @return ACT_STATUS
   */
  ACT_STATUS GetPortPCP(const ActDevice &device, const QString &action_key,
                        const ActFeatureMethodProtocol &protocol_elem,
                        QSet<ActDefaultPriorityEntry> &default_priority_entries);

  /**
   * @brief Set the Port default PCP object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param default_priority_entries
   * @return ACT_STATUS
   */
  ACT_STATUS SetPortPCP(const ActDevice &device, const QString &action_key,
                        const ActFeatureMethodProtocol &protocol_elem,
                        const QSet<ActDefaultPriorityEntry> &default_priority_entries);

  /**
   * @brief Get the InterfaceStadPort
   *
   * @param device
   * @param protocol_elem
   * @param interface_stad_port_entries
   * @return ACT_STATUS
   */
  ACT_STATUS GetInterfaceStadPort(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                  QSet<ActInterfaceStadPortEntry> &interface_stad_port_entries);

  /**
   * @brief Set the InterfaceStadPort
   *
   * @param device
   * @param protocol_elem
   * @param interface_stad_port_entries
   * @return ACT_STATUS
   */
  ACT_STATUS SetInterfaceStadPort(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                  const QSet<ActInterfaceStadPortEntry> &interface_stad_port_entries);

  /**
   * @brief Get the StadConfig
   *
   * @param device
   * @param protocol_elem
   * @param stad_config_entries
   * @return ACT_STATUS
   */
  ACT_STATUS GetStadConfig(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                           QSet<ActStadConfigEntry> &stad_config_entries);

  /**
   * @brief Set the StadConfig
   *
   * @param device
   * @param protocol_elem
   * @param stad_config_entries
   * @return ACT_STATUS
   */
  ACT_STATUS SetStadConfig(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                           const QSet<ActStadConfigEntry> &stad_config_entries);

  /**
  * @brief Get the Dot1qStatic Forward Unicast object

   *
   * @param device
   * @param protocol_elem
   * @param static_forward_entries
   * @return * ACT_STATUS
   */
  ACT_STATUS GetDot1qStaticUnicast(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                   QSet<ActStaticForwardEntry> &static_forward_entries);

  /**
   * @brief Set the Dot1qStatic Forward Unicast object
   *
   * @param device
   * @param protocol_elem
   * @param static_forward_entries
   * @return ACT_STATUS
   */
  ACT_STATUS SetDot1qStaticUnicast(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                   const QSet<ActStaticForwardEntry> &static_forward_entries);

  /**
  * @brief Get the Dot1qStatic Forward Multicast object

   *
   * @param device
   * @param protocol_elem
   * @param static_forward_entries
   * @return * ACT_STATUS
   */
  ACT_STATUS GetDot1qStaticMulticast(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                     QSet<ActStaticForwardEntry> &static_forward_entries);

  /**
   * @brief Set the Dot1qStatic Forward Multicast object
   *
   * @param device
   * @param protocol_elem
   * @param static_forward_entries
   * @return ACT_STATUS
   */
  ACT_STATUS SetDot1qStaticMulticast(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                     const QSet<ActStaticForwardEntry> &static_forward_entries);

  /**
   * @brief Set the Spanning Tree Method object
   *
   * @param device
   * @param protocol_elem
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetSpanningTreeMethod(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                   const ActRstpTable &rstp_table);

  /**
   * @brief Set the TWS switch Spanning Tree Method object
   *
   * @param device
   * @param protocol_elem
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetTWSSpanningTreeMethod(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                      const ActRstpTable &rstp_table);

  /**
   * @brief Get the Spanning Tree Method object
   *
   * @param device
   * @param protocol_elem
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetSpanningTreeMethod(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                   ActRstpTable &rstp_table);

  /**
   * @brief Get the TWS Spanning Tree Method object
   *
   * @param device
   * @param protocol_elem
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS GetTWSSpanningTreeMethod(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                      ActRstpTable &rstp_table);

  /**
   * @brief Set the Spanning Tree HelloTime object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetIeee8021SpanningTreeBridgeHelloTime(const ActDevice &device, const QString &action_key,
                                                    const ActFeatureMethodProtocol &protocol_elem,
                                                    const ActRstpTable &rstp_table);

  /**
   * @brief Set the Spanning Tree HelloTime object (ECos switch)
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetSpanningTreeHelloTime(const ActDevice &device, const QString &action_key,
                                      const ActFeatureMethodProtocol &protocol_elem, const ActRstpTable &rstp_table);

  /**
   * @brief Set the Spanning Tree Priority object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetIeee8021SpanningTreePriority(const ActDevice &device, const QString &action_key,
                                             const ActFeatureMethodProtocol &protocol_elem,
                                             const ActRstpTable &rstp_table);

  /**
   * @brief Set the Spanning Tree Priority object (Ecos switch)
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetSpanningTreePriority(const ActDevice &device, const QString &action_key,
                                     const ActFeatureMethodProtocol &protocol_elem, const ActRstpTable &rstp_table);

  /**
   * @brief Set the Spanning Tree RootGuard object
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param rstp_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetSpanningTreeRootGuard(const ActDevice &device, const QString &action_key,
                                      const ActFeatureMethodProtocol &protocol_elem, const ActRstpTable &rstp_table);
  /**
   * @brief Set the Enable Warm Start object
   *
   * @param device
   * @param protocol_elem
   * @return ACT_STATUS
   */
  ACT_STATUS SetEnableWarmStart(const ActDevice &device, const QString &action_key,
                                const ActFeatureMethodProtocol &protocol_elem);

  /**
   * @brief Set the Enable Factory Default object
   *
   * @param device
   * @param protocol_elem
   * @return ACT_STATUS
   */
  ACT_STATUS SetEnableFactoryDefault(const ActDevice &device, const QString &action_key,
                                     const ActFeatureMethodProtocol &protocol_elem);

  //////// Refactor end

  /**
   * @brief Set the RFC-1213 SysName object
   *
   * @param device
   * @param sys_name
   * @return ACT_STATUS
   */
  ACT_STATUS SetSysName(const ActDevice &device, const QString &sys_name);

  // /**
  //  * @brief Get the Lldp RemPortDesc
  //  *
  //  * @param device
  //  * @param port_oid_value_set
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS GetLldpRemPortDesc(const ActDevice &device, QSet<ActPortOidValue> &port_oid_value_set);

  /**
   * @brief Get the Ieee8021AsPropagationDelayFromHsMsLs
   *
   * @param device
   * @param port_oid_value_set
   * @return ACT_STATUS
   */
  ACT_STATUS Get8021AsPortDSNeighborPropDelayFromMsLs(const ActDevice &device,
                                                      QSet<ActPortOidValue> &port_oid_value_set);
};
#endif /* ACT_SNMP_HANDLER_H */