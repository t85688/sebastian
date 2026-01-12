#include "classbased_broadcastsearchandipsetting.h"

namespace ClassBased {

void setErrorInformation(
    UaNode* pUaNode, OpcUa_UInt32 errorCode, UaString errorMessage,
    MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::BroadcastSearchAndIPSettingState state) {
  MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType* pBroadcastSearchAndIPSettingStateMachineType =
      (MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType*)pUaNode;

  qDebug() << errorMessage.toUtf8();

  pBroadcastSearchAndIPSettingStateMachineType->setErrorCode(errorCode);
  pBroadcastSearchAndIPSettingStateMachineType->setErrorMessage(errorMessage);
  pBroadcastSearchAndIPSettingStateMachineType->setBroadcastSearchAndIPSettingState(state);
  return;
}

UaStatus startBroadcastSearchAndIPSettingMethod(
    UaNode* pUaNode,
    MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::BroadcastSearchAndIPSettingState state) {
  UaStatus ret;

  MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType* pBroadcastSearchAndIPSettingStateMachineType =
      (MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType*)pUaNode;

  // Init error message
  pBroadcastSearchAndIPSettingStateMachineType->setErrorCode(M_UA_NO_ERROR);
  pBroadcastSearchAndIPSettingStateMachineType->setErrorMessage(UaString(""));

  switch (state) {
    case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::Init: {
      foreach (void* value, pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap) {
        MoxaClassBased::DiscoveredBridgeType* pDiscoveredBridgeType = (MoxaClassBased::DiscoveredBridgeType*)value;

        ret = pMoxaNodeManager->deleteUaNode(pMoxaNodeManager->getNode(pDiscoveredBridgeType->nodeId()), OpcUa_True,
                                             OpcUa_True, OpcUa_True);
        if (ret.isNotGood()) {
          setErrorInformation(
              pUaNode, M_UA_INTERNAL_ERROR,
              UaString("Invalid: Remove node %1 failed").arg(pDiscoveredBridgeType->nodeId().toFullString()),
              MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::Init);
          return ret;
        }
      }
      pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap.clear();
      pBroadcastSearchAndIPSettingStateMachineType->setBroadcastSearchAndIPSettingState(state);
    } break;

    case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscovering:
      pBroadcastSearchAndIPSettingStateMachineType->setBroadcastSearchAndIPSettingState(state);
      pBroadcastSearchAndIPSettingStateMachineType->start();
      break;

    case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::RetryConnection:
      pBroadcastSearchAndIPSettingStateMachineType->setBroadcastSearchAndIPSettingState(state);
      pBroadcastSearchAndIPSettingStateMachineType->start();
      break;

    case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::LinkSequenceDetecting:
      pBroadcastSearchAndIPSettingStateMachineType->setBroadcastSearchAndIPSettingState(state);
      pBroadcastSearchAndIPSettingStateMachineType->start();
      break;

    case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::IpConfiguring: {
      MoxaClassBased_IpConfigureDataType* pIpConfigures =
          pBroadcastSearchAndIPSettingStateMachineType->ipConfigureDataList.rawData();
      OpcUa_UInt32 length = pBroadcastSearchAndIPSettingStateMachineType->ipConfigureDataList.length();

      for (OpcUa_UInt32 idx = 0; idx < length; idx++) {
        UaString macAddress(pIpConfigures[idx].MacAddress);
        if (!pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap.contains(macAddress)) {
          setErrorInformation(pUaNode, M_UA_INTERNAL_ERROR,
                              UaString("Invalid: Cannot find discovered device \"%1\"").arg(macAddress), state);
          return ret;
        }

        MoxaClassBased::DiscoveredBridgeType* pDiscoveredBridgeType =
            (MoxaClassBased::DiscoveredBridgeType*)
                pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap.value(macAddress);
        if (!pDiscoveredBridgeType->getRESTful()->getStatus() || !pDiscoveredBridgeType->getSNMP()->getStatus()) {
          setErrorInformation(pUaNode, M_UA_INTERNAL_ERROR,
                              UaString("Invalid: The discovered device \"%1\" %2 is disconnected.")
                                  .arg(macAddress)
                                  .arg((!pDiscoveredBridgeType->getRESTful()->getStatus() &&
                                        !pDiscoveredBridgeType->getSNMP()->getStatus())
                                           ? UaString("RESTful & SNMP")
                                       : !pDiscoveredBridgeType->getRESTful()->getStatus() ? UaString("RESTful")
                                                                                           : UaString("SNMP")),
                              state);
          return ret;
        }
        pDiscoveredBridgeType->setNewAssignedIpAddress(UaString(pIpConfigures[idx].AssignedIpAddress));
      }
      pBroadcastSearchAndIPSettingStateMachineType->setBroadcastSearchAndIPSettingState(state);
      pBroadcastSearchAndIPSettingStateMachineType->start();
    } break;

    default:
      ret = OpcUa_BadNotExecutable;
      break;
  }
  return ret;
}

void broadcastSearchAndIPSettingProgramRunCb(ACT_STATUS status, void* arg) {
  ((ActStatusBase*)arg)->SetStatus(status->GetStatus());
  ((ActStatusBase*)arg)->SetErrorMessage(status->GetErrorMessage());
}

void runBroadcastSearchAndIPSettingMethod(
    UaNode* pUaNode,
    MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::BroadcastSearchAndIPSettingState state) {
  ACT_STATUS_INIT();

  MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType* pBroadcastSearchAndIPSettingStateMachineType =
      (MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType*)pUaNode;

  // Create a std::promise object
  pBroadcastSearchAndIPSettingStateMachineType->signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = pBroadcastSearchAndIPSettingStateMachineType->signal_sender->get_future();

  // Get project nodeId
  qint64 project_id = pMoxaNodeManager->getProjectId(pUaNode->nodeId());

  ActStatusBase status(ActStatusType::kRunning);
  switch (state) {
    case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscovering: {
      ActDeviceDiscoveryConfig actDeviceDiscoveryConfig;
      MoxaClassBased::DeviceDiscoveryDataType deviceDiscoveryData =
          pBroadcastSearchAndIPSettingStateMachineType->deviceDiscoveryData;
      ActDefineDeviceType actDefineDeviceType;
      actDefineDeviceType.SetMoxaIndustrialEthernetProduct(deviceDiscoveryData.getMoxaIndustrialEthernetSwitch());
      actDeviceDiscoveryConfig.SetDefineDeviceType(actDefineDeviceType);

      ActSnmpConfiguration actSnmpConfiguration;
      actSnmpConfiguration.SetPort(deviceDiscoveryData.getSNMP().getPort());
      actSnmpConfiguration.SetReadCommunity(QString(deviceDiscoveryData.getSNMP().getReadCommunity().toUtf8()));
      actSnmpConfiguration.SetWriteCommunity(QString(deviceDiscoveryData.getSNMP().getWriteCommunity().toUtf8()));
      actSnmpConfiguration.SetVersion(pMoxaNodeManager->setSnmpVersion(deviceDiscoveryData.getSNMP().getVersion()));
      actDeviceDiscoveryConfig.SetSnmpConfiguration(actSnmpConfiguration);

      ActRestfulConfiguration aActRestfulConfiguration;
      aActRestfulConfiguration.SetPort(quint16(deviceDiscoveryData.getRESTful().getPort()));
      actDeviceDiscoveryConfig.SetRestfulConfiguration(aActRestfulConfiguration);

      act_status =
          act::core::g_core.OpcUaStartDeviceDiscovery(project_id, actDeviceDiscoveryConfig, std::move(signal_receiver),
                                                      broadcastSearchAndIPSettingProgramRunCb, (void*)&status);
      if (!IsActStatusSuccess(act_status)) {
        return setErrorInformation(
            pUaNode, static_cast<OpcUa_UInt32>(act_status->GetStatus()),
            UaString(act_status->GetErrorMessage().toStdString().c_str()),
            MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscoveringFailed);
      }
    } break;
    case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::RetryConnection: {
      ActRetryConnectConfig actRetryConnectConfig;
      MoxaClassBased::SNMPDataType SNMP = pBroadcastSearchAndIPSettingStateMachineType->SNMPData;
      MoxaClassBased::RESTfulDataType RESTful = pBroadcastSearchAndIPSettingStateMachineType->RESTfulData;

      ActSnmpConfiguration actSnmpConfiguration;
      actSnmpConfiguration.SetPort(SNMP.getPort());
      actSnmpConfiguration.SetReadCommunity(QString(SNMP.getReadCommunity().toUtf8()));
      actSnmpConfiguration.SetWriteCommunity(QString(SNMP.getWriteCommunity().toUtf8()));
      actSnmpConfiguration.SetVersion(pMoxaNodeManager->setSnmpVersion(SNMP.getVersion()));
      actRetryConnectConfig.SetSnmpConfiguration(actSnmpConfiguration);

      ActRestfulConfiguration actRestfulConfiguration;
      actRestfulConfiguration.SetPort(quint16(RESTful.getPort()));
      actRetryConnectConfig.SetRestfulConfiguration(actRestfulConfiguration);

      ActProject actProject;
      act_status = act::core::g_core.GetProject(project_id, actProject);
      if (!IsActStatusSuccess(act_status)) {
        return setErrorInformation(pUaNode, static_cast<OpcUa_UInt32>(act_status->GetStatus()),
                                   UaString(act_status->GetErrorMessage().toStdString().c_str()),
                                   MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscovered);
      }

      QList<qint64> retryDeviceList;
      for (ActDevice actDevice : actProject.broadcast_search_devices_) {
        if (!actDevice.GetDeviceStatus().GetSNMPStatus() || !actDevice.GetDeviceStatus().GetRESTfulStatus()) {
          retryDeviceList.append(actDevice.GetId());
        }
      }
      actRetryConnectConfig.SetId(retryDeviceList);

      act_status =
          act::core::g_core.OpcUaStartRetryConnect(project_id, actRetryConnectConfig, std::move(signal_receiver),
                                                   broadcastSearchAndIPSettingProgramRunCb, (void*)&status);
      if (!IsActStatusSuccess(act_status)) {
        return setErrorInformation(pUaNode, static_cast<OpcUa_UInt32>(act_status->GetStatus()),
                                   UaString(act_status->GetErrorMessage().toStdString().c_str()),
                                   MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscovered);
      }
    } break;
    case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::LinkSequenceDetecting: {
      act_status = act::core::g_core.OpcUaStartLinkSequenceDetect(
          project_id, std::move(signal_receiver), broadcastSearchAndIPSettingProgramRunCb, (void*)&status);
      if (!IsActStatusSuccess(act_status)) {
        return setErrorInformation(
            pUaNode, static_cast<OpcUa_UInt32>(act_status->GetStatus()),
            UaString(act_status->GetErrorMessage().toStdString().c_str()),
            MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::LinkSequenceDetectingFailed);
      }
    } break;
    case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::IpConfiguring: {
      MoxaClassBased_IpConfigureDataType* pIpConfigures =
          pBroadcastSearchAndIPSettingStateMachineType->ipConfigureDataList.rawData();
      OpcUa_UInt32 length = pBroadcastSearchAndIPSettingStateMachineType->ipConfigureDataList.length();

      ActProject actProject;
      act_status = act::core::g_core.GetProject(project_id, actProject);
      if (!IsActStatusSuccess(act_status)) {
        return setErrorInformation(pUaNode, static_cast<OpcUa_UInt32>(act_status->GetStatus()),
                                   UaString(act_status->GetErrorMessage().toStdString().c_str()),
                                   MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::IpConfiguringFailed);
      }

      QList<ActDeviceIpConfiguration> actDeviceIpConfigurationList;
      for (OpcUa_UInt32 idx = 0; idx < length; idx++) {
        UaString macAddress(pIpConfigures[idx].MacAddress);
        if (!pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap.contains(macAddress)) {
          return setErrorInformation(pUaNode, M_UA_INTERNAL_ERROR,
                                     UaString("Invalid: Cannot find discovered device \"%1\"").arg(macAddress),
                                     MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::IpConfiguringFailed);
        }

        MoxaClassBased::DiscoveredBridgeType* pDiscoveredBridgeType =
            (MoxaClassBased::DiscoveredBridgeType*)
                pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap.value(macAddress);
        if (!pDiscoveredBridgeType->getRESTful()->getStatus() || !pDiscoveredBridgeType->getSNMP()->getStatus()) {
          return setErrorInformation(pUaNode, M_UA_INTERNAL_ERROR,
                                     UaString("Invalid: The discovered device \"%1\" %2 is disconnected.")
                                         .arg(macAddress)
                                         .arg((!pDiscoveredBridgeType->getRESTful()->getStatus() &&
                                               !pDiscoveredBridgeType->getSNMP()->getStatus())
                                                  ? UaString("RESTful & SNMP")
                                              : !pDiscoveredBridgeType->getRESTful()->getStatus() ? UaString("RESTful")
                                                                                                  : UaString("SNMP")),
                                     MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::IpConfiguringFailed);
        }

        for (ActDevice actDevice : actProject.broadcast_search_devices_) {
          if (UaString(actDevice.GetMacAddress().toStdString().c_str()) != macAddress) {
            continue;
          }
          ActDeviceIpConfiguration actDeviceIpConfiguration;
          actDeviceIpConfiguration.SetGateway(actDevice.GetIpv4().GetGateway());
          actDeviceIpConfiguration.SetMacAddress(actDevice.GetMacAddress());
          actDeviceIpConfiguration.SetNewIp(QString(UaString(pIpConfigures[idx].AssignedIpAddress).toUtf8()));
          actDeviceIpConfiguration.SetOriginIp(actDevice.GetIpv4().GetIpAddress());
          actDeviceIpConfiguration.SetSubnetMask(actDevice.GetIpv4().GetSubnetMask());
          actDeviceIpConfiguration.SetAccount(actDevice.GetAccount());
          actDeviceIpConfigurationList.append(actDeviceIpConfiguration);
          break;
        }
      }

      act_status = act::core::g_core.OpcUaStartIpConfiguration(project_id, actDeviceIpConfigurationList, true,
                                                               std::move(signal_receiver),
                                                               broadcastSearchAndIPSettingProgramRunCb, (void*)&status);
      if (!IsActStatusSuccess(act_status)) {
        return setErrorInformation(pUaNode, static_cast<OpcUa_UInt32>(act_status->GetStatus()),
                                   UaString(act_status->GetErrorMessage().toStdString().c_str()),
                                   MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::IpConfiguringFailed);
      }
      break;
    }
    default:
      status.SetStatus(ActStatusType::kFailed);
      break;
  }

  // wait for computing
  while (status.GetStatus() == ActStatusType::kRunning) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (status.GetStatus() == ActStatusType::kFinished || status.GetStatus() == ActStatusType::kSuccess) {
    switch (state) {
      case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscovering: {
        ActProject actProject;
        act_status = act::core::g_core.GetProject(project_id, actProject);
        if (!IsActStatusSuccess(act_status)) {
          return setErrorInformation(
              pUaNode, static_cast<OpcUa_UInt32>(act_status->GetStatus()),
              UaString(act_status->GetErrorMessage().toStdString().c_str()),
              MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscoveringFailed);
        }
        UaNodeId discoveredDevicesNodeId =
            pBroadcastSearchAndIPSettingStateMachineType->getDiscoveredDevices()->nodeId();
        for (ActDevice actDevice : actProject.broadcast_search_devices_) {
          UaString ipAddress(actDevice.GetIpv4().GetIpAddress().toStdString().c_str());
          UaString macAddress(actDevice.GetMacAddress().toStdString().c_str());
          UaNodeId newNodeId(discoveredDevicesNodeId.toString() + macAddress, pMoxaNodeManager->getNameSpaceIndex());

          // If succeeded, create the new node to the OPC UA nodemanager
          MoxaClassBased::DiscoveredBridgeType* pDiscoveredBridgeType = new MoxaClassBased::DiscoveredBridgeType(
              newNodeId, macAddress, pMoxaNodeManager->getNameSpaceIndex(), pMoxaNodeManager->getNodeManagerConfig());
          if (!pDiscoveredBridgeType) {
            return setErrorInformation(
                pUaNode, M_UA_INTERNAL_ERROR, UaString("Invalid: Allocate memory failed"),
                MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscoveringFailed);
          }

          pDiscoveredBridgeType->getIpSetting()->setIpAddress(ipAddress);
          pDiscoveredBridgeType->setMacAddress(macAddress);
          pDiscoveredBridgeType->setHopCount(actDevice.GetDistance());

          // Get NETCONF
          MoxaClassBased::NETCONFType* pNETCONF = pDiscoveredBridgeType->getNETCONF();
          pNETCONF->setSSHPort(OpcUa_UInt16(actDevice.GetNetconfConfiguration().GetNetconfOverSSH().GetSSHPort()));
          pNETCONF->setStatus(actDevice.GetDeviceStatus().GetNETCONFStatus());

          // Get SNMP
          MoxaClassBased::SNMPType* pSNMP = pDiscoveredBridgeType->getSNMP();
          pSNMP->setPort(OpcUa_UInt16(actDevice.GetSnmpConfiguration().GetPort()));
          pSNMP->setVersion(pMoxaNodeManager->getSnmpVersion(actDevice.GetSnmpConfiguration().GetVersion()));
          pSNMP->setStatus(actDevice.GetDeviceStatus().GetSNMPStatus());

          // Get RESTful
          MoxaClassBased::RESTfulType* pRESTful = pDiscoveredBridgeType->getRESTful();
          pRESTful->setPort(OpcUa_UInt16(actDevice.GetRestfulConfiguration().GetPort()));
          pRESTful->setStatus(actDevice.GetDeviceStatus().GetRESTfulStatus());

          UaStatus ret = pMoxaNodeManager->addNodeAndReference(
              discoveredDevicesNodeId, pDiscoveredBridgeType->getUaReferenceLists(), OpcUaId_HasComponent);
          if (ret.isNotGood()) {
            return setErrorInformation(
                pUaNode, M_UA_INTERNAL_ERROR, UaString("Invalid: Add node reference failed"),
                MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscoveringFailed);
          }

          pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap.insert(macAddress,
                                                                                   (void*)pDiscoveredBridgeType);
        }

        pBroadcastSearchAndIPSettingStateMachineType->setBroadcastSearchAndIPSettingState(
            MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscovered);
      } break;

      case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::RetryConnection: {
        ActProject actProject;
        act_status = act::core::g_core.GetProject(project_id, actProject);
        if (!IsActStatusSuccess(act_status)) {
          return setErrorInformation(pUaNode, static_cast<OpcUa_UInt32>(act_status->GetStatus()),
                                     UaString(act_status->GetErrorMessage().toStdString().c_str()),
                                     MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscovered);
        }

        // Assigned connection status
        for (ActDevice actDevice : actProject.broadcast_search_devices_) {
          UaString macAddress(actDevice.GetMacAddress().toStdString().c_str());
          if (!pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap.contains(macAddress)) {
            continue;
          }
          MoxaClassBased::DiscoveredBridgeType* pDiscoveredBridgeType =
              (MoxaClassBased::DiscoveredBridgeType*)
                  pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap.value(macAddress);

          // Get NETCONF
          MoxaClassBased::NETCONFType* pNETCONF = pDiscoveredBridgeType->getNETCONF();
          pNETCONF->setSSHPort(OpcUa_UInt16(actDevice.GetNetconfConfiguration().GetNetconfOverSSH().GetSSHPort()));
          pNETCONF->setStatus(actDevice.GetDeviceStatus().GetNETCONFStatus());

          // Get SNMP
          MoxaClassBased::SNMPType* pSNMP = pDiscoveredBridgeType->getSNMP();
          pSNMP->setPort(OpcUa_UInt16(actDevice.GetSnmpConfiguration().GetPort()));
          pSNMP->setVersion(pMoxaNodeManager->getSnmpVersion(actDevice.GetSnmpConfiguration().GetVersion()));
          pSNMP->setStatus(actDevice.GetDeviceStatus().GetSNMPStatus());

          // Get RESTful
          MoxaClassBased::RESTfulType* pRESTful = pDiscoveredBridgeType->getRESTful();
          pRESTful->setPort(OpcUa_UInt16(actDevice.GetRestfulConfiguration().GetPort()));
          pRESTful->setStatus(actDevice.GetDeviceStatus().GetRESTfulStatus());
        }

        pBroadcastSearchAndIPSettingStateMachineType->setBroadcastSearchAndIPSettingState(
            MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscovered);
      } break;

      case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::LinkSequenceDetecting: {
        ActProject actProject;
        act_status = act::core::g_core.GetProject(project_id, actProject);
        if (!IsActStatusSuccess(act_status)) {
          return setErrorInformation(
              pUaNode, static_cast<OpcUa_UInt32>(act_status->GetStatus()),
              UaString(act_status->GetErrorMessage().toStdString().c_str()),
              MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::LinkSequenceDetectingFailed);
        }

        // Assign device hop count
        for (ActDevice actDevice : actProject.broadcast_search_devices_) {
          UaString macAddress(actDevice.GetMacAddress().toStdString().c_str());
          if (!pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap.contains(macAddress)) {
            continue;
          }
          MoxaClassBased::DiscoveredBridgeType* pDiscoveredBridgeType =
              (MoxaClassBased::DiscoveredBridgeType*)
                  pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap.value(macAddress);

          pDiscoveredBridgeType->setHopCount(actDevice.GetDistance());
        }

        pBroadcastSearchAndIPSettingStateMachineType->setBroadcastSearchAndIPSettingState(
            MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::LinkSequenceDetected);
      } break;
      case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::IpConfiguring:
        pBroadcastSearchAndIPSettingStateMachineType->setBroadcastSearchAndIPSettingState(
            MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::IpConfigured);
        break;
      default:
        break;
    }
  } else {
    switch (state) {
      case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscovering:
        setErrorInformation(pUaNode, static_cast<OpcUa_UInt32>(status.GetStatus()),
                            UaString(status.GetErrorMessage().toStdString().c_str()),
                            MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscoveringFailed);
        break;

      case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::RetryConnection:
        setErrorInformation(pUaNode, static_cast<OpcUa_UInt32>(status.GetStatus()),
                            UaString(status.GetErrorMessage().toStdString().c_str()),
                            MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::DeviceDiscovered);
        break;

      case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::LinkSequenceDetecting:
        setErrorInformation(pUaNode, static_cast<OpcUa_UInt32>(status.GetStatus()),
                            UaString(status.GetErrorMessage().toStdString().c_str()),
                            MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::LinkSequenceDetectingFailed);
        break;

      case MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::IpConfiguring:
        setErrorInformation(pUaNode, static_cast<OpcUa_UInt32>(status.GetStatus()),
                            UaString(status.GetErrorMessage().toStdString().c_str()),
                            MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::IpConfiguringFailed);
        break;
      default:
        break;
    }
  }

  return;
}

UaStatus getDiscoveredDevicesMethod(UaNode* pUaNode, MoxaClassBased::DiscoveredDeviceDataTypes& discoveredDevices) {
  UaStatus ret;

  MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType* pBroadcastSearchAndIPSettingStateMachineType =
      (MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType*)pUaNode;

  // MoxaClassBased_DiscoveredDeviceDataType
  //     discoveredDevice[pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap.count()];
  MoxaClassBased_DiscoveredDeviceDataType* discoveredDevice =
      new MoxaClassBased_DiscoveredDeviceDataType[pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap
                                                      .count()];
  OpcUa_UInt32 discoveredDeviceIdx = 0;
  foreach (void* value, pBroadcastSearchAndIPSettingStateMachineType->discoveredBridgeMap) {
    MoxaClassBased::DiscoveredBridgeType* pDiscoveredBridgeType = (MoxaClassBased::DiscoveredBridgeType*)value;

    MoxaClassBased::DiscoveredDeviceDataType discoveredDeviceDataType;
    discoveredDeviceDataType.setOriginalIpAddress(pDiscoveredBridgeType->getIpSetting()->getIpAddress());
    discoveredDeviceDataType.setNewAssignedIpAddress(pDiscoveredBridgeType->getNewAssignedIpAddress());
    discoveredDeviceDataType.setMacAddress(pDiscoveredBridgeType->getMacAddress());
    discoveredDeviceDataType.setSNMPConnected(pDiscoveredBridgeType->getSNMP()->getStatus());
    discoveredDeviceDataType.setRESTfulConnected(pDiscoveredBridgeType->getRESTful()->getStatus());
    discoveredDeviceDataType.setHopCount(pDiscoveredBridgeType->getHopCount());

    discoveredDeviceDataType.copyTo(&discoveredDevice[discoveredDeviceIdx++]);
  }
  discoveredDevices.setDiscoveredDeviceDataTypes(discoveredDeviceIdx, discoveredDevice);

  delete[] discoveredDevice;
  return ret;
}
}  // namespace ClassBased