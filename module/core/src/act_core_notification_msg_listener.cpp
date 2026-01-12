
#include "act_core.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::CheckWSConnect(qint64 &project_id) {
  ACT_STATUS_INIT();

  // Check project by id
  if (project_id != -1) {
    ActProject project;
    act_status = this->GetProject(project_id, project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Get project failed with project id:" << project_id;
      return act_status;
    }
  }

  // Check WebSocket connection size
  if (this->ws_listeners_.size() >= ACT_WS_SOCKET_MAX_SIZE) {
    QString error_msg = QString("WebSocket connection count(%1) exceeds the limit(%2)")
                            .arg(this->ws_listeners_.size())
                            .arg(ACT_WS_SOCKET_MAX_SIZE);
    qCritical() << __func__ << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

ACT_STATUS ActCore::AddWSListener(const std::shared_ptr<WSListener> &ws_listener) {
  ACT_STATUS_INIT();

  qint64 listener_id;
  ws_listener->getId(listener_id);
  this->ws_listeners_.insert(listener_id, ws_listener);

  return act_status;
}

ACT_STATUS ActCore::RemoveWSListener(const qint64 &id) {
  ACT_STATUS_INIT();
  qDebug() << __func__;
  qDebug() << __func__ << QString("Remove WS Listener(%1)").arg(id).toStdString().c_str();
  this->ws_listeners_.remove(id);
  qDebug() << __func__ << "done";
  return act_status;
}

// XXX: unused function
// ACT_STATUS ActCore::SendMessageToAllWSListeners(const QString &message) {
//   ACT_STATUS_INIT();
//   // qDebug() << __func__ << QString("WS Listeners size: %1").arg(this->ws_listeners_.size()).toStdString().c_str();

//   // Send message to all WSListener
//   for (auto key : this->ws_listeners_.keys()) {
//     auto &ws_listener = ws_listeners_.value(key);

//     // Get Project ID
//     qint64 listener_project_id = 0;
//     ws_listener->getProjectId(listener_project_id);

//     ws_listener->sendMessage(message.toStdString().c_str());
//     // qDebug()
//     //     << __func__
//     //     << QString("Send WS message to client(%1,
//     //     project:%2)").arg(key).arg(listener_project_id).toStdString().c_str();
//   }

//   return act_status;
// }

ACT_STATUS ActCore::SendMessageToSystemWSListeners(const QString &message) {
  ACT_STATUS_INIT();

  // Send message to System WSListener(ProjectId = -1)
  for (auto key : this->ws_listeners_.keys()) {
    auto &ws_listener = ws_listeners_.value(key);

    // Get Project ID
    qint64 listener_project_id = 0;
    ws_listener->getProjectId(listener_project_id);

    if (listener_project_id == -1) {  // WSListener(ProjectId = -1)
      ws_listener->sendMessage(message.toStdString().c_str());
    }
  }

  return act_status;
}

ACT_STATUS ActCore::SendMessageToProjectWSListeners(const qint64 &project_id, const QString &message) {
  ACT_STATUS_INIT();

  if (project_id == -1) {
    QString error_msg = QString("project id unknown.");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Send message to Project WSListener
  for (auto key : this->ws_listeners_.keys()) {
    auto &ws_listener = ws_listeners_.value(key);

    // Get Project ID
    qint64 listener_project_id = 0;
    ws_listener->getProjectId(listener_project_id);

    if (listener_project_id == project_id) {
      ws_listener->sendMessage(message.toStdString().c_str());
    }
  }

  return act_status;
}

ACT_STATUS ActCore::SendMessageToWSListener(const qint64 &id, const QString &message) {
  ACT_STATUS_INIT();

  if (id == -1) {
    QString error_msg = QString("project id unknown.");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (!ws_listeners_.contains(id)) {
    qCritical() << __func__ << QString("Websocket Listener(%1) not found").arg(id);
    return std::make_shared<ActStatusNotFound>(QString("Websocket Listener(%1)").arg(id));
  }

  // Send message to specify WSListener
  auto &ws_listener = ws_listeners_.value(id);

  // Get Project ID
  qint64 listener_project_id = 0;
  ws_listener->getProjectId(listener_project_id);

  ws_listener->sendMessage(message.toStdString().c_str());

  return act_status;
}

}  // namespace core
}  // namespace act
