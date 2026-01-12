#include "classbased_vlan.h"

namespace ClassBased {

UaStatus updateVlanNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                        UaString& errorMessage) {
  UaStatus ret;

  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetAccessTrunkMode()) {
    return ret;
  }

  UaNodeId deviceNodeId = pMoxaNodeManager->getDeviceNodeId(project.GetId(), device.GetId());
  MoxaClassBased::BridgeType* pBridgeType = (MoxaClassBased::BridgeType*)pMoxaNodeManager->getNode(deviceNodeId);
  MoxaClassBased::VlanSettingType* pVlanSettingType = pBridgeType->getDeviceConfig()->getVlanSetting();

  const ActVlanTable& vlan_table = project.GetDeviceConfig().GetVlanTables()[device.GetId()];
  const ActDefaultPriorityTable& port_priority_table =
      project.GetDeviceConfig().GetPortDefaultPCPTables()[device.GetId()];

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetManagementVLAN()) {
    pVlanSettingType->setManagementVlan(OpcUa_UInt16(vlan_table.GetManagementVlan()));
  }

  QMap<qint64, QSet<qint32>> untagged_vlan_map;  // {port: [vlan_id]}
  QMap<qint64, QSet<qint32>> tagged_vlan_map;    // {port: [vlan_id]}
  const QSet<ActVlanStaticEntry>& vlan_static_entries = vlan_table.GetVlanStaticEntries();
  for (ActVlanStaticEntry vlan_static_entry : vlan_static_entries) {
    UaNodeId vlanNodeId(UaString("%1.%2")
                            .arg(pVlanSettingType->getVlanTable()->nodeId().toString())
                            .arg(int(vlan_static_entry.GetVlanId())),
                        pMoxaNodeManager->getNameSpaceIndex());
    UaString browseName(UaString("%1_%2")
                            .arg(int(vlan_static_entry.GetVlanId()))
                            .arg(vlan_static_entry.GetName().toStdString().c_str()));

    MoxaClassBased::VlanType* pVlanType = (MoxaClassBased::VlanType*)pMoxaNodeManager->getNode(vlanNodeId);
    if (pVlanType == NULL) {
      // NodeId for the node to create

      // If succeeded, create the new node to the OPC UA nodemanager
      pVlanType = new MoxaClassBased::VlanType(vlanNodeId, browseName, pMoxaNodeManager->getNameSpaceIndex(),
                                               pMoxaNodeManager->getNodeManagerConfig());
      if (!pVlanType) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Allocate memory failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      ret = pMoxaNodeManager->addNodeAndReference(pVlanSettingType->getVlanTable()->nodeId(),
                                                  pVlanType->getUaReferenceLists(), OpcUaId_HasComponent);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Create VLAN node failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    } else {
      pVlanType->setBrowseName(UaQualifiedName(browseName, pMoxaNodeManager->getNameSpaceIndex()));
      pVlanType->setDisplayName(UaLocalizedText(UaString(), browseName));
    }

    qint32& vlan_id = vlan_static_entry.GetVlanId();
    QSet<qint64>& untagged_ports = vlan_static_entry.GetUntaggedPorts();
    pVlanType->setVlanId(OpcUa_UInt16(vlan_id));
    pVlanType->setVlanName(vlan_static_entry.GetName().toStdString().c_str());
    pVlanType->setTeMstid(vlan_static_entry.GetTeMstid());
    UaUInt16Array egress_ports;
    egress_ports.create(OpcUa_UInt32(vlan_static_entry.GetEgressPorts().size()));
    OpcUa_UInt32 idx = 0;
    for (qint64 egress_port : vlan_static_entry.GetEgressPorts()) {
      egress_ports[idx++] = OpcUa_UInt16(egress_port);

      if (!untagged_ports.contains(egress_port)) {
        tagged_vlan_map[egress_port].insert(vlan_id);
      }
    }
    pVlanType->setMemberPort(egress_ports);

    for (qint64 untagged_port : untagged_ports) {
      untagged_vlan_map[untagged_port].insert(vlan_id);
    }
  }

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(pVlanSettingType->getVlanTable()->nodeId(), OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_VlanType) {
      continue;
    }
    UaNodeId vlanNodeId(references[idx].NodeId.NodeId);
    MoxaClassBased::VlanType* pVlanType = (MoxaClassBased::VlanType*)pMoxaNodeManager->getNode(vlanNodeId);

    OpcUa_UInt16 vlan_id = pVlanType->getVlanId();
    QSet<ActVlanStaticEntry>::const_iterator vlan_static_entry_iter =
        vlan_static_entries.find(ActVlanStaticEntry(qint32(vlan_id)));
    if (vlan_static_entry_iter == vlan_static_entries.end()) {
      ret = pMoxaNodeManager->deleteUaNode(pVlanType, OpcUa_True, OpcUa_True, OpcUa_True);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Remove node %1 failed").arg(vlanNodeId.toFullString());
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }
  }

  const QSet<ActPortVlanEntry>& port_vlan_entries = vlan_table.GetPortVlanEntries();
  const QSet<ActVlanPortTypeEntry>& vlan_port_type_entries = vlan_table.GetVlanPortTypeEntries();
  const QSet<ActDefaultPriorityEntry>& default_priority_entries = port_priority_table.GetDefaultPriorityEntries();
  for (ActInterface intf : device.GetInterfaces()) {
    UaNodeId interfaceNodeId =
        pMoxaNodeManager->getInterfaceNodeId(project.GetId(), device.GetId(), intf.GetInterfaceId());

    MoxaClassBased::EthernetInterfaceType* pEthernetInterfaceType =
        (MoxaClassBased::EthernetInterfaceType*)pMoxaNodeManager->getNode(interfaceNodeId);
    if (pEthernetInterfaceType == NULL) {
      continue;
    }
    MoxaClassBased::VlanPortType* pVlanPortType = pEthernetInterfaceType->getVlanPort();

    ActPortVlanEntry port_vlan_entry(intf.GetInterfaceId());
    QSet<ActPortVlanEntry>::const_iterator port_vlan_entry_iter = port_vlan_entries.find(port_vlan_entry);
    if (port_vlan_entry_iter != port_vlan_entries.end()) {
      port_vlan_entry = *port_vlan_entry_iter;
    }

    ActVlanPortTypeEntry vlan_port_type_entry(intf.GetInterfaceId());
    QSet<ActVlanPortTypeEntry>::const_iterator vlan_port_type_entry_iter =
        vlan_port_type_entries.find(vlan_port_type_entry);
    if (vlan_port_type_entry_iter != vlan_port_type_entries.end()) {
      vlan_port_type_entry = *vlan_port_type_entry_iter;
    }

    ActDefaultPriorityEntry default_priority_entry(intf.GetInterfaceId());
    QSet<ActDefaultPriorityEntry>::const_iterator default_priority_entry_iter =
        default_priority_entries.find(default_priority_entry);
    if (default_priority_entry_iter != default_priority_entries.end()) {
      default_priority_entry = *default_priority_entry_iter;
    }

    pVlanPortType->setPvid(OpcUa_UInt16(port_vlan_entry.GetPVID()));
    pVlanPortType->setPriorityCodePoint(OpcUa_UInt16(default_priority_entry.GetDefaultPCP()));

    UaUInt16Array tagged_vlan;
    tagged_vlan.create(OpcUa_UInt16(tagged_vlan_map[intf.GetInterfaceId()].size()));
    OpcUa_UInt32 idx = 0;
    for (qint32 vlan : tagged_vlan_map[intf.GetInterfaceId()]) {
      tagged_vlan[idx++] = OpcUa_UInt16(vlan);
    }
    pVlanPortType->setTaggedVlan(tagged_vlan);

    UaUInt16Array untagged_vlan;
    untagged_vlan.create(OpcUa_UInt16(untagged_vlan_map[intf.GetInterfaceId()].size()));
    idx = 0;
    for (qint32 vlan : untagged_vlan_map[intf.GetInterfaceId()]) {
      untagged_vlan[idx++] = OpcUa_UInt16(vlan);
    }
    pVlanPortType->setUntaggedVlan(untagged_vlan);

    switch (vlan_port_type_entry.GetVlanPortType()) {
      case ActVlanPortTypeEnum::kAccess:
        pVlanPortType->setMode(MoxaClassBased::PortModeEnumType::PortModeEnumType_Access);
        break;
      case ActVlanPortTypeEnum::kTrunk:
        pVlanPortType->setMode(MoxaClassBased::PortModeEnumType::PortModeEnumType_Trunk);
        break;
      case ActVlanPortTypeEnum::kHybrid:
        pVlanPortType->setMode(MoxaClassBased::PortModeEnumType::PortModeEnumType_Hybrid);
        break;
    }
  }

  return ret;
}

UaStatus addVlanMethod(const UaNodeId& vlanTableFolderNodeId, const MoxaClassBased::VlanDataType& configuration,
                       UaNodeId& vlanNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(vlanTableFolderNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(vlanTableFolderNodeId);

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

  ActVlanTable& vlan_table = project.GetDeviceConfig().GetVlanTables()[device_id];
  QSet<ActVlanStaticEntry>& vlan_static_entries = vlan_table.GetVlanStaticEntries();
  QSet<ActVlanPortTypeEntry>& vlan_port_type_entries = vlan_table.GetVlanPortTypeEntries();
  QSet<ActPortVlanEntry>& port_vlan_entries = vlan_table.GetPortVlanEntries();

  quint32 vlan_id = quint32(configuration.getVlanId());
  ActVlanStaticEntry vlan_static_entry(vlan_id);
  QSet<ActVlanStaticEntry>::iterator vlan_static_entry_iter = vlan_static_entries.find(vlan_static_entry);
  if (vlan_static_entry_iter != vlan_static_entries.end()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: VLAN %1 is exist").arg(int(vlan_id));
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  if (configuration.isVlanNameSet()) {
    QString vlan_name(configuration.getVlanName().toUtf8());

    if (vlan_name.contains(' ')) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: The VLAN name cannot contain space character");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    vlan_static_entry.SetName(vlan_name);
  }

  if (configuration.isTeMstidSet()) {
    if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetTEMSTID()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: The device %1 doesn't support Te-Mstid configuration")
                         .arg(device.GetIpv4().GetIpAddress().toStdString().c_str());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }
    vlan_static_entry.SetTeMstid(configuration.getTeMstid());
  }

  if (configuration.isMemberPortSet()) {
    UaUInt16Array member_port;
    configuration.getMemberPort(member_port);

    QSet<qint64> egress_ports;
    QSet<qint64> untagged_ports;
    for (OpcUa_UInt32 index = 0; index < member_port.length(); index++) {
      qint64 port_id = qint64(member_port[index]);

      if (port_id == 0 || port_id > device.GetInterfaces().size()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: The VLAN %1 cannot have member port %2. The device doesn't have port %2")
                           .arg(int(vlan_id))
                           .arg(int(port_id));
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      ActVlanPortTypeEntry vlan_port_type_entry(port_id);
      QSet<ActVlanPortTypeEntry>::iterator vlan_port_type_entry_iter =
          vlan_port_type_entries.find(vlan_port_type_entry);
      if (vlan_port_type_entry_iter != vlan_port_type_entries.end()) {
        vlan_port_type_entry = *vlan_port_type_entry_iter;
      } else {
        vlan_port_type_entries.insert(vlan_port_type_entry);
      }

      if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kAccess) {
        untagged_ports.insert(port_id);
      }

      egress_ports.insert(port_id);
    }

    vlan_static_entry.SetEgressPorts(egress_ports);
    vlan_static_entry.SetUntaggedPorts(untagged_ports);

    for (ActInterface intf : device.GetInterfaces()) {
      qint64 port_id = intf.GetInterfaceId();

      ActVlanPortTypeEntry vlan_port_type_entry(port_id);
      QSet<ActVlanPortTypeEntry>::iterator vlan_port_type_entry_iter =
          vlan_port_type_entries.find(vlan_port_type_entry);
      if (vlan_port_type_entry_iter != vlan_port_type_entries.end()) {
        vlan_port_type_entry = *vlan_port_type_entry_iter;
      } else {
        vlan_port_type_entries.insert(vlan_port_type_entry);
      }

      ActPortVlanEntry port_vlan_entry(port_id);
      QSet<ActPortVlanEntry>::iterator port_vlan_entry_iter = port_vlan_entries.find(port_vlan_entry);
      if (port_vlan_entry_iter != port_vlan_entries.end()) {
        port_vlan_entry = *port_vlan_entry_iter;
      } else {
        port_vlan_entries.insert(port_vlan_entry);
      }

      if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kAccess) {
        if (port_vlan_entry.GetPVID() != vlan_id && egress_ports.contains(port_id)) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage =
              UaString(
                  "Invalid: The VLAN %1 cannot have member port %2, because the port %2 is Access mode and PVID is %3")
                  .arg(int(vlan_id))
                  .arg(int(port_id))
                  .arg(port_vlan_entry.GetPVID());
          qDebug() << errorMessage.toUtf8();
          return ret;
        }
      }

      if (port_vlan_entry.GetPVID() == vlan_id && !egress_ports.contains(port_id)) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: The VLAN %1 must have member port %2 to satisfy PVID setting")
                           .arg(int(vlan_id))
                           .arg(int(port_id));
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }
  }

  vlan_static_entries.insert(vlan_static_entry);

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

UaStatus setVlanMethod(const UaNodeId& vlanTableFolderNodeId, const MoxaClassBased::VlanDataType& configuration,
                       OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(vlanTableFolderNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(vlanTableFolderNodeId);

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

  ActVlanTable& vlan_table = project.GetDeviceConfig().GetVlanTables()[device_id];
  QSet<ActVlanStaticEntry>& vlan_static_entries = vlan_table.GetVlanStaticEntries();
  QSet<ActVlanPortTypeEntry>& vlan_port_type_entries = vlan_table.GetVlanPortTypeEntries();
  QSet<ActPortVlanEntry>& port_vlan_entries = vlan_table.GetPortVlanEntries();

  quint32 vlan_id = quint32(configuration.getVlanId());
  ActVlanStaticEntry vlan_static_entry(vlan_id);
  QSet<ActVlanStaticEntry>::iterator vlan_static_entry_iter = vlan_static_entries.find(vlan_static_entry);
  if (vlan_static_entry_iter == vlan_static_entries.end()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: VLAN %1 is not found").arg(int(vlan_id));
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  vlan_static_entry = *vlan_static_entry_iter;
  vlan_static_entries.erase(vlan_static_entry_iter);

  if (configuration.isVlanNameSet()) {
    QString vlan_name(configuration.getVlanName().toUtf8());

    if (vlan_name.contains(' ')) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: The VLAN name cannot contain space character");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    vlan_static_entry.SetName(vlan_name);
  }

  if (configuration.isTeMstidSet()) {
    if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetTEMSTID()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: The device %1 doesn't support Te-Mstid configuration")
                         .arg(device.GetIpv4().GetIpAddress().toStdString().c_str());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }
    vlan_static_entry.SetTeMstid(configuration.getTeMstid());
  }

  if (configuration.isMemberPortSet()) {
    UaUInt16Array member_port;
    configuration.getMemberPort(member_port);

    QSet<qint64> egress_ports;
    QSet<qint64> untagged_ports;
    for (OpcUa_UInt32 index = 0; index < member_port.length(); index++) {
      qint64 port_id = qint64(member_port[index]);

      if (port_id == 0 || port_id > device.GetInterfaces().size()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: The VLAN %1 cannot have member port %2. The device doesn't have port %2")
                           .arg(int(vlan_id))
                           .arg(int(port_id));
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      ActVlanPortTypeEntry vlan_port_type_entry(port_id);
      QSet<ActVlanPortTypeEntry>::iterator vlan_port_type_entry_iter =
          vlan_port_type_entries.find(vlan_port_type_entry);
      if (vlan_port_type_entry_iter != vlan_port_type_entries.end()) {
        vlan_port_type_entry = *vlan_port_type_entry_iter;
      } else {
        vlan_port_type_entries.insert(vlan_port_type_entry);
      }

      if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kAccess) {
        untagged_ports.insert(port_id);
      }

      egress_ports.insert(port_id);
    }

    vlan_static_entry.SetEgressPorts(egress_ports);
    vlan_static_entry.SetUntaggedPorts(untagged_ports);

    for (ActInterface intf : device.GetInterfaces()) {
      qint64 port_id = intf.GetInterfaceId();

      ActVlanPortTypeEntry vlan_port_type_entry(port_id);
      QSet<ActVlanPortTypeEntry>::iterator vlan_port_type_entry_iter =
          vlan_port_type_entries.find(vlan_port_type_entry);
      if (vlan_port_type_entry_iter != vlan_port_type_entries.end()) {
        vlan_port_type_entry = *vlan_port_type_entry_iter;
      } else {
        vlan_port_type_entries.insert(vlan_port_type_entry);
      }

      ActPortVlanEntry port_vlan_entry(port_id);
      QSet<ActPortVlanEntry>::iterator port_vlan_entry_iter = port_vlan_entries.find(port_vlan_entry);
      if (port_vlan_entry_iter != port_vlan_entries.end()) {
        port_vlan_entry = *port_vlan_entry_iter;
      } else {
        port_vlan_entries.insert(port_vlan_entry);
      }

      if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kAccess) {
        if (port_vlan_entry.GetPVID() != vlan_id && egress_ports.contains(port_id)) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage =
              UaString(
                  "Invalid: The VLAN %1 cannot have member port %2, because the port %2 is Access mode and PVID is %3")
                  .arg(int(vlan_id))
                  .arg(int(port_id))
                  .arg(port_vlan_entry.GetPVID());
          qDebug() << errorMessage.toUtf8();
          return ret;
        }
      }
      if (port_vlan_entry.GetPVID() == vlan_id && !egress_ports.contains(port_id)) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: The VLAN %1 must have member port %2 to satisfy PVID setting")
                           .arg(int(vlan_id))
                           .arg(int(port_id));
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }
  }

  vlan_static_entries.insert(vlan_static_entry);

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

UaStatus removeVlanMethod(const UaNodeId& vlanTableFolderNodeId, const UaNodeId& vlanNodeId, OpcUa_UInt32& errorCode,
                          UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(vlanTableFolderNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(vlanTableFolderNodeId);

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
  MoxaClassBased::VlanType* pVlanType = NULL;
  pMoxaNodeManager->getNodeReference(vlanTableFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_VlanType) {
      continue;
    }

    if (UaNodeId(references[idx].NodeId.NodeId) == vlanNodeId) {
      pVlanType = (MoxaClassBased::VlanType*)pMoxaNodeManager->getNode(vlanNodeId);
      break;
    }
  }

  if (pVlanType == NULL) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: NodeId %1 is not found").arg(vlanNodeId.toFullString());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActVlanTable& vlan_table = project.GetDeviceConfig().GetVlanTables()[device_id];
  QSet<ActVlanStaticEntry>& vlan_static_entries = vlan_table.GetVlanStaticEntries();

  qint32 vlan_id = qint32(pVlanType->getVlanId());
  ActVlanStaticEntry vlan_static_entry(vlan_id);
  QSet<ActVlanStaticEntry>::iterator vlan_static_entry_iter = vlan_static_entries.find(vlan_static_entry);
  if (vlan_static_entry_iter == vlan_static_entries.end()) {
    return ret;
  }

  vlan_static_entry = *vlan_static_entry_iter;

  if (vlan_table.GetManagementVlan() == vlan_id) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: The management VLAN %1 cannot be deleted").arg(int(vlan_id));
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  if (device.GetDeviceProperty().GetReservedVlan().contains(vlan_id)) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: The reserved VLAN %1 cannot be deleted").arg(int(vlan_id));
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  if (!vlan_static_entry.GetEgressPorts().isEmpty()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: The VLAN %1 is used. It cannot be deleted").arg(int(vlan_id));
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  vlan_static_entries.remove(vlan_static_entry);

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

UaStatus setVlanPortMethod(const UaNodeId& vlanPortNodeId, const MoxaClassBased::VlanPortDataType& configuration,
                           OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;

  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(vlanPortNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(vlanPortNodeId);
  qint64 interface_id = pMoxaNodeManager->getInterfaceId(vlanPortNodeId);

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

  ActVlanTable& vlan_table = project.GetDeviceConfig().GetVlanTables()[device_id];
  QSet<ActVlanStaticEntry>& vlan_static_entries = vlan_table.GetVlanStaticEntries();
  QSet<ActPortVlanEntry>& port_vlan_entries = vlan_table.GetPortVlanEntries();
  QSet<ActVlanPortTypeEntry>& vlan_port_type_entries = vlan_table.GetVlanPortTypeEntries();

  ActVlanPortTypeEntry vlan_port_type_entry(interface_id);
  QSet<ActVlanPortTypeEntry>::iterator vlan_port_type_entry_iter = vlan_port_type_entries.find(vlan_port_type_entry);
  if (vlan_port_type_entry_iter != vlan_port_type_entries.end()) {
    vlan_port_type_entry = *vlan_port_type_entry_iter;
    vlan_port_type_entries.erase(vlan_port_type_entry_iter);
  }

  ActPortVlanEntry port_vlan_entry(interface_id);
  QSet<ActPortVlanEntry>::iterator port_vlan_entry_iter = port_vlan_entries.find(port_vlan_entry);
  if (port_vlan_entry_iter != port_vlan_entries.end()) {
    port_vlan_entry = *port_vlan_entry_iter;
    port_vlan_entries.erase(port_vlan_entry_iter);
  }

  ActDefaultPriorityTable& port_priority_table = project.GetDeviceConfig().GetPortDefaultPCPTables()[device_id];
  QSet<ActDefaultPriorityEntry>& default_priority_entries = port_priority_table.GetDefaultPriorityEntries();
  ActDefaultPriorityEntry default_priority_entry(interface_id);
  QSet<ActDefaultPriorityEntry>::iterator default_priority_entry_iter =
      default_priority_entries.find(default_priority_entry);
  if (default_priority_entry_iter != default_priority_entries.end()) {
    default_priority_entry = *default_priority_entry_iter;
    default_priority_entries.erase(default_priority_entry_iter);
  }

  if (configuration.isModeSet()) {
    switch (configuration.getMode()) {
      case MoxaClassBased::PortModeEnumType::PortModeEnumType_Access: {
        if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetAccessTrunkMode()) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: The device %1 doesn't support Access mode")
                             .arg(device.GetIpv4().GetIpAddress().toStdString().c_str());
          qDebug() << errorMessage.toUtf8();
          return ret;
        }
        vlan_port_type_entry.SetVlanPortType(ActVlanPortTypeEnum::kAccess);
      } break;
      case MoxaClassBased::PortModeEnumType::PortModeEnumType_Trunk: {
        if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetAccessTrunkMode()) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: The device %1 doesn't support Trunk mode")
                             .arg(device.GetIpv4().GetIpAddress().toStdString().c_str());
          qDebug() << errorMessage.toUtf8();
          return ret;
        }
        vlan_port_type_entry.SetVlanPortType(ActVlanPortTypeEnum::kTrunk);
      } break;
      case MoxaClassBased::PortModeEnumType::PortModeEnumType_Hybrid: {
        if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: The device %1 doesn't support Hybrid mode")
                             .arg(device.GetIpv4().GetIpAddress().toStdString().c_str());
          qDebug() << errorMessage.toUtf8();
          return ret;
        }
        vlan_port_type_entry.SetVlanPortType(ActVlanPortTypeEnum::kHybrid);
      } break;
    }
  }
  vlan_port_type_entries.insert(vlan_port_type_entry);

  if (configuration.isPvidSet()) {
    if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetDefaultPVID()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: The device %1 doesn't support to set PVID configuration")
                         .arg(device.GetIpv4().GetIpAddress().toStdString().c_str());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    quint16 pvid = quint16(configuration.getPvid());

    QSet<ActVlanStaticEntry>::const_iterator vlan_static_entry_iter =
        vlan_static_entries.find(ActVlanStaticEntry(pvid));
    if (vlan_static_entry_iter == vlan_static_entries.end()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: The VLAN %1 is not exist in VLAN table").arg(pvid);
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    port_vlan_entry.SetPVID(pvid);
  }
  port_vlan_entries.insert(port_vlan_entry);

  if (configuration.isPriorityCodePointSet()) {
    if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetDefaultPCP()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: The device %1 doesn't support to set priority code point")
                         .arg(device.GetIpv4().GetIpAddress().toStdString().c_str());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    default_priority_entry.SetDefaultPCP(quint8(configuration.getPriorityCodePoint()));
    default_priority_entries.insert(default_priority_entry);
  }

  QSet<qint32> untagged_vlan_set;
  if (configuration.isUntaggedVlanSet()) {
    UaUInt16Array untagged_vlan;
    configuration.getUntaggedVlan(untagged_vlan);

    for (OpcUa_UInt32 index = 0; index < untagged_vlan.length(); index++) {
      qint32 vlan_id = qint32(untagged_vlan[index]);
      untagged_vlan_set.insert(vlan_id);

      QSet<ActVlanStaticEntry>::iterator vlan_static_entry_iter = vlan_static_entries.find(ActVlanStaticEntry(vlan_id));
      if (vlan_static_entry_iter == vlan_static_entries.end()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: The untagged VLAN %1 is not exist in VLAN table").arg(vlan_id);
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kAccess &&
          vlan_id != port_vlan_entry.GetPVID()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage =
            UaString("Invalid: The Access mode cannot have untagged VLAN %1, it needs to be the same as PVID %2")
                .arg(vlan_id)
                .arg(port_vlan_entry.GetPVID());
        qDebug() << errorMessage.toUtf8();
        return ret;
      } else if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kTrunk) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: The Trunk mode cannot have untagged VLAN");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }

    if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kAccess && untagged_vlan_set.isEmpty()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage =
          UaString("Invalid: The Access mode need to have untagged VLAN %1 as PVID").arg(port_vlan_entry.GetPVID());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    QSet<ActVlanStaticEntry> new_vlan_static_entries;
    for (ActVlanStaticEntry vlan_static_entry : vlan_static_entries) {
      if (untagged_vlan_set.contains(vlan_static_entry.GetVlanId())) {
        vlan_static_entry.GetEgressPorts().insert(interface_id);
        vlan_static_entry.GetUntaggedPorts().insert(interface_id);
      } else {
        vlan_static_entry.GetEgressPorts().remove(interface_id);
        vlan_static_entry.GetUntaggedPorts().remove(interface_id);
      }
      new_vlan_static_entries.insert(vlan_static_entry);
    }
    vlan_static_entries = new_vlan_static_entries;
  }

  QSet<qint32> tagged_vlan_set;
  if (configuration.isTaggedVlanSet()) {
    UaUInt16Array tagged_vlan;
    configuration.getTaggedVlan(tagged_vlan);

    for (OpcUa_UInt32 index = 0; index < tagged_vlan.length(); index++) {
      qint32 vlan_id = qint32(tagged_vlan[index]);
      tagged_vlan_set.insert(vlan_id);

      QSet<ActVlanStaticEntry>::iterator vlan_static_entry_iter = vlan_static_entries.find(ActVlanStaticEntry(vlan_id));
      if (vlan_static_entry_iter == vlan_static_entries.end()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: The tagged VLAN %1 is not exist in VLAN table").arg(vlan_id);
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }

    if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kAccess) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: The Access mode cannot have tagged VLAN");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kTrunk &&
        (!tagged_vlan_set.contains(port_vlan_entry.GetPVID()) || tagged_vlan_set.isEmpty())) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: The Trunk mode need to set tagged VLAN %1 to satisfy the PVID setting")
                         .arg(port_vlan_entry.GetPVID());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    QSet<ActVlanStaticEntry> new_vlan_static_entries;
    for (ActVlanStaticEntry vlan_static_entry : vlan_static_entries) {
      if (tagged_vlan_set.contains(vlan_static_entry.GetVlanId())) {
        vlan_static_entry.GetEgressPorts().insert(interface_id);
        vlan_static_entry.GetUntaggedPorts().remove(interface_id);
      } else {
        vlan_static_entry.GetEgressPorts().remove(interface_id);
        vlan_static_entry.GetUntaggedPorts().remove(interface_id);
      }
      new_vlan_static_entries.insert(vlan_static_entry);
    }
    vlan_static_entries = new_vlan_static_entries;
  }

  if (untagged_vlan_set.intersects(tagged_vlan_set)) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: The Tagged VLAN and Untagged VLAN cannot have the same VLAN ID");
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActVlanStaticEntry vlan_static_entry(port_vlan_entry.GetPVID());
  QSet<ActVlanStaticEntry>::const_iterator vlan_static_entry_iter = vlan_static_entries.find(vlan_static_entry);
  if (vlan_static_entry_iter != vlan_static_entries.end()) {
    vlan_static_entry = *vlan_static_entry_iter;
  }

  if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kHybrid &&
      !vlan_static_entry.GetEgressPorts().contains(port_vlan_entry.GetPVID())) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage =
        UaString(
            "Invalid: The Hybrid mode requires setting tagged VLAN %1 or untagged VLAN %1 to satisfy the PVID setting")
            .arg(int(port_vlan_entry.GetPVID()));
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

UaStatus setVlanSettingMethod(const UaNodeId& nodeId, const MoxaClassBased::VlanSettingDataType& configuration,
                              OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(nodeId);

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

  ActVlanTable& vlan_table = project.GetDeviceConfig().GetVlanTables()[device_id];
  if (configuration.isManagementVlanSet()) {
    if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetManagementVLAN()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: The device %1 doesn't support mangement VLAN configuration")
                         .arg(device.GetIpv4().GetIpAddress().toStdString().c_str());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }
    vlan_table.SetManagementVlan(qint32(configuration.getManagementVlan()));
  }

  QSet<ActVlanStaticEntry>& vlan_static_entries = vlan_table.GetVlanStaticEntries();
  if (!vlan_static_entries.contains(ActVlanStaticEntry(vlan_table.GetManagementVlan()))) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage =
        UaString("Invalid: The VLAN table cannot find management VLAN %1").arg(int(vlan_table.GetManagementVlan()));
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

}  // namespace ClassBased