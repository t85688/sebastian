/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_DEPLOY_PATH_DATA_HPP
#define ACT_DEPLOY_PATH_DATA_HPP
#include "act_algorithm_configuration.hpp"
#include "act_deploy.hpp"
#include "act_device_profile.hpp"
#include "act_json.hpp"
#include "act_project.hpp"
#include "act_southbound.hpp"
#include "act_status.hpp"
#include "deploy_entry/act_deploy_table.hpp"
#include "deploy_entry/act_frer_entry.hpp"
#include "deploy_entry/act_gate_parameters.hpp"
#include "deploy_entry/act_stad_config_entry.hpp"
#include "deploy_entry/act_stad_port_entry.hpp"
#include "deploy_entry/act_static_forward_entry.hpp"
#include "deploy_entry/act_stream_identity_entry.hpp"
#include "deploy_entry/act_vlan_static_entry.hpp"
#include "stream/act_stream.hpp"
#include "topology/act_device.hpp"
#include "topology/act_link.hpp"

namespace act {
namespace deploy {

/**
 * @brief Deploy path data structure class
 *
 */
class ActDeployPathData : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActProject, act_project, ActProject);  ///< The ActProject

  // Target deploy result tables
  ACT_JSON_QT_SET_OBJECTS(ActGclTable, gcl_tables, GCLTables);                        ///< GclTable set
  ACT_JSON_QT_SET_OBJECTS(ActTeMstidTable, te_mstid_tables, TeMstidTables);           ///< TeMstidTables set
  ACT_JSON_QT_SET_OBJECTS(ActStadPortTable, stad_port_tables, StadPortTables);        ///< ActStadPortTable set
  ACT_JSON_QT_SET_OBJECTS(ActStadConfigTable, stad_config_tables, StadConfigTables);  ///< ActStadConfigTable set

  ACT_JSON_QT_SET_OBJECTS(ActVlanStaticTable, vlan_static_tables, VlanStaticTables);  ///< VlanStaticTables set
  ACT_JSON_QT_SET_OBJECTS(ActVlanPortTypeTable, vlan_port_type_tables,
                          VlanPortTypeTables);  ///< VlanPortTypeTables(hybrid, access, trunk) set

  ACT_JSON_QT_SET_OBJECTS(ActPortVlanTable, port_vlan_tables, PortVlanTables);  ///< PortVlanTables(PVID) set
  ACT_JSON_QT_SET_OBJECTS(ActDefaultPriorityTable, default_priority_tables,
                          DefaultPriorityTables);  ///< DefaultPriorityTables set

  ACT_JSON_QT_SET_OBJECTS(ActStaticForwardTable, unicast_static_forward_tables,
                          UnicastStaticForwardTables);  ///< UnicastStaticForwardTables set
  ACT_JSON_QT_SET_OBJECTS(ActStaticForwardTable, multicast_static_forward_tables,
                          MulticastStaticForwardTables);  ///< MulticastStaticForwardTables set

  ACT_JSON_QT_SET_OBJECTS(ActRstpTable, rstp_tables, RstpTables);  ///< RstpTables set
  ACT_JSON_QT_SET_OBJECTS(ActCbTable, cb_tables, CbTables);        ///< CbTables set

 private:
  quint32 stream_handle_;
  ActSouthbound southbound_;

 public:
  qint32 GetStreamHandle() { return stream_handle_++; }

  /**
   * @brief Generate GateControlList table
   *
   */
  ACT_STATUS GenerateGclTable();

  /**
   * @brief Generate Ieee802.1CB FRER table
   *
   */
  ACT_STATUS GenerateCbTable(const ActRoutingResult &routing_result);

  /**
   * @brief Generate StreamPriority table
   *
   * @param routing_result
   */
  ACT_STATUS GenerateStreamPriorityTable(const ActRoutingResult &routing_result);

  /**
   * @brief Generate VlanPortType table by Vlan static tables
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateVlanPortTypeTableByVlanStaticTables();

  /**
   * @brief Check the IPs is same subnet
   *
   * @param ip_address1
   * @param ip_address2
   * @param subnet_mask
   * @param result
   * @return ACT_STATUS
   */
  ACT_STATUS CheckIPSameSubnet(const QString &ip_address1, const QString &ip_address2, const QString &subnet_mask,
                               bool &result);

  /**
   * @brief Check StadPortEntry can be insert to Set<ActStadPortEntry>
   *
   * @param stad_port_entry_set
   * @param stad_port_entry
   * @param result
   * @return ACT_STATUS
   */
  ACT_STATUS CheckStadPortEntryCanBeInsertToSet(const QSet<ActStadPortEntry> &stad_port_entry_set,
                                                const ActStadPortEntry &stad_port_entry, bool &result);

  // StadPortEntry GenerateStadPortEntry(const qint32& index, const quint16& vlan_id, const QString& source_port,
  //                                     const Stream& stream);

  /**
   * @brief Set the StadPort Table object
   *
   * @param device
   * @param port_id
   * @param stream_priority_setting
   * @return ACT_STATUS
   */
  ACT_STATUS SetStadPortTable(const ActDevice &device, const qint64 &port_id,
                              const ActStreamPrioritySetting &stream_priority_setting);

  /**
   * @brief Set the StadConfig Table object
   *
   * @param device_id
   * @param port_id
   * @return ACT_STATUS
   */
  ACT_STATUS SetStadConfigTable(const qint64 &device_id, const qint64 &port_id);

  /**
   * @brief Get the Listener By Device Id object
   *
   * @param listeners
   * @param device_id
   * @param result_listener
   * @return ACT_STATUS
   */
  ACT_STATUS GetListenerByDeviceId(const QList<ActListener> &listeners, const qint64 &device_id,
                                   ActListener &result_listener);

  /**
   * @brief Generate VLAN static table
   *
   * @param routing_result
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateVlanStaticTable(const ActRoutingResult &routing_result);

  /**
   * @brief Set the VlanStaticEntry Table object
   *
   * @param device_id
   * @param port
   * @param vlan_priority
   * @param vlan_id
   * @param untag
   * @param te_mstid
   * @return ACT_STATUS
   */

  ACT_STATUS SetVlanStaticEntryTable(const qint64 &device_id, const qint64 &port,
                                     const ActVlanPriorityEnum &vlan_priority, const quint16 &vlan_id,
                                     const bool &untag, const bool &te_mstid);

  /**
   * @brief Add the Untag port to VlanStaticEntry
   *
   * @param device_id
   * @param port
   * @param vlan_priority
   * @param vlan_id
   * @param te_mstid
   * @return ACT_STATUS
   */
  ACT_STATUS AddUntagToVlanStaticEntry(const qint64 &device_id, const qint64 &port,
                                       const ActVlanPriorityEnum &vlan_priority, const quint16 &vlan_id,
                                       const bool &te_mstid);

  /**
   * @brief Generate PortVlan and DefaultPriority table
   *
   * @param routing_result
   * @return ACT_STATUS
   */
  ACT_STATUS GeneratePortVlanAndDefaultPriorityTable(const ActRoutingResult &routing_result);

  /**
   * @brief Set the Port Vlan Table object
   *
   * @param device_id
   * @param port_id
   * @param pvid
   * @return ACT_STATUS
   */
  ACT_STATUS SetPortVlanTable(const qint64 &device_id, const qint32 &port_id, const quint16 &pvid);

  /**
   * @brief Set the Default Priority Table object
   *
   * @param device_id
   * @param port_id
   * @param default_pcp
   * @return ACT_STATUS
   */
  ACT_STATUS SetDefaultPriorityTable(const qint64 &device_id, const qint32 &port_id, const quint8 &default_pcp);

  /**
   * @brief Generate StaticForward table
   *
   * @param routing_result
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateStaticForwardTable(const ActRoutingResult &routing_result);

  /**
   * @brief Init the RSTP Table when it not used
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS InitRSTPTableIfNotUsed(const ActDevice &device);

  /**
   * @brief Init the CbTable when it not used
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS InitCbTableIfNotUsed(const ActDevice &device);

  /**
   * @brief Init the UnicastStaticForwardTable when it not used
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS InitUnicastStaticForwardTableIfNotUsed(const ActDevice &device);

  /**
   * @brief Init the MulticastStaticForwardTable when it not used
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS InitMulticastStaticForwardTableIfNotUsed(const ActDevice &device);

  /**
   * @brief Init the StadPortTable when it not used
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS InitStadPortTableIfNotUsed(const ActDevice &device);

  /**
   * @brief Insert disable entry to StadConfigTable
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS InsertDisableToStadConfigTable(const ActDevice &device);

  /**
   * @brief Insert disable entry to GCL table
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS InsertDisableToGclTable(const ActDevice &device);

  /**
   * @brief Insert init entry to VlanPortTypeTable
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS InsertInitEntryToVlanPortTypeTable(const ActDevice &device);

  /**
   * @brief Insert init PVID(1) entry to VlanStaticTable
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS InsertInitPVIDEntryToVlanStaticTable(const ActDevice &device);

  /**
   * @brief Insert init entry to PortVlanTable
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS InsertInitEntryToPortVlanTable(const ActDevice &device);

  /**
   * @brief Insert init entry to DefaultPriorityTable
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS InsertInitEntryToDefaultPriorityTable(const ActDevice &device);

 public:
  ActDeployPathData() { stream_handle_ = 1; }
  ActDeployPathData(const ActProject &act_project) : ActDeployPathData() { act_project_ = act_project; }

  /**
   * @brief Generate the deploy data path
   *
   */
  ACT_STATUS GenerateData();

  /**
   * @brief Only Generate the VlanStaticData
   *
   */
  ACT_STATUS GenerateVlanStaticData();
};

}  // namespace deploy
}  // namespace act
#endif  // ACT_DEPLOY_PATH_DATA_HPP