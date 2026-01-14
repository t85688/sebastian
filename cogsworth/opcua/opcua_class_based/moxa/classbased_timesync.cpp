#include "classbased_timesync.h"

namespace ClassBased {
static ACT_STATUS checkTimeSyncPortNode(ActTimeSync802Dot1ASPortEntry& port_entry) {
  ACT_STATUS_INIT();

  if (port_entry.GetAnnounceInterval() < 0 || port_entry.GetAnnounceInterval() > 4) {
    QString error_msg = QString("Invalid: The valid range of AnnounceInterval is 0 to 4");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (port_entry.GetAnnounceReceiptTimeout() < 2 || port_entry.GetAnnounceReceiptTimeout() > 10) {
    QString error_msg = QString("Invalid: The valid range of AnnounceReceiptTimeout is 2 to 10");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (port_entry.GetSyncInterval() < -3 || port_entry.GetSyncInterval() > 5) {
    QString error_msg = QString("Invalid: The valid range of SyncInterval is -3 to 5");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (port_entry.GetSyncReceiptTimeout() < 2 || port_entry.GetSyncReceiptTimeout() > 10) {
    QString error_msg = QString("Invalid: The valid range of SyncReceiptTimeout is 2 to 10");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (port_entry.GetPdelayReqInterval() < -3 || port_entry.GetPdelayReqInterval() > 5) {
    QString error_msg = QString("Invalid: The valid range of PdelayRequestInterval is -3 to 5");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (port_entry.GetNeighborPropDelayThresh() < 1 || port_entry.GetNeighborPropDelayThresh() > 10000) {
    QString error_msg = QString("Invalid: The valid range of NeighborPropagationDelayThreshold is 1 to 10000");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

static ACT_STATUS checkTimeSyncNode(ActTimeSyncTable& time_sync_table) {
  ACT_STATUS_INIT();

  switch (time_sync_table.GetProfile()) {
    case ActTimeSyncProfileEnum::kIEEE_802Dot1AS_2011: {
      ActTimeSync802Dot1ASConfig& ieee_802dot1as_2011 = time_sync_table.GetIEEE_802Dot1AS_2011();

      if (ieee_802dot1as_2011.GetPriority1() < 0 || ieee_802dot1as_2011.GetPriority1() > 255) {
        QString error_msg = QString("Invalid: The valid range of Priority1 is 0 to 255");
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }

      if (ieee_802dot1as_2011.GetPriority2() < 0 || ieee_802dot1as_2011.GetPriority2() > 255) {
        QString error_msg = QString("Invalid: The valid range of Priority2 is 0 to 255");
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }

      if (ieee_802dot1as_2011.GetAccuracyAlert() < 50 || ieee_802dot1as_2011.GetAccuracyAlert() > 250000000) {
        QString error_msg = QString("Invalid: The valid range of AccuracyAlert is 50 to 250000000");
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    } break;
    default:
      break;
  }

  return act_status;
}

UaStatus updateTimeSyncNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                            UaString& errorMessage) {
  UaStatus ret;

  UaNodeId deviceNodeId = pMoxaNodeManager->getDeviceNodeId(project.GetId(), device.GetId());

  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSyncSetting().GetIEEE802Dot1AS_2011()) {
    return ret;
  }

  MoxaClassBased::BridgeType* pBridgeType = (MoxaClassBased::BridgeType*)pMoxaNodeManager->getNode(deviceNodeId);
  MoxaClassBased::TimeSyncType* pTimeSyncType = pBridgeType->getDeviceConfig()->getTimeSync();

  const ActTimeSyncTable& time_sync_table = project.GetDeviceConfig().GetTimeSyncTables()[device.GetId()];

  pTimeSyncType->setActive(time_sync_table.GetEnabled());
  switch (time_sync_table.GetProfile()) {
    case ActTimeSyncProfileEnum::kIEEE_802Dot1AS_2011:
      pTimeSyncType->setProfile(MoxaClassBased::ProfileEnumType::ProfileEnumType_IEEE_802Dot1AS_2011);
      break;
    default:
      pTimeSyncType->setProfile(MoxaClassBased::ProfileEnumType::ProfileEnumType_NotSupport);
      break;
  }

  const ActTimeSync802Dot1ASConfig& ieee_802dot1as_2011 = time_sync_table.GetIEEE_802Dot1AS_2011();
  pTimeSyncType->getIeeeDot1AS2011()->setAccurateAlert(ieee_802dot1as_2011.GetAccuracyAlert());
  pTimeSyncType->getIeeeDot1AS2011()->setPriority1(ieee_802dot1as_2011.GetPriority1());
  pTimeSyncType->getIeeeDot1AS2011()->setPriority2(ieee_802dot1as_2011.GetPriority2());

  QMap<qint32, ActTimeSync802Dot1ASPortEntry> port_entry_map;
  for (ActTimeSync802Dot1ASPortEntry port_entry : ieee_802dot1as_2011.GetPortEntries()) {
    port_entry_map.insert(port_entry.GetPortId(), port_entry);
  }

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(deviceNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_EthernetInterfaceType) {
      continue;
    }
    UaNodeId ethernetInterfaceNodeId(references[idx].NodeId.NodeId);
    MoxaClassBased::EthernetInterfaceType* pEthernetInterfaceType =
        (MoxaClassBased::EthernetInterfaceType*)pMoxaNodeManager->getNode(ethernetInterfaceNodeId);
    MoxaClassBased::IeeeDot1AS2011PortType* pIeeeDot1AS2011PortType = pEthernetInterfaceType->getTimeSyncPort();

    ActTimeSync802Dot1ASPortEntry& port_entry = port_entry_map[qint32(pEthernetInterfaceType->getInterfaceId())];
    pIeeeDot1AS2011PortType->setActive(port_entry.GetEnable());
    pIeeeDot1AS2011PortType->setAnnounceInterval(port_entry.GetAnnounceInterval());
    pIeeeDot1AS2011PortType->setAnnounceReceiptTimeout(port_entry.GetAnnounceReceiptTimeout());
    pIeeeDot1AS2011PortType->setNeighborPropagationDelayThreshold(port_entry.GetNeighborPropDelayThresh());
    pIeeeDot1AS2011PortType->setPdelayRequestInterval(port_entry.GetPdelayReqInterval());
    pIeeeDot1AS2011PortType->setSyncInterval(port_entry.GetSyncInterval());
    pIeeeDot1AS2011PortType->setSyncReceiptTimeout(port_entry.GetSyncReceiptTimeout());
  }

  return ret;
}

UaStatus setTimeSyncMethod(const UaNodeId& nodeId, const MoxaClassBased::TimeSyncDataType& configuration,
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
  ActTimeSyncTable& time_sync_table = device_config.GetTimeSyncTables()[device_id];

  if (configuration.isActiveSet()) {
    time_sync_table.SetEnabled(bool(configuration.getActive()));
  }

  time_sync_table.SetProfile(ActTimeSyncProfileEnum::kIEEE_802Dot1AS_2011);

  ActTimeSync802Dot1ASConfig& ieee_802dot1as_2011 = time_sync_table.GetIEEE_802Dot1AS_2011();
  if (configuration.getIeeeDot1AS2011().isAccuracyAlertSet()) {
    ieee_802dot1as_2011.SetAccuracyAlert(configuration.getIeeeDot1AS2011().getAccuracyAlert());
  }
  if (configuration.getIeeeDot1AS2011().isPriority1Set()) {
    ieee_802dot1as_2011.SetPriority1(configuration.getIeeeDot1AS2011().getPriority1());
  }
  if (configuration.getIeeeDot1AS2011().isPriority2Set()) {
    ieee_802dot1as_2011.SetPriority2(configuration.getIeeeDot1AS2011().getPriority2());
  }

  act_status = checkTimeSyncNode(time_sync_table);
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

UaStatus setIeeeDot1AS2011PortMethod(const UaNodeId& nodeId,
                                     const MoxaClassBased::IeeeDot1AS2011PortDataType& configuration,
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

  ActDeviceConfig& device_config = project.GetDeviceConfig();
  ActTimeSyncTable& time_sync_table = device_config.GetTimeSyncTables()[device_id];
  ActTimeSync802Dot1ASConfig& ieee_802dot1as_2011 = time_sync_table.GetIEEE_802Dot1AS_2011();
  QSet<ActTimeSync802Dot1ASPortEntry>& port_entries = ieee_802dot1as_2011.GetPortEntries();

  time_sync_table.SetProfile(ActTimeSyncProfileEnum::kIEEE_802Dot1AS_2011);

  ActTimeSync802Dot1ASPortEntry port_entry(interface_id);
  QSet<ActTimeSync802Dot1ASPortEntry>::iterator port_entry_iter = port_entries.find(port_entry);
  if (port_entry_iter != port_entries.end()) {
    port_entry = *port_entry_iter;
    port_entries.erase(port_entry_iter);
  }

  if (configuration.isActiveSet()) {
    port_entry.SetEnable(configuration.getActive());
  }
  if (configuration.isAnnounceIntervalSet()) {
    port_entry.SetAnnounceInterval(configuration.getAnnounceInterval());
  }
  if (configuration.isAnnounceReceiptTimeoutSet()) {
    port_entry.SetAnnounceReceiptTimeout(configuration.getAnnounceReceiptTimeout());
  }
  if (configuration.isSyncIntervalSet()) {
    port_entry.SetSyncInterval(configuration.getSyncInterval());
  }
  if (configuration.isSyncReceiptTimeoutSet()) {
    port_entry.SetSyncReceiptTimeout(configuration.getSyncReceiptTimeout());
  }
  if (configuration.isPdelayRequestIntervalSet()) {
    port_entry.SetPdelayReqInterval(configuration.getPdelayRequestInterval());
  }
  if (configuration.isNeighborPropagationDelayThresholdSet()) {
    port_entry.SetNeighborPropDelayThresh(configuration.getNeighborPropagationDelayThreshold());
  }
  port_entries.insert(port_entry);

  act_status = checkTimeSyncPortNode(port_entry);
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