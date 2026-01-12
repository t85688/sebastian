#include "classbased_reboot.h"

namespace ClassBased {

UaStatus startRebootMethod(const UaNodeId& nodeId) {
  UaStatus ret;
  ACT_STATUS_INIT();

  MoxaClassBased::RebootStateMachineType* pRebootStateMachineType =
      (MoxaClassBased::RebootStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  switch (pRebootStateMachineType->getProgramState()) {
    case MoxaClassBased::RebootStateMachineType::Completed:
    case MoxaClassBased::RebootStateMachineType::Failed:
    case MoxaClassBased::RebootStateMachineType::Ready:
      pRebootStateMachineType->setErrorCode(M_UA_NO_ERROR);
      pRebootStateMachineType->setErrorMessage(UaString(""));
      pRebootStateMachineType->setProgramState(MoxaClassBased::RebootStateMachineType::Running);
      pRebootStateMachineType->start();
      break;
    default:
      ret = OpcUa_BadNotExecutable;
      break;
  }
  return ret;
}

void runRebootCb(ACT_STATUS status, void* arg) {
  ((ActStatusBase*)arg)->SetStatus(status->GetStatus());
  ((ActStatusBase*)arg)->SetErrorMessage(status->GetErrorMessage());
}

void runRebootMethod(const UaNodeId& nodeId) {
  ACT_STATUS_INIT();

  MoxaClassBased::RebootStateMachineType* pRebootStateMachineType =
      (MoxaClassBased::RebootStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  // Create a std::promise object
  pRebootStateMachineType->signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = pRebootStateMachineType->signal_sender->get_future();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  QList<qint64> dev_id_list = {pMoxaNodeManager->getDeviceId(nodeId)};

  ActStatusBase status(ActStatusType::kRunning);
  act_status = act::core::g_core.OpcUaStartConfigReboot(project_id, dev_id_list, std::move(signal_receiver),
                                                        runRebootCb, (void*)&status);
  if (!IsActStatusSuccess(act_status)) {
    // Set failed state
    pRebootStateMachineType->setProgramState(MoxaClassBased::RebootStateMachineType::Failed);

    // Set error message
    pRebootStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pRebootStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
    return;
  }

  // wait for computing
  while (status.GetStatus() == ActStatusType::kRunning) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (status.GetStatus() == ActStatusType::kFinished || status.GetStatus() == ActStatusType::kSuccess) {
    // Set completed state
    pRebootStateMachineType->setProgramState(MoxaClassBased::RebootStateMachineType::Completed);
  } else if (status.GetStatus() == ActStatusType::kStop) {
    // Set ready state
    pRebootStateMachineType->setProgramState(MoxaClassBased::RebootStateMachineType::Ready);
  } else {
    // Set failed state
    pRebootStateMachineType->setProgramState(MoxaClassBased::RebootStateMachineType::Failed);

    // Set error message
    pRebootStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(status.GetStatus()));
    pRebootStateMachineType->setErrorMessage(UaString(status.GetErrorMessage().toStdString().c_str()));
  }

  return;
}

UaStatus stopRebootMethod(const UaNodeId& nodeId) {
  UaStatus ret;

  MoxaClassBased::RebootStateMachineType* pRebootStateMachineType =
      (MoxaClassBased::RebootStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  pRebootStateMachineType->signal_sender->set_value();

  return ret;
}
}  // namespace ClassBased