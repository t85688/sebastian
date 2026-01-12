#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::CheckProjectSetting(const ActProjectSetting &project_setting, bool check_account) {
  ACT_STATUS_INIT();

  // Check ProjectName
  act_status = this->CheckProjectName(project_setting.GetProjectName());
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Check the ProjectName failed";
    return act_status;
  }

  // Check VLAN range
  act_status = this->CheckVlanRange(project_setting.GetVlanRange(), project_setting.GetProjectName());
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Check the VLAN range failed";
    return act_status;
  }

  // Check Connection(SNMP, NETCONF, RESTful)
  if (check_account) {
    act_status = this->CheckConnectionConfigField(project_setting);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << QString("Check the ProjectSetting connection config failed.");
      return act_status;
    }
  }

  // Check SNMP Trap Configuration
  if (check_account) {
    act_status =
        this->CheckSnmpTrapConfiguration(project_setting.GetSnmpTrapConfiguration(), project_setting.GetProjectName());
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Check the SNMP Trap Configuration failed";
      return act_status;
    }
  }

  // Check TrafficTypeToPriorityCodePointMapping
  act_status = this->CheckTrafficTypeToPCPMapping(project_setting.GetTrafficTypeToPriorityCodePointMapping(),
                                                  project_setting.GetProjectName());
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Check the TrafficType to PriorityCodePointer mapping failed";
    return act_status;
  }

  // Check AlgorithmConfiguration
  // [bugfix:2152] Missing check algorithm project setting
  act_status =
      this->CheckAlgorithmConfiguration(project_setting.GetAlgorithmConfiguration(), project_setting.GetProjectName());
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Check the algorithm configuration failed";
    return act_status;
  }

  // Check ScanIpRanges
  act_status = this->CheckScanIpRanges(project_setting.GetScanIpRanges(), project_setting.GetProjectName());
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Check the ScanIpRanges failed";
    return act_status;
  }

  // Check Project Start IP
  act_status =
      this->CheckProjectStartIP(project_setting.GetProjectStartIp().GetIpAddress(), project_setting.GetProjectName());
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Check the Start IP failed";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::CheckAlgorithmConfiguration(const ActAlgorithmConfiguration &algorithm_config,
                                                QString project_name) {
  ACT_STATUS_INIT();

  // The valid range should be 1 ~ 64.
  if (algorithm_config.GetMediaSpecificOverheadBytes() < ACT_MEDIA_OVERHEAD_MIN ||
      algorithm_config.GetMediaSpecificOverheadBytes() > ACT_MEDIA_OVERHEAD_MAX) {
    QString error_msg = QString("AlgorithmConfiguration - The valid media overhead should be %1 ~ %2, instead of %3")
                            .arg(ACT_MEDIA_OVERHEAD_MIN)
                            .arg(ACT_MEDIA_OVERHEAD_MAX)
                            .arg(QString::number(algorithm_config.GetMediaSpecificOverheadBytes()));
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // The valid range should be 0 ~ 1000.
  if (algorithm_config.GetTimeSyncDelay() > ACT_TIME_SYNC_DELAY_MAX) {
    QString error_msg = QString("AlgorithmConfiguration - The valid time sync delay should be %1 ~ %2, instead of %3")
                            .arg(ACT_TIME_SYNC_DELAY_MIN)
                            .arg(ACT_TIME_SYNC_DELAY_MAX)
                            .arg(QString::number(algorithm_config.GetTimeSyncDelay()));
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // The timeout of z3 computing time.
  if (algorithm_config.GetTimeout() > ACT_COMPUTE_TIMEOUT_MAX) {
    QString error_msg =
        QString("AlgorithmConfiguration - The valid timeout of computation should be %1 ~ %2, instead of %3")
            .arg(ACT_COMPUTE_TIMEOUT_MIN)
            .arg(ACT_COMPUTE_TIMEOUT_MAX)
            .arg(QString::number(algorithm_config.GetTimeout()));
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

ACT_STATUS ActCore::CheckProjectName(const QString &project_name) {
  ACT_STATUS_INIT();

  // The length should be 1 ~ 64 characters.
  if (project_name.length() < ACT_STRING_LENGTH_MIN || project_name.length() > ACT_STRING_LENGTH_MAX) {
    QString error_msg = QString("The length of project name %1 should be %2 ~ %3 characters")
                            .arg(project_name)
                            .arg(ACT_STRING_LENGTH_MIN)
                            .arg(ACT_STRING_LENGTH_MAX);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

ACT_STATUS ActCore::CheckTrafficTypeToPCPMapping(
    const ActTrafficTypeToPriorityCodePointMapping &traffic_type_to_pcp_mapping, QString project_name) {
  ACT_STATUS_INIT();

  // Check traffic type to pcp mapping
  if (traffic_type_to_pcp_mapping.GetBestEffort().size() == 0) {
    QString error_msg =
        QString("Project (%1) - There should be at least one PCP for Best Effort traffic").arg(project_name);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  } else if (!traffic_type_to_pcp_mapping.GetBestEffort().contains(0)) {
    QString error_msg = QString("Project (%1) - There should be Best Effort traffic in PCP queue 0").arg(project_name);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  } else if (traffic_type_to_pcp_mapping.GetTimeSync().size() == 0) {
    QString error_msg =
        QString("Project (%1) - There should be at least one PCP for Time Sync traffic").arg(project_name);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

bool check_ip_range_is_overlap(const ActScanIpRangeEntry &scan_ip_range_a, const ActScanIpRangeEntry &scan_ip_range_b) {
  quint32 start_ip_a = QHostAddress(scan_ip_range_a.GetStartIp()).toIPv4Address();
  quint32 end_ip_a = QHostAddress(scan_ip_range_a.GetEndIp()).toIPv4Address();
  quint32 start_ip_b = QHostAddress(scan_ip_range_b.GetStartIp()).toIPv4Address();
  quint32 end_ip_b = QHostAddress(scan_ip_range_b.GetEndIp()).toIPv4Address();

  if ((end_ip_a < start_ip_b) || (end_ip_b < start_ip_a)) {
    return false;
  }

  return true;
}

ACT_STATUS ActCore::CheckScanIpRanges(const QList<ActScanIpRangeEntry> &scan_ip_ranges, QString project_name) {
  ACT_STATUS_INIT();

  // For check scan_ip_range overlap
  QList<ActScanIpRangeEntry> checked_scan_ip_ranges;

  // Check all ScanIpRange
  for (auto scan_ip_range : scan_ip_ranges) {
    // Check IP address format
    QHostAddress first_addr(scan_ip_range.GetStartIp());
    if (first_addr.isNull()) {
      QString error_msg = QString("Project (%1) - The Start IP address is not valid: %2")
                              .arg(project_name)
                              .arg(scan_ip_range.GetStartIp());
      qCritical() << __func__ << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    // Check IP address format
    QHostAddress last_addr(scan_ip_range.GetEndIp());
    if (last_addr.isNull()) {
      QString error_msg =
          QString("Project (%1) - The End IP address is not valid: %2").arg(project_name).arg(scan_ip_range.GetEndIp());
      qCritical() << __func__ << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    // Check overlap
    for (auto checked_scan_ip_range : checked_scan_ip_ranges) {
      if (check_ip_range_is_overlap(scan_ip_range, checked_scan_ip_range)) {  // is overlap
        QString error_msg = QString("Project (%1) - The IP Range is overlap(%2-%3, %4-%5)")
                                .arg(project_name)
                                .arg(scan_ip_range.GetStartIp())
                                .arg(scan_ip_range.GetEndIp())
                                .arg(checked_scan_ip_range.GetStartIp())
                                .arg(checked_scan_ip_range.GetEndIp());
        qCritical() << __func__ << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }
    // Add scan_ip_range to checked_scan_ip_ranges
    checked_scan_ip_ranges.append(scan_ip_range);

    // Check Connection(SNMP, NETCONF, RESTful)
    act_status = this->CheckConnectionConfigField(scan_ip_range);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__
                  << QString("Project (%1) - Check the ScanIpRange connection config failed.").arg(project_name);
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS ActCore::CheckProjectStartIP(const QString &start_ip, QString project_name) {
  ACT_STATUS_INIT();

  // Check IP address format
  QHostAddress ip_addr(start_ip);
  if (ip_addr.isNull()) {
    QString error_msg = QString("Project (%1) - The Start IP address is not valid: %2").arg(project_name).arg(start_ip);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }
  return act_status;
}

ACT_STATUS ActCore::CheckVlanRange(const ActVlanRange &vlan_range, QString project_name) {
  ACT_STATUS_INIT();

  // The valid VLAN is 2 ~ 4094. (VLAN 1 is for managed VLAN).
  if (vlan_range.GetMin() < ACT_VLAN_MIN || vlan_range.GetMax() > ACT_VLAN_MAX) {
    QString error_msg = QString("Project (%1) - The VLAN %2 ~ %3 should be limit in 2 ~ 4094")
                            .arg(project_name)
                            .arg(vlan_range.GetMin())
                            .arg(vlan_range.GetMax());
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (vlan_range.GetMin() > vlan_range.GetMax()) {
    QString error_msg = QString("Project (%1) - The maximum VLAN %2 is smaller than minimum VLAN %3")
                            .arg(project_name)
                            .arg(vlan_range.GetMax())
                            .arg(vlan_range.GetMin());
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

ACT_STATUS ActCore::ReplaceProjSettingScanIpRangesByMemoryFromWizard(qint64 project_id,
                                                                     const QList<qint64> &skip_devices) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QList<ActScanIpRangeEntry> scan_ip_ranges;

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }
  auto project_setting = project.GetProjectSetting();

  // Get Project's DeviceIpConnectConfig
  QMap<qint64, ActDeviceIpConnectConfig> dev_ip_conn_cfg_map;
  act::core::g_core.GetProjectDevIpConnCfgMap(project_id, dev_ip_conn_cfg_map);
  // qDebug() << __func__
  //          << QString("Project(%1) dev_ip_conn_cfg_map size: %2")
  //                 .arg(project_id)
  //                 .arg(dev_ip_conn_cfg_map.size())
  //                 .toStdString()
  //                 .c_str();

  if (!dev_ip_conn_cfg_map.isEmpty()) {
    QList<QPair<quint32, qint64>> dev_ip_id_pair_list;  // QPair<ip_num, dev_id>

    // Generate QPair<ip_num, dev_id> list
    for (auto dev_id : dev_ip_conn_cfg_map.keys()) {
      // Skip device
      if (skip_devices.contains(dev_id)) {
        continue;
      }

      quint32 ip_num = 0;
      ActIpv4::AddressStrToNumber(dev_ip_conn_cfg_map[dev_id].GetIpAddress(), ip_num);
      dev_ip_id_pair_list.append(qMakePair(ip_num, dev_id));
    }

    // Sort the list by the value of the first item
    std::sort(dev_ip_id_pair_list.begin(), dev_ip_id_pair_list.end(),
              [](const QPair<quint32, qint64> &pair1, const QPair<quint32, qint64> &pair2) {
                return pair1.first < pair2.first;
              });

    // Iterator to Generate ScanIpRange list
    qint64 scan_ip_range_entry_id = 0;
    quint32 start_ip_num = dev_ip_id_pair_list.first().first;
    quint32 end_ip_num = dev_ip_id_pair_list.first().first;
    for (int i = 0; i < dev_ip_id_pair_list.size(); i++) {
      auto current_conn_cfg = dev_ip_conn_cfg_map[dev_ip_id_pair_list[i].second];
      quint32 current_ip_num = 0;
      quint32 next_ip_num = 0;

      // Check next one can join to scan_range_entry (skip last one)
      if ((dev_ip_id_pair_list.size() > 1) && (i < dev_ip_id_pair_list.size() - 1)) {
        current_ip_num = dev_ip_id_pair_list[i].first;
        next_ip_num = dev_ip_id_pair_list[i + 1].first;

        // Check consecutive
        if ((current_ip_num + 1) == next_ip_num) {
          auto next_conn_cfg = dev_ip_conn_cfg_map[dev_ip_id_pair_list[i + 1].second];

          // Check connect config
          if (this->CompareConnectionConfigConsistent(current_conn_cfg, next_conn_cfg)) {
            end_ip_num = next_ip_num;
            continue;
          }
        }
      }

      // Create Scan IP range and append to list
      scan_ip_range_entry_id = scan_ip_range_entry_id + 1;
      QString start_ip_str;
      QString end_ip_str;
      ActIpv4::AddressNumberToStr(start_ip_num, start_ip_str);
      ActIpv4::AddressNumberToStr(end_ip_num, end_ip_str);
      auto account = current_conn_cfg.GetAccount();
      auto snmp_cfg = current_conn_cfg.GetSnmpConfiguration();
      auto netconf_cfg = current_conn_cfg.GetNetconfConfiguration();
      auto restful_cfg = current_conn_cfg.GetRestfulConfiguration();

      // Check default connect config
      account.SetDefaultSetting(ActDeviceAccount::CompareConnectConsistent(account, project_setting.GetAccount()));
      snmp_cfg.SetDefaultSetting(
          ActSnmpConfiguration::CompareConnectConsistent(snmp_cfg, project_setting.GetSnmpConfiguration()));
      netconf_cfg.SetDefaultSetting(
          ActNetconfConfiguration::CompareConnectConsistent(netconf_cfg, project_setting.GetNetconfConfiguration()));
      restful_cfg.SetDefaultSetting(
          ActRestfulConfiguration::CompareConnectConsistent(restful_cfg, project_setting.GetRestfulConfiguration()));

      scan_ip_ranges.append(ActScanIpRangeEntry(scan_ip_range_entry_id, start_ip_str, end_ip_str, account, snmp_cfg,
                                                netconf_cfg, restful_cfg, current_conn_cfg.GetEnableSnmpSetting(),
                                                true));

      // Re assign the start & end ip num
      start_ip_num = next_ip_num;
      end_ip_num = next_ip_num;
    }
  }

  // qDebug() << __func__
  //          << QString("Project(%1) scan_ip_ranges size: %2")
  //                 .arg(project_id)
  //                 .arg(scan_ip_ranges.size())
  //                 .toStdString()
  //                 .c_str();

  // Replace the project setting > ScanIpRanges

  project_setting.SetScanIpRanges(scan_ip_ranges);
  act_status = this->UpdateProjectSetting(project_id, project_setting);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "UpdateProjectSetting() failed";
  }

  return act_status;
}

ACT_STATUS ActCore::HandleProjectSettingScanIpRangesField(ActProjectSetting &update_project_setting,
                                                          ActProject &db_project) {
  ACT_STATUS_INIT();

  auto db_project_setting = db_project.GetProjectSetting();
  QList<ActScanIpRangeEntry> new_scan_ip_ranges;
  auto db_scan_ip_ranges = db_project_setting.GetScanIpRanges();
  for (auto scan_ip_range : update_project_setting.GetScanIpRanges()) {
    // Find db scan_ip_range
    // Find scan_ip_range from db_project_setting
    qint32 scan_ip_range_index = -1;
    act_status =
        ActGetListItemIndexById<ActScanIpRangeEntry>(db_scan_ip_ranges, scan_ip_range.GetId(), scan_ip_range_index);
    if (!IsActStatusSuccess(act_status)) {  // not found
      // New
      this->HandleConnectionConfigField(scan_ip_range, db_project_setting, db_project_setting);

      // Generate a new unique id
      qint64 id;
      // qint64 last_id = 0;
      QSet<ActScanIpRangeEntry> scan_ip_range_set(db_scan_ip_ranges.begin(), db_scan_ip_ranges.end());
      act_status = this->GenerateUniqueId<ActScanIpRangeEntry>(scan_ip_range_set,
                                                               db_project.last_assigned_scan_ip_range_id_, id);

      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "Cannot get an available unique id";
        return act_status;
      }

      // Assign ID
      scan_ip_range.SetId(id);
      db_project.last_assigned_scan_ip_range_id_ = id;

    } else {  // found
      ActScanIpRangeEntry db_scan_ip_range = db_scan_ip_ranges.at(scan_ip_range_index);

      this->HandleConnectionConfigField(scan_ip_range, db_scan_ip_range, db_project_setting);
    }

    new_scan_ip_ranges.append(scan_ip_range);
  }
  update_project_setting.SetScanIpRanges(new_scan_ip_ranges);

  return act_status;
}

ACT_STATUS ActCore::CheckMonitorConfiguration(ActMonitorConfiguration &config, QString project_name) {
  ACT_STATUS_INIT();

  if (!(config.GetFromIpScanList() || config.GetFromOfflineProject())) {
    QString error_msg = QString("Project (%1) - The scan target must be chosen").arg(project_name);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateProjectSetting(qint64 &project_id, ActProjectSetting &project_setting, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();
  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QString project_name = project.GetProjectName();

  act_status = this->UpdateProjectSetting(project, project_setting);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
    // compute routing result
    act_status = this->ComputeTopologySetting(project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Compute routing results failed";
      return act_status;
    }
  }

  if (project.GetProjectName() != project_name) {
    act_status = act::database::project::DeleteProjectFile(project_id, project_name);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << QString("Unable to delete project file %1_%2.json").arg(project.GetId()).arg(project_name);
      return act_status;
    }
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project_id;
    return act_status;
  }

  // Send update msg
  ActProjectSettingPatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetProjectSetting(), true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateProjectSetting(ActProject &project, ActProjectSetting &project_setting) {
  ACT_STATUS_INIT();

  // Doesn't need to check the topology
  act_status = this->CheckProjectSetting(project_setting, true);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Check ProjectSetting failed";

    return act_status;
  }

  // Check the project does not exist with duplicated name
  QSet<ActProject> project_set = this->GetProjectSet();
  for (auto proj : project_set) {
    if (proj.GetId() == project.GetId()) {
      continue;
    }

    if (proj.GetProjectName() == project_setting.GetProjectName() &&
        proj.GetProjectMode() == project.GetProjectMode()) {
      qCritical() << __func__ << "The project name" << proj.GetProjectName() << "is duplicated";

      return std::make_shared<ActDuplicatedError>(proj.GetProjectName());
    }
  }

  // If the password is empty, which means this update is for other fields
  // The default setting of the project's connection config should always be false
  project_setting.GetAccount().SetDefaultSetting(false);
  project_setting.GetNetconfConfiguration().SetDefaultSetting(false);
  project_setting.GetSnmpConfiguration().SetDefaultSetting(false);
  project_setting.GetRestfulConfiguration().SetDefaultSetting(false);
  this->HandleConnectionConfigField(project_setting, project.GetProjectSetting(), project.GetProjectSetting());

  // Handle ScanIpRange ConnectionConfigField
  // Sync project > ProjectSetting > ScanIpRange to project_setting
  this->HandleProjectSettingScanIpRangesField(project_setting, project);

  // Sync default connection configuration of Project
  SyncDeviceAccountDefault(project_setting.GetAccount(), project);
  SyncNetconfDefaultConfiguration(project_setting.GetNetconfConfiguration(), project);
  SyncSnmpDefaultConfiguration(project_setting.GetSnmpConfiguration(), project);
  SyncRestfulDefaultConfiguration(project_setting.GetRestfulConfiguration(), project);

  // [feat:1805] Re-assign VLAN while the VLAN range is changed
  QSet<ActStream> stream_set = project.GetStreams();
  for (ActStream stream : stream_set) {
    if (stream.GetUserDefinedVlan()) {
      continue;
    }

    if (stream.GetVlanId() < project_setting.GetVlanRange().GetMin() ||
        stream.GetVlanId() > project_setting.GetVlanRange().GetMax()) {
      act_status = this->UpdateStream(project, stream);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "Update stream VLAN failed";

        return act_status;
      }
    }
  }

  project.SetProjectSetting(project_setting);

  return act_status;
}

ACT_STATUS ActCore::UpdateProjectSettingMember(const ActProjectSettingMember &project_setting_member,
                                               qint64 &project_id, ActProjectSetting &update_data, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateProjectSettingMember(project_setting_member, project, update_data);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  if (!is_operation) {
    // compute routing result
    act_status = this->ComputeTopologySetting(project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Compute routing results failed";
      return act_status;
    }
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Cannot update the project id:" << project_id;
    return act_status;
  }

  // The project setting might change the SNMP, NETCONF, RESTful connection parameters
  // The related device configuration should be updated too
  // So it will put the update notification to the temporary list
  // We need to send the device change notification to the websocket first
  // Then send the project setting change notification to websocket
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);
  ActProjectSettingPatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetProjectSetting(), true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateProjectSettingMember(const ActProjectSettingMember &project_setting_member,
                                               ActProject &project, ActProjectSetting &update_data) {
  ACT_STATUS_INIT();

  // Check & Update
  ActProjectSetting project_setting = project.GetProjectSetting();
  switch (project_setting_member) {
    case ActProjectSettingMember::kProjectName: {
      act_status = this->CheckProjectName(update_data.GetProjectName());
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      // Check the project does not exist with duplicated name
      QSet<ActProject> project_set = this->GetProjectSet();

      // Check the project does not exist with duplicated name
      // [bugfix:3190] Project should not be created when its name is over 64 digits
      QRegularExpression re("\\((?<idx>\\d+)\\)$");
      while (1) {
        bool duplicated = false;
        for (auto proj : project_set) {
          if (proj.GetProjectName() == update_data.GetProjectName() &&
              proj.GetProjectMode() == project.GetProjectMode()) {
            if (proj.GetId() == project.GetId()) {
              return act_status;
            }

            int idx = 1;
            QString project_name = update_data.GetProjectName();
            QRegularExpressionMatch match = re.match(project_name);
            if (match.hasMatch()) {
              idx = match.captured("idx").toInt() + 1;
              project_name.replace(re, QString("(%1)").arg(idx).toStdString().c_str());
            } else {
              project_name.append(QString("(%1)").arg(idx).toStdString().c_str());
            }
            update_data.SetProjectName(project_name);
            duplicated = true;
            break;
          }
        }
        if (!duplicated) break;
      }

      // Update file name in db
      act_status = act::database::project::UpdateProjectFileName(project.GetId(), project_setting.GetProjectName(),
                                                                 update_data.GetProjectName());

      project_setting.SetProjectName(update_data.GetProjectName());
    } break;

    case ActProjectSettingMember::kAlgorithmConfiguration:
      act_status = this->CheckAlgorithmConfiguration(update_data.GetAlgorithmConfiguration(), project.GetProjectName());
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      project_setting.SetAlgorithmConfiguration(update_data.GetAlgorithmConfiguration());
      break;

    case ActProjectSettingMember::kVlanRange:
      act_status = this->CheckVlanRange(update_data.GetVlanRange(), project.GetProjectName());
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      project_setting.SetVlanRange(update_data.GetVlanRange());
      break;

    case ActProjectSettingMember::kCfgWizardSetting:
      project_setting.SetCfgWizardSetting(update_data.GetCfgWizardSetting());
      break;

    case ActProjectSettingMember::kAccount: {
      if (update_data.GetAccount().GetPassword().length() > 0) {
        act_status = this->CheckDeviceAccount(update_data.GetAccount(), project.GetProjectName());
        if (!IsActStatusSuccess(act_status)) {
          return act_status;
        }
      }

      // Default setting always as false
      update_data.GetAccount().SetDefaultSetting(false);
      this->HandleDeviceAccount(update_data.GetAccount(), project.GetProjectSetting().GetAccount(),
                                project.GetProjectSetting().GetAccount());

      project_setting.SetAccount(update_data.GetAccount());
      SyncDeviceAccountDefault(project_setting.GetAccount(), project);
      // Sync project > ProjectSetting > ScanIpRange to project_setting
      project_setting.SetScanIpRanges(project.GetProjectSetting().GetScanIpRanges());
    } break;

    case ActProjectSettingMember::kNetconfConfiguration: {
      // Default setting always as false
      update_data.GetNetconfConfiguration().SetDefaultSetting(false);
      this->HandleNetconfConfiguration(update_data.GetNetconfConfiguration(),
                                       project.GetProjectSetting().GetNetconfConfiguration());
      project_setting.SetNetconfConfiguration(update_data.GetNetconfConfiguration());

      SyncNetconfDefaultConfiguration(project_setting.GetNetconfConfiguration(), project);
      // Sync project > ProjectSetting > ScanIpRange to project_setting
      project_setting.SetScanIpRanges(project.GetProjectSetting().GetScanIpRanges());
    } break;

    case ActProjectSettingMember::kSnmpConfiguration: {
      // Check for SNMPv1 & SNMPv2c
      if (update_data.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV2c ||
          update_data.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
        if (update_data.GetSnmpConfiguration().GetReadCommunity().length() > 0) {
          act_status = this->CheckSnmpConfiguration(update_data.GetSnmpConfiguration(), project.GetProjectName());
          if (!IsActStatusSuccess(act_status)) {
            return act_status;
          }
        }
      }

      // Check for SNMPv3
      if (update_data.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV3) {
        // AuthenticationType == none
        // or AuthenticationPassword.length > 0
        if ((update_data.GetSnmpConfiguration().GetAuthenticationType() == ActSnmpAuthenticationTypeEnum::kNone) ||
            (update_data.GetSnmpConfiguration().GetAuthenticationPassword().length() > 0)) {
          act_status = this->CheckSnmpConfiguration(update_data.GetSnmpConfiguration(), project.GetProjectName());
          if (!IsActStatusSuccess(act_status)) {
            return act_status;
          }
        }
      }

      // Default setting always as false
      update_data.GetSnmpConfiguration().SetDefaultSetting(false);
      this->HandleSnmpConfiguration(update_data.GetSnmpConfiguration(),
                                    project.GetProjectSetting().GetSnmpConfiguration(),
                                    project.GetProjectSetting().GetSnmpConfiguration());

      project_setting.SetSnmpConfiguration(update_data.GetSnmpConfiguration());
      SyncSnmpDefaultConfiguration(project_setting.GetSnmpConfiguration(), project);
      // Sync project > ProjectSetting > ScanIpRange to project_setting
      project_setting.SetScanIpRanges(project.GetProjectSetting().GetScanIpRanges());
    } break;

    case ActProjectSettingMember::kRestfulConfiguration: {
      // Default setting always as false
      update_data.GetRestfulConfiguration().SetDefaultSetting(false);
      this->HandleRestfulConfiguration(update_data.GetRestfulConfiguration(),
                                       project.GetProjectSetting().GetRestfulConfiguration());

      project_setting.SetRestfulConfiguration(update_data.GetRestfulConfiguration());
      SyncRestfulDefaultConfiguration(project_setting.GetRestfulConfiguration(), project);
      // Sync project > ProjectSetting > ScanIpRange to project_setting
      project_setting.SetScanIpRanges(project.GetProjectSetting().GetScanIpRanges());
    } break;

    case ActProjectSettingMember::kTrafficTypeToPriorityCodePointMapping:
      act_status = this->CheckTrafficTypeToPCPMapping(update_data.GetTrafficTypeToPriorityCodePointMapping(),
                                                      project.GetProjectName());
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      project_setting.SetTrafficTypeToPriorityCodePointMapping(update_data.GetTrafficTypeToPriorityCodePointMapping());
      break;

    case ActProjectSettingMember::kPriorityCodePointToQueueMapping:
      project_setting.SetPriorityCodePointToQueueMapping(update_data.GetPriorityCodePointToQueueMapping());
      break;

    case ActProjectSettingMember::kScanIpRanges: {
      act_status = this->CheckScanIpRanges(update_data.GetScanIpRanges(), project.GetProjectName());
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      this->HandleProjectSettingScanIpRangesField(update_data, project);
      project_setting.SetScanIpRanges(update_data.GetScanIpRanges());

    } break;

    case ActProjectSettingMember::kProjectStartIp: {
      act_status = this->CheckProjectStartIP(update_data.GetProjectStartIp().GetIpAddress(), project.GetProjectName());
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      project_setting.SetProjectStartIp(update_data.GetProjectStartIp());
    } break;

    case ActProjectSettingMember::kSnmpTrapConfiguration: {
      // Check for SNMPv1 & SNMPv2c
      if (update_data.GetSnmpTrapConfiguration().GetVersion() == ActSnmpVersionEnum::kV2c ||
          update_data.GetSnmpTrapConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
        if (update_data.GetSnmpTrapConfiguration().GetTrapCommunity().length() > 0) {
          act_status =
              this->CheckSnmpTrapConfiguration(update_data.GetSnmpTrapConfiguration(), project.GetProjectName());
          if (!IsActStatusSuccess(act_status)) {
            return act_status;
          }
        }
      }

      // Check for SNMPv3
      if (update_data.GetSnmpTrapConfiguration().GetVersion() == ActSnmpVersionEnum::kV3) {
        QString error_msg = QString("The SNMP Trap Server doesn't support the V3 version.");
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }

      project_setting.SetSnmpTrapConfiguration(update_data.GetSnmpTrapConfiguration());

    } break;

    case ActProjectSettingMember::kMonitorConfiguration: {
      act_status = this->CheckMonitorConfiguration(update_data.GetMonitorConfiguration(), project.GetProjectName());
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      project_setting.SetMonitorConfiguration(update_data.GetMonitorConfiguration());

    } break;

    default:
      QString error_msg = QString("Not support update the %1 of the project setting.")
                              .arg(kActProjectSettingMemberMap.key(project_setting_member));
      qCritical() << __func__ << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
  }

  project.SetProjectSetting(project_setting);

  return act_status;
}

}  // namespace core
}  // namespace act