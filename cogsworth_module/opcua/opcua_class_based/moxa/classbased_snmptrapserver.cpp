#include "classbased_snmptrapserver.h"

namespace ClassBased {

static ACT_STATUS checkSNMPTrapServerNode(ActSnmpTrapSettingTable& snmp_trap_setting_table) {
  ACT_STATUS_INIT();

  QList<ActSnmpTrapHostEntry>& host_list = snmp_trap_setting_table.GetHostList();

  if (host_list.size() > 2) {
    QString error_msg = QString("Invalid: SNMP hosts cannot have more than 2 entries");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (ActSnmpTrapHostEntry host_entry : host_list) {
    if (host_entry.GetHostName() == QString()) {
      QString error_msg = QString("Invalid: The host name cannot be empty");
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    if (host_entry.GetTrapCommunity().length() < 4 || host_entry.GetTrapCommunity().length() > 32) {
      QString error_msg = QString("Invalid: The valid range of TrapCommunity is 4 to 32");
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  return act_status;
}

static bool clearSNMPTrapHostNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                  UaString& errorMessage) {
  UaStatus ret;

  UaNodeId deviceNodeId = pMoxaNodeManager->getDeviceNodeId(project.GetId(), device.GetId());
  MoxaClassBased::BridgeType* pBridgeType = (MoxaClassBased::BridgeType*)pMoxaNodeManager->getNode(deviceNodeId);
  MoxaClassBased::SNMPTrapServerType* pSNMPTrapServerType = pBridgeType->getDeviceConfig()->getSNMPTrapServer();

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(pSNMPTrapServerType->nodeId(), OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_SNMPTrapHostType) {
      continue;
    }
    UaNode* pNode = pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);

    ret = pMoxaNodeManager->deleteUaNode(pNode, OpcUa_True, OpcUa_True, OpcUa_True);
    if (ret.isNotGood()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Remove SNMP trap host node failed");
      qDebug() << errorMessage.toUtf8();
      return false;
    }
  }
  return true;
}

UaStatus updateSNMPTrapServerNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                  UaString& errorMessage) {
  UaStatus ret;

  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSNMPTrapSetting()) {
    return ret;
  }

  if (!clearSNMPTrapHostNode(project, device, errorCode, errorMessage)) {
    return ret;
  }

  UaNodeId deviceNodeId = pMoxaNodeManager->getDeviceNodeId(project.GetId(), device.GetId());
  MoxaClassBased::BridgeType* pBridgeType = (MoxaClassBased::BridgeType*)pMoxaNodeManager->getNode(deviceNodeId);
  MoxaClassBased::SNMPTrapServerType* pSNMPTrapServerType = pBridgeType->getDeviceConfig()->getSNMPTrapServer();

  const ActSnmpTrapSettingTable& snmp_trap_setting_table =
      project.GetDeviceConfig().GetSnmpTrapSettingTables()[device.GetId()];

  const QList<ActSnmpTrapHostEntry>& host_list = snmp_trap_setting_table.GetHostList();
  for (int i = 0; i < host_list.size(); i++) {
    const ActSnmpTrapHostEntry& host_entry = host_list[i];

    UaString browse_name = UaString("Host%1").arg(i + 1);
    UaNodeId SNMPTrapHostNodeId(UaString("%1.%2").arg(pSNMPTrapServerType->nodeId().toString()).arg(browse_name),
                                pMoxaNodeManager->getNameSpaceIndex());

    // If succeeded, create the new node to the OPC UA nodemanager
    MoxaClassBased::SNMPTrapHostType* pSNMPTrapHostType =
        new MoxaClassBased::SNMPTrapHostType(SNMPTrapHostNodeId, browse_name, pMoxaNodeManager->getNameSpaceIndex(),
                                             pMoxaNodeManager->getNodeManagerConfig());
    if (!pSNMPTrapHostType) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Allocate memory failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    pSNMPTrapHostType->setIndex(i + 1);
    pSNMPTrapHostType->setHostIP(host_entry.GetHostName().toStdString().c_str());
    switch (host_entry.GetMode()) {
      case ActSnmpTrapModeEnum::kTrapV1:
        pSNMPTrapHostType->setMode(MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_TrapV1);
        break;
      case ActSnmpTrapModeEnum::kTrapV2c:
        pSNMPTrapHostType->setMode(MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_TrapV2c);
        break;
      case ActSnmpTrapModeEnum::kInformV2c:
        pSNMPTrapHostType->setMode(MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_InformV2c);
        break;
      case ActSnmpTrapModeEnum::kNotSupport:
        pSNMPTrapHostType->setMode(MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_NotSupport);
        break;
    }
    pSNMPTrapHostType->setTrapCommunity(host_entry.GetTrapCommunity().toStdString().c_str());

    ret = pMoxaNodeManager->addNodeAndReference(pSNMPTrapServerType->nodeId(), pSNMPTrapHostType->getUaReferenceLists(),
                                                OpcUaId_HasComponent);
    if (ret.isNotGood()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Create SNMP trap host node failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }
  }

  return ret;
}

UaStatus addSNMPTrapHostMethod(const UaNodeId& SNMPTrapServerNodeId,
                               const MoxaClassBased::SNMPTrapHostDataType& configuration, UaNodeId& SNMPTrapHostNodeId,
                               OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(SNMPTrapServerNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(SNMPTrapServerNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActDeviceConfig& device_config = project.GetDeviceConfig();
  ActSnmpTrapSettingTable& snmp_trap_setting_table = device_config.GetSnmpTrapSettingTables()[device_id];
  QList<ActSnmpTrapHostEntry>& host_list = snmp_trap_setting_table.GetHostList();

  snmp_trap_setting_table.SetDeviceId(device_id);

  ActSnmpTrapHostEntry host_entry;
  if (configuration.isHostIPSet()) {
    host_entry.SetHostName(configuration.getHostIP().toUtf8());
  }
  if (configuration.isModeSet()) {
    switch (configuration.getMode()) {
      case MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_TrapV1:
        host_entry.SetMode(ActSnmpTrapModeEnum::kTrapV1);
        break;
      case MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_TrapV2c:
        host_entry.SetMode(ActSnmpTrapModeEnum::kTrapV2c);
        break;
      case MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_InformV2c:
        host_entry.SetMode(ActSnmpTrapModeEnum::kInformV2c);
        break;
      case MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_NotSupport:
        host_entry.SetMode(ActSnmpTrapModeEnum::kNotSupport);
        break;
    }
  }
  if (configuration.isTrapCommunitySet()) {
    host_entry.SetTrapCommunity(configuration.getTrapCommunity().toUtf8());
  }
  host_list.append(host_entry);

  act_status = checkSNMPTrapServerNode(snmp_trap_setting_table);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(SNMPTrapServerNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_SNMPTrapHostType) {
      continue;
    }
    MoxaClassBased::SNMPTrapHostType* pSNMPTrapHostType =
        (MoxaClassBased::SNMPTrapHostType*)pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);

    MoxaClassBased::SNMPTrapModeEnumType mode;
    switch (host_entry.GetMode()) {
      case ActSnmpTrapModeEnum::kTrapV1:
        mode = MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_TrapV1;
        break;
      case ActSnmpTrapModeEnum::kTrapV2c:
        mode = MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_TrapV2c;
        break;
      case ActSnmpTrapModeEnum::kInformV2c:
        mode = MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_InformV2c;
        break;
      default:
        mode = MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_NotSupport;
        break;
    }

    if (pSNMPTrapHostType->getHostIP() != UaString(host_entry.GetHostName().toStdString().c_str()) ||
        pSNMPTrapHostType->getTrapCommunity() != UaString(host_entry.GetTrapCommunity().toStdString().c_str()) ||
        pSNMPTrapHostType->getMode() != mode) {
      continue;
    }

    SNMPTrapHostNodeId = pSNMPTrapHostType->nodeId();
    break;
  }

  return ret;
}

UaStatus setSNMPTrapHostMethod(const UaNodeId& SNMPTrapHostNodeId,
                               const MoxaClassBased::SNMPTrapHostDataType& configuration, OpcUa_UInt32& errorCode,
                               UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(SNMPTrapHostNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(SNMPTrapHostNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  MoxaClassBased::SNMPTrapHostType* pSNMPTrapHostType =
      (MoxaClassBased::SNMPTrapHostType*)pMoxaNodeManager->getNode(SNMPTrapHostNodeId);
  int index = pSNMPTrapHostType->getIndex() - 1;

  ActDeviceConfig& device_config = project.GetDeviceConfig();
  ActSnmpTrapSettingTable& snmp_trap_setting_table = device_config.GetSnmpTrapSettingTables()[device_id];
  QList<ActSnmpTrapHostEntry>& host_list = snmp_trap_setting_table.GetHostList();
  ActSnmpTrapHostEntry& host_entry = host_list[index];

  if (configuration.isHostIPSet()) {
    host_entry.SetHostName(configuration.getHostIP().toUtf8());
  }
  if (configuration.isModeSet()) {
    switch (configuration.getMode()) {
      case MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_TrapV1:
        host_entry.SetMode(ActSnmpTrapModeEnum::kTrapV1);
        break;
      case MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_TrapV2c:
        host_entry.SetMode(ActSnmpTrapModeEnum::kTrapV2c);
        break;
      case MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_InformV2c:
        host_entry.SetMode(ActSnmpTrapModeEnum::kInformV2c);
        break;
      case MoxaClassBased::SNMPTrapModeEnumType::SNMPTrapModeEnumType_NotSupport:
        host_entry.SetMode(ActSnmpTrapModeEnum::kNotSupport);
        break;
    }
  }
  if (configuration.isTrapCommunitySet()) {
    host_entry.SetTrapCommunity(configuration.getTrapCommunity().toUtf8());
  }

  act_status = checkSNMPTrapServerNode(snmp_trap_setting_table);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}

UaStatus removeSNMPTrapHostMethod(const UaNodeId& SNMPTrapServerNodeId, const UaNodeId& SNMPTrapHostNodeId,
                                  OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(SNMPTrapServerNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(SNMPTrapServerNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  UaReferenceDescriptions references;
  MoxaClassBased::SNMPTrapHostType* pSNMPTrapHostType = NULL;
  pMoxaNodeManager->getNodeReference(SNMPTrapServerNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_SNMPTrapHostType) {
      continue;
    }

    if (UaNodeId(references[idx].NodeId.NodeId) == SNMPTrapHostNodeId) {
      pSNMPTrapHostType = (MoxaClassBased::SNMPTrapHostType*)pMoxaNodeManager->getNode(SNMPTrapHostNodeId);
      break;
    }
  }

  if (pSNMPTrapHostType == NULL) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: NodeId %1 is not found").arg(SNMPTrapHostNodeId.toFullString());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActDeviceConfig& device_config = project.GetDeviceConfig();
  ActSnmpTrapSettingTable& snmp_trap_setting_table = device_config.GetSnmpTrapSettingTables()[device_id];
  QList<ActSnmpTrapHostEntry>& host_list = snmp_trap_setting_table.GetHostList();

  int index = int(pSNMPTrapHostType->getIndex()) - 1;
  host_list.removeAt(index);

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}
}  // namespace ClassBased