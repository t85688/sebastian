#include "classbased_syncdeviceconfig.h"

namespace ClassBased {

UaStatus startSyncDeviceConfigMethod(const UaNodeId& nodeId) {
  UaStatus ret;
  ACT_STATUS_INIT();

  MoxaClassBased::SyncDeviceConfigStateMachineType* pSyncDeviceConfigStateMachineType =
      (MoxaClassBased::SyncDeviceConfigStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  switch (pSyncDeviceConfigStateMachineType->getProgramState()) {
    case MoxaClassBased::SyncDeviceConfigStateMachineType::Completed:
    case MoxaClassBased::SyncDeviceConfigStateMachineType::Failed:
    case MoxaClassBased::SyncDeviceConfigStateMachineType::Ready:
      pSyncDeviceConfigStateMachineType->setErrorCode(M_UA_NO_ERROR);
      pSyncDeviceConfigStateMachineType->setErrorMessage(UaString(""));
      pSyncDeviceConfigStateMachineType->setProgramState(MoxaClassBased::SyncDeviceConfigStateMachineType::Running);
      pSyncDeviceConfigStateMachineType->start();
      break;
    default:
      ret = OpcUa_BadNotExecutable;
      break;
  }
  return ret;
}

void runSyncDeviceConfigCb(ACT_STATUS status, void* arg) {
  ((ActStatusBase*)arg)->SetStatus(status->GetStatus());
  ((ActStatusBase*)arg)->SetErrorMessage(status->GetErrorMessage());
}

void runSyncDeviceConfigMethod(const UaNodeId& nodeId) {
  ACT_STATUS_INIT();

  MoxaClassBased::SyncDeviceConfigStateMachineType* pSyncDeviceConfigStateMachineType =
      (MoxaClassBased::SyncDeviceConfigStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  // Create a std::promise object
  pSyncDeviceConfigStateMachineType->signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = pSyncDeviceConfigStateMachineType->signal_sender->get_future();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  QList<qint64> dev_id_list = {pMoxaNodeManager->getDeviceId(nodeId)};

  ActStatusBase status(ActStatusType::kRunning);
  act_status = act::core::g_core.OpcUaStartSyncDevices(project_id, dev_id_list, std::move(signal_receiver),
                                                       runSyncDeviceConfigCb, (void*)&status);
  if (!IsActStatusSuccess(act_status)) {
    // Set failed state
    pSyncDeviceConfigStateMachineType->setProgramState(MoxaClassBased::SyncDeviceConfigStateMachineType::Failed);

    // Set error message
    pSyncDeviceConfigStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pSyncDeviceConfigStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
    return;
  }

  // wait for computing
  while (status.GetStatus() == ActStatusType::kRunning) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (status.GetStatus() == ActStatusType::kFinished || status.GetStatus() == ActStatusType::kSuccess) {
    // Set completed state
    pSyncDeviceConfigStateMachineType->setProgramState(MoxaClassBased::SyncDeviceConfigStateMachineType::Completed);
  } else if (status.GetStatus() == ActStatusType::kStop) {
    // Set ready state
    pSyncDeviceConfigStateMachineType->setProgramState(MoxaClassBased::SyncDeviceConfigStateMachineType::Ready);
  } else {
    // Set failed state
    pSyncDeviceConfigStateMachineType->setProgramState(MoxaClassBased::SyncDeviceConfigStateMachineType::Failed);

    // Set error message
    pSyncDeviceConfigStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(status.GetStatus()));
    pSyncDeviceConfigStateMachineType->setErrorMessage(UaString(status.GetErrorMessage().toStdString().c_str()));
  }

  return;
}

UaStatus stopSyncDeviceConfigMethod(const UaNodeId& nodeId) {
  UaStatus ret;

  MoxaClassBased::SyncDeviceConfigStateMachineType* pSyncDeviceConfigStateMachineType =
      (MoxaClassBased::SyncDeviceConfigStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  pSyncDeviceConfigStateMachineType->signal_sender->set_value();

  return ret;
}
}  // namespace ClassBased