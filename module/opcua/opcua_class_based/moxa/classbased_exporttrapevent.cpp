#include "classbased_exporttrapevent.h"

namespace ClassBased {

UaStatus startExportTrapEventMethod(const UaNodeId& nodeId, const UaString& filePath) {
  UaStatus ret;
  ACT_STATUS_INIT();

  MoxaClassBased::ExportTrapEventStateMachineType* pExportTrapEventStateMachineType =
      (MoxaClassBased::ExportTrapEventStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  switch (pExportTrapEventStateMachineType->getProgramState()) {
    case MoxaClassBased::ExportTrapEventStateMachineType::Completed:
    case MoxaClassBased::ExportTrapEventStateMachineType::Failed:
    case MoxaClassBased::ExportTrapEventStateMachineType::Ready:
      pExportTrapEventStateMachineType->filePath = filePath;
      pExportTrapEventStateMachineType->setErrorCode(M_UA_NO_ERROR);
      pExportTrapEventStateMachineType->setErrorMessage(UaString(""));
      pExportTrapEventStateMachineType->setProgramState(MoxaClassBased::ExportTrapEventStateMachineType::Running);
      pExportTrapEventStateMachineType->start();
      break;
    default:
      ret = OpcUa_BadNotExecutable;
      break;
  }
  return ret;
}

void runExportTrapEventMethod(const UaNodeId& nodeId) {
  ACT_STATUS_INIT();

  MoxaClassBased::ExportTrapEventStateMachineType* pExportTrapEventStateMachineType =
      (MoxaClassBased::ExportTrapEventStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  QString export_path(pExportTrapEventStateMachineType->filePath.toUtf8());

  act_status = act::core::g_core.SaveEventsAsCsv(export_path);
  if (!IsActStatusSuccess(act_status)) {
    // Set failed state
    pExportTrapEventStateMachineType->setProgramState(MoxaClassBased::ExportTrapEventStateMachineType::Failed);

    // Set error message
    pExportTrapEventStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pExportTrapEventStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
    return;
  }

  pExportTrapEventStateMachineType->setProgramState(MoxaClassBased::ExportTrapEventStateMachineType::Completed);

  return;
}

UaStatus stopExportTrapEventMethod(const UaNodeId& nodeId) {
  UaStatus ret;

  MoxaClassBased::ExportTrapEventStateMachineType* pExportTrapEventStateMachineType =
      (MoxaClassBased::ExportTrapEventStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  pExportTrapEventStateMachineType->setProgramState(MoxaClassBased::ExportTrapEventStateMachineType::Ready);

  return ret;
}
}  // namespace ClassBased