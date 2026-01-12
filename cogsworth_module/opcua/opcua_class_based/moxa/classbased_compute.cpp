#include "classbased_compute.h"

namespace ClassBased {

UaStatus startComputeMethod(const UaNodeId& nodeId) {
  UaStatus ret;
  ACT_STATUS_INIT();

  MoxaClassBased::ComputeStateMachineType* pComputeStateMachineType =
      (MoxaClassBased::ComputeStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  switch (pComputeStateMachineType->getProgramState()) {
    case MoxaClassBased::ComputeStateMachineType::Completed:
    case MoxaClassBased::ComputeStateMachineType::Failed:
    case MoxaClassBased::ComputeStateMachineType::Ready:
      pComputeStateMachineType->setErrorCode(M_UA_NO_ERROR);
      pComputeStateMachineType->setErrorMessage(UaString(""));
      pComputeStateMachineType->setProgramState(MoxaClassBased::ComputeStateMachineType::Running);
      pComputeStateMachineType->start();
      break;
    default:
      ret = OpcUa_BadNotExecutable;
      break;
  }
  return ret;
}

void runComputeCb(ACT_STATUS status, void* arg) {
  ((ActStatusBase*)arg)->SetStatus(status->GetStatus());
  ((ActStatusBase*)arg)->SetErrorMessage(status->GetErrorMessage());
}

void runComputeMethod(const UaNodeId& nodeId) {
  ACT_STATUS_INIT();

  MoxaClassBased::ComputeStateMachineType* pComputeStateMachineType =
      (MoxaClassBased::ComputeStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  // Create a std::promise object
  pComputeStateMachineType->signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = pComputeStateMachineType->signal_sender->get_future();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    // Set failed state
    pComputeStateMachineType->setProgramState(MoxaClassBased::ComputeStateMachineType::Failed);

    // Set error message
    pComputeStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pComputeStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
  }

  QList<qint64> dev_id_list;
  for (ActDevice device : project.GetDevices()) {
    dev_id_list.append(device.GetId());
  }

  ActStatusBase status(ActStatusType::kRunning);
  act_status =
      act::core::g_core.OpcUaStartCompute(project_id, std::move(signal_receiver), runComputeCb, (void*)&status);
  if (!IsActStatusSuccess(act_status)) {
    // Set failed state
    pComputeStateMachineType->setProgramState(MoxaClassBased::ComputeStateMachineType::Failed);

    // Set error message
    pComputeStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pComputeStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
    return;
  }

  // wait for computing
  while (status.GetStatus() == ActStatusType::kRunning) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (status.GetStatus() == ActStatusType::kFinished || status.GetStatus() == ActStatusType::kSuccess) {
    // Set completed state
    pComputeStateMachineType->setProgramState(MoxaClassBased::ComputeStateMachineType::Completed);
  } else if (status.GetStatus() == ActStatusType::kStop) {
    // Set ready state
    pComputeStateMachineType->setProgramState(MoxaClassBased::ComputeStateMachineType::Ready);
  } else {
    // Set failed state
    pComputeStateMachineType->setProgramState(MoxaClassBased::ComputeStateMachineType::Failed);

    // Set error message
    pComputeStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(status.GetStatus()));
    pComputeStateMachineType->setErrorMessage(UaString(status.GetErrorMessage().toStdString().c_str()));
  }

  return;
}

UaStatus stopComputeMethod(const UaNodeId& nodeId) {
  UaStatus ret;

  MoxaClassBased::ComputeStateMachineType* pComputeStateMachineType =
      (MoxaClassBased::ComputeStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  pComputeStateMachineType->signal_sender->set_value();

  return ret;
}
}  // namespace ClassBased