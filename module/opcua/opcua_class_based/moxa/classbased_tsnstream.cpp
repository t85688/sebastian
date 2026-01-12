#include "classbased_tsnstream.h"

namespace ClassBased {

UaStatus updateTsnStreamNodes(const ActProject& project, const ActTrafficDesign& traffic_design,
                              OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  QSet<QString> stream_nodes;
  UaNodeId tsnStreamFolderNodeId = pMoxaNodeManager->getTsnStreamFolderNodeId(project.GetId());

  for (ActTrafficStream stream : traffic_design.GetStreamSetting()) {
    UaNodeId tsnStreamNodeId = pMoxaNodeManager->getTsnStreamNodeId(project.GetId(), stream.GetId());
    MoxaClassBased::TsnStreamType* pTsnStreamType =
        (MoxaClassBased::TsnStreamType*)pMoxaNodeManager->getNode(tsnStreamNodeId);
    UaString streamBrowseName = UaString(stream.GetStreamName().toStdString().c_str());
    stream_nodes.insert(QString(streamBrowseName.toUtf8()));
    if (pTsnStreamType == NULL) {
      pTsnStreamType =
          new MoxaClassBased::TsnStreamType(tsnStreamNodeId, streamBrowseName, pMoxaNodeManager->getNameSpaceIndex(),
                                            pMoxaNodeManager->getNodeManagerConfig());
      if (!pTsnStreamType) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Allocate memory failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      ret = pMoxaNodeManager->addNodeAndReference(tsnStreamFolderNodeId, pTsnStreamType->getUaReferenceLists(),
                                                  OpcUaId_HasComponent);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Create SNMP trap host node failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    } else {
      pTsnStreamType->setBrowseName(UaQualifiedName(streamBrowseName, pMoxaNodeManager->getNameSpaceIndex()));
      pTsnStreamType->setDisplayName(UaLocalizedText(UaString(), streamBrowseName));
    }

    pTsnStreamType->setStreamName(stream.GetStreamName().toStdString().c_str());

    ActTrafficApplication application;
    act_status = ActGetItemById<ActTrafficApplication>(project.GetTrafficDesign().GetApplicationSetting(),
                                                       stream.GetApplicationId(), application);
    if (!IsActStatusSuccess(act_status)) {
      errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
      errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    pTsnStreamType->setApplicationName(application.GetApplicationName().toStdString().c_str());
    pTsnStreamType->setMulticast(stream.GetMulticast());

    // Transfer the MAC address to byte array
    pTsnStreamType->setDestinationAddress(UaString(stream.GetDestinationMac().toUtf8()));

    // Talker setting
    const ActTrafficStreamInterface& talker = stream.GetTalker();
    ActDevice talker_device;
    act_status = project.GetDeviceById(talker_device, talker.GetDeviceId());
    if (!IsActStatusSuccess(act_status)) {
      errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
      errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    ActInterface talker_interface;
    for (ActInterface intf : talker_device.GetInterfaces()) {
      if (intf.GetInterfaceId() == talker.GetInterfaceId()) {
        talker_interface = intf;
      }
    }

    UaString talkerIpAddress = UaString(talker_device.GetIpv4().GetIpAddress().toStdString().c_str());
    UaString talkerInterfaceName = UaString(talker_interface.GetInterfaceName().toStdString().c_str());
    UaString talkerBrowseName = UaString("%1_%2").arg(talkerIpAddress).arg(talkerInterfaceName);
    UaNodeId tsnTalkerNodeId =
        pMoxaNodeManager->getTsnStreamTalkerNodeId(project.GetId(), stream.GetId(), QString(talkerBrowseName.toUtf8()));

    MoxaClassBased::TsnInterfaceConfigurationTalkerType* pTsnInterfaceConfigurationTalkerType =
        (MoxaClassBased::TsnInterfaceConfigurationTalkerType*)pMoxaNodeManager->getNode(tsnTalkerNodeId);
    if (pTsnInterfaceConfigurationTalkerType == NULL) {
      pTsnInterfaceConfigurationTalkerType = new MoxaClassBased::TsnInterfaceConfigurationTalkerType(
          tsnTalkerNodeId, talkerBrowseName, pMoxaNodeManager->getNameSpaceIndex(),
          pMoxaNodeManager->getNodeManagerConfig());
      if (!pTsnInterfaceConfigurationTalkerType) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Allocate memory failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      OpcUa::FolderType* pTalkerInterfaceFolderType = pTsnStreamType->getTalkerInterface();
      ret = pMoxaNodeManager->addNodeAndReference(pTalkerInterfaceFolderType->nodeId(),
                                                  pTsnInterfaceConfigurationTalkerType->getUaReferenceLists(),
                                                  OpcUaId_HasComponent);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Create talker node failed");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }

    pTsnInterfaceConfigurationTalkerType->setIpAddress(talkerIpAddress);
    pTsnInterfaceConfigurationTalkerType->setInterfaceName(talkerInterfaceName);

    // Remove unused talker
    UaReferenceDescriptions talker_references;
    pMoxaNodeManager->getNodeReference(pTsnStreamType->getTalkerInterface()->nodeId(), OpcUa_False, talker_references);
    for (OpcUa_UInt32 idx = 0; idx < talker_references.length(); idx++) {
      UaNode* pNode = pMoxaNodeManager->getNode(talker_references[idx].NodeId.NodeId);
      if (talker_references[idx].TypeDefinition.NodeId.Identifier.Numeric ==
              MoxaClassBasedId_TsnInterfaceConfigurationTalkerType &&
          pNode->browseName().toString() == talkerBrowseName) {
        continue;
      }
      ret = pMoxaNodeManager->deleteUaNode(pNode, OpcUa_True, OpcUa_True, OpcUa_True);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Remove talker %1 failed").arg(pNode->nodeId().toString());
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }

    // Listener setting
    QSet<QString> listener_nodes;
    for (ActTrafficStreamInterface listener : stream.GetListeners()) {
      ActDevice listener_device;
      act_status = project.GetDeviceById(listener_device, listener.GetDeviceId());
      if (!IsActStatusSuccess(act_status)) {
        errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
        errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      ActInterface listener_interface;
      for (ActInterface intf : listener_device.GetInterfaces()) {
        if (intf.GetInterfaceId() == listener.GetInterfaceId()) {
          listener_interface = intf;
          break;
        }
      }

      UaString listenerIpAddress = UaString(listener_device.GetIpv4().GetIpAddress().toStdString().c_str());
      UaString listenerInterfaceName = UaString(listener_interface.GetInterfaceName().toStdString().c_str());
      UaString listenerBrowseName = UaString("%1_%2").arg(listenerIpAddress).arg(listenerInterfaceName);
      UaNodeId tsnListenerNodeId = pMoxaNodeManager->getTsnStreamListenerNodeId(project.GetId(), stream.GetId(),
                                                                                QString(listenerBrowseName.toUtf8()));

      MoxaClassBased::TsnInterfaceConfigurationListenerType* pTsnInterfaceConfigurationListenerType =
          (MoxaClassBased::TsnInterfaceConfigurationListenerType*)pMoxaNodeManager->getNode(tsnListenerNodeId);
      if (pTsnInterfaceConfigurationListenerType == NULL) {
        pTsnInterfaceConfigurationListenerType = new MoxaClassBased::TsnInterfaceConfigurationListenerType(
            tsnListenerNodeId, listenerBrowseName, pMoxaNodeManager->getNameSpaceIndex(),
            pMoxaNodeManager->getNodeManagerConfig());
        if (!pTsnInterfaceConfigurationListenerType) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: Allocate memory failed");
          qDebug() << errorMessage.toUtf8();
          return ret;
        }

        OpcUa::FolderType* pListenerInterfaceFolderType = pTsnStreamType->getListenerInterfaces();
        ret = pMoxaNodeManager->addNodeAndReference(pListenerInterfaceFolderType->nodeId(),
                                                    pTsnInterfaceConfigurationListenerType->getUaReferenceLists(),
                                                    OpcUaId_HasComponent);
        if (ret.isNotGood()) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: Create listener node failed");
          qDebug() << errorMessage.toUtf8();
          return ret;
        }
      }

      pTsnInterfaceConfigurationListenerType->setIpAddress(listenerIpAddress);
      pTsnInterfaceConfigurationListenerType->setInterfaceName(listenerInterfaceName);

      listener_nodes.insert(QString(listenerBrowseName.toUtf8()));
    }

    // Remove unused listeners
    UaReferenceDescriptions listener_references;
    pMoxaNodeManager->getNodeReference(pTsnStreamType->getListenerInterfaces()->nodeId(), OpcUa_False,
                                       listener_references);
    for (OpcUa_UInt32 idx = 0; idx < listener_references.length(); idx++) {
      UaNode* pNode = pMoxaNodeManager->getNode(listener_references[idx].NodeId.NodeId);
      if (listener_references[idx].TypeDefinition.NodeId.Identifier.Numeric ==
              MoxaClassBasedId_TsnInterfaceConfigurationListenerType &&
          listener_nodes.contains(QString(pNode->browseName().toString().toUtf8()))) {
        continue;
      }
      ret = pMoxaNodeManager->deleteUaNode(pNode, OpcUa_True, OpcUa_True, OpcUa_True);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Remove listener %1 failed").arg(pNode->nodeId().toString());
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }
  }

  // Remove unused streams
  UaReferenceDescriptions stream_references;
  pMoxaNodeManager->getNodeReference(tsnStreamFolderNodeId, OpcUa_False, stream_references);
  for (OpcUa_UInt32 idx = 0; idx < stream_references.length(); idx++) {
    UaNode* pNode = pMoxaNodeManager->getNode(stream_references[idx].NodeId.NodeId);
    if (stream_references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_TsnStreamType) {
      continue;
    }
    if (!stream_nodes.contains(QString(pNode->browseName().toString().toUtf8()))) {
      ret = pMoxaNodeManager->deleteUaNode(pNode, OpcUa_True, OpcUa_True, OpcUa_True);
      if (ret.isNotGood()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Remove stream %1 failed").arg(pNode->nodeId().toString());
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }
  }

  return ret;
}

UaStatus addTsnStreamMethod(const UaNodeId& tsnStreamFolderNodeId,
                            const MoxaClassBased::TsnStreamDataType& configuration, UaNodeId& tsnStreamNodeId,
                            OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  UaString browse_name = configuration.getStreamName();
  qint64 project_id = pMoxaNodeManager->getProjectId(tsnStreamFolderNodeId);
  QRegExp macRegex("^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$");

  if (!configuration.isStreamNameSet()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: User need to set StreamName");
    qDebug() << errorMessage.toUtf8();
    return ret;
  } else if (!configuration.isApplicationNameSet()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: User need to set ApplicationName");
    qDebug() << errorMessage.toUtf8();
    return ret;
  } else if (!configuration.isDestinationAddressSet()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: User need to set DestinationAddress");
    qDebug() << errorMessage.toUtf8();
    return ret;
  } else if (!macRegex.exactMatch(QString(configuration.getDestinationAddress().toUtf8()))) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: The DestinationAddress format is xx:xx:xx:xx:xx:xx or xx-xx-xx-xx-xx-xx");
    qDebug() << errorMessage.toUtf8();
    return ret;
  } else if (!configuration.isTsnTalkerInterfaceConfigurationSet()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: User need to set TsnTalkerInterfaceConfiguration");
    qDebug() << errorMessage.toUtf8();
    return ret;
  } else if (!configuration.isTsnListenerInterfaceConfigurationSet()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: User need to set TsnListenerInterfaceConfiguration");
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

  ActTrafficStream traffic_stream;
  traffic_stream.SetStreamName(configuration.getStreamName().toUtf8());
  traffic_stream.SetDestinationMac(configuration.getDestinationAddress().toUtf8());

  QSet<ActTrafficApplication>& application_setting = project.GetTrafficDesign().GetApplicationSetting();
  for (ActTrafficApplication application : application_setting) {
    if (application.GetApplicationName() == QString(configuration.getApplicationName().toUtf8())) {
      traffic_stream.SetApplicationId(application.GetId());
      break;
    }
  }
  if (traffic_stream.GetApplicationId() == 0) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Cannot find application %1").arg(configuration.getApplicationName().toUtf8());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }
  if (configuration.isMulticastSet()) {
    traffic_stream.SetMulticast(configuration.getMulticast());
  }

  MoxaClassBased::TsnInterfaceConfigurationTalkerDataType tsnTalkerInterfaceConfiguration =
      configuration.getTsnTalkerInterfaceConfiguration();
  ActDevice device;
  act_status = project.GetDeviceByIp(QString(tsnTalkerInterfaceConfiguration.getIpAddress().toUtf8()), device);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActTrafficStreamInterface talker;
  talker.SetDeviceId(device.GetId());

  QString interface_name = QString(tsnTalkerInterfaceConfiguration.getInterfaceName().toUtf8());
  for (ActInterface intf : device.GetInterfaces()) {
    if (intf.GetInterfaceName() != interface_name) {
      continue;
    } else if (!intf.GetUsed()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: The device %1 interface %2 doesn't connect link")
                         .arg(device.GetIpv4().GetIpAddress().toStdString().c_str())
                         .arg(intf.GetInterfaceName().toStdString().c_str());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }
    talker.SetInterfaceId(intf.GetInterfaceId());
  }

  if (talker.GetInterfaceId() == 0) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Cannot find interface name %1 in talker device %2")
                       .arg(tsnTalkerInterfaceConfiguration.getInterfaceName())
                       .arg(tsnTalkerInterfaceConfiguration.getIpAddress());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  traffic_stream.SetTalker(talker);

  MoxaClassBased::TsnInterfaceConfigurationListenerDataTypes tsnListenerInterfaceConfigurations;
  configuration.getTsnListenerInterfaceConfiguration(tsnListenerInterfaceConfigurations);

  QSet<ActTrafficStreamInterface> listeners;
  for (OpcUa_UInt32 index = 0; index < tsnListenerInterfaceConfigurations.length(); index++) {
    MoxaClassBased_TsnInterfaceConfigurationListenerDataType& tsnInterfaceConfigurationListenerDataType =
        tsnListenerInterfaceConfigurations[index];

    MoxaClassBased::TsnInterfaceConfigurationListenerDataType tsnListenerInterfaceConfiguration(
        tsnInterfaceConfigurationListenerDataType);

    ActDevice device;
    act_status = project.GetDeviceByIp(QString(tsnListenerInterfaceConfiguration.getIpAddress().toUtf8()), device);
    if (!IsActStatusSuccess(act_status)) {
      errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
      errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    ActTrafficStreamInterface listener;
    listener.SetDeviceId(device.GetId());

    QString interface_name = QString(tsnListenerInterfaceConfiguration.getInterfaceName().toUtf8());
    for (ActInterface intf : device.GetInterfaces()) {
      if (intf.GetInterfaceName() != interface_name) {
        continue;
      } else if (!intf.GetUsed()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: The device %1 interface %2 doesn't connect link")
                           .arg(device.GetIpv4().GetIpAddress().toStdString().c_str())
                           .arg(intf.GetInterfaceName().toStdString().c_str());
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
      listener.SetInterfaceId(intf.GetInterfaceId());
    }

    if (listener.GetInterfaceId() == 0) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Cannot find interface name %1 in listener device %2")
                         .arg(tsnListenerInterfaceConfiguration.getInterfaceName())
                         .arg(tsnListenerInterfaceConfiguration.getIpAddress());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    listeners.insert(listener);
  }
  traffic_stream.SetListeners(listeners);

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.CreateTrafficStreamSetting(project_id, traffic_stream);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(tsnStreamFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_TsnStreamType) {
      continue;
    }
    MoxaClassBased::TsnStreamType* pTsnStreamType =
        (MoxaClassBased::TsnStreamType*)pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);

    if (pTsnStreamType->browseName().toString() == browse_name) {
      tsnStreamNodeId = pTsnStreamType->nodeId();
      break;
    }
  }

  return ret;
}

UaStatus setTsnStreamMethod(const UaNodeId& tsnStreamNodeId, const MoxaClassBased::TsnStreamDataType& configuration,
                            OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  UaString browse_name = configuration.getStreamName();
  qint64 project_id = pMoxaNodeManager->getProjectId(tsnStreamNodeId);
  qint64 stream_id = pMoxaNodeManager->getTsnStreamId(tsnStreamNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  QSet<ActTrafficStream>& stream_setting = project.GetTrafficDesign().GetStreamSetting();
  ActTrafficStream traffic_stream(stream_id);
  QSet<ActTrafficStream>::iterator traffic_stream_iter = stream_setting.find(traffic_stream);
  if (traffic_stream_iter == stream_setting.end()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Cannot find stream %1 in stream setting").arg(int(stream_id));
    qDebug() << errorMessage.toUtf8();
    return ret;
  }
  traffic_stream = *traffic_stream_iter;

  if (configuration.isStreamNameSet()) {
    traffic_stream.SetStreamName(configuration.getStreamName().toUtf8());
  }
  if (configuration.isApplicationNameSet()) {
    QSet<ActTrafficApplication>& application_setting = project.GetTrafficDesign().GetApplicationSetting();
    for (ActTrafficApplication application : application_setting) {
      if (application.GetApplicationName() == QString(configuration.getApplicationName().toUtf8())) {
        traffic_stream.SetApplicationId(application.GetId());
        break;
      }
    }
  }
  if (configuration.isMulticastSet()) {
    traffic_stream.SetMulticast(configuration.getMulticast());
  }
  if (configuration.isDestinationAddressSet()) {
    QRegExp macRegex("^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$");
    if (!macRegex.exactMatch(QString(configuration.getDestinationAddress().toUtf8()))) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: The DestinationAddress format is xx:xx:xx:xx:xx:xx or xx-xx-xx-xx-xx-xx");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }
    traffic_stream.SetDestinationMac(configuration.getDestinationAddress().toUtf8());
  }

  if (configuration.isTsnTalkerInterfaceConfigurationSet()) {
    MoxaClassBased::TsnInterfaceConfigurationTalkerDataType tsnTalkerInterfaceConfiguration =
        configuration.getTsnTalkerInterfaceConfiguration();

    ActDevice device;
    act_status = project.GetDeviceByIp(QString(tsnTalkerInterfaceConfiguration.getIpAddress().toUtf8()), device);
    if (!IsActStatusSuccess(act_status)) {
      errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
      errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    ActTrafficStreamInterface talker;
    talker.SetDeviceId(device.GetId());

    QString interface_name = QString(tsnTalkerInterfaceConfiguration.getInterfaceName().toUtf8());
    for (ActInterface intf : device.GetInterfaces()) {
      if (intf.GetInterfaceName() != interface_name) {
        continue;
      }
      talker.SetInterfaceId(intf.GetInterfaceId());
    }

    if (talker.GetInterfaceId() == 0) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Cannot find interface name %1 in talker device %2")
                         .arg(tsnTalkerInterfaceConfiguration.getInterfaceName())
                         .arg(tsnTalkerInterfaceConfiguration.getIpAddress());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    traffic_stream.SetTalker(talker);
  }

  if (configuration.isTsnListenerInterfaceConfigurationSet()) {
    MoxaClassBased::TsnInterfaceConfigurationListenerDataTypes tsnListenerInterfaceConfigurations;
    configuration.getTsnListenerInterfaceConfiguration(tsnListenerInterfaceConfigurations);

    QSet<ActTrafficStreamInterface> listeners;
    for (OpcUa_UInt32 index = 0; index < tsnListenerInterfaceConfigurations.length(); index++) {
      MoxaClassBased_TsnInterfaceConfigurationListenerDataType& tsnInterfaceConfigurationListenerDataType =
          tsnListenerInterfaceConfigurations[index];

      MoxaClassBased::TsnInterfaceConfigurationListenerDataType tsnListenerInterfaceConfiguration(
          tsnInterfaceConfigurationListenerDataType);

      ActDevice device;
      act_status = project.GetDeviceByIp(QString(tsnListenerInterfaceConfiguration.getIpAddress().toUtf8()), device);
      if (!IsActStatusSuccess(act_status)) {
        errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
        errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      ActTrafficStreamInterface listener;
      listener.SetDeviceId(device.GetId());

      QString interface_name = QString(tsnListenerInterfaceConfiguration.getInterfaceName().toUtf8());
      for (ActInterface intf : device.GetInterfaces()) {
        if (intf.GetInterfaceName() != interface_name) {
          continue;
        }
        listener.SetInterfaceId(intf.GetInterfaceId());
      }

      if (listener.GetInterfaceId() == 0) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: Cannot find interface name %1 in listener device %2")
                           .arg(tsnListenerInterfaceConfiguration.getInterfaceName())
                           .arg(tsnListenerInterfaceConfiguration.getIpAddress());
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      listeners.insert(listener);
    }
    traffic_stream.SetListeners(listeners);
  }

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.UpdateTrafficStreamSetting(project_id, traffic_stream);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}

UaStatus removeTsnStreamMethod(const UaNodeId& tsnStreamFolderNodeId, const UaNodeId& tsnStreamNodeId,
                               OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(tsnStreamNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  UaReferenceDescriptions references;
  MoxaClassBased::TsnStreamType* pTsnStreamType = NULL;
  pMoxaNodeManager->getNodeReference(tsnStreamFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_TsnStreamType) {
      continue;
    }

    if (UaNodeId(references[idx].NodeId.NodeId) == tsnStreamNodeId) {
      pTsnStreamType = (MoxaClassBased::TsnStreamType*)pMoxaNodeManager->getNode(tsnStreamNodeId);
      break;
    }
  }

  if (pTsnStreamType == NULL) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: NodeId %1 is not found").arg(tsnStreamNodeId.toFullString());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  QMutexLocker lock(&act::core::g_core.mutex_);

  qint64 traffic_stream_id = pMoxaNodeManager->getTsnStreamId(tsnStreamNodeId);
  act_status = act::core::g_core.DeleteTrafficStreamSetting(project_id, traffic_stream_id);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}
}  // namespace ClassBased