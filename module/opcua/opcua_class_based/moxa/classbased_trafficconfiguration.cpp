#include "classbased_trafficconfiguration.h"

namespace ClassBased {

UaStatus updateTrafficConfigurationNodes(const ActProject& project, const ActTrafficDesign& traffic_design,
                                         OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;

  UaNodeId projectNodeId = pMoxaNodeManager->getProjectNodeId(project.GetId());
  MoxaClassBased::ProjectType* pProjectType = (MoxaClassBased::ProjectType*)pMoxaNodeManager->getNode(projectNodeId);
  MoxaClassBased::TrafficConfigurationTableType* pTrafficConfigurationTableType =
      pProjectType->getTrafficDesign()->getTrafficConfigurationTable();

  MoxaClassBased::TrafficConfigurationEntryDataTypes trafficConfigurationEntries;
  pTrafficConfigurationTableType->getTrafficConfigurationEntries(trafficConfigurationEntries);

  if (trafficConfigurationEntries.length() == 0) {
    trafficConfigurationEntries.create(8);
  }

  OpcUa_UInt32 index = 0;
  for (ActTrafficTypeConfiguration configuration : traffic_design.GetTrafficTypeConfigurationSetting()) {
    MoxaClassBased::TrafficConfigurationEntryDataType trafficConfigurationEntry;

    trafficConfigurationEntry.setIndex(OpcUa_Byte(index));
    UaString trafficType = UaString(configuration.GetTrafficType().toStdString().c_str());
    trafficConfigurationEntry.setTrafficType(trafficType);
    UaByteArray pcp;
    pcp.create(configuration.GetPriorityCodePointSet().size());
    int i = 0;
    for (quint8 priority_code_point : configuration.GetPriorityCodePointSet()) {
      pcp[i++] = (char)priority_code_point;
    }
    trafficConfigurationEntry.setPCP(pcp);
    trafficConfigurationEntry.setReservedTime(OpcUa_Double(configuration.GetReservedTime()));

    trafficConfigurationEntry.copyTo(&trafficConfigurationEntries[index]);

    index++;
  }
  pTrafficConfigurationTableType->setTrafficConfigurationEntries(trafficConfigurationEntries);

  return ret;
}

UaStatus setTrafficConfigurationEntryMethod(const UaNodeId& trafficConfigurationTableNodeId,
                                            const MoxaClassBased::TrafficConfigurationEntryDataType configuration,
                                            OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(trafficConfigurationTableNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  QList<ActTrafficTypeConfiguration>& traffic_type_configuration_setting =
      project.GetTrafficDesign().GetTrafficTypeConfigurationSetting();

  int index = int(configuration.getIndex());
  if (index >= traffic_type_configuration_setting.size()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Index %1 is unavailable").arg(index);
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActTrafficTypeConfiguration& traffic_type_configuration = traffic_type_configuration_setting[index];

  if (configuration.isTrafficTypeSet()) {
    traffic_type_configuration.SetTrafficType(configuration.getTrafficType().toUtf8());
  }

  if (configuration.isPCPSet()) {
    UaByteArray pcp;
    configuration.getPCP(pcp);

    if (pcp.size() == 0) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: PCP cannot be empty");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    QSet<quint8> priority_code_point_set;
    for (int i = 0; i < pcp.size(); i++) {
      quint8 priority_code_point = quint8(pcp[i]);
      if (priority_code_point > 7) {
        errorCode = M_UA_INTERNAL_ERROR;
        errorMessage = UaString("Invalid: The valid range of PCP is 0 to 7");
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
      priority_code_point_set.insert(quint8(pcp[i]));
    }
    traffic_type_configuration.SetPriorityCodePointSet(priority_code_point_set);
  }

  if (configuration.isReservedTimeSet()) {
    traffic_type_configuration.SetReservedTime(configuration.getReservedTime());
  }

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.UpdateTrafficTypeConfigurationSetting(project_id, traffic_type_configuration);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}
}  // namespace ClassBased