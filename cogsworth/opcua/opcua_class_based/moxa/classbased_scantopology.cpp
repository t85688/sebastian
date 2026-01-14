#include "classbased_deploy.h"

namespace ClassBased {

UaStatus startScanTopologyMethod(const UaNodeId& nodeId, const MoxaClassBased::AutoScanDataTypes& NetworkIpRange) {
  UaStatus ret;
  ACT_STATUS_INIT();

  QObject::disconnect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsError, nullptr, nullptr);
  QObject::disconnect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsMessage, nullptr, nullptr);

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  MoxaClassBased::ScanTopologyStateMachineType* pScanTopologyStateMachineType =
      (MoxaClassBased::ScanTopologyStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  QList<ActScanIpRangeEntry> scanIpRangeEntryLists;
  for (OpcUa_UInt32 idx = 0; idx < NetworkIpRange.length(); idx++) {
    MoxaClassBased::AutoScanDataType autoScanDataType(NetworkIpRange[idx]);

    // IP range
    UaString firstIpAddress(autoScanDataType.getFirstIpAddress());
    UaString lastIpAddress(autoScanDataType.getLastIpAddress());

    // Device Account
    ActDeviceAccount deviceAccountCfg;

    // SNMP
    ActSnmpConfiguration snmpCfg;
    if (autoScanDataType.isSNMPSet()) {
      MoxaClassBased::SNMPDataType SNMP(autoScanDataType.getSNMP());
      snmpCfg.SetVersion(pMoxaNodeManager->setSnmpVersion(SNMP.getVersion()));
      snmpCfg.SetPort(quint16(SNMP.getPort()));
      snmpCfg.SetReadCommunity(QString(SNMP.getReadCommunity().toUtf8()));
      snmpCfg.SetWriteCommunity(QString(SNMP.getWriteCommunity().toUtf8()));
      snmpCfg.SetDefaultSetting(false);
    }

    // NETCONF
    ActNetconfConfiguration netconfConf;
    if (autoScanDataType.isNETCONFSet()) {
      MoxaClassBased::NETCONFDataType NETCONF(autoScanDataType.getNETCONF());
      ActNetconfOverSSH netconfSSH;
      netconfConf.SetNetconfOverSSH(netconfSSH);
      netconfConf.SetDefaultSetting(false);
    }

    // RESTful
    ActRestfulConfiguration restfulCfg;
    if (autoScanDataType.isRESTfulSet()) {
      MoxaClassBased::RESTfulDataType RESTful(autoScanDataType.getRESTful());
      restfulCfg.SetPort(quint16(RESTful.getPort()));
      restfulCfg.SetDefaultSetting(false);
    }

    // Append to scan range list
    bool enable_snmp_setting = true;
    bool auto_probe = true;
    scanIpRangeEntryLists.append(ActScanIpRangeEntry(qint64(idx), firstIpAddress.toUtf8(), lastIpAddress.toUtf8(),
                                                     deviceAccountCfg, snmpCfg, netconfConf, restfulCfg,
                                                     enable_snmp_setting, auto_probe));
  }

  ActProjectSetting project_setting;
  project_setting.SetScanIpRanges(scanIpRangeEntryLists);
  act_status =
      act::core::g_core.UpdateProjectSettingMember(ActProjectSettingMember::kScanIpRanges, project_id, project_setting);
  if (!IsActStatusSuccess(act_status)) {
    pScanTopologyStateMachineType->setProgramState(MoxaClassBased::ScanTopologyStateMachineType::Failed);
    pScanTopologyStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pScanTopologyStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
    return ret;
  }

  switch (pScanTopologyStateMachineType->getProgramState()) {
    case MoxaClassBased::ScanTopologyStateMachineType::Completed:
    case MoxaClassBased::ScanTopologyStateMachineType::Failed:
    case MoxaClassBased::ScanTopologyStateMachineType::Ready:
      pScanTopologyStateMachineType->setErrorCode(M_UA_NO_ERROR);
      pScanTopologyStateMachineType->setErrorMessage(UaString(""));
      pScanTopologyStateMachineType->setProgramState(MoxaClassBased::ScanTopologyStateMachineType::Ready);
      pScanTopologyStateMachineType->start();
      break;
    default:
      ret = OpcUa_BadNotExecutable;
      break;
  }
  return ret;
}

void runScanTopologyMethod(const UaNodeId& nodeId) {
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  MoxaClassBased::ScanTopologyStateMachineType* pScanTopologyStateMachineType =
      (MoxaClassBased::ScanTopologyStateMachineType*)pMoxaNodeManager->getNode(nodeId);

  pScanTopologyStateMachineType->setProgramState(MoxaClassBased::ScanTopologyStateMachineType::Running);

  QObject::connect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsMessage, [=](const QString& message) {
    ScanTopologyWsResponse response;
    try {
      // Do patch parser
      response.FromString(message);
      ActStatusType status_code = static_cast<ActStatusType>(response.GetStatusCode());
      switch (status_code) {
        case ActStatusType::kRunning:
          pScanTopologyStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Running);
          break;
        case ActStatusType::kStop:
          pScanTopologyStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Ready);
          break;
        case ActStatusType::kSuccess:
        case ActStatusType::kFinished:
          pScanTopologyStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Completed);
          break;
        case ActStatusType::kFailed:
        case ActStatusType::kBadRequest:
        case ActStatusType::kUnauthorized:
        case ActStatusType::kForbidden:
        case ActStatusType::kNotFound:
        case ActStatusType::kConflict:
        case ActStatusType::kUnProcessable:
        case ActStatusType::kInternalError:
        case ActStatusType::kServiceUnavailable:
          pScanTopologyStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Failed);
          pScanTopologyStateMachineType->setErrorCode(OpcUa_UInt32(response.GetStatusCode()));
          pScanTopologyStateMachineType->setErrorMessage(UaString(response.GetErrorMessage().toStdString().c_str()));
          break;
        default:
          pScanTopologyStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Failed);
          pScanTopologyStateMachineType->setErrorCode(OpcUa_UInt32(ActStatusType::kFailed));
          pScanTopologyStateMachineType->setErrorMessage(
              UaString("Unknown status code %1").arg(response.GetStatusCode()));
          break;
      }
      if (status_code != ActStatusType::kRunning) {
        QObject::disconnect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsError, nullptr, nullptr);
        QObject::disconnect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsMessage, nullptr, nullptr);
      }
    } catch (std::exception& e) {
      pScanTopologyStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Failed);
      pScanTopologyStateMachineType->setErrorCode(OpcUa_UInt32(ActStatusType::kFailed));
      pScanTopologyStateMachineType->setErrorMessage(e.what());

      QObject::disconnect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsError, nullptr, nullptr);
      QObject::disconnect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsMessage, nullptr, nullptr);
    }
  });

  QObject::connect(pMoxaNodeManager->getWsBridge(), &WsBridge::handleWsError, [=](const QString& message) {
    pScanTopologyStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Failed);
    pScanTopologyStateMachineType->setErrorCode(OpcUa_UInt32(ActStatusType::kFailed));
    pScanTopologyStateMachineType->setErrorMessage(message.toStdString().c_str());
  });

  ScanTopologyWsCommand scan_topology_ws_cmd(ActWSCommandEnum::kStartScanTopology, project_id, true);
  pMoxaNodeManager->sendWsFromUaThread(scan_topology_ws_cmd.ToString().toStdString().c_str());

  return;
}

UaStatus stopScanTopologyMethod(const UaNodeId& nodeId) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);

  WsBaseCommand ws_cmd(ActWSCommandEnum::kStopScanTopology, project_id);
  pMoxaNodeManager->sendWsFromUaThread(ws_cmd.ToString().toStdString().c_str());

  MoxaClassBased::ScanTopologyStateMachineType* pScanTopologyStateMachineType =
      (MoxaClassBased::ScanTopologyStateMachineType*)pMoxaNodeManager->getNode(nodeId);
  act_status = act::core::g_core.UpdateProjectStatus(project_id, ActProjectStatusEnum::kIdle);
  if (!IsActStatusSuccess(act_status)) {
    pScanTopologyStateMachineType->setProgramState(MoxaClassBased::DeployStateMachineType::Failed);
    pScanTopologyStateMachineType->setErrorCode(static_cast<OpcUa_UInt32>(act_status->GetStatus()));
    pScanTopologyStateMachineType->setErrorMessage(UaString(act_status->GetErrorMessage().toStdString().c_str()));
  } else {
    pScanTopologyStateMachineType->setProgramState(MoxaClassBased::ScanTopologyStateMachineType::Ready);
    pScanTopologyStateMachineType->setErrorCode(M_UA_NO_ERROR);
    pScanTopologyStateMachineType->setErrorMessage(UaString(""));
  }

  return ret;
}
}  // namespace ClassBased