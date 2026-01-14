#include "classbased_factorydefault.h"

namespace ClassBased {

UaStatus startFactoryDefaultMethod(const UaNodeId& nodeId) {
  UaStatus ret;
  ACT_STATUS_INIT();

  MoxaClassBased::FactoryDefaultStateMachineType* pFactoryDefaultStateMachineType =
      (MoxaClassBased::FactoryDefaultStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  switch (pFactoryDefaultStateMachineType->getProgramState()) {
    case MoxaClassBased::FactoryDefaultStateMachineType::Completed:
    case MoxaClassBased::FactoryDefaultStateMachineType::Failed:
    case MoxaClassBased::FactoryDefaultStateMachineType::Ready:
      pFactoryDefaultStateMachineType->setErrorCode(M_UA_NO_ERROR);
      pFactoryDefaultStateMachineType->setErrorMessage(UaString(""));
      pFactoryDefaultStateMachineType->setProgramState(MoxaClassBased::FactoryDefaultStateMachineType::Running);
      pFactoryDefaultStateMachineType->start();
      break;
    default:
      ret = OpcUa_BadNotExecutable;
      break;
  }
  return ret;
}

void runFactoryDefaultCb(ACT_STATUS status, void* arg) {
  ((ActStatusBase*)arg)->SetStatus(status->GetStatus());
  ((ActStatusBase*)arg)->SetErrorMessage(status->GetErrorMessage());
}

void runFactoryDefaultMethod(const UaNodeId& nodeId) {
  ACT_STATUS_INIT();

  MoxaClassBased::FactoryDefaultStateMachineType* pFactoryDefaultStateMachineType =
      (MoxaClassBased::FactoryDefaultStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  // Create a std::promise object
  pFactoryDefaultStateMachineType->signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = pFactoryDefaultStateMachineType->signal_sender->get_future();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  QList<qint64> dev_id_list = {pMoxaNodeManager->getDeviceId(nodeId)};

  ActStatusBase status(ActStatusType::kRunning);
  act_status = act::core::g_core.OpcUaStartConfigFactoryDefault(project_id, dev_id_list, std::move(signal_receiver),
                                                                runFactoryDefaultCb, (void*)&status);
  if (!IsActStatusSuccess(act_status)) {
    // Set failed state
    pFactoryDefaultStateMachineType->setProgramState(MoxaClassBased::FactoryDefaultStateMachineType::Failed);

    // Set error message
    pFactoryDefaultStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pFactoryDefaultStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
    return;
  }

  // wait for computing
  while (status.GetStatus() == ActStatusType::kRunning) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (status.GetStatus() == ActStatusType::kFinished || status.GetStatus() == ActStatusType::kSuccess) {
    // Set completed state
    pFactoryDefaultStateMachineType->setProgramState(MoxaClassBased::FactoryDefaultStateMachineType::Completed);
  } else if (status.GetStatus() == ActStatusType::kStop) {
    // Set ready state
    pFactoryDefaultStateMachineType->setProgramState(MoxaClassBased::FactoryDefaultStateMachineType::Ready);
  } else {
    // Set failed state
    pFactoryDefaultStateMachineType->setProgramState(MoxaClassBased::FactoryDefaultStateMachineType::Failed);

    // Set error message
    pFactoryDefaultStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(status.GetStatus()));
    pFactoryDefaultStateMachineType->setErrorMessage(UaString(status.GetErrorMessage().toStdString().c_str()));
  }

  return;
}

UaStatus stopFactoryDefaultMethod(const UaNodeId& nodeId) {
  UaStatus ret;

  MoxaClassBased::FactoryDefaultStateMachineType* pFactoryDefaultStateMachineType =
      (MoxaClassBased::FactoryDefaultStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  pFactoryDefaultStateMachineType->signal_sender->set_value();

  return ret;
}
}  // namespace ClassBased