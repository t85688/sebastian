#include "classbased_exportsyslog.h"

namespace ClassBased {

UaStatus startExportSyslogMethod(const UaNodeId& nodeId, const UaString& filePath) {
  UaStatus ret;
  ACT_STATUS_INIT();

  MoxaClassBased::ExportSyslogStateMachineType* pExportSyslogStateMachineType =
      (MoxaClassBased::ExportSyslogStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  switch (pExportSyslogStateMachineType->getProgramState()) {
    case MoxaClassBased::ExportSyslogStateMachineType::Completed:
    case MoxaClassBased::ExportSyslogStateMachineType::Failed:
    case MoxaClassBased::ExportSyslogStateMachineType::Ready:
      pExportSyslogStateMachineType->filePath = filePath;
      pExportSyslogStateMachineType->setErrorCode(M_UA_NO_ERROR);
      pExportSyslogStateMachineType->setErrorMessage(UaString(""));
      pExportSyslogStateMachineType->setProgramState(MoxaClassBased::ExportSyslogStateMachineType::Running);
      pExportSyslogStateMachineType->start();
      break;
    default:
      ret = OpcUa_BadNotExecutable;
      break;
  }
  return ret;
}

void runExportSyslogMethod(const UaNodeId& nodeId) {
  ACT_STATUS_INIT();

  MoxaClassBased::ExportSyslogStateMachineType* pExportSyslogStateMachineType =
      (MoxaClassBased::ExportSyslogStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  QString export_path(pExportSyslogStateMachineType->filePath.toUtf8());

  act_status = act::core::g_core.SaveSyslogsAsCsv(export_path);
  if (!IsActStatusSuccess(act_status)) {
    // Set failed state
    pExportSyslogStateMachineType->setProgramState(MoxaClassBased::ExportSyslogStateMachineType::Failed);

    // Set error message
    pExportSyslogStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pExportSyslogStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
    return;
  }

  pExportSyslogStateMachineType->setProgramState(MoxaClassBased::ExportSyslogStateMachineType::Completed);

  return;
}

UaStatus stopExportSyslogMethod(const UaNodeId& nodeId) {
  UaStatus ret;

  MoxaClassBased::ExportSyslogStateMachineType* pExportSyslogStateMachineType =
      (MoxaClassBased::ExportSyslogStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  pExportSyslogStateMachineType->setProgramState(MoxaClassBased::ExportSyslogStateMachineType::Ready);

  return ret;
}
}  // namespace ClassBased