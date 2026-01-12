#include "classbased_bridge.h"
#include "classbased_endstation.h"
#include "classbased_ipsetting.h"

namespace ClassBased {

UaStatus updateDeviceNodes(const ActProject& project, OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  UaNodeId projectNodeId = pMoxaNodeManager->getProjectNodeId(project.GetId());
  MoxaClassBased::ProjectType* pProjectType = (MoxaClassBased::ProjectType*)pMoxaNodeManager->getNode(projectNodeId);
  MoxaClassBased::DeviceFolderType* pDeviceFolderType = pProjectType->getDevices();

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(pDeviceFolderType->nodeId(), OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_BridgeType &&
        references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_EndStationType) {
      continue;
    }
    MoxaClassBased::DeviceType* pDeviceType =
        (MoxaClassBased::DeviceType*)pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);
    UaString browseName = pDeviceType->browseName().toString();

    qint64 device_id = pMoxaNodeManager->getDeviceId(pDeviceType->nodeId());
    ActDevice device;
    act_status = project.GetDeviceById(device, device_id);
    if (IsActStatusNotFound(act_status)) {
      ret = pMoxaNodeManager->deleteUaNode(pDeviceType, OpcUa_True, OpcUa_True, OpcUa_True);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Remove device %1 failed").arg(browseName);
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
      continue;
    }

    UaString device_profile_name = UaString(device.GetDeviceProperty().GetModelName().toStdString().c_str());
    if (pDeviceType->getDeviceProfileName() != device_profile_name) {
      ret = pMoxaNodeManager->deleteUaNode(pDeviceType, OpcUa_True, OpcUa_True, OpcUa_True);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Remove device %1 failed").arg(browseName);
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
      continue;
    }
  }

  for (ActDevice device : project.GetDevices()) {
    if (device.GetDeviceType() == ActDeviceTypeEnum::kSwitch ||
        device.GetDeviceType() == ActDeviceTypeEnum::kTSNSwitch) {
      ret = updateBridgeNode(project, device, errorCode, errorMessage);
    } else if (device.GetDeviceType() == ActDeviceTypeEnum::kEndStation) {
      ret = updateEndStationNode(project, device, errorCode, errorMessage);
    }
    if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
      return ret;
    }
  }

  return ret;
}

UaStatus updateDeviceNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                          UaString& errorMessage) {
  UaStatus ret;

  UaNodeId deviceNodeId = pMoxaNodeManager->getDeviceNodeId(project.GetId(), device.GetId());
  MoxaClassBased::DeviceType* pDeviceType = (MoxaClassBased::DeviceType*)pMoxaNodeManager->getNode(deviceNodeId);

  pDeviceType->setAlias(device.GetDeviceAlias().toStdString().c_str());
  pDeviceType->getConnectionAccount()->setUserName(device.GetAccount().GetUsername().toStdString().c_str());
  pDeviceType->getDeviceInformation()->setFirmwareVersion(device.GetFirmwareVersion().toStdString().c_str());
  pDeviceType->setDeviceProfileName(device.GetDeviceProperty().GetModelName().toStdString().c_str());
  pDeviceType->setMacAddress(device.GetMacAddress().toStdString().c_str());
  pDeviceType->getNETCONF()->setSSHPort(
      OpcUa_UInt16(device.GetNetconfConfiguration().GetNetconfOverSSH().GetSSHPort()));
  pDeviceType->getSNMP()->setPort(OpcUa_UInt16(device.GetSnmpConfiguration().GetPort()));
  pDeviceType->getSNMP()->setVersion(pMoxaNodeManager->getSnmpVersion(device.GetSnmpConfiguration().GetVersion()));
  pDeviceType->getRESTful()->setPort(OpcUa_UInt16(device.GetRestfulConfiguration().GetPort()));

  const ActIpv4& ipv4 = device.GetIpv4();
  MoxaClassBased::IpSettingType* pIpSettingType = pDeviceType->getIpSetting();
  pIpSettingType->setIpAddress(ipv4.GetIpAddress().toStdString().c_str());
  pIpSettingType->setSubnetMask(ipv4.GetSubnetMask().toStdString().c_str());
  pIpSettingType->setGateway(ipv4.GetGateway().toStdString().c_str());
  pIpSettingType->setDNS1(ipv4.GetDNS1().toStdString().c_str());
  pIpSettingType->setDNS2(ipv4.GetDNS2().toStdString().c_str());

  return ret;
}

UaStatus removeDeviceMethod(const UaNodeId& deviceNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(deviceNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(deviceNodeId);

  QMutexLocker lock(&act::core::g_core.mutex_);

  // Delete Act device
  act_status = act::core::g_core.DeleteDevice(project_id, device_id);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}
}  // namespace ClassBased