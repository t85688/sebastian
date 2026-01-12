#include "classbased_importdeviceconfig.h"

namespace ClassBased {

UaStatus startImportDeviceConfigMethod(const UaNodeId& nodeId, const UaString& filePath) {
  UaStatus ret;
  ACT_STATUS_INIT();

  MoxaClassBased::ImportDeviceConfigStateMachineType* pImportDeviceConfigStateMachineType =
      (MoxaClassBased::ImportDeviceConfigStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  switch (pImportDeviceConfigStateMachineType->getProgramState()) {
    case MoxaClassBased::ImportDeviceConfigStateMachineType::Completed:
    case MoxaClassBased::ImportDeviceConfigStateMachineType::Failed:
    case MoxaClassBased::ImportDeviceConfigStateMachineType::Ready:
      pImportDeviceConfigStateMachineType->filePath = filePath;
      pImportDeviceConfigStateMachineType->setErrorCode(M_UA_NO_ERROR);
      pImportDeviceConfigStateMachineType->setErrorMessage(UaString(""));
      pImportDeviceConfigStateMachineType->setProgramState(MoxaClassBased::ImportDeviceConfigStateMachineType::Running);
      pImportDeviceConfigStateMachineType->start();
      break;
    default:
      ret = OpcUa_BadNotExecutable;
      break;
  }
  return ret;
}

void runImportDeviceConfigCb(ACT_STATUS status, void* arg) {
  ((ActStatusBase*)arg)->SetStatus(status->GetStatus());
  ((ActStatusBase*)arg)->SetErrorMessage(status->GetErrorMessage());
}

void runImportDeviceConfigMethod(const UaNodeId& nodeId) {
  ACT_STATUS_INIT();

  MoxaClassBased::ImportDeviceConfigStateMachineType* pImportDeviceConfigStateMachineType =
      (MoxaClassBased::ImportDeviceConfigStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  // Create a std::promise object
  pImportDeviceConfigStateMachineType->signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = pImportDeviceConfigStateMachineType->signal_sender->get_future();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  QList<qint64> dev_id_list = {pMoxaNodeManager->getDeviceId(nodeId)};
  QString import_path(pImportDeviceConfigStateMachineType->filePath.toUtf8());

  ActStatusBase status(ActStatusType::kRunning);
  act_status = act::core::g_core.OpcUaStartImportDeviceConfig(
      project_id, dev_id_list, import_path, std::move(signal_receiver), runImportDeviceConfigCb, (void*)&status);
  if (!IsActStatusSuccess(act_status)) {
    // Set failed state
    pImportDeviceConfigStateMachineType->setProgramState(MoxaClassBased::ImportDeviceConfigStateMachineType::Failed);

    // Set error message
    pImportDeviceConfigStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pImportDeviceConfigStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
    return;
  }

  // wait for computing
  while (status.GetStatus() == ActStatusType::kRunning) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (status.GetStatus() == ActStatusType::kFinished || status.GetStatus() == ActStatusType::kSuccess) {
    // Set completed state
    pImportDeviceConfigStateMachineType->setProgramState(MoxaClassBased::ImportDeviceConfigStateMachineType::Completed);
  } else if (status.GetStatus() == ActStatusType::kStop) {
    // Set ready state
    pImportDeviceConfigStateMachineType->setProgramState(MoxaClassBased::ImportDeviceConfigStateMachineType::Ready);
  } else {
    // Set failed state
    pImportDeviceConfigStateMachineType->setProgramState(MoxaClassBased::ImportDeviceConfigStateMachineType::Failed);

    // Set error message
    pImportDeviceConfigStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(status.GetStatus()));
    pImportDeviceConfigStateMachineType->setErrorMessage(UaString(status.GetErrorMessage().toStdString().c_str()));
  }

  return;
}

UaStatus stopImportDeviceConfigMethod(const UaNodeId& nodeId) {
  UaStatus ret;

  MoxaClassBased::ImportDeviceConfigStateMachineType* pImportDeviceConfigStateMachineType =
      (MoxaClassBased::ImportDeviceConfigStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  pImportDeviceConfigStateMachineType->signal_sender->set_value();

  return ret;
}
}  // namespace ClassBased