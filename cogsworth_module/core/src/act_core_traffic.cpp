#include <QRegExp>
#include <QRegExpValidator>
#include <QSet>

#include "act_core.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::GetTrafficTypeConfigurationSetting(
    qint64 &project_id, ActTrafficTypeConfigurationSetting &traffic_type_configuration_setting) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  traffic_type_configuration_setting.SetTrafficTypeConfigurationSetting(
      project.GetTrafficDesign().GetTrafficTypeConfigurationSetting());

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateTrafficTypeConfigurationSetting(qint64 &project_id,
                                                          ActTrafficTypeConfiguration &traffic_type_configuration) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<ActTrafficTypeConfiguration> &traffic_type_configuration_setting =
      project.GetTrafficDesign().GetTrafficTypeConfigurationSetting();
  int idx = traffic_type_configuration_setting.indexOf(traffic_type_configuration);
  if (idx == -1) {
    QString error_msg = QString("The traffic type configuration %1").arg(traffic_type_configuration.ToString());
    qCritical() << error_msg.toStdString().c_str() << "is not found";
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  traffic_type_configuration_setting[idx] = traffic_type_configuration;

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActTrafficDesignPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetTrafficDesign(),
                                        true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetTrafficApplicationSetting(qint64 &project_id,
                                                 ActTrafficApplicationSetting &traffic_application_setting) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  traffic_application_setting.SetApplicationSetting(project.GetTrafficDesign().GetApplicationSetting());

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::CreateTrafficApplicationSetting(qint64 &project_id, ActTrafficApplication &traffic_application) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  if (traffic_application.GetVlanSetting().GetEtherType() == 0x88F7) {
    // [bugfix:3370] block ether type 0x88F7
    QString error_msg = QString("Blocking ether type 0x88F7 used by IEEE 1588 PTP or 802.1AS packet");
    return std::make_shared<ActBadRequest>(error_msg);
  } else if (traffic_application.GetVlanSetting().GetUdpPort() == 319) {
    QString error_msg = QString("Blocking UDP port 319 used by IEEE 1588 PTP or 802.1AS packet");
    return std::make_shared<ActBadRequest>(error_msg);
  } else if (traffic_application.GetVlanSetting().GetUdpPort() == 320) {
    QString error_msg = QString("Blocking UDP port 320 used by IEEE 1588 PTP or 802.1AS packet");
    return std::make_shared<ActBadRequest>(error_msg);
  }

  QSet<ActTrafficApplication> &application_setting = project.GetTrafficDesign().GetApplicationSetting();

  qint64 id;
  act_status = this->GenerateUniqueId<ActTrafficApplication>(application_setting,
                                                             project.last_assigned_traffic_application_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot get an available unique id";
    return act_status;
  }
  traffic_application.SetId(id);

  application_setting.insert(traffic_application);

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActTrafficDesignPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetTrafficDesign(),
                                        true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::CopyTrafficApplicationSetting(qint64 &project_id, qint64 &application_id,
                                                  ActTrafficApplication &traffic_application) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  if (traffic_application.GetVlanSetting().GetEtherType() == 0x88F7) {
    // [bugfix:3370] block ether type 0x88F7
    QString error_msg = QString("Blocking ether type 0x88F7 used by IEEE 1588 PTP or 802.1AS packet");
    return std::make_shared<ActBadRequest>(error_msg);
  } else if (traffic_application.GetVlanSetting().GetUdpPort() == 319) {
    QString error_msg = QString("Blocking UDP port 319 used by IEEE 1588 PTP or 802.1AS packet");
    return std::make_shared<ActBadRequest>(error_msg);
  } else if (traffic_application.GetVlanSetting().GetUdpPort() == 320) {
    QString error_msg = QString("Blocking UDP port 320 used by IEEE 1588 PTP or 802.1AS packet");
    return std::make_shared<ActBadRequest>(error_msg);
  }

  QSet<ActTrafficApplication> &application_setting = project.GetTrafficDesign().GetApplicationSetting();

  QSet<ActTrafficApplication>::iterator application_iter =
      application_setting.find(ActTrafficApplication(application_id));
  if (application_iter == application_setting.end()) {
    QString error_msg = QString("Application %1").arg(application_id);
    qWarning() << error_msg.toStdString().c_str() << "is not found";
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  traffic_application = *application_iter;
  QString application_name = QString("%1_copy").arg(traffic_application.GetApplicationName());
  traffic_application.SetApplicationName(application_name);

  qint64 id;
  act_status = this->GenerateUniqueId<ActTrafficApplication>(application_setting,
                                                             project.last_assigned_traffic_application_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot get an available unique id";
    return act_status;
  }
  traffic_application.SetId(id);

  application_setting.insert(traffic_application);

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActTrafficDesignPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetTrafficDesign(),
                                        true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateTrafficApplicationSetting(qint64 &project_id, ActTrafficApplication &traffic_application) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  if (traffic_application.GetVlanSetting().GetEtherType() == 0x88F7) {
    // [bugfix:3370] block ether type 0x88F7
    QString error_msg = QString("Blocking ether type 0x88F7 used by IEEE 1588 PTP or 802.1AS packet");
    return std::make_shared<ActBadRequest>(error_msg);
  } else if (traffic_application.GetVlanSetting().GetUdpPort() == 319) {
    QString error_msg = QString("Blocking UDP port 319 used by IEEE 1588 PTP or 802.1AS packet");
    return std::make_shared<ActBadRequest>(error_msg);
  } else if (traffic_application.GetVlanSetting().GetUdpPort() == 320) {
    QString error_msg = QString("Blocking UDP port 320 used by IEEE 1588 PTP or 802.1AS packet");
    return std::make_shared<ActBadRequest>(error_msg);
  }

  QSet<ActTrafficApplication> &application_setting = project.GetTrafficDesign().GetApplicationSetting();
  QSet<ActTrafficApplication>::iterator iter = application_setting.find(traffic_application);
  if (iter == application_setting.end()) {
    QString error_msg = QString("The traffic per-stream priority %1").arg(traffic_application.ToString());
    qCritical() << error_msg.toStdString().c_str() << "is not found";
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  application_setting.erase(iter);
  application_setting.insert(traffic_application);

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActTrafficDesignPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetTrafficDesign(),
                                        true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::DeleteTrafficApplicationSetting(qint64 &project_id, qint64 &traffic_application_id) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QSet<ActTrafficApplication> &application_setting = project.GetTrafficDesign().GetApplicationSetting();
  QSet<ActTrafficApplication>::iterator application_iter =
      application_setting.find(ActTrafficApplication(traffic_application_id));
  if (application_iter == application_setting.end()) {
    QString error_msg = QString("The traffic application %1").arg(traffic_application_id);
    qCritical() << error_msg.toStdString().c_str() << "is not found";
    return std::make_shared<ActStatusNotFound>(error_msg);
  }
  application_setting.erase(application_iter);

  QSet<ActTrafficStream> &stream_setting = project.GetTrafficDesign().GetStreamSetting();
  QSet<ActTrafficStream>::iterator stream_setting_iter = stream_setting.begin();
  while (stream_setting_iter != stream_setting.end()) {
    if (stream_setting_iter->GetApplicationId() == traffic_application_id) {
      stream_setting_iter = stream_setting.erase(stream_setting_iter);
    } else {
      stream_setting_iter++;
    }
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActTrafficDesignPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetTrafficDesign(),
                                        true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetTrafficStreamSetting(qint64 &project_id, ActTrafficStreamSetting &traffic_stream_setting) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  traffic_stream_setting.SetStreamSetting(project.GetTrafficDesign().GetStreamSetting());

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::CreateTrafficStreamSetting(qint64 &project_id, ActTrafficStream &traffic_stream) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QSet<ActTrafficStream> &stream_setting = project.GetTrafficDesign().GetStreamSetting();

  qint64 id;
  act_status = this->GenerateUniqueId<ActTrafficStream>(stream_setting, project.last_assigned_traffic_stream_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot get an available unique id";
    return act_status;
  }
  traffic_stream.SetId(id);

  stream_setting.insert(traffic_stream);

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActTrafficDesignPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetTrafficDesign(),
                                        true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::CopyTrafficStreamSetting(qint64 &project_id, qint64 &stream_id, ActTrafficStream &traffic_stream) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QSet<ActTrafficStream> &stream_setting = project.GetTrafficDesign().GetStreamSetting();

  QSet<ActTrafficStream>::iterator stream_iter = stream_setting.find(ActTrafficStream(stream_id));
  if (stream_iter == stream_setting.end()) {
    QString error_msg = QString("Stream %1").arg(stream_id);
    qWarning() << error_msg.toStdString().c_str() << "is not found";
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  traffic_stream = *stream_iter;
  QString stream_name = QString("%1_copy").arg(traffic_stream.GetStreamName());
  traffic_stream.SetStreamName(stream_name);

  qint64 id;
  act_status = this->GenerateUniqueId<ActTrafficStream>(stream_setting, project.last_assigned_traffic_stream_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot get an available unique id";
    return act_status;
  }
  traffic_stream.SetId(id);

  stream_setting.insert(traffic_stream);

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActTrafficDesignPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetTrafficDesign(),
                                        true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateTrafficStreamSetting(qint64 &project_id, ActTrafficStream &traffic_stream) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateTrafficStreamSetting(project, traffic_stream);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update streams failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActTrafficDesignPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetTrafficDesign(),
                                        true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateTrafficStreamSetting(ActProject &project, ActTrafficStream &traffic_stream) {
  QSet<ActTrafficStream> &stream_setting = project.GetTrafficDesign().GetStreamSetting();
  QSet<ActTrafficStream>::iterator iter = stream_setting.find(traffic_stream);
  if (iter == stream_setting.end()) {
    QString error_msg = QString("The traffic per-stream priority %1").arg(traffic_stream.ToString());
    qCritical() << error_msg.toStdString().c_str() << "is not found";
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  stream_setting.erase(iter);
  stream_setting.insert(traffic_stream);
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::DeleteTrafficStreamSetting(qint64 &project_id, qint64 &traffic_stream_id) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteTrafficStreamSetting(project, traffic_stream_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete stream failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActTrafficDesignPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetTrafficDesign(),
                                        true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::DeleteTrafficStreamSetting(ActProject &project, qint64 &traffic_stream_id) {
  QSet<ActTrafficStream> &stream_setting = project.GetTrafficDesign().GetStreamSetting();
  QSet<ActTrafficStream>::iterator iter = stream_setting.find(ActTrafficStream(traffic_stream_id));
  if (iter == stream_setting.end()) {
    QString error_msg = QString("The traffic stream %1").arg(traffic_stream_id);
    qCritical() << error_msg.toStdString().c_str() << "is not found";
    return std::make_shared<ActStatusNotFound>(error_msg);
  }

  stream_setting.erase(iter);

  return ACT_STATUS_SUCCESS;
}
}  // namespace core
}  // namespace act