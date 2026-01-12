#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::CheckProject(ActProject &project, bool check_feasibility) {
  ACT_STATUS_INIT();

  // Check project name is valid
  // Check if contains special characters (\ / : * ? “ < > |) that can not use in windows
  const QString project_name = project.GetProjectName();
  const QString forbidden_char = "\\/:*?\"<>|";
  for (const QChar &c : forbidden_char) {
    if (project_name.contains(c)) {
      QString error_msg = QString("Project name contains forbidden characters, the name is %1").arg(project_name);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Check if trailing space and multiple consecutive spaces
  if (project_name.endsWith(" ") || project_name.contains(QRegExp("\\s{2,}"))) {
    QString error_msg = QString("Project name contains multiple consecutive spaces or end with space, the name is %1")
                            .arg(project_name);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check project setting
  // Doesn't need to check the account setting
  act_status = this->CheckProjectSetting(project.GetProjectSetting(), false);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check project setting failed";
    return act_status;
  }

  ActProjectSetting proj_setting = project.GetProjectSetting();
  ActTrafficTypeToPriorityCodePointMapping traffic_type_to_pcp_mapping =
      proj_setting.GetTrafficTypeToPriorityCodePointMapping();
  QSet<quint8> time_sync_pcp = traffic_type_to_pcp_mapping.GetTimeSync();

  // Check Device
  for (ActDevice device : project.GetDevices()) {
    act_status = this->CheckDevice(project, device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Check device failed";
      return act_status;
    }

    if (device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      continue;
    }

    // [bugfix:2311] Check traffic type to PCP mapping is consistent with device
    quint8 ptp_queue_id = device.GetDeviceProperty().GetPtpQueueId();
    if (!time_sync_pcp.contains(ptp_queue_id)) {
      QString log_msg =
          QString("Project (%1) - The time sync PCP %1 in the device %2 is not consistent with project setting")
              .arg(project.GetProjectName())
              .arg(QString::number(ptp_queue_id))
              .arg(device.GetIpv4().GetIpAddress());
      qCritical() << log_msg;
      return std::make_shared<ActStatusTimeSyncPcpNotConsistentWithDevice>(ptp_queue_id,
                                                                           device.GetIpv4().GetIpAddress());
    }
  }

  // Check Link
  for (ActLink link : project.GetLinks()) {
    act_status = this->CheckLink(project, link);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Check link failed";
      return act_status;
    }
  }

  // Check Stream
  QSet<ActStream> new_stream_set;
  for (ActStream stream : project.GetStreams()) {
    act_status = this->CheckStream(project, stream, check_feasibility);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Check stream failed";
      return act_status;
    }
    new_stream_set.insert(stream);
  }
  project.SetStreams(new_stream_set);

  return act_status;
}

ACT_STATUS ActCore::SaveProject(qint64 &project_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;

    return act_status;
  }

  act_status = this->SaveProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Save project failed";

    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::SaveProject(ActProject &project) {
  ACT_STATUS_INIT();

  // Write to db
  return act::database::project::WriteData(project);
}

ACT_STATUS ActCore::ImportProject(ActImportProject &imported_cfg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Fetch the data from imported file
  ActProject &imported_project = imported_cfg.GetProject();
  QSet<ActDeviceProfile> imported_device_profile_set = imported_cfg.GetDeviceProfiles();
  QSet<ActExportDeviceProfileInfo> imported_device_profile_infos = imported_cfg.GetDeviceProfileInfos();
  bool overwrite = imported_cfg.GetOverwrite();

  // [bugfix:2514] AutoScan can not identify device
  // DecryptPassword
  imported_project.DecryptPassword();

  QSet<ActDevice> imported_device_set = imported_project.GetDevices();
  QSet<ActDevice> new_device_set;

  QSet<ActDeviceProfile> sys_device_profile_set = this->GetDeviceProfileSet();
  QSet<ActProject> sys_project_set = this->GetProjectSet();

  for (ActProject sys_project : sys_project_set) {
    // Check not another Manufacture project
    if (imported_project.GetProjectMode() == ActProjectModeEnum::kManufacture) {
      if (sys_project.GetProjectMode() == ActProjectModeEnum::kManufacture) {
        QString error_msg = QString("Manufacture stage only supports one project");
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }

    if (sys_project.GetProjectMode() != imported_project.GetProjectMode() ||
        sys_project.GetUUID() != imported_project.GetUUID()) {
      continue;
    }

    if (overwrite) {
      imported_project.SetId(sys_project.GetId());
      sys_project_set.remove(sys_project);
    } else {
      imported_project.SetUUID(QUuid::createUuid().toString(QUuid::WithoutBraces));
    }
    break;
  }

  // Add license control
  /* quint16 lic_project_size = this->GetLicense().GetSize().GetProjectSize();
  if ((sys_project_set.size() >= lic_project_size) &&
      (lic_project_size != 0)) {  // [feat:2852] License - project size = 0 means unlimited
    QString error_msg = QString("The project size exceeds the limit: %1").arg(lic_project_size);
    qCritical() << error_msg.toStdString().c_str();

    return std::make_shared<ActLicenseSizeFailedRequest>("Project Size", lic_project_size);
  } */

  // [feat:3889] General Profile cannot be imported into other Stages
  // If the license does not support the profile, reject it
  // Add self-panning case for old project
  /* if (imported_project.GetProfile() != ActServiceProfileForLicenseEnum::kSelfPlanning &&
      !this->GetLicense().GetProfiles().contains(imported_project.GetProfile())) {
    QString error_msg = QString("The project profile %1 is not supported by the license")
                            .arg(GetStringFromEnum<ActServiceProfileForLicenseEnum>(
                                imported_project.GetProfile(), kActServiceProfileForLicenseEnumMap));
    qCritical() << error_msg.toStdString().c_str();

    return std::make_shared<ActLicenseNotSupport>(GetStringFromEnum<ActServiceProfileForLicenseEnum>(
        imported_project.GetProfile(), kActServiceProfileForLicenseEnumMap));
  } */

  act_status = this->CheckProject(imported_project, false);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check project failed";

    return act_status;
  }

  // Check the project does not exist with duplicated name
  // [bugfix:3190] Project should not be created when its name is over 64 digits
  QRegularExpression re("\\((?<idx>\\d+)\\)$");
  while (1) {
    bool duplicated = false;
    for (auto proj : sys_project_set) {
      if (proj.GetProjectName() == imported_project.GetProjectName() &&
          proj.GetProjectMode() == imported_project.GetProjectMode()) {
        int idx = 1;
        QString project_name = imported_project.GetProjectName();
        QRegularExpressionMatch match = re.match(project_name);
        if (match.hasMatch()) {
          idx = match.captured("idx").toInt() + 1;
          project_name.replace(re, QString("(%1)").arg(idx).toStdString().c_str());
        } else {
          project_name.append(QString("(%1)").arg(idx).toStdString().c_str());
        }
        imported_project.SetProjectName(project_name);
        duplicated = true;
        break;
      }
    }
    if (!duplicated) break;
  }

  // Check the device profile is exist in the system or the imported resource
  QList<QString> aggregated_device_profile_keys;
  for (ActDeviceProfile dev_profile : sys_device_profile_set) {
    QString system_device_profile_key;
    dev_profile.GetClassKeys(system_device_profile_key);
    aggregated_device_profile_keys.push_back(system_device_profile_key);
  }

  for (ActDeviceProfile imported_dev_profile : imported_device_profile_set) {
    QString imported_device_profile_key;
    imported_dev_profile.GetClassKeys(imported_device_profile_key);
    aggregated_device_profile_keys.push_back(imported_device_profile_key);
  }

  // If the key of the device profile does not exist in the system or the imported file, return failed
  for (ActExportDeviceProfileInfo imported_dev_profile_info : imported_device_profile_infos) {
    if (!aggregated_device_profile_keys.contains(imported_dev_profile_info.GetKey())) {
      QString error_msg = QString("The device profile %1 not found").arg(imported_dev_profile_info.GetKey());
      qCritical() << error_msg.toStdString().c_str();

      return std::make_shared<ActStatusNotFound>(imported_dev_profile_info.GetKey());
    }
  }

  // Need to replace the imported device profile id to the system exist items
  for (ActExportDeviceProfileInfo imported_dev_profile_info : imported_device_profile_infos) {
    for (ActDeviceProfile sys_dev_profile : sys_device_profile_set) {
      QString system_device_profile_key;
      sys_dev_profile.GetClassKeys(system_device_profile_key);

      if (imported_dev_profile_info.GetKey() == system_device_profile_key) {
        for (QSet<ActDevice>::iterator device_iterator = imported_device_set.begin();
             device_iterator != imported_device_set.end();) {
          ActDevice device = *device_iterator;
          if (device.GetDeviceProfileId() == imported_dev_profile_info.GetId()) {
            device_iterator = imported_device_set.erase(device_iterator);
            device.SetDeviceProfileId(sys_dev_profile.GetId());
            new_device_set.insert(device);
          } else {
            device_iterator++;
          }
        }

        break;
      }
    }
  }

  // Check imported customized device profile
  for (ActDeviceProfile imported_dev_profile : imported_device_profile_set) {
    if (imported_dev_profile.GetBuiltIn()) {
      continue;
    }

    qint64 orig_device_profile_id = imported_dev_profile.GetId();

    // Check the device profile exist
    QString imported_device_profile_key;
    imported_dev_profile.GetClassKeys(imported_device_profile_key);
    bool check_exist = false;
    for (ActDeviceProfile sys_dev_profile : sys_device_profile_set) {
      QString sys_device_profile_key;
      sys_dev_profile.GetClassKeys(sys_device_profile_key);

      // Check the device profile exist or not
      if (imported_device_profile_key == sys_device_profile_key) {
        check_exist = true;
        break;
      }
    }

    // If the device profile exist, skip it
    if (check_exist) {
      continue;
    }

    act_status = act::core::g_core.UploadDeviceProfile(imported_dev_profile);

    // Replace the origin device profile id to new id in the device list of the imported project
    qint64 new_device_profile_id = imported_dev_profile.GetId();
    for (QSet<ActDevice>::iterator device_iterator = imported_device_set.begin();
         device_iterator != imported_device_set.end();) {
      ActDevice device = *device_iterator;
      if (device.GetDeviceProfileId() == orig_device_profile_id) {
        device_iterator = imported_device_set.erase(device_iterator);
        device.SetDeviceProfileId(new_device_profile_id);
        new_device_set.insert(device);
      } else {
        device_iterator++;
      }
    }
  }

  if (imported_device_set.size() != 0) {
    qint16 device_count = 0;
    QString error_msg = QString("There are %1 devices have no device profile in the system after importing:")
                            .arg(imported_device_set.size());

    // [bugfix:2879] Import Project error message is empty
    auto backend_log = error_msg;
    for (auto imported_device : imported_device_set) {
      if (device_count < 10) {
        error_msg.append(QString("\\n%1").arg(imported_device.GetIpv4().GetIpAddress()));
      }
      if (device_count == 10) {
        error_msg.append("\\n...");
      }
      backend_log.append(QString("\n%1").arg(imported_device.GetIpv4().GetIpAddress()));

      device_count += 1;
    }
    qCritical() << backend_log.toStdString().c_str();

    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Assign used IP address
  for (ActDevice dev : new_device_set) {
    // Check the device does not exist with duplicated IP address
    quint32 ip_num = 0;
    ActIpv4::AddressStrToNumber(dev.GetIpv4().GetIpAddress(), ip_num);
    imported_project.used_ip_addresses_.insert(ip_num);
  }

  // Import project
  imported_project.SetDevices(new_device_set);

  // Generate a new unique id
  qint64 project_id = imported_project.GetId();
  act_status =
      this->GenerateUniqueIdFromTimeStamp<ActProject>(sys_project_set, this->last_assigned_project_id_, project_id);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot get an available unique id";

    return act_status;
  }
  imported_project.SetId(project_id);

  // TODO: Version upgrade

  // Assign required properties
  QString act_version(QString::number(ACT_MAJOR_VERSION));
  act_version.append(".");
  act_version.append(QString::number(ACT_MINOR_VERSION));
  if constexpr (ACT_CUSTOMIZED_VERSION != 0) {
    act_version.append(".");
    act_version.append(QString::number(ACT_CUSTOMIZED_VERSION));
  }
  imported_project.SetActVersion(act_version);

  imported_project.SetDataVersion(ACT_PROJECT_DATA_VERSION);

  qint64 current_timestamp = QDateTime::currentSecsSinceEpoch();
  imported_project.SetLastModifiedTime(current_timestamp);

  QSet<qint64> new_design_baseline_ids;
  QSet<qint64> new_operation_baseline_ids;

  // [feat:3854] Include Baselines when exporting the project
  if (!imported_cfg.GetDesignBaselines().isEmpty()) {
    QSet<ActNetworkBaseline> design_baseline_set = this->GetDesignBaselineSet();
    QSet<ActNetworkBaseline> operation_baseline_set = this->GetOperationBaselineSet();

    qint64 new_activate_baseline_id = -1;
    for (auto design_baseline : imported_cfg.GetDesignBaselines()) {
      qint64 old_baseline_id = design_baseline.GetId();

      // Generate a new unique id
      qint64 new_baseline_id;
      act_status = this->GenerateUniqueId<ActNetworkBaseline>(design_baseline_set,
                                                              this->last_assigned_design_baseline_id_, new_baseline_id);
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__ << "Cannot get an available unique id";
        return act_status;
      }

      // Handle Design Baseline
      design_baseline.DecryptPassword();
      design_baseline.SetProjectId(project_id);
      design_baseline.SetId(new_baseline_id);
      design_baseline.GetProject().SetId(project_id);
      design_baseline.GetProject().SetActivateBaselineId(-1);
      new_design_baseline_ids.insert(new_baseline_id);
      if (old_baseline_id == imported_project.GetActivateBaselineId()) {
        new_activate_baseline_id = new_baseline_id;
      }

      // Write to db
      act_status = act::database::networkbaseline::WriteData(ActBaselineModeEnum::kDesign, design_baseline);
      design_baseline_set.insert(design_baseline);

      // Handle Operation Baseline
      // Find Operation Baseline
      qint32 operation_baseline_index = -1;
      auto find_status = ActGetListItemIndexById<ActNetworkBaseline>(imported_cfg.GetOperationBaselines(),
                                                                     old_baseline_id, operation_baseline_index);
      if (IsActStatusSuccess(find_status)) {  // found
        ActNetworkBaseline operation_baseline = imported_cfg.GetOperationBaselines().at(operation_baseline_index);
        operation_baseline.DecryptPassword();
        operation_baseline.SetProjectId(project_id);
        operation_baseline.SetId(new_baseline_id);
        operation_baseline.GetProject().SetId(project_id);
        operation_baseline.GetProject().SetActivateBaselineId(-1);
        new_operation_baseline_ids.insert(new_baseline_id);

        // Write to db
        find_status = act::database::networkbaseline::WriteData(ActBaselineModeEnum::kOperation, operation_baseline);
        operation_baseline_set.insert(operation_baseline);
      }
    }
    imported_project.SetDesignBaselineIds(new_design_baseline_ids);
    imported_project.SetOperationBaselineIds(new_operation_baseline_ids);
    imported_project.SetActivateBaselineId(new_activate_baseline_id);

    this->SetDesignBaselineSet(design_baseline_set);
    this->SetOperationBaselineSet(operation_baseline_set);
  }

  // Write to db
  act_status = act::database::project::WriteData(imported_project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Write database failed";
    return act_status;
  }

  // Insert the project to core set
  sys_project_set.insert(imported_project);

  this->SetProjectSet(sys_project_set);

  // Send update msg
  ActProjectPatchUpdateMsg msg(ActPatchUpdateActionEnum::kCreate, imported_project, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, msg);

  // [feat:955] Undo/Redo - Initiate the project operation history list
  undo_operation_history[project_id] = QStack<ActProject>();
  redo_operation_history[project_id] = QStack<ActProject>();

  // Initiate the project activate deploy flag
  deploy_available[project_id] = false;

  // [bug:2832] move the project status out of the project configuration
  this->project_status_list[project_id] = ActProjectStatusEnum::kIdle;

  return act_status;
}

ACT_STATUS ActCore::CopyProject(qint64 &project_id, QString &project_name, ActProject &copied_project) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  act_status = this->GetProject(project_id, copied_project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  copied_project.SetProjectName(project_name);

  // Assign new UUID for the new project
  copied_project.SetUUID(QUuid::createUuid().toString(QUuid::WithoutBraces));

  // Copy the Baselines
  QSet<ActNetworkBaseline> design_baseline_set = this->GetDesignBaselineSet();
  QSet<ActNetworkBaseline> operation_baseline_set = this->GetOperationBaselineSet();
  QSet<qint64> new_design_baseline_ids;
  QSet<qint64> new_operation_baseline_ids;

  for (auto old_baseline_id : copied_project.GetDesignBaselineIds()) {
    // Find Design Baseline
    ActNetworkBaseline design_baseline;
    act_status = ActGetItemById<ActNetworkBaseline>(this->GetDesignBaselineSet(), old_baseline_id, design_baseline);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << __func__ << QString("Cannot found the DesignBaseline id %1").arg(old_baseline_id);
      continue;
    }

    // Generate a new unique id
    qint64 new_baseline_id;
    act_status = this->GenerateUniqueId<ActNetworkBaseline>(design_baseline_set,
                                                            this->last_assigned_design_baseline_id_, new_baseline_id);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << __func__ << "Cannot get an available unique id";
      return act_status;
    }
    // Handle Design Baseline
    design_baseline.SetProjectId(copied_project.GetId());
    design_baseline.SetId(new_baseline_id);
    // Write to db
    act_status = act::database::networkbaseline::WriteData(ActBaselineModeEnum::kDesign, design_baseline);
    design_baseline_set.insert(design_baseline);
    new_design_baseline_ids.insert(new_baseline_id);

    // Handle Operation Baseline
    // Find Operation Baseline
    ActNetworkBaseline operation_baseline;
    act_status =
        ActGetItemById<ActNetworkBaseline>(this->GetOperationBaselineSet(), old_baseline_id, operation_baseline);
    if (IsActStatusSuccess(act_status)) {  // found
      operation_baseline.SetProjectId(copied_project.GetId());
      operation_baseline.SetId(new_baseline_id);
      // Write to db
      act_status = act::database::networkbaseline::WriteData(ActBaselineModeEnum::kOperation, operation_baseline);
      operation_baseline_set.insert(operation_baseline);
      new_operation_baseline_ids.insert(new_baseline_id);
    }
  }

  this->SetDesignBaselineSet(design_baseline_set);
  this->SetOperationBaselineSet(operation_baseline_set);
  copied_project.SetDesignBaselineIds(new_design_baseline_ids);
  copied_project.SetOperationBaselineIds(new_operation_baseline_ids);

  // Create the project in core memory
  act_status = this->CreateProject(copied_project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot create the copied project:" << copied_project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::CloneProjectToOtherMode(qint64 &project_id, const ActProjectModeEnum &mode,
                                            ActProject &copied_project) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  act_status = this->GetProject(project_id, copied_project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  copied_project.SetProjectMode(mode);

  // Create the project in core memory
  act_status = this->CreateProject(copied_project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot clone the project:" << copied_project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::CreateProject(ActProject &project) {
  ACT_STATUS_INIT();

  QSet<ActProject> project_set = this->GetProjectSet();

  // Add license control
  /* quint16 lic_project_size = this->GetLicense().GetSize().GetProjectSize();
  if ((project_set.size() >= lic_project_size) &&
      (lic_project_size != 0)) {  // [feat:2852] License - project size = 0 means unlimited
    QString error_msg = QString("The project size exceeds the limit: %1").arg(lic_project_size);
    qCritical() << error_msg.toStdString().c_str();

    return std::make_shared<ActLicenseSizeFailedRequest>("Project Size", lic_project_size);
  } */

  // [feat:3889] General Profile cannot be imported into other Stages
  // If the license does not support the profile, reject it
  // Add self-panning case for old project
  /* if (project.GetProfile() != ActServiceProfileForLicenseEnum::kSelfPlanning &&
      !this->GetLicense().GetProfiles().contains(project.GetProfile())) {
    QString error_msg = QString("The project profile %1 is not supported by the license")
                            .arg(GetStringFromEnum<ActServiceProfileForLicenseEnum>(
                                project.GetProfile(), kActServiceProfileForLicenseEnumMap));
    qCritical() << error_msg.toStdString().c_str();

    return std::make_shared<ActLicenseNotSupport>(
        GetStringFromEnum<ActServiceProfileForLicenseEnum>(project.GetProfile(), kActServiceProfileForLicenseEnumMap));
  } */

  act_status = this->CheckProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check project failed";

    return act_status;
  }

  // Check the project does not exist with duplicated name
  // [bugfix:3190] Project should not be created when its name is over 64 digits
  QRegularExpression re("\\((?<idx>\\d+)\\)$");
  while (1) {
    bool duplicated = false;
    for (auto proj : project_set) {
      if (proj.GetProjectName() == project.GetProjectName() && proj.GetProjectMode() == project.GetProjectMode()) {
        int idx = 1;
        QString project_name = project.GetProjectName();
        QRegularExpressionMatch match = re.match(project_name);
        if (match.hasMatch()) {
          idx = match.captured("idx").toInt() + 1;
          project_name.replace(re, QString("(%1)").arg(idx).toStdString().c_str());
        } else {
          project_name.append(QString("(%1)").arg(idx).toStdString().c_str());
        }
        project.SetProjectName(project_name);
        duplicated = true;
        break;
      }
    }
    if (!duplicated) break;
  }

  // Generate a new unique id
  qint64 id;
  act_status = this->GenerateUniqueIdFromTimeStamp<ActProject>(project_set, this->last_assigned_project_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot get an available unique id";

    return act_status;
  }
  project.SetId(id);

  // Assign required properties
  QString act_version(QString::number(ACT_MAJOR_VERSION));
  act_version.append(".");
  act_version.append(QString::number(ACT_MINOR_VERSION));
  if constexpr (ACT_CUSTOMIZED_VERSION != 0) {
    act_version.append(".");
    act_version.append(QString::number(ACT_CUSTOMIZED_VERSION));
  }
  project.SetActVersion(act_version);

  project.SetDataVersion(ACT_PROJECT_DATA_VERSION);

  qint64 current_timestamp = QDateTime::currentSecsSinceEpoch();
  project.SetCreatedTime(current_timestamp);
  project.SetLastModifiedTime(current_timestamp);

  // Insert the project to core set
  project_set.insert(project);

  this->SetProjectSet(project_set);

  // Send update msg
  ActProjectPatchUpdateMsg msg(ActPatchUpdateActionEnum::kCreate, project, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, msg);

  // [feat:955] Undo/Redo - Initiate the project operation history list
  qint64 project_id = project.GetId();
  undo_operation_history[project_id] = QStack<ActProject>();
  redo_operation_history[project_id] = QStack<ActProject>();

  // Initiate the project activate deploy flag
  deploy_available[project_id] = false;

  // [bug:2832] move the project status out of the project configuration
  this->project_status_list[project.GetId()] = ActProjectStatusEnum::kIdle;

  // Write to db
  return act::database::project::WriteData(project);
}

ACT_STATUS ActCore::GetProject(const qint64 project_id, ActProject &project, bool is_operation) {
  ACT_STATUS_INIT();

  act_status = ActGetItemById<ActProject>(this->GetProjectSet(), project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("Project id %1").arg(project_id);
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  // If the operation mode is enabled, return the operation project
  if (is_operation) {
    // [bugfix:4091] The project creation failed because the topology is not empty
    if (project_id == this->monitor_project_.GetId()) {
      project = this->monitor_project_;
      return ACT_STATUS_SUCCESS;
    } else {
      QString error_msg = QString("The project %1 hasn't started operation yet.").arg(project.GetProjectName());
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetProjectName(const qint64 project_id, QString &project_name) {
  ACT_STATUS_INIT();

  ActProject project;
  act_status = ActGetItemById<ActProject>(this->GetProjectSet(), project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("Project id %1").arg(project_id);
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  project_name = project.GetProjectName();

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetProjectByUUIDFromMode(const QString &uuid, const ActProjectModeEnum &mode, ActProject &project) {
  ACT_STATUS_INIT();

  QSet<ActProject> project_set = this->GetProjectSet();
  for (ActProject proj : project_set) {
    if (proj.GetUUID() == uuid && proj.GetProjectMode() == mode) {
      project = proj;
      return act_status;
    }
  }

  QString error_msg = QString("Project uuid %1").arg(uuid);
  return std::make_shared<ActStatusNotFound>(error_msg);
}

ACT_STATUS ActCore::GetSimpleProjectSet(QSet<ActSimpleProject> &simple_project_set) {
  ACT_STATUS_INIT();

  QSet<ActProject> project_set = this->GetProjectSet();
  for (ActProject project : project_set) {
    ActSimpleProject simple_project(project);

    simple_project.SetProjectStatus(this->project_status_list[project.GetId()]);

    simple_project_set.insert(simple_project);
  }

  return act_status;
}

ACT_STATUS ActCore::GetSimpleProject(const qint64 project_id, ActSimpleProject &simple_project) {
  ACT_STATUS_INIT();

  ActProject project;
  act_status = ActGetItemById<ActProject>(this->GetProjectSet(), project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("Project id %1").arg(project_id);
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  ActSimpleProject tmp_simple_project(project);
  tmp_simple_project.SetProjectStatus(this->project_status_list[project_id]);
  simple_project = tmp_simple_project;

  return act_status;
}

ACT_STATUS ActCore::UpdateProject(ActProject &project, bool sync_to_websocket, bool is_operation) {
  ACT_STATUS_INIT();

  act_status = this->CheckProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check project failed";
    return act_status;
  }

  // Update the last modified timestamp
  qint64 current_timestamp = QDateTime::currentSecsSinceEpoch();
  project.SetLastModifiedTime(current_timestamp);

  // If the monitor mode is enabled, return the monitor project
  if (is_operation) {
    monitor_mutex_.lock();
    this->monitor_project_ = project;

    monitor_mutex_.unlock();
    return ACT_STATUS_SUCCESS;
  }

  QSet<ActProject> project_set = this->GetProjectSet();

  // Check the item does exist by id
  typename QSet<ActProject>::const_iterator iterator;
  iterator = project_set.find(project);
  if (iterator != project_set.end()) {
    // [feat:955] Undo/Redo - Insert it to the history list
    ActProject prev_project = *iterator;
    SaveOperation(prev_project);

    // If yes, delete it
    project_set.erase(iterator);
  }

  qint64 project_id = project.GetId();
  if (project_id == -1) {
    // Generate a new unique id
    qint64 id;
    act_status = this->GenerateUniqueIdFromTimeStamp<ActProject>(project_set, this->last_assigned_project_id_, id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Cannot get an available unique id";

      return act_status;
    }
    project.SetId(id);
  }

  // Insert the project to core set
  project_set.insert(project);
  this->SetProjectSet(project_set);

  // [feat:722] Auto Save
  if (this->GetSystemConfig().GetAutoSave()) {
    act_status = this->SaveProject(project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Save project failed with project id:" << project_id;

      return act_status;
    }
  }

  // Send update msg
  ActProjectPatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, project, sync_to_websocket);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project.GetId());

  return act_status;
}

ACT_STATUS ActCore::DeleteProject(qint64 &project_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QSet<ActProject> project_set = this->GetProjectSet();

  if (this->monitor_project_.GetId() == project_id) {
    if (this->project_status_list[project_id] == ActProjectStatusEnum::kMonitoring) {
      QString error_msg = QString("Delete project failed, project (%1) is in operation mode.")
                              .arg(this->monitor_project_.GetProjectName());
      qCritical() << error_msg.toStdString().c_str();

      return std::make_shared<ActBadRequest>(error_msg);
    }
    this->monitor_project_ = ActProject();
  }

  ActProject project;
  act_status = GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QString project_name;
  project_name = project.GetProjectName();
  // If yes, delete it
  // qDebug() << "Delete item id:" << QString::number(project_id);
  project_set.remove(project);

  // [feat:955] Undo/Redo - Destroy the undo/redo stack
  undo_operation_history.remove(project_id);
  redo_operation_history.remove(project_id);

  // Destroy the project activate deploy flag
  deploy_available.remove(project_id);

  if (ws_thread_handler_pools.contains(project_id)) {
    act_status = RemoveWSJob(project_id);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // [feat:3602] Network Baseline check
  DeleteProjectAllBaselines(project_id);

  this->SetProjectSet(project_set);
  this->project_status_list.remove(project_id);

  // Write to db
  act_status = act::database::project::DeleteProjectFile(project_id, project_name);

  // Send update msg
  ActProjectPatchUpdateMsg msg_project(ActPatchUpdateActionEnum::kDelete, project, true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg_project, project_id);
  ActProjectPatchUpdateMsg msg_system(ActPatchUpdateActionEnum::kDelete, project, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, msg_system);

  return act_status;
}

ACT_STATUS ActCore::UpdateProjectStatus(const qint64 &project_id, const ActProjectStatusEnum &status) {
  ACT_STATUS_INIT();

  // Check the item does exist by id
  if (!this->project_status_list.contains(project_id)) {
    QString error_msg = QString("Project id %1").arg(project_id);
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  switch (status) {
    case ActProjectStatusEnum::kIdle: {
      this->project_status_list[project_id] = ActProjectStatusEnum::kIdle;
    } break;
    case ActProjectStatusEnum::kAborted: {
      if (this->project_status_list[project_id] == ActProjectStatusEnum::kIdle ||
          this->project_status_list[project_id] == ActProjectStatusEnum::kFinished) {
        QString error_msg = QString("Project (%1) is already in %2 status")
                                .arg(project.GetProjectName())
                                .arg(GetStringFromEnum<ActProjectStatusEnum>(this->project_status_list[project_id],
                                                                             kActProjectStatusEnumMap));
        return std::make_shared<ActBadRequest>(error_msg);
      }
      this->project_status_list[project_id] = status;
    } break;
    case ActProjectStatusEnum::kFinished: {
      this->project_status_list[project_id] = ActProjectStatusEnum::kIdle;
    } break;
    case ActProjectStatusEnum::kComputing:
    case ActProjectStatusEnum::kComparing:
    case ActProjectStatusEnum::kIntelligentRequestSending:
    case ActProjectStatusEnum::kIntelligentUploadSending:
    case ActProjectStatusEnum::kIntelligentDownloadSending: {
      if (this->project_status_list[project_id] != ActProjectStatusEnum::kIdle &&
          this->project_status_list[project_id] != ActProjectStatusEnum::kFinished &&
          this->project_status_list[project_id] != ActProjectStatusEnum::kAborted) {
        QString error_msg = QString("Project (%1) is in %2 status")
                                .arg(project.GetProjectName())
                                .arg(GetStringFromEnum<ActProjectStatusEnum>(this->project_status_list[project_id],
                                                                             kActProjectStatusEnumMap));
        return std::make_shared<ActBadRequest>(error_msg);
      }
    } break;
    case ActProjectStatusEnum::kDeploying:
    case ActProjectStatusEnum::kSyncing:
    case ActProjectStatusEnum::kScanning:
    case ActProjectStatusEnum::kMonitoring:
    case ActProjectStatusEnum::kTopologyMapping:
    case ActProjectStatusEnum::kBroadcastSearching:
    case ActProjectStatusEnum::kDeviceConfiguring: {
      if (this->project_status_list[project_id] != ActProjectStatusEnum::kIdle &&
          this->project_status_list[project_id] != ActProjectStatusEnum::kFinished &&
          this->project_status_list[project_id] != ActProjectStatusEnum::kAborted) {
        QString error_msg = QString("Project (%1) is in %2 status")
                                .arg(project.GetProjectName())
                                .arg(GetStringFromEnum<ActProjectStatusEnum>(this->project_status_list[project_id],
                                                                             kActProjectStatusEnumMap));
        return std::make_shared<ActBadRequest>(error_msg);
      }
      for (auto status_iter = this->project_status_list.begin(); status_iter != this->project_status_list.end();
           status_iter++) {
        if (status_iter.key() != project_id && (status_iter.value() == ActProjectStatusEnum::kDeploying ||
                                                status_iter.value() == ActProjectStatusEnum::kSyncing ||
                                                status_iter.value() == ActProjectStatusEnum::kScanning ||
                                                status_iter.value() == ActProjectStatusEnum::kMonitoring ||
                                                status_iter.value() == ActProjectStatusEnum::kTopologyMapping ||
                                                status_iter.value() == ActProjectStatusEnum::kBroadcastSearching ||
                                                status_iter.value() == ActProjectStatusEnum::kDeviceConfiguring)) {
          QString another_project_name;
          act_status = this->GetProjectName(status_iter.key(), another_project_name);
          if (!IsActStatusSuccess(act_status)) {
            qCritical() << "Get project name failed with project id:" << status_iter.key();
            return act_status;
          }
          QString error_msg =
              QString("Another project (%1) is in %2 status")
                  .arg(another_project_name)
                  .arg(GetStringFromEnum<ActProjectStatusEnum>(status_iter.value(), kActProjectStatusEnumMap));
          return std::make_shared<ActBadRequest>(error_msg);
        }
      }
      this->project_status_list[project_id] = status;
    } break;
    default: {
      QString error_msg = QString("Project status %1 is invalid")
                              .arg(GetStringFromEnum<ActProjectStatusEnum>(status, kActProjectStatusEnumMap));
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  ActSimpleProject simple_project(project);
  simple_project.SetProjectStatus(this->project_status_list[project_id]);

  // Send update msg
  ActSimpleProjectPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, simple_project, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_msg);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetProjectDataVersion(qint64 &project_id, const qint64 &ws_listener_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kGetProjectDataVersion, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return act_status;
  }

  ActDataVersionWSResponse ws_resp(project.GetDataVersion());
  qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return act_status;
}

ACT_STATUS ActCore::CheckServerProjectExpired() {
  ACT_STATUS_INIT();

  /* QMutexLocker lock(&this->mutex_);

  // if the deployment type is server in the license, check the project expiration
  if (this->GetLicense().GetDeploymentType() != ActDeploymentTypeEnum::kServer) {
    return act_status;
  }

  qint64 current_timestamp = QDateTime::currentSecsSinceEpoch();
  QSet<ActProject> project_set = this->GetProjectSet();
  for (auto it = project_set.begin(); it != project_set.end();) {
    ActProject project = *it;
    qint64 last_modified_time = project.GetLastModifiedTime();
    qint64 project_expired_time = last_modified_time + ACT_SERVER_PROJECT_EXPIRED_TIMEOUT;
    if (current_timestamp > project_expired_time) {
      // [feat:3602] Network Baseline check
      DeleteProjectAllBaselines(project.GetId());

      // [feat:955] Undo/Redo - Destroy the undo/redo stack
      undo_operation_history.remove(project.GetId());
      redo_operation_history.remove(project.GetId());

      // Destroy the project activate deploy flag
      deploy_available.remove(project.GetId());

      if (ws_thread_handler_pools.contains(project.GetId())) {
        act_status = RemoveWSJob(project.GetId());
        if (!IsActStatusSuccess(act_status)) {
          return act_status;
        }
      }

      // [bug:2832] move the project status out of the project configuration
      this->project_status_list.remove(project.GetId());

      // Write to db
      act_status = act::database::project::DeleteProjectFile(project.GetId(), project.GetProjectName());

      // Send update msg
      ActProjectPatchUpdateMsg msg_project(ActPatchUpdateActionEnum::kDelete, project, true);
      this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg_project, project.GetId());
      ActProjectPatchUpdateMsg msg_system(ActPatchUpdateActionEnum::kDelete, project, true);
      this->SendMessageToListener(ActWSTypeEnum::kSystem, false, msg_system);

      // Remove the project from the set
      it = project_set.erase(it);

      qWarning() << "Project expired:" << project.GetProjectName().toStdString().c_str();
    } else {
      ++it;
    }
  }

  // Update the project set
  this->SetProjectSet(project_set); */

  return act_status;
}

}  // namespace core
}  // namespace act
