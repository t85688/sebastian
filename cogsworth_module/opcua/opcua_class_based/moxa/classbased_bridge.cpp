#include "classbased_bridge.h"

namespace ClassBased {

UaStatus updateBridgeNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                          UaString& errorMessage) {
  UaStatus ret;

  UaNodeId bridgeNodeId = pMoxaNodeManager->getDeviceNodeId(project.GetId(), device.GetId());
  MoxaClassBased::BridgeType* pBridgeType = (MoxaClassBased::BridgeType*)pMoxaNodeManager->getNode(bridgeNodeId);
  UaString browseName(device.GetIpv4().GetIpAddress().toStdString().c_str());

  if (pBridgeType == NULL) {
    // If succeeded, create the new node to the OPC UA nodemanager
    pBridgeType = new MoxaClassBased::BridgeType(bridgeNodeId, browseName, pMoxaNodeManager->getNameSpaceIndex(),
                                                 pMoxaNodeManager->getNodeManagerConfig());
    if (!pBridgeType) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Allocate memory failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    UaNodeId projectNodeId = pMoxaNodeManager->getProjectNodeId(project.GetId());
    MoxaClassBased::ProjectType* pProjectType = (MoxaClassBased::ProjectType*)pMoxaNodeManager->getNode(projectNodeId);
    MoxaClassBased::DeviceFolderType* pDeviceFolderType = pProjectType->getDevices();
    ret = pMoxaNodeManager->addNodeAndReference(pDeviceFolderType->nodeId(), pBridgeType->getUaReferenceLists(),
                                                OpcUaId_HasComponent);
    if (ret.isNotGood()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Create bridge node failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    if (device.GetDeviceProperty().GetFeatureGroup().GetOperation().GetFactoryDefault()) {
      pBridgeType->getOperation()->getFactoryDefault();
    }

    if (device.GetDeviceProperty().GetFeatureGroup().GetOperation().GetFirmwareUpgrade()) {
      pBridgeType->getOperation()->getFirmwareUpgrade();
    }

    if (device.GetDeviceProperty().GetFeatureGroup().GetOperation().GetImportExport()) {
      pBridgeType->getOperation()->getImportDeviceConfig();
      pBridgeType->getOperation()->getExportDeviceConfig();
    }

    if (device.GetDeviceProperty().GetFeatureGroup().GetOperation().GetReboot()) {
      pBridgeType->getOperation()->getReboot();
    }

    if (device.GetDeviceProperty().GetFeatureGroup().GetOperation().GetEventLog()) {
      pBridgeType->getOperation()->getExportEventLog();
    }

    if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().CheckSupportAnyOne()) {
      pBridgeType->getOperation()->getSyncDeviceConfig();
    }

  } else {
    pBridgeType->setBrowseName(UaQualifiedName(browseName, pMoxaNodeManager->getNameSpaceIndex()));
    pBridgeType->setDisplayName(UaLocalizedText(UaString(), browseName));
  }

  for (ActInterface intf : device.GetInterfaces()) {
    UaNodeId interfaceNodeId =
        pMoxaNodeManager->getInterfaceNodeId(project.GetId(), device.GetId(), intf.GetInterfaceId());
    MoxaClassBased::EthernetInterfaceType* pEthernetInterface =
        (MoxaClassBased::EthernetInterfaceType*)pMoxaNodeManager->getNode(interfaceNodeId);
    UaString intfBrowseName = UaString("%1_%2")
                                  .arg(device.GetIpv4().GetIpAddress().toStdString().c_str())
                                  .arg(intf.GetInterfaceName().toStdString().c_str());
    if (pEthernetInterface == NULL) {
      pEthernetInterface = new MoxaClassBased::EthernetInterfaceType(interfaceNodeId, intfBrowseName,
                                                                     pMoxaNodeManager->getNameSpaceIndex(),
                                                                     pMoxaNodeManager->getNodeManagerConfig());
      if (!pEthernetInterface) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Allocate memory failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      ret = pMoxaNodeManager->addNodeAndReference(bridgeNodeId, pEthernetInterface->getUaReferenceLists(),
                                                  OpcUaId_HasComponent);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Create interface node failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    } else {
      pEthernetInterface->setBrowseName(UaQualifiedName(intfBrowseName, pMoxaNodeManager->getNameSpaceIndex()));
      pEthernetInterface->setDisplayName(UaLocalizedText(UaString(), intfBrowseName));
    }

    pEthernetInterface->setActive(intf.GetActive());
    pEthernetInterface->setInterfaceId(OpcUa_UInt16(intf.GetInterfaceId()));
    pEthernetInterface->setInterfaceName(intf.GetInterfaceName().toStdString().c_str());
  }

  ret = updateDeviceNode(project, device, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateDeviceAccountNode(project, device, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateTimeAwareShaperNode(project, device, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updatePerStreamPriorityNode(project, device, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateManagementInterfaceNode(project, device, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateSpanningTreeNode(project, device, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateUnicastStaticForwardNode(project, device, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateMulticastStaticForwardNode(project, device, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateTimeSyncNode(project, device, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateSyslogServerNode(project, device, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateSNMPTrapServerNode(project, device, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateVlanNode(project, device, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  errorCode = M_UA_NO_ERROR;

  return ret;
}

UaStatus addBridgeMethod(const UaNodeId& folderNodeId, const MoxaClassBased::BridgeDataType& configuration,
                         UaNodeId& bridgeNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  UaString deviceProfileName("Switch");
  if (configuration.isDeviceProfileNameSet()) {
    deviceProfileName = configuration.getDeviceProfileName();
  }
  if (!pMoxaNodeManager->isDeviceProfileExist(deviceProfileName)) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Device profile %1 cannot find").arg(deviceProfileName);
    qDebug() << errorMessage.toUtf8();
    return ret;
  } else if (pMoxaNodeManager->getDeviceProfileType(deviceProfileName) != ActDeviceTypeEnum::kSwitch &&
             pMoxaNodeManager->getDeviceProfileType(deviceProfileName) != ActDeviceTypeEnum::kTSNSwitch) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Device profile %1 is not Switch").arg(configuration.getDeviceProfileName());
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
    device.SetDeviceAlias(configuration.getAlias().toUtf8());
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

  bridgeNodeId = pMoxaNodeManager->getDeviceNodeId(project_id, device.GetId());

  return ret;
}

UaStatus setBridgeSettingMethod(const UaNodeId& bridgeNodeId, const MoxaClassBased::BridgeDataType& configuration,
                                OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(bridgeNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(bridgeNodeId);

  MoxaClassBased::BridgeType* pBridgeType = (MoxaClassBased::BridgeType*)pMoxaNodeManager->getNode(bridgeNodeId);

  UaString deviceProfileName = pBridgeType->getDeviceProfileName();
  if (configuration.isDeviceProfileNameSet()) {
    deviceProfileName = configuration.getDeviceProfileName();
  }
  if (!pMoxaNodeManager->isDeviceProfileExist(deviceProfileName)) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Device profile %1 cannot find").arg(deviceProfileName);
    qDebug() << errorMessage.toUtf8();
    return ret;
  } else if (pMoxaNodeManager->getDeviceProfileType(deviceProfileName) != ActDeviceTypeEnum::kSwitch &&
             pMoxaNodeManager->getDeviceProfileType(deviceProfileName) != ActDeviceTypeEnum::kTSNSwitch) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Device profile %1 is not Switch").arg(configuration.getDeviceProfileName());
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
    device.SetDeviceAlias(configuration.getAlias().toUtf8());
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