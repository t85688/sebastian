#include "classbased_unicaststaticforward.h"

namespace ClassBased {

static ACT_STATUS checkUnicastStaticForwardNode(ActProject& project, ActDevice& device,
                                                ActStaticForwardEntry& static_forward_entry) {
  ACT_STATUS_INIT();

  if (static_forward_entry.GetVlanId() < 1 || static_forward_entry.GetVlanId() > 4094) {
    QString error_msg = QString("Invalid: The valid range of VlanId is 1 to 4094");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  QSet<ActVlanStaticEntry>& vlan_static_entries =
      project.GetDeviceConfig().GetVlanTables()[device.GetId()].GetVlanStaticEntries();
  ActVlanStaticEntry vlan_static_entry(static_forward_entry.GetVlanId());
  QSet<ActVlanStaticEntry>::iterator vlan_static_entry_iter = vlan_static_entries.find(vlan_static_entry);
  if (vlan_static_entry_iter == vlan_static_entries.end()) {
    QString error_msg =
        QString("Invalid: The VlanId %1 isn't exist in VLAN table").arg(static_forward_entry.GetVlanId());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  } else {
    vlan_static_entry = *vlan_static_entry_iter;
    if (!vlan_static_entry.GetEgressPorts().contains(static_forward_entry.GetEgressPorts())) {
      QStringList static_forward_entry_egress_ports, vlan_static_entry_egress_ports;
      for (qint64 egress_port : static_forward_entry.GetEgressPorts()) {
        static_forward_entry_egress_ports << QString::number(egress_port);
      }
      for (qint64 egress_port : vlan_static_entry.GetEgressPorts()) {
        vlan_static_entry_egress_ports << QString::number(egress_port);
      }
      QString error_msg = QString("Invalid: EgressPort %1 may not be in the VLAN Table (VlanID: %2, EgressPorts: %3)")
                              .arg(static_forward_entry_egress_ports.join(", "))
                              .arg(static_forward_entry.GetVlanId())
                              .arg(vlan_static_entry_egress_ports.join(", "));
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  if (static_forward_entry.GetMAC() == QString()) {
    QString error_msg = QString("Invalid: The MAC Address cannot be empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  } else {
    QRegularExpression regex(R"(^([0-9A-Fa-f]{2}([-:])){5}([0-9A-Fa-f]{2})$)");
    if (!regex.match(static_forward_entry.GetMAC()).hasMatch()) {
      QString error_msg = QString("Invalid: The MAC Address format is xx:xx:xx:xx:xx:xx or xx-xx-xx-xx-xx-xx");
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  QSet<ActStaticForwardEntry> static_forward_entries =
      project.GetDeviceConfig().GetMulticastStaticForwardTables()[device.GetId()].GetStaticForwardEntries();
  if (static_forward_entries.find(static_forward_entry) != static_forward_entries.end()) {
    QString error_msg = QString("Invalid: The entry is duplicated in the multicast static forward table.");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (qint64 egress_port : static_forward_entry.GetEgressPorts()) {
    if (egress_port == 0 || egress_port > device.GetInterfaces().size()) {
      QString error_msg = QString("Invalid: The EgressPorts is the array of port id, the valid range is 1 to %1")
                              .arg(device.GetInterfaces().size());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  return act_status;
}

UaStatus updateUnicastStaticForwardNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                        UaString& errorMessage) {
  UaStatus ret;

  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetStaticForwardSetting().GetUnicast()) {
    return ret;
  }

  UaNodeId deviceNodeId = pMoxaNodeManager->getDeviceNodeId(project.GetId(), device.GetId());
  MoxaClassBased::BridgeType* pBridgeType = (MoxaClassBased::BridgeType*)pMoxaNodeManager->getNode(deviceNodeId);
  MoxaClassBased::UnicastStaticForwardFolderType* pUnicastStaticForwardFolderType =
      pBridgeType->getDeviceConfig()->getUnicastStaticForwardTable();

  const ActStaticForwardTable& unicast_static_forward_table =
      project.GetDeviceConfig().GetUnicastStaticForwardTables()[device.GetId()];

  const QSet<ActStaticForwardEntry>& static_forward_entries = unicast_static_forward_table.GetStaticForwardEntries();
  for (ActStaticForwardEntry static_forward_entry : static_forward_entries) {
    UaString browse_name = UaString("%1_%2")
                               .arg(int(static_forward_entry.GetVlanId()))
                               .arg(static_forward_entry.GetMAC().toStdString().c_str());
    UaReferenceDescriptions references;
    pMoxaNodeManager->getNodeReference(pUnicastStaticForwardFolderType->nodeId(), OpcUa_False, references);
    OpcUa_UInt32 idx = 0;
    MoxaClassBased::UnicastStaticForwardType* pUnicastStaticForwardType;
    for (idx = 0; idx < references.length(); idx++) {
      if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_UnicastStaticForwardType) {
        continue;
      }
      UaNodeId unicastStaticForwardNodeId(references[idx].NodeId.NodeId);
      pUnicastStaticForwardType =
          (MoxaClassBased::UnicastStaticForwardType*)pMoxaNodeManager->getNode(unicastStaticForwardNodeId);
      if (pUnicastStaticForwardType->browseName().toString() != browse_name) {
        continue;
      }
      break;
    }

    if (idx == references.length()) {
      // NodeId for the node to create
      UaNodeId unicastStaticForwardNodeId(
          UaString("%1.%2").arg(pUnicastStaticForwardFolderType->nodeId().toString()).arg(browse_name),
          pMoxaNodeManager->getNameSpaceIndex());

      // If succeeded, create the new node to the OPC UA nodemanager
      pUnicastStaticForwardType = new MoxaClassBased::UnicastStaticForwardType(
          unicastStaticForwardNodeId, browse_name, pMoxaNodeManager->getNameSpaceIndex(),
          pMoxaNodeManager->getNodeManagerConfig());
      if (!pUnicastStaticForwardType) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Allocate memory failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      ret =
          pMoxaNodeManager->addNodeAndReference(pUnicastStaticForwardFolderType->nodeId(),
                                                pUnicastStaticForwardType->getUaReferenceLists(), OpcUaId_HasComponent);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Create static forward node failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }

    pUnicastStaticForwardType->setVlanId(OpcUa_UInt16(static_forward_entry.GetVlanId()));
    pUnicastStaticForwardType->setMacAddress(static_forward_entry.GetMAC().toStdString().c_str());
    if (!static_forward_entry.GetEgressPorts().isEmpty()) {
      pUnicastStaticForwardType->setEgressPort(OpcUa_UInt32(static_forward_entry.GetEgressPorts().values()[0]));
    }
  }

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(pUnicastStaticForwardFolderType->nodeId(), OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_UnicastStaticForwardType) {
      continue;
    }
    UaNodeId unicastStaticForwardNodeId(references[idx].NodeId.NodeId);
    MoxaClassBased::UnicastStaticForwardType* pUnicastStaticForwardType =
        (MoxaClassBased::UnicastStaticForwardType*)pMoxaNodeManager->getNode(unicastStaticForwardNodeId);

    OpcUa_UInt16 vlan_id = pUnicastStaticForwardType->getVlanId();
    UaString mac_address = pUnicastStaticForwardType->getMacAddress();
    QSet<ActStaticForwardEntry>::const_iterator static_forward_entry_iter =
        static_forward_entries.find(ActStaticForwardEntry(qint32(vlan_id), QString(mac_address.toUtf8())));
    if (static_forward_entry_iter == static_forward_entries.end()) {
      ret = pMoxaNodeManager->deleteUaNode(pUnicastStaticForwardType, OpcUa_True, OpcUa_True, OpcUa_True);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Remove node %1 failed").arg(unicastStaticForwardNodeId.toFullString());
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }
  }

  return ret;
}

UaStatus addUnicastStaticForwardMethod(const UaNodeId& unicastStaticForwardFolderNodeId,
                                       const MoxaClassBased::UnicastStaticForwardDataType& configuration,
                                       UaNodeId& unicastStaticForwardNodeId, OpcUa_UInt32& errorCode,
                                       UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(unicastStaticForwardFolderNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(unicastStaticForwardFolderNodeId);

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

  ActStaticForwardTable& unicast_static_forward_table =
      project.GetDeviceConfig().GetUnicastStaticForwardTables()[device_id];
  QSet<ActStaticForwardEntry>& static_forward_entries = unicast_static_forward_table.GetStaticForwardEntries();
  if (static_forward_entries.size() == 256) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: The maximum number of unicast static forward entries is up to 256");
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(unicastStaticForwardFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_UnicastStaticForwardType) {
      continue;
    }
    MoxaClassBased::UnicastStaticForwardType* pUnicastStaticForwardType =
        (MoxaClassBased::UnicastStaticForwardType*)pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);

    if (configuration.getVlanId() == pUnicastStaticForwardType->getVlanId() &&
        configuration.getMacAddress() == pUnicastStaticForwardType->getMacAddress()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Static forward entry duplicated");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }
  }

  ActStaticForwardEntry static_forward_entry(qint32(configuration.getVlanId()),
                                             QString(configuration.getMacAddress().toUtf8()));

  QSet<ActStaticForwardEntry>::iterator static_forward_entry_iter = static_forward_entries.find(static_forward_entry);
  if (static_forward_entry_iter != static_forward_entries.end()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Static forward entry duplicated");
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  QSet<qint64> egress_ports;
  if (configuration.isEgressPortSet()) {
    egress_ports.insert(qint64(configuration.getEgressPort()));
  } else {
    egress_ports.insert(1);
  }
  static_forward_entry.SetEgressPorts(egress_ports);

  act_status = checkUnicastStaticForwardNode(project, device, static_forward_entry);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  static_forward_entries.insert(static_forward_entry);

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  pMoxaNodeManager->getNodeReference(unicastStaticForwardFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_UnicastStaticForwardType) {
      continue;
    }
    MoxaClassBased::UnicastStaticForwardType* pUnicastStaticForwardType =
        (MoxaClassBased::UnicastStaticForwardType*)pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);

    if (configuration.getVlanId() == pUnicastStaticForwardType->getVlanId() &&
        configuration.getMacAddress() == pUnicastStaticForwardType->getMacAddress()) {
      unicastStaticForwardNodeId = pUnicastStaticForwardType->nodeId();
      break;
    }
  }

  return ret;
}

UaStatus setUnicastStaticForwardMethod(const UaNodeId& unicastStaticForwardFolderNodeId,
                                       const MoxaClassBased::UnicastStaticForwardDataType& configuration,
                                       OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(unicastStaticForwardFolderNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(unicastStaticForwardFolderNodeId);

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

  ActStaticForwardTable& unicast_static_forward_table =
      project.GetDeviceConfig().GetUnicastStaticForwardTables()[device_id];
  QSet<ActStaticForwardEntry>& static_forward_entries = unicast_static_forward_table.GetStaticForwardEntries();

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(unicastStaticForwardFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_UnicastStaticForwardType) {
      continue;
    }
    UaNodeId unicastStaticForwardNodeId(references[idx].NodeId.NodeId);
    MoxaClassBased::UnicastStaticForwardType* pUnicastStaticForwardType =
        (MoxaClassBased::UnicastStaticForwardType*)pMoxaNodeManager->getNode(unicastStaticForwardNodeId);

    if (configuration.getVlanId() != pUnicastStaticForwardType->getVlanId() ||
        configuration.getMacAddress() != pUnicastStaticForwardType->getMacAddress()) {
      continue;
    }

    ActStaticForwardEntry static_forward_entry(qint32(pUnicastStaticForwardType->getVlanId()),
                                               QString(pUnicastStaticForwardType->getMacAddress().toUtf8()));

    QSet<ActStaticForwardEntry>::iterator static_forward_entry_iter = static_forward_entries.find(static_forward_entry);
    if (static_forward_entry_iter != static_forward_entries.end()) {
      static_forward_entry = *static_forward_entry_iter;
      static_forward_entries.erase(static_forward_entry_iter);
    }

    if (configuration.isEgressPortSet()) {
      QSet<qint64> egress_ports;
      egress_ports.insert(qint64(configuration.getEgressPort()));
      static_forward_entry.SetEgressPorts(egress_ports);
    }

    act_status = checkUnicastStaticForwardNode(project, device, static_forward_entry);
    if (!IsActStatusSuccess(act_status)) {
      errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
      errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    static_forward_entries.insert(static_forward_entry);

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

  errorCode = M_UA_INTERNAL_ERROR;
  errorMessage = UaString("Invalid: Static forward entry not found");
  qDebug() << errorMessage.toUtf8();

  return ret;
}

UaStatus removeUnicastStaticForwardMethod(const UaNodeId& unicastStaticForwardFolderNodeId,
                                          const UaNodeId& unicastStaticForwardNodeId, OpcUa_UInt32& errorCode,
                                          UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(unicastStaticForwardNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(unicastStaticForwardNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  UaReferenceDescriptions references;
  MoxaClassBased::UnicastStaticForwardType* pUnicastStaticForwardType = NULL;
  pMoxaNodeManager->getNodeReference(unicastStaticForwardFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_UnicastStaticForwardType) {
      continue;
    }

    if (UaNodeId(references[idx].NodeId.NodeId) == unicastStaticForwardNodeId) {
      pUnicastStaticForwardType =
          (MoxaClassBased::UnicastStaticForwardType*)pMoxaNodeManager->getNode(unicastStaticForwardNodeId);
      break;
    }
  }

  if (pUnicastStaticForwardType == NULL) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: NodeId %1 is not found").arg(unicastStaticForwardNodeId.toFullString());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActStaticForwardTable& unicast_static_forward_table =
      project.GetDeviceConfig().GetUnicastStaticForwardTables()[device_id];
  QSet<ActStaticForwardEntry>& static_forward_entries = unicast_static_forward_table.GetStaticForwardEntries();

  ActStaticForwardEntry static_forward_entry(qint32(pUnicastStaticForwardType->getVlanId()),
                                             QString(pUnicastStaticForwardType->getMacAddress().toUtf8()));

  QSet<ActStaticForwardEntry>::iterator static_forward_entry_iter = static_forward_entries.find(static_forward_entry);
  if (static_forward_entry_iter == static_forward_entries.end()) {
    return ret;
  }

  static_forward_entries.erase(static_forward_entry_iter);

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