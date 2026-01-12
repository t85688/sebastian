#include "classbased_spanningtree.h"

namespace ClassBased {

static ACT_STATUS checkSpanningTreePortNode(ActRstpPortEntry& rstp_port_entry) {
  ACT_STATUS_INIT();

  if (rstp_port_entry.GetPortPriority() < 0 || rstp_port_entry.GetPortPriority() > 240 ||
      rstp_port_entry.GetPortPriority() % 16 != 0) {
    QString error_msg = QString("Invalid: The valid range of Priority is 0 to 240 with multiples of 16");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (rstp_port_entry.GetPathCost() < 0 || rstp_port_entry.GetPathCost() > 200000000) {
    QString error_msg = QString("Invalid: The valid range of PathCost is 0 to 200000000 seconds");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

static ACT_STATUS checkSpanningTreeNode(ActRstpTable& rstp_table) {
  ACT_STATUS_INIT();

  if (rstp_table.GetPriority() < 0 || rstp_table.GetPriority() > 61440 || rstp_table.GetPriority() % 4096 != 0) {
    QString error_msg = QString("Invalid: The valid range of BridgePriority is from 0 to 61440 with multiples of 4096");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (rstp_table.GetForwardDelay() < 4 || rstp_table.GetForwardDelay() > 30) {
    QString error_msg = QString("Invalid: The valid range of ForwardDelayTime is 4 to 30 seconds");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (rstp_table.GetHelloTime() < 1 || rstp_table.GetHelloTime() > 2) {
    QString error_msg = QString("Invalid: The valid range of HelloTime is 1 to 2 seconds");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (rstp_table.GetMaxAge() < 6 || rstp_table.GetMaxAge() > 40) {
    QString error_msg = QString("Invalid: The valid range of MaxAge is 6 to 40 seconds");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

UaStatus updateSpanningTreeNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                UaString& errorMessage) {
  UaStatus ret;

  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetRSTP()) {
    return ret;
  }

  UaNodeId deviceNodeId = pMoxaNodeManager->getDeviceNodeId(project.GetId(), device.GetId());
  MoxaClassBased::BridgeType* pBridgeType = (MoxaClassBased::BridgeType*)pMoxaNodeManager->getNode(deviceNodeId);
  MoxaClassBased::SpanningTreeType* pSpanningTreeType = pBridgeType->getDeviceConfig()->getSpanningTree();

  const ActRstpTable& rstp_table = project.GetDeviceConfig().GetRstpTables()[device.GetId()];

  pSpanningTreeType->setActive(rstp_table.GetActive());
  pSpanningTreeType->setBridgePriority(rstp_table.GetPriority());
  switch (rstp_table.GetSpanningTreeVersion()) {
    case ActSpanningTreeVersionEnum::kSTP:
      pSpanningTreeType->setCompatibility(
          MoxaClassBased::SpanningTreeCompatibilityEnumType::SpanningTreeCompatibilityEnumType_STP);
      break;
    case ActSpanningTreeVersionEnum::kRSTP:
      pSpanningTreeType->setCompatibility(
          MoxaClassBased::SpanningTreeCompatibilityEnumType::SpanningTreeCompatibilityEnumType_RSTP);
      break;
    case ActSpanningTreeVersionEnum::kNotSupport:
      pSpanningTreeType->setCompatibility(
          MoxaClassBased::SpanningTreeCompatibilityEnumType::SpanningTreeCompatibilityEnumType_NotSupport);
      break;
  }
  pSpanningTreeType->setForwardDelayTime(rstp_table.GetForwardDelay());
  pSpanningTreeType->setHelloTime(rstp_table.GetHelloTime());
  pSpanningTreeType->setMaxAge(rstp_table.GetMaxAge());

  const QSet<ActRstpPortEntry>& rstp_port_entries = rstp_table.GetRstpPortEntries();
  for (ActInterface intf : device.GetInterfaces()) {
    UaNodeId interfaceNodeId =
        pMoxaNodeManager->getInterfaceNodeId(project.GetId(), device.GetId(), intf.GetInterfaceId());

    MoxaClassBased::EthernetInterfaceType* pEthernetInterfaceType =
        (MoxaClassBased::EthernetInterfaceType*)pMoxaNodeManager->getNode(interfaceNodeId);
    MoxaClassBased::SpanningTreePortType* pSpanningTreePortType = pEthernetInterfaceType->getSpanningTreePort();

    ActRstpPortEntry rstp_port_entry(intf.GetInterfaceId());
    QSet<ActRstpPortEntry>::const_iterator rstp_port_entry_iter = rstp_port_entries.find(rstp_port_entry);
    if (rstp_port_entry_iter != rstp_port_entries.end()) {
      rstp_port_entry = *rstp_port_entry_iter;
    }

    pSpanningTreePortType->setBPDUFilter(rstp_port_entry.GetBpduFilter());
    switch (rstp_port_entry.GetEdge()) {
      case ActRstpEdgeEnum::kAuto:
        pSpanningTreePortType->setEdge(MoxaClassBased::EdgeEnumType::EdgeEnumType_Auto);
        break;
      case ActRstpEdgeEnum::kYes:
        pSpanningTreePortType->setEdge(MoxaClassBased::EdgeEnumType::EdgeEnumType_Yes);
        break;
      case ActRstpEdgeEnum::kNo:
        pSpanningTreePortType->setEdge(MoxaClassBased::EdgeEnumType::EdgeEnumType_No);
        break;
    }
    switch (rstp_port_entry.GetLinkType()) {
      case ActRstpLinkTypeEnum::kAuto:
        pSpanningTreePortType->setLinkType(MoxaClassBased::LinkTypeEnumType::LinkTypeEnumType_Auto);
        break;
      case ActRstpLinkTypeEnum::kPointToPoint:
        pSpanningTreePortType->setLinkType(MoxaClassBased::LinkTypeEnumType::LinkTypeEnumType_PointToPoint);
        break;
      case ActRstpLinkTypeEnum::kShared:
        pSpanningTreePortType->setLinkType(MoxaClassBased::LinkTypeEnumType::LinkTypeEnumType_Shared);
        break;
    }
    pSpanningTreePortType->setPathCost(rstp_port_entry.GetPathCost());
    pSpanningTreePortType->setPriority(rstp_port_entry.GetPortPriority());
  }

  return ret;
}

UaStatus setSpanningTreeMethod(const UaNodeId& nodeId, const MoxaClassBased::SpanningTreeDataType& configuration,
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

  ActDeviceConfig& device_config = project.GetDeviceConfig();
  ActRstpTable& rstp_table = device_config.GetRstpTables()[device_id];

  if (configuration.isActiveSet()) {
    rstp_table.SetActive(configuration.getActive());
  }
  if (configuration.isCompatibilitySet()) {
    switch (configuration.getCompatibility()) {
      case MoxaClassBased::SpanningTreeCompatibilityEnumType::SpanningTreeCompatibilityEnumType_STP:
        rstp_table.SetSpanningTreeVersion(ActSpanningTreeVersionEnum::kSTP);
        break;
      case MoxaClassBased::SpanningTreeCompatibilityEnumType::SpanningTreeCompatibilityEnumType_RSTP:
        rstp_table.SetSpanningTreeVersion(ActSpanningTreeVersionEnum::kRSTP);
        break;
      case MoxaClassBased::SpanningTreeCompatibilityEnumType::SpanningTreeCompatibilityEnumType_NotSupport:
        rstp_table.SetSpanningTreeVersion(ActSpanningTreeVersionEnum::kNotSupport);
        break;
    }
  }
  if (configuration.isBridgePrioritySet()) {
    rstp_table.SetPriority(configuration.getBridgePriority());
  }
  if (configuration.isForwardDelayTimeSet()) {
    rstp_table.SetForwardDelay(configuration.getForwardDelayTime());
  }
  if (configuration.isHelloTimeSet()) {
    rstp_table.SetHelloTime(configuration.getHelloTime());
  }
  if (configuration.isMaxAgeSet()) {
    rstp_table.SetMaxAge(configuration.getMaxAge());
  }

  act_status = checkSpanningTreeNode(rstp_table);
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

UaStatus setSpanningTreePortMethod(const UaNodeId& nodeId,
                                   const MoxaClassBased::SpanningTreePortDataType& configuration,
                                   OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(nodeId);
  qint64 interface_id = pMoxaNodeManager->getInterfaceId(nodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActRstpTable& rstp_table = project.GetDeviceConfig().GetRstpTables()[device_id];
  QSet<ActRstpPortEntry>& rstp_port_entries = rstp_table.GetRstpPortEntries();
  ActRstpPortEntry rstp_port_entry(interface_id);
  QSet<ActRstpPortEntry>::iterator rstp_port_entry_iter = rstp_port_entries.find(rstp_port_entry);
  if (rstp_port_entry_iter != rstp_port_entries.end()) {
    rstp_port_entry = *rstp_port_entry_iter;
    rstp_port_entries.erase(rstp_port_entry_iter);
  }

  if (configuration.isBPDUFilterSet()) {
    rstp_port_entry.SetBpduFilter(configuration.getBPDUFilter());
  }
  if (configuration.isEdgeSet()) {
    switch (configuration.getEdge()) {
      case MoxaClassBased::EdgeEnumType::EdgeEnumType_Auto:
        rstp_port_entry.SetEdge(ActRstpEdgeEnum::kAuto);
        break;
      case MoxaClassBased::EdgeEnumType::EdgeEnumType_Yes:
        rstp_port_entry.SetEdge(ActRstpEdgeEnum::kYes);
        break;
      case MoxaClassBased::EdgeEnumType::EdgeEnumType_No:
        rstp_port_entry.SetEdge(ActRstpEdgeEnum::kNo);
        break;
    }
  }
  if (configuration.isLinkTypeSet()) {
    switch (configuration.getLinkType()) {
      case MoxaClassBased::LinkTypeEnumType::LinkTypeEnumType_Auto:
        rstp_port_entry.SetLinkType(ActRstpLinkTypeEnum::kAuto);
        break;
      case MoxaClassBased::LinkTypeEnumType::LinkTypeEnumType_PointToPoint:
        rstp_port_entry.SetLinkType(ActRstpLinkTypeEnum::kPointToPoint);
        break;
      case MoxaClassBased::LinkTypeEnumType::LinkTypeEnumType_Shared:
        rstp_port_entry.SetLinkType(ActRstpLinkTypeEnum::kShared);
        break;
    }
  }
  if (configuration.isPathCostSet()) {
    rstp_port_entry.SetPathCost(configuration.getPathCost());
  }
  if (configuration.isPrioritySet()) {
    rstp_port_entry.SetPortPriority(configuration.getPriority());
  }

  rstp_port_entries.insert(rstp_port_entry);

  act_status = checkSpanningTreePortNode(rstp_port_entry);
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

}  // namespace ClassBased