#include "classbased_exportdeviceconfig.h"

namespace ClassBased {

UaStatus startExportDeviceConfigMethod(const UaNodeId& nodeId, const UaString& filePath) {
  UaStatus ret;
  ACT_STATUS_INIT();

  MoxaClassBased::ExportDeviceConfigStateMachineType* pExportDeviceConfigStateMachineType =
      (MoxaClassBased::ExportDeviceConfigStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  switch (pExportDeviceConfigStateMachineType->getProgramState()) {
    case MoxaClassBased::ExportDeviceConfigStateMachineType::Completed:
    case MoxaClassBased::ExportDeviceConfigStateMachineType::Failed:
    case MoxaClassBased::ExportDeviceConfigStateMachineType::Ready:
      pExportDeviceConfigStateMachineType->filePath = filePath;
      pExportDeviceConfigStateMachineType->setErrorCode(M_UA_NO_ERROR);
      pExportDeviceConfigStateMachineType->setErrorMessage(UaString(""));
      pExportDeviceConfigStateMachineType->setProgramState(MoxaClassBased::ExportDeviceConfigStateMachineType::Running);
      pExportDeviceConfigStateMachineType->start();
      break;
    default:
      ret = OpcUa_BadNotExecutable;
      break;
  }
  return ret;
}

void runExportDeviceConfigCb(ACT_STATUS status, void* arg) {
  ((ActStatusBase*)arg)->SetStatus(status->GetStatus());
  ((ActStatusBase*)arg)->SetErrorMessage(status->GetErrorMessage());
}

void runExportDeviceConfigMethod(const UaNodeId& nodeId) {
  ACT_STATUS_INIT();

  MoxaClassBased::ExportDeviceConfigStateMachineType* pExportDeviceConfigStateMachineType =
      (MoxaClassBased::ExportDeviceConfigStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  // Create a std::promise object
  pExportDeviceConfigStateMachineType->signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = pExportDeviceConfigStateMachineType->signal_sender->get_future();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  QList<qint64> dev_id_list = {pMoxaNodeManager->getDeviceId(nodeId)};
  QString export_path(pExportDeviceConfigStateMachineType->filePath.toUtf8());

  ActStatusBase status(ActStatusType::kRunning);
  act_status = act::core::g_core.OpcUaStartExportDeviceConfig(
      project_id, dev_id_list, export_path, std::move(signal_receiver), runExportDeviceConfigCb, (void*)&status);
  if (!IsActStatusSuccess(act_status)) {
    // Set failed state
    pExportDeviceConfigStateMachineType->setProgramState(MoxaClassBased::ExportDeviceConfigStateMachineType::Failed);

    // Set error message
    pExportDeviceConfigStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pExportDeviceConfigStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
    return;
  }

  // wait for computing
  while (status.GetStatus() == ActStatusType::kRunning) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (status.GetStatus() == ActStatusType::kFinished || status.GetStatus() == ActStatusType::kSuccess) {
    // Set completed state
    pExportDeviceConfigStateMachineType->setProgramState(MoxaClassBased::ExportDeviceConfigStateMachineType::Completed);
  } else if (status.GetStatus() == ActStatusType::kStop) {
    // Set ready state
    pExportDeviceConfigStateMachineType->setProgramState(MoxaClassBased::ExportDeviceConfigStateMachineType::Ready);
  } else {
    // Set failed state
    pExportDeviceConfigStateMachineType->setProgramState(MoxaClassBased::ExportDeviceConfigStateMachineType::Failed);

    // Set error message
    pExportDeviceConfigStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(status.GetStatus()));
    pExportDeviceConfigStateMachineType->setErrorMessage(UaString(status.GetErrorMessage().toStdString().c_str()));
  }

  return;
}

UaStatus stopExportDeviceConfigMethod(const UaNodeId& nodeId) {
  UaStatus ret;

  MoxaClassBased::ExportDeviceConfigStateMachineType* pExportDeviceConfigStateMachineType =
      (MoxaClassBased::ExportDeviceConfigStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  pExportDeviceConfigStateMachineType->signal_sender->set_value();

  return ret;
}
}  // namespace ClassBased