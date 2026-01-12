#include "classbased_perstreampriority.h"

namespace ClassBased {

static ACT_STATUS checkPerStreamPriorityNode(ActInterfaceStadPortEntry& interface_stad_port_entry) {
  ACT_STATUS_INIT();

  for (ActStadPortEntry stad_port_entry : interface_stad_port_entry.GetStadPortEntries()) {
    if (stad_port_entry.GetEthertypeValue() < 0 || stad_port_entry.GetEthertypeValue() > 65535) {
      QString error_msg = QString("Invalid: The valid range of EtherType is 0x0000 to 0xFFFF");
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    if (stad_port_entry.GetSubtypeEnable() == 1 &&
        (stad_port_entry.GetSubtypeValue() < 0 || stad_port_entry.GetSubtypeValue() > 255)) {
      QString error_msg = QString("Invalid: The valid range of SubType is 0x00 to 0xFF");
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    if (stad_port_entry.GetVlanId() < 1 || stad_port_entry.GetVlanId() > 4094) {
      QString error_msg = QString("Invalid: The valid range of VlanId is 1 to 4094");
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    if (stad_port_entry.GetVlanPcp() < 0 || stad_port_entry.GetVlanPcp() > 7) {
      QString error_msg = QString("Invalid: The valid range of PriorityCodePoint is 0 to 7");
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  return act_status;
}

UaStatus updatePerStreamPriorityNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                     UaString& errorMessage) {
  UaStatus ret;

  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetPerStreamPriority() &&
      !device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetPerStreamPriorityV2()) {
    return ret;
  }

  const ActStadPortTable& stream_priority_ingress_table =
      project.GetDeviceConfig().GetStreamPriorityIngressTables()[device.GetId()];
  const QSet<ActInterfaceStadPortEntry>& interface_stad_port_entries =
      stream_priority_ingress_table.GetInterfaceStadPortEntries();

  for (ActInterface intf : device.GetInterfaces()) {
    UaNodeId interfaceNodeId =
        pMoxaNodeManager->getInterfaceNodeId(project.GetId(), device.GetId(), intf.GetInterfaceId());

    MoxaClassBased::EthernetInterfaceType* pEthernetInterfaceType =
        (MoxaClassBased::EthernetInterfaceType*)pMoxaNodeManager->getNode(interfaceNodeId);

    MoxaClassBased::PerStreamPriorityFolderType* pPerStreamPriorityFolderType =
        pEthernetInterfaceType->getPerStreamPriorities();

    UaReferenceDescriptions references;
    pMoxaNodeManager->getNodeReference(pPerStreamPriorityFolderType->nodeId(), OpcUa_False, references);
    for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
      if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_PerStreamPriorityType) {
        continue;
      }

      UaNode* pNode = pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);
      ret = pMoxaNodeManager->deleteUaNode(pNode, OpcUa_True, OpcUa_True, OpcUa_True);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Remove per-stream priority %1 failed").arg(pNode->browseName().toString());
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }

    ActInterfaceStadPortEntry interface_stad_port_entry(intf.GetInterfaceId());
    QSet<ActInterfaceStadPortEntry>::const_iterator interface_stad_port_entry_iter =
        interface_stad_port_entries.find(interface_stad_port_entry);
    if (interface_stad_port_entry_iter != interface_stad_port_entries.end()) {
      interface_stad_port_entry = *interface_stad_port_entry_iter;

      for (ActStadPortEntry stad_port_entry : interface_stad_port_entry.GetStadPortEntries()) {
        UaString browse_name;
        if (stad_port_entry.GetType().isEmpty()) {
          continue;
        } else if (*stad_port_entry.GetType().begin() == ActStreamPriorityTypeEnum::kEthertype) {
          QString etherType =
              QString("0X%1").arg(stad_port_entry.GetEthertypeValue(), 4, 16, QLatin1Char('0')).toUpper();
          browse_name = UaString("%1_VLAN_%2_PCP_%3_L2_%4")
                            .arg(int(stad_port_entry.GetIngressIndex()))
                            .arg(int(stad_port_entry.GetVlanId()))
                            .arg(int(stad_port_entry.GetVlanPcp()))
                            .arg(etherType.toStdString().c_str());
        } else if (*stad_port_entry.GetType().begin() == ActStreamPriorityTypeEnum::kTcp) {
          browse_name = UaString("%1_VLAN_%2_PCP_%3_L3TCP_%4")
                            .arg(int(stad_port_entry.GetIngressIndex()))
                            .arg(int(stad_port_entry.GetVlanId()))
                            .arg(int(stad_port_entry.GetVlanPcp()))
                            .arg(int(stad_port_entry.GetTcpPort()));
        } else if (*stad_port_entry.GetType().begin() == ActStreamPriorityTypeEnum::kUdp) {
          browse_name = UaString("%1_VLAN_%2_PCP_%3_L3UDP_%4")
                            .arg(int(stad_port_entry.GetIngressIndex()))
                            .arg(int(stad_port_entry.GetVlanId()))
                            .arg(int(stad_port_entry.GetVlanPcp()))
                            .arg(int(stad_port_entry.GetUdpPort()));
        } else {
          continue;
        }

        // NodeId for the node to create
        UaNodeId perStreamPriorityNodeId(
            UaString("%1.%2").arg(pPerStreamPriorityFolderType->nodeId().toString()).arg(browse_name),
            pMoxaNodeManager->getNameSpaceIndex());

        UaNode* pNode = pMoxaNodeManager->getNode(perStreamPriorityNodeId);
        if (pNode) {
          ret = pMoxaNodeManager->deleteUaNode(pNode, OpcUa_True, OpcUa_True, OpcUa_True);
          if (ret.isNotGood()) {
            errorCode = M_UA_INTERNAL_ERROR;
            errorMessage =
                UaString("Invalid: Remove per-stream priority %1 failed").arg(pNode->browseName().toString());
            qDebug() << errorMessage.toUtf8();
            return ret;
          }
        }

        // If succeeded, create the new node to the OPC UA nodemanager
        MoxaClassBased::PerStreamPriorityType* pPerStreamPriorityType = new MoxaClassBased::PerStreamPriorityType(
            perStreamPriorityNodeId, browse_name, pMoxaNodeManager->getNameSpaceIndex(),
            pMoxaNodeManager->getNodeManagerConfig());
        if (!pPerStreamPriorityType) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: Allocate memory failed");
          qDebug() << errorMessage.toUtf8();
          return ret;
        }

        ret =
            pMoxaNodeManager->addNodeAndReference(pPerStreamPriorityFolderType->nodeId(),
                                                  pPerStreamPriorityType->getUaReferenceLists(), OpcUaId_HasComponent);
        if (ret.isNotGood()) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: Create per-stream priority node failed");
          qDebug() << errorMessage.toUtf8();
          return ret;
        }

        pPerStreamPriorityType->getVlanTag()->setVlanId(stad_port_entry.GetVlanId());
        pPerStreamPriorityType->getVlanTag()->setPriorityCodePoint(stad_port_entry.GetVlanPcp());
        QSet<ActStreamPriorityTypeEnum>& type = stad_port_entry.GetType();
        if (!type.isEmpty()) {
          switch (*type.begin()) {
            case ActStreamPriorityTypeEnum::kEthertype:
              pPerStreamPriorityType->setPerStreamPriorityMode(
                  MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L2);
              pPerStreamPriorityType->setEtherType(stad_port_entry.GetEthertypeValue());
              pPerStreamPriorityType->setSubtypeEnable(stad_port_entry.GetSubtypeEnable() == 1 ? true : false);
              pPerStreamPriorityType->setSubtype(stad_port_entry.GetSubtypeValue());
              break;
            case ActStreamPriorityTypeEnum::kTcp:
              pPerStreamPriorityType->setPerStreamPriorityMode(
                  MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3TCP);
              pPerStreamPriorityType->setTCPPort(stad_port_entry.GetTcpPort());
              break;
            case ActStreamPriorityTypeEnum::kUdp:
              pPerStreamPriorityType->setPerStreamPriorityMode(
                  MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3UDP);
              pPerStreamPriorityType->setUDPPort(stad_port_entry.GetUdpPort());
              break;
          }
        }
      }
    }
  }

  return ret;
}

UaStatus addPerStreamPriorityMethod(const UaNodeId& perStreamPriorityFolderNodeId,
                                    const MoxaClassBased::PerStreamPriorityDataType& configuration, UaNodeId& nodeId,
                                    OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(perStreamPriorityFolderNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(perStreamPriorityFolderNodeId);
  qint64 interface_id = pMoxaNodeManager->getInterfaceId(perStreamPriorityFolderNodeId);

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

  if (!project.GetDeviceConfig().GetStreamPriorityIngressTables().contains(device.GetId())) {
    project.GetDeviceConfig().GetStreamPriorityIngressTables().insert(device.GetId(), ActStadPortTable(device.GetId()));
  }

  QSet<ActInterfaceStadPortEntry>& interface_stad_port_entries =
      project.GetDeviceConfig().GetStreamPriorityIngressTables()[device.GetId()].GetInterfaceStadPortEntries();

  ActInterfaceStadPortEntry interface_stad_port_entry(interface_id);
  QSet<ActInterfaceStadPortEntry>::iterator interface_stad_port_entry_iter =
      interface_stad_port_entries.find(interface_stad_port_entry);

  if (interface_stad_port_entry_iter != interface_stad_port_entries.end()) {
    interface_stad_port_entry = *interface_stad_port_entry_iter;
    interface_stad_port_entries.erase(interface_stad_port_entry_iter);
  }

  QSet<ActStadPortEntry>& stad_port_entries = interface_stad_port_entry.GetStadPortEntries();
  if (stad_port_entries.size() == 10) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: The maximum number of per-stream priority is 10");
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActStadPortEntry stad_port_entry;
  stad_port_entry.SetPortId(interface_id);
  stad_port_entry.SetIngressIndex(stad_port_entries.size());
  stad_port_entry.SetIndexEnable(1);
  if (!configuration.isVlanTagSet()) {
    QString error_msg = QString("Invalid: VlanTag is not set");
    qCritical() << error_msg.toStdString().c_str();
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString(error_msg.toStdString().c_str());
    return ret;
  }
  stad_port_entry.SetVlanId(qint32(configuration.getVlanTag().getVlanId()));
  stad_port_entry.SetVlanPcp(qint32(configuration.getVlanTag().getPriorityCodePoint()));

  UaString browse_name;
  switch (configuration.getPerStreamPriorityMode()) {
    case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L2: {
      stad_port_entry.SetType({ActStreamPriorityTypeEnum::kEthertype});
      if (!configuration.isEtherTypeSet()) {
        QString error_msg = QString("Invalid: EtherType is not set");
        qCritical() << error_msg.toStdString().c_str();
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString(error_msg.toStdString().c_str());
        return ret;
      }
      stad_port_entry.SetEthertypeValue(qint32(configuration.getEtherType()));
      if (configuration.isSubtypeEnableSet()) {
        stad_port_entry.SetSubtypeEnable(configuration.getSubtypeEnable() ? 1 : 2);
      }
      if (configuration.isSubtypeSet()) {
        stad_port_entry.SetSubtypeValue(qint32(configuration.getSubtype()));
      }
      QString etherType = QString("0X%1").arg(stad_port_entry.GetEthertypeValue(), 4, 16, QLatin1Char('0')).toUpper();
      browse_name = UaString("%1_VLAN_%2_PCP_%3_L2_%4")
                        .arg(int(stad_port_entry.GetIngressIndex()))
                        .arg(int(stad_port_entry.GetVlanId()))
                        .arg(int(stad_port_entry.GetVlanPcp()))
                        .arg(etherType.toStdString().c_str());
    } break;
    case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3TCP: {
      stad_port_entry.SetType({ActStreamPriorityTypeEnum::kTcp});
      if (!configuration.isTCPPortSet()) {
        QString error_msg = QString("Invalid: TCPPort is not set");
        qCritical() << error_msg.toStdString().c_str();
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString(error_msg.toStdString().c_str());
        return ret;
      }
      stad_port_entry.SetTcpPort(qint32(configuration.getTCPPort()));
      browse_name = UaString("%1_VLAN_%2_PCP_%3_L3TCP_%4")
                        .arg(int(stad_port_entry.GetIngressIndex()))
                        .arg(int(stad_port_entry.GetVlanId()))
                        .arg(int(stad_port_entry.GetVlanPcp()))
                        .arg(int(stad_port_entry.GetTcpPort()));
    } break;
    case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3UDP: {
      stad_port_entry.SetType({ActStreamPriorityTypeEnum::kUdp});
      if (!configuration.isUDPPortSet()) {
        QString error_msg = QString("Invalid: UDPPort is not set");
        qCritical() << error_msg.toStdString().c_str();
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString(error_msg.toStdString().c_str());
        return ret;
      }
      stad_port_entry.SetUdpPort(qint32(configuration.getUDPPort()));
      browse_name = UaString("%1_VLAN_%2_PCP_%3_L3UDP_%4")
                        .arg(int(stad_port_entry.GetIngressIndex()))
                        .arg(int(stad_port_entry.GetVlanId()))
                        .arg(int(stad_port_entry.GetVlanPcp()))
                        .arg(int(stad_port_entry.GetUdpPort()));
    } break;
  }

  stad_port_entries.insert(stad_port_entry);

  interface_stad_port_entries.insert(interface_stad_port_entry);

  act_status = checkPerStreamPriorityNode(interface_stad_port_entry);
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
  pMoxaNodeManager->getNodeReference(perStreamPriorityFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_PerStreamPriorityType) {
      continue;
    }
    MoxaClassBased::PerStreamPriorityType* pPerStreamPriorityType =
        (MoxaClassBased::PerStreamPriorityType*)pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);

    if (pPerStreamPriorityType->browseName().toString() == browse_name) {
      nodeId = pPerStreamPriorityType->nodeId();
      break;
    }
  }

  return ret;
}

UaStatus setPerStreamPriorityMethod(const UaNodeId& perStreamPriorityNodeId,
                                    const MoxaClassBased::PerStreamPriorityDataType& configuration,
                                    OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  MoxaClassBased::PerStreamPriorityType* pPerStreamPriorityType =
      (MoxaClassBased::PerStreamPriorityType*)pMoxaNodeManager->getNode(perStreamPriorityNodeId);
  if (!pPerStreamPriorityType) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: The per-stream priority node is not found");
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  QString browse_name(pPerStreamPriorityType->browseName().toString().toUtf8());
  QStringList str_list = browse_name.split("_");
  qint32 index = qint32(str_list[0].toUInt());

  qint64 project_id = pMoxaNodeManager->getProjectId(perStreamPriorityNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(perStreamPriorityNodeId);
  qint64 interface_id = pMoxaNodeManager->getInterfaceId(perStreamPriorityNodeId);

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

  if (!project.GetDeviceConfig().GetStreamPriorityIngressTables().contains(device_id)) {
    project.GetDeviceConfig().GetStreamPriorityIngressTables().insert(device_id, ActStadPortTable(device_id));
  }

  ActStadPortTable& stream_priority_ingress_table =
      project.GetDeviceConfig().GetStreamPriorityIngressTables()[device_id];
  QSet<ActInterfaceStadPortEntry>& interface_stad_port_entries =
      stream_priority_ingress_table.GetInterfaceStadPortEntries();
  ActInterfaceStadPortEntry interface_stad_port_entry(interface_id);
  QSet<ActInterfaceStadPortEntry>::iterator interface_stad_port_entry_iter =
      interface_stad_port_entries.find(interface_stad_port_entry);
  if (interface_stad_port_entry_iter != interface_stad_port_entries.end()) {
    interface_stad_port_entry = *interface_stad_port_entry_iter;
    interface_stad_port_entries.erase(interface_stad_port_entry_iter);
  }

  QSet<ActStadPortEntry> stad_port_entries;
  for (ActStadPortEntry stad_port_entry : interface_stad_port_entry.GetStadPortEntries()) {
    if (stad_port_entry.GetIngressIndex() == index) {
      if (configuration.isVlanTagSet()) {
        stad_port_entry.SetVlanId(qint32(configuration.getVlanTag().getVlanId()));
        stad_port_entry.SetVlanPcp(qint32(configuration.getVlanTag().getPriorityCodePoint()));
      }
      switch (configuration.getPerStreamPriorityMode()) {
        case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L2:
          if (!stad_port_entry.GetType().contains(ActStreamPriorityTypeEnum::kEthertype)) {
            if (!configuration.isEtherTypeSet()) {
              QString error_msg = QString("Invalid: EtherType is not set");
              qCritical() << error_msg.toStdString().c_str();
              errorCode = M_UA_INTERNAL_ERROR;
              errorMessage = UaString(error_msg.toStdString().c_str());
              return ret;
            }
          }

          stad_port_entry.SetType({ActStreamPriorityTypeEnum::kEthertype});
          if (configuration.isEtherTypeSet()) {
            stad_port_entry.SetEthertypeValue(qint32(configuration.getEtherType()));
          }
          if (configuration.isSubtypeEnableSet()) {
            stad_port_entry.SetSubtypeEnable(configuration.getSubtypeEnable() ? 1 : 2);
          }
          if (configuration.isSubtypeSet()) {
            stad_port_entry.SetSubtypeValue(qint32(configuration.getSubtype()));
          }
          break;
        case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3TCP:
          if (!stad_port_entry.GetType().contains(ActStreamPriorityTypeEnum::kTcp)) {
            if (!configuration.isTCPPortSet()) {
              QString error_msg = QString("Invalid: TCPPort is not set");
              qCritical() << error_msg.toStdString().c_str();
              errorCode = M_UA_INTERNAL_ERROR;
              errorMessage = UaString(error_msg.toStdString().c_str());
              return ret;
            }
          }
          stad_port_entry.SetType({ActStreamPriorityTypeEnum::kTcp});
          if (configuration.isTCPPortSet()) {
            stad_port_entry.SetTcpPort(qint32(configuration.getTCPPort()));
          }
          break;
        case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3UDP:
          if (!stad_port_entry.GetType().contains(ActStreamPriorityTypeEnum::kUdp)) {
            if (!configuration.isUDPPortSet()) {
              QString error_msg = QString("Invalid: UDPPort is not set");
              qCritical() << error_msg.toStdString().c_str();
              errorCode = M_UA_INTERNAL_ERROR;
              errorMessage = UaString(error_msg.toStdString().c_str());
              return ret;
            }
          }
          stad_port_entry.SetType({ActStreamPriorityTypeEnum::kUdp});
          if (configuration.isUDPPortSet()) {
            stad_port_entry.SetUdpPort(qint32(configuration.getUDPPort()));
          }
          break;
      }
    }
    stad_port_entries.insert(stad_port_entry);
  }

  interface_stad_port_entry.SetStadPortEntries(stad_port_entries);

  interface_stad_port_entries.insert(interface_stad_port_entry);

  act_status = checkPerStreamPriorityNode(interface_stad_port_entry);
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

UaStatus removePerStreamPriorityMethod(const UaNodeId& perStreamPriorityFolderNodeId,
                                       const UaNodeId& perStreamPriorityNodeId, OpcUa_UInt32& errorCode,
                                       UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  UaReferenceDescriptions references;
  MoxaClassBased::PerStreamPriorityType* pPerStreamPriorityType = NULL;
  pMoxaNodeManager->getNodeReference(perStreamPriorityFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_PerStreamPriorityType) {
      continue;
    }

    if (UaNodeId(references[idx].NodeId.NodeId) == perStreamPriorityNodeId) {
      pPerStreamPriorityType =
          (MoxaClassBased::PerStreamPriorityType*)pMoxaNodeManager->getNode(perStreamPriorityNodeId);
      break;
    }
  }

  if (pPerStreamPriorityType == NULL) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: NodeId %1 is not found").arg(perStreamPriorityNodeId.toFullString());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  QString browse_name(pPerStreamPriorityType->browseName().toString().toUtf8());
  QStringList str_list = browse_name.split("_");
  qint32 index = qint32(str_list[0].toUInt());

  qint64 project_id = pMoxaNodeManager->getProjectId(perStreamPriorityFolderNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(perStreamPriorityFolderNodeId);
  qint64 interface_id = pMoxaNodeManager->getInterfaceId(perStreamPriorityFolderNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActStadPortTable& stream_priority_ingress_table =
      project.GetDeviceConfig().GetStreamPriorityIngressTables()[device_id];
  QSet<ActInterfaceStadPortEntry>& interface_stad_port_entries =
      stream_priority_ingress_table.GetInterfaceStadPortEntries();
  ActInterfaceStadPortEntry interface_stad_port_entry(interface_id);
  QSet<ActInterfaceStadPortEntry>::iterator interface_stad_port_entry_iter =
      interface_stad_port_entries.find(interface_stad_port_entry);
  if (interface_stad_port_entry_iter != interface_stad_port_entries.end()) {
    interface_stad_port_entry = *interface_stad_port_entry_iter;
    interface_stad_port_entries.erase(interface_stad_port_entry_iter);
  }

  QSet<ActStadPortEntry> stad_port_entries;
  for (ActStadPortEntry stad_port_entry : interface_stad_port_entry.GetStadPortEntries()) {
    if (stad_port_entry.GetIngressIndex() == index) {
      continue;
    }
    if (stad_port_entry.GetIngressIndex() > index) {
      stad_port_entry.SetIngressIndex(stad_port_entry.GetIngressIndex() - 1);
    }
    stad_port_entries.insert(stad_port_entry);
  }
  interface_stad_port_entry.SetStadPortEntries(stad_port_entries);

  interface_stad_port_entries.insert(interface_stad_port_entry);

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