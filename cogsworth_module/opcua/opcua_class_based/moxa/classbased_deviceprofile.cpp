#include "classbased_deviceprofile.h"

namespace ClassBased {

UaStatus createDeviceProfileNode(const ActDeviceProfile& deviceProfile, OpcUa_UInt32& errorCode,
                                 UaString& errorMessage) {
  UaStatus ret;

  UaString deviceModelName = UaString(deviceProfile.GetModelName().toStdString().c_str());

  if (pMoxaNodeManager->isDeviceProfileExist(deviceModelName)) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Device profile %1 already exists").arg(deviceModelName);
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  // Moxa, ICMP, Unknown
  if (deviceModelName == UaString("Moxa") || deviceModelName == UaString("ICMP") ||
      deviceModelName == UaString("Unknown")) {
    return ret;
  }

  // NodeId for the node to create
  UaNodeId deviceProfileNodeId(pMoxaNodeManager->getDeviceProfileNodeId(deviceProfile.GetId()));

  // create the new node to the OPC UA nodemanager
  MoxaClassBased::DeviceProfileType* pDeviceProfileType =
      new MoxaClassBased::DeviceProfileType(deviceProfileNodeId, deviceModelName, pMoxaNodeManager->getNameSpaceIndex(),
                                            pMoxaNodeManager->getNodeManagerConfig());
  if (!pDeviceProfileType) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Allocate memory failed");
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  // Set device profile name
  pDeviceProfileType->setDeviceProfileName(deviceModelName);

  // Add node to the specified folder
  UaNodeId deviceProfileFolderNodeId(MoxaClassBasedId_Server_Resources_Communication_DeviceProfiles,
                                     pClassBased->getNameSpaceIndex());
  ret = pMoxaNodeManager->addNodeAndReference(deviceProfileFolderNodeId, pDeviceProfileType->getUaReferenceLists(),
                                              OpcUaId_HasComponent);
  if (ret.isNotGood()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Add node reference failed");
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  // Insert device profile to map
  pMoxaNodeManager->setDeviceProfileName(deviceModelName, deviceProfileNodeId, deviceProfile.GetId(),
                                         deviceProfile.GetDeviceType());

  errorCode = M_UA_NO_ERROR;
  return ret;
}

UaStatus removeDeviceProfileNode(const ActDeviceProfile& deviceProfile, OpcUa_UInt32& errorCode,
                                 UaString& errorMessage) {
  UaStatus ret;

  UaString deviceModelName = UaString(deviceProfile.GetModelName().toStdString().c_str());

  if (!pMoxaNodeManager->isDeviceProfileExist(deviceModelName)) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Device profile %1 is not found").arg(deviceModelName);
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  UaNodeId deviceProfileNodeId(pMoxaNodeManager->getDeviceProfileNodeId(deviceProfile.GetId()));

  UaNode* pNode = pMoxaNodeManager->getNode(deviceProfileNodeId);
  if (!pNode || pNode->typeDefinitionId().identifierNumeric() != MoxaClassBasedId_DeviceProfileType) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Node %1 type is not DeviceProfileType").arg(deviceProfileNodeId.toFullString());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  // Remove device profile node
  ret = pMoxaNodeManager->deleteUaNode(pNode, OpcUa_True, OpcUa_True, OpcUa_True);
  if (ret.isNotGood()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Remove node %1 failed").arg(deviceProfileNodeId.toFullString());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  // Remove name from device profile map
  pMoxaNodeManager->removeDeviceProfileName(deviceModelName);

  return ret;
}

UaStatus checkDeviceProfileMethod(const UaString& deviceProfileName, OpcUa_Boolean& registered, OpcUa_UInt32& errorCode,
                                  UaString& errorMessage) {
  UaStatus ret;

  registered = pMoxaNodeManager->isDeviceProfileExist(deviceProfileName);
  return ret;
}

UaStatus importDeviceProfileMethod(const UaString& profileData, UaNodeId& deviceProfileNodeId, OpcUa_UInt32& errorCode,
                                   UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  // Parse device profile from file
  ActDeviceProfile deviceProfile;

  try {
    deviceProfile.FromString(profileData.toUtf8());
  } catch (std::exception& e) {
    ActBadRequest bad_request(e.what());
    errorCode = static_cast<OpcUa_UInt32>(bad_request.GetStatus());
    errorMessage = UaString(bad_request.GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  QMutexLocker lock(&act::core::g_core.mutex_);

  // Upload device profile to Act
  act_status = act::core::g_core.UploadDeviceProfile(deviceProfile);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  deviceProfileNodeId = pMoxaNodeManager->getDeviceProfileNodeId(deviceProfile.GetId());

  return ret;
}

UaStatus removeDeviceProfileMethod(const UaNodeId& deviceProfileNodeId, OpcUa_UInt32& errorCode,
                                   UaString& errorMessage) {
  UaStatus ret;

  ACT_STATUS_INIT();

  // Find device profile node
  MoxaClassBased::DeviceProfileType* pDeviceProfileType =
      (MoxaClassBased::DeviceProfileType*)pMoxaNodeManager->getNode(deviceProfileNodeId);
  if (!pDeviceProfileType ||
      pDeviceProfileType->typeDefinitionId().identifierNumeric() != MoxaClassBasedId_DeviceProfileType) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Node %1 type is not DeviceProfileType").arg(deviceProfileNodeId.toFullString());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }
  UaString deviceProfileName = pDeviceProfileType->getDeviceProfileName();
  qint64 device_profile_id = pMoxaNodeManager->getDeviceProfileId(deviceProfileName);

  QMutexLocker lock(&act::core::g_core.mutex_);

  // Remove Act device profile
  act_status = act::core::g_core.DeleteDeviceProfile(device_profile_id);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}

}  // namespace ClassBased