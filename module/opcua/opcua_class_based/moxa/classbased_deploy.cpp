#include "classbased_deploy.h"

namespace ClassBased {

UaStatus startDeployMethod(const UaNodeId& nodeId) {
  UaStatus ret;
  ACT_STATUS_INIT();

  QObject::disconnect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsError, nullptr, nullptr);
  QObject::disconnect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsMessage, nullptr, nullptr);

  MoxaClassBased::DeployStateMachineType* pDeployStateMachineType =
      (MoxaClassBased::DeployStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  switch (pDeployStateMachineType->getProgramState()) {
    case MoxaClassBased::DeployStateMachineType::Completed:
    case MoxaClassBased::DeployStateMachineType::Failed:
    case MoxaClassBased::DeployStateMachineType::Ready:
      pDeployStateMachineType->setErrorCode(M_UA_NO_ERROR);
      pDeployStateMachineType->setErrorMessage(UaString(""));
      pDeployStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Ready);
      pDeployStateMachineType->start();
      break;
    default:
      ret = OpcUa_BadNotExecutable;
      break;
  }
  return ret;
}

void runDeployMethod(const UaNodeId& nodeId) {
  ACT_STATUS_INIT();

  MoxaClassBased::DeployStateMachineType* pDeployStateMachineType =
      (MoxaClassBased::DeployStateMachineType*)pMoxaNodeManager->getNode(nodeId);
  pDeployStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Running);

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    // Set failed state
    pDeployStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Failed);

    // Set error message
    pDeployStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pDeployStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
    return;
  }

  QList<qint64> dev_id_list;
  for (ActDevice device : project.GetDevices()) {
    if (device.CheckCanDeploy()) {
      dev_id_list.append(device.GetId());
    }
  }

  QObject::connect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsMessage, [=](const QString& message) {
    DeployWsResponse response;
    try {
      // Do patch parser
      response.FromString(message);
      ActStatusType status_code = static_cast<ActStatusType>(response.GetStatusCode());
      switch (status_code) {
        case ActStatusType::kRunning:
          pDeployStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Running);
          break;
        case ActStatusType::kStop:
          pDeployStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Ready);
          break;
        case ActStatusType::kSuccess:
        case ActStatusType::kFinished:
          pDeployStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Completed);
          break;
        case ActStatusType::kFailed:
        case ActStatusType::kBadRequest:
        case ActStatusType::kUnauthorized:
        case ActStatusType::kForbidden:
        case ActStatusType::kNotFound:
        case ActStatusType::kConflict:
        case ActStatusType::kUnProcessable:
        case ActStatusType::kInternalError:
        case ActStatusType::kServiceUnavailable:
          pDeployStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Failed);
          pDeployStateMachineType->setErrorCode(OpcUa_UInt32(response.GetStatusCode()));
          pDeployStateMachineType->setErrorMessage(UaString(response.GetErrorMessage().toStdString().c_str()));
          break;
        default:
          pDeployStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Failed);
          pDeployStateMachineType->setErrorCode(OpcUa_UInt32(ActStatusType::kFailed));
          pDeployStateMachineType->setErrorMessage(UaString("Unknown status code %1").arg(response.GetStatusCode()));
          break;
      }
      if (status_code != ActStatusType::kRunning) {
        QObject::disconnect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsError, nullptr, nullptr);
        QObject::disconnect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsMessage, nullptr, nullptr);
      }
    } catch (std::exception& e) {
      pDeployStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Failed);
      pDeployStateMachineType->setErrorCode(OpcUa_UInt32(ActStatusType::kFailed));
      pDeployStateMachineType->setErrorMessage(e.what());

      QObject::disconnect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsError, nullptr, nullptr);
      QObject::disconnect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsMessage, nullptr, nullptr);
    }
  });

  QObject::connect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsError, [=](const QString& message) {
    pDeployStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Failed);
    pDeployStateMachineType->setErrorCode(OpcUa_UInt32(ActStatusType::kFailed));
    pDeployStateMachineType->setErrorMessage(message.toStdString().c_str());
  });

  DeployWsCommand deploy_ws_cmd(ActWSCommandEnum::kStartDeploy, project_id, -1, dev_id_list, true);
  pMoxaNodeManager->sendWsFromUaThread(deploy_ws_cmd.ToString().toStdString().c_str());

  return;
}

UaStatus stopDeployMethod(const UaNodeId& nodeId) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);

  WsBaseCommand ws_cmd(ActWSCommandEnum::kStopDeploy, project_id);
  pMoxaNodeManager->sendWsFromUaThread(ws_cmd.ToString().toStdString().c_str());

  MoxaClassBased::DeployStateMachineType* pDeployStateMachineType =
      (MoxaClassBased::DeployStateMachineType*)pMoxaNodeManager->getNode(nodeId);
  act_status = act::core::g_core.UpdateProjectStatus(project_id, ActProjectStatusEnum::kIdle);
  if (!IsActStatusSuccess(act_status)) {
    pDeployStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Failed);
    pDeployStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pDeployStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
  } else {
    pDeployStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Ready);
    pDeployStateMachineType->setErrorCode(M_UA_NO_ERROR);
    pDeployStateMachineType->setErrorMessage(UaString(""));
  }
  return ret;
}
}  // namespace ClassBased