#include "classbased_firmwareupgrade.h"

namespace ClassBased {

UaStatus startFirmwareUpgradeMethod(const UaNodeId& nodeId, const UaString& filePath) {
  UaStatus ret;
  ACT_STATUS_INIT();

  MoxaClassBased::FirmwareUpgradeStateMachineType* pFirmwareUpgradeStateMachineType =
      (MoxaClassBased::FirmwareUpgradeStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  switch (pFirmwareUpgradeStateMachineType->getProgramState()) {
    case MoxaClassBased::FirmwareUpgradeStateMachineType::Completed:
    case MoxaClassBased::FirmwareUpgradeStateMachineType::Failed:
    case MoxaClassBased::FirmwareUpgradeStateMachineType::Ready:
      pFirmwareUpgradeStateMachineType->file_path = filePath;
      pFirmwareUpgradeStateMachineType->setErrorCode(M_UA_NO_ERROR);
      pFirmwareUpgradeStateMachineType->setErrorMessage(UaString(""));
      pFirmwareUpgradeStateMachineType->setProgramState(MoxaClassBased::FirmwareUpgradeStateMachineType::Running);
      pFirmwareUpgradeStateMachineType->start();
      break;
    default:
      ret = OpcUa_BadNotExecutable;
      break;
  }
  return ret;
}

void runFirmwareUpgradeCb(ACT_STATUS status, void* arg) {
  ((ActStatusBase*)arg)->SetStatus(status->GetStatus());
  ((ActStatusBase*)arg)->SetErrorMessage(status->GetErrorMessage());
}

void runFirmwareUpgradeMethod(const UaNodeId& nodeId) {
  ACT_STATUS_INIT();

  MoxaClassBased::FirmwareUpgradeStateMachineType* pFirmwareUpgradeStateMachineType =
      (MoxaClassBased::FirmwareUpgradeStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  // Create a std::promise object
  pFirmwareUpgradeStateMachineType->signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = pFirmwareUpgradeStateMachineType->signal_sender->get_future();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  QList<qint64> dev_id_list = {pMoxaNodeManager->getDeviceId(nodeId)};
  QString file_path = QString(pFirmwareUpgradeStateMachineType->file_path.toUtf8());

  ActStatusBase status(ActStatusType::kRunning);
  act_status = act::core::g_core.OpcUaStartConfigFirmwareUpgrade(
      project_id, dev_id_list, file_path, std::move(signal_receiver), runFirmwareUpgradeCb, (void*)&status);
  if (!IsActStatusSuccess(act_status)) {
    // Set failed state
    pFirmwareUpgradeStateMachineType->setProgramState(MoxaClassBased::FirmwareUpgradeStateMachineType::Failed);

    // Set error message
    pFirmwareUpgradeStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pFirmwareUpgradeStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
    return;
  }

  // wait for computing
  while (status.GetStatus() == ActStatusType::kRunning) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (status.GetStatus() == ActStatusType::kFinished || status.GetStatus() == ActStatusType::kSuccess) {
    // Set completed state
    pFirmwareUpgradeStateMachineType->setProgramState(MoxaClassBased::FirmwareUpgradeStateMachineType::Completed);
  } else if (status.GetStatus() == ActStatusType::kStop) {
    // Set ready state
    pFirmwareUpgradeStateMachineType->setProgramState(MoxaClassBased::FirmwareUpgradeStateMachineType::Ready);
  } else {
    // Set failed state
    pFirmwareUpgradeStateMachineType->setProgramState(MoxaClassBased::FirmwareUpgradeStateMachineType::Failed);

    // Set error message
    pFirmwareUpgradeStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(status.GetStatus()));
    pFirmwareUpgradeStateMachineType->setErrorMessage(UaString(status.GetErrorMessage().toStdString().c_str()));
  }

  return;
}

UaStatus stopFirmwareUpgradeMethod(const UaNodeId& nodeId) {
  UaStatus ret;

  MoxaClassBased::FirmwareUpgradeStateMachineType* pFirmwareUpgradeStateMachineType =
      (MoxaClassBased::FirmwareUpgradeStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  pFirmwareUpgradeStateMachineType->signal_sender->set_value();

  return ret;
}
}  // namespace ClassBased