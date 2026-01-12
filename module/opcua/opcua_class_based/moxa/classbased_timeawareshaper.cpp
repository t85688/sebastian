#include "classbased_timeawareshaper.h"

namespace ClassBased {

static ACT_STATUS checkTimeAwareShaperNode(ActDevice& device, ActGclTable& gcl_table) {
  ACT_STATUS_INIT();

  for (ActInterfaceGateParameters interfaces_gate_parameter : gcl_table.GetInterfacesGateParameters()) {
    ActGateParameters& gate_parameters = interfaces_gate_parameter.GetGateParameters();

    for (ActAdminControl& admin_control_list : gate_parameters.GetAdminControlList()) {
      ActSGSParams& sgs_params = admin_control_list.GetSgsParams();
      if (sgs_params.GetTimeIntervalValue() < ACT_PERIOD_MIN ||
          sgs_params.GetTimeIntervalValue() > device.GetDeviceProperty().GetGCLOffsetMaxDuration()) {
        QString error_msg = QString("Invalid: The valid range of Interval is from %1 to %2 nsec")
                                .arg(ACT_PERIOD_MIN)
                                .arg(device.GetDeviceProperty().GetGCLOffsetMaxDuration());
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }

    if (gate_parameters.GetAdminCycleTime().GetNumerator() < ACT_TIME_AWARE_MIN ||
        gate_parameters.GetAdminCycleTime().GetNumerator() > ACT_TIME_AWARE_MAX) {
      QString error_msg = QString("Invalid: The valid range of cycle time is from %1 to %2 nsec")
                              .arg(ACT_PERIOD_MIN)
                              .arg(device.GetDeviceProperty().GetGCLOffsetMaxDuration());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  return act_status;
}

UaStatus updateTimeAwareShaperNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                   UaString& errorMessage) {
  UaStatus ret;

  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTSN().GetIEEE802Dot1Qbv()) {
    return ret;
  }

  const ActGclTable& gcl_table = project.GetDeviceConfig().GetGCLTables()[device.GetId()];
  const QSet<ActInterfaceGateParameters>& interfaces_gate_parameters = gcl_table.GetInterfacesGateParameters();

  for (ActInterface intf : device.GetInterfaces()) {
    UaNodeId interfaceNodeId =
        pMoxaNodeManager->getInterfaceNodeId(project.GetId(), device.GetId(), intf.GetInterfaceId());
    MoxaClassBased::EthernetInterfaceType* pEthernetInterfaceType =
        (MoxaClassBased::EthernetInterfaceType*)pMoxaNodeManager->getNode(interfaceNodeId);
    MoxaClassBased::TimeAwareShaperType* pTimeAwareShaperType = pEthernetInterfaceType->getTimeAwareShaper();

    ActInterfaceGateParameters interfaces_gate_parameter(intf.GetInterfaceId());
    QSet<ActInterfaceGateParameters>::const_iterator interfaces_gate_parameter_iter =
        interfaces_gate_parameters.find(interfaces_gate_parameter);
    if (interfaces_gate_parameter_iter != interfaces_gate_parameters.end()) {
      interfaces_gate_parameter = *interfaces_gate_parameter_iter;

      ActGateParameters& gate_parameters = interfaces_gate_parameter.GetGateParameters();
      for (ActAdminControl admin_control : gate_parameters.GetAdminControlList()) {
        UaString browse_name = UaString("Slot%1").arg(int(admin_control.GetIndex()));
        UaReferenceDescriptions references;
        pMoxaNodeManager->getNodeReference(pTimeAwareShaperType->nodeId(), OpcUa_False, references);
        MoxaClassBased::GateControlType* pGateControlType = NULL;
        for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
          if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_GateControlType) {
            continue;
          }
          UaNode* pNode = pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);
          if (pNode->browseName().toString() == browse_name) {
            pGateControlType = (MoxaClassBased::GateControlType*)pNode;
            break;
          }
        }

        if (pGateControlType == NULL) {
          // NodeId for the node to create
          UaNodeId gateControlNodeId(UaString("%1.%2").arg(pTimeAwareShaperType->nodeId().toString()).arg(browse_name),
                                     pMoxaNodeManager->getNameSpaceIndex());

          // If succeeded, create the new node to the OPC UA nodemanager
          pGateControlType =
              new MoxaClassBased::GateControlType(gateControlNodeId, browse_name, pMoxaNodeManager->getNameSpaceIndex(),
                                                  pMoxaNodeManager->getNodeManagerConfig());
          if (!pGateControlType) {
            errorCode = M_UA_INTERNAL_ERROR;
            errorMessage = UaString("Invalid: Allocate memory failed");
            qDebug() << errorMessage.toUtf8();
            return ret;
          }

          ret = pMoxaNodeManager->addNodeAndReference(pTimeAwareShaperType->nodeId(),
                                                      pGateControlType->getUaReferenceLists(), OpcUaId_HasComponent);
          if (ret.isNotGood()) {
            errorCode = M_UA_INTERNAL_ERROR;
            errorMessage = UaString("Invalid: Create gate control node failed");
            qDebug() << errorMessage.toUtf8();
            return ret;
          }
        }

        pGateControlType->setIndex(OpcUa_Int64(admin_control.GetIndex()));
        pGateControlType->setInterval(OpcUa_Int64(admin_control.GetSgsParams().GetTimeIntervalValue()));
        quint8 gate_states_value = admin_control.GetSgsParams().GetGateStatesValue();
        pGateControlType->setQueue0(OpcUa_Boolean(gate_states_value & (1 << 0)));
        pGateControlType->setQueue1(OpcUa_Boolean(gate_states_value & (1 << 1)));
        pGateControlType->setQueue2(OpcUa_Boolean(gate_states_value & (1 << 2)));
        pGateControlType->setQueue3(OpcUa_Boolean(gate_states_value & (1 << 3)));
        pGateControlType->setQueue4(OpcUa_Boolean(gate_states_value & (1 << 4)));
        pGateControlType->setQueue5(OpcUa_Boolean(gate_states_value & (1 << 5)));
        pGateControlType->setQueue6(OpcUa_Boolean(gate_states_value & (1 << 6)));
        pGateControlType->setQueue7(OpcUa_Boolean(gate_states_value & (1 << 7)));
      }
    }

    UaReferenceDescriptions references;
    pMoxaNodeManager->getNodeReference(pTimeAwareShaperType->nodeId(), OpcUa_False, references);
    for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
      if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_GateControlType) {
        continue;
      }
      MoxaClassBased::GateControlType* pGateControlType =
          (MoxaClassBased::GateControlType*)pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);

      UaString browse_name = pGateControlType->browseName().toString();

      ActInterfaceGateParameters interfaces_gate_parameter(intf.GetInterfaceId());
      auto interfaces_gate_parameter_iter = interfaces_gate_parameters.find(interfaces_gate_parameter);
      if (interfaces_gate_parameter_iter == interfaces_gate_parameters.end()) {
        ret = pMoxaNodeManager->deleteUaNode(pGateControlType, OpcUa_True, OpcUa_True, OpcUa_True);
        if (ret.isNotGood()) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: Remove gate control %1 failed").arg(browse_name);
          qDebug() << errorMessage.toUtf8();
          return ret;
        }
        continue;
      }

      interfaces_gate_parameter = *interfaces_gate_parameter_iter;

      if (interfaces_gate_parameter.GetGateParameters().GetAdminControlList().isEmpty()) {
        ret = pMoxaNodeManager->deleteUaNode(pGateControlType, OpcUa_True, OpcUa_True, OpcUa_True);
        if (ret.isNotGood()) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: Remove gate control %1 failed").arg(browse_name);
          qDebug() << errorMessage.toUtf8();
          return ret;
        }
        continue;
      }

      ActAdminControl admin_control_list = interfaces_gate_parameter.GetGateParameters().GetAdminControlList().back();
      if (pGateControlType->getIndex() > OpcUa_Int64(admin_control_list.GetIndex())) {
        ret = pMoxaNodeManager->deleteUaNode(pGateControlType, OpcUa_True, OpcUa_True, OpcUa_True);
        if (ret.isNotGood()) {
          errorCode = M_UA_INTERNAL_ERROR;
          errorMessage = UaString("Invalid: Remove gate control %1 failed").arg(browse_name);
          qDebug() << errorMessage.toUtf8();
          return ret;
        }
      }
    }
  }

  return ret;
}

UaStatus insertTimeAwareShaperMethod(const UaNodeId& timeAwareShaperNodeId,
                                     const MoxaClassBased::GateControlDataType& configuration, UaNodeId& nodeId,
                                     OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(timeAwareShaperNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(timeAwareShaperNodeId);
  qint64 interface_id = pMoxaNodeManager->getInterfaceId(timeAwareShaperNodeId);

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

  if (!project.GetDeviceConfig().GetGCLTables().contains(device.GetId())) {
    project.GetDeviceConfig().GetGCLTables().insert(device.GetId(), ActGclTable(device.GetId()));
  }

  ActGclTable& gcl_table = project.GetDeviceConfig().GetGCLTables()[device_id];
  QSet<ActInterfaceGateParameters>& interfaces_gate_parameters = gcl_table.GetInterfacesGateParameters();
  ActInterfaceGateParameters interfaces_gate_parameter(interface_id);
  QSet<ActInterfaceGateParameters>::iterator interfaces_gate_parameter_iter =
      interfaces_gate_parameters.find(interfaces_gate_parameter);
  if (interfaces_gate_parameter_iter != interfaces_gate_parameters.end()) {
    interfaces_gate_parameter = *interfaces_gate_parameter_iter;
    interfaces_gate_parameters.erase(interfaces_gate_parameter_iter);
  }

  ActGateParameters& gate_parameters = interfaces_gate_parameter.GetGateParameters();
  QList<ActAdminControl>& admin_control_list = gate_parameters.GetAdminControlList();

  int insert_index = configuration.getIndex();
  quint32 index = ((insert_index == -1) || (insert_index > admin_control_list.length())) ? admin_control_list.length()
                                                                                         : insert_index;
  quint8 gate_state_value = 0;
  gate_state_value += (configuration.getQueue0()) ? 1 << 0 : 0;
  gate_state_value += (configuration.getQueue1()) ? 1 << 1 : 0;
  gate_state_value += (configuration.getQueue2()) ? 1 << 2 : 0;
  gate_state_value += (configuration.getQueue3()) ? 1 << 3 : 0;
  gate_state_value += (configuration.getQueue4()) ? 1 << 4 : 0;
  gate_state_value += (configuration.getQueue5()) ? 1 << 5 : 0;
  gate_state_value += (configuration.getQueue6()) ? 1 << 6 : 0;
  gate_state_value += (configuration.getQueue7()) ? 1 << 7 : 0;
  quint32 interval = 0;
  if (configuration.isIntervalSet()) {
    interval = quint32(configuration.getInterval());
  }

  ActAdminControl admin_control(index, "set-gate-states", ActSGSParams(gate_state_value, interval));

  quint32 cycle_time = interval;
  for (auto admin_control_iter = admin_control_list.rbegin(); admin_control_iter != admin_control_list.rend();
       ++admin_control_iter) {
    if (admin_control_iter->GetIndex() >= index) {
      admin_control_iter->SetIndex(admin_control_iter->GetIndex() + 1);
    }
    cycle_time += admin_control_iter->GetSgsParams().GetTimeIntervalValue();
  }
  admin_control_list.insert(index, admin_control);

  quint16 queue_count = device.GetDeviceProperty().GetNumberOfQueue();
  quint8 admin_gate_states_value = qPow(2, queue_count) - 1;

  gate_parameters.SetGateEnabled(true);
  gate_parameters.SetConfigChange(true);
  gate_parameters.SetAdminGateStates(admin_gate_states_value);
  gate_parameters.SetAdminBaseTime(project.GetCycleSetting().GetAdminBaseTime());
  gate_parameters.SetAdminCycleTime(ActAdminCycleTime(cycle_time, 1000000000));
  gate_parameters.SetAdminControlListLength(admin_control_list.size());

  interfaces_gate_parameters.insert(interfaces_gate_parameter);

  act_status = checkTimeAwareShaperNode(device, gcl_table);
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
  pMoxaNodeManager->getNodeReference(timeAwareShaperNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_GateControlType) {
      continue;
    }
    MoxaClassBased::GateControlType* pGateControlType =
        (MoxaClassBased::GateControlType*)pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);

    if (pGateControlType->getIndex() == index) {
      nodeId = pGateControlType->nodeId();
      break;
    }
  }

  return ret;
}

UaStatus setTimeAwareShaperMethod(const UaNodeId& timeAwareShaperNodeId,
                                  const MoxaClassBased::GateControlDataType& configuration, OpcUa_UInt32& errorCode,
                                  UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(timeAwareShaperNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(timeAwareShaperNodeId);
  qint64 interface_id = pMoxaNodeManager->getInterfaceId(timeAwareShaperNodeId);

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

  int index = configuration.getIndex();
  UaString browse_name = UaString("Slot%1").arg(index);

  UaReferenceDescriptions references;
  pMoxaNodeManager->getNodeReference(timeAwareShaperNodeId, OpcUa_False, references);
  MoxaClassBased::GateControlType* pGateControlType = NULL;
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_GateControlType) {
      continue;
    }

    UaNode* pNode = pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);

    if (pNode->browseName().toString() == browse_name) {
      pGateControlType = (MoxaClassBased::GateControlType*)pNode;
      break;
    }
  }

  if (pGateControlType == NULL) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Gate control index %1 is not found").arg(index);
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActGclTable& gcl_table = project.GetDeviceConfig().GetGCLTables()[device_id];
  QSet<ActInterfaceGateParameters>& interfaces_gate_parameters = gcl_table.GetInterfacesGateParameters();
  ActInterfaceGateParameters interfaces_gate_parameter(interface_id);
  QSet<ActInterfaceGateParameters>::iterator interfaces_gate_parameter_iter =
      interfaces_gate_parameters.find(interfaces_gate_parameter);
  if (interfaces_gate_parameter_iter == interfaces_gate_parameters.end()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Gate control index %1 is not found").arg(index);
    qDebug() << errorMessage.toUtf8();
    return ret;
  }
  interfaces_gate_parameter = *interfaces_gate_parameter_iter;
  interfaces_gate_parameters.erase(interfaces_gate_parameter_iter);

  ActGateParameters& gate_parameters = interfaces_gate_parameter.GetGateParameters();
  QList<ActAdminControl>& admin_control_list = gate_parameters.GetAdminControlList();

  quint8 gate_state_value = 0;
  gate_state_value += (configuration.getQueue0()) ? 1 << 0 : 0;
  gate_state_value += (configuration.getQueue1()) ? 1 << 1 : 0;
  gate_state_value += (configuration.getQueue2()) ? 1 << 2 : 0;
  gate_state_value += (configuration.getQueue3()) ? 1 << 3 : 0;
  gate_state_value += (configuration.getQueue4()) ? 1 << 4 : 0;
  gate_state_value += (configuration.getQueue5()) ? 1 << 5 : 0;
  gate_state_value += (configuration.getQueue6()) ? 1 << 6 : 0;
  gate_state_value += (configuration.getQueue7()) ? 1 << 7 : 0;

  quint32 cycle_time = 0;
  for (auto admin_control_iter = admin_control_list.rbegin(); admin_control_iter != admin_control_list.rend();
       ++admin_control_iter) {
    if (admin_control_iter->GetIndex() == quint32(index)) {
      if (configuration.isIntervalSet()) {
        admin_control_iter->GetSgsParams().SetTimeIntervalValue(quint32(configuration.getInterval()));
      }
      admin_control_iter->GetSgsParams().SetGateStatesValue(gate_state_value);
    }
    cycle_time += admin_control_iter->GetSgsParams().GetTimeIntervalValue();
  }

  gate_parameters.SetGateEnabled(true);
  gate_parameters.SetConfigChange(true);
  gate_parameters.SetAdminCycleTime(ActAdminCycleTime(cycle_time, 1000000000));

  interfaces_gate_parameters.insert(interfaces_gate_parameter);

  act_status = checkTimeAwareShaperNode(device, gcl_table);
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

UaStatus removeTimeAwareShaperMethod(const UaNodeId& timeAwareShaperNodeId, const OpcUa_Int64& index,
                                     OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(timeAwareShaperNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(timeAwareShaperNodeId);
  qint64 interface_id = pMoxaNodeManager->getInterfaceId(timeAwareShaperNodeId);

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

  int remove_index = int(index);
  UaString browse_name = UaString("Slot%1").arg(remove_index);

  UaReferenceDescriptions references;
  MoxaClassBased::GateControlType* pGateControlType = NULL;
  pMoxaNodeManager->getNodeReference(timeAwareShaperNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_GateControlType) {
      continue;
    }

    UaNode* pNode = pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);

    if (pNode->browseName().toString() == browse_name) {
      pGateControlType = (MoxaClassBased::GateControlType*)pNode;
      break;
    }
  }

  if (pGateControlType == NULL) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Gate control %1 is not found").arg(browse_name);
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActGclTable& gcl_table = project.GetDeviceConfig().GetGCLTables()[device_id];
  QSet<ActInterfaceGateParameters>& interfaces_gate_parameters = gcl_table.GetInterfacesGateParameters();
  ActInterfaceGateParameters interfaces_gate_parameter(interface_id);
  QSet<ActInterfaceGateParameters>::iterator interfaces_gate_parameter_iter =
      interfaces_gate_parameters.find(interfaces_gate_parameter);
  if (interfaces_gate_parameter_iter == interfaces_gate_parameters.end()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Gate control index %1 is not found").arg(remove_index);
    qDebug() << errorMessage.toUtf8();
    return ret;
  }
  interfaces_gate_parameter = *interfaces_gate_parameter_iter;
  interfaces_gate_parameters.erase(interfaces_gate_parameter_iter);

  ActGateParameters& gate_parameters = interfaces_gate_parameter.GetGateParameters();
  QList<ActAdminControl>& admin_control_list = gate_parameters.GetAdminControlList();

  qint64 cycle_time = 0;
  auto admin_control_iter = admin_control_list.begin();
  while (admin_control_iter != admin_control_list.end()) {
    if (admin_control_iter->GetIndex() == quint32(index)) {
      admin_control_iter = admin_control_list.erase(admin_control_iter);
      continue;
    } else if (admin_control_iter->GetIndex() > quint32(index)) {
      admin_control_iter->SetIndex(admin_control_iter->GetIndex() - 1);
    }
    cycle_time += admin_control_iter->GetSgsParams().GetTimeIntervalValue();
    ++admin_control_iter;
  }

  gate_parameters.SetGateEnabled(!admin_control_list.isEmpty());
  gate_parameters.SetConfigChange(true);
  gate_parameters.SetAdminCycleTime(ActAdminCycleTime(cycle_time, 1000000000));
  gate_parameters.SetAdminControlListLength(admin_control_list.size());

  interfaces_gate_parameters.insert(interfaces_gate_parameter);

  act_status = checkTimeAwareShaperNode(device, gcl_table);
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