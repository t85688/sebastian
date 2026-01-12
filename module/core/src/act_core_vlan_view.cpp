#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::GetVlanViewIds(qint64 &project_id, ActVlanViewIds &vlan_view_ids, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Get vlan id
  QSet<qint32> vlan_id_set;
  for (auto vlan_cfg_table : project.GetDeviceConfig().GetVlanTables()) {
    // For each vlan_entries
    for (auto vlan_static_entry : vlan_cfg_table.GetVlanStaticEntries()) {
      // Insert vlan_id to set
      vlan_id_set.insert(vlan_static_entry.GetVlanId());
    }
  }

  // Transfer QSet to QList
  QList<qint32> vlan_id_list = vlan_id_set.values();

  // Sort QList
  std::sort(vlan_id_list.begin(), vlan_id_list.end());

  vlan_view_ids.SetVlanIdList(vlan_id_list);

  return act_status;
}

ACT_STATUS ActCore::GetVlanViews(qint64 &project_id, ActVlanViewTable &vlan_view_table, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->GetVlanViews(project, vlan_view_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get vlan views failed";
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetVlanViews(ActProject &project, ActVlanViewTable &vlan_view_table) {
  ACT_STATUS_INIT();

  // Transfer vlan-config to vlan_view_table
  QSet<ActVlanView> vlan_view_set;
  for (auto vlan_cfg_table : project.GetDeviceConfig().GetVlanTables()) {
    // For each vlan_entries
    for (auto vlan_static_entry : vlan_cfg_table.GetVlanStaticEntries()) {
      // Found vlan_view
      ActVlanView vlan_view(vlan_static_entry.GetVlanId());
      auto vlan_view_iter = vlan_view_set.find(vlan_view);
      if (vlan_view_iter != vlan_view_set.end()) {  // found
        vlan_view = *vlan_view_iter;
      }

      // Found vlan_view_device
      ActVlanViewDevice vlan_view_device(vlan_cfg_table.GetDeviceId());
      auto vlan_view_device_set = vlan_view.GetDevices();
      auto vlan_view_device_iter = vlan_view_device_set.find(vlan_view_device);
      if (vlan_view_device_iter != vlan_view_device_set.end()) {  // found
        vlan_view_device = *vlan_view_device_iter;
      }
      auto port_set = vlan_view_device.GetPorts();
      // Insert egress_ports to device's ports
      for (auto egress_port : vlan_static_entry.GetEgressPorts()) {
        // Find egress_port's vlan_port_type entry
        ActVlanPortTypeEntry vlan_port_type_entry(egress_port, ActVlanPortTypeEnum::kHybrid,
                                                  ActVlanPriorityEnum::kNonTSN);
        auto vlan_port_type_entry_iter = vlan_cfg_table.GetVlanPortTypeEntries().find(vlan_port_type_entry);
        if (vlan_port_type_entry_iter != vlan_cfg_table.GetVlanPortTypeEntries().end()) {  //  found
          vlan_port_type_entry = *vlan_port_type_entry_iter;
        }

        port_set.insert(vlan_port_type_entry);
      }

      // Insert untagged_ports to device's ports
      for (auto untag_port : vlan_static_entry.GetUntaggedPorts()) {
        // Find untag_port's vlan_port_type entry
        ActVlanPortTypeEntry vlan_port_type_entry(untag_port, ActVlanPortTypeEnum::kHybrid,
                                                  ActVlanPriorityEnum::kNonTSN);
        auto vlan_port_type_entry_iter = vlan_cfg_table.GetVlanPortTypeEntries().find(vlan_port_type_entry);
        if (vlan_port_type_entry_iter != vlan_cfg_table.GetVlanPortTypeEntries().end()) {  //  found
          vlan_port_type_entry = *vlan_port_type_entry_iter;
        }

        port_set.insert(vlan_port_type_entry);
      }

      // Generate vlan_view
      vlan_view_device.SetPorts(port_set);
      if (vlan_view_device_iter != vlan_view_device_set.end()) {  // remove exists vlan_view_device
        vlan_view_device_set.erase(vlan_view_device_iter);
      }
      vlan_view_device_set.insert(vlan_view_device);
      vlan_view.SetDevices(vlan_view_device_set);

      // Insert vlan_view
      if (vlan_view_iter != vlan_view_set.end()) {  // remove exists vlan_view
        vlan_view_set.erase(vlan_view_iter);
      }
      vlan_view_set.insert(vlan_view);
    }
  }

  vlan_view_table.SetVlanViews(vlan_view_set);
  // qDebug() << __func__ << "vlan_view_table:" << vlan_view_table.ToString().toStdString().c_str();

  return act_status;
}

ACT_STATUS ActCore::GetVlanView(qint64 &project_id, qint32 &vlan_id, ActVlanView &vlan_view, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->GetVlanView(project, vlan_id, vlan_view);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get vlan view failed";
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetVlanView(ActProject &project, qint32 &vlan_id, ActVlanView &vlan_view) {
  ACT_STATUS_INIT();

  ActVlanViewTable vlan_view_table;
  act_status = GetVlanViews(project, vlan_view_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "GetVlanViews() failed with project id:" << project.GetId();
    return act_status;
  }

  auto vlan_view_set = vlan_view_table.GetVlanViews();
  auto vlan_view_iter = vlan_view_set.find(ActVlanView(vlan_id));
  if (vlan_view_iter == vlan_view_set.end()) {  // not found
    return std::make_shared<ActStatusNotFound>(QString("Vlan-view(VlanID:%1)").arg(vlan_id));
  }
  vlan_view = *vlan_view_iter;
  return act_status;
}
}  // namespace core
}  // namespace act
