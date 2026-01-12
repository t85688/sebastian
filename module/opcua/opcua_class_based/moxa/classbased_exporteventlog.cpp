#include "classbased_exporteventlog.h"

namespace ClassBased {

UaStatus startExportEventLogMethod(const UaNodeId& nodeId, const UaString& filePath) {
  UaStatus ret;
  ACT_STATUS_INIT();

  MoxaClassBased::ExportEventLogStateMachineType* pExportEventLogStateMachineType =
      (MoxaClassBased::ExportEventLogStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  switch (pExportEventLogStateMachineType->getProgramState()) {
    case MoxaClassBased::ExportEventLogStateMachineType::Completed:
    case MoxaClassBased::ExportEventLogStateMachineType::Failed:
    case MoxaClassBased::ExportEventLogStateMachineType::Ready:
      pExportEventLogStateMachineType->filePath = filePath;
      pExportEventLogStateMachineType->setErrorCode(M_UA_NO_ERROR);
      pExportEventLogStateMachineType->setErrorMessage(UaString(""));
      pExportEventLogStateMachineType->setProgramState(MoxaClassBased::ExportEventLogStateMachineType::Running);
      pExportEventLogStateMachineType->start();
      break;
    default:
      ret = OpcUa_BadNotExecutable;
      break;
  }
  return ret;
}

void runExportEventLogCb(ACT_STATUS status, void* arg) {
  ((ActStatusBase*)arg)->SetStatus(status->GetStatus());
  ((ActStatusBase*)arg)->SetErrorMessage(status->GetErrorMessage());
}

void runExportEventLogMethod(const UaNodeId& nodeId) {
  ACT_STATUS_INIT();

  MoxaClassBased::ExportEventLogStateMachineType* pExportEventLogStateMachineType =
      (MoxaClassBased::ExportEventLogStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  // Create a std::promise object
  pExportEventLogStateMachineType->signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = pExportEventLogStateMachineType->signal_sender->get_future();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  QList<qint64> dev_id_list = {pMoxaNodeManager->getDeviceId(nodeId)};
  QString export_path(pExportEventLogStateMachineType->filePath.toUtf8());

  ActStatusBase status(ActStatusType::kRunning);
  act_status = act::core::g_core.OpcUaStartOperationEventLog(
      project_id, dev_id_list, export_path, std::move(signal_receiver), runExportEventLogCb, (void*)&status);
  if (!IsActStatusSuccess(act_status)) {
    // Set failed state
    pExportEventLogStateMachineType->setProgramState(MoxaClassBased::ExportEventLogStateMachineType::Failed);

    // Set error message
    pExportEventLogStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pExportEventLogStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
    return;
  }

  // wait for computing
  while (status.GetStatus() == ActStatusType::kRunning) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (status.GetStatus() == ActStatusType::kFinished || status.GetStatus() == ActStatusType::kSuccess) {
    // Set completed state
    pExportEventLogStateMachineType->setProgramState(MoxaClassBased::ExportEventLogStateMachineType::Completed);
  } else if (status.GetStatus() == ActStatusType::kStop) {
    // Set ready state
    pExportEventLogStateMachineType->setProgramState(MoxaClassBased::ExportEventLogStateMachineType::Ready);
  } else {
    // Set failed state
    pExportEventLogStateMachineType->setProgramState(MoxaClassBased::ExportEventLogStateMachineType::Failed);

    // Set error message
    pExportEventLogStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(status.GetStatus()));
    pExportEventLogStateMachineType->setErrorMessage(UaString(status.GetErrorMessage().toStdString().c_str()));
  }

  return;
}

UaStatus stopExportEventLogMethod(const UaNodeId& nodeId) {
  UaStatus ret;

  MoxaClassBased::ExportEventLogStateMachineType* pExportEventLogStateMachineType =
      (MoxaClassBased::ExportEventLogStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  pExportEventLogStateMachineType->signal_sender->set_value();

  return ret;
}
}  // namespace ClassBased