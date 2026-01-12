#include "classbased_deviceaccount.h"

namespace ClassBased {

static ACT_STATUS checkDeviceAccountNode(ActUserAccount& user_account) {
  ACT_STATUS_INIT();

  if (user_account.GetUsername().length() < 4 || user_account.GetUsername().length() > 32) {
    QString error_msg = QString("Invalid: The valid range of UserName is 4 to 32");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (user_account.GetPassword().length() < 4 || user_account.GetPassword().length() > 63) {
    QString error_msg = QString("Invalid: The valid range of Password is 4 to 63");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (user_account.GetEmail().length() > 63) {
    QString error_msg = QString("Invalid: The valid range of Email is less than 63");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

UaStatus updateDeviceAccountNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                 UaString& errorMessage) {
  UaStatus ret;

  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetUserAccount()) {
    return ret;
  }

  UaNodeId deviceNodeId = pMoxaNodeManager->getDeviceNodeId(project.GetId(), device.GetId());
  MoxaClassBased::BridgeType* pBridgeType = (MoxaClassBased::BridgeType*)pMoxaNodeManager->getNode(deviceNodeId);
  MoxaClassBased::DeviceAccountFolderType* pDeviceAccountFolderType =
      pBridgeType->getDeviceConfig()->getDeviceAccounts();

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(pDeviceAccountFolderType->nodeId(), OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_DeviceAccountType) {
      continue;
    }
    UaNode* pNode = pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);
    ret = pMoxaNodeManager->deleteUaNode(pNode, OpcUa_True, OpcUa_True, OpcUa_True);
    if (ret.isNotGood()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Remove device %1 failed").arg(pNode->browseName().toString());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }
  }

  const ActUserAccountTable& user_account_table = project.GetDeviceConfig().GetUserAccountTables()[device.GetId()];

  for (ActUserAccount user_account : user_account_table.GetAccounts().values()) {
    UaString browse_name = UaString(user_account.GetUsername().toStdString().c_str());

    // NodeId for the node to create
    UaNodeId deviceAccountNodeId(UaString("%1.%2").arg(pDeviceAccountFolderType->nodeId().toString()).arg(browse_name),
                                 pMoxaNodeManager->getNameSpaceIndex());

    MoxaClassBased::DeviceAccountType* pDeviceAccountType =
        new MoxaClassBased::DeviceAccountType(deviceAccountNodeId, browse_name, pMoxaNodeManager->getNameSpaceIndex(),
                                              pMoxaNodeManager->getNodeManagerConfig());
    if (!pDeviceAccountType) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Allocate memory failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    ret = pMoxaNodeManager->addNodeAndReference(pDeviceAccountFolderType->nodeId(),
                                                pDeviceAccountType->getUaReferenceLists(), OpcUaId_HasComponent);
    if (ret.isNotGood()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Create device account node failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    pDeviceAccountType->setUserName(user_account.GetUsername().toStdString().c_str());
    pDeviceAccountType->setEmail(user_account.GetEmail().toStdString().c_str());
    switch (user_account.GetRole()) {
      case ActUserAccountRoleEnum::kAdmin:
        pDeviceAccountType->setAuthority(MoxaClassBased::AuthorityEnumType::AuthorityEnumType_Admin);
        break;
      case ActUserAccountRoleEnum::kSupervisor:
        pDeviceAccountType->setAuthority(MoxaClassBased::AuthorityEnumType::AuthorityEnumType_Supervisor);
        break;
      case ActUserAccountRoleEnum::kUser:
        pDeviceAccountType->setAuthority(MoxaClassBased::AuthorityEnumType::AuthorityEnumType_User);
        break;
    }
    pDeviceAccountType->setSyncToConnectionAccount(user_account.GetUsername() ==
                                                   user_account_table.GetSyncConnectionAccount());
  }

  return ret;
}

UaStatus addDeviceAccountMethod(const UaNodeId& deviceAccountFolderNodeId,
                                const MoxaClassBased::DeviceAccountDataType& configuration,
                                UaNodeId& deviceAccountNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(deviceAccountFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_DeviceAccountType) {
      continue;
    }
    MoxaClassBased::DeviceAccountType* pDeviceAccountType =
        (MoxaClassBased::DeviceAccountType*)pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);

    if (configuration.getUserName() == pDeviceAccountType->getUserName()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: User name %1 is already in use").arg(configuration.getUserName());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }
  }

  ActUserAccount user_account;
  user_account.SetUsername(configuration.getUserName().toUtf8());
  if (!configuration.isPasswordSet()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Need to set password");
    qDebug() << errorMessage.toUtf8();
    return ret;
  }
  user_account.SetPassword(configuration.getPassword().toUtf8());

  if (configuration.isEmailSet()) {
    user_account.SetEmail(configuration.getEmail().toUtf8());
  }
  if (configuration.isAuthoritySet()) {
    switch (configuration.getAuthority()) {
      case MoxaClassBased::AuthorityEnumType::AuthorityEnumType_Admin:
        user_account.SetRole(ActUserAccountRoleEnum::kAdmin);
        break;
      case MoxaClassBased::AuthorityEnumType::AuthorityEnumType_Supervisor:
        user_account.SetRole(ActUserAccountRoleEnum::kSupervisor);
        break;
      case MoxaClassBased::AuthorityEnumType::AuthorityEnumType_User:
        user_account.SetRole(ActUserAccountRoleEnum::kUser);
        break;
    }
  }

  qint64 project_id = pMoxaNodeManager->getProjectId(deviceAccountFolderNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(deviceAccountFolderNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActUserAccountTable& user_account_table = project.GetDeviceConfig().GetUserAccountTables()[device_id];
  if (configuration.isSyncToConnectionAccountSet() && configuration.getSyncToConnectionAccount()) {
    user_account_table.SetSyncConnectionAccount(configuration.getUserName().toUtf8());
  }

  user_account_table.GetAccounts().insert(user_account.GetUsername(), user_account);

  act_status = checkDeviceAccountNode(user_account);
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

  UaString browse_name = UaString(user_account.GetUsername().toStdString().c_str());
  deviceAccountNodeId = UaNodeId(UaString("%1.%2").arg(deviceAccountFolderNodeId.toString()).arg(browse_name),
                                 pMoxaNodeManager->getNameSpaceIndex());

  return ret;
}

UaStatus setDeviceAccountMethod(const UaNodeId& deviceAccountFolderNodeId,
                                const MoxaClassBased::DeviceAccountDataType& configuration, OpcUa_UInt32& errorCode,
                                UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  UaString browse_name = configuration.getUserName();

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(deviceAccountFolderNodeId, OpcUa_False, references);
  MoxaClassBased::DeviceAccountType* pDeviceAccountType = NULL;
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_DeviceAccountType) {
      continue;
    }
    UaNode* pNode = pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);

    if (pNode->browseName().toString() == browse_name) {
      pDeviceAccountType = (MoxaClassBased::DeviceAccountType*)pNode;
      break;
    }
  }

  if (pDeviceAccountType == NULL) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: User name %1 is not found").arg(configuration.getUserName());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  qint64 project_id = pMoxaNodeManager->getProjectId(deviceAccountFolderNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(deviceAccountFolderNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActDevice device;
  act_status = project.GetDeviceById(device, device_id);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActUserAccountTable& user_account_table = project.GetDeviceConfig().GetUserAccountTables()[device_id];
  if (configuration.isSyncToConnectionAccountSet() && configuration.getSyncToConnectionAccount()) {
    user_account_table.SetSyncConnectionAccount(configuration.getUserName().toUtf8());
  }

  ActUserAccount& user_account = user_account_table.GetAccounts()[configuration.getUserName().toUtf8()];
  user_account.SetUsername(configuration.getUserName().toUtf8());
  if (configuration.isPasswordSet()) {
    user_account.SetPassword(configuration.getPassword().toUtf8());
  }
  if (configuration.isEmailSet()) {
    user_account.SetEmail(configuration.getEmail().toUtf8());
  }
  if (configuration.isAuthoritySet()) {
    switch (configuration.getAuthority()) {
      case MoxaClassBased::AuthorityEnumType::AuthorityEnumType_Admin:
        user_account.SetRole(ActUserAccountRoleEnum::kAdmin);
        break;
      case MoxaClassBased::AuthorityEnumType::AuthorityEnumType_Supervisor:
        user_account.SetRole(ActUserAccountRoleEnum::kSupervisor);
        break;
      case MoxaClassBased::AuthorityEnumType::AuthorityEnumType_User:
        user_account.SetRole(ActUserAccountRoleEnum::kUser);
        break;
    }
  }

  act_status = checkDeviceAccountNode(user_account);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  if (device.GetAccount().GetUsername() == user_account.GetUsername()) {
    errorCode = M_UA_WARNING;
    errorMessage =
        UaString("Warning: The device may be disconnected, the user needs to modify the connection account.");
    qDebug() << errorMessage.toUtf8();
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

UaStatus removeDeviceAccountMethod(const UaNodeId& deviceAccountFolderNodeId, const UaNodeId& deviceAccountNodeId,
                                   OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(deviceAccountFolderNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(deviceAccountFolderNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActDevice device;
  act_status = project.GetDeviceById(device, device_id);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  UaReferenceDescriptions references;
  MoxaClassBased::DeviceAccountType* pDeviceAccountType = NULL;
  pMoxaNodeManager->getNodeReference(deviceAccountFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_DeviceAccountType) {
      continue;
    }

    if (UaNodeId(references[idx].NodeId.NodeId) == deviceAccountNodeId) {
      pDeviceAccountType = (MoxaClassBased::DeviceAccountType*)pMoxaNodeManager->getNode(deviceAccountNodeId);
      break;
    }
  }

  if (pDeviceAccountType == NULL) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: NodeId %1 is not found").arg(deviceAccountNodeId.toFullString());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  QString user_name(pDeviceAccountType->getUserName().toUtf8());
  ActUserAccountTable& user_account_table = project.GetDeviceConfig().GetUserAccountTables()[device_id];

  user_account_table.GetAccounts().remove(user_name);

  if (user_account_table.GetAccounts().isEmpty()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: The device account table cannot be empty");
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  if (device.GetAccount().GetUsername() == user_name) {
    errorCode = M_UA_WARNING;
    errorMessage =
        UaString("Warning: The device may be disconnected, the user needs to modify the connection account.");
    qDebug() << errorMessage.toUtf8();
  }

  if (user_account_table.GetSyncConnectionAccount() == user_name) {
    user_account_table.SetSyncConnectionAccount(user_account_table.GetAccounts().keys().first());
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

}  // namespace ClassBased