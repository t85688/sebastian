#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {
extern QMap<qint64, bool> g_monitor_link_status;  // <link_id, link_status>

ACT_STATUS ActCore::CheckRedundantSwift(ActProject &project, ActSwift &swift) {
  ACT_STATUS_INIT();

  const qint64 &root_device = swift.GetRootDevice();
  const qint64 &backup_root_device = swift.GetBackupRootDevice();

  ActDevice root;
  act_status = project.GetDeviceById(root, root_device);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("Swift - Cannot find root device");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActStatusFeasibilityCheckFailed>(error_msg);
  } else if (!root.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetSwift()) {
    QString error_msg = QString("Swift - device %1 doesn't support swift").arg(root.GetIpv4().GetIpAddress());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActStatusFeasibilityCheckFailed>(error_msg);
  }

  ActDevice backup_root;
  act_status = project.GetDeviceById(backup_root, backup_root_device);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("Swift - Cannot find backup root device");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActStatusFeasibilityCheckFailed>(error_msg);
  } else if (!backup_root.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetSwift()) {
    QString error_msg = QString("Swift - device %1 doesn't support swift").arg(backup_root.GetIpv4().GetIpAddress());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActStatusFeasibilityCheckFailed>(error_msg);
  }

  /* root & backup root links */
  QSet<qint64> links;
  for (const ActLink &link : project.GetLinks()) {
    if (this->GetSystemStatus() == ActSystemStatusEnum::kMonitoring && !g_monitor_link_status[link.GetId()]) {
      continue;
    }

    if (link.GetSourceDeviceId() == root_device && link.GetDestinationDeviceId() == backup_root_device) {
      links.insert(link.GetId());
    } else if (link.GetDestinationDeviceId() == root_device && link.GetSourceDeviceId() == backup_root_device) {
      links.insert(link.GetId());
    }
  }

  if (links.size() != 2) {
    QString error_msg = QString("Swift - root & backup root need to connect two links");
    // qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActStatusFeasibilityCheckFailed>(error_msg);
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateRedundantSwift(qint64 &project_id, ActSwift &swift) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateRedundantSwift(project, swift);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update swift failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, true);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateRedundantSwift(ActProject &project, ActSwift &swift) {
  ACT_STATUS_INIT();

  act_status = CheckRedundantSwift(project, swift);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  swift.SetActive(true);
  project.GetTopologySetting().GetRedundantGroup().SetSwift(swift);

  act_status = this->ComputeRedundantSwift(project);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  swift = project.GetTopologySetting().GetRedundantGroup().GetSwift();

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::DeleteRedundantSwift(qint64 &project_id) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteRedundantSwift(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete swift failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, true);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::DeleteRedundantSwift(ActProject &project) {
  ACT_STATUS_INIT();

  ActSwift swift;
  project.GetTopologySetting().GetRedundantGroup().SetSwift(swift);

  act_status = this->ComputeRedundantSwift(project);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::ComputeRedundantSwift(qint64 &project_id, ActSwift &swift) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->ComputeRedundantSwift(project, swift);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute swift failed";
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::ComputeRedundantSwift(ActProject &project, ActSwift &swift) {
  ACT_STATUS_INIT();

  if (!swift.GetActive()) {
    return ACT_STATUS_SUCCESS;
  }

  swift.GetDeviceTierMap().clear();
  swift.GetLinks().clear();

  act_status = CheckRedundantSwift(project, swift);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  QQueue<qint64> queue;
  QMap<qint16, QSet<qint64>> swift_map;
  QMap<qint64, qint16> tier_map;

  queue.enqueue(swift.GetRootDevice());
  tier_map[swift.GetRootDevice()] = 0;

  queue.enqueue(swift.GetBackupRootDevice());
  tier_map[swift.GetBackupRootDevice()] = 0;

  // start from root & backup root device
  while (!queue.isEmpty()) {
    qint64 device_id = queue.dequeue();
    qint16 tier = tier_map[device_id];

    ActDevice device;
    project.GetDeviceById(device, device_id);

    // skip device over 2 tiers from root & backup root device
    if (device.GetDeviceType() != ActDeviceTypeEnum::kEndStation &&
        (tier > 2 || !device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetSwift())) {
      continue;
    }

    QMap<qint16, QSet<qint64>> neighbor_map;       // collect neighbor device by tier
    QMap<qint16, QSet<qint64>> neighbor_link_map;  // collect neighbor link by tier
    for (ActInterface &interface : device.GetInterfaces()) {
      if (!interface.GetUsed()) {
        continue;
      }
      ActLink link;
      project.GetLinkByInterfaceId(link, device_id, interface.GetInterfaceId());

      if (this->GetSystemStatus() == ActSystemStatusEnum::kMonitoring && !g_monitor_link_status[link.GetId()]) {
        continue;
      }

      qint64 neighbor_id =
          (link.GetSourceDeviceId() == device_id) ? link.GetDestinationDeviceId() : link.GetSourceDeviceId();

      if (!tier_map.contains(neighbor_id)) {
        tier_map[neighbor_id] = tier + 1;
        neighbor_map[tier + 1].insert(neighbor_id);
        neighbor_link_map[tier + 1].insert(link.GetId());
      } else if (tier > 0 && swift_map[tier - 1].contains(neighbor_id)) {
        neighbor_map[tier - 1].insert(neighbor_id);
        neighbor_link_map[tier - 1].insert(link.GetId());
      } else if (tier_map[neighbor_id] == tier) {
        neighbor_map[tier].insert(neighbor_id);
        neighbor_link_map[tier].insert(link.GetId());
      }
    }

    if (tier == 0 || device.GetDeviceType() == ActDeviceTypeEnum::kEndStation ||
        (neighbor_map[tier - 1].size() == 2 && neighbor_link_map[tier - 1].size() == 2 &&
         neighbor_map[tier].size() == 0)) {
      swift_map[tier].insert(device_id);
      for (qint64 neighbor_id : neighbor_map[tier + 1]) {
        queue.enqueue(neighbor_id);
      }
    }
  }

  for (qint16 tier : swift_map.keys()) {
    for (qint64 device_id : swift_map[tier]) {
      swift.GetDeviceTierMap()[device_id] = tier;
    }
  }

  QSet<qint64> links;
  for (const ActLink &link : project.GetLinks()) {
    if (swift.GetDeviceTierMap().contains(link.GetSourceDeviceId()) &&
        swift.GetDeviceTierMap().contains(link.GetDestinationDeviceId())) {
      links.insert(link.GetId());
    }
  }
  swift.SetLinks(links);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::ComputeRedundantSwiftCandidate(qint64 &project_id, ActSwiftCandidates &swift_candidates) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->ComputeRedundantSwiftCandidate(project, swift_candidates);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute swift failed";
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::ComputeRedundantSwiftCandidate(ActProject &project, ActSwiftCandidates &swift_candidates) {
  ACT_STATUS_INIT();
  QSet<ActSwiftCandidate> candidates;
  QMap<qint64, QMap<qint64, QSet<qint64>>> map;

  for (auto link : project.GetLinks()) {
    map[link.GetSourceDeviceId()][link.GetDestinationDeviceId()].insert(link.GetId());
    map[link.GetDestinationDeviceId()][link.GetSourceDeviceId()].insert(link.GetId());
  }

  for (qint64 root_device : map.keys()) {
    for (qint64 backup_root_device : map[root_device].keys()) {
      if (map[root_device][backup_root_device].size() == 2) {
        ActDevice root, backup_root;
        project.GetDeviceById(root, root_device);
        project.GetDeviceById(backup_root, backup_root_device);

        if (!root.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetSwift() ||
            !backup_root.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetSwift()) {
          continue;
        }

        ActSwiftCandidate candidate(root_device, backup_root_device);
        if (!candidates.contains(candidate)) {
          candidates.insert(candidate);
        }
      }
    }
  }

  swift_candidates.SetCandidates(candidates);

  return ACT_STATUS_SUCCESS;
}

static void ConfigRedundantSwift(ActProject &project, ActSwift &swift) {
  if (!swift.GetActive()) {
    return;
  }

  QSet<qint64> second_tiers;
  for (qint64 link_id : swift.GetLinks()) {
    ActLink link;
    project.GetLinkById(link, link_id);

    if (link.GetSourceDeviceId() == swift.GetRootDevice() &&
        link.GetDestinationDeviceId() != swift.GetBackupRootDevice()) {
      second_tiers.insert(link.GetDestinationDeviceId());
    } else if (link.GetDestinationDeviceId() == swift.GetRootDevice() &&
               link.GetSourceDeviceId() != swift.GetBackupRootDevice()) {
      second_tiers.insert(link.GetSourceDeviceId());
    }
  }

  QSet<ActDevice> devices;
  QMap<qint64, ActRstpTable> &rstp_tables = project.GetDeviceConfig().GetRstpTables();
  for (ActDevice device : project.GetDevices()) {
    qint64 &device_id = device.GetId();
    ActSTPRSTPItem &stp_rstp = device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP();
    ActDeviceRoleEnum device_role =
        !stp_rstp.GetRSTP() ? ActDeviceRoleEnum::kUnknown
        : (!swift.GetActive() || !stp_rstp.GetSwift() || !swift.GetDeviceTierMap().contains(device_id))
            ? ActDeviceRoleEnum::kRSTP
        : swift.GetRootDevice() == device_id       ? ActDeviceRoleEnum::kSwiftRoot
        : swift.GetBackupRootDevice() == device_id ? ActDeviceRoleEnum::kSwiftBackupRoot
                                                   : ActDeviceRoleEnum::kSwift;
    device.SetDeviceRole(device_role);
    devices.insert(device);

    if (!stp_rstp.GetRSTP()) {
      rstp_tables.remove(device_id);
      continue;
    }

    ActRstpTable rstp_table(device);
    if (rstp_tables.contains(device_id)) {
      rstp_table = rstp_tables[device_id];
    }
    bool rstp_config_swift = (stp_rstp.GetSwift() && swift.GetActive() &&
                              (swift.GetRootDevice() == device_id || swift.GetBackupRootDevice() == device_id ||
                               second_tiers.contains(device_id)))
                                 ? true
                                 : false;
    bool rstp_config_revert = (stp_rstp.GetSwift() && swift.GetActive() &&
                               (swift.GetRootDevice() == device_id || swift.GetBackupRootDevice() == device_id))
                                  ? true
                                  : false;
    qint64 priority = (!stp_rstp.GetSwift() || !stp_rstp.GetRSTP())                                      ? 32768
                      : (swift.GetRootDevice() == device_id || swift.GetBackupRootDevice() == device_id) ? 4096
                      : second_tiers.contains(device_id)                                                 ? 8192
                                                                                                         : 32768;
    rstp_table.SetActive(stp_rstp.GetRSTP());
    rstp_table.SetRstpConfigSwift(rstp_config_swift);
    rstp_table.SetRstpConfigRevert(rstp_config_revert);
    rstp_table.SetPriority(priority);
    rstp_table.SetHelloTime(1);
    rstp_tables.insert(device_id, rstp_table);
  }

  project.SetDevices(devices);
}

ACT_STATUS ActCore::ComputeRedundantSwift(qint64 &project_id) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->ComputeRedundantSwift(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute swift failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, true);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::ComputeRedundantSwift(ActProject &project) {
  ACT_STATUS_INIT();

  ActSwift &swift = project.GetTopologySetting().GetRedundantGroup().GetSwift();

  act_status = this->ComputeRedundantSwift(project, swift);
  if (!IsActStatusSuccess(act_status)) {
    this->DeleteRedundantSwift(project);
  }

  ConfigRedundantSwift(project, swift);

  return ACT_STATUS_SUCCESS;
}
}  // namespace core
}  // namespace act
