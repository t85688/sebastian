/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#include "act_core.hpp"

namespace act {
namespace core {

// Helper functions for Group management
static ActGroup *FindGroupByName(QMap<qint64, ActGroup> &groups, const QString &name) {
  for (auto &group : groups) {
    if (group.GetName() == name) {
      return &group;
    }
  }
  return nullptr;
}

ACT_STATUS ActCore::CheckGroup(ActProject &project, ActGroup &group) {
  ACT_STATUS_INIT();

  if (group.GetName().isEmpty()) {
    QString error_msg = "Group name cannot be empty";
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check the length of the group name, Length: 1-63 chars
  if (group.GetName().length() < 1 || group.GetName().length() > 63) {
    QString error_msg = QString("Group name length must be between 1 and 63 characters");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check duplicate name
  QMap<qint64, ActGroup> &groups = project.GetTopologySetting().GetGroups();
  ActGroup *existing = FindGroupByName(groups, group.GetName());
  if (existing && existing->GetId() != group.GetId()) {
    QString error_msg = QString("Group name %1 already exists").arg(group.GetName());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check parent existence if specified
  if (group.GetParentId() != -1) {
    if (!groups.contains(group.GetParentId())) {
      QString error_msg = QString("Parent group id %1 not found").arg(group.GetParentId());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  return act_status;
}

ACT_STATUS ActCore::CreateGroup(const qint64 project_id, ActGroup &group, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->CreateGroup(project, group);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Create group failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::CreateGroup(ActProject &project, ActGroup &group) {
  ACT_STATUS_INIT();

  act_status = this->CheckGroup(project, group);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check group failed";
    return act_status;
  }

  // Generate ID if not set
  QMap<qint64, ActGroup> &groups = project.GetTopologySetting().GetGroups();
  if (group.GetId() == -1) {
    qint64 new_id = 1;
    while (1) {
      qint64 current_timestamp = QDateTime::currentSecsSinceEpoch();
      // Check duplicated
      if (!groups.contains(current_timestamp)) {
        // Not duplicated
        new_id = current_timestamp;
        break;
      }
    }

    group.SetId(new_id);
  }

  // Insert Group
  groups.insert(group.GetId(), group);

  return act_status;
}

ACT_STATUS ActCore::UpdateGroup(const qint64 project_id, ActGroup &group, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateGroup(project, group);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update group failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateGroup(ActProject &project, ActGroup &group) {
  ACT_STATUS_INIT();

  act_status = this->CheckGroup(project, group);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check group failed";
    return act_status;
  }

  QMap<qint64, ActGroup> &groups = project.GetTopologySetting().GetGroups();

  // Find existing group
  if (!groups.contains(group.GetId())) {
    QString error_msg = QString("Group id %1 not found for update").arg(group.GetId());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Update group details
  groups[group.GetId()] = group;

  return act_status;
}

ACT_STATUS ActCore::DeleteGroup(const qint64 project_id, const qint64 group_id, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteGroup(project, group_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete group failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

/**
 * @brief Delete a group object and its devices
 *
 * @param project
 * @param group_id
 * @return ACT_STATUS
 */
ACT_STATUS ActCore::DeleteGroup(ActProject &project, const qint64 group_id) {
  ACT_STATUS_INIT();

  QMap<qint64, ActGroup> &groups = project.GetTopologySetting().GetGroups();

  if (!groups.contains(group_id)) {
    QString error_msg = QString("Delete group failed, cannot found group id %1").arg(group_id);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Find all descendants to delete (Cascading delete)
  QSet<qint64> groups_to_delete;
  groups_to_delete.insert(group_id);

  QList<qint64> queue;
  queue.append(group_id);

  int head = 0;
  while (head < queue.size()) {
    qint64 current_parent = queue[head++];
    // scan for children
    for (auto it = groups.begin(); it != groups.end(); ++it) {
      if (it.value().GetParentId() == current_parent) {
        if (!groups_to_delete.contains(it.key())) {
          groups_to_delete.insert(it.key());
          queue.append(it.key());
        }
      }
    }
  }

  // Collect all devices in these groups
  QList<qint64> devices_to_delete;
  for (qint64 id : groups_to_delete) {
    if (groups.contains(id)) {
      devices_to_delete.append(groups[id].GetDeviceIds().toList());
    }
  }

  // Delete devices
  if (!devices_to_delete.isEmpty()) {
    act_status = act::core::g_core.DeleteDevices(project, devices_to_delete, false);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Delete devices in group failed";
      return act_status;
    }
  }

  for (qint64 id : groups_to_delete) {
    groups.remove(id);
  }

  return act_status;
}

ACT_STATUS ActCore::UnGroup(const qint64 project_id, const qint64 group_id, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UnGroup(project, group_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Ungroup failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::UnGroup(ActProject &project, const qint64 group_id) {
  ACT_STATUS_INIT();

  QMap<qint64, ActGroup> &groups = project.GetTopologySetting().GetGroups();
  QSet<ActDevice> &device_set = project.GetDevices();

  if (!groups.contains(group_id)) {
    QString error_msg = QString("Ungroup failed, cannot found group id %1").arg(group_id);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  ActGroup target_group = groups[group_id];
  qint64 parent_id = target_group.GetParentId();
  QSet<qint64> devices_in_group = target_group.GetDeviceIds();

  // 1. Move devices to parent group
  for (qint64 dev_id : devices_in_group) {
    ActDevice device;
    // Find device in QSet
    auto it = device_set.find(ActDevice(dev_id));
    if (it != device_set.end()) {
      device = *it;
      device_set.erase(it);  // Remove old

      // Update Group ID
      device.SetGroupId(parent_id);

      device_set.insert(device);  // Insert new

      // If parent group exists, add device ID to parent group's list
      if (parent_id != -1) {
        if (groups.contains(parent_id)) {
          groups[parent_id].GetDeviceIds().insert(dev_id);
        } else {
          QString error_msg = QString("Ungroup failed, cannot found parent group id %1").arg(parent_id);
          qCritical() << error_msg.toStdString().c_str();
          return std::make_shared<ActBadRequest>(error_msg);
        }
      }
    } else {
      QString error_msg = QString("Ungroup failed, cannot found device id %1 in project").arg(dev_id);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // 2. Move sub-groups to parent level (Update parent_id of children)
  bool subGroupMoved = false;
  for (auto &g : groups) {
    if (g.GetParentId() == group_id) {
      g.SetParentId(parent_id);
      subGroupMoved = true;
    }
  }

  // 3. Delete the target group
  groups.remove(group_id);

  return act_status;
}

}  // namespace core
}  // namespace act