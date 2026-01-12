#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::CreateVlanGroup(qint64 &project_id, ActIntelligentVlan &intelligent_vlan) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->CreateVlanGroup(project, intelligent_vlan);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Create VLAN group failed";
    return act_status;
  }

  act_status = this->ComputeVlanConfig(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute VLAN config failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::CreateVlanGroup(ActProject &project, ActIntelligentVlan &intelligent_vlan) {
  ACT_STATUS_INIT();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support System VLAN Setting";
    qCritical() << __func__ << error_msg;
    return std::make_shared<ActLicenseNotActiveFailed>("System VLAN Setting");
  } */

  quint16 vlan_id = intelligent_vlan.GetVlanId();
  QSet<qint64> end_device_ids = intelligent_vlan.GetEndStationList();

  if (end_device_ids.size() < 2) {
    return std::make_shared<ActStatusFeasibilityCheckFailed>(
        QString("Create vlan group need more than one end-devices"));
  }

  // Check vlan is not exist
  if (project.GetTopologySetting().GetIntelligentVlanGroup().contains(intelligent_vlan)) {
    return std::make_shared<ActDuplicatedError>(QString("vlan id %1").arg(vlan_id));
  }

  for (ActStream stream : project.GetStreams()) {
    if (stream.GetVlanId() == vlan_id) {
      return std::make_shared<ActDuplicatedError>(QString("vlan id %1").arg(vlan_id));
    }
  }

  project.GetTopologySetting().GetIntelligentVlanGroup().insert(intelligent_vlan);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateVlanGroup(qint64 &project_id, ActIntelligentVlan &intelligent_vlan) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateVlanGroup(project, intelligent_vlan);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update vlan group failed";
    return act_status;
  }

  act_status = this->ComputeVlanConfig(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute VLAN config failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateVlanGroup(ActProject &project, ActIntelligentVlan &intelligent_vlan) {
  ACT_STATUS_INIT();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support System VLAN Setting";
    qCritical() << __func__ << error_msg;
    return std::make_shared<ActLicenseNotActiveFailed>("System VLAN Setting");
  } */

  act_status = this->DeleteVlanGroup(project, intelligent_vlan.GetVlanId());
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete vlan group failed with vlan id:" << intelligent_vlan.GetVlanId();
    return act_status;
  }

  act_status = this->CreateVlanGroup(project, intelligent_vlan);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Create vlan group failed";
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::DeleteVlanGroup(qint64 &project_id, quint16 &vlan_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteVlanGroup(project, vlan_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete vlan_config failed with vlan id:" << vlan_id;
    return act_status;
  }

  act_status = this->ComputeVlanConfig(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute VLAN config failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::DeleteVlanGroup(ActProject &project, quint16 &vlan_id) {
  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support System VLAN Setting";
    qCritical() << __func__ << error_msg;
    return std::make_shared<ActLicenseNotActiveFailed>("System VLAN Setting");
  } */

  project.GetTopologySetting().GetIntelligentVlanGroup().remove(ActIntelligentVlan(vlan_id));

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateVlanConfig(qint64 &project_id, ActVlanTable &vlan_config_table) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateVlanConfig(project, vlan_config_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update vlan_config failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateVlanConfig(ActProject &project, ActVlanTable &vlan_config_table) {
  ACT_STATUS_INIT();

  // Check device exists
  ActDevice device;
  QSet<ActDevice> device_set = project.GetDevices();
  act_status = ActGetItemById<ActDevice>(device_set, vlan_config_table.GetDeviceId(), device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Device id:" << vlan_config_table.GetDeviceId() << "not found";
    return act_status;
  }

  // Update the VlanConfig table
  project.GetDeviceConfig().GetVlanTables()[device.GetId()] = vlan_config_table;

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateVlanConfig(qint64 &project_id, ActVlanConfig &vlan_config) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateVlanConfig(project, vlan_config);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update vlan_config failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateVlanConfig(ActProject &project, ActVlanConfig &vlan_config) {
  ACT_STATUS_INIT();

  qint32 vlan_id = vlan_config.GetVlanId();
  ActVlanPortTypeEnum port_type = vlan_config.GetPortType();

  for (qint64 device_id : vlan_config.GetDevices()) {
    ActDevice device;
    act_status = project.GetDeviceById(device, device_id);
    if (!IsActStatusSuccess(act_status)) {
      QString err_msg = QString("Device %1 in project").arg(device_id);
      return std::make_shared<ActStatusNotFound>(err_msg);
    }

    QSet<qint64> ports;
    for (ActInterface interface : device.GetInterfaces()) {
      ports.insert(interface.GetInterfaceId());
    }

    if ((port_type == ActVlanPortTypeEnum::kAccess || port_type == ActVlanPortTypeEnum::kTrunk) &&
        !device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetAccessTrunkMode()) {
      QString err_msg = QString("Device %1 doesn't support Access/Trunk mode").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(err_msg);
    } else if (port_type == ActVlanPortTypeEnum::kHybrid &&
               !device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) {
      QString err_msg = QString("Device %1 doesn't support Hybrid mode").arg(device.GetIpv4().GetIpAddress());
      return std::make_shared<ActBadRequest>(err_msg);
    }

    ActVlanTable vlan_table(device_id);
    act_status = this->GetVlanConfig(project, device_id, vlan_table);
    if (!IsActStatusSuccess(act_status)) {
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

    for (qint64 port : vlan_config.GetPorts()) {
      if (!ports.contains(port)) {
        QString err_msg = QString("Port %1 in device %2").arg(port).arg(device.GetIpv4().GetIpAddress());
        return std::make_shared<ActStatusNotFound>(err_msg);
      }
      // vlan static entry
      {
        QSet<ActVlanStaticEntry> vlan_static_entries;
        for (ActVlanStaticEntry vlan_static_entry : vlan_table.GetVlanStaticEntries()) {
          if (port_type == ActVlanPortTypeEnum::kAccess) {
            vlan_static_entry.GetEgressPorts().remove(port);
            vlan_static_entry.GetUntaggedPorts().remove(port);
          } else if (port_type == ActVlanPortTypeEnum::kTrunk) {
            vlan_static_entry.GetUntaggedPorts().remove(port);
          }
          if (!vlan_static_entry.GetEgressPorts().isEmpty() || vlan_static_entry.GetVlanId() == ACT_VLAN_INIT_PVID ||
              device.GetDeviceProperty().GetReservedVlan().contains(vlan_static_entry.GetVlanId())) {
            vlan_static_entries.insert(vlan_static_entry);
          }
        }
        vlan_table.SetVlanStaticEntries(vlan_static_entries);
      }

      ActVlanStaticEntry vlan_static_entry(vlan_id);
      QSet<ActVlanStaticEntry> &vlan_static_entries = vlan_table.GetVlanStaticEntries();
      QSet<ActVlanStaticEntry>::iterator vlan_static_entry_iter = vlan_static_entries.find(vlan_static_entry);
      if (vlan_static_entry_iter != vlan_static_entries.end()) {
        vlan_static_entry = *vlan_static_entry_iter;
        vlan_static_entries.erase(vlan_static_entry_iter);
      }

      vlan_static_entry.SetVlanPriority(ActVlanPriorityEnum::kNonTSN);
      vlan_static_entry.SetName(QString("v%1").arg(vlan_id));
      if (port_type == ActVlanPortTypeEnum::kAccess) {
        vlan_static_entry.GetUntaggedPorts().insert(port);
      }
      vlan_static_entry.GetEgressPorts().insert(port);
      vlan_static_entries.insert(vlan_static_entry);

      // port vlan entry
      if (port_type == ActVlanPortTypeEnum::kAccess) {
        QSet<ActPortVlanEntry> &port_vlan_entries = vlan_table.GetPortVlanEntries();
        ActPortVlanEntry port_vlan_entry(port, static_cast<quint16>(vlan_id), ActVlanPriorityEnum::kNonTSN);
        QSet<ActPortVlanEntry>::iterator port_vlan_entry_iter = port_vlan_entries.find(port_vlan_entry);
        if (port_vlan_entry_iter != port_vlan_entries.end()) {
          port_vlan_entries.erase(port_vlan_entry_iter);
        }
        port_vlan_entries.insert(port_vlan_entry);
      }

      // vlan port type entry
      QSet<ActVlanPortTypeEntry> &vlan_port_type_entries = vlan_table.GetVlanPortTypeEntries();
      ActVlanPortTypeEntry vlan_port_type_entry(port, port_type, ActVlanPriorityEnum::kNonTSN);
      QSet<ActVlanPortTypeEntry>::iterator vlan_port_type_entry_iter =
          vlan_port_type_entries.find(vlan_port_type_entry);
      if (vlan_port_type_entry_iter != vlan_port_type_entries.end()) {
        vlan_port_type_entries.erase(vlan_port_type_entry_iter);
      }
      vlan_port_type_entries.insert(vlan_port_type_entry);
    }

    act_status = this->UpdateVlanConfig(project, vlan_table);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::DeleteVlanConfig(qint64 &project_id, qint64 &device_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteVlanConfig(project, device_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete vlan_config failed with device id:" << device_id;
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::DeleteVlanConfig(ActProject &project, qint64 &device_id) {
  // Remove VLAN config table
  auto &vlan_cfg_table_map = project.GetDeviceConfig().GetVlanTables();
  if (vlan_cfg_table_map.contains(device_id)) {
    vlan_cfg_table_map.remove(device_id);
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::DeleteVlanConfig(qint64 &project_id, ActVlanConfig &vlan_config) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteVlanConfig(project, vlan_config);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete vlan_config failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::DeleteVlanConfig(ActProject &project, ActVlanConfig &vlan_config) {
  ACT_STATUS_INIT();

  qint32 vlan_id = vlan_config.GetVlanId();

  for (qint64 device_id : vlan_config.GetDevices()) {
    ActDevice device;
    act_status = project.GetDeviceById(device, device_id);
    if (!IsActStatusSuccess(act_status)) {
      QString err_msg = QString("Device %1 in project").arg(device_id);
      return std::make_shared<ActStatusNotFound>(err_msg);
    }

    ActVlanTable vlan_table(device_id);
    act_status = this->GetVlanConfig(project, device_id, vlan_table);
    if (!IsActStatusSuccess(act_status)) {
      continue;
    }

    for (qint64 port : vlan_config.GetPorts()) {
      QSet<ActVlanStaticEntry> &vlan_static_entries = vlan_table.GetVlanStaticEntries();
      QSet<ActPortVlanEntry> &port_vlan_entries = vlan_table.GetPortVlanEntries();
      QSet<ActVlanPortTypeEntry> &vlan_port_type_entries = vlan_table.GetVlanPortTypeEntries();

      // vlan static entry
      ActVlanStaticEntry vlan_static_entry(vlan_id);
      QSet<ActVlanStaticEntry>::iterator vlan_static_entry_iter = vlan_static_entries.find(vlan_static_entry);
      if (vlan_static_entry_iter != vlan_static_entries.end()) {
        vlan_static_entry = *vlan_static_entry_iter;
        vlan_static_entries.erase(vlan_static_entry_iter);
      }
      vlan_static_entry.GetEgressPorts().remove(port);
      vlan_static_entry.GetUntaggedPorts().remove(port);
      if (!vlan_static_entry.GetEgressPorts().isEmpty()) {
        vlan_static_entries.insert(vlan_static_entry);
      }

      // vlan port type entry
      QSet<qint32> vlan_set;
      for (ActVlanStaticEntry vlan_static_entry : vlan_static_entries) {
        if (vlan_static_entry.GetEgressPorts().contains(port) ||
            device.GetDeviceProperty().GetReservedVlan().contains(vlan_static_entry.GetVlanId())) {
          vlan_set.insert(vlan_static_entry.GetVlanId());
        }
      }

      if (vlan_set.isEmpty()) {
        ActVlanStaticEntry vlan_static_entry(ACT_VLAN_INIT_PVID);
        vlan_static_entry_iter = vlan_static_entries.find(vlan_static_entry);
        if (vlan_static_entry_iter != vlan_static_entries.end()) {
          vlan_static_entry = *vlan_static_entry_iter;
          vlan_static_entries.erase(vlan_static_entry_iter);
        }
        vlan_static_entry.SetVlanPriority(ActVlanPriorityEnum::kNonTSN);
        vlan_static_entry.GetEgressPorts().insert(port);
        vlan_static_entry.GetUntaggedPorts().insert(port);
        vlan_static_entries.insert(vlan_static_entry);

        ActPortVlanEntry port_vlan_entry(port, ACT_VLAN_INIT_PVID, ActVlanPriorityEnum::kNonTSN);
        QSet<ActPortVlanEntry>::iterator port_vlan_entry_iter = port_vlan_entries.find(port_vlan_entry);
        if (port_vlan_entry_iter != port_vlan_entries.end()) {
          port_vlan_entries.erase(port_vlan_entry_iter);
        }
        port_vlan_entries.insert(port_vlan_entry);

        ActVlanPortTypeEntry vlan_port_type_entry(port, ActVlanPortTypeEnum::kAccess, ActVlanPriorityEnum::kNonTSN);
        QSet<ActVlanPortTypeEntry>::iterator vlan_port_type_entry_iter =
            vlan_port_type_entries.find(vlan_port_type_entry);
        if (vlan_port_type_entry_iter != vlan_port_type_entries.end()) {
          vlan_port_type_entries.erase(vlan_port_type_entry_iter);
        }
        vlan_port_type_entries.insert(vlan_port_type_entry);
      } else {
        ActPortVlanEntry port_vlan_entry(port, ACT_VLAN_INIT_PVID, ActVlanPriorityEnum::kNonTSN);
        QSet<ActPortVlanEntry>::iterator port_vlan_entry_iter = port_vlan_entries.find(port_vlan_entry);
        if (port_vlan_entry_iter != port_vlan_entries.end() && port_vlan_entry_iter->GetPVID() == vlan_id) {
          port_vlan_entries.erase(port_vlan_entry_iter);
          port_vlan_entries.insert(port_vlan_entry);

          ActVlanPortTypeEntry vlan_port_type_entry(port, ActVlanPortTypeEnum::kAccess, ActVlanPriorityEnum::kNonTSN);
          QSet<ActVlanPortTypeEntry>::iterator vlan_port_type_entry_iter =
              vlan_port_type_entries.find(vlan_port_type_entry);
          if (vlan_port_type_entry_iter != vlan_port_type_entries.end()) {
            vlan_port_type_entry = *vlan_port_type_entry_iter;
            vlan_port_type_entries.erase(vlan_port_type_entry_iter);
          }
          vlan_port_type_entries.insert(vlan_port_type_entry);

          ActVlanStaticEntry vlan_static_entry(ACT_VLAN_INIT_PVID);
          vlan_static_entry_iter = vlan_static_entries.find(vlan_static_entry);
          if (vlan_static_entry_iter != vlan_static_entries.end()) {
            vlan_static_entry = *vlan_static_entry_iter;
            vlan_static_entries.erase(vlan_static_entry_iter);
          }
          vlan_static_entry.SetVlanPriority(ActVlanPriorityEnum::kNonTSN);
          vlan_static_entry.GetEgressPorts().insert(port);
          if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kAccess) {
            vlan_static_entry.GetUntaggedPorts().insert(port);
          }
          vlan_static_entries.insert(vlan_static_entry);
        }
      }
    }

    act_status = this->UpdateVlanConfig(project, vlan_table);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetVlanConfig(qint64 &project_id, qint64 &device_id, ActVlanTable &vlan_config_table) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->GetVlanConfig(project, device_id, vlan_config_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get vlan config failed";
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetVlanConfig(ActProject &project, qint64 &device_id, ActVlanTable &vlan_config_table) {
  ACT_STATUS_INIT();

  // Found VlanConfig table
  auto vlan_cfg_table_map = project.GetDeviceConfig().GetVlanTables();
  if (!vlan_cfg_table_map.contains(device_id)) {  // not found
    return std::make_shared<ActStatusNotFound>(QString("VlanConfig(Device:%1)").arg(device_id));
  }

  vlan_config_table = vlan_cfg_table_map[device_id];

  return ACT_STATUS_SUCCESS;
}

static void InsertDefaultPVIDVlanToVlanConfig(const ActDevice &device, ActVlanTable &vlan_config_table) {
  ActVlanStaticEntry vlan_static_entry(ACT_VLAN_INIT_PVID);
  vlan_static_entry.SetVlanPriority(ActVlanPriorityEnum::kNonTSN);
  vlan_static_entry.SetName(QString("v%1").arg(ACT_VLAN_INIT_PVID));
  vlan_static_entry.SetTeMstid(false);
  vlan_static_entry.SetRowStatus(1);
  QSet<qint64> egress_ports;
  QSet<qint64> untagged_ports;

  // Create the VlanStaticEntry by VlanPortTypeEntry
  for (auto vlan_port_type_entry : vlan_config_table.GetVlanPortTypeEntries()) {
    if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kTrunk) {
      // Insert egress_ports(Member port)
      egress_ports.insert(vlan_port_type_entry.GetPortId());
    } else {  // Hybrid & Access
      // Insert egress_ports(Member port)
      egress_ports.insert(vlan_port_type_entry.GetPortId());
      // Insert untagged_ports
      untagged_ports.insert(vlan_port_type_entry.GetPortId());
    }
  }
  vlan_static_entry.SetEgressPorts(egress_ports);
  vlan_static_entry.SetUntaggedPorts(untagged_ports);

  // Insert to VlanStaticEntries
  auto vlan_static_entry_set = vlan_config_table.GetVlanStaticEntries();
  if (vlan_static_entry_set.contains(vlan_static_entry)) {
    vlan_static_entry_set.remove(vlan_static_entry);
  }
  vlan_static_entry_set.insert(vlan_static_entry);
  vlan_config_table.SetVlanStaticEntries(vlan_static_entry_set);
}

static void InsertDefaultPVIDToVlanConfig(const ActDevice &device, ActVlanTable &vlan_config_table) {
  auto port_vlan_entry_set = vlan_config_table.GetPortVlanEntries();
  for (auto interface : device.GetInterfaces()) {
    // Insert to port_vlan_entry_set
    ActPortVlanEntry entry(interface.GetInterfaceId(), ACT_VLAN_INIT_PVID, ActVlanPriorityEnum::kNonTSN);
    if (!port_vlan_entry_set.contains(entry)) {
      port_vlan_entry_set.insert(entry);
    }
  }
  vlan_config_table.SetPortVlanEntries(port_vlan_entry_set);
}

static void InsertDefaultVlanPortTypeToVlanConfig(const ActDevice &device, ActVlanTable &vlan_config_table) {
  auto vlan_port_type_entry_set = vlan_config_table.GetVlanPortTypeEntries();
  for (auto interface : device.GetInterfaces()) {
    // Insert to vlan_port_type_entry_set
    ActVlanPortTypeEntry entry(interface.GetInterfaceId(), ActVlanPortTypeEnum::kAccess, ActVlanPriorityEnum::kNonTSN);
    if (!vlan_port_type_entry_set.contains(entry)) {
      vlan_port_type_entry_set.insert(entry);
    }
  }
  vlan_config_table.SetVlanPortTypeEntries(vlan_port_type_entry_set);
}

ACT_STATUS ActCore::ComputeVlanConfig(ActProject &project) {
  ACT_STATUS_INIT();

  project.GetDeviceConfig().GetVlanTables().clear();

  for (ActIntelligentVlan intelligent_vlan : project.GetTopologySetting().GetIntelligentVlanGroup()) {
    QQueue<QList<qint64>> queue;
    QSet<qint64> visited;
    for (qint64 device_id : intelligent_vlan.GetEndStationList()) {
      queue.enqueue({device_id});
      visited.insert(device_id);
    }

    QSet<qint64> vlan_devices;
    while (!queue.isEmpty()) {
      QList<qint64> path = queue.dequeue();
      qint64 last_id = path.last();

      ActDevice device;
      project.GetDeviceById(device, last_id);

      QSet<qint64> neighbor_ids;
      for (ActInterface interface : device.GetInterfaces()) {
        if (!interface.GetUsed()) {
          continue;
        }
        ActLink link;
        project.GetLinkByInterfaceId(link, last_id, interface.GetInterfaceId());

        qint64 neighbor_id =
            (link.GetSourceDeviceId() == last_id) ? link.GetDestinationDeviceId() : link.GetSourceDeviceId();
        qint64 neighbor_intf_id = (link.GetSourceInterfaceId() == interface.GetInterfaceId())
                                      ? link.GetDestinationInterfaceId()
                                      : link.GetSourceInterfaceId();

        if (path.contains(neighbor_id)) {
          continue;
        }

        if (visited.contains(neighbor_id)) {
          for (qint64 device_id : path) {
            if (vlan_devices.contains(device_id)) {
              continue;
            }
            vlan_devices.insert(device_id);
          }
          continue;
        }

        ActDevice neighbor;
        project.GetDeviceById(neighbor, neighbor_id);

        if (neighbor.GetDeviceType() != ActDeviceTypeEnum::kSwitch &&
            neighbor.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation &&
            neighbor.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch) {
          continue;
        }

        QList<qint64> copy_path = path;
        copy_path.append(neighbor_id);
        queue.enqueue(copy_path);
        neighbor_ids.insert(neighbor_id);
      }
      visited.unite(neighbor_ids);
    }

    QMap<qint64, QSet<qint64>> vlan_devices_map;
    int end_station_count = 0;
    for (qint64 device_id : vlan_devices) {
      ActDevice device;
      project.GetDeviceById(device, device_id);

      if (device.GetDeviceType() == ActDeviceTypeEnum::kEndStation) {
        end_station_count++;
        continue;
      }

      for (ActInterface interface : device.GetInterfaces()) {
        if (!interface.GetUsed()) {
          continue;
        }

        ActLink link;
        qint64 interface_id = interface.GetInterfaceId();
        project.GetLinkByInterfaceId(link, device_id, interface_id);

        qint64 neighbor_id =
            (link.GetSourceDeviceId() == device_id) ? link.GetDestinationDeviceId() : link.GetSourceDeviceId();
        qint64 neighbor_intf_id = (link.GetSourceInterfaceId() == interface_id) ? link.GetDestinationInterfaceId()
                                                                                : link.GetSourceInterfaceId();

        if (!vlan_devices.contains(neighbor_id)) {
          continue;
        }

        vlan_devices_map[device_id].insert(interface_id);
      }
    }

    if (end_station_count < 2) {
      continue;
    }

    for (qint64 device_id : vlan_devices_map.keys()) {
      ActDevice device;
      project.GetDeviceById(device, device_id);
      ActVlanTable vlan_config_table(device_id);
      act_status = GetVlanConfig(project, device_id, vlan_config_table);
      if (!IsActStatusNotFound(act_status) && !IsActStatusSuccess(act_status)) {
        return act_status;
      }

      QSet<ActVlanStaticEntry> &vlan_static_entries = vlan_config_table.GetVlanStaticEntries();
      ActVlanStaticEntry vlan_static_entry(intelligent_vlan.GetVlanId());
      QSet<ActVlanStaticEntry>::iterator vlan_static_entries_iter = vlan_static_entries.find(vlan_static_entry);
      if (vlan_static_entries_iter != vlan_static_entries.end()) {
        vlan_static_entry = *vlan_static_entries_iter;
        vlan_static_entries.remove(vlan_static_entry);
      }
      vlan_static_entry.SetVlanPriority(ActVlanPriorityEnum::kNonTSN);
      vlan_static_entry.SetName(QString("v%1").arg(intelligent_vlan.GetVlanId()));
      vlan_static_entry.GetEgressPorts().unite(vlan_devices_map[device_id]);
      vlan_static_entries.insert(vlan_static_entry);

      QSet<ActVlanPortTypeEntry> &vlan_port_type_entries = vlan_config_table.GetVlanPortTypeEntries();
      for (qint64 egress_port_id : vlan_devices_map[device_id].values()) {
        ActVlanPortTypeEntry vlan_port_type_entry(egress_port_id, ActVlanPortTypeEnum::kHybrid,
                                                  ActVlanPriorityEnum::kNonTSN);
        QSet<ActVlanPortTypeEntry>::iterator vlan_port_type_entry_iter =
            vlan_port_type_entries.find(vlan_port_type_entry);
        if (vlan_port_type_entry_iter != vlan_port_type_entries.end()) {
          vlan_port_type_entries.erase(vlan_port_type_entry_iter);
        }
        vlan_port_type_entries.insert(vlan_port_type_entry);
      }

      // Insert  PVID(1)
      InsertDefaultPVIDToVlanConfig(device, vlan_config_table);

      // Insert Default VLAN port type
      InsertDefaultVlanPortTypeToVlanConfig(device, vlan_config_table);

      // Insert PVID VLAN
      InsertDefaultPVIDVlanToVlanConfig(device, vlan_config_table);

      act_status = UpdateVlanConfig(project, vlan_config_table);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
    }
  }
  return ACT_STATUS_SUCCESS;
}
}  // namespace core
}  // namespace act
