#include "classbased_tsnstream.h"

namespace ClassBased {

static bool checkTsnStreamApplicationFeasibility(const ActProject& project,
                                                 const ActTrafficApplication& traffic_application,
                                                 const MoxaClassBased::TsnStreamApplicationDataType& configuration,
                                                 const bool new_entry, OpcUa_UInt32& errorCode,
                                                 UaString& errorMessage) {
  if (new_entry) {
    if (!configuration.isApplicationNameSet()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: User need to set ApplicationName");
      qDebug() << errorMessage.toUtf8();
      return false;
    }

    if (!configuration.isTrafficTypeSet()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: User need to set TrafficType");
      qDebug() << errorMessage.toUtf8();
      return false;
    }

    if (!configuration.isQoSTypeSet()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: User need to set QoSType");
      qDebug() << errorMessage.toUtf8();
      return false;
    }

    if (!configuration.isTagStreamSet()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: User need to set TagStream");
      qDebug() << errorMessage.toUtf8();
      return false;
    }
  }

  QMap<QString, ActTrafficTypeConfiguration> traffic_type_map;
  QMap<quint8, ActTrafficTypeConfiguration> traffic_class_map;
  for (ActTrafficTypeConfiguration traffic_type_configuration :
       project.GetTrafficDesign().GetTrafficTypeConfigurationSetting()) {
    traffic_type_map.insert(traffic_type_configuration.GetTrafficType(), traffic_type_configuration);
    traffic_class_map.insert(traffic_type_configuration.GetTrafficClass(), traffic_type_configuration);
  }

  QString traffic_type;
  if (configuration.isTrafficTypeSet()) {
    traffic_type = QString(configuration.getTrafficType().toUtf8());
    if (!traffic_type_map.contains(traffic_type)) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: TrafficType %1 is not found").arg(traffic_type.toStdString().c_str());
      qDebug() << errorMessage.toUtf8();
      return false;
    }
  } else {
    traffic_type = traffic_class_map[traffic_application.GetTrafficSetting().GetTrafficTypeClass()].GetTrafficType();
  }

  if (configuration.isQoSTypeSet()) {
    switch (configuration.getQoSType()) {
      case MoxaClassBased::QoSEnumType::QoSEnumType_Bandwidth:
        break;
      case MoxaClassBased::QoSEnumType::QoSEnumType_BoundedLatency:
        if (!configuration.isReceiveOffsetMinSet() || !configuration.isReceiveOffsetMaxSet()) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage =
              UaString("Invalid: QoSType is BoundedLatency, user need to set ReceiveOffsetMin and ReceiveOffsetMax");
          qDebug() << errorMessage.toUtf8();
          return false;
        }
        if (configuration.getReceiveOffsetMin() < 1 || configuration.getReceiveOffsetMin() > 999999.999) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: The valid range of ReceiveOffsetMin is 1 to 999999.999 us.");
          qDebug() << errorMessage.toUtf8();
          return false;
        }
        if (configuration.getReceiveOffsetMax() < 1 || configuration.getReceiveOffsetMax() > 999999.999) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: The valid range of ReceiveOffsetMax is 1 to 999999.999 us.");
          qDebug() << errorMessage.toUtf8();
          return false;
        }
        if (configuration.getReceiveOffsetMin() > configuration.getReceiveOffsetMax()) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: The ReceiveOffsetMin %1 cannot be larger than ReceiveOffsetMax %2")
                             .arg(configuration.getReceiveOffsetMin())
                             .arg(configuration.getReceiveOffsetMax());
          qDebug() << errorMessage.toUtf8();
          return false;
        }
        break;
      case MoxaClassBased::QoSEnumType::QoSEnumType_Deadline:
        if (!configuration.isLatencyMaxSet()) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: QoSType is Deadline, user need to set LatencyMax");
          qDebug() << errorMessage.toUtf8();
          return false;
        }
        if (configuration.getLatencyMax() < 1 || configuration.getLatencyMax() > 999999.999) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: The valid range of LatencyMax is 1 to 999999.999 us.");
          qDebug() << errorMessage.toUtf8();
          return false;
        }
        break;
    }
  }

  if (configuration.isTagStreamSet()) {
    if (configuration.getTagStream()) {
      if (!configuration.isTagStreamVlanTagSet()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: TagStream is enable, user need to set TagStreamVlanTag");
        qDebug() << errorMessage.toUtf8();
        return false;
      }
      if (configuration.getTagStreamVlanTag().getVlanId() < project.GetVlanRange().GetMin() ||
          configuration.getTagStreamVlanTag().getVlanId() > project.GetVlanRange().GetMax()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: The valid range of VlanId is %1 to %2. The input value is %3")
                           .arg(project.GetVlanRange().GetMin())
                           .arg(project.GetVlanRange().GetMax())
                           .arg(configuration.getTagStreamVlanTag().getVlanId());
        qDebug() << errorMessage.toUtf8();
        return false;
      }
      quint8 priority_code_point = configuration.getTagStreamVlanTag().getPriorityCodePoint();
      if (!traffic_type_map[traffic_type].GetPriorityCodePointSet().contains(priority_code_point)) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage =
            UaString("Invalid: TrafficType %1 doesn't contain Priority Code Point %2 in TrafficConfigurationTable")
                .arg(traffic_type.toStdString().c_str())
                .arg(priority_code_point);
        qDebug() << errorMessage.toUtf8();
        return false;
      }
    } else {
      if (!configuration.isPerStreamPrioritySet()) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: TagStream is disable, user need to set PerStreamPrioriy");
        qDebug() << errorMessage.toUtf8();
        return false;
      }

      if (configuration.getPerStreamPriority().isVlanTagSet()) {
        if (configuration.getPerStreamPriority().getVlanTag().getVlanId() < project.GetVlanRange().GetMin() ||
            configuration.getPerStreamPriority().getVlanTag().getVlanId() > project.GetVlanRange().GetMax()) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: The valid range of VlanId is %1 to %2. The input value is %3")
                             .arg(project.GetVlanRange().GetMin())
                             .arg(project.GetVlanRange().GetMax())
                             .arg(configuration.getPerStreamPriority().getVlanTag().getVlanId());
          qDebug() << errorMessage.toUtf8();
          return false;
        }
        quint8 priority_code_point = configuration.getPerStreamPriority().getVlanTag().getPriorityCodePoint();
        if (!traffic_type_map[traffic_type].GetPriorityCodePointSet().contains(priority_code_point)) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage =
              UaString("Invalid: TrafficType %1 doesn't contain Priority Code Point %2 in TrafficConfigurationTable")
                  .arg(traffic_type.toStdString().c_str())
                  .arg(priority_code_point);
          qDebug() << errorMessage.toUtf8();
          return false;
        }
      }

      switch (configuration.getPerStreamPriority().getPerStreamPriorityMode()) {
        case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L2:
          if (!configuration.getPerStreamPriority().isEtherTypeSet()) {
            errorCode = M_UA_INTERNAL_ERROR;
            errorMessage = UaString("Invalid: PerStreamPriorityMode is L2, user need to set EtherType");
            qDebug() << errorMessage.toUtf8();
            return false;
          }
          if (configuration.getPerStreamPriority().isSubtypeEnableSet()) {
            if (configuration.getPerStreamPriority().getSubtypeEnable() &&
                !configuration.getPerStreamPriority().isSubtypeSet()) {
              errorCode = M_UA_INTERNAL_ERROR;
              errorMessage =
                  UaString("Invalid: PerStreamPriorityMode is L2 and EnableSubType is true, user need to set SubType");
              qDebug() << errorMessage.toUtf8();
              return false;
            }
          }
          break;
        case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3TCP:
          if (!configuration.getPerStreamPriority().isTCPPortSet()) {
            errorCode = M_UA_INTERNAL_ERROR;
            errorMessage = UaString("Invalid: PerStreamPriorityMode is L3TCP, user need to set TCPPort");
            qDebug() << errorMessage.toUtf8();
            return false;
          }
          break;
        case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3UDP:
          if (!configuration.getPerStreamPriority().isUDPPortSet()) {
            errorCode = M_UA_INTERNAL_ERROR;
            errorMessage = UaString("Invalid: PerStreamPriorityMode is L3UDP, user need to set UDPPort.");
            qDebug() << errorMessage.toUtf8();
            return false;
          }
          break;
      }
    }
  } else {
    if (configuration.isTagStreamSet()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: User need to set TagStream to true");
      qDebug() << errorMessage.toUtf8();
      return false;
    } else if (configuration.isPerStreamPrioritySet() || configuration.isTagMethodSet()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: User need to set TagStream to false");
      qDebug() << errorMessage.toUtf8();
      return false;
    }
  }

  // Stream Parameter
  qreal interval = traffic_application.GetStreamParameter().GetInterval();
  quint32 max_frame_size = traffic_application.GetStreamParameter().GetMaxFrameSize();
  quint32 max_frames_per_interval = traffic_application.GetStreamParameter().GetMaxFramesPerInterval();
  quint32 max_bytes_per_interval = traffic_application.GetStreamParameter().GetMaxBytesPerInterval();
  qreal earliest_transmit_offset = traffic_application.GetStreamParameter().GetEarliestTransmitOffset();
  qreal latest_transmit_offset = traffic_application.GetStreamParameter().GetLatestTransmitOffset();
  qreal jitter = traffic_application.GetStreamParameter().GetJitter();
  if (configuration.isTsnTrafficSpecificationSet()) {
    interval = qreal(configuration.getTsnTrafficSpecification().getInterval().getNumerator()) /
               qreal(configuration.getTsnTrafficSpecification().getInterval().getDenominator());
    max_frame_size = configuration.getTsnTrafficSpecification().getMaxFrameSize();
    max_frames_per_interval = configuration.getTsnTrafficSpecification().getMaxIntervalFrames();
    max_bytes_per_interval = configuration.getTsnTrafficSpecification().getMaxBytesPerInterval();
  }

  if (interval < 30 || interval > 999999.999) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage =
        UaString("Invalid: The valid range of Interval is 30 to 999999.999 us. The value is %1 us.").arg(interval);
    qDebug() << errorMessage.toUtf8();
    return false;
  }
  if (max_frame_size < 46 || max_frame_size > 1500) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: The valid range of MaxFrameSize is 46 to 1500 bytes. The value is %1 bytes.")
                       .arg(max_frame_size);
    qDebug() << errorMessage.toUtf8();
    return false;
  }
  if (max_frames_per_interval < 1 || max_frames_per_interval > 65535) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString(
                       "Invalid: The valid range of MaxIntervalFrames is 1 to 65535. The value is %1 frames "
                       "per interval.")
                       .arg(max_frames_per_interval);
    qDebug() << errorMessage.toUtf8();
    return false;
  }
  if (max_bytes_per_interval < 0 || max_bytes_per_interval > max_frame_size * max_frames_per_interval) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: The valid range of MaxBytesPerInterval is 0 to %1 bytes. The value is %2 bytes.")
                       .arg(max_frame_size * max_frames_per_interval)
                       .arg(max_bytes_per_interval);
    qDebug() << errorMessage.toUtf8();
    return false;
  }

  if (configuration.isTransmissionOffsetMinSet()) {
    earliest_transmit_offset = configuration.getTransmissionOffsetMin();
  }
  if (earliest_transmit_offset < 0 || earliest_transmit_offset > interval) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage =
        UaString("Invalid: The valid range of TransmissionOffsetMin is 0 to %1 us. The input value is %2 us.")
            .arg(interval)
            .arg(earliest_transmit_offset);
    qDebug() << errorMessage.toUtf8();
    return false;
  }

  if (configuration.isTransmissionOffsetMaxSet()) {
    latest_transmit_offset = configuration.getTransmissionOffsetMax();
  }
  if (latest_transmit_offset < earliest_transmit_offset || latest_transmit_offset > interval) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage =
        UaString("Invalid: The valid range of TransmissionOffsetMax is %1 to %2 us. The input value is %3 us.")
            .arg(earliest_transmit_offset)
            .arg(interval)
            .arg(latest_transmit_offset);
    qDebug() << errorMessage.toUtf8();
    return false;
  }

  if (configuration.isTransmissionJitterSet()) {
    jitter = configuration.getTransmissionJitter();
  }
  if (jitter < 0 || jitter > 999999.999) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage =
        UaString("Invalid: The valid range of TransmissionJitter is 0 to 999999.999 us. The input value is %1 us.")
            .arg(jitter);
    qDebug() << errorMessage.toUtf8();
    return false;
  }

  return true;
}

UaStatus updateTsnStreamApplicationNodes(const ActProject& project, const ActTrafficDesign& traffic_design,
                                         OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  UaNodeId tsnStreamApplicationFolderNodeId = pMoxaNodeManager->getTsnStreamApplicationFolderNodeId(project.GetId());

  // clear applications
  UaReferenceDescriptions application_references;
  pMoxaNodeManager->getNodeReference(tsnStreamApplicationFolderNodeId, OpcUa_False, application_references);
  for (OpcUa_UInt32 idx = 0; idx < application_references.length(); idx++) {
    UaNode* pNode = pMoxaNodeManager->getNode(application_references[idx].NodeId.NodeId);
    if (application_references[idx].TypeDefinition.NodeId.Identifier.Numeric !=
        MoxaClassBasedId_TsnStreamApplicationType) {
      continue;
    }
    ret = pMoxaNodeManager->deleteUaNode(pNode, OpcUa_True, OpcUa_True, OpcUa_True);
    if (ret.isNotGood()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Remove application %1 failed").arg(pNode->nodeId().toString());
      qDebug() << errorMessage.toUtf8();
      return ret;
    }
  }

  for (ActTrafficApplication application : traffic_design.GetApplicationSetting()) {
    UaNodeId tsnStreamApplicationNodeId =
        pMoxaNodeManager->getTsnStreamApplicationNodeId(project.GetId(), application.GetId());
    UaString browseName = UaString(application.GetApplicationName().toStdString().c_str());

    MoxaClassBased::TsnStreamApplicationType* pTsnStreamApplicationType = new MoxaClassBased::TsnStreamApplicationType(
        tsnStreamApplicationNodeId, browseName, pMoxaNodeManager->getNameSpaceIndex(),
        pMoxaNodeManager->getNodeManagerConfig());
    if (!pTsnStreamApplicationType) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Allocate memory failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    ret = pMoxaNodeManager->addNodeAndReference(tsnStreamApplicationFolderNodeId,
                                                pTsnStreamApplicationType->getUaReferenceLists(), OpcUaId_HasComponent);
    if (ret.isNotGood()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Create SNMP trap host node failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    pTsnStreamApplicationType->setApplicationName(application.GetApplicationName().toStdString().c_str());

    // Traffic Setting
    ActTrafficSetting& traffic_setting = application.GetTrafficSetting();
    for (ActTrafficTypeConfiguration traffic_type_configuration : traffic_design.GetTrafficTypeConfigurationSetting()) {
      if (traffic_type_configuration.GetTrafficClass() != traffic_setting.GetTrafficTypeClass()) {
        continue;
      }
      pTsnStreamApplicationType->setTrafficType(
          UaString(traffic_type_configuration.GetTrafficType().toStdString().c_str()));
      break;
    }
    switch (traffic_setting.GetQosType()) {
      case ActTrafficQosTypeEnum::kBandwidth:
        pTsnStreamApplicationType->setQoSType(MoxaClassBased::QoSEnumType::QoSEnumType_Bandwidth);
        break;
      case ActTrafficQosTypeEnum::kBoundedLatency:
        pTsnStreamApplicationType->setQoSType(MoxaClassBased::QoSEnumType::QoSEnumType_BoundedLatency);
        pTsnStreamApplicationType->getTsnListenerStream()->setReceiveOffsetMin(
            OpcUa_Double(traffic_setting.GetMinReceiveOffset()));
        pTsnStreamApplicationType->getTsnListenerStream()->setReceiveOffsetMax(
            OpcUa_Double(traffic_setting.GetMaxReceiveOffset()));
        pTsnStreamApplicationType->getTsnListenerStream()->setLatencyMax(0);
        break;
      case ActTrafficQosTypeEnum::kDeadline:
        pTsnStreamApplicationType->setQoSType(MoxaClassBased::QoSEnumType::QoSEnumType_Deadline);
        pTsnStreamApplicationType->getTsnListenerStream()->setReceiveOffsetMin(0);
        pTsnStreamApplicationType->getTsnListenerStream()->setReceiveOffsetMax(0);
        pTsnStreamApplicationType->getTsnListenerStream()->setLatencyMax(
            OpcUa_Double(traffic_setting.GetMaxReceiveOffset()));
        break;
    }

    // TSN
    ActTrafficTSN& tsn = application.GetTSN();
    pTsnStreamApplicationType->setFRERMode(tsn.GetFRER());

    // VLAN Setting
    ActTrafficVlanSetting& vlan_setting = application.GetVlanSetting();
    pTsnStreamApplicationType->setTagStream(vlan_setting.GetTagged());
    if (vlan_setting.GetTagged()) {
      pTsnStreamApplicationType->getTagStreamVlanTag()->setVlanId(OpcUa_UInt16(vlan_setting.GetVlanId()));
      pTsnStreamApplicationType->getTagStreamVlanTag()->setPriorityCodePoint(
          OpcUa_Byte(vlan_setting.GetPriorityCodePoint()));
    } else {
      switch (vlan_setting.GetUntaggedMode()) {
        case ActTrafficUntaggedModeEnum::kPerStreamPriority:
          pTsnStreamApplicationType->setTagMethod(
              MoxaClassBased::TagMethodEnumType::TagMethodEnumType_PerStreamPriority);
          break;
      }
      pTsnStreamApplicationType->getPerStreamPriority()->getVlanTag()->setVlanId(
          OpcUa_UInt16(vlan_setting.GetVlanId()));
      pTsnStreamApplicationType->getPerStreamPriority()->getVlanTag()->setPriorityCodePoint(
          OpcUa_Byte(vlan_setting.GetPriorityCodePoint()));
      pTsnStreamApplicationType->getPerStreamPriority()->setEtherType(OpcUa_UInt32(vlan_setting.GetEtherType()));
      pTsnStreamApplicationType->getPerStreamPriority()->setSubtype(OpcUa_UInt16(vlan_setting.GetSubType()));
      pTsnStreamApplicationType->getPerStreamPriority()->setSubtypeEnable(vlan_setting.GetEnableSubType());
      switch (*vlan_setting.GetType().begin()) {
        case ActStreamPriorityTypeEnum::kEthertype:
          pTsnStreamApplicationType->getPerStreamPriority()->setPerStreamPriorityMode(
              MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L2);
          break;
        case ActStreamPriorityTypeEnum::kTcp:
          pTsnStreamApplicationType->getPerStreamPriority()->setPerStreamPriorityMode(
              MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3TCP);
          break;
        case ActStreamPriorityTypeEnum::kUdp:
          pTsnStreamApplicationType->getPerStreamPriority()->setPerStreamPriorityMode(
              MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3UDP);
          break;
      }
      pTsnStreamApplicationType->getPerStreamPriority()->setUDPPort(OpcUa_UInt32(vlan_setting.GetUdpPort()));
      pTsnStreamApplicationType->getPerStreamPriority()->setTCPPort(OpcUa_UInt32(vlan_setting.GetTcpPort()));
    }

    // Stream Parameter
    ActTrafficStreamParameter& stream_parameter = application.GetStreamParameter();
    pTsnStreamApplicationType->getTsnTalkerStream()->getTsnTrafficSpecification()->setInterval(
        OpcUaClassBnm::UnsignedRationalNumber(OpcUa_UInt32(stream_parameter.GetInterval() * 1000), 1000));
    pTsnStreamApplicationType->getTsnTalkerStream()->getTsnTrafficSpecification()->setMaxBytesPerInterval(
        OpcUa_UInt32(stream_parameter.GetMaxBytesPerInterval()));
    pTsnStreamApplicationType->getTsnTalkerStream()->getTsnTrafficSpecification()->setMaxFrameSize(
        OpcUa_UInt32(stream_parameter.GetMaxFrameSize()));
    pTsnStreamApplicationType->getTsnTalkerStream()->getTsnTrafficSpecification()->setMaxIntervalFrames(
        OpcUa_UInt32(stream_parameter.GetMaxFramesPerInterval()));
    pTsnStreamApplicationType->getTsnTalkerStream()->setTransmissionJitter(OpcUa_Double(stream_parameter.GetJitter()));
    pTsnStreamApplicationType->getTsnTalkerStream()->setTransmissionOffsetMax(
        OpcUa_Double(stream_parameter.GetLatestTransmitOffset()));
    pTsnStreamApplicationType->getTsnTalkerStream()->setTransmissionOffsetMin(
        OpcUa_Double(stream_parameter.GetEarliestTransmitOffset()));
  }

  return ret;
}

UaStatus addTsnStreamApplicationMethod(const UaNodeId& tsnStreamApplicationFolderNodeId,
                                       const MoxaClassBased::TsnStreamApplicationDataType& configuration,
                                       UaNodeId& tsnStreamApplicationNodeId, OpcUa_UInt32& errorCode,
                                       UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(tsnStreamApplicationFolderNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  if (!checkTsnStreamApplicationFeasibility(project, ActTrafficApplication(), configuration, true, errorCode,
                                            errorMessage)) {
    return ret;
  }

  QMap<QString, ActTrafficTypeConfiguration> traffic_type_map;
  for (ActTrafficTypeConfiguration traffic_type_configuration :
       project.GetTrafficDesign().GetTrafficTypeConfigurationSetting()) {
    traffic_type_map.insert(traffic_type_configuration.GetTrafficType(), traffic_type_configuration);
  }

  ActTrafficApplication traffic_application;

  traffic_application.SetApplicationName(configuration.getApplicationName().toUtf8());

  // Traffic Setting
  ActTrafficSetting& traffic_setting = traffic_application.GetTrafficSetting();

  QString traffic_type = QString(configuration.getTrafficType().toUtf8());
  traffic_setting.SetTrafficTypeClass(traffic_type_map[traffic_type].GetTrafficClass());

  switch (configuration.getQoSType()) {
    case MoxaClassBased::QoSEnumType::QoSEnumType_Bandwidth:
      traffic_setting.SetQosType(ActTrafficQosTypeEnum::kBandwidth);
      break;
    case MoxaClassBased::QoSEnumType::QoSEnumType_BoundedLatency:
      traffic_setting.SetQosType(ActTrafficQosTypeEnum::kBoundedLatency);
      if (configuration.isReceiveOffsetMinSet()) {
        traffic_setting.SetMinReceiveOffset(qreal(configuration.getReceiveOffsetMin()));
      }
      if (configuration.isReceiveOffsetMaxSet()) {
        traffic_setting.SetMaxReceiveOffset(qreal(configuration.getReceiveOffsetMax()));
      }
      break;
    case MoxaClassBased::QoSEnumType::QoSEnumType_Deadline:
      traffic_setting.SetQosType(ActTrafficQosTypeEnum::kDeadline);
      if (configuration.isLatencyMaxSet()) {
        traffic_setting.SetMaxReceiveOffset(qreal(configuration.getLatencyMax()));
      }
      break;
  }

  // TSN
  ActTrafficTSN& tsn = traffic_application.GetTSN();
  tsn.SetActive(true);
  if (configuration.isFRERModeSet()) {
    tsn.SetFRER(configuration.getFRERMode());
  }

  // VLAN Setting
  ActTrafficVlanSetting& vlan_setting = traffic_application.GetVlanSetting();
  vlan_setting.SetTagged(configuration.getTagStream());

  if (vlan_setting.GetTagged()) {
    vlan_setting.SetVlanId(quint16(configuration.getTagStreamVlanTag().getVlanId()));
    vlan_setting.SetPriorityCodePoint(quint8(configuration.getTagStreamVlanTag().getPriorityCodePoint()));
  } else {
    vlan_setting.SetUntaggedMode(ActTrafficUntaggedModeEnum::kPerStreamPriority);
    vlan_setting.SetUserDefinedVlan(configuration.getPerStreamPriority().isVlanTagSet());

    if (configuration.getPerStreamPriority().isVlanTagSet()) {
      vlan_setting.SetVlanId(quint16(configuration.getPerStreamPriority().getVlanTag().getVlanId()));
      vlan_setting.SetPriorityCodePoint(
          quint8(configuration.getPerStreamPriority().getVlanTag().getPriorityCodePoint()));
    }

    switch (configuration.getPerStreamPriority().getPerStreamPriorityMode()) {
      case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L2:
        vlan_setting.SetType({ActStreamPriorityTypeEnum::kEthertype});
        vlan_setting.SetEtherType(quint32(configuration.getPerStreamPriority().getEtherType()));
        if (configuration.getPerStreamPriority().isSubtypeEnableSet()) {
          vlan_setting.SetEnableSubType(configuration.getPerStreamPriority().getSubtypeEnable());
          if (configuration.getPerStreamPriority().isSubtypeSet()) {
            vlan_setting.SetSubType(quint16(configuration.getPerStreamPriority().getSubtype()));
          }
        }
        break;
      case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3TCP:
        vlan_setting.SetType({ActStreamPriorityTypeEnum::kTcp});
        vlan_setting.SetTcpPort(qint32(configuration.getPerStreamPriority().getTCPPort()));
        break;
      case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3UDP:
        vlan_setting.SetType({ActStreamPriorityTypeEnum::kUdp});
        vlan_setting.SetUdpPort(qint32(configuration.getPerStreamPriority().getUDPPort()));
        break;
    }
  }

  // Stream Parameter
  ActTrafficStreamParameter& stream_parameter = traffic_application.GetStreamParameter();
  if (configuration.isTsnTrafficSpecificationSet()) {
    stream_parameter.SetInterval(qreal(configuration.getTsnTrafficSpecification().getInterval().getNumerator()) /
                                 qreal(configuration.getTsnTrafficSpecification().getInterval().getDenominator()));
    stream_parameter.SetMaxFrameSize(quint32(configuration.getTsnTrafficSpecification().getMaxFrameSize()));
    stream_parameter.SetMaxFramesPerInterval(
        quint32(configuration.getTsnTrafficSpecification().getMaxIntervalFrames()));
    stream_parameter.SetMaxBytesPerInterval(
        quint32(configuration.getTsnTrafficSpecification().getMaxBytesPerInterval()));
  }
  if (configuration.isTransmissionOffsetMinSet()) {
    stream_parameter.SetEarliestTransmitOffset(qreal(configuration.getTransmissionOffsetMin()));
  }
  if (configuration.isTransmissionOffsetMaxSet()) {
    stream_parameter.SetLatestTransmitOffset(qreal(configuration.getTransmissionOffsetMax()));
  } else {
    stream_parameter.SetLatestTransmitOffset(stream_parameter.GetInterval());
  }
  if (configuration.isTransmissionJitterSet()) {
    stream_parameter.SetJitter(qreal(configuration.getTransmissionJitter()));
  }

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.CreateTrafficApplicationSetting(project_id, traffic_application);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  UaString browse_name = configuration.getApplicationName();

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(tsnStreamApplicationFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_TsnStreamApplicationType) {
      continue;
    }
    MoxaClassBased::TsnStreamApplicationType* pTsnStreamApplicationType =
        (MoxaClassBased::TsnStreamApplicationType*)pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);

    if (pTsnStreamApplicationType->browseName().toString() == browse_name) {
      tsnStreamApplicationNodeId = pTsnStreamApplicationType->nodeId();
      break;
    }
  }

  return ret;
}

UaStatus setTsnStreamApplicationMethod(const UaNodeId& tsnStreamApplicationNodeId,
                                       const MoxaClassBased::TsnStreamApplicationDataType& configuration,
                                       OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(tsnStreamApplicationNodeId);
  qint64 application_id = pMoxaNodeManager->getTsnStreamApplicationId(tsnStreamApplicationNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  QSet<ActTrafficApplication>& application_setting = project.GetTrafficDesign().GetApplicationSetting();
  ActTrafficApplication traffic_application(application_id);
  QSet<ActTrafficApplication>::iterator traffic_application_iter = application_setting.find(traffic_application);
  if (traffic_application_iter == application_setting.end()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Cannot find application %1 in application setting").arg(int(application_id));
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  traffic_application = *traffic_application_iter;

  if (!checkTsnStreamApplicationFeasibility(project, traffic_application, configuration, false, errorCode,
                                            errorMessage)) {
    return ret;
  }

  if (configuration.isApplicationNameSet()) {
    traffic_application.SetApplicationName(configuration.getApplicationName().toUtf8());
  }

  // Traffic Setting
  ActTrafficSetting& traffic_setting = traffic_application.GetTrafficSetting();

  if (configuration.isTrafficTypeSet()) {
    QString traffic_type = QString(configuration.getTrafficType().toUtf8());
    for (ActTrafficTypeConfiguration traffic_type_configuration :
         project.GetTrafficDesign().GetTrafficTypeConfigurationSetting()) {
      if (traffic_type_configuration.GetTrafficType() == traffic_type) {
        traffic_setting.SetTrafficTypeClass(traffic_type_configuration.GetTrafficClass());
      }
    }
  }

  if (configuration.isQoSTypeSet()) {
    switch (configuration.getQoSType()) {
      case MoxaClassBased::QoSEnumType::QoSEnumType_Bandwidth:
        traffic_setting.SetQosType(ActTrafficQosTypeEnum::kBandwidth);
        break;
      case MoxaClassBased::QoSEnumType::QoSEnumType_BoundedLatency:
        traffic_setting.SetQosType(ActTrafficQosTypeEnum::kBoundedLatency);
        break;
      case MoxaClassBased::QoSEnumType::QoSEnumType_Deadline:
        traffic_setting.SetQosType(ActTrafficQosTypeEnum::kDeadline);
        break;
    }
  }
  if (traffic_setting.GetQosType() == ActTrafficQosTypeEnum::kBoundedLatency && configuration.isReceiveOffsetMinSet()) {
    traffic_setting.SetMinReceiveOffset(qreal(configuration.getReceiveOffsetMin()));
  }
  if (traffic_setting.GetQosType() == ActTrafficQosTypeEnum::kBoundedLatency && configuration.isReceiveOffsetMaxSet()) {
    traffic_setting.SetMaxReceiveOffset(qreal(configuration.getReceiveOffsetMax()));
  }
  if (traffic_setting.GetQosType() == ActTrafficQosTypeEnum::kDeadline && configuration.isLatencyMaxSet()) {
    traffic_setting.SetMinReceiveOffset(0);
    traffic_setting.SetMaxReceiveOffset(qreal(configuration.getLatencyMax()));
  }

  // TSN
  if (configuration.isFRERModeSet()) {
    ActTrafficTSN& tsn = traffic_application.GetTSN();
    tsn.SetActive(true);
    tsn.SetFRER(configuration.getFRERMode());
  }

  // VLAN Setting
  ActTrafficVlanSetting& vlan_setting = traffic_application.GetVlanSetting();
  if (configuration.isTagStreamSet()) {
    vlan_setting.SetTagged(configuration.getTagStream());
  }

  if (configuration.isTagStreamVlanTagSet()) {
    vlan_setting.SetVlanId(quint16(configuration.getTagStreamVlanTag().getVlanId()));
    vlan_setting.SetPriorityCodePoint(quint8(configuration.getTagStreamVlanTag().getPriorityCodePoint()));
  }

  vlan_setting.SetUntaggedMode(ActTrafficUntaggedModeEnum::kPerStreamPriority);

  if (configuration.isPerStreamPrioritySet()) {
    vlan_setting.SetUserDefinedVlan(configuration.getPerStreamPriority().isVlanTagSet());

    if (configuration.getPerStreamPriority().isVlanTagSet()) {
      vlan_setting.SetVlanId(quint16(configuration.getPerStreamPriority().getVlanTag().getVlanId()));
      vlan_setting.SetPriorityCodePoint(
          quint8(configuration.getPerStreamPriority().getVlanTag().getPriorityCodePoint()));
    }

    switch (configuration.getPerStreamPriority().getPerStreamPriorityMode()) {
      case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L2:
        vlan_setting.SetType({ActStreamPriorityTypeEnum::kEthertype});
        if (configuration.getPerStreamPriority().isEtherTypeSet()) {
          vlan_setting.SetEtherType(quint32(configuration.getPerStreamPriority().getEtherType()));
        }
        if (configuration.getPerStreamPriority().isSubtypeEnableSet()) {
          vlan_setting.SetEnableSubType(configuration.getPerStreamPriority().getSubtypeEnable());
        }
        if (configuration.getPerStreamPriority().isSubtypeSet()) {
          vlan_setting.SetSubType(quint16(configuration.getPerStreamPriority().getSubtype()));
        }
        break;
      case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3TCP:
        vlan_setting.SetType({ActStreamPriorityTypeEnum::kTcp});
        if (configuration.getPerStreamPriority().isTCPPortSet()) {
          vlan_setting.SetTcpPort(qint32(configuration.getPerStreamPriority().getTCPPort()));
        }
        break;
      case MoxaClassBased::PerStreamPriorityModeEnumType::PerStreamPriorityModeEnumType_L3UDP:
        vlan_setting.SetType({ActStreamPriorityTypeEnum::kUdp});
        if (configuration.getPerStreamPriority().isUDPPortSet()) {
          vlan_setting.SetUdpPort(qint32(configuration.getPerStreamPriority().getUDPPort()));
        }
        break;
    }
  }

  // Stream Parameter
  ActTrafficStreamParameter& stream_parameter = traffic_application.GetStreamParameter();
  if (configuration.isTsnTrafficSpecificationSet()) {
    stream_parameter.SetInterval(qreal(configuration.getTsnTrafficSpecification().getInterval().getNumerator()) /
                                 qreal(configuration.getTsnTrafficSpecification().getInterval().getDenominator()));
    stream_parameter.SetMaxFrameSize(quint32(configuration.getTsnTrafficSpecification().getMaxFrameSize()));
    stream_parameter.SetMaxFramesPerInterval(
        quint32(configuration.getTsnTrafficSpecification().getMaxIntervalFrames()));
    stream_parameter.SetMaxBytesPerInterval(
        quint32(configuration.getTsnTrafficSpecification().getMaxBytesPerInterval()));
  }
  if (configuration.isTransmissionOffsetMinSet()) {
    stream_parameter.SetEarliestTransmitOffset(qreal(configuration.getTransmissionOffsetMin()));
  }
  if (configuration.isTransmissionOffsetMaxSet()) {
    stream_parameter.SetLatestTransmitOffset(qreal(configuration.getTransmissionOffsetMax()));
  }
  if (configuration.isTransmissionJitterSet()) {
    stream_parameter.SetJitter(qreal(configuration.getTransmissionJitter()));
  }

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.UpdateTrafficApplicationSetting(project_id, traffic_application);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}

UaStatus removeTsnStreamApplicationMethod(const UaNodeId& tsnStreamApplicationFolderNodeId,
                                          const UaNodeId& tsnStreamApplicationNodeId, OpcUa_UInt32& errorCode,
                                          UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(tsnStreamApplicationNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  UaReferenceDescriptions references;
  MoxaClassBased::TsnStreamApplicationType* pTsnStreamApplicationType = NULL;
  pMoxaNodeManager->getNodeReference(tsnStreamApplicationFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_TsnStreamApplicationType) {
      continue;
    }

    if (UaNodeId(references[idx].NodeId.NodeId) == tsnStreamApplicationNodeId) {
      pTsnStreamApplicationType =
          (MoxaClassBased::TsnStreamApplicationType*)pMoxaNodeManager->getNode(tsnStreamApplicationNodeId);
      break;
    }
  }

  if (pTsnStreamApplicationType == NULL) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: NodeId %1 is not found").arg(tsnStreamApplicationNodeId.toFullString());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  QMutexLocker lock(&act::core::g_core.mutex_);

  qint64 traffic_application_id = pMoxaNodeManager->getTsnStreamApplicationId(tsnStreamApplicationNodeId);
  act_status = act::core::g_core.DeleteTrafficApplicationSetting(project_id, traffic_application_id);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}
}  // namespace ClassBased