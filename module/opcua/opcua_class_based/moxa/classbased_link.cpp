#include "classbased_link.h"

namespace ClassBased {

UaStatus updateLinkNodes(const ActProject& project, OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  UaNodeId projectNodeId = pMoxaNodeManager->getProjectNodeId(project.GetId());
  MoxaClassBased::ProjectType* pProjectType = (MoxaClassBased::ProjectType*)pMoxaNodeManager->getNode(projectNodeId);
  MoxaClassBased::LinkFolderType* pLinkFolderType = pProjectType->getLinks();

  for (ActLink link : project.GetLinks()) {
    qint64 link_id = link.GetId();
    qint64 src_dev_id = link.GetSourceDeviceId();
    qint64 dst_dev_id = link.GetDestinationDeviceId();
    qint64 src_inf_id = link.GetSourceInterfaceId();
    qint64 dst_inf_id = link.GetDestinationInterfaceId();

    ActDevice src_dev;
    project.GetDeviceById(src_dev, src_dev_id);
    UaString src_dev_ip(src_dev.GetIpv4().GetIpAddress().toStdString().c_str());
    UaString src_inf_name;
    for (ActInterface intf : src_dev.GetInterfaces()) {
      if (intf.GetInterfaceId() == src_inf_id) {
        src_inf_name = intf.GetInterfaceName().toStdString().c_str();
      }
    }

    ActDevice dst_dev;
    project.GetDeviceById(dst_dev, dst_dev_id);
    UaString dst_dev_ip(dst_dev.GetIpv4().GetIpAddress().toStdString().c_str());
    UaString dst_inf_name;
    for (ActInterface intf : dst_dev.GetInterfaces()) {
      if (intf.GetInterfaceId() == dst_inf_id) {
        dst_inf_name = intf.GetInterfaceName().toStdString().c_str();
      }
    }

    // browseName
    UaString SourceCommLinkTo = UaString("%1_%2").arg(src_dev_ip).arg(src_inf_name);
    UaString DestinationCommLinkTo = UaString("%1_%2").arg(dst_dev_ip).arg(dst_inf_name);
    UaString browseName = UaString("%1_%2").arg(SourceCommLinkTo).arg(DestinationCommLinkTo);

    UaNodeId linkNodeId = pMoxaNodeManager->getLinkNodeId(project.GetId(), link_id);
    MoxaClassBased::LinkType* pLinkType = (MoxaClassBased::LinkType*)pMoxaNodeManager->getNode(linkNodeId);
    if (pLinkType == NULL) {
      // NodeId for the node to create

      // If succeeded, create the new node to the OPC UA nodemanager
      pLinkType = new MoxaClassBased::LinkType(linkNodeId, browseName, pMoxaNodeManager->getNameSpaceIndex(),
                                               pMoxaNodeManager->getNodeManagerConfig());
      if (!pLinkType) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Allocate memory failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      ret = pMoxaNodeManager->addNodeAndReference(pLinkFolderType->nodeId(), pLinkType->getUaReferenceLists(),
                                                  OpcUaId_HasComponent);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Add node reference failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    } else {
      if (pLinkType->browseName().toString() == browseName) {
        continue;
      }

      pLinkType->setBrowseName(UaQualifiedName(browseName, pMoxaNodeManager->getNameSpaceIndex()));
      pLinkType->setDisplayName(UaLocalizedText(UaString(), browseName));

      UaReferenceDescriptions references;
      pMoxaNodeManager->getNodeReference(pLinkType->nodeId(), OpcUa_False, references);
      OpcUa_UInt32 idx = 0;
      for (idx = 0; idx < references.length(); idx++) {
        if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_EthernetInterfaceType &&
            references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_EthernetInterfaceType) {
          continue;
        }

        UaNodeId interfaceNodeId(references[idx].NodeId.NodeId);
        ret = pMoxaNodeManager->deleteUaReference(
            pLinkType->nodeId(), interfaceNodeId,
            UaNodeId(OpcUaClassBnmId_CommLinkTo, pOpcUaClassBnm->getNameSpaceIndex()));
        if (ret.isNotGood()) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: Delete interface link failed");
          qDebug() << errorMessage.toUtf8();
          return ret;
        }
      }
    }

    pLinkType->setSpeed(OpcUa_UInt64(link.GetSpeed()));
    switch (link.GetCableType()) {
      case ActCableTypeEnum::kCopper:
        pLinkType->setCableType(MoxaClassBased::LinkCableType::LinkCableType_Copper);
        break;
      case ActCableTypeEnum::kFiber:
        pLinkType->setCableType(MoxaClassBased::LinkCableType::LinkCableType_Fiber);
        break;
    }
    pLinkType->setCableLength(OpcUa_UInt16(link.GetCableLength()));
    pLinkType->setPropagationDelay(OpcUa_UInt32(link.GetPropagationDelay()));

    // Find source and destination interface
    UaNodeId sourceNodeId = pMoxaNodeManager->getInterfaceNodeId(project.GetId(), src_dev_id, src_inf_id);
    UaNodeId destinationNodeId = pMoxaNodeManager->getInterfaceNodeId(project.GetId(), dst_dev_id, dst_inf_id);

    // Add source interface link
    ret = pMoxaNodeManager->addUaReference(pLinkType->nodeId(), sourceNodeId,
                                           UaNodeId(OpcUaClassBnmId_CommLinkTo, pOpcUaClassBnm->getNameSpaceIndex()));
    if (ret.isNotGood()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Add source interface link failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    // Add destination interface link
    ret = pMoxaNodeManager->addUaReference(pLinkType->nodeId(), destinationNodeId,
                                           UaNodeId(OpcUaClassBnmId_CommLinkTo, pOpcUaClassBnm->getNameSpaceIndex()));
    if (ret.isNotGood()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Add destination interface link failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }
  }

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(pLinkFolderType->nodeId(), OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_LinkType) {
      continue;
    }
    UaNodeId linkNodeId(references[idx].NodeId.NodeId);
    qint64 link_id = pMoxaNodeManager->getLinkId(linkNodeId);
    ActLink link;
    act_status = project.GetLinkById(link, link_id);
    if (!IsActStatusSuccess(act_status)) {
      MoxaClassBased::LinkType* pLinkType = (MoxaClassBased::LinkType*)pMoxaNodeManager->getNode(linkNodeId);
      ret = pMoxaNodeManager->deleteUaNode(pLinkType, OpcUa_True, OpcUa_True, OpcUa_True);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Remove node %1 failed").arg(linkNodeId.toFullString());
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }
  }

  return ret;
}

UaStatus addLinkMethod(const UaNodeId& folderNodeId, const MoxaClassBased::LinkDataType& configuration,
                       UaNodeId& LinkNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(folderNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActDevice src_dev, dst_dev;
  ActInterface src_intf, dst_intf;
  for (ActDevice device : project.GetDevices()) {
    UaString ipAddress = UaString(device.GetIpv4().GetIpAddress().toStdString().c_str());
    if (ipAddress == configuration.getSourceIpAddress()) {
      src_dev = device;
      QList<ActInterface> interfaces = device.GetInterfaces();
      for (int i = 0; i < interfaces.size(); ++i) {
        UaString interfaceName = UaString(interfaces.at(i).GetInterfaceName().toStdString().c_str());
        if (interfaceName == configuration.getSourceInterfaceName()) {
          src_intf = interfaces.at(i);
        }
      }
    }
    if (ipAddress == configuration.getDestinationIpAddress()) {
      dst_dev = device;
      QList<ActInterface> interfaces = device.GetInterfaces();
      for (int i = 0; i < interfaces.size(); ++i) {
        UaString interfaceName = UaString(interfaces.at(i).GetInterfaceName().toStdString().c_str());
        if (interfaceName == configuration.getDestinationInterfaceName()) {
          dst_intf = interfaces.at(i);
        }
      }
    }
  }

  if (src_dev.GetId() == -1 || dst_dev.GetId() == -1 || src_intf.GetInterfaceId() == -1 ||
      dst_intf.GetInterfaceId() == -1) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Source or destination device/interface is not exist");
    qDebug() << errorMessage.toUtf8();
    return ret;
  } else if (src_dev.GetId() == dst_dev.GetId() && src_intf.GetInterfaceId() == dst_intf.GetInterfaceId()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Source and destination device/interface are the same");
    qDebug() << errorMessage.toUtf8();
    return ret;
  } else if (src_intf.GetUsed() || dst_intf.GetUsed()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Source or destination interface is already used");
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActLink link;
  link.SetSourceDeviceId(src_dev.GetId());
  link.SetSourceDeviceIp(src_dev.GetIpv4().GetIpAddress());
  link.SetSourceInterfaceId(src_intf.GetInterfaceId());
  link.SetDestinationDeviceId(dst_dev.GetId());
  link.SetDestinationDeviceIp(dst_dev.GetIpv4().GetIpAddress());
  link.SetDestinationInterfaceId(dst_intf.GetInterfaceId());

  QList<qint64> src_support_speeds = src_intf.GetSupportSpeeds();
  QList<qint64> dst_support_speeds = dst_intf.GetSupportSpeeds();
  while (true) {
    if (src_support_speeds.isEmpty() || dst_support_speeds.isEmpty()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Link speed is not match");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }
    if (src_support_speeds.last() > dst_support_speeds.last()) {
      src_support_speeds.pop_back();
    } else if (src_support_speeds.last() < dst_support_speeds.last()) {
      dst_support_speeds.pop_back();
    } else {
      link.SetSpeed(src_support_speeds.last());
      break;
    }
  }

  QSet<ActCableTypeEnum> cable_types = src_intf.GetCableTypes().intersect(dst_intf.GetCableTypes());
  if (cable_types.isEmpty()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Link cable type is not match");
    qDebug() << errorMessage.toUtf8();
    return ret;
  }
  link.SetCableType(cable_types.values().first());

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.CreateLink(project_id, link);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}

UaStatus removeLinkMethod(const UaNodeId& LinkNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(LinkNodeId);
  qint64 link_id = pMoxaNodeManager->getLinkId(LinkNodeId);

  QMutexLocker lock(&act::core::g_core.mutex_);

  // Delete ACT link
  act_status = act::core::g_core.DeleteLink(project_id, link_id);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}

}  // namespace ClassBased