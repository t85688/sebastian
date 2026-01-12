#include "act_deploy_path_data.hpp"

#include <QtCore/qmath.h>

#include <QDebug>

#include "act_device_profile.hpp"
using namespace act::deploy;

#define ACT_TIME_DENOMINATOR_NANO 1000000000ll  ///< The denominator of Admin Cycle Time, nano time is 1000000000ll
// Ref: https://blog.csdn.net/momo0853/article/details/116719172

ACT_STATUS ActDeployPathData::GenerateVlanStaticData() {
  ACT_STATUS_INIT();

  // Generate VLAN Static
  for (auto routing_result : act_project_.GetComputedResult().GetRoutingResults()) {
    act_status = GenerateVlanStaticTable(routing_result);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenerateVlanStaticTable() failed.";
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS ActDeployPathData::GenerateData() {
  ACT_STATUS_INIT();

  // Generate GCL
  act_status = GenerateGclTable();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GenerateGclTable() failed.";
    return act_status;
  }

  // Advanced mode

  // Generate config by ComputeResult (StreamSetting)
  // StreamPriority, VLAN, PVID & PCP, StaticForward, CB
  for (auto routing_result : act_project_.GetComputedResult().GetRoutingResults()) {
    // Generate StreamPriority(by StreamSetting)
    act_status = GenerateStreamPriorityTable(routing_result);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenerateStreamPriorityTable() failed.";
      return act_status;
    }

    // Generate VLAN Static
    act_status = GenerateVlanStaticTable(routing_result);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenerateVlanStaticTable() failed.";
      return act_status;
    }

    // Generate PortVlan & PortDefaultPriority(PVID & PCP)
    act_status = GeneratePortVlanAndDefaultPriorityTable(routing_result);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GeneratePortVlanAndDefaultPriorityTable() failed.";
      return act_status;
    }

    // Generate Static Forward
    act_status = GenerateStaticForwardTable(routing_result);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenerateStaticForwardTable() failed.";
      return act_status;
    }

    // Generate 802.1CB Table
    act_status = GenerateCbTable(routing_result);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenerateCbTable() failed.";
      return act_status;
    }
  }
  // }

  // Generate Vlan Port Type
  act_status = GenerateVlanPortTypeTableByVlanStaticTables();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GenerateVlanPortTypeTableByVlanStaticTables() failed.";
    return act_status;
  }

  // [feat:2729] Deploy - Reset unused device configuration
  // Init not use table: CB, UnicastStaticForward & MulticastStaticForward & StreamPriorityIngress
  // Insert StreamPriorityEgress(disable) & GCL(disable) & VlanPortType(Hybrid(3))
  // & PVID(1) & Default_PCP(0) to not use interface at each switch
  for (auto device : act_project_.GetDevices()) {
    auto dev_config_feat_group = device.GetDeviceProperty().GetFeatureGroup().GetConfiguration();

    // // RSTPTable(empty table)
    // auto rstp_group = dev_config_feat_group.GetSTPRSTP();
    // if (rstp_group.GetHelloTime() || rstp_group.GetPriority()) {
    //   act_status = InitRSTPTableIfNotUsed(device);
    //   if (!IsActStatusSuccess(act_status)) {
    //     qCritical() << __func__ << "InitRSTPTableIfNotUsed() failed.";
    //     return act_status;
    //   }
    // }

    // CBTable(empty table)
    if (dev_config_feat_group.GetTSN().GetIEEE802Dot1CB()) {
      act_status = InitCbTableIfNotUsed(device);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "InitCbTableIfNotUsed() failed.";
        return act_status;
      }
    }

    // UnicastStaticForwardTable(empty table)
    // If the device not use UnicastStaticForwardTable would init this table
    // if (dev_config_feat_group.GetStaticForwardSetting().GetUnicast()) {
    //   act_status = InitUnicastStaticForwardTableIfNotUsed(device);
    //   if (!IsActStatusSuccess(act_status)) {
    //     qCritical() << __func__ << "InitUnicastStaticForwardTableIfNotUsed() failed.";
    //     return act_status;
    //   }
    // }

    // MulticastStaticForwardTable(empty table)
    // If the device not use MulticastStaticForwardTable would init this table
    // if (dev_config_feat_group.GetStaticForwardSetting().GetMulticast()) {
    //   act_status = InitMulticastStaticForwardTableIfNotUsed(device);
    //   if (!IsActStatusSuccess(act_status)) {
    //     qCritical() << __func__ << "InitMulticastStaticForwardTableIfNotUsed() failed.";
    //     return act_status;
    //   }
    // }

    // StreamPriorityIngress(empty table)
    // If the device not use StadPortTable would init this table
    // if (dev_config_feat_group.GetVLANSetting().GetPerStreamPriority() ||
    //     dev_config_feat_group.GetVLANSetting().GetPerStreamPriorityV2()) {
    //   act_status = InitStadPortTableIfNotUsed(device);
    //   if (!IsActStatusSuccess(act_status)) {
    //     qCritical() << __func__ << "InsertEmptyTableToStadPortTable() failed.";
    //     return act_status;
    //   }
    // }

    // StreamPriorityEgress(disable)
    // PerStreamPriorityV2 not support egress
    // if (dev_config_feat_group.GetVLANSetting().GetPerStreamPriority()) {
    //   act_status = InsertDisableToStadConfigTable(device);
    //   if (!IsActStatusSuccess(act_status)) {
    //     qCritical() << __func__ << "InsertDisableToStadConfigTable() failed.";
    //     return act_status;
    //   }
    // }

    // VlanPortType(Access)
    // if (dev_config_feat_group.GetVLANSetting().GetAccessTrunkMode()) {
    //   act_status = InsertInitEntryToVlanPortTypeTable(device);
    //   if (!IsActStatusSuccess(act_status)) {
    //     qCritical() << __func__ << "InsertInitEntryToVlanPortTypeTable() failed.";
    //     return act_status;
    //   }
    // }

    // [bugfix:2629] Unused ports should be reverted to Access type & VLAN 1
    // VlanStaticEntry(VLAN 1)
    // if (dev_config_feat_group.GetVLANSetting().GetAccessTrunkMode()) {
    //   act_status = InsertInitPVIDEntryToVlanStaticTable(device);
    //   if (!IsActStatusSuccess(act_status)) {
    //     qCritical() << __func__ << "InsertInitPVIDEntryToVlanStaticTable() failed.";
    //     return act_status;
    //   }
    // }

    // PVID(1)
    // if (dev_config_feat_group.GetVLANSetting().GetDefaultPVID()) {
    //   act_status = InsertInitEntryToPortVlanTable(device);
    //   if (!IsActStatusSuccess(act_status)) {
    //     qCritical() << __func__ << "InsertInitEntryToPortVlanTable() failed.";
    //     return act_status;
    //   }
    // }

    // Default_PCP(0)
    // if (dev_config_feat_group.GetVLANSetting().GetDefaultPCP()) {
    //   act_status = InsertInitEntryToDefaultPriorityTable(device);
    //   if (!IsActStatusSuccess(act_status)) {
    //     qCritical() << __func__ << "InsertInitEntryToDefaultPriorityTable() failed.";
    //     return act_status;
    //   }
    // }

    // GCL(disable)
    // if (dev_config_feat_group.GetTSN().GetIEEE802Dot1Qbv()) {
    //   act_status = InsertDisableToGclTable(device);
    //   if (!IsActStatusSuccess(act_status)) {
    //     qCritical() << __func__ << "InsertDisableToGclTable() failed.";
    //     return act_status;
    //   }
    // }
  }

  // qDebug() << __func__ << "DeployPathData::GenerateData() Done";

  // qDebug() << __func__ << "---------- DeployPathData (Start)--------";
  // for (auto table : gcl_tables_) {
  //   qDebug() << __func__ << "gcl_tables_:" << table.ToString().toStdString().c_str();
  // }
  // for (auto table : stad_port_tables_) {
  //   qDebug() << __func__ << "stad_port_tables_:" << table.ToString().toStdString().c_str();
  // }
  // for (auto table : stad_config_tables_) {
  //   qDebug() << __func__ << "stad_config_tables_:" << table.ToString().toStdString().c_str();
  // }
  // for (auto table : vlan_static_tables_) {
  //   qDebug() << __func__ << "vlan_static_tables_:" << table.ToString().toStdString().c_str();
  // }
  // for (auto table : vlan_port_type_tables_) {
  //   qDebug() << __func__ << "vlan_port_type_tables_:" << table.ToString().toStdString().c_str();
  // }
  // for (auto table : port_vlan_tables_) {
  //   qDebug() << __func__ << "port_vlan_tables_:" << table.ToString().toStdString().c_str();
  // }
  // for (auto table : default_priority_tables_) {
  //   qDebug() << __func__ << "default_priority_tables_:" << table.ToString().toStdString().c_str();
  // }
  // for (auto table : unicast_static_forward_tables_) {
  //   qDebug() << __func__ << "unicast_static_forward_tables_:" << table.ToString().toStdString().c_str();
  // }
  // for (auto table : multicast_static_forward_tables_) {
  //   qDebug() << __func__ << "multicast_static_forward_tables_:" << table.ToString().toStdString().c_str();
  // }
  // for (auto table : cb_tables_) {
  //   qDebug() << __func__ << "cb_tables_:" << table.ToString().toStdString().c_str();
  // }
  // qDebug() << __func__ << "---------- DeployPathData (End)--------";
  return act_status;
}
ACT_STATUS ActDeployPathData::GenerateVlanPortTypeTableByVlanStaticTables() {
  ACT_STATUS_INIT();

  for (auto vlan_static_table : vlan_static_tables_) {
    // Find device
    ActDevice device;
    act_status = ActGetItemById<ActDevice>(act_project_.GetComputedResult().GetDevices(),
                                           vlan_static_table.GetDeviceId(), device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Device not found. Device:" << vlan_static_table.GetDeviceId();
      return std::make_shared<ActStatusInternalError>("Deploy");
    }

    QSet<ActVlanPortTypeEntry> vlan_port_type_entry_set;
    for (auto entry : vlan_static_table.GetVlanStaticEntries()) {
      // Egress port

      for (auto port : entry.GetEgressPorts()) {
        // [bugfix:881] Deploy sequence should go from far to near
        if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) {
          vlan_port_type_entry_set.insert(
              ActVlanPortTypeEntry(port, ActVlanPortTypeEnum::kHybrid, ActVlanPriorityEnum::kTSNSystem));  // hybrid
        } else {
          vlan_port_type_entry_set.insert(
              ActVlanPortTypeEntry(port, ActVlanPortTypeEnum::kTrunk, ActVlanPriorityEnum::kTSNSystem));  // trunk
        }
      }

      // Untag port (only hybrid)
      // [feat: 592] untag for hybrid
      for (auto port : entry.GetUntaggedPorts()) {
        // [bugfix:881] Deploy sequence should go from far to near
        if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) {
          vlan_port_type_entry_set.insert(
              ActVlanPortTypeEntry(port, ActVlanPortTypeEnum::kHybrid, ActVlanPriorityEnum::kTSNSystem));  // hybrid
        }
      }
    }
    vlan_port_type_tables_.insert(ActVlanPortTypeTable(device.GetId(), vlan_port_type_entry_set));
  }

  return act_status;
}

ACT_STATUS ActDeployPathData::InsertInitPVIDEntryToVlanStaticTable(const ActDevice &device) {
  ACT_STATUS_INIT();

  // ActVlanStaticTable vlan_static_table(device.GetId());
  // QSet<ActVlanStaticTable>::iterator vlan_static_table_iter;
  // QSet<ActVlanStaticEntry> vlan_static_entry_set;

  // // Find the vlan_static_table in vlan_static_tables_
  // vlan_static_table_iter = vlan_static_tables_.find(vlan_static_table);
  // if (vlan_static_table_iter != vlan_static_tables_.end()) {  // found
  //   vlan_static_entry_set = vlan_static_table_iter->GetVlanStaticEntries();
  //   vlan_static_tables_.erase(vlan_static_table_iter);
  // }

  // Find the vlan_port_type_table in vlan_port_type_tables_
  ActVlanPortTypeTable vlan_port_type_table(device.GetId());
  QSet<ActVlanPortTypeTable>::iterator vlan_port_type_table_iter;
  QSet<ActVlanPortTypeEntry> vlan_port_type_entry_set;
  vlan_port_type_table_iter = vlan_port_type_tables_.find(vlan_port_type_table);
  if (vlan_port_type_table_iter == vlan_port_type_tables_.end()) {  // not found
    qCritical() << __func__ << "VlanPortTypeTable not found. Device:" << device.GetIpv4().GetIpAddress();
    return std::make_shared<ActStatusInternalError>("Deploy");
  }

  // Create the VlanStaticEntry by VlanPortTypeEntry
  for (auto vlan_port_type_entry : vlan_port_type_table_iter->GetVlanPortTypeEntries()) {
    if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kTrunk) {
      SetVlanStaticEntryTable(device.GetId(), vlan_port_type_entry.GetPortId(), ActVlanPriorityEnum::kTSNSystem,
                              ACT_VLAN_INIT_PVID, false, false);
    } else {  // Hybrid & Access
      SetVlanStaticEntryTable(device.GetId(), vlan_port_type_entry.GetPortId(), ActVlanPriorityEnum::kTSNSystem,
                              ACT_VLAN_INIT_PVID, true, false);
    }
  }

  // for (auto interface : device.GetInterfaces()) {
  //   // Because this is the SET, the insert entry doesn't cover the exists entry.
  //   // if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) {
  //   //   vlan_static_entry_set.insert(ActVlanPortTypeEntry(interface.GetInterfaceId(),
  //   //   ActVlanPortTypeEnum::kHybrid));
  //   // } else {
  //   //   vlan_static_entry_set.insert(ActVlanPortTypeEntry(interface.GetInterfaceId(),
  //   //   ActVlanPortTypeEnum::kAccess));
  //   // }
  //   // [bugfix:2629] Deploy - Unused ports should be reverted to Access & VLAN 1
  //   vlan_static_entry_set.insert(ActVlanPortTypeEntry(interface.GetInterfaceId(), ActVlanPortTypeEnum::kAccess));
  // }

  // // // Find the entry(VID 1)
  // ActVlanStaticEntry vlan_static_entry(ACT_VLAN_INIT_PVID);
  // auto vlan_static_entry_iter = vlan_static_entry_set.find(vlan_static_entry);

  // vlan_static_table.SetVlanPortTypeEntries(vlan_static_entry_set);
  // vlan_static_tables_.insert(vlan_static_table);

  return act_status;
}

ACT_STATUS ActDeployPathData::InsertDisableToStadConfigTable(const ActDevice &device) {
  ACT_STATUS_INIT();

  ActStadConfigTable stad_config_table(device.GetId());
  QSet<ActStadConfigTable>::iterator stad_config_table_iter;
  QSet<ActStadConfigEntry> stad_config_entry_set;

  // Find the stad_config_table in stad_config_tables_
  stad_config_table_iter = stad_config_tables_.find(stad_config_table);
  if (stad_config_table_iter != stad_config_tables_.end()) {  // found
    stad_config_entry_set = stad_config_table_iter->GetStadConfigEntries();
    stad_config_tables_.erase(stad_config_table_iter);
  }

  for (auto interface : device.GetInterfaces()) {
    // Because this is the SET, the insert entry doesn't cover the exists entry.
    // disable(2)
    stad_config_entry_set.insert(ActStadConfigEntry(interface.GetInterfaceId(), 2));
  }
  stad_config_table.SetStadConfigEntries(stad_config_entry_set);
  stad_config_tables_.insert(stad_config_table);

  return act_status;
}

ACT_STATUS act::deploy::ActDeployPathData::InitRSTPTableIfNotUsed(const ActDevice &device) {
  ACT_STATUS_INIT();

  ActRstpTable rstp_table(device);
  QSet<ActRstpTable>::iterator rstp_table_iter;

  // Find the rstp_table in unicast_rstp_tables
  rstp_table_iter = rstp_tables_.find(rstp_table);
  if (rstp_table_iter != rstp_tables_.end()) {  // found
    return act_status;
  }

  rstp_tables_.insert(rstp_table);
  return act_status;
}

ACT_STATUS act::deploy::ActDeployPathData::InitCbTableIfNotUsed(const ActDevice &device) {
  ACT_STATUS_INIT();

  ActCbTable cb_table(device.GetId());
  QSet<ActCbTable>::iterator cb_table_iter;

  // Find the cb_table in unicast_cb_tables
  cb_table_iter = cb_tables_.find(cb_table);
  if (cb_table_iter != cb_tables_.end()) {  // found
    return act_status;
  }

  cb_tables_.insert(cb_table);
  return act_status;
}

ACT_STATUS ActDeployPathData::InitUnicastStaticForwardTableIfNotUsed(const ActDevice &device) {
  ACT_STATUS_INIT();

  ActStaticForwardTable static_forward_table(device.GetId());
  QSet<ActStaticForwardTable>::iterator static_forward_table_iter;

  // Find the static_forward_table in unicast_static_forward_tables
  static_forward_table_iter = unicast_static_forward_tables_.find(static_forward_table);
  if (static_forward_table_iter != unicast_static_forward_tables_.end()) {  // found
    return act_status;
  }

  unicast_static_forward_tables_.insert(static_forward_table);

  return act_status;
}

ACT_STATUS ActDeployPathData::InitMulticastStaticForwardTableIfNotUsed(const ActDevice &device) {
  ACT_STATUS_INIT();

  ActStaticForwardTable static_forward_table(device.GetId());
  QSet<ActStaticForwardTable>::iterator static_forward_table_iter;

  // Find the static_forward_table in multicast_static_forward_tables
  static_forward_table_iter = multicast_static_forward_tables_.find(static_forward_table);
  if (static_forward_table_iter != multicast_static_forward_tables_.end()) {  // found
    return act_status;
  }

  multicast_static_forward_tables_.insert(static_forward_table);

  return act_status;
}

ACT_STATUS ActDeployPathData::InitStadPortTableIfNotUsed(const ActDevice &device) {
  ACT_STATUS_INIT();

  ActStadPortTable stad_port_table(device.GetId());
  QSet<ActStadPortTable>::iterator stad_port_table_iter;

  // Find the stad_port_table in stad_port_tables_
  stad_port_table_iter = stad_port_tables_.find(stad_port_table);
  if (stad_port_table_iter != stad_port_tables_.end()) {  // found
    return act_status;
  }

  stad_port_tables_.insert(stad_port_table);

  return act_status;
}

ACT_STATUS ActDeployPathData::InsertInitEntryToVlanPortTypeTable(const ActDevice &device) {
  ACT_STATUS_INIT();

  ActVlanPortTypeTable vlan_port_type_table(device.GetId());
  QSet<ActVlanPortTypeTable>::iterator vlan_port_type_table_iter;
  QSet<ActVlanPortTypeEntry> vlan_port_type_entry_set;

  // Find the vlan_port_type_table in vlan_port_type_tables_
  vlan_port_type_table_iter = vlan_port_type_tables_.find(vlan_port_type_table);
  if (vlan_port_type_table_iter != vlan_port_type_tables_.end()) {  // found
    vlan_port_type_entry_set = vlan_port_type_table_iter->GetVlanPortTypeEntries();
    vlan_port_type_tables_.erase(vlan_port_type_table_iter);
  }

  for (auto interface : device.GetInterfaces()) {
    // Because this is the SET, the insert entry doesn't cover the exists entry.
    // if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) {
    //   vlan_port_type_entry_set.insert(ActVlanPortTypeEntry(interface.GetInterfaceId(),
    //   ActVlanPortTypeEnum::kHybrid));
    // } else {
    //   vlan_port_type_entry_set.insert(ActVlanPortTypeEntry(interface.GetInterfaceId(),
    //   ActVlanPortTypeEnum::kAccess));
    // }
    // [bugfix:2629] Deploy - Unused ports should be reverted to Access & VLAN 1
    vlan_port_type_entry_set.insert(
        ActVlanPortTypeEntry(interface.GetInterfaceId(), ActVlanPortTypeEnum::kAccess, ActVlanPriorityEnum::kNonTSN));
  }
  vlan_port_type_table.SetVlanPortTypeEntries(vlan_port_type_entry_set);
  vlan_port_type_tables_.insert(vlan_port_type_table);

  return act_status;
}

ACT_STATUS ActDeployPathData::InsertInitEntryToPortVlanTable(const ActDevice &device) {
  ACT_STATUS_INIT();

  ActPortVlanTable port_vlan_table(device.GetId());
  QSet<ActPortVlanTable>::iterator port_vlan_table_iter;
  QSet<ActPortVlanEntry> port_vlan_entry_set;

  // Find the port_vlan_table in port_vlan_tables_
  port_vlan_table_iter = port_vlan_tables_.find(port_vlan_table);
  if (port_vlan_table_iter != port_vlan_tables_.end()) {  // found
    port_vlan_entry_set = port_vlan_table_iter->GetPortVlanEntries();
    port_vlan_tables_.erase(port_vlan_table_iter);
  }

  for (auto interface : device.GetInterfaces()) {
    // Because this is the SET, the insert entry doesn't cover the exists entry.
    port_vlan_entry_set.insert(
        ActPortVlanEntry(interface.GetInterfaceId(), ACT_VLAN_INIT_PVID, ActVlanPriorityEnum::kNonTSN));
  }
  port_vlan_table.SetPortVlanEntries(port_vlan_entry_set);
  port_vlan_tables_.insert(port_vlan_table);

  return act_status;
}

ACT_STATUS ActDeployPathData::InsertInitEntryToDefaultPriorityTable(const ActDevice &device) {
  ACT_STATUS_INIT();

  ActDefaultPriorityTable default_priority_table(device.GetId());
  QSet<ActDefaultPriorityTable>::iterator default_priority_table_iter;
  QSet<ActDefaultPriorityEntry> default_priority_entry_set;

  // Find the default_priority_table in default_priority_tables_
  default_priority_table_iter = default_priority_tables_.find(default_priority_table);
  if (default_priority_table_iter != default_priority_tables_.end()) {  // found
    default_priority_entry_set = default_priority_table_iter->GetDefaultPriorityEntries();
    default_priority_tables_.erase(default_priority_table_iter);
  }

  for (auto interface : device.GetInterfaces()) {
    // Because this is the SET, the insert entry doesn't cover the exists entry.
    default_priority_entry_set.insert(ActDefaultPriorityEntry(interface.GetInterfaceId(), ACT_INIT_DEFAULT_PCP));
  }
  default_priority_table.SetDefaultPriorityEntries(default_priority_entry_set);
  default_priority_tables_.insert(default_priority_table);

  return act_status;
}

ACT_STATUS ActDeployPathData::InsertDisableToGclTable(const ActDevice &device) {
  ACT_STATUS_INIT();

  ActGclTable gcl_table(device.GetId());
  QSet<ActGclTable>::iterator gcl_table_iter;
  QSet<ActInterfaceGateParameters> gcl_parameters_set;

  // Find the gcl_table in gcl_tables_
  gcl_table_iter = gcl_tables_.find(gcl_table);
  if (gcl_table_iter != gcl_tables_.end()) {  // found
    gcl_parameters_set = gcl_table_iter->GetInterfacesGateParameters();
    gcl_tables_.erase(gcl_table_iter);
  }

  for (auto interface : device.GetInterfaces()) {
    // Because this is the SET, the insert entry doesn't cover the exists entry.
    ActGateParameters new_gate_parameters;
    new_gate_parameters.SetGateEnabled(false);
    gcl_parameters_set.insert(ActInterfaceGateParameters(interface.GetInterfaceId(), new_gate_parameters));
  }
  gcl_table.SetInterfacesGateParameters(gcl_parameters_set);
  gcl_tables_.insert(gcl_table);

  return act_status;
}

ACT_STATUS ActDeployPathData::CheckIPSameSubnet(const QString &ip_address1, const QString &ip_address2,
                                                const QString &subnet_mask, bool &result) {
  ACT_STATUS_INIT();

  QHostAddress ip1(ip_address1);
  QHostAddress ip2(ip_address2);
  QHostAddress mask(subnet_mask);

  if (ip1.protocol() != QAbstractSocket::IPv4Protocol || ip2.protocol() != QAbstractSocket::IPv4Protocol ||
      mask.protocol() != QAbstractSocket::IPv4Protocol) {
    // Make sure all addresses are IPv4
    qCritical() << __func__ << "The address is not in IPv4 format";
    return std::make_shared<ActStatusInternalError>("CheckIPSameSubnet");
  }

  quint32 ip1_value = ip1.toIPv4Address();
  quint32 ip2_value = ip2.toIPv4Address();
  quint32 mask_value = mask.toIPv4Address();

  // Calculate the network portion of the IPs using the subnet mask
  quint32 network1 = ip1_value & mask_value;
  quint32 network2 = ip2_value & mask_value;

  // Check if the network portions are the same
  result = (network1 == network2);
  return act_status;
}

ACT_STATUS ActDeployPathData::GenerateStaticForwardTable(const ActRoutingResult &routing_result) {
  ACT_STATUS_INIT();

  QSet<ActStaticForwardTable> static_forward_tables;

  // Get StreamSetting (Destination MAC address (Unicast / Multicast))
  ActTrafficStream stream_setting;
  act_status = act_project_.GetComputedResult().GetTrafficDesign().GetStreamSettingByStreamID(
      routing_result.GetStreamId(), stream_setting);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetStreamSettingByStreamID() failed";
    return act_status;
  }

  if (!routing_result.GetMulticast()) {  // Unicast
    static_forward_tables = unicast_static_forward_tables_;
  } else {  // Multicast
    static_forward_tables = multicast_static_forward_tables_;
  }

  for (auto routing_path : routing_result.GetRoutingPaths()) {  // RoutingPaths layer(multiple / single paths)
    QSet<qint64> used_link_ids;
    for (auto path : routing_path.GetRedundantPaths()) {  // Paths layer (CB or not)
      QList<qint64> device_ids = path.GetDeviceIds();
      QList<qint64> link_ids = path.GetLinkIds();
      for (int i = 0; i < device_ids.size(); i++) {
        qint64 device_id = device_ids[i];
        ActDevice device;
        act_status = ActGetItemById<ActDevice>(act_project_.GetComputedResult().GetDevices(), device_id, device);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Source device not found. Device:" << device_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }

        qint64 next_device_id = (i < link_ids.size()) ? device_ids[i + 1] : 0;
        ActDevice next_device;
        act_status =
            ActGetItemById<ActDevice>(act_project_.GetComputedResult().GetDevices(), next_device_id, next_device);
        if ((i < link_ids.size()) && !IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Next device not found. Device:" << next_device_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }

        qint64 ingress_link_id = (i > 0) ? link_ids[i - 1] : 0;
        ActLink ingress_link;
        act_status =
            ActGetItemById<ActLink>(act_project_.GetComputedResult().GetLinks(), ingress_link_id, ingress_link);
        if ((i > 0) && !IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Link not found. Link:" << ingress_link_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }
        qint64 ingress_interface_id = (i > 0) ? (ingress_link.GetSourceDeviceId() == device_id)
                                                    ? ingress_link.GetSourceInterfaceId()
                                                    : ingress_link.GetDestinationInterfaceId()
                                              : 0;

        qint64 egress_link_id = (i < link_ids.size()) ? link_ids[i] : 0;
        ActLink egress_link;
        act_status = ActGetItemById<ActLink>(act_project_.GetComputedResult().GetLinks(), egress_link_id, egress_link);
        if ((i < link_ids.size()) && !IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Link not found. Link:" << egress_link_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }
        qint64 egress_interface_id = (i < link_ids.size()) ? (egress_link.GetSourceDeviceId() == device_id)
                                                                 ? egress_link.GetSourceInterfaceId()
                                                                 : egress_link.GetDestinationInterfaceId()
                                                           : 0;

        // Skip to set vlan_entry
        if (used_link_ids.contains(ingress_link_id) && used_link_ids.contains(egress_link_id)) {
          used_link_ids.insert(ingress_link_id);
          continue;
        }

        //  Skip to set vlan_entry by FeatureGroup (Unicast or Multicast)
        if (!routing_result.GetMulticast()) {  // Unicast
          if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetStaticForwardSetting().GetUnicast()) {
            used_link_ids.insert(ingress_link_id);
            continue;
          }
        } else {  // Multicast
          if (!device.GetDeviceProperty()
                   .GetFeatureGroup()
                   .GetConfiguration()
                   .GetStaticForwardSetting()
                   .GetMulticast()) {
            used_link_ids.insert(ingress_link_id);
            continue;
          }
        }

        // Find StaticForwardTable
        ActStaticForwardTable static_forward_table(device_id);
        QSet<ActStaticForwardTable>::iterator static_forward_table_iter;
        static_forward_table_iter = static_forward_tables.find(static_forward_table);
        if (static_forward_table_iter != static_forward_tables.end()) {  // found
          static_forward_table = *static_forward_table_iter;
          static_forward_tables.erase(static_forward_table_iter);
        }

        ActStaticForwardEntry static_forward_entry;
        static_forward_entry.SetMAC(stream_setting.GetDestinationMac());
        if (used_link_ids.contains(ingress_link_id) && !used_link_ids.contains(egress_link_id)) {  // split
          static_forward_entry.SetVlanId(routing_result.GetVlanId());
        } else {
          static_forward_entry.SetVlanId(path.GetVlanId());
        }

        QSet<ActStaticForwardEntry>::iterator static_forward_entry_iter;
        QSet<ActStaticForwardEntry> &static_forward_entries_set = static_forward_table.GetStaticForwardEntries();
        static_forward_entry_iter = static_forward_entries_set.find(static_forward_entry);
        if (static_forward_entry_iter != static_forward_entries_set.end()) {  // Has found
          static_forward_entry = *static_forward_entry_iter;
          static_forward_entries_set.erase(static_forward_entry_iter);
        }
        // static_forward_entry.SetIngressPort(ingress_interface_id);

        static_forward_entry.InsertEgressPort(egress_interface_id);
        // Use static_forward_entry construct default
        // static_forward_entry.SetStorageType(2);
        // static_forward_entry.SetRowStatus(1);
        // static_forward_entry.SetDot1qStatus(3);

        static_forward_entries_set.insert(static_forward_entry);
        static_forward_tables.insert(static_forward_table);
        used_link_ids.insert(ingress_link_id);
      }
    }
  }

  if (!routing_result.GetMulticast()) {  // Unicast
    unicast_static_forward_tables_ = static_forward_tables;
  } else {
    multicast_static_forward_tables_ = static_forward_tables;
  }

  // Remove: [feat:3250] Southbound - Remove/Hide PVID & Mgnt. Endpoint feature
  // // [feat:2540] UI Feature - Allow users to specify management interfaces
  // // Add the unicast static_forward entry by management interfaces
  // if (!stream.GetTagged() && stream.GetUntaggedMode() == ActTrafficUntaggedModeEnum::kPVID) {
  //   QSet<ActStaticForwardTable> unicast_static_forward_tables;
  //   unicast_static_forward_tables = unicast_static_forward_tables_;

  //   // Get ACT Host adapters
  //   QMap<QString, QString> adapter_table;  // <IP, MAC>
  //   act_status = southbound_.GetLocalhostAdapterTable(adapter_table);
  //   if (!IsActStatusSuccess(act_status)) {
  //     qCritical() << __func__ << "GetLocalhostAdapterTable() failed.";
  //     return act_status;
  //   }

  //   qint16 pvid = routing_result.GetVlanId();
  //   qint64 talker_id = stream.GetTalker().GetEndStationInterface().GetDeviceId();
  //   qint64 talker_intf = stream.GetTalker().GetEndStationInterface().GetInterfaceId();

  //   ActHostPair host_pair;
  //   host_pair.SetSourceDeviceId(talker_id);
  //   host_pair.SetSourceInterfaceId(talker_intf);

  //   QMap<qint64, QList<ActRoutingLink>> links_from_nodes;
  //   ActAlgorithm compute;
  //   compute.PrepareLinksFromNodes(links_from_nodes, act_project_);

  // for (auto mgmt_interface : act_project_.GetTopologySetting().GetManagementInterfaces()) {
  //   ActDevice device;
  //   act_status = act_project_.GetDeviceById(device, mgmt_interface.GetDeviceId());
  //   if (IsActStatusNotFound(act_status)) {
  //     qDebug() << __func__
  //              << QString("Device(%1) not found in project, so skip configuring it ManagementInterfaces")
  //                     .arg(mgmt_interface.GetDeviceId());
  //     continue;
  //   }

  //   // Get ACT host MAC
  //   QSet<QString> mac_list;  // Adapter's IP and Device's IP are in the same subnet
  //   const QString subnet_mask = "255.255.255.0";
  //   for (auto adapter_ip : adapter_table.keys()) {
  //     bool check_result = false;
  //     CheckIPSameSubnet(adapter_ip, device.GetIpv4().GetIpAddress(), subnet_mask, check_result);

  //     if (check_result) {  // same subnet
  //       mac_list.insert(adapter_table[adapter_ip]);
  //     }
  //   }

  //   host_pair.SetDestinationDeviceId(mgmt_interface.GetDeviceId());
  //   for (ActRoutingLink routing_link : links_from_nodes[mgmt_interface.GetDeviceId()]) {
  //     if (mgmt_interface.GetInterfaces().contains(routing_link.GetSourceInterfaceId())) {
  //       continue;
  //     }
  //     host_pair.SetDestinationInterfaceId(routing_link.GetSourceInterfaceId());

  //     QList<ActRedundantPath> redundant_paths;
  //     compute.DoMultiplePathRoutingForStream(redundant_paths, links_from_nodes, host_pair,
  //                                            {ActDeviceTypeEnum::kTSNSwitch, ActDeviceTypeEnum::kBridgedEndStation,
  //                                             ActDeviceTypeEnum::kSwitch, ActDeviceTypeEnum::kEndStation});

  //     for (ActRedundantPath redundant_path : redundant_paths) {
  //       QList<qint64> device_ids = redundant_path.GetDeviceIds();
  //       QList<qint64> link_ids = redundant_path.GetLinkIds();
  //       for (int i = 1; i < device_ids.size(); i++) {
  //         qint64 device_id = device_ids[i];
  //         qint64 ingress_link_id = link_ids[i - 1];
  //         ActLink ingress_link;
  //         act_status =
  //             ActGetItemById<ActLink>(act_project_.GetComputedResult().GetLinks(), ingress_link_id, ingress_link);
  //         if ((i > 0) && !IsActStatusSuccess(act_status)) {
  //           qCritical() << __func__ << "Link not found. Link:" << ingress_link_id;
  //           return std::make_shared<ActStatusInternalError>("Deploy");
  //         }
  //         qint64 ingress_interface_id = (ingress_link.GetSourceDeviceId() == device_id)
  //                                           ? ingress_link.GetSourceInterfaceId()
  //                                           : ingress_link.GetDestinationInterfaceId();

  //         qint64 egress_link_id = (i < link_ids.size()) ? link_ids[i] : 0;
  //         ActLink egress_link;
  //         act_status =
  //             ActGetItemById<ActLink>(act_project_.GetComputedResult().GetLinks(), egress_link_id, egress_link);
  //         if ((i < link_ids.size()) && !IsActStatusSuccess(act_status)) {
  //           qCritical() << __func__ << "Link not found. Link:" << egress_link_id;
  //           return std::make_shared<ActStatusInternalError>("Deploy");
  //         }
  //         qint64 egress_interface_id = (i < link_ids.size()) ? (egress_link.GetSourceDeviceId() == device_id)
  //                                                                  ? egress_link.GetSourceInterfaceId()
  //                                                                  : egress_link.GetDestinationInterfaceId()
  //                                                            : 0;

  //         // Find StaticForwardTable
  //         ActStaticForwardTable static_forward_table(device_id);
  //         QSet<ActStaticForwardTable>::iterator static_forward_table_iter;
  //         static_forward_table_iter = unicast_static_forward_tables.find(static_forward_table);
  //         if (static_forward_table_iter != unicast_static_forward_tables.end()) {  // found
  //           static_forward_table = *static_forward_table_iter;
  //           unicast_static_forward_tables.erase(static_forward_table_iter);
  //         }
  //         QSet<ActStaticForwardEntry>& static_forward_entries_set = static_forward_table.GetStaticForwardEntries();

  //         // For each mac
  //         for (auto mac : mac_list) {
  //           ActStaticForwardEntry static_forward_entry(pvid, mac);

  //           // Found duplicated static_forward_entry
  //           QSet<ActStaticForwardEntry>::iterator static_forward_entry_iter;
  //           static_forward_entry_iter = static_forward_entries_set.find(static_forward_entry);
  //           if (static_forward_entry_iter != static_forward_entries_set.end()) {  // Has found
  //             static_forward_entry = *static_forward_entry_iter;
  //             static_forward_entries_set.erase(static_forward_entry_iter);
  //           }

  //           static_forward_entry.SetIngressPort(ingress_interface_id);
  //           if (device_id != device_ids.last()) {
  //             static_forward_entry.InsertEgressPort(egress_interface_id);
  //           } else {
  //             for (auto interface_id : mgmt_interface.GetInterfaces()) {
  //               static_forward_entry.InsertEgressPort(interface_id);
  //             }
  //           }
  //           static_forward_entries_set.insert(static_forward_entry);
  //         }
  //         unicast_static_forward_tables.insert(static_forward_table);
  //       }
  //     }
  //   }
  // }
  //   unicast_static_forward_tables_ = unicast_static_forward_tables;
  // }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActDeployPathData::GenerateVlanStaticTable(const ActRoutingResult &routing_result) {  // Not include CB
  ACT_STATUS_INIT();

  // Get StreamSetting (Talker ID & Talker InterfaceID)
  ActTrafficStream stream_setting;
  act_status = act_project_.GetComputedResult().GetTrafficDesign().GetStreamSettingByStreamID(
      routing_result.GetStreamId(), stream_setting);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetStreamSettingByStreamID() failed";
    return act_status;
  }

  // Get ApplicationSetting (VLAN Tagged & UntaggedMode & UserDefinedVlan)
  ActTrafficApplication app_setting;
  act_status = stream_setting.GetApplicationSetting(
      act_project_.GetComputedResult().GetTrafficDesign().GetApplicationSetting(), app_setting);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ApplicationSetting not found.";
    return std::make_shared<ActStatusInternalError>("Deploy");
  }

  auto vlan_priority = app_setting.GetVlanSetting().GetUserDefinedVlan() ? ActVlanPriorityEnum::kTSNUser
                                                                         : ActVlanPriorityEnum::kTSNSystem;
  for (auto routing_path : routing_result.GetRoutingPaths()) {  // routingPaths layer (multiple / single paths)
    QSet<qint64> used_link_ids;
    for (auto path : routing_path.GetRedundantPaths()) {  // paths layer (CB or not)
      QList<qint64> device_ids = path.GetDeviceIds();
      QList<qint64> link_ids = path.GetLinkIds();
      for (int i = 0; i < device_ids.size(); i++) {
        qint64 device_id = device_ids[i];
        ActDevice device;
        act_status = ActGetItemById<ActDevice>(act_project_.GetComputedResult().GetDevices(), device_id, device);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Source device not found. Device:" << device_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }

        qint64 next_device_id = (i < link_ids.size()) ? device_ids[i + 1] : 0;
        ActDevice next_device;
        act_status =
            ActGetItemById<ActDevice>(act_project_.GetComputedResult().GetDevices(), next_device_id, next_device);
        if ((i < link_ids.size()) && !IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Next device not found. Device:" << next_device_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }

        qint64 ingress_link_id = (i > 0) ? link_ids[i - 1] : 0;
        ActLink ingress_link;
        act_status =
            ActGetItemById<ActLink>(act_project_.GetComputedResult().GetLinks(), ingress_link_id, ingress_link);
        if ((i > 0) && !IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Link not found. Link:" << ingress_link_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }
        qint64 ingress_interface_id = (i > 0) ? (ingress_link.GetSourceDeviceId() == device_id)
                                                    ? ingress_link.GetSourceInterfaceId()
                                                    : ingress_link.GetDestinationInterfaceId()
                                              : 0;

        qint64 egress_link_id = (i < link_ids.size()) ? link_ids[i] : 0;
        ActLink egress_link;
        act_status = ActGetItemById<ActLink>(act_project_.GetComputedResult().GetLinks(), egress_link_id, egress_link);
        if ((i < link_ids.size()) && !IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Link not found. Link:" << egress_link_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }
        qint64 egress_interface_id = (i < link_ids.size()) ? (egress_link.GetSourceDeviceId() == device_id)
                                                                 ? egress_link.GetSourceInterfaceId()
                                                                 : egress_link.GetDestinationInterfaceId()
                                                           : 0;

        // Skip to set vlan_entry
        if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetAccessTrunkMode()) {
          used_link_ids.insert(ingress_link_id);
          continue;
        }

        bool te_mstid = device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetTEMSTID()
                            ? true
                            : false;

        // ingress interface
        if (i > 0 && !used_link_ids.contains(ingress_link_id)) {
          SetVlanStaticEntryTable(device_id, ingress_interface_id, vlan_priority, path.GetVlanId(), false, te_mstid);
        }

        // egress interface
        if (i < link_ids.size()) {
          if (!used_link_ids.contains(ingress_link_id)) {  // forward switch, merge switch
            if (!app_setting.GetVlanSetting().GetTagged() &&
                device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode() &&
                (!next_device.GetDeviceProperty()
                      .GetFeatureGroup()
                      .GetConfiguration()
                      .GetVLANSetting()
                      .GetAccessTrunkMode())) {
              // Set last TSN-Switch egress port(Only untagged stream)
              // If TSN-Switch supports hybrid would set untag or not
              SetVlanStaticEntryTable(device_id, egress_interface_id, vlan_priority, path.GetVlanId(), true, te_mstid);
            } else {
              // Tagged stream or not support hybrid wouldn't set untag
              SetVlanStaticEntryTable(device_id, egress_interface_id, vlan_priority, path.GetVlanId(), false, te_mstid);
            }
          } else if (!used_link_ids.contains(egress_link_id)) {  // split switch
            // set stream vlan id
            SetVlanStaticEntryTable(device_id, egress_interface_id, vlan_priority, routing_result.GetVlanId(), false,
                                    te_mstid);
            // set split overwrite vlan id
            SetVlanStaticEntryTable(device_id, egress_interface_id, vlan_priority, path.GetVlanId(), false, te_mstid);
          }
        }
        used_link_ids.insert(ingress_link_id);
      }
    }
  }

  if (!app_setting.GetVlanSetting().GetTagged() &&
      app_setting.GetVlanSetting().GetUntaggedMode() == ActTrafficUntaggedModeEnum::kPVID) {
    qint16 pvid = routing_result.GetVlanId();
    qint64 talker_id = stream_setting.GetTalker().GetDeviceId();
    qint64 talker_intf = stream_setting.GetTalker().GetInterfaceId();

    ActHostPair host_pair;
    host_pair.SetSourceDeviceId(talker_id);
    host_pair.SetSourceInterfaceId(talker_intf);

    // Remove: [feat:3250] Southbound - Remove/Hide PVID & Mgnt. Endpoint feature
    // bool te_mstid =
    // device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetTEMSTID() ? true : false;
    // for (ActManagementInterface mgmt_interface : act_project_.GetTopologySetting().GetManagementInterfaces()) {
    //   host_pair.SetDestinationDeviceId(mgmt_interface.GetDeviceId());
    //   for (ActRoutingLink routing_link : links_from_nodes[mgmt_interface.GetDeviceId()]) {
    //     if (mgmt_interface.GetInterfaces().contains(routing_link.GetSourceInterfaceId())) {
    //       continue;
    //     }
    //     host_pair.SetDestinationInterfaceId(routing_link.GetSourceInterfaceId());

    //     QList<ActRedundantPath> redundant_paths;
    //     compute.DoMultiplePathRoutingForStream(redundant_paths, links_from_nodes, host_pair,
    //                                            {ActDeviceTypeEnum::kTSNSwitch, ActDeviceTypeEnum::kBridgedEndStation,
    //                                             ActDeviceTypeEnum::kSwitch, ActDeviceTypeEnum::kEndStation});

    //     for (ActRedundantPath redundant_path : redundant_paths) {
    //       QList<qint64> device_ids = redundant_path.GetDeviceIds();
    //       QList<qint64> link_ids = redundant_path.GetLinkIds();
    //       for (int i = 1; i < device_ids.size(); i++) {
    //         qint64 device_id = device_ids[i];
    //         qint64 ingress_link_id = link_ids[i - 1];
    //         ActLink ingress_link;
    //         act_status =
    //             ActGetItemById<ActLink>(act_project_.GetComputedResult().GetLinks(), ingress_link_id, ingress_link);
    //         if ((i > 0) && !IsActStatusSuccess(act_status)) {
    //           qCritical() << __func__ << "Link not found. Link:" << ingress_link_id;
    //           return std::make_shared<ActStatusInternalError>("Deploy");
    //         }
    //         qint64 ingress_interface_id = (ingress_link.GetSourceDeviceId() == device_id)
    //                                           ? ingress_link.GetSourceInterfaceId()
    //                                           : ingress_link.GetDestinationInterfaceId();

    //         qint64 egress_link_id = (i < link_ids.size()) ? link_ids[i] : 0;
    //         ActLink egress_link;
    //         act_status =
    //             ActGetItemById<ActLink>(act_project_.GetComputedResult().GetLinks(), egress_link_id, egress_link);
    //         if ((i < link_ids.size()) && !IsActStatusSuccess(act_status)) {
    //           qCritical() << __func__ << "Link not found. Link:" << egress_link_id;
    //           return std::make_shared<ActStatusInternalError>("Deploy");
    //         }
    //         qint64 egress_interface_id = (i < link_ids.size()) ? (egress_link.GetSourceDeviceId() == device_id)
    //                                                                  ? egress_link.GetSourceInterfaceId()
    //                                                                  : egress_link.GetDestinationInterfaceId()
    //                                                            : 0;

    //         if (i > 1) {
    //           SetVlanStaticEntryTable(device_id, ingress_interface_id, vlan_priority, pvid, false, te_mstid);
    //         }
    //         if (i < link_ids.size()) {
    //           SetVlanStaticEntryTable(device_id, egress_interface_id, vlan_priority, pvid, false, te_mstid);
    //         }
    //       }
    //     }
    //   }

    //   // [feat:2540] UI Feature - Allow users to specify management interfaces
    //   // Add untag PVID to management interfaces
    //   ActDevice device;
    //   act_status = act_project_.GetDeviceById(device, mgmt_interface.GetDeviceId());
    //   if (IsActStatusNotFound(act_status)) {
    //     qDebug() << __func__
    //              << QString("Device(%1) not found in project, so skip configuring it ManagementInterfaces")
    //                     .arg(mgmt_interface.GetDeviceId());
    //     continue;
    //   }

    //   // Check device support VLAN hybrid
    //   if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) {
    //     for (auto interface_id : mgmt_interface.GetInterfaces()) {
    //       AddUntagToVlanStaticEntry(mgmt_interface.GetDeviceId(), interface_id, vlan_priority, pvid, te_mstid);
    //     }
    //   }
    // }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActDeployPathData::GeneratePortVlanAndDefaultPriorityTable(
    const ActRoutingResult &routing_result) {  // Not include CB
  ACT_STATUS_INIT();

  // Get ApplicationSetting (VLAN Tagged & UntaggedMode)
  ActTrafficApplication app_setting;
  act_status = act_project_.GetComputedResult().GetTrafficDesign().GetApplicationSettingByStreamID(
      routing_result.GetStreamId(), app_setting);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ApplicationSetting not found.";
    return std::make_shared<ActStatusInternalError>("Deploy");
  }

  // Return tagged stream
  if (app_setting.GetVlanSetting().GetTagged()) {
    // qDebug() << __func__
    //          << QString("Skip generate PortVlan & DefaultPriority Table, because it is tagged stream(%1)")
    //                 .arg(routing_result.GetStreamId())
    //                 .toStdString()
    //                 .c_str();
    return act_status;
  }

  // return Per-Stream Priority stream
  if (app_setting.GetVlanSetting().GetUntaggedMode() == ActTrafficUntaggedModeEnum::kPerStreamPriority) {
    // qDebug() << __func__
    //          << QString("Skip generate PortVlan & DefaultPriority Table, because the stream(%1) active
    //          StreamPriority")
    //                 .arg(routing_result.GetStreamId())
    //                 .toStdString()
    //                 .c_str();
    return act_status;
  }
  for (auto routing_path : routing_result.GetRoutingPaths()) {  // routingPaths layer (multiple / single paths)
    for (auto path : routing_path.GetRedundantPaths()) {        // paths layer (CB or not)
      if (routing_result.GetVlanId() != path.GetVlanId()) {     // skip the CB path
        continue;
      }

      // Find First TSN-Switch node
      ActDevice first_switch_dev(-1);
      for (auto dev_id : path.GetDeviceIds()) {
        ActDevice target_dev;
        act_status = ActGetItemById<ActDevice>(act_project_.GetComputedResult().GetDevices(), dev_id, target_dev);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Device not found. Device:" << dev_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }
        // Check device FeatureGroup support DefaultPVID
        if (target_dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetDefaultPVID()) {
          first_switch_dev = target_dev;
          break;
        }
      }

      // [bugfix:3196] Compute success can't click the deploy button
      if (first_switch_dev.GetId() == -1) {
        qCritical() << __func__
                    << "Not found the first support DefaultPVID device. StreamID:" << routing_result.GetStreamId();
        return std::make_shared<ActStatusInternalError>("Deploy");
      }

      // Find enter TSN domain interface (EndStation <--> TSN-Switch)
      ActLink enter_link;
      qint64 enter_link_if_id = -1;
      for (auto link_id : path.GetLinkIds()) {
        ActLink target_link;
        act_status = ActGetItemById<ActLink>(act_project_.GetComputedResult().GetLinks(), link_id, target_link);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Link not found. Link:" << link_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }
        // Check link src or dst is first_switch_dev id
        if (target_link.GetSourceDeviceId() == first_switch_dev.GetId()) {
          enter_link = target_link;
          enter_link_if_id = target_link.GetSourceInterfaceId();
          break;
        }
        if (target_link.GetDestinationDeviceId() == first_switch_dev.GetId()) {
          enter_link = target_link;
          enter_link_if_id = target_link.GetDestinationInterfaceId();
          break;
        }
      }

      // Check enter_link_if_id has found
      if (enter_link_if_id == -1) {
        qCritical() << __func__ << "Enter link's interface not found";
        return std::make_shared<ActStatusInternalError>("Deploy");
      }

      // Set PortVlan Table
      act_status = SetPortVlanTable(first_switch_dev.GetId(), enter_link_if_id, routing_result.GetVlanId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "SetPortVlanTable() failed.";
        return act_status;
      }

      // Set DefaultPriority Table
      act_status =
          SetDefaultPriorityTable(first_switch_dev.GetId(), enter_link_if_id, routing_result.GetPriorityCodePoint());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "SetDefaultPriorityTable() failed.";
        return act_status;
      }
    }
    // If not Multicast would break
    if (!routing_result.GetMulticast()) {
      break;
    }
  }

  return act_status;
}

ACT_STATUS ActDeployPathData::SetPortVlanTable(const qint64 &device_id, const qint32 &port_id, const quint16 &pvid) {
  ACT_STATUS_INIT();

  ActPortVlanTable port_vlan_table(device_id);
  QSet<ActPortVlanTable>::iterator port_vlan_table_iter;
  QSet<ActPortVlanEntry> port_vlan_entry_set;

  port_vlan_table_iter = port_vlan_tables_.find(port_vlan_table);
  if (port_vlan_table_iter != port_vlan_tables_.end()) {  // found
    port_vlan_entry_set = port_vlan_table_iter->GetPortVlanEntries();
    port_vlan_tables_.erase(port_vlan_table_iter);
  }

  ActPortVlanEntry port_vlan_entry(port_id, pvid, ActVlanPriorityEnum::kTSNSystem);
  // Check has exists
  auto port_vlan_entry_iter = port_vlan_entry_set.find(port_vlan_entry);
  if (port_vlan_entry_iter != port_vlan_entry_set.end()) {  //  exists
    if (port_vlan_entry_iter->GetPVID() != pvid) {
      qCritical() << __func__
                  << QString("Has the different PVID(%1) of the PortVlanEntry(Port:%2, PVID:%3) exists. Device: %4")
                         .arg(pvid)
                         .arg(port_vlan_entry_iter->GetPortId())
                         .arg(port_vlan_entry_iter->GetPVID())
                         .arg(device_id)
                         .toStdString()
                         .c_str();
      return std::make_shared<ActStatusInternalError>("Deploy");
    }
  } else {  // not exists
    port_vlan_entry_set.insert(port_vlan_entry);
  }

  port_vlan_table.SetPortVlanEntries(port_vlan_entry_set);  // set to PortVlanTable
  port_vlan_tables_.insert(port_vlan_table);

  return act_status;
}

ACT_STATUS ActDeployPathData::SetDefaultPriorityTable(const qint64 &device_id, const qint32 &port_id,
                                                      const quint8 &default_pcp) {
  ACT_STATUS_INIT();

  ActDefaultPriorityTable default_priority_table(device_id);
  QSet<ActDefaultPriorityTable>::iterator default_priority_table_iter;
  QSet<ActDefaultPriorityEntry> default_priority_table_set;

  default_priority_table_iter = default_priority_tables_.find(default_priority_table);
  if (default_priority_table_iter != default_priority_tables_.end()) {  // Has the DefaultPriorityTable table
    default_priority_table_set = default_priority_table_iter->GetDefaultPriorityEntries();
    default_priority_tables_.erase(default_priority_table_iter);
  }

  ActDefaultPriorityEntry default_priority_entry(port_id, default_pcp);

  // Check has exists
  auto default_priority_entry_iter = default_priority_table_set.find(default_priority_entry);
  if (default_priority_entry_iter != default_priority_table_set.end()) {  //  exists
    if (default_priority_entry_iter->GetDefaultPCP() != default_pcp) {
      qCritical() << __func__
                  << QString(
                         "Has the different DefaultPCP(%1) of the DefaultPriorityEntry(Port:%2, DefaultPCP:%3) exists. "
                         "Device: %4")
                         .arg(default_pcp)
                         .arg(default_priority_entry_iter->GetPortId())
                         .arg(default_priority_entry_iter->GetDefaultPCP())
                         .arg(device_id)
                         .toStdString()
                         .c_str();
      return std::make_shared<ActStatusInternalError>("Deploy");
    }
  } else {  // not exists
    default_priority_table_set.insert(default_priority_entry);
  }

  default_priority_table.SetDefaultPriorityEntries(default_priority_table_set);  // set to DefaultPriorityTable
  default_priority_tables_.insert(default_priority_table);

  return act_status;
}

ACT_STATUS ActDeployPathData::SetVlanStaticEntryTable(const qint64 &device_id, const qint64 &port,
                                                      const ActVlanPriorityEnum &vlan_priority, const quint16 &vlan_id,
                                                      const bool &untag, const bool &te_mstid) {
  ACT_STATUS_INIT();

  ActVlanStaticTable vlan_static_table(device_id);
  QSet<ActVlanStaticEntry> vlan_static_entries_set;

  // Find VlanTable
  auto vlan_static_table_iter = vlan_static_tables_.find(vlan_static_table);
  if (vlan_static_table_iter == vlan_static_tables_.end()) {  // not found
    // Create new & insert VlanTable
    ActVlanStaticEntry vlan_static_entry(vlan_priority, vlan_id, QString("v%1").arg(vlan_id), port, 1, untag, te_mstid);
    vlan_static_entries_set.insert(vlan_static_entry);

    vlan_static_table.SetVlanStaticEntries(vlan_static_entries_set);
    vlan_static_tables_.insert(vlan_static_table);
    return act_status;
  }

  // VlanTable has found -> find the entry
  vlan_static_entries_set = vlan_static_table_iter->GetVlanStaticEntries();
  ActVlanStaticEntry vlan_static_entry(vlan_id);
  auto vlan_static_entry_iter = vlan_static_entries_set.find(vlan_static_entry);
  if (vlan_static_entry_iter == vlan_static_entries_set.end()) {  // entry not found
    // Create new entry
    vlan_static_entry =
        ActVlanStaticEntry(vlan_priority, vlan_id, QString("v%1").arg(vlan_id), port, 1, untag, te_mstid);
  } else {  // found
    // Insert the Egress port
    vlan_static_entry = *vlan_static_entry_iter;
    if (untag) {
      vlan_static_entry.InsertUntaggedPorts(port);
    } else {
      vlan_static_entry.RemoveUntaggedPorts(port);
    }
    vlan_static_entry.InsertEgressPorts(port);
    vlan_static_entry.SetTeMstid(te_mstid);
    vlan_static_entries_set.erase(vlan_static_entry_iter);
  }

  // Update vlan_static_tables_
  vlan_static_entries_set.insert(vlan_static_entry);
  vlan_static_table.SetVlanStaticEntries(vlan_static_entries_set);
  vlan_static_tables_.erase(vlan_static_table_iter);
  vlan_static_tables_.insert(vlan_static_table);

  return act_status;
}

ACT_STATUS ActDeployPathData::AddUntagToVlanStaticEntry(const qint64 &device_id, const qint64 &port,
                                                        const ActVlanPriorityEnum &vlan_priority,
                                                        const quint16 &vlan_id, const bool &te_mstid) {
  ACT_STATUS_INIT();

  ActVlanStaticTable vlan_static_table(device_id);
  QSet<ActVlanStaticEntry> vlan_static_entries_set;

  // Find VlanTable
  auto vlan_static_table_iter = vlan_static_tables_.find(vlan_static_table);
  if (vlan_static_table_iter == vlan_static_tables_.end()) {  // not found
    // Create new & insert VlanTable
    ActVlanStaticEntry vlan_static_entry(vlan_priority, vlan_id, QString("v%1").arg(vlan_id), port, 1, true, te_mstid);
    vlan_static_entries_set.insert(vlan_static_entry);

    vlan_static_table.SetVlanStaticEntries(vlan_static_entries_set);
    vlan_static_tables_.insert(vlan_static_table);
    return act_status;
  }

  // VlanTable has found -> find the entry
  vlan_static_entries_set = vlan_static_table_iter->GetVlanStaticEntries();
  ActVlanStaticEntry vlan_static_entry(vlan_id);
  auto vlan_static_entry_iter = vlan_static_entries_set.find(vlan_static_entry);
  if (vlan_static_entry_iter == vlan_static_entries_set.end()) {  // entry not found

    // Create new entry
    vlan_static_entry =
        ActVlanStaticEntry(vlan_priority, vlan_id, QString("v%1").arg(vlan_id), port, 1, true, te_mstid);
  } else {  // found

    vlan_static_entry = *vlan_static_entry_iter;

    // Insert to Egress port
    vlan_static_entry.InsertEgressPorts(port);

    // Insert to Untagged port
    vlan_static_entry.InsertUntaggedPorts(port);

    vlan_static_entries_set.erase(vlan_static_entry_iter);
  }

  // Update vlan_static_tables_
  vlan_static_entries_set.insert(vlan_static_entry);
  vlan_static_table.SetVlanStaticEntries(vlan_static_entries_set);
  vlan_static_tables_.erase(vlan_static_table_iter);
  vlan_static_tables_.insert(vlan_static_table);

  return act_status;
}

ACT_STATUS ActDeployPathData::GenerateStreamPriorityTable(const ActRoutingResult &routing_result) {
  ACT_STATUS_INIT();

  // Get ApplicationSetting (VLAN Tagged & UntaggedMode)
  ActTrafficApplication app_setting;
  act_status = act_project_.GetComputedResult().GetTrafficDesign().GetApplicationSettingByStreamID(
      routing_result.GetStreamId(), app_setting);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ApplicationSetting not found.";
    return std::make_shared<ActStatusInternalError>("Deploy");
  }

  // Return tagged stream
  if (app_setting.GetVlanSetting().GetTagged()) {
    // qDebug() << __func__
    //          << QString("Skip generate StreamPriority Table, because it is tagged stream(%1)")
    //                 .arg(routing_result.GetStreamId())
    //                 .toStdString()
    //                 .c_str();
    return act_status;
  }

  for (auto routing_path : routing_result.GetRoutingPaths()) {  // RoutingPaths layer(multiple / single paths)
    for (auto path : routing_path.GetRedundantPaths()) {        // Paths layer(CB or not)
      // Add init table to config

      // [feat:2237] Feature - Multiple Linked End Station Support/Display
      // [feat:2540] UI Feature - Allow users to specify management interfaces
      // Find First SupportPerStreamPriority node
      ActDevice first_switch_dev(-1);
      for (auto dev_id : path.GetDeviceIds()) {
        ActDevice target_dev;
        act_status = ActGetItemById<ActDevice>(act_project_.GetComputedResult().GetDevices(), dev_id, target_dev);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Device not found. Device:" << dev_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }
        // Check device FeatureGroup support PerStreamPriority or PerStreamPriorityV2
        if (target_dev.GetDeviceProperty()
                .GetFeatureGroup()
                .GetConfiguration()
                .GetVLANSetting()
                .GetPerStreamPriority() ||
            target_dev.GetDeviceProperty()
                .GetFeatureGroup()
                .GetConfiguration()
                .GetVLANSetting()
                .GetPerStreamPriorityV2()) {
          first_switch_dev = target_dev;
          break;
        }
      }
      // [bugfix:3196] Compute success can't click the deploy button
      if (first_switch_dev.GetId() == -1) {
        qCritical() << __func__ << "Not found the first support Per-StreamPriority device. StreamID:"
                    << routing_result.GetStreamId();
        return std::make_shared<ActStatusInternalError>("Deploy");
      }

      // Find enter TSN domain interface (EndStation <--> SupportPerStreamPriority Node)
      ActLink enter_link;
      qint64 enter_link_if_id = -1;
      for (auto link_id : path.GetLinkIds()) {
        ActLink target_link;
        act_status = ActGetItemById<ActLink>(act_project_.GetComputedResult().GetLinks(), link_id, target_link);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Link not found. Link:" << link_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }
        // Check link src or dst is first_switch_dev id
        if (target_link.GetSourceDeviceId() == first_switch_dev.GetId()) {
          enter_link = target_link;
          enter_link_if_id = target_link.GetSourceInterfaceId();
          break;
        }
        if (target_link.GetDestinationDeviceId() == first_switch_dev.GetId()) {
          enter_link = target_link;
          enter_link_if_id = target_link.GetDestinationInterfaceId();
          break;
        }
      }

      // Check enter_link_if_id has found
      if (enter_link_if_id == -1) {
        qCritical() << __func__ << "Enter link's interface not found";
        return std::make_shared<ActStatusInternalError>("Deploy");
      }

      // Find last SupportPerStreamPriority node
      ActDevice last_switch_dev(-1);
      for (auto it = path.GetDeviceIds().rbegin(); it != path.GetDeviceIds().rend(); ++it) {
        qint64 dev_id = *it;
        ActDevice target_dev;
        act_status = ActGetItemById<ActDevice>(act_project_.GetComputedResult().GetDevices(), dev_id, target_dev);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Device not found. Device:" << dev_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }
        // Check device FeatureGroup support PerStreamPriority or PerStreamPriorityV2
        if (target_dev.GetDeviceProperty()
                .GetFeatureGroup()
                .GetConfiguration()
                .GetVLANSetting()
                .GetPerStreamPriority() ||
            target_dev.GetDeviceProperty()
                .GetFeatureGroup()
                .GetConfiguration()
                .GetVLANSetting()
                .GetPerStreamPriorityV2()) {
          last_switch_dev = target_dev;
          break;
        }
      }

      if (last_switch_dev.GetId() == -1) {
        qCritical() << __func__ << "Not found the latest support Per-StreamPriority device. StreamID:"
                    << routing_result.GetStreamId();
        return std::make_shared<ActStatusInternalError>("Deploy");
      }

      // Find leave TSN domain interface (SupportPerStreamPriority Node <--> EndStation)
      ActLink leave_link;
      qint64 leave_link_if_id = -1;
      for (auto it = path.GetLinkIds().rbegin(); it != path.GetLinkIds().rend(); ++it) {
        qint64 link_id = *it;
        ActLink target_link;
        act_status = ActGetItemById<ActLink>(act_project_.GetComputedResult().GetLinks(), link_id, target_link);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Link not found. Link:" << link_id;
          return std::make_shared<ActStatusInternalError>("Deploy");
        }

        // Check link src or dst is leave_switch_dev id
        if (target_link.GetSourceDeviceId() == last_switch_dev.GetId()) {
          leave_link = target_link;
          leave_link_if_id = target_link.GetSourceInterfaceId();
          break;
        }
        if (target_link.GetDestinationDeviceId() == last_switch_dev.GetId()) {
          leave_link = target_link;
          leave_link_if_id = target_link.GetDestinationInterfaceId();
          break;
        }
      }
      // Check leave_link_if_id has found
      if (leave_link_if_id == -1) {
        qCritical() << __func__ << "Leave link's interface not found";
        return std::make_shared<ActStatusInternalError>("Deploy");
      }

      // Set StadPort Table(Ingress)
      // Add the VLAN tag at enter the TSN domain interface
      // Check UntaggedMode is PerStreamPriority
      if (app_setting.GetVlanSetting().GetUntaggedMode() == ActTrafficUntaggedModeEnum::kPerStreamPriority) {
        // Check PerStreamPriority or PerStreamPriorityV2 is capable
        if (first_switch_dev.GetDeviceProperty()
                .GetFeatureGroup()
                .GetConfiguration()
                .GetVLANSetting()
                .GetPerStreamPriority() ||
            first_switch_dev.GetDeviceProperty()
                .GetFeatureGroup()
                .GetConfiguration()
                .GetVLANSetting()
                .GetPerStreamPriorityV2()) {
          ActStreamPrioritySetting sp_setting;
          sp_setting.SetVlanId(path.GetVlanId());
          sp_setting.SetPriorityCodePoint(routing_result.GetPriorityCodePoint());
          sp_setting.SetEtherType(app_setting.GetVlanSetting().GetEtherType());
          sp_setting.SetEnableSubType(app_setting.GetVlanSetting().GetEnableSubType());
          sp_setting.SetSubType(app_setting.GetVlanSetting().GetSubType());
          // L3
          sp_setting.SetType(app_setting.GetVlanSetting().GetType());
          sp_setting.SetUdpPort(app_setting.GetVlanSetting().GetUdpPort());
          sp_setting.SetTcpPort(app_setting.GetVlanSetting().GetTcpPort());

          act_status = SetStadPortTable(first_switch_dev, enter_link_if_id, sp_setting);
          if (!IsActStatusSuccess(act_status)) {
            return act_status;
          }
        }
      }

      // Set StadConfig Table(Egress untagged)
      // Remove the VLAN tag at the leave the TSN domain interface
      // Check (VlanHybrid is not capable) && (PerStreamPriority or PerStreamPriorityV2 is capable)
      auto last_sw_dev_vlan_feat_group =
          last_switch_dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting();
      if (!last_sw_dev_vlan_feat_group.GetHybridMode()) {
        if (last_sw_dev_vlan_feat_group.GetPerStreamPriority()) {  // PerStreamPriorityV2 not support egress
          SetStadConfigTable(last_switch_dev.GetId(), leave_link_if_id);
        }
      }

      // [feat:2540] UI Feature - Allow users to specify management interfaces
      // Remove the VLAN tag at the leave the TSN domain interface
      // Check UntaggedMode is PVID
      if (!app_setting.GetVlanSetting().GetTagged()) {
        // Check (VlanHybrid is not capable) && (PerStreamPriority or PerStreamPriorityV2 is capable)
        auto first_sw_dev_vlan_feat_group =
            first_switch_dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting();
        if (!first_sw_dev_vlan_feat_group.GetHybridMode()) {
          if (first_sw_dev_vlan_feat_group.GetPerStreamPriority()) {  // PerStreamPriorityV2 not support egress
            SetStadConfigTable(first_switch_dev.GetId(), enter_link_if_id);
          }
        }
      }

      // If not CB would break
      if (!routing_result.GetCB()) {
        break;
      }
    }
    // If not Multicast would break
    if (!routing_result.GetMulticast()) {
      break;
    }
  }

  // Remove: [feat:3250] Southbound - Remove/Hide PVID & Mgnt. Endpoint feature
  // [feat:2540] UI Feature - Allow users to specify management interfaces
  // Add untag PVID to management interfaces
  // if (!stream.GetTagged()) {
  //   auto untagged_vlan = stream.GetVlanId();
  //   for (auto mgmt_interface : act_project_.GetTopologySetting().GetManagementInterfaces()) {
  //     ActDevice device;
  //     act_status = act_project_.GetDeviceById(device, mgmt_interface.GetDeviceId());
  //     if (IsActStatusNotFound(act_status)) {
  //       qDebug() << __func__
  //                << QString("Device(%1) not found in project, so skip configuring it ManagementInterfaces")
  //                       .arg(mgmt_interface.GetDeviceId());
  //       continue;
  //     }

  //     // Check device (not support VLAN hybrid) && (support PerStreamPriority)
  //     if ((!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) &&
  //         (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetPerStreamPriority()))
  //         {
  //       for (auto interface_id : mgmt_interface.GetInterfaces()) {
  //         SetStadConfigTable(mgmt_interface.GetDeviceId(), interface_id);
  //       }
  //     }
  //   }
  // }

  return act_status;
}
ACT_STATUS ActDeployPathData::GetListenerByDeviceId(const QList<ActListener> &listeners, const qint64 &device_id,
                                                    ActListener &result_listener) {
  ACT_STATUS_INIT();

  for (auto listener : listeners) {
    if (listener.GetEndStationInterface().GetDeviceId() == device_id) {
      result_listener = listener;
      return act_status;
    }
  }

  // not found
  return std::make_shared<ActStatusNotFound>("Listener");

  // find_if need operator
  // auto listener_iter = std::find_if(listeners.begin(), listeners.end(), [](ActListener* listener) {
  //   return listener->GetEndStationInterface().GetDeviceId() == device_id;
  // });
  // if (listener_iter != listeners.end()) {
  //   return std::make_shared<ActStatusBase>(ActStatusType::kNotFound, ActSeverity::kCritical);
  // }
  // result_listener = *listener_iter;
  // return act_status;
}

ACT_STATUS ActDeployPathData::SetStadConfigTable(const qint64 &device_id, const qint64 &port_id) {
  ACT_STATUS_INIT();

  ActStadConfigTable stad_config_table(device_id);
  QSet<ActStadConfigTable>::iterator stad_config_table_iter;
  QSet<ActStadConfigEntry> stad_config_table_set;

  stad_config_table_iter = stad_config_tables_.find(stad_config_table);
  if (stad_config_table_iter != stad_config_tables_.end()) {  // Has the StadConfigTable table
    stad_config_table_set = stad_config_table_iter->GetStadConfigEntries();
    stad_config_tables_.erase(stad_config_table_iter);
  }

  ActStadConfigEntry stad_config_entry(port_id, 1);

  stad_config_table_set.insert(stad_config_entry);

  stad_config_table.SetStadConfigEntries(stad_config_table_set);  // set to StadConfigTable
  stad_config_tables_.insert(stad_config_table);

  return act_status;
}

ACT_STATUS ActDeployPathData::SetStadPortTable(const ActDevice &device, const qint64 &port_id,
                                               const ActStreamPrioritySetting &stream_priority_setting) {
  ACT_STATUS_INIT();

  // Check the L3 FeatureGroup
  if (stream_priority_setting.GetType().contains(ActStreamPriorityTypeEnum::kTcp) ||
      stream_priority_setting.GetType().contains(ActStreamPriorityTypeEnum::kUdp)) {
    if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetPerStreamPriorityV2()) {
      QString error_msg = QString("Device(%1) - Only supports the Per-StreamPriority Ethertype type")
                              .arg(device.GetIpv4().GetIpAddress());
      qDebug() << __func__ << error_msg.toStdString().c_str();

      // return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  QSet<ActInterfaceStadPortEntry> interface_stad_port_entry_set;
  QSet<ActStadPortEntry> stad_port_entry_set;
  qint32 ingress_index = 0;
  ActStadPortTable stad_port_table(device.GetId());
  ActInterfaceStadPortEntry if_stad_port_entry(port_id);

  // Find ActStadPortTable
  auto stad_port_table_iter = stad_port_tables_.find(stad_port_table);
  if (stad_port_table_iter != stad_port_tables_.end()) {  // Has the ActStadPort Table
    // Find ActInterfaceStadPortEntry
    auto if_stad_port_entry_iter = stad_port_table_iter->GetInterfaceStadPortEntries().find(if_stad_port_entry);
    if (if_stad_port_entry_iter != stad_port_table_iter->GetInterfaceStadPortEntries().end()) {  // Has the if_entry
      stad_port_entry_set = if_stad_port_entry_iter->GetStadPortEntries();
      ingress_index = stad_port_entry_set.size() + 1;
      // stad_port_tables_.erase(stad_port_table_iter);
    }
    interface_stad_port_entry_set = stad_port_table_iter->GetInterfaceStadPortEntries();
    stad_port_tables_.erase(stad_port_table_iter);
  }

  ActStadPortEntry stad_port_entry(port_id, stream_priority_setting.GetVlanId(),
                                   stream_priority_setting.GetPriorityCodePoint(), 1, ingress_index,
                                   stream_priority_setting.GetEtherType());

  // Set Subtype
  if (stream_priority_setting.GetEnableSubType() && (stream_priority_setting.GetSubType() < 256)) {
    stad_port_entry.SetSubtypeEnable(1);
    stad_port_entry.SetSubtypeValue(stream_priority_setting.GetSubType());
  }

  // Set L3 fields
  stad_port_entry.SetType(stream_priority_setting.GetType());
  stad_port_entry.SetUdpPort(stream_priority_setting.GetUdpPort());
  stad_port_entry.SetTcpPort(stream_priority_setting.GetTcpPort());

  bool check_result;
  CheckStadPortEntryCanBeInsertToSet(stad_port_entry_set, stad_port_entry, check_result);
  if (check_result) {
    stad_port_entry_set.insert(stad_port_entry);
    if_stad_port_entry.SetStadPortEntries(stad_port_entry_set);
    interface_stad_port_entry_set.insert(if_stad_port_entry);
    stad_port_table.SetInterfaceStadPortEntries(interface_stad_port_entry_set);
    stad_port_tables_.insert(stad_port_table);
  }

  return act_status;
}

ACT_STATUS ActDeployPathData::CheckStadPortEntryCanBeInsertToSet(const QSet<ActStadPortEntry> &stad_port_entry_set,
                                                                 const ActStadPortEntry &stad_port_entry_target,
                                                                 bool &result) {
  ACT_STATUS_INIT();

  QString target_entry_key = QString("%1.%2.%3.%4.%5")
                                 .arg(stad_port_entry_target.GetPortId())
                                 .arg(stad_port_entry_target.GetEthertypeValue())
                                 .arg(stad_port_entry_target.GetSubtypeValue())
                                 .arg(stad_port_entry_target.GetVlanId())
                                 .arg(stad_port_entry_target.GetVlanPcp());

  for (auto stad_port_entry : stad_port_entry_set) {
    // PortId.EtherType.Subtype.vid.pcp
    QString exisit_entry_key = QString("%1.%2.%3.%4.%5")
                                   .arg(stad_port_entry.GetPortId())
                                   .arg(stad_port_entry.GetEthertypeValue())
                                   .arg(stad_port_entry.GetSubtypeValue())
                                   .arg(stad_port_entry.GetVlanId())
                                   .arg(stad_port_entry.GetVlanPcp());
    if (target_entry_key == exisit_entry_key) {
      result = false;
      break;
    }
  }
  result = true;

  return act_status;
}

ACT_STATUS ActDeployPathData::GenerateGclTable() {
  ACT_STATUS_INIT();

  for (auto gcl_result : act_project_.GetComputedResult().GetGclResults()) {
    // Find device
    ActDevice dev;
    act_status =
        ActGetItemById<ActDevice>(act_project_.GetComputedResult().GetDevices(), gcl_result.GetDeviceId(), dev);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Target device not found in the devices. Device:" << gcl_result.GetDeviceId();
      return std::make_shared<ActStatusInternalError>("Deploy");
    }

    // [feat:2237] Feature - Multiple Linked End Station Support/Display
    if (!dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTSN().GetIEEE802Dot1Qbv()) {
      continue;
    }

    QSet<ActInterfaceGateParameters> if_gate_parameters_set;
    for (auto interface_gcls : gcl_result.GetInterfaceGateControls()) {
      qint64 cycle_time = 0;
      QList<ActAdminControl> admin_control_list;
      quint32 admin_control_index = 0;
      for (auto gcl : interface_gcls.GetGateControls()) {
        // GetGCLOffsetMinDuration & GetGCLOffsetMaxDuration for algorithm compute.
        qint64 duration = gcl.GetStopTime() - gcl.GetStartTime();
        cycle_time += duration;
        admin_control_list.push_back(
            ActAdminControl(admin_control_index, "set-gate-states", ActSGSParams(gcl.GetGateStatesValue(), duration)));
        admin_control_index += 1;
      }

      quint16 queue_count = dev.GetDeviceProperty().GetNumberOfQueue();
      quint8 admin_gate_states_value = qPow(2, queue_count) - 1;
      ActAdminCycleTime admin_cycle_time(cycle_time, ACT_TIME_DENOMINATOR_NANO);

      // Generate GateParameters
      ActGateParameters gate_parameters(true, true, admin_gate_states_value,
                                        act_project_.GetCycleSetting().GetAdminBaseTime(), admin_cycle_time, 0,
                                        admin_control_list.size(), admin_control_list);

      // Generate PortGateParameters
      ActInterfaceGateParameters port_gate_parameters(interface_gcls.GetInterfaceId(), gate_parameters);

      // Insert PortGateParameters
      if_gate_parameters_set.insert(port_gate_parameters);
    }
    ActGclTable gcl_table(gcl_result.GetDeviceId(), if_gate_parameters_set);
    gcl_tables_.insert(gcl_table);
  }

  return act_status;
}

// StadPortEntry DeployPathData::GenerateStadPortEntry(const qint32& index, const quint16& vlan_id,
//                                                     const QString& source_port, const Stream& stream) {
//   StadPortEntry stad_port_entry;

//   // CCLinkIeTsn
//   if (stream.GetStreamPriority().GetFrameType() == ActFrameTypeEnum::kCCLinkIeTsn) {
//     CCLinkIeTsn cc_link_ie_tsn = stream.GetStreamPriority().GetCCLinkIeTSN();
//     if (cc_link_ie_tsn.GetEtherType() == 0) {
//       // TODO: act error
//       qFatal("The StreamPriority's ethertype value is 0");
//     }

//     stad_port_entry =
//         StadPortEntry(1, source_port, index, vlan_id, cc_link_ie_tsn.GetEtherType(), stream.GetStreamId());

//     // set Subtype
//     if (cc_link_ie_tsn.GetEnableSubType() && (cc_link_ie_tsn.GetSubType() < 256)) {
//       stad_port_entry.SetSubtypeEnable(1);
//       stad_port_entry.SetSubtypeValue(cc_link_ie_tsn.GetSubType());
//     }

//     // Set vlan pcp
//     for (auto data_frame_spec : stream.GetTalker().GetDataFrameSpecifications()) {
//       if (data_frame_spec.GetType() == FieldTypeEnum::kIeee802VlanTag) {
//         if (data_frame_spec.GetIeee802VlanTag().GetPriorityCodePoint() != 0) {
//           stad_port_entry.SetVlanPcp(data_frame_spec.GetIeee802VlanTag().GetPriorityCodePoint());
//         }
//       }
//     }
//   }
//   return stad_port_entry;
// }

ACT_STATUS ActDeployPathData::GenerateCbTable(const ActRoutingResult &routing_result) {
  ACT_STATUS_INIT();

  if (!routing_result.GetCB()) {
    return act_status;
  }

  // Get StreamSetting (Destination MAC address (Unicast / Multicast))
  ActTrafficStream stream_setting;
  act_status = act_project_.GetComputedResult().GetTrafficDesign().GetStreamSettingByStreamID(
      routing_result.GetStreamId(), stream_setting);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetStreamSettingByStreamID() failed";
    return act_status;
  }

  QString destination_mac = stream_setting.GetDestinationMac();
  qint32 stream_handle_1 = this->GetStreamHandle();
  qint32 stream_handle_2 = this->GetStreamHandle();
  qint32 stream_handle_3 = this->GetStreamHandle();

  quint16 stream_vlan_id = routing_result.GetVlanId();

  bool sequence_id_generated = false;
  for (ActRoutingPath routing_path : routing_result.GetRoutingPaths()) {
    // QMap<qint64, QSet<qint32>> used_link_ids;
    QSet<qint64> used_link_ids;
    qint32 stream_handle = stream_handle_1;
    for (ActRedundantPath redundant_path : routing_path.GetRedundantPaths()) {
      QList<qint64> link_ids = redundant_path.GetLinkIds();
      QList<qint64> device_ids = redundant_path.GetDeviceIds();
      quint16 redundant_vlan_id = redundant_path.GetVlanId();

      for (int i = 0; i < device_ids.size(); i++) {
        ActDevice device, last_device, next_device;
        qint64 device_id = device_ids[i];
        act_project_.GetDeviceById(device, device_id);

        qint64 last_device_id = i > 0 ? device_ids[i - 1] : 0;
        qint64 next_device_id = (i < link_ids.size()) ? device_ids[i + 1] : 0;
        act_project_.GetDeviceById(last_device, last_device_id);
        act_project_.GetDeviceById(next_device, next_device_id);

        ActLink ingress_link;
        qint64 ingress_link_id = (i > 0) ? link_ids[i - 1] : 0;
        act_project_.GetLinkById(ingress_link, ingress_link_id);
        qint64 ingress_interface_id = (i > 0) ? (ingress_link.GetSourceDeviceId() == device_id)
                                                    ? ingress_link.GetSourceInterfaceId()
                                                    : ingress_link.GetDestinationInterfaceId()
                                              : 0;
        ActLink egress_link;
        qint64 egress_link_id = (i < link_ids.size()) ? link_ids[i] : 0;
        act_project_.GetLinkById(egress_link, egress_link_id);
        qint64 egress_interface_id = (i < link_ids.size()) ? (egress_link.GetSourceDeviceId() == device_id)
                                                                 ? egress_link.GetSourceInterfaceId()
                                                                 : egress_link.GetDestinationInterfaceId()
                                                           : 0;

        QString egress_interface_name;
        QString ingress_interface_name;
        for (ActInterface interface : device.GetInterfaces()) {
          if (interface.GetInterfaceId() == egress_interface_id) {
            egress_interface_name = interface.GetInterfaceName();
          } else if (interface.GetInterfaceId() == ingress_interface_id) {
            ingress_interface_name = interface.GetInterfaceName();
          }
        }

        if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTSN().GetIEEE802Dot1CB()) {
          used_link_ids.insert(ingress_link_id);
          continue;
        }

        // redundant path duplicated
        if (used_link_ids.contains(ingress_link_id) && used_link_ids.contains(egress_link_id)) {
          continue;
        }

        // Find device cb table
        ActCbTable cb_table(device_id);
        QSet<ActCbTable>::iterator cb_table_iter = cb_tables_.find(cb_table);
        if (cb_table_iter != cb_tables_.end()) {
          cb_table = *cb_table_iter;
          cb_tables_.remove(cb_table);
        }

        // stream identity list
        QList<ActStreamIdentityEntry> &stream_identity_list = cb_table.GetStreamIdentityList();
        // FRER entry
        ActFREREntry &frer = cb_table.GetFRER();
        QList<ActSequenceIdentificationList> &sequence_identification_lists = frer.GetSequenceIdentificationLists();
        QList<ActSequenceGenerationList> &sequence_generation_lists = frer.GetSequenceGenerationLists();
        QList<ActSequenceRecoveryList> &sequence_recovery_lists = frer.GetSequenceRecoveryLists();

        // ingress port for split, forward, merge device
        if (!used_link_ids.contains(ingress_link_id)) {
          ActStreamIdentityEntry stream_identity_entry(stream_handle,
                                                       ActIdentificationType::kNull_stream_identification);
          int stream_identity_idx = stream_identity_list.indexOf(stream_identity_entry);
          if (stream_identity_idx == -1) {
            ActStreamIdentityEntry stream_identity_entry(stream_handle,
                                                         ActIdentificationType::kNull_stream_identification);
            stream_identity_entry.SetIndex(cb_table.GetStreamIdentityIndex());
            ActTsnStreamIdEntryGroup &tsn_stream_id_entry_group = stream_identity_entry.GetTsnStreamIdEntryGroup();
            ActOutFacing &out_facing = tsn_stream_id_entry_group.GetOutFacing();
            QSet<QString> &input_port_list = out_facing.GetInputPortList();
            input_port_list.insert(ingress_interface_name);
            ActNullStreamIdentificationGroup &null_stream_identification =
                tsn_stream_id_entry_group.GetNullStreamIdentification();
            null_stream_identification.SetDestinationMac(destination_mac);
            null_stream_identification.SetTagged(ActVlanTagIdentificationType::kTagged);
            null_stream_identification.SetVlan(redundant_vlan_id);
            stream_identity_list.append(stream_identity_entry);
          }

          // if (device_id != device_ids.first()) {
          //   // Ingress interface always set to passive mode
          //   int sequence_identification_idx =
          //       sequence_identification_lists.indexOf(ActSequenceIdentificationList(ingress_interface_name, false));
          //   if (sequence_identification_idx == -1) {
          //     ActSequenceIdentificationList sequence_identification_list(ingress_interface_name, false);
          //     sequence_identification_list.SetStreamList(stream_handle);
          //     sequence_identification_lists.append(sequence_identification_list);
          //   } else {
          //     ActSequenceIdentificationList& sequence_identification_list =
          //         sequence_identification_lists[sequence_identification_idx];
          //     sequence_identification_list.SetStreamList(stream_handle);
          //   }
          // }

          // Generate sequence gereration list
          if (!sequence_id_generated) {
            ActSequenceGenerationList sequence_generation_list(frer.GetSequenceGenerationIndex(), false, false);
            sequence_generation_list.SetStreamList(stream_handle);
            sequence_generation_lists.append(sequence_generation_list);
            sequence_id_generated = true;
          }
        }

        // merge device need to change stream handle on egress port
        if (!used_link_ids.contains(ingress_link_id) && used_link_ids.contains(egress_link_id)) {
          stream_handle = stream_handle_3;
        }

        // Generate individual recovery
        if (!(used_link_ids.contains(ingress_link_id) && !used_link_ids.contains(egress_link_id))) {
          int sequence_recovery_idx = sequence_recovery_lists.indexOf(
              ActSequenceRecoveryList(egress_interface_name, stream_handle, ActSequenceRecoveryAlgorithm::kMatch));
          if (sequence_recovery_idx == -1) {
            ActSequenceRecoveryList sequence_recovery_list(frer.SequenceRecoveryIndexIncrease());
            sequence_recovery_list.SetStreamList(stream_handle);
            sequence_recovery_list.SetPortList(egress_interface_name);
            ActSequenceRecoveryEntry &sequence_recovery_entry = sequence_recovery_list.GetSequenceRecoveryEntry();
            sequence_recovery_entry.SetAlgorithm(ActSequenceRecoveryAlgorithm::kMatch);
            sequence_recovery_entry.SetIndividualRecovery(true);
            sequence_recovery_lists.append(sequence_recovery_list);
          }
        }

        // Generate sequence recovery on merge device
        if (!used_link_ids.contains(ingress_link_id) && used_link_ids.contains(egress_link_id)) {
          int sequence_recovery_idx = sequence_recovery_lists.indexOf(
              ActSequenceRecoveryList(egress_interface_name, stream_handle, ActSequenceRecoveryAlgorithm::kVector));
          if (sequence_recovery_idx == -1) {
            ActSequenceRecoveryList sequence_recovery_list(frer.SequenceRecoveryIndexIncrease());
            sequence_recovery_list.SetStreamList(stream_handle_1);
            sequence_recovery_list.SetStreamList(stream_handle);
            sequence_recovery_list.SetPortList(egress_interface_name);
            ActSequenceRecoveryEntry &sequence_recovery_entry = sequence_recovery_list.GetSequenceRecoveryEntry();
            sequence_recovery_entry.SetAlgorithm(ActSequenceRecoveryAlgorithm::kVector);
            sequence_recovery_entry.SetHistoryLength(HISTORY_LENGTH);
            sequence_recovery_entry.SetIndividualRecovery(false);
            sequence_recovery_lists.append(sequence_recovery_list);
          }
        }

        // Generate sequence identification list
        if (device_id != device_ids.last()) {
          if (!next_device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTSN().GetIEEE802Dot1CB()) {
            // If next device is not TSN or BridgedEndStation, R-tag should be remove, set to active mode
            int sequence_identification_idx =
                sequence_identification_lists.indexOf(ActSequenceIdentificationList(egress_interface_name, true));
            if (sequence_identification_idx == -1) {
              ActSequenceIdentificationList sequence_identification_list(egress_interface_name, true);
              sequence_identification_list.SetStreamList(stream_handle);
              sequence_identification_lists.append(sequence_identification_list);
            } else {
              ActSequenceIdentificationList &sequence_identification_list =
                  sequence_identification_lists[sequence_identification_idx];
              sequence_identification_list.SetStreamList(stream_handle);
            }
          } else {
            // If next device is TSN or BridgedEndStation, R-tag should be kept, set to passive mode
            int sequence_identification_idx =
                sequence_identification_lists.indexOf(ActSequenceIdentificationList(egress_interface_name, false));
            if (sequence_identification_idx == -1) {
              ActSequenceIdentificationList sequence_identification_list(egress_interface_name, false);
              sequence_identification_list.SetStreamList(stream_handle);
              sequence_identification_lists.append(sequence_identification_list);
            } else {
              ActSequenceIdentificationList &sequence_identification_list =
                  sequence_identification_lists[sequence_identification_idx];
              sequence_identification_list.SetStreamList(stream_handle);
            }
          }
        }

        // stream identity split egress port
        if (used_link_ids.contains(ingress_link_id) && !used_link_ids.contains(egress_link_id)) {
          int stream_identity_idx = stream_identity_list.indexOf(
              ActStreamIdentityEntry(stream_handle, ActIdentificationType::kDmac_vlan_stream_identification));
          if (stream_identity_idx == -1) {
            ActStreamIdentityEntry stream_identity_entry(stream_handle,
                                                         ActIdentificationType::kDmac_vlan_stream_identification);
            stream_identity_entry.SetIndex(cb_table.GetStreamIdentityIndex());
            ActTsnStreamIdEntryGroup &tsn_stream_id_entry_group = stream_identity_entry.GetTsnStreamIdEntryGroup();
            ActOutFacing &out_facing = tsn_stream_id_entry_group.GetOutFacing();
            QSet<QString> &output_port_list = out_facing.GetOutputPortList();
            output_port_list.insert(egress_interface_name);
            ActDmacVlanStreamIdentificationGroup &dmac_vlan_stream_identification =
                tsn_stream_id_entry_group.GetDmacVlanStreamIdentification();
            ActDown &down = dmac_vlan_stream_identification.GetDown();
            down.SetDestinationMac(destination_mac);
            down.SetTagged(ActVlanTagIdentificationType::kTagged);
            down.SetVlan(redundant_vlan_id);
            down.SetPriority(routing_result.GetPriorityCodePoint());
            ActUp &up = dmac_vlan_stream_identification.GetUp();
            up.SetDestinationMac(destination_mac);
            up.SetTagged(ActVlanTagIdentificationType::kTagged);
            up.SetVlan(stream_vlan_id);
            up.SetPriority(routing_result.GetPriorityCodePoint());
            stream_identity_list.append(stream_identity_entry);
          } else {
            ActStreamIdentityEntry &stream_identity_entry = stream_identity_list[stream_identity_idx];
            ActTsnStreamIdEntryGroup &tsn_stream_id_entry_group = stream_identity_entry.GetTsnStreamIdEntryGroup();
            ActOutFacing &out_facing = tsn_stream_id_entry_group.GetOutFacing();
            QSet<QString> &output_port_list = out_facing.GetOutputPortList();
            output_port_list.insert(egress_interface_name);
            ActDmacVlanStreamIdentificationGroup &dmac_vlan_stream_identification =
                tsn_stream_id_entry_group.GetDmacVlanStreamIdentification();
            ActDown &down = dmac_vlan_stream_identification.GetDown();
            down.SetDestinationMac(destination_mac);
            down.SetTagged(ActVlanTagIdentificationType::kTagged);
            down.SetVlan(redundant_vlan_id);
            down.SetPriority(routing_result.GetPriorityCodePoint());
            ActUp &up = dmac_vlan_stream_identification.GetUp();
            up.SetDestinationMac(destination_mac);
            up.SetTagged(ActVlanTagIdentificationType::kTagged);
            up.SetVlan(stream_vlan_id);
            up.SetPriority(routing_result.GetPriorityCodePoint());
          }
        }

        // stream identity merge egress port
        if (!used_link_ids.contains(ingress_link_id) && used_link_ids.contains(egress_link_id)) {
          int stream_identity_idx = stream_identity_list.indexOf(
              ActStreamIdentityEntry(stream_handle, ActIdentificationType::kDmac_vlan_stream_identification));
          if (stream_identity_idx == -1) {
            ActStreamIdentityEntry stream_identity_entry(stream_handle,
                                                         ActIdentificationType::kDmac_vlan_stream_identification);
            stream_identity_entry.SetIndex(cb_table.GetStreamIdentityIndex());
            ActTsnStreamIdEntryGroup &tsn_stream_id_entry_group = stream_identity_entry.GetTsnStreamIdEntryGroup();
            ActOutFacing &out_facing = tsn_stream_id_entry_group.GetOutFacing();
            QSet<QString> &output_port_list = out_facing.GetOutputPortList();
            output_port_list.insert(egress_interface_name);
            ActDmacVlanStreamIdentificationGroup &dmac_vlan_stream_identification =
                tsn_stream_id_entry_group.GetDmacVlanStreamIdentification();
            ActDown &down = dmac_vlan_stream_identification.GetDown();
            down.SetDestinationMac(destination_mac);
            down.SetTagged(ActVlanTagIdentificationType::kTagged);
            down.SetVlan(stream_vlan_id);
            down.SetPriority(routing_result.GetPriorityCodePoint());
            ActUp &up = dmac_vlan_stream_identification.GetUp();
            up.SetDestinationMac(destination_mac);
            up.SetTagged(ActVlanTagIdentificationType::kTagged);
            up.SetVlan(redundant_vlan_id);
            up.SetPriority(routing_result.GetPriorityCodePoint());
            stream_identity_list.append(stream_identity_entry);
          } else {
            ActStreamIdentityEntry &stream_identity_entry = stream_identity_list[stream_identity_idx];
            ActTsnStreamIdEntryGroup &tsn_stream_id_entry_group = stream_identity_entry.GetTsnStreamIdEntryGroup();
            ActOutFacing &out_facing = tsn_stream_id_entry_group.GetOutFacing();
            QSet<QString> &output_port_list = out_facing.GetOutputPortList();
            output_port_list.insert(egress_interface_name);
            ActDmacVlanStreamIdentificationGroup &dmac_vlan_stream_identification =
                tsn_stream_id_entry_group.GetDmacVlanStreamIdentification();
            ActDown &down = dmac_vlan_stream_identification.GetDown();
            down.SetDestinationMac(destination_mac);
            down.SetTagged(ActVlanTagIdentificationType::kTagged);
            down.SetVlan(stream_vlan_id);
            down.SetPriority(routing_result.GetPriorityCodePoint());
            ActUp &up = dmac_vlan_stream_identification.GetUp();
            up.SetDestinationMac(destination_mac);
            up.SetTagged(ActVlanTagIdentificationType::kTagged);
            up.SetVlan(redundant_vlan_id);
            up.SetPriority(routing_result.GetPriorityCodePoint());
          }
        }

        cb_tables_.insert(cb_table);
        used_link_ids.insert(ingress_link_id);
        // if (!used_link_ids.contains(ingress_link_id)) {
        //   used_link_ids[ingress_link_id].insert(stream_handle);
        // }
      }
      stream_handle = stream_handle_2;
    }
  }

  return ACT_STATUS_SUCCESS;
}