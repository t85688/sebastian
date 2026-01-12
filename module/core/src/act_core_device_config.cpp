#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"
#include "act_deploy_path_data.hpp"
namespace act {
namespace core {

ACT_STATUS ActCore::UpdateDeviceConfig(qint64 &project_id, ActDeviceConfig &device_config) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateDeviceConfig(project, device_config);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Cannot update the project id:" << project_id;
    return act_status;
  }

  // Send update msg
  ActDeviceConfigPatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetDeviceConfig(), true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateDeviceConfig(ActProject &project, ActDeviceConfig &device_config) {
  ACT_STATUS_INIT();

  // Check DeviceConfig
  act_status = this->CheckDeviceConfig(device_config);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Check DeviceConfig failed";

    return act_status;
  }

  project.SetDeviceConfig(device_config);

  return act_status;
}

ACT_STATUS ActCore::CheckDeviceConfig(const ActDeviceConfig &device_config) {
  ACT_STATUS_INIT();

  // TODO
  static_cast<void>(device_config);
  return act_status;
}

ACT_STATUS ActCore::GenerateDeviceConfigByComputeResult(const ActProject &project, ActDeviceConfig &device_config) {
  ACT_STATUS_INIT();

  ActDeviceConfig dev_cfg = project.GetDeviceConfig();

  act::deploy::ActDeployPathData deploy_path_data(project);
  act_status = deploy_path_data.GenerateData();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GenerateData() failed.";
    return act_status;
  }

  // VLAN Table(VlanStatic, PortVlan(PVID), VlanPortType)
  // Remove old TSN stream's vlan_static_entry
  for (ActDevice device : project.GetDevices()) {
    if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetAccessTrunkMode()) {
      continue;
    }
    ActVlanTable vlan_table(device.GetId());
    if (dev_cfg.GetVlanTables().contains(device.GetId())) {
      vlan_table = dev_cfg.GetVlanTables()[device.GetId()];
    } else {
      ActVlanStaticEntry vlan_static_entry(ACT_VLAN_INIT_PVID);
      for (ActInterface interface : device.GetInterfaces()) {
        vlan_static_entry.GetEgressPorts().insert(interface.GetInterfaceId());
        vlan_static_entry.GetUntaggedPorts().insert(interface.GetInterfaceId());
        if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetDefaultPVID()) {
          vlan_table.GetPortVlanEntries().insert(
              ActPortVlanEntry(interface.GetInterfaceId(), ACT_VLAN_INIT_PVID, ActVlanPriorityEnum::kNonTSN));
        }
        if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetAccessTrunkMode()) {
          vlan_table.GetVlanPortTypeEntries().insert(ActVlanPortTypeEntry(
              interface.GetInterfaceId(), ActVlanPortTypeEnum::kAccess, ActVlanPriorityEnum::kNonTSN));
        }
      }
      vlan_table.GetVlanStaticEntries().insert(vlan_static_entry);
    }

    QSet<ActVlanStaticEntry> vlan_static_entries;
    for (ActVlanStaticEntry vlan_static_entry : vlan_table.GetVlanStaticEntries()) {
      if (vlan_static_entry.GetVlanPriority() == ActVlanPriorityEnum::kNonTSN) {
        vlan_static_entries.insert(vlan_static_entry);
      } else if (device.GetDeviceProperty().GetReservedVlan().contains(vlan_static_entry.GetVlanId())) {
        vlan_static_entries.insert(vlan_static_entry);
      }
    }
    vlan_table.SetVlanStaticEntries(vlan_static_entries);

    QSet<ActPortVlanEntry> port_vlan_entries;
    for (ActPortVlanEntry port_vlan_entry : vlan_table.GetPortVlanEntries()) {
      if (port_vlan_entry.GetVlanPriority() != ActVlanPriorityEnum::kNonTSN) {
        port_vlan_entry.SetPVID(ACT_VLAN_INIT_PVID);
        port_vlan_entry.SetVlanPriority(ActVlanPriorityEnum::kNonTSN);
      }
      port_vlan_entries.insert(port_vlan_entry);
    }
    vlan_table.SetPortVlanEntries(port_vlan_entries);

    QSet<ActVlanPortTypeEntry> vlan_port_type_entries;
    for (ActVlanPortTypeEntry vlan_port_type_entry : vlan_table.GetVlanPortTypeEntries()) {
      if (vlan_port_type_entry.GetVlanPriority() != ActVlanPriorityEnum::kNonTSN) {
        vlan_port_type_entry.SetVlanPortType(ActVlanPortTypeEnum::kAccess);
        vlan_port_type_entry.SetVlanPriority(ActVlanPriorityEnum::kNonTSN);
      }
      vlan_port_type_entries.insert(vlan_port_type_entry);
    }
    vlan_table.SetVlanPortTypeEntries(vlan_port_type_entries);

    dev_cfg.GetVlanTables()[device.GetId()] = vlan_table;
  }

  // Add TSN stream VLAN
  for (ActVlanStaticTable vlan_static_table : deploy_path_data.GetVlanStaticTables()) {
    qint64 dev_id = vlan_static_table.GetDeviceId();
    ActDevice device;
    act_status = project.GetDeviceById(device, dev_id);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    ActVlanTable vlan_table(dev_id);
    // Get Old VlanTable (Project > DeviceConfig)
    if (dev_cfg.GetVlanTables().contains(dev_id)) {
      vlan_table = dev_cfg.GetVlanTables()[dev_id];
    }

    // VlanStatic
    for (ActVlanStaticEntry vlan_static_entry : vlan_static_table.GetVlanStaticEntries()) {
      // Remove old duplicated entry
      if (vlan_table.GetVlanStaticEntries().contains(vlan_static_entry) &&
          !device.GetDeviceProperty().GetReservedVlan().contains(vlan_static_entry.GetVlanId())) {
        vlan_table.GetVlanStaticEntries().remove(vlan_static_entry);
      }
      vlan_table.GetVlanStaticEntries().insert(vlan_static_entry);
    }

    // PortVlan(PVID)
    auto port_vlan_tables = deploy_path_data.GetPortVlanTables();
    auto port_vlan_table_iter = port_vlan_tables.find(ActPortVlanTable(dev_id));
    if (port_vlan_table_iter != port_vlan_tables.end()) {  // found
      for (ActPortVlanEntry port_vlan_entry : port_vlan_table_iter->GetPortVlanEntries()) {
        auto port_vlan_entry_iter = vlan_table.GetPortVlanEntries().find(port_vlan_entry);
        if (port_vlan_entry_iter != vlan_table.GetPortVlanEntries().end()) {
          vlan_table.GetPortVlanEntries().erase(port_vlan_entry_iter);
        }
        vlan_table.GetPortVlanEntries().insert(port_vlan_entry);
      }
    }

    // VlanPortType
    auto vlan_port_type_tables = deploy_path_data.GetVlanPortTypeTables();
    auto vlan_port_type_table_iter = vlan_port_type_tables.find(ActVlanPortTypeTable(dev_id));
    if (vlan_port_type_table_iter != vlan_port_type_tables.end()) {  // found
      for (ActVlanPortTypeEntry vlan_port_type_entry : vlan_port_type_table_iter->GetVlanPortTypeEntries()) {
        auto vlan_port_type_entry_iter = vlan_table.GetVlanPortTypeEntries().find(vlan_port_type_entry);
        if (vlan_port_type_entry_iter != vlan_table.GetVlanPortTypeEntries().end()) {
          vlan_table.GetVlanPortTypeEntries().erase(vlan_port_type_entry_iter);
        }
        vlan_table.GetVlanPortTypeEntries().insert(vlan_port_type_entry);
      }
    }

    // Replace VLAN table
    dev_cfg.GetVlanTables()[dev_id] = vlan_table;
  }

  // // RSTP (Add the not exists table)
  // for (auto table : deploy_path_data.GetRstpTables()) {
  //   // if (!dev_cfg.GetRstpTables().contains(table.GetDeviceId())) {  // not exists
  //   //   dev_cfg.GetRstpTables()[table.GetDeviceId()] = table;
  //   // }
  //   dev_cfg.GetRstpTables()[table.GetDeviceId()] = table;
  // }

  // Unicast StaticForward
  for (auto table : deploy_path_data.GetUnicastStaticForwardTables()) {
    auto &unicast_static_forward_table = dev_cfg.GetUnicastStaticForwardTables()[table.GetDeviceId()];
    auto &multicast_static_forward_table = dev_cfg.GetMulticastStaticForwardTables()[table.GetDeviceId()];
    for (auto entry : table.GetStaticForwardEntries()) {
      auto unicast_static_forward_entry_iter = unicast_static_forward_table.GetStaticForwardEntries().find(entry);
      if (unicast_static_forward_entry_iter != unicast_static_forward_table.GetStaticForwardEntries().end()) {
        unicast_static_forward_table.GetStaticForwardEntries().erase(unicast_static_forward_entry_iter);
      }
      auto multicast_static_forward_entry_iter = multicast_static_forward_table.GetStaticForwardEntries().find(entry);
      if (multicast_static_forward_entry_iter != multicast_static_forward_table.GetStaticForwardEntries().end()) {
        multicast_static_forward_table.GetStaticForwardEntries().erase(multicast_static_forward_entry_iter);
      }
      unicast_static_forward_table.GetStaticForwardEntries().insert(entry);
    }
  }

  // Remove doesn't contain in vlan table entry
  for (auto device_id : dev_cfg.GetUnicastStaticForwardTables().keys()) {
    auto vlan_table = dev_cfg.GetVlanTables()[device_id];
    auto &static_forward_table = dev_cfg.GetUnicastStaticForwardTables()[device_id];
    auto static_forward_entries = static_forward_table.GetStaticForwardEntries();
    for (auto static_forward_entry : static_forward_entries) {
      ActVlanStaticEntry vlan_static_entry(static_forward_entry.GetVlanId());
      if (!vlan_table.GetVlanStaticEntries().contains(vlan_static_entry)) {
        static_forward_table.GetStaticForwardEntries().remove(static_forward_entry);
      } else {
        vlan_static_entry = *vlan_table.GetVlanStaticEntries().find(vlan_static_entry);
        static_forward_entry.GetEgressPorts().intersect(vlan_static_entry.GetEgressPorts());
        if (static_forward_entry.GetEgressPorts().isEmpty()) {
          static_forward_table.GetStaticForwardEntries().remove(static_forward_entry);
        }
      }
    }
  }

  // Multicast StaticForward
  for (auto table : deploy_path_data.GetMulticastStaticForwardTables()) {
    auto &unicast_static_forward_table = dev_cfg.GetUnicastStaticForwardTables()[table.GetDeviceId()];
    auto &multicast_static_forward_table = dev_cfg.GetMulticastStaticForwardTables()[table.GetDeviceId()];
    for (auto entry : table.GetStaticForwardEntries()) {
      auto unicast_static_forward_entry_iter = unicast_static_forward_table.GetStaticForwardEntries().find(entry);
      if (unicast_static_forward_entry_iter != unicast_static_forward_table.GetStaticForwardEntries().end()) {
        unicast_static_forward_table.GetStaticForwardEntries().erase(unicast_static_forward_entry_iter);
      }
      auto multicast_static_forward_entry_iter = multicast_static_forward_table.GetStaticForwardEntries().find(entry);
      if (multicast_static_forward_entry_iter != multicast_static_forward_table.GetStaticForwardEntries().end()) {
        multicast_static_forward_table.GetStaticForwardEntries().erase(multicast_static_forward_entry_iter);
      }
      multicast_static_forward_table.GetStaticForwardEntries().insert(entry);
    }
  }

  // Remove doesn't contain in vlan table entry
  for (auto device_id : dev_cfg.GetMulticastStaticForwardTables().keys()) {
    auto vlan_table = dev_cfg.GetVlanTables()[device_id];
    auto &static_forward_table = dev_cfg.GetMulticastStaticForwardTables()[device_id];
    auto static_forward_entries = static_forward_table.GetStaticForwardEntries();
    for (auto static_forward_entry : static_forward_entries) {
      ActVlanStaticEntry vlan_static_entry(static_forward_entry.GetVlanId());
      if (!vlan_table.GetVlanStaticEntries().contains(vlan_static_entry)) {
        static_forward_table.GetStaticForwardEntries().remove(static_forward_entry);
      } else {
        vlan_static_entry = *vlan_table.GetVlanStaticEntries().find(vlan_static_entry);
        static_forward_entry.GetEgressPorts().intersect(vlan_static_entry.GetEgressPorts());
        if (static_forward_entry.GetEgressPorts().isEmpty()) {
          static_forward_table.GetStaticForwardEntries().remove(static_forward_entry);
        }
      }
    }
  }

  // Port Default PCP
  for (auto table : deploy_path_data.GetDefaultPriorityTables()) {
    auto &default_priority_table = dev_cfg.GetPortDefaultPCPTables()[table.GetDeviceId()];
    auto &default_priority_entries = default_priority_table.GetDefaultPriorityEntries();
    for (auto entry : table.GetDefaultPriorityEntries()) {
      auto default_priority_entry_iter = default_priority_entries.find(entry);
      if (default_priority_entry_iter != default_priority_entries.end()) {
        default_priority_entries.erase(default_priority_entry_iter);
      }
      default_priority_entries.insert(entry);
    }
  }

  // StreamPriorityIngress
  for (auto table : deploy_path_data.GetStadPortTables()) {
    auto &stream_priority_ingress_table = dev_cfg.GetStreamPriorityIngressTables()[table.GetDeviceId()];
    auto &interface_stad_port_entries = stream_priority_ingress_table.GetInterfaceStadPortEntries();
    for (auto entry : table.GetInterfaceStadPortEntries()) {
      auto interface_stad_port_entry_iter = interface_stad_port_entries.find(entry);
      if (interface_stad_port_entry_iter != interface_stad_port_entries.end()) {
        interface_stad_port_entries.erase(interface_stad_port_entry_iter);
      }
      interface_stad_port_entries.insert(entry);
    }
  }

  // StreamPriorityEgress
  for (auto table : deploy_path_data.GetStadConfigTables()) {
    auto &stream_priority_egress_table = dev_cfg.GetStreamPriorityEgressTables()[table.GetDeviceId()];
    auto &stad_config_entries = stream_priority_egress_table.GetStadConfigEntries();
    for (auto entry : table.GetStadConfigEntries()) {
      auto stad_config_entry_iter = stad_config_entries.find(entry);
      if (stad_config_entry_iter != stad_config_entries.end()) {
        stad_config_entries.erase(stad_config_entry_iter);
      }
      stad_config_entries.insert(entry);
    }
  }

  // GCL
  for (auto table : deploy_path_data.GetGCLTables()) {
    auto &gcl_table = dev_cfg.GetGCLTables()[table.GetDeviceId()];
    auto &interfaces_gate_parameters = gcl_table.GetInterfacesGateParameters();
    for (auto entry : table.GetInterfacesGateParameters()) {
      auto interfaces_gate_parameter_iter = interfaces_gate_parameters.find(entry);
      if (interfaces_gate_parameter_iter != interfaces_gate_parameters.end()) {
        interfaces_gate_parameters.erase(interfaces_gate_parameter_iter);
      }
      interfaces_gate_parameters.insert(entry);
    }
  }

  // CB
  for (auto table : deploy_path_data.GetCbTables()) {
    dev_cfg.GetCbTables()[table.GetDeviceId()] = table;
  }

  device_config = dev_cfg;
  return act_status;
}

ACT_STATUS ActCore::GenerateVLANStaticDeviceConfigByComputeResult(const ActProject &project,
                                                                  ActDeviceConfig &device_config) {
  ACT_STATUS_INIT();

  ActDeviceConfig dev_cfg = project.GetDeviceConfig();

  act::deploy::ActDeployPathData deploy_path_data(project);
  act_status = deploy_path_data.GenerateVlanStaticData();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GenerateVlanStaticData() failed.";
    return act_status;
  }

  // VLAN Table(VlanStatic)
  // Remove old TSN stream's vlan_static_entry
  for (auto old_vlan_table_id : dev_cfg.GetVlanTables().keys()) {
    ActDevice dev;
    act_status = project.GetDeviceById(dev, old_vlan_table_id);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    QSet<ActVlanStaticEntry> keep_vlan_static_entries;
    for (auto old_vlan_static_entry : dev_cfg.GetVlanTables()[old_vlan_table_id].GetVlanStaticEntries()) {
      if ((old_vlan_static_entry.GetVlanPriority() == ActVlanPriorityEnum::kNonTSN) ||
          dev.GetDeviceProperty().GetReservedVlan().contains(old_vlan_static_entry.GetVlanId())) {
        keep_vlan_static_entries.insert(old_vlan_static_entry);
      }
    }
    dev_cfg.GetVlanTables()[old_vlan_table_id].SetVlanStaticEntries(keep_vlan_static_entries);
  }

  // Add TSN stream VLAN
  for (auto vlan_static_table : deploy_path_data.GetVlanStaticTables()) {
    auto dev_id = vlan_static_table.GetDeviceId();

    ActVlanTable cfg_vlan_table(dev_id);
    // Get Old VlanTable (Project > DeviceConfig)
    if (dev_cfg.GetVlanTables().contains(dev_id)) {
      cfg_vlan_table = dev_cfg.GetVlanTables()[dev_id];
    }

    // VlanStatic
    for (auto new_vlan_static_entry : vlan_static_table.GetVlanStaticEntries()) {
      // Remove old duplicated entry
      if (cfg_vlan_table.GetVlanStaticEntries().contains(new_vlan_static_entry)) {
        cfg_vlan_table.GetVlanStaticEntries().remove(new_vlan_static_entry);
      }
      cfg_vlan_table.GetVlanStaticEntries().insert(new_vlan_static_entry);
    }

    // Replace VLAN table
    dev_cfg.GetVlanTables()[dev_id] = cfg_vlan_table;
  }

  device_config = dev_cfg;
  return act_status;
}

bool ActCore::CanDeployProject(qint64 project_id) const { return this->deploy_available[project_id]; }

bool CheckDeviceConfigIsEmpty(const ActDeviceConfig &device_config) {
  // NetworkSettingTables
  if (!device_config.GetNetworkSettingTables().isEmpty()) {
    return false;
  }

  // UserAccount
  if (!device_config.GetUserAccountTables().isEmpty()) {
    return false;
  }

  // TimeSync
  if (!device_config.GetTimeSyncTables().isEmpty()) {
    return false;
  }

  // LoginPolicy
  if (!device_config.GetLoginPolicyTables().isEmpty()) {
    return false;
  }

  // LoopProtection
  if (!device_config.GetLoopProtectionTables().isEmpty()) {
    return false;
  }

  // SNMP Trap Setting
  if (!device_config.GetSnmpTrapSettingTables().isEmpty()) {
    return false;
  }

  // Syslog Setting
  if (!device_config.GetSyslogSettingTables().isEmpty()) {
    return false;
  }

  // Time Setting
  if (!device_config.GetTimeSettingTables().isEmpty()) {
    return false;
  }

  // InformationSetting
  if (!device_config.GetInformationSettingTables().isEmpty()) {
    return false;
  }

  // ManagementInterface
  if (!device_config.GetManagementInterfaceTables().isEmpty()) {
    return false;
  }

  // PortSettingTables
  if (!device_config.GetPortSettingTables().isEmpty()) {
    return false;
  }

  // VlanTables
  if (!device_config.GetVlanTables().isEmpty()) {
    return false;
  }

  // UnicastStaticForwardTables
  if (!device_config.GetUnicastStaticForwardTables().isEmpty()) {
    return false;
  }

  // MulticastStaticForwardTables
  if (!device_config.GetMulticastStaticForwardTables().isEmpty()) {
    return false;
  }

  // PortDefaultPCPTables
  if (!device_config.GetPortDefaultPCPTables().isEmpty()) {
    return false;
  }

  // StreamPriorityIngressTables
  if (!device_config.GetStreamPriorityIngressTables().isEmpty()) {
    return false;
  }

  // StreamPriorityEgressTables
  if (!device_config.GetStreamPriorityEgressTables().isEmpty()) {
    return false;
  }

  // GCLTables
  if (!device_config.GetGCLTables().isEmpty()) {
    return false;
  }

  // CbTables
  if (!device_config.GetCbTables().isEmpty()) {
    return false;
  }

  // RstpTables
  if (!device_config.GetRstpTables().isEmpty()) {
    return false;
  }

  return true;
}

bool ActCore::CheckDeployAvailableByDeviceConfig(const ActProject &project) {
  // Check DeviceConfig not empty
  if (CheckDeviceConfigIsEmpty(project.GetDeviceConfig())) {
    // qDebug() << __func__ << QString("Project(%1): DeviceConfig is empty").arg(project.GetId());
    return false;
  }

  // Generate new device config
  ActDeviceConfig new_device_config;
  auto act_status = this->GenerateVLANStaticDeviceConfigByComputeResult(project, new_device_config);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << __func__
             << QString("Project(%1): GenerateVLANStaticDeviceConfigByComputeResult() failed").arg(project.GetId());
    return false;
  }

  // Check VLAN Static
  auto &old_vlan_tables = project.GetDeviceConfig().GetVlanTables();
  for (auto new_vlan_table : new_device_config.GetVlanTables()) {
    // Check old vlan_table exist
    if (!old_vlan_tables.contains(new_vlan_table.GetDeviceId())) {
      qDebug() << __func__
               << QString("Project(%1): DeviceConfig has no VlanTable(%2)")
                      .arg(project.GetId())
                      .arg(new_vlan_table.GetDeviceId());
      return false;
    }

    auto &old_vlan_table = old_vlan_tables[new_vlan_table.GetDeviceId()];
    // Check vlan_static_entry
    for (auto new_vlan_static_entry : new_vlan_table.GetVlanStaticEntries()) {
      // Only check VlanPriority is TSNSystem
      if (new_vlan_static_entry.GetVlanPriority() != ActVlanPriorityEnum::kTSNSystem) {
        continue;
      }

      // Check old vlan_static_entry exist
      ActVlanStaticEntry old_vlan_static_entry(new_vlan_static_entry.GetVlanId());
      auto old_vlan_static_entry_iter = old_vlan_table.GetVlanStaticEntries().find(old_vlan_static_entry);
      if (old_vlan_static_entry_iter == old_vlan_table.GetVlanStaticEntries().end()) {  // not found
        qDebug() << __func__
                 << QString("Project(%1): VlanTable(%2) has no vlan_static_entry(%3)")
                        .arg(project.GetId())
                        .arg(new_vlan_table.GetDeviceId())
                        .arg(new_vlan_static_entry.GetVlanId());
        return false;
      }

      // Check Priority same as TSNSystem
      ActVlanStaticEntry old_ent = *old_vlan_static_entry_iter;

      if (old_vlan_static_entry_iter->GetVlanPriority() != ActVlanPriorityEnum::kTSNSystem) {
        qDebug() << __func__
                 << QString("Project(%1): VlanStaticEntry(VLAN_ID: %2) VlanPriority no as TSNSystem")
                        .arg(project.GetId())
                        .arg(new_vlan_static_entry.GetVlanId());
        return false;
      }
    }
  }

  return true;
}

ACT_STATUS ActCore::DeleteDeviceDeviceConfig(ActProject &project, qint64 &device_id) {
  ACT_STATUS_INIT();

  if (project.GetDeviceConfig().GetMappingDeviceIpSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetMappingDeviceIpSettingTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetNetworkSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetNetworkSettingTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetUserAccountTables().contains(device_id)) {
    project.GetDeviceConfig().GetUserAccountTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetTimeSyncTables().contains(device_id)) {
    project.GetDeviceConfig().GetTimeSyncTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetLoginPolicyTables().contains(device_id)) {
    project.GetDeviceConfig().GetLoginPolicyTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetLoopProtectionTables().contains(device_id)) {
    project.GetDeviceConfig().GetLoopProtectionTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetSnmpTrapSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetSnmpTrapSettingTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetSyslogSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetSyslogSettingTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetTimeSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetTimeSettingTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetInformationSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetInformationSettingTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetManagementInterfaceTables().contains(device_id)) {
    project.GetDeviceConfig().GetManagementInterfaceTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetPortSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetPortSettingTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetVlanTables().contains(device_id)) {
    project.GetDeviceConfig().GetVlanTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetUnicastStaticForwardTables().contains(device_id)) {
    project.GetDeviceConfig().GetUnicastStaticForwardTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetMulticastStaticForwardTables().contains(device_id)) {
    project.GetDeviceConfig().GetMulticastStaticForwardTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetPortDefaultPCPTables().contains(device_id)) {
    project.GetDeviceConfig().GetPortDefaultPCPTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetStreamPriorityIngressTables().contains(device_id)) {
    project.GetDeviceConfig().GetStreamPriorityIngressTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetStreamPriorityEgressTables().contains(device_id)) {
    project.GetDeviceConfig().GetStreamPriorityEgressTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetGCLTables().contains(device_id)) {
    project.GetDeviceConfig().GetGCLTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetCbTables().contains(device_id)) {
    project.GetDeviceConfig().GetCbTables().remove(device_id);
  }

  if (project.GetDeviceConfig().GetRstpTables().contains(device_id)) {
    project.GetDeviceConfig().GetRstpTables().remove(device_id);
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateDeviceDeviceConfig(ActProject &project, const ActDeviceConfig &update_device_config,
                                             const qint64 &device_id) {
  ACT_STATUS_INIT();

  if (update_device_config.GetMappingDeviceIpSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetMappingDeviceIpSettingTables()[device_id] =
        update_device_config.GetMappingDeviceIpSettingTables()[device_id];
  }

  if (update_device_config.GetNetworkSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetNetworkSettingTables()[device_id] =
        update_device_config.GetNetworkSettingTables()[device_id];
  }

  if (update_device_config.GetUserAccountTables().contains(device_id)) {
    project.GetDeviceConfig().GetUserAccountTables()[device_id] =
        update_device_config.GetUserAccountTables()[device_id];
  }

  if (update_device_config.GetTimeSyncTables().contains(device_id)) {
    project.GetDeviceConfig().GetTimeSyncTables()[device_id] = update_device_config.GetTimeSyncTables()[device_id];
  }

  if (update_device_config.GetLoginPolicyTables().contains(device_id)) {
    project.GetDeviceConfig().GetLoginPolicyTables()[device_id] =
        update_device_config.GetLoginPolicyTables()[device_id];
  }

  if (update_device_config.GetLoopProtectionTables().contains(device_id)) {
    project.GetDeviceConfig().GetLoopProtectionTables()[device_id] =
        update_device_config.GetLoopProtectionTables()[device_id];
  }

  if (update_device_config.GetSnmpTrapSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetSnmpTrapSettingTables()[device_id] =
        update_device_config.GetSnmpTrapSettingTables()[device_id];
  }

  if (update_device_config.GetSyslogSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetSyslogSettingTables()[device_id] =
        update_device_config.GetSyslogSettingTables()[device_id];
  }

  if (update_device_config.GetTimeSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetTimeSettingTables()[device_id] =
        update_device_config.GetTimeSettingTables()[device_id];
  }

  if (update_device_config.GetInformationSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetInformationSettingTables()[device_id] =
        update_device_config.GetInformationSettingTables()[device_id];
  }

  if (update_device_config.GetManagementInterfaceTables().contains(device_id)) {
    project.GetDeviceConfig().GetManagementInterfaceTables()[device_id] =
        update_device_config.GetManagementInterfaceTables()[device_id];
  }

  if (update_device_config.GetPortSettingTables().contains(device_id)) {
    project.GetDeviceConfig().GetPortSettingTables()[device_id] =
        update_device_config.GetPortSettingTables()[device_id];
  }

  if (update_device_config.GetVlanTables().contains(device_id)) {
    project.GetDeviceConfig().GetVlanTables()[device_id] = update_device_config.GetVlanTables()[device_id];
  }

  if (update_device_config.GetUnicastStaticForwardTables().contains(device_id)) {
    project.GetDeviceConfig().GetUnicastStaticForwardTables()[device_id] =
        update_device_config.GetUnicastStaticForwardTables()[device_id];
  }

  if (update_device_config.GetMulticastStaticForwardTables().contains(device_id)) {
    project.GetDeviceConfig().GetMulticastStaticForwardTables()[device_id] =
        update_device_config.GetMulticastStaticForwardTables()[device_id];
  }

  if (update_device_config.GetPortDefaultPCPTables().contains(device_id)) {
    project.GetDeviceConfig().GetPortDefaultPCPTables()[device_id] =
        update_device_config.GetPortDefaultPCPTables()[device_id];
  }

  if (update_device_config.GetStreamPriorityIngressTables().contains(device_id)) {
    project.GetDeviceConfig().GetStreamPriorityIngressTables()[device_id] =
        update_device_config.GetStreamPriorityIngressTables()[device_id];
  }

  if (update_device_config.GetStreamPriorityEgressTables().contains(device_id)) {
    project.GetDeviceConfig().GetStreamPriorityEgressTables()[device_id] =
        update_device_config.GetStreamPriorityEgressTables()[device_id];
  }

  if (update_device_config.GetGCLTables().contains(device_id)) {
    project.GetDeviceConfig().GetGCLTables()[device_id] = update_device_config.GetGCLTables()[device_id];
  }

  if (update_device_config.GetCbTables().contains(device_id)) {
    project.GetDeviceConfig().GetCbTables()[device_id] = update_device_config.GetCbTables()[device_id];
  }

  if (update_device_config.GetRstpTables().contains(device_id)) {
    project.GetDeviceConfig().GetRstpTables()[device_id] = update_device_config.GetRstpTables()[device_id];
  }

  return act_status;
}

}  // namespace core
}  // namespace act
