#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::CheckRedundantRSTP(const ActProject &project, const ActRSTP &check_rstp) {
  QSet<ActRSTP> rstp_set = project.GetTopologySetting().GetRedundantGroup().GetRSTP();
  ACT_STATUS_INIT();

  // Check devices don't contain in another RSTP group
  for (ActRSTP rstp : rstp_set) {
    if (check_rstp.GetDevices().intersects(rstp.GetDevices())) {
      return std::make_shared<ActBadRequest>(
          QString("RSTP group devices has at least one in common with another group."));
    }
  }

  // Check root contains in devices
  if (!check_rstp.GetDevices().contains(check_rstp.GetRootDevice())) {
    return std::make_shared<ActStatusNotFound>(QString("Root device %1 in RSTP group").arg(check_rstp.GetRootDevice()));
  }

  // Backup root device is optional
  if (check_rstp.GetBackupRootDevice() == -1) {
    return ACT_STATUS_SUCCESS;
  }

  // Check backup root contains in devices
  if (!check_rstp.GetDevices().contains(check_rstp.GetBackupRootDevice())) {
    return std::make_shared<ActStatusNotFound>(
        QString("Backup root device %1 in RSTP group").arg(check_rstp.GetBackupRootDevice()));
  }

  // Check root & backup root device is not the same
  if (check_rstp.GetRootDevice() == check_rstp.GetBackupRootDevice()) {
    return std::make_shared<ActBadRequest>(QString("Backup root device cannot be duplicated with root device."));
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::CreateRedundantRSTP(qint64 &project_id, ActRSTP &rstp) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->CreateRedundantRSTP(project, rstp);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Create RSTP failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, true);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::CreateRedundantRSTP(ActProject &project, ActRSTP &rstp) {
  ACT_STATUS_INIT();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support System RSTP Setting";
    qCritical() << __func__ << error_msg;
    return std::make_shared<ActLicenseNotActiveFailed>("System RSTP Setting");
  } */

  QSet<ActRSTP> &rstp_set = project.GetTopologySetting().GetRedundantGroup().GetRSTP();

  // Check RSTP configuration
  act_status = this->CheckRedundantRSTP(project, rstp);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Check redundant RSTP failed with project id:" << project.GetId();
    return act_status;
  }

  // Generate a new unique id
  qint64 id;
  act_status = this->GenerateUniqueId<ActRSTP>(rstp_set, project.last_assigned_rstp_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot get an available unique id";
    return act_status;
  }
  rstp.SetId(id);
  rstp_set.insert(rstp);

  // update redundant group
  this->ComputeRedundantRSTP(project);

  return act_status;
}

ACT_STATUS ActCore::UpdateRedundantRSTP(qint64 &project_id, ActRSTP &rstp) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateRedundantRSTP(project, rstp);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update RSTP failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, true);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateRedundantRSTP(ActProject &project, ActRSTP &rstp) {
  ACT_STATUS_INIT();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support System RSTP Setting";
    qCritical() << __func__ << error_msg;
    return std::make_shared<ActLicenseNotActiveFailed>("System RSTP Setting");
  } */

  QSet<ActRSTP> &rstp_set = project.GetTopologySetting().GetRedundantGroup().GetRSTP();

  // Check RSTP configuration
  act_status = this->CheckRedundantRSTP(project, rstp);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Check redundant RSTP failed with project id:" << project.GetId();
    return act_status;
  }

  // Check the item does exist by id
  typename QSet<ActRSTP>::const_iterator iterator;
  iterator = rstp_set.find(rstp);
  if (iterator == rstp_set.end()) {
    qCritical() << QString("Cannot found rstp %1").arg(rstp.ToString()).toStdString().c_str();
    return ACT_STATUS_SUCCESS;
  }
  rstp_set.erase(iterator);
  rstp_set.insert(rstp);

  return act_status;
}

ACT_STATUS ActCore::DeleteRedundantRSTP(qint64 &project_id, qint64 &rstp_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteRedundantRSTP(project, rstp_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete rstp failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, true);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::DeleteRedundantRSTP(ActProject &project, qint64 &rstp_id) {
  ACT_STATUS_INIT();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support System RSTP Setting";
    qCritical() << __func__ << error_msg;
    return std::make_shared<ActLicenseNotActiveFailed>("System RSTP Setting");
  } */

  QSet<ActRSTP> &rstp_set = project.GetTopologySetting().GetRedundantGroup().GetRSTP();

  // Check the item does exist by id
  typename QSet<ActRSTP>::const_iterator iterator;
  iterator = rstp_set.find(ActRSTP(rstp_id));
  if (iterator == rstp_set.end()) {
    qCritical() << QString("Cannot found rstp id %1").arg(rstp_id).toStdString().c_str();
    return ACT_STATUS_SUCCESS;
  }
  rstp_set.erase(iterator);

  // update redundant group
  this->ComputeRedundantRSTP(project);

  return ACT_STATUS_SUCCESS;
}

void ActCore::ComputeRedundantRSTP(ActProject &project) {
  QSet<ActDevice> device_set;
  const QSet<ActRSTP> &rstp_groups = project.GetTopologySetting().GetRedundantGroup().GetRSTP();
  QMap<qint64, ActRstpTable> &rstp_tables = project.GetDeviceConfig().GetRstpTables();
  rstp_tables.clear();

  for (const ActRSTP &rstp : rstp_groups) {
    // Compute device tier
    QMap<qint16, QQueue<ActDevice>> BFS_queue;
    qint16 tier = 0;

    qint64 root_id = rstp.GetRootDevice();
    ActDevice root;
    project.GetDeviceById(root, root_id);
    root.SetTier(tier);
    root.SetDeviceRole(ActDeviceRoleEnum::kRSTPRoot);
    BFS_queue[tier].enqueue(root);

    if (rstp.GetBackupRootDevice() != -1) {
      qint64 backup_root_id = rstp.GetBackupRootDevice();
      ActDevice backup_root;
      project.GetDeviceById(backup_root, backup_root_id);
      backup_root.SetTier(0);
      backup_root.SetDeviceRole(ActDeviceRoleEnum::kRSTPBackupRoot);
      BFS_queue[tier].enqueue(backup_root);
    }

    while (!BFS_queue[tier].isEmpty()) {
      ActDevice device = BFS_queue[tier].dequeue();

      QSet<qint64> root_guards;
      for (ActInterface &interface : device.GetInterfaces()) {
        ActLink link;
        if (IsActStatusNotFound(project.GetLinkByInterfaceId(link, device.GetId(), interface.GetInterfaceId()))) {
          interface.SetRootGuard(false);
          continue;
        }
        ActDevice neighbor;
        qint64 neighbor_id =
            (link.GetSourceDeviceId() == device.GetId()) ? link.GetDestinationDeviceId() : link.GetSourceDeviceId();
        project.GetDeviceById(neighbor, neighbor_id);

        if (!rstp.GetDevices().contains(neighbor_id) || device_set.contains(neighbor) ||
            BFS_queue[tier].contains(neighbor)) {
          interface.SetRootGuard(false);
          continue;
        }

        if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetRootGuard()) {
          interface.SetRootGuard(true);
          root_guards.insert(interface.GetInterfaceId());
        } else {
          interface.SetRootGuard(false);
        }

        if (!BFS_queue[tier + 1].contains(neighbor)) {
          neighbor.SetTier(tier + 1);
          neighbor.SetDeviceRole(ActDeviceRoleEnum::kRSTP);
          BFS_queue[tier + 1].enqueue(neighbor);
        }
      }
      device_set.insert(device);

      ActRstpTable rstp_table(device.GetId());
      rstp_table.SetActive(true);
      rstp_table.SetHelloTime(rstp.GetHelloTime());
      if (device.GetDeviceRole() == ActDeviceRoleEnum::kRSTPRoot) {
        rstp_table.SetPriority(4096);
      } else if (device.GetDeviceRole() == ActDeviceRoleEnum::kRSTPBackupRoot) {
        rstp_table.SetPriority(8192);
      } else {
        rstp_table.SetPriority(32768);
      }
      // rstp_table.SetRootGuards(root_guards);
      rstp_tables.insert(device.GetId(), rstp_table);

      if (BFS_queue[tier].isEmpty()) {
        tier++;
      }
    }
  }

  for (ActDevice device : project.GetDevices()) {
    if (device_set.contains(device)) {
      continue;
    }
    device.SetTier(0);
    device.SetDeviceRole(ActDeviceRoleEnum::kUnknown);
    for (auto &interface : device.GetInterfaces()) {
      interface.SetRootGuard(false);
    }
    device_set.insert(device);

    if (device.GetDeviceType() == ActDeviceTypeEnum::kEndStation ||
        device.GetDeviceType() == ActDeviceTypeEnum::kICMP || device.GetDeviceType() == ActDeviceTypeEnum::kMoxa ||
        device.GetDeviceType() == ActDeviceTypeEnum::kUnknown) {
      continue;
    }
    ActRstpTable rstp_table(device.GetId());
    rstp_tables.insert(device.GetId(), rstp_table);
  }
  project.SetDevices(device_set);

  return;
}
}  // namespace core
}  // namespace act
