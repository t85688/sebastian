#include <QSet>
#include <QTimeZone>

#include "act_core.hpp"
#include "act_db.hpp"
#include "act_network_baseline.hpp"
#include "act_service_platform_client_handler.hpp"
#include "act_service_platform_request.hpp"

namespace act {
namespace core {

void GetSystemAssignBaselineName(QString &baseline_name) {
  // FR-10-7: Baseline_<yyyyMMddHHmmss>_UTC<timeZone>, ex: Baseline_20250602085419_UTC+8
  QDateTime now = QDateTime::currentDateTime();
  QTimeZone tz = QTimeZone::systemTimeZone();
  int offset_sec = tz.offsetFromUtc(now);
  // auto tz = now.timeZone();
  // QString tz = now.toTimeZone(now.timeZone()).toString(Qt::ISODate);
  QString ts = now.toString("yyyyMMddHHmmss");
  QString utc_offset = QString("UTC%1%2").arg(offset_sec >= 0 ? "+" : "").arg(offset_sec / 3600);
  baseline_name = QString("Baseline_%1_%2").arg(ts, utc_offset);
  // baseline_name = QString("Baseline_%1_%2").arg(ts);
}

ACT_STATUS GetBaselineDeviceConfigurationFromOfflineConfig(QString &file_path, QString &configuration) {
  ACT_STATUS_INIT();

  QString deviceConfigFilePath = GetDeviceConfigFilePath();
  QDir dir(deviceConfigFilePath);

  // Check folder exist
  if (!dir.exists()) {
    return std::make_shared<ActStatusNotFound>(QString("%1 folder").arg(deviceConfigFilePath));
  }

  QFile file(file_path);
  if (!file.open(QIODevice::ReadOnly)) {
    QString error_msg = QString("Cannot read file: %1").arg(file_path);
    return std::make_shared<ActBadRequest>(error_msg);
  }

  configuration = QString::fromUtf8(file.readAll());

  // Delete the string before "configure terminal"
  int index = configuration.indexOf("configure terminal");
  if (index != -1) {
    configuration = configuration.mid(index);
  }

  file.close();

  return act_status;
}

ACT_STATUS ActCore::ActivateBaselineAtDesignBaselineSet(const qint64 &unactivate_baseline_id,
                                                        const qint64 &activate_baseline_id,
                                                        const QString &activate_user, const quint64 &activate_date) {
  ACT_STATUS_INIT();

  QSet<ActNetworkBaseline> baseline_set = this->GetDesignBaselineSet();

  // Unactivate baseline
  typename QSet<ActNetworkBaseline>::const_iterator iterator;
  iterator = baseline_set.find(ActNetworkBaseline(unactivate_baseline_id));
  if (iterator != baseline_set.end()) {  // found
    ActNetworkBaseline unactivate_baseline = *iterator;
    baseline_set.erase(iterator);
    unactivate_baseline.SetActivate(false);
    baseline_set.insert(unactivate_baseline);

    this->SetDesignBaselineSet(baseline_set);
    // Write to db
    act_status = act::database::networkbaseline::WriteData(ActBaselineModeEnum::kDesign, unactivate_baseline);
    // Send update msg
    ActSimpleDesignBaselinePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, unactivate_baseline.GetProjectId(),
                                                 unactivate_baseline, true);
    this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, unactivate_baseline.GetProjectId());
  }

  // Activate baseline
  iterator = baseline_set.find(ActNetworkBaseline(activate_baseline_id));
  if (iterator != baseline_set.end()) {  // found
    ActNetworkBaseline activate_baseline = *iterator;
    baseline_set.erase(iterator);
    activate_baseline.SetActivate(true);
    activate_baseline.SetActivatedUser(activate_user);
    activate_baseline.SetActivatedDate(activate_date);
    baseline_set.insert(activate_baseline);

    this->SetDesignBaselineSet(baseline_set);
    // Write to db
    act_status = act::database::networkbaseline::WriteData(ActBaselineModeEnum::kDesign, activate_baseline);
    // Send update msg
    ActSimpleDesignBaselinePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, activate_baseline.GetProjectId(),
                                                 activate_baseline, true);
    this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, activate_baseline.GetProjectId());

  } else {  // not found
    QString error_msg = QString("Baseline id %1").arg(activate_baseline_id);
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  return act_status;
}

ACT_STATUS ActCore::ActivateBaselineAtOperationBaselineSet(const qint64 &unactivate_baseline_id,
                                                           const qint64 &activate_baseline_id,
                                                           const QString &activate_user, const quint64 &activate_date) {
  ACT_STATUS_INIT();

  QSet<ActNetworkBaseline> baseline_set = this->GetOperationBaselineSet();

  // Unactivate baseline
  typename QSet<ActNetworkBaseline>::const_iterator iterator;
  iterator = baseline_set.find(ActNetworkBaseline(unactivate_baseline_id));
  if (iterator != baseline_set.end()) {  // found
    ActNetworkBaseline unactivate_baseline = *iterator;
    baseline_set.erase(iterator);
    unactivate_baseline.SetActivate(false);
    baseline_set.insert(unactivate_baseline);

    this->SetOperationBaselineSet(baseline_set);
    // Write to db
    act_status = act::database::networkbaseline::WriteData(ActBaselineModeEnum::kOperation, unactivate_baseline);
    // Send update msg
    ActSimpleOperationBaselinePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate,
                                                    unactivate_baseline.GetProjectId(), unactivate_baseline, true);
    this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, unactivate_baseline.GetProjectId());
  }

  // Activate baseline
  iterator = baseline_set.find(ActNetworkBaseline(activate_baseline_id));
  if (iterator != baseline_set.end()) {  // found
    ActNetworkBaseline activate_baseline = *iterator;
    baseline_set.erase(iterator);
    activate_baseline.SetActivate(true);
    activate_baseline.SetActivatedUser(activate_user);
    activate_baseline.SetActivatedDate(activate_date);
    baseline_set.insert(activate_baseline);

    this->SetOperationBaselineSet(baseline_set);
    // Write to db
    act_status = act::database::networkbaseline::WriteData(ActBaselineModeEnum::kOperation, activate_baseline);
    // Send update msg
    ActSimpleOperationBaselinePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, activate_baseline.GetProjectId(),
                                                    activate_baseline, true);
    this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, activate_baseline.GetProjectId());
  } else {  // not found
    QString error_msg = QString("Baseline id %1").arg(activate_baseline_id);
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  return act_status;
}

ACT_STATUS ActCore::GetDesignBaselineList(qint64 &project_id, ActNetworkBaselineList &baseline_list) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActSimpleNetworkBaseline> simple_nb_list;
  for (auto baseline_id : project.GetDesignBaselineIds()) {
    ActNetworkBaseline baseline;
    act_status = ActGetItemById<ActNetworkBaseline>(this->GetDesignBaselineSet(), baseline_id, baseline);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << __func__ << QString("Not found the Design Baseline id %1").arg(baseline_id);
    } else {
      simple_nb_list.append(ActSimpleNetworkBaseline(baseline));
    }
  }

  // // Append DB NetworkBaselines
  // for (auto baseline : this->GetDesignBaselineSet()) {
  //   if (baseline.GetProjectId() == project_id) {
  //     simple_nb_list.append(ActSimpleNetworkBaseline(baseline));
  //   }
  // }

  // sort by date (from small to large)
  std::sort(
      simple_nb_list.begin(), simple_nb_list.end(),
      [](const ActSimpleNetworkBaseline &x, const ActSimpleNetworkBaseline &y) { return x.GetDate() < y.GetDate(); });

  baseline_list.SetNetworkBaselineList(simple_nb_list);
  return act_status;
}

ACT_STATUS ActCore::GetOperationBaselineList(qint64 &project_id, ActNetworkBaselineList &baseline_list) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActSimpleNetworkBaseline> simple_nb_list;

  // Append CURRENT NetworkBaseline
  ActSimpleNetworkBaseline current_simple_nb;
  current_simple_nb.SetId(-1);
  current_simple_nb.SetName(ACT_NETWORK_BASELINE_CURRENT_NAME);
  current_simple_nb.SetDate(0);
  current_simple_nb.SetCreatedUser("");
  current_simple_nb.SetProjectId(project_id);
  simple_nb_list.append(current_simple_nb);

  // Append DB NetworkBaselines
  for (auto baseline_id : project.GetOperationBaselineIds()) {
    ActNetworkBaseline baseline;
    act_status = ActGetItemById<ActNetworkBaseline>(this->GetOperationBaselineSet(), baseline_id, baseline);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << __func__ << QString("Not found the Operation Baseline id %1").arg(baseline_id);
    } else {
      simple_nb_list.append(ActSimpleNetworkBaseline(baseline));
    }
  }

  // for (auto baseline : this->GetOperationBaselineSet()) {
  //   if (baseline.GetProjectId() == project_id) {
  //     simple_nb_list.append(ActSimpleNetworkBaseline(baseline));
  //   }
  // }

  // sort by date (from small to large)
  std::sort(
      simple_nb_list.begin(), simple_nb_list.end(),
      [](const ActSimpleNetworkBaseline &x, const ActSimpleNetworkBaseline &y) { return x.GetDate() < y.GetDate(); });

  baseline_list.SetNetworkBaselineList(simple_nb_list);
  return act_status;
}

ACT_STATUS ActCore::GetActivateOperationSimpleBaseline(qint64 &project_id, ActSimpleNetworkBaseline &simple_baseline) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Get Activate Baseline
  ActNetworkBaseline baseline;
  act_status =
      ActGetItemById<ActNetworkBaseline>(this->GetOperationBaselineSet(), project.GetActivateBaselineId(), baseline);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("Operation Baseline id %1").arg(project.GetActivateBaselineId());
    return std::make_shared<ActStatusNotFound>(error_msg);
  }
  simple_baseline = ActSimpleNetworkBaseline(baseline);

  return act_status;
}

ACT_STATUS ActCore::GetActivateOperationBaselineProject(qint64 &project_id, ActProject &baseline_project) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Get Activate Baseline
  ActNetworkBaseline baseline;
  act_status =
      ActGetItemById<ActNetworkBaseline>(this->GetOperationBaselineSet(), project.GetActivateBaselineId(), baseline);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("Operation Baseline id %1").arg(project.GetActivateBaselineId());
    return std::make_shared<ActStatusNotFound>(error_msg);
  }
  baseline_project = baseline.GetProject();

  return act_status;
}

ACT_STATUS ActCore::GetDesignBaselineProject(qint64 &project_id, const qint64 &baseline_id,
                                             ActProject &baseline_project) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Get Activate Baseline
  ActNetworkBaseline baseline;
  act_status = ActGetItemById<ActNetworkBaseline>(this->GetDesignBaselineSet(), baseline_id, baseline);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("Design Baseline id %1").arg(baseline_id);
    return std::make_shared<ActStatusNotFound>(error_msg);
  }
  baseline_project = baseline.GetProject();

  return act_status;
}

ACT_STATUS ActCore::CreateDesignBaseline(qint64 &project_id, const ActNetworkBaselineInfo &baseline_info,
                                         const qint64 &created_user_id, ActNetworkBaseline &baseline) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Get User
  ActUser user;
  act_status = this->GetUser(qint64(created_user_id), user);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get user failed with create user id:" << created_user_id;
    return act_status;
  }

  auto baseline_name = baseline_info.GetName();
  if (baseline_name.isEmpty()) {  // assign system defined baseline_name
    GetSystemAssignBaselineName(baseline_name);
  }

  // Create baseline
  baseline.SetName(baseline_name);
  quint64 current_timestamp = QDateTime::currentSecsSinceEpoch();
  baseline.SetDate(current_timestamp);
  baseline.SetCreatedUser(user.GetUsername());
  baseline.SetProjectId(project_id);
  baseline.SetDescription(baseline_info.GetDescription());
  baseline.SetMode(ActBaselineModeEnum::kDesign);
  auto baseline_project = project;
  baseline_project.GetDesignBaselineIds().clear();
  baseline.SetProject(baseline_project);
  baseline.GetDevices().clear();

  // Check baseline
  act_status = CheckDesignBaseline(project_id, baseline);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << __func__ << "Check Baseline failed";
    return act_status;
  }

  QSet<ActNetworkBaseline> baseline_set = this->GetDesignBaselineSet();

  // Generate a new unique id
  qint64 id;
  act_status = this->GenerateUniqueId<ActNetworkBaseline>(baseline_set, this->last_assigned_design_baseline_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << __func__ << "Cannot get an available unique id";
    return act_status;
  }
  baseline.SetId(id);

  baseline.SetDataVersion(ACT_BASELINE_DATA_VERSION);

  // Insert the NetworkBaseline to core set
  baseline_set.insert(baseline);
  this->SetDesignBaselineSet(baseline_set);

  // Write to db
  act_status = act::database::networkbaseline::WriteData(ActBaselineModeEnum::kDesign, baseline);

  // Add the baseline_id to project
  project.GetDesignBaselineIds().insert(id);
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
  }

  // Send update msg
  ActSimpleDesignBaselinePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kCreate, project_id, baseline, true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return act_status;
}

ACT_STATUS ActCore::CopyDesignBaselineToOperation(ActProject &project, const qint64 &design_baseline_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get DesignBaseline
  ActNetworkBaseline design_baseline;
  act_status = ActGetItemById<ActNetworkBaseline>(this->GetDesignBaselineSet(), design_baseline_id, design_baseline);
  if (!IsActStatusSuccess(act_status)) {
    qWarning() << __func__ << QString("Not found the Design Baseline id %1").arg(design_baseline_id);
  }

  QSet<ActNetworkBaseline> baseline_set = this->GetOperationBaselineSet();

  // Create baseline
  auto baseline_id = design_baseline.GetId();
  auto operation_baseline = design_baseline;
  operation_baseline.SetMode(ActBaselineModeEnum::kOperation);
  auto baseline_project = project;
  baseline_project.GetDesignBaselineIds().clear();
  baseline_project.SetProjectMode(ActProjectModeEnum::kOperation);
  operation_baseline.SetProject(baseline_project);
  operation_baseline.GetDevices().clear();

  // Check the item does exists by baseline_id
  typename QSet<ActNetworkBaseline>::const_iterator iterator;
  iterator = baseline_set.find(ActNetworkBaseline(baseline_id));
  if (iterator != baseline_set.end()) {  // if exists would remove old baseline
    act_status = DeleteOperationBaseline(project, baseline_id);
    if (!IsActStatusSuccess(act_status)) {
      QString error_msg = QString("DeleteOperationBaseline() failed(baseline_id: %1)").arg(baseline_id);
      return act_status;
    }
  }

  // Insert the NetworkBaseline to core set
  baseline_set.insert(operation_baseline);
  this->SetOperationBaselineSet(baseline_set);

  // Write to db
  act_status = act::database::networkbaseline::WriteData(ActBaselineModeEnum::kOperation, operation_baseline);

  // Add the baseline_id to project
  project.GetOperationBaselineIds().insert(baseline_id);
  // act_status = this->UpdateProject(project);
  // if (!IsActStatusSuccess(act_status)) {
  //   qCritical() << project.GetProjectName() << "Cannot update the project";
  // }

  // Send update msg
  ActSimpleOperationBaselinePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kCreate, project.GetId(),
                                                  operation_baseline, true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project.GetId());

  return act_status;
}

ACT_STATUS ActCore::ActivateDesignBaseline(qint64 &project_id, const qint64 &activate_baseline_id,
                                           const qint64 &activate_user_id) {
  ACT_STATUS_INIT();
  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }
  // Get User
  ActUser user;
  act_status = this->GetUser(qint64(activate_user_id), user);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get user failed with activate user id:" << activate_user_id;
    return act_status;
  }

  // Activate flow:
  // 1. Activate Design Baseline, and unactivate the other baseline
  // 2. Copy Design Baseline to Operation Baseline set (if operation exists would overwrite)
  // 3. Activate the Operation Baseline, and unactivate the other baseline

  // Activate Design Baseline
  quint64 activated_date = QDateTime::currentSecsSinceEpoch();
  qint64 unactivate_baseline_id = project.GetActivateBaselineId();
  QString user_name = user.GetUsername();
  act_status =
      ActivateBaselineAtDesignBaselineSet(unactivate_baseline_id, activate_baseline_id, user_name, activated_date);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  //  Copy Design Baseline to Operation Baseline set (overwrite)
  act_status = CopyDesignBaselineToOperation(project, activate_baseline_id);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg =
        QString("CopyDesignBaselineToOperation() failed(activate_baseline_id: %1)").arg(activate_baseline_id);
    return act_status;
  }

  // Activate the Operation Baseline
  act_status =
      ActivateBaselineAtOperationBaselineSet(unactivate_baseline_id, activate_baseline_id, user_name, activated_date);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Update Activate ID to project
  project.SetActivateBaselineId(activate_baseline_id);

  // Update Project
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
  }

  return act_status;
}

ACT_STATUS ActCore::RegisterDesignBaseline(qint64 &project_id, const qint64 &baseline_id,
                                           const qint64 &register_user_id) {
  ACT_STATUS_INIT();
  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Create the baseline set
  ActNetworkBaseline baseline;
  act_status = ActGetItemById<ActNetworkBaseline>(this->GetDesignBaselineSet(), baseline_id, baseline);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("Design Baseline id %1").arg(baseline_id);
    return std::make_shared<ActStatusNotFound>(error_msg);
  }
  QSet<ActNetworkBaseline> baseline_set = {baseline};

  // Get the ServicePlatform info
  QString service_platform_endpoint = GetServicePlatformEndpointFromEnviron();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::servicePlatformClient::ActServicePlatformClient service_platform_client;
  ActServicePlatformRegisterResponse register_res;

  // Register
  act_status = service_platform_client.Register(service_platform_endpoint, http_proxy_endpoint, register_user_id,
                                                project.GetPlatformProjectId(), project.GetProjectName(), baseline_set,
                                                register_res);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Register() failed.";
    // If the status is 401, which means the token has expired or not exist, we need to notify the user re-login
    if (act_status->GetStatus() == ActStatusType::kUnauthorized) {
      act_status->SetStatus(ActStatusType::kServicePlatformUnauthorized);
      act_status->SetStatusCode(static_cast<qint64>(ActStatusType::kServicePlatformUnauthorized));
      act_status->SetSeverity(ActSeverity::kCritical);
      act_status->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kServicePlatformUnauthorized));

      this->DeleteServicePlatformToken(register_user_id);
    }
    return act_status;
  }

  project.SetPlatformProjectId(register_res.GetId());

  act_status = UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "UpdateProject() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::RollbackDesignBaseline(qint64 &project_id, const qint64 &baseline_id) {
  ACT_STATUS_INIT();
  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Get the baseline
  ActNetworkBaseline baseline;
  act_status = ActGetItemById<ActNetworkBaseline>(this->GetDesignBaselineSet(), baseline_id, baseline);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("Design Baseline id %1").arg(baseline_id);
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  // Rollback
  auto rollback_project = baseline.GetProject();
  rollback_project.SetDesignBaselineIds(project.GetDesignBaselineIds());
  act_status = this->UpdateProject(rollback_project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
  }

  return act_status;
}

ACT_STATUS ActCore::GetDesignBaseline(qint64 &project_id, qint64 &baseline_id, ActNetworkBaseline &baseline) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = ActGetItemById<ActNetworkBaseline>(this->GetDesignBaselineSet(), baseline_id, baseline);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("Design Baseline id %1").arg(baseline_id);
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  // qDebug() << __func__ << "baseline::" << baseline.ToString().toStdString().c_str();

  return act_status;
}

ACT_STATUS ActCore::GetOperationBaseline(qint64 &project_id, qint64 &baseline_id, ActNetworkBaseline &baseline) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Handle CURRENT(id == -1)
  if (baseline_id == -1) {
    baseline.SetId(-1);
    baseline.SetDate(0);
    baseline.SetCreatedUser("");
    baseline.SetName(ACT_NETWORK_BASELINE_CURRENT_NAME);
    baseline.SetProjectId(project_id);
    baseline.SetProject(project);
    baseline.GetDevices().clear();
  } else {
    act_status = ActGetItemById<ActNetworkBaseline>(this->GetOperationBaselineSet(), baseline_id, baseline);
    if (!IsActStatusSuccess(act_status)) {
      QString error_msg = QString("Operation Baseline id %1").arg(baseline_id);
      return std::make_shared<ActStatusNotFound>(error_msg);
    }
  }

  // qDebug() << __func__ << "baseline::" << baseline.ToString().toStdString().c_str();

  return act_status;
}

ACT_STATUS ActCore::UpdateDesignBaseline(qint64 &project_id, ActNetworkBaseline &baseline) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();
  // qDebug() << __func__ << "Update baseline::" << baseline.ToString().toStdString().c_str();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  if (baseline.GetId() == -1) {  // CURRENT
    QString error_msg = QString("Cannot Update the CURRENT Network Baseline(id: %1)").arg(baseline.GetId());
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (baseline.GetName().isEmpty()) {  // assign system defined baseline_name
    GetSystemAssignBaselineName(baseline.GetName());
  }

  act_status = this->CheckDesignBaseline(project_id, baseline);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Check Baseline failed with baseline id:" << baseline.GetId();
    return act_status;
  }
  QSet<ActNetworkBaseline> baseline_set = this->GetDesignBaselineSet();

  // Check the item does exist by baseline id
  typename QSet<ActNetworkBaseline>::const_iterator iterator;
  QString old_name = "";
  bool found_old_nb = false;
  iterator = baseline_set.find(baseline);
  if (iterator != baseline_set.end()) {  // found
    old_name = iterator->GetName();
    found_old_nb = true;
    baseline_set.erase(iterator);
  }

  baseline.SetDataVersion(ACT_BASELINE_DATA_VERSION);

  // Insert the baseline to core set
  baseline_set.insert(baseline);
  this->SetDesignBaselineSet(baseline_set);

  // Write to db
  if (found_old_nb && (old_name != baseline.GetName())) {  // Update db baseline name
    act_status = act::database::networkbaseline::UpdateNetworkBaselineFileName(
        ActBaselineModeEnum::kDesign, baseline.GetId(), old_name, baseline.GetName());
    qCritical() << __func__ << "UpdateNetworkBaselineFileName(Design) failed with baseline id:" << baseline.GetId();
    return act_status;
  }
  act_status = act::database::networkbaseline::WriteData(ActBaselineModeEnum::kDesign, baseline);

  // Send update msg
  ActSimpleDesignBaselinePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, baseline, true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateOperationBaseline(qint64 &project_id, ActNetworkBaseline &baseline) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();
  // qDebug() << __func__ << "Update baseline::" << baseline.ToString().toStdString().c_str();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  if (baseline.GetId() == -1) {  // CURRENT
    QString error_msg = QString("Cannot Update the CURRENT Network Baseline(id: %1)").arg(baseline.GetId());
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (baseline.GetName().isEmpty()) {  // assign system defined baseline_name
    GetSystemAssignBaselineName(baseline.GetName());
  }

  act_status = this->CheckDesignBaseline(project_id, baseline);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Check Baseline failed with baseline id:" << baseline.GetId();
    return act_status;
  }
  QSet<ActNetworkBaseline> baseline_set = this->GetDesignBaselineSet();

  // Check the item does exist by baseline id
  typename QSet<ActNetworkBaseline>::const_iterator iterator;
  QString old_name = "";
  bool found_old_nb = false;
  iterator = baseline_set.find(baseline);
  if (iterator != baseline_set.end()) {  // found
    old_name = iterator->GetName();
    found_old_nb = true;
    baseline_set.erase(iterator);
  }

  baseline.SetDataVersion(ACT_BASELINE_DATA_VERSION);

  // Insert the baseline to core set
  baseline_set.insert(baseline);
  this->SetOperationBaselineSet(baseline_set);

  // Write to db
  if (found_old_nb && (old_name != baseline.GetName())) {  // Update db device name
    act_status = act::database::networkbaseline::UpdateNetworkBaselineFileName(
        ActBaselineModeEnum::kOperation, baseline.GetId(), old_name, baseline.GetName());
    qCritical() << __func__ << "UpdateNetworkBaselineFileName(Operation) failed with baseline id:" << baseline.GetId();
    return act_status;
  }
  act_status = act::database::networkbaseline::WriteData(ActBaselineModeEnum::kOperation, baseline);

  // Send update msg
  ActSimpleOperationBaselinePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, baseline, true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return act_status;
}

ACT_STATUS ActCore::CheckDesignBaseline(const qint64 &project_id, const ActNetworkBaseline &baseline) {
  ACT_STATUS_INIT();

  // Check the name length should be 1 ~ 64 characters.
  if (baseline.GetName().length() < ACT_NETWORK_BASELINE_NAME_LENGTH_MIN ||
      baseline.GetName().length() > ACT_NETWORK_BASELINE_NAME_LENGTH_MAX) {
    QString error_msg = QString("The length of name(%1) should be %2 ~ %3 characters")
                            .arg(baseline.GetName())
                            .arg(ACT_NETWORK_BASELINE_NAME_LENGTH_MIN)
                            .arg(ACT_NETWORK_BASELINE_NAME_LENGTH_MAX);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  QSet<ActNetworkBaseline> baseline_set = this->GetDesignBaselineSet();

  // Check the DesignBaseline name size
  if (ACT_NETWORK_BASELINE_SIZE <= baseline_set.size()) {
    QString error_msg = QString("The DesignBaseline size exceeds the limit: %1").arg(ACT_NETWORK_BASELINE_SIZE);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActLicenseSizeFailedRequest>("DesignBaseline size", ACT_NETWORK_BASELINE_SIZE);
  }

  // Check the name does not exist
  for (auto db_nb : baseline_set) {
    // Same project & different DesignBaseline
    if ((db_nb.GetProjectId() == project_id) && (db_nb.GetId() != baseline.GetId())) {
      if (db_nb.GetName() == baseline.GetName()) {
        qCritical() << __func__ << "The name(" << baseline.GetName() << ")is duplicated";
        return std::make_shared<ActDuplicatedError>(baseline.GetName());
      }
    }
  }

  return act_status;
}

ACT_STATUS ActCore::CheckOperationBaseline(const qint64 &project_id, const ActNetworkBaseline &baseline) {
  ACT_STATUS_INIT();

  // Check the name length should be 1 ~ 64 characters.
  if (baseline.GetName().length() < ACT_NETWORK_BASELINE_NAME_LENGTH_MIN ||
      baseline.GetName().length() > ACT_NETWORK_BASELINE_NAME_LENGTH_MAX) {
    QString error_msg = QString("The length of name(%1) should be %2 ~ %3 characters")
                            .arg(baseline.GetName())
                            .arg(ACT_NETWORK_BASELINE_NAME_LENGTH_MIN)
                            .arg(ACT_NETWORK_BASELINE_NAME_LENGTH_MAX);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  QSet<ActNetworkBaseline> baseline_set = this->GetOperationBaselineSet();

  // Check the OperationBaseline name size
  if (ACT_NETWORK_BASELINE_SIZE <= baseline_set.size()) {
    QString error_msg = QString("The OperationBaseline size exceeds the limit: %1").arg(ACT_NETWORK_BASELINE_SIZE);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActLicenseSizeFailedRequest>("OperationBaseline size", ACT_NETWORK_BASELINE_SIZE);
  }

  // Check the name does not exist
  for (auto db_nb : baseline_set) {
    // Same project & different OperationBaseline
    if ((db_nb.GetProjectId() == project_id) && (db_nb.GetId() != baseline.GetId())) {
      if (db_nb.GetName() == baseline.GetName()) {
        qCritical() << __func__ << "The name(" << baseline.GetName() << ")is duplicated";
        return std::make_shared<ActDuplicatedError>(baseline.GetName());
      }
    }
  }

  return act_status;
}

ACT_STATUS ActCore::GetDesignBaselineBOMDetail(qint64 &project_id, qint64 &baseline_id,
                                               ActBaselineBOMDetail &baseline_bom_detail) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Find Baseline
  ActNetworkBaseline baseline;
  act_status = ActGetItemById<ActNetworkBaseline>(this->GetDesignBaselineSet(), baseline_id, baseline);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("Design Baseline id %1").arg(baseline_id);
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  baseline_bom_detail = ActBaselineBOMDetail(baseline);

  return act_status;
}

ACT_STATUS ActCore::GetOperationBaselineBOMDetail(qint64 &project_id, qint64 &baseline_id,
                                                  ActBaselineBOMDetail &baseline_bom_detail) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Find Baseline
  ActNetworkBaseline baseline;

  // Handle CURRENT(id == -1)
  if (baseline_id == -1) {
    baseline.SetId(-1);
    baseline.SetDate(0);
    baseline.SetCreatedUser("");
    baseline.SetName(ACT_NETWORK_BASELINE_CURRENT_NAME);
    baseline.SetProjectId(project_id);
    baseline.SetProject(project);
    baseline.GetDevices().clear();
  } else {
    act_status = ActGetItemById<ActNetworkBaseline>(this->GetOperationBaselineSet(), baseline_id, baseline);
    if (!IsActStatusSuccess(act_status)) {
      QString error_msg = QString("Operation Baseline id %1").arg(baseline_id);
      return std::make_shared<ActStatusNotFound>(error_msg);
    }
  }

  baseline_bom_detail = ActBaselineBOMDetail(baseline);

  return act_status;
}

ACT_STATUS ActCore::GetDesignBaselineWithDevices(qint64 &project_id, qint64 &baseline_id,
                                                 ActNetworkBaseline &baseline) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject tmp_project;
  act_status = this->GetProject(project_id, tmp_project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->GetDesignBaseline(project_id, baseline_id, baseline);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }
  ActProject project = baseline.GetProject();
  baseline.GetDevices().clear();

  // Generate the NetworkBaselineDevices
  QList<qint64> dev_id_list;
  // Generate the all can deploy device
  for (auto dev : project.GetDevices()) {
    if (dev.CheckCanDeploy()) {
      dev_id_list.append(dev.GetId());
    } else {
      // Add the empty configuration device
      ActNetworkBaselineDevice baseline_dev(dev.GetId(), dev.GetIpv4().GetIpAddress(),
                                            dev.GetDeviceProperty().GetModelName(), dev.GetFirmwareVersion());
      baseline.GetDevices().insert(baseline_dev);
    }
  }

  // Add the Device with the configuration
  // Clear DeviceConfig tmp folder
  act_status = ClearFolder(GetDeviceConfigFilePath());
  // kene-
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Generate the ini file
  ActDeviceOfflineConfigFileMap device_offline_config_file_map;
  act_status = GenerateDeviceIniConfigFile(project, dev_id_list, device_offline_config_file_map);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("Baseline - %1, %2").arg(baseline.GetName()).arg(act_status->GetErrorMessage());
    return std::make_shared<ActStatusInternalError>(error_msg);
  }
  qDebug() << __func__
           << "device_offline_config_file_map:" << device_offline_config_file_map.ToString().toStdString().c_str();

  // Check map[deviceId] has value (generated offline config)
  act_status = CheckDeviceConfigValuesNotEmpty(project, dev_id_list, device_offline_config_file_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Export and Unzip All device config to Tmp Folder
  ActDeviceOfflineConfigFileMap device_offline_config_ini_file_path_map;
  act_status = ExportAndUnzipConfigFile(project, dev_id_list, device_offline_config_ini_file_path_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  for (auto dev : project.GetDevices()) {
    if (!dev_id_list.contains(dev.GetId())) {
      continue;
    }
    ActNetworkBaselineDevice baseline_dev(dev.GetId(), dev.GetIpv4().GetIpAddress(),
                                          dev.GetDeviceProperty().GetModelName(), dev.GetFirmwareVersion());
    // Generate the device configuration
    QString configuration;
    act_status = GetBaselineDeviceConfigurationFromOfflineConfig(
        device_offline_config_ini_file_path_map.GetDeviceOfflineConfigFileMap()[dev.GetId()], configuration);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    baseline_dev.SetConfiguration(configuration);
    baseline.GetDevices().insert(baseline_dev);
  }

  // qDebug() << __func__ << "baseline::" << baseline.ToString().toStdString().c_str();

  return act_status;
}

ACT_STATUS ActCore::GetOperationBaselineWithDevices(qint64 &project_id, qint64 &baseline_id,
                                                    ActNetworkBaseline &baseline) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject tmp_project;
  act_status = this->GetProject(project_id, tmp_project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActProject project;
  // Handle CURRENT(id == -1)
  if (baseline_id == -1) {
    project = tmp_project;
    baseline.SetId(-1);
    baseline.SetDate(0);
    baseline.SetCreatedUser("");
    baseline.SetName(ACT_NETWORK_BASELINE_CURRENT_NAME);
    baseline.SetProjectId(project_id);
    baseline.SetProject(project);
    baseline.GetDevices().clear();
  } else {
    act_status = this->GetOperationBaseline(project_id, baseline_id, baseline);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
    project = baseline.GetProject();
    baseline.GetDevices().clear();
  }

  // Generate the NetworkBaselineDevices
  QList<qint64> dev_id_list;
  // Generate the all can deploy device
  for (auto dev : project.GetDevices()) {
    if (dev.CheckCanDeploy()) {
      dev_id_list.append(dev.GetId());
    } else {
      // Add the empty configuration device
      ActNetworkBaselineDevice baseline_dev(dev.GetId(), dev.GetIpv4().GetIpAddress(),
                                            dev.GetDeviceProperty().GetModelName(), dev.GetFirmwareVersion());
      baseline.GetDevices().insert(baseline_dev);
    }
  }

  // Add the Device with the configuration
  // Clear DeviceConfig tmp folder
  act_status = ClearFolder(GetDeviceConfigFilePath());
  // kene-
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Generate the ini file
  ActDeviceOfflineConfigFileMap device_offline_config_file_map;
  act_status = GenerateDeviceIniConfigFile(project, dev_id_list, device_offline_config_file_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Check map[deviceId] has value (generated offline config)
  act_status = CheckDeviceConfigValuesNotEmpty(project, dev_id_list, device_offline_config_file_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Export and Unzip All device config to Tmp Folder
  ActDeviceOfflineConfigFileMap device_offline_config_ini_file_path_map;
  act_status = ExportAndUnzipConfigFile(project, dev_id_list, device_offline_config_ini_file_path_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  for (auto dev : project.GetDevices()) {
    if (!dev_id_list.contains(dev.GetId())) {
      continue;
    }
    ActNetworkBaselineDevice baseline_dev(dev.GetId(), dev.GetIpv4().GetIpAddress(),
                                          dev.GetDeviceProperty().GetModelName(), dev.GetFirmwareVersion());
    // Generate the device configuration
    QString configuration;
    act_status = GetBaselineDeviceConfigurationFromOfflineConfig(
        device_offline_config_ini_file_path_map.GetDeviceOfflineConfigFileMap()[dev.GetId()], configuration);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    baseline_dev.SetConfiguration(configuration);
    baseline.GetDevices().insert(baseline_dev);
  }

  // qDebug() << __func__ << "baseline::" << baseline.ToString().toStdString().c_str();

  return act_status;
}

ACT_STATUS ActCore::DeleteDesignAndOperationBaseline(qint64 &project_id, qint64 &baseline_id) {
  ACT_STATUS_INIT();
  // Delete Design and Operation(if exists) Baseline

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Design Baseline
  act_status = DeleteDesignBaseline(project, baseline_id);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Operation Baseline
  if (project.GetOperationBaselineIds().contains(baseline_id)) {
    act_status = DeleteOperationBaseline(project, baseline_id);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Update Project
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::DeleteDesignBaseline(qint64 &project_id, qint64 &baseline_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = DeleteDesignBaseline(project, baseline_id);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Update Project
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::DeleteDesignBaseline(ActProject &project, qint64 &baseline_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QSet<ActNetworkBaseline> baseline_set = this->GetDesignBaselineSet();

  if (baseline_id == -1) {  // CURRENT
    QString error_msg = QString("Cannot Delete the CURRENT Baseline(id: %1)").arg(baseline_id);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check the item does exist by baseline_id
  typename QSet<ActNetworkBaseline>::const_iterator iterator;
  iterator = baseline_set.find(ActNetworkBaseline(baseline_id));
  if (iterator == baseline_set.end()) {
    QString error_msg = QString("Delete Baseline failed, cannot found baseline id %1").arg(baseline_id);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  ActNetworkBaseline baseline = (*iterator);

  // Check baseline's project.GetId()
  if (baseline.GetProjectId() != project.GetId()) {
    QString error_msg = QString("Delete Baseline(%1) failed, it does not belong to this project(%2)")
                            .arg(baseline_id)
                            .arg(project.GetId());
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  QString baseline_name = baseline.GetName();

  // Delete it
  baseline_set.erase(iterator);
  this->SetDesignBaselineSet(baseline_set);

  // Write to db
  act_status =
      act::database::networkbaseline::DeleteBaselineFile(ActBaselineModeEnum::kDesign, baseline_id, baseline_name);

  // Send update msg
  ActSimpleDesignBaselinePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kDelete, project.GetId(), baseline, true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project.GetId());

  // Update Project Ids
  project.GetDesignBaselineIds().remove(baseline_id);

  return act_status;
}

ACT_STATUS ActCore::DeleteOperationBaseline(qint64 &project_id, qint64 &baseline_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = DeleteOperationBaseline(project, baseline_id);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Update Project
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::DeleteOperationBaseline(ActProject &project, qint64 &baseline_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QSet<ActNetworkBaseline> baseline_set = this->GetOperationBaselineSet();

  if (baseline_id == -1) {  // CURRENT
    QString error_msg = QString("Cannot Delete the CURRENT Baseline(id: %1)").arg(baseline_id);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check the item does exist by baseline_id
  typename QSet<ActNetworkBaseline>::const_iterator iterator;
  iterator = baseline_set.find(ActNetworkBaseline(baseline_id));
  if (iterator == baseline_set.end()) {
    QString error_msg = QString("Delete Baseline failed, cannot found baseline id %1").arg(baseline_id);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  ActNetworkBaseline baseline = (*iterator);

  // Check baseline's project_id
  if (baseline.GetProjectId() != project.GetId()) {
    QString error_msg = QString("Delete Baseline(%1) failed, it does not belong to this project(%2)")
                            .arg(baseline_id)
                            .arg(project.GetId());
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  QString baseline_name = baseline.GetName();

  // Delete it
  baseline_set.erase(iterator);
  this->SetOperationBaselineSet(baseline_set);

  // Write to db
  act_status =
      act::database::networkbaseline::DeleteBaselineFile(ActBaselineModeEnum::kOperation, baseline_id, baseline_name);

  // Send update msg
  ActSimpleOperationBaselinePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kDelete, project.GetId(), baseline, true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project.GetId());

  // Update Project Ids
  project.GetOperationBaselineIds().remove(baseline_id);

  return act_status;
}

ACT_STATUS ActCore::DeleteProjectAllBaselines(qint64 &project_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Delete Design Baselines
  QSet<ActNetworkBaseline> design_baseline_set = this->GetDesignBaselineSet();
  for (auto iterator = design_baseline_set.begin(); iterator != design_baseline_set.end();) {
    if (iterator->GetProjectId() == project_id) {
      ActNetworkBaseline baseline = *iterator;
      // Handle DB
      act_status = act::database::networkbaseline::DeleteBaselineFile(ActBaselineModeEnum::kDesign, baseline.GetId(),
                                                                      baseline.GetName());

      iterator = design_baseline_set.erase(iterator);  // delete & move to next
    } else {
      ++iterator;  // move to next
    }
  }
  this->SetDesignBaselineSet(design_baseline_set);

  // Delete Operation Baselines
  QSet<ActNetworkBaseline> operation_baseline_set = this->GetOperationBaselineSet();
  for (auto iterator = operation_baseline_set.begin(); iterator != operation_baseline_set.end();) {
    if (iterator->GetProjectId() == project_id) {
      ActNetworkBaseline baseline = *iterator;
      // Handle DB
      act_status = act::database::networkbaseline::DeleteBaselineFile(ActBaselineModeEnum::kOperation, baseline.GetId(),
                                                                      baseline.GetName());

      iterator = operation_baseline_set.erase(iterator);  // delete & move to next
    } else {
      ++iterator;  // move to next
    }
  }
  this->SetOperationBaselineSet(operation_baseline_set);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::CreateDesignBaselineAndActivate(qint64 &project_id, const ActNetworkBaselineInfo &baseline_info,
                                                    const qint64 &created_user_id, ActNetworkBaseline &baseline) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Create DesignBaseline
  act_status = CreateDesignBaseline(project_id, baseline_info, created_user_id, baseline);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Activate Design Baseline(include copy Operation baseline & activate)
  act_status = ActivateDesignBaseline(project_id, baseline.GetId(), created_user_id);
  if (!IsActStatusSuccess(act_status)) {
    auto delete_status = DeleteDesignAndOperationBaseline(project_id, baseline.GetId());
    if (!IsActStatusSuccess(delete_status)) {
      return delete_status;
    }
    return act_status;
  }

  // Get Baseline from DB
  act_status = GetDesignBaseline(project_id, baseline.GetId(), baseline);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }
  return act_status;
}

ACT_STATUS ActCore::CheckDesignBaselineProjectDiffWithProject(qint64 &project_id, const qint64 &baseline_id,
                                                              ActBaselineProjectDiffReport &diff_report) {
  ACT_STATUS_INIT();

  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Get Design Baseline Project
  ActProject baseline_project;
  act_status = GetDesignBaselineProject(project_id, baseline_id, baseline_project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << QString("Not found the Design Baseline Project. Baseline id %1").arg(baseline_id);
    return act_status;
  }

  diff_report.SetId(baseline_id);  // baselineID
  diff_report.SetProjectId(project_id);
  diff_report.SetHasDiff(false);

  //  BOM
  if (project.ToString("SkuQuantitiesMap") != baseline_project.ToString("SkuQuantitiesMap")) {  // diff
    diff_report.GetDiffDetail().SetBOM(true);
    diff_report.SetHasDiff(true);
  } else {
    diff_report.GetDiffDetail().SetBOM(false);
  }

  //  Device Config
  if (project.ToString("DeviceConfig") != baseline_project.ToString("DeviceConfig")) {  // diff
    diff_report.GetDiffDetail().SetDeviceConfig(true);
    diff_report.SetHasDiff(true);
  } else {
    diff_report.GetDiffDetail().SetDeviceConfig(false);
  }

  //  Project Setting
  if (project.ToString("ProjectSetting") != baseline_project.ToString("ProjectSetting")) {  // diff
    diff_report.GetDiffDetail().SetProjectSetting(true);
    diff_report.SetHasDiff(true);
  } else {
    diff_report.GetDiffDetail().SetProjectSetting(false);
  }

  //  Topology Device
  if (project.ToString("Devices") != baseline_project.ToString("Devices")) {  // diff
    diff_report.GetDiffDetail().SetTopologyDevice(true);
    diff_report.SetHasDiff(true);
  } else {
    diff_report.GetDiffDetail().SetTopologyDevice(false);
  }

  //  Topology Link
  if (project.ToString("Links") != baseline_project.ToString("Links")) {  // diff
    diff_report.GetDiffDetail().SetTopologyLink(true);
    diff_report.SetHasDiff(true);
  } else {
    diff_report.GetDiffDetail().SetTopologyLink(false);
  }

  return act_status;
}

}  // namespace core
}  // namespace act
