#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"
namespace act {
namespace core {

ACT_STATUS ActCore::EnableMultipleTransaction(qint64 project_id) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;

    return std::make_shared<ActStatusInternalError>("Get project failed");
  }

  if (share_transaction_list.contains(project_id)) {
    QString error_msg = QString("Project (%1) - Multiple transaction is running").arg(project.GetProjectName());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActStatusInternalError>(error_msg);
  }

  // Enable transaction from multiple APIs
  share_transaction_list[project_id] = true;

  act::core::g_core.StartTransaction(project_id);

  return act_status;
}

ACT_STATUS ActCore::DisableMultipleTransaction(qint64 project_id) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;

    return std::make_shared<ActStatusInternalError>("Get project failed");
  }

  if (!share_transaction_list.contains(project_id)) {
    QString error_msg = QString("Project (%1) - Multiple transaction is not running").arg(project.GetProjectName());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActStatusInternalError>(error_msg);
  }

  // Disable transaction from multiple APIs
  share_transaction_list.remove(project_id);

  return act_status;
}

ACT_STATUS ActCore::StartTransaction(qint64 project_id) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;

    return std::make_shared<ActStatusInternalError>("Get project failed");
  }

  bool init_flag = false;
  // If multiple transaction is active,
  // the transaction list may be initialised by another request
  if (share_transaction_list.contains(project_id)) {
    if (!transaction_list.contains(project_id)) {
      // If this is the first API of the multiple requests,
      // initiate the list
      init_flag = true;
    }
  } else {
    if (transaction_list.contains(project_id)) {
      QString error_msg = QString("Project (%1) - Transaction is running").arg(project.GetProjectName());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActStatusInternalError>(error_msg);
    }

    // Always initiate the transaction list
    init_flag = true;
  }

  if (init_flag) {
    // Initiate the transaction state
    transaction_list[project_id] = QList<ActProject>();
    transaction_list[project_id].append(project);
  }

  return act_status;
}

ACT_STATUS ActCore::StopTransaction(qint64 project_id) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;

    return std::make_shared<ActStatusInternalError>("Get project failed");
  }

  if (!transaction_list.contains(project_id)) {
    QString error_msg = QString("Project (%1) - Transaction is not running").arg(project.GetProjectName());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActStatusInternalError>(error_msg);
  }

  // Destroy all the operation in the transaction list
  transaction_list.remove(project_id);

  return act_status;
}

ACT_STATUS ActCore::SaveOperation(const ActProject &project) {
  ACT_STATUS_INIT();

  qint64 project_id = project.GetId();

  // If the project does not exist in the transaction list,
  // which means that this operation does not need to be saved
  if (!transaction_list.contains(project_id)) {
    return act_status;
  }

  // Remove the oldest snapshot to make space for the new one
  QList<ActProject> &project_transaction = transaction_list[project_id];

  project_transaction.append(project);

  if (project_transaction.size() > 100) {
    QString warning_msg = QString("Project (%1) - The size %2 of the transaction is quite large")
                              .arg(project.GetProjectName())
                              .arg(QString::number(project_transaction.size()));
    qWarning() << warning_msg.toStdString().c_str();
  }

  return act_status;
}

ACT_STATUS ActCore::CommitTransaction(qint64 project_id) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;

    return std::make_shared<ActStatusInternalError>("Get project failed");
  }

  // If multiple transaction is active,
  // which means there is another request still to be processed
  // skip it
  if (share_transaction_list.contains(project_id)) {
    return act_status;
  }

  if (!transaction_list.contains(project_id)) {
    QString error_msg = QString("Project (%1) - Transaction is not running").arg(project.GetProjectName());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActStatusInternalError>(error_msg);
  }

  QList<ActProject> &project_transaction = transaction_list[project_id];

  // If there is no operation in the transaction list other than the initial state,
  // simply skip it
  if (project_transaction.size() == 1) {
    transaction_list.remove(project_id);
    return act_status;
  }

  if (!undo_operation_history.contains(project_id)) {
    QString error_msg = QString("Project (%1) - Missing operation history").arg(project.GetProjectName());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActStatusInternalError>(error_msg);
  }

  // Remove the oldest snapshot to make space for the new one
  QStack<ActProject> &project_history = undo_operation_history[project_id];
  if (project_history.size() >= HISTORY_LIMIT) {
    project_history.removeFirst();
  }

  // Delete the redo record since this change
  redo_operation_history[project_id].clear();

  // Keep only the first transaction, which is the state before the project change
  project = project_transaction[0];
  project_history.push(project);

  // After commit the transaction, destroy the transaction
  transaction_list.remove(project_id);

  // Send features available status msg
  ActFeaturesAvailableStatusResult result(CanUndoProject(project_id), CanRedoProject(project_id),
                                          CanDeployProject(project_id));
  ActFeaturesAvailableStatusWSResponse ws_features_available_status_msg(result);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_features_available_status_msg, project_id);

  return act_status;
}

bool ActCore::CanUndoProject(qint64 project_id) const { return undo_operation_history[project_id].size() > 0; }

bool ActCore::CanRedoProject(qint64 project_id) const { return redo_operation_history[project_id].size() > 0; }

ACT_STATUS ActCore::UndoProject(qint64 project_id) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;

    return std::make_shared<ActBadRequest>("Get project failed");
  }

  if (!CanUndoProject(project_id)) {
    QString error_msg = QString("Project (%1) - Can not undo project").arg(project.GetProjectName());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  redo_operation_history[project_id].push(project);
  project = undo_operation_history[project_id].pop();
  return ApplyOperation(project_id, project);
}

ACT_STATUS ActCore::RedoProject(qint64 project_id) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;

    return std::make_shared<ActBadRequest>("Get project failed");
  }

  if (!CanRedoProject(project_id)) {
    QString error_msg = QString("Project (%1) - Can not redo project").arg(project.GetProjectName());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  undo_operation_history[project_id].push(project);
  project = redo_operation_history[project_id].pop();
  return ApplyOperation(project_id, project);
}

ACT_STATUS ActCore::ApplyOperation(qint64 project_id, ActProject &project) {
  ACT_STATUS_INIT();
  qDebug() << "Applying operation for project" << project_id;

  act_status = this->CheckProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Check project failed";
    return act_status;
  }

  QSet<ActProject> project_set = this->GetProjectSet();

  // Check the item does exist by id
  typename QSet<ActProject>::const_iterator iterator;
  iterator = project_set.find(project);
  if (iterator != project_set.end()) {
    // If yes, delete it
    project_set.erase(iterator);
  }

  // Update the last modified timestamp
  qint64 current_timestamp = QDateTime::currentSecsSinceEpoch();
  project.SetLastModifiedTime(current_timestamp);

  // Insert the project to core set
  project_set.insert(project);
  this->SetProjectSet(project_set);

  // [feat:722] Auto Save
  if (this->GetSystemConfig().GetAutoSave()) {
    act_status = this->SaveProject(project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Save project failed with project id:" << project.GetId();

      return act_status;
    }
  }

  // Send update project msg
  ActProjectPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project, true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project.GetId());

  // Send features available status msg
  ActFeaturesAvailableStatusResult result(CanUndoProject(project_id), CanRedoProject(project_id),
                                          CanDeployProject(project_id));
  ActFeaturesAvailableStatusWSResponse ws_features_available_status_msg(result);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_features_available_status_msg, project_id);

  return act_status;
}

}  // namespace core
}  // namespace act