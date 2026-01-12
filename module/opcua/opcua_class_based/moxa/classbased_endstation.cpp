#include "classbased_endstation.h"

namespace ClassBased {

UaStatus updateEndStationNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                              UaString& errorMessage) {
  UaStatus ret;

  UaNodeId endStationNodeId = pMoxaNodeManager->getDeviceNodeId(project.GetId(), device.GetId());
  MoxaClassBased::EndStationType* pEndStationType =
      (MoxaClassBased::EndStationType*)pMoxaNodeManager->getNode(endStationNodeId);
  UaString browseName(device.GetIpv4().GetIpAddress().toStdString().c_str());

  if (pEndStationType == NULL) {
    // If succeeded, create the new node to the OPC UA nodemanager
    pEndStationType = new MoxaClassBased::EndStationType(
        endStationNodeId, browseName, pMoxaNodeManager->getNameSpaceIndex(), pMoxaNodeManager->getNodeManagerConfig());
    if (!pEndStationType) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Allocate memory failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    UaNodeId projectNodeId = pMoxaNodeManager->getProjectNodeId(project.GetId());
    MoxaClassBased::ProjectType* pProjectType = (MoxaClassBased::ProjectType*)pMoxaNodeManager->getNode(projectNodeId);
    MoxaClassBased::DeviceFolderType* pDeviceFolderType = pProjectType->getDevices();

    ret = pMoxaNodeManager->addNodeAndReference(pDeviceFolderType->nodeId(), pEndStationType->getUaReferenceLists(),
                                                OpcUaId_HasComponent);
    if (ret.isNotGood()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Create end-station node failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    for (ActInterface intf : device.GetInterfaces()) {
      UaNodeId interfaceNodeId =
          pMoxaNodeManager->getInterfaceNodeId(project.GetId(), device.GetId(), intf.GetInterfaceId());
      UaString browseName = UaString("%1_%2")
                                .arg(device.GetIpv4().GetIpAddress().toStdString().c_str())
                                .arg(intf.GetInterfaceName().toStdString().c_str());

      MoxaClassBased::EthernetInterfaceType* pEthernetInterface = new MoxaClassBased::EthernetInterfaceType(
          interfaceNodeId, browseName, pMoxaNodeManager->getNameSpaceIndex(), pMoxaNodeManager->getNodeManagerConfig());
      if (!pEthernetInterface) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Allocate memory failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      pEthernetInterface->setInterfaceId(OpcUa_UInt16(intf.GetInterfaceId()));
      pEthernetInterface->setInterfaceName(intf.GetInterfaceName().toStdString().c_str());

      ret = pMoxaNodeManager->addNodeAndReference(endStationNodeId, pEthernetInterface->getUaReferenceLists(),
                                                  OpcUaId_HasComponent);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Create interface node failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }
  } else {
    pEndStationType->setBrowseName(UaQualifiedName(browseName, pMoxaNodeManager->getNameSpaceIndex()));
    pEndStationType->setDisplayName(UaLocalizedText(UaString(), browseName));
  }

  ret = updateDeviceNode(project, device, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  errorCode = M_UA_NO_ERROR;

  return ret;
}

UaStatus addEndStationMethod(const UaNodeId& folderNodeId, const MoxaClassBased::EndStationDataType& configuration,
                             UaNodeId& endStationNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  UaString deviceProfileName("End-Station");
  if (configuration.isDeviceProfileNameSet()) {
    deviceProfileName = configuration.getDeviceProfileName();
  }
  if (!pMoxaNodeManager->isDeviceProfileExist(deviceProfileName)) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Device profile %1 cannot find").arg(deviceProfileName);
    qDebug() << errorMessage.toUtf8();
    return ret;
  } else if (pMoxaNodeManager->getDeviceProfileType(deviceProfileName) != ActDeviceTypeEnum::kEndStation) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Device profile %1 is not End-Station").arg(configuration.getDeviceProfileName());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActDevice device;
  device.SetDeviceType(pMoxaNodeManager->getDeviceProfileType(deviceProfileName));
  device.SetDeviceProfileId(pMoxaNodeManager->getDeviceProfileId(deviceProfileName));

  // Set Firmware Virsion
  if (configuration.isFirmwareVersionSet()) {
    device.SetFirmwareVersion(configuration.getFirmwareVersion().toUtf8());
  }

  // Set Device Alias
  if (configuration.isAliasSet()) {
    device.SetDeviceAlias(QString(configuration.getAlias().toUtf8()));
  }

  // Set Connection Account
  ActDeviceAccount& account = device.GetAccount();
  if (configuration.getConnectionAccount().isUserNameSet()) {
    account.SetDefaultSetting(false);
    account.SetUsername(configuration.getConnectionAccount().getUserName().toUtf8());
  }
  if (configuration.getConnectionAccount().isPasswordSet()) {
    account.SetDefaultSetting(false);
    account.SetPassword(configuration.getConnectionAccount().getPassword().toUtf8());
  }

  // Save NETCONF
  ActNetconfConfiguration& netconfConfiguration = device.GetNetconfConfiguration();
  netconfConfiguration.SetTLS(false);
  ActNetconfOverSSH& netconfOverSSH = netconfConfiguration.GetNetconfOverSSH();
  if (configuration.getNETCONF().isSSHPortSet()) {
    netconfOverSSH.SetSSHPort(quint16(configuration.getNETCONF().getSSHPort()));
  }

  // Save SNMP
  ActSnmpConfiguration& snmpConfiguration = device.GetSnmpConfiguration();
  if (configuration.getSNMP().isPortSet()) {
    snmpConfiguration.SetPort(configuration.getSNMP().getPort());
  }
  if (configuration.getSNMP().isReadCommunitySet()) {
    snmpConfiguration.SetReadCommunity(QString(configuration.getSNMP().getReadCommunity().toUtf8()));
  }
  if (configuration.getSNMP().isWriteCommunitySet()) {
    snmpConfiguration.SetWriteCommunity(QString(configuration.getSNMP().getWriteCommunity().toUtf8()));
  }
  if (configuration.getSNMP().isVersionSet()) {
    snmpConfiguration.SetVersion(pMoxaNodeManager->setSnmpVersion(configuration.getSNMP().getVersion()));
  }

  // Save RESTful
  ActRestfulConfiguration& restfulConfiguration = device.GetRestfulConfiguration();
  if (configuration.getRESTful().isPortSet()) {
    restfulConfiguration.SetPort(quint16(configuration.getRESTful().getPort()));
  }

  // Set IP address
  ActIpv4& ipv4 = device.GetIpv4();
  if (configuration.getIpSetting().isIpAddressSet()) {
    ipv4.SetIpAddress(configuration.getIpSetting().getIpAddress().toUtf8());
  }
  if (configuration.getIpSetting().isSubnetMaskSet()) {
    ipv4.SetSubnetMask(configuration.getIpSetting().getSubnetMask().toUtf8());
  }
  if (configuration.getIpSetting().isGatewaySet()) {
    ipv4.SetGateway(configuration.getIpSetting().getGateway().toUtf8());
  }
  if (configuration.getIpSetting().isDNS1Set()) {
    ipv4.SetDNS1(configuration.getIpSetting().getDNS1().toUtf8());
  }
  if (configuration.getIpSetting().isDNS2Set()) {
    ipv4.SetDNS2(configuration.getIpSetting().getDNS2().toUtf8());
  }

  // Set Interface Name
  MoxaClassBased::EthernetInterfaceDataTypes ethernetInterfaces;
  configuration.getEthernetInterfaces(ethernetInterfaces);

  QList<ActInterface>& interfaces = device.GetInterfaces();
  for (OpcUa_UInt32 i = 0; i < ethernetInterfaces.length(); i++) {
    MoxaClassBased::EthernetInterfaceDataType ethernetInterfaceDataType(ethernetInterfaces[i]);
    ActInterface intf;
    intf.SetInterfaceId(qint64(i + 1));
    intf.SetInterfaceName(ethernetInterfaceDataType.getInterfaceName().toUtf8());
    if (ethernetInterfaceDataType.isPhysAddressSet()) {
      intf.SetMacAddress(ethernetInterfaceDataType.getPhysAddress().toUtf8());
    }

    interfaces.append(intf);
  }

  qint64 project_id = pMoxaNodeManager->getProjectId(folderNodeId);

  QMutexLocker lock(&act::core::g_core.mutex_);

  const bool from_bag = false;
  act_status = act::core::g_core.CreateDevice(project_id, device, from_bag);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  endStationNodeId = pMoxaNodeManager->getDeviceNodeId(project_id, device.GetId());

  return ret;
}

UaStatus setEndStationSettingMethod(const UaNodeId& endStationNodeId,
                                    const MoxaClassBased::EndStationDataType& configuration, OpcUa_UInt32& errorCode,
                                    UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(endStationNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(endStationNodeId);

  MoxaClassBased::EndStationType* pEndStationType =
      (MoxaClassBased::EndStationType*)pMoxaNodeManager->getNode(endStationNodeId);

  UaString deviceProfileName = pEndStationType->getDeviceProfileName();
  if (configuration.isDeviceProfileNameSet()) {
    deviceProfileName = configuration.getDeviceProfileName();
  }
  if (!pMoxaNodeManager->isDeviceProfileExist(deviceProfileName)) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Device profile %1 cannot find").arg(deviceProfileName);
    qDebug() << errorMessage.toUtf8();
    return ret;
  } else if (pMoxaNodeManager->getDeviceProfileType(deviceProfileName) != ActDeviceTypeEnum::kEndStation) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Device profile %1 is not End-Station").arg(configuration.getDeviceProfileName());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

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

  device.SetDeviceType(pMoxaNodeManager->getDeviceProfileType(deviceProfileName));
  device.SetDeviceProfileId(pMoxaNodeManager->getDeviceProfileId(deviceProfileName));

  // Set Firmware Virsion
  if (configuration.isFirmwareVersionSet()) {
    device.SetFirmwareVersion(configuration.getFirmwareVersion().toUtf8());
  }

  // Set Device Alias
  if (configuration.isAliasSet()) {
    device.SetDeviceAlias(QString(configuration.getAlias().toUtf8()));
  }

  // Set Connection Account
  ActDeviceAccount& account = device.GetAccount();
  if (configuration.getConnectionAccount().isUserNameSet()) {
    account.SetDefaultSetting(false);
    account.SetUsername(configuration.getConnectionAccount().getUserName().toUtf8());
  }
  if (configuration.getConnectionAccount().isPasswordSet()) {
    account.SetDefaultSetting(false);
    account.SetPassword(configuration.getConnectionAccount().getPassword().toUtf8());
  }

  // Save NETCONF
  ActNetconfConfiguration& netconfConfiguration = device.GetNetconfConfiguration();
  netconfConfiguration.SetTLS(false);
  ActNetconfOverSSH& netconfOverSSH = netconfConfiguration.GetNetconfOverSSH();
  if (configuration.getNETCONF().isSSHPortSet()) {
    netconfOverSSH.SetSSHPort(quint16(configuration.getNETCONF().getSSHPort()));
  }

  // Save SNMP
  ActSnmpConfiguration& snmpConfiguration = device.GetSnmpConfiguration();
  if (configuration.getSNMP().isPortSet()) {
    snmpConfiguration.SetPort(configuration.getSNMP().getPort());
  }
  if (configuration.getSNMP().isReadCommunitySet()) {
    snmpConfiguration.SetReadCommunity(QString(configuration.getSNMP().getReadCommunity().toUtf8()));
  }
  if (configuration.getSNMP().isWriteCommunitySet()) {
    snmpConfiguration.SetWriteCommunity(QString(configuration.getSNMP().getWriteCommunity().toUtf8()));
  }
  if (configuration.getSNMP().isVersionSet()) {
    snmpConfiguration.SetVersion(pMoxaNodeManager->setSnmpVersion(configuration.getSNMP().getVersion()));
  }

  // Save RESTful
  ActRestfulConfiguration& restfulConfiguration = device.GetRestfulConfiguration();
  if (configuration.getRESTful().isPortSet()) {
    restfulConfiguration.SetPort(quint16(configuration.getRESTful().getPort()));
  }

  // Set IP address
  ActIpv4& ipv4 = device.GetIpv4();
  if (configuration.getIpSetting().isIpAddressSet()) {
    ipv4.SetIpAddress(configuration.getIpSetting().getIpAddress().toUtf8());
  }
  if (configuration.getIpSetting().isSubnetMaskSet()) {
    ipv4.SetSubnetMask(configuration.getIpSetting().getSubnetMask().toUtf8());
  }
  if (configuration.getIpSetting().isGatewaySet()) {
    ipv4.SetGateway(configuration.getIpSetting().getGateway().toUtf8());
  }
  if (configuration.getIpSetting().isDNS1Set()) {
    ipv4.SetDNS1(configuration.getIpSetting().getDNS1().toUtf8());
  }
  if (configuration.getIpSetting().isDNS2Set()) {
    ipv4.SetDNS2(configuration.getIpSetting().getDNS2().toUtf8());
  }

  // Set Interface Name
  MoxaClassBased::EthernetInterfaceDataTypes ethernetInterfaces;
  configuration.getEthernetInterfaces(ethernetInterfaces);

  QList<ActInterface>& interfaces = device.GetInterfaces();
  for (OpcUa_UInt32 i = 0; i < ethernetInterfaces.length(); i++) {
    MoxaClassBased::EthernetInterfaceDataType ethernetInterfaceDataType(ethernetInterfaces[i]);
    ActInterface intf;
    intf.SetInterfaceId(qint64(i + 1));
    intf.SetInterfaceName(ethernetInterfaceDataType.getInterfaceName().toUtf8());
    if (ethernetInterfaceDataType.isPhysAddressSet()) {
      intf.SetMacAddress(ethernetInterfaceDataType.getPhysAddress().toUtf8());
    }

    interfaces.append(intf);
  }

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.UpdateDevice(project_id, device);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}
}  // namespace ClassBased