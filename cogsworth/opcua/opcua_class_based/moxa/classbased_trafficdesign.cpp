#include "classbased_trafficdesign.h"

#include "classbased_trafficconfiguration.h"
#include "classbased_tsnstream.h"
#include "classbased_tsnstreamapplication.h"

namespace ClassBased {

UaStatus updateTrafficDesignNodes(const ActProject& project, OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;

  const ActTrafficDesign& traffic_design = project.GetTrafficDesign();

  ret = updateTrafficConfigurationNodes(project, traffic_design, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateTsnStreamApplicationNodes(project, traffic_design, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateTsnStreamNodes(project, traffic_design, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  return ret;
}

UaStatus getAllStreamComputedResultMethod(const UaNodeId& nodeId,
                                          MoxaClassBased::StreamComputedResultDataTypes& streamResults,
                                          OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  streamResults.create(project.GetTrafficDesign().GetStreamSetting().size());
  int index = 0;
  for (ActTrafficStream traffic_stream : project.GetTrafficDesign().GetStreamSetting()) {
    MoxaClassBased::StreamComputedResultDataType streamResult;
    ret = getStreamComputedResultMethod(nodeId, UaString(traffic_stream.GetStreamName().toStdString().c_str()),
                                        streamResult, errorCode, errorMessage);
    if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
      return ret;
    }
    streamResult.copyTo(&streamResults[index++]);
  }

  return ret;
}

UaStatus getStreamComputedResultMethod(const UaNodeId& nodeId, const UaString& streamName,
                                       MoxaClassBased::StreamComputedResultDataType& streamResult,
                                       OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(nodeId);
  QString stream_name(streamName.toUtf8());

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  QMap<QString, ActRoutingResult> routing_result_map;
  for (ActRoutingResult routing_result : project.GetComputedResult().GetRoutingResults()) {
    routing_result_map.insert(routing_result.GetStreamName(), routing_result);
  }

  if (!routing_result_map.contains(stream_name)) {
    errorCode = M_UA_STREAM_NOT_EXIST;
    errorMessage = UaString("Invalid: Cannot find stream %1 in the computed result").arg(streamName);
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  streamResult.setStreamName(streamName);

  // Set VLAN tag
  ActRoutingResult routing_result = routing_result_map[stream_name];
  MoxaClassBased::IeeeTsnVlanTagDataType vlanTag;
  vlanTag.setVlanId(static_cast<OpcUa_UInt16>(routing_result.GetVlanId()));
  vlanTag.setPriorityCodePoint(static_cast<OpcUa_Byte>(routing_result.GetPriorityCodePoint()));
  streamResult.setVlanTag(vlanTag);

  for (ActTrafficStream traffic_stream : project.GetTrafficDesign().GetStreamSetting()) {
    if (traffic_stream.GetStreamName() == stream_name) {
      // Set traffic type
      for (ActTrafficApplication traffic_application : project.GetTrafficDesign().GetApplicationSetting()) {
        if (traffic_application.GetId() == traffic_stream.GetApplicationId()) {
          for (ActTrafficTypeConfiguration traffic_configuration :
               project.GetTrafficDesign().GetTrafficTypeConfigurationSetting()) {
            if (traffic_configuration.GetTrafficClass() ==
                traffic_application.GetTrafficSetting().GetTrafficTypeClass()) {
              streamResult.setTrafficType(UaString(traffic_configuration.GetTrafficType().toStdString().c_str()));
            }
          }
        }
      }

      // Set talker
      ActTrafficStreamInterface& traffic_stream_talker = traffic_stream.GetTalker();
      ActDevice talker_device;
      act_status = project.GetDeviceById(talker_device, traffic_stream_talker.GetDeviceId());
      if (!IsActStatusSuccess(act_status)) {
        errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
        errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
        qDebug() << errorMessage.toUtf8();
        return ret;
      }

      MoxaClassBased::TsnInterfaceConfigurationTalkerDataType talker;
      talker.setIpAddress(UaString(talker_device.GetIpv4().GetIpAddress().toStdString().c_str()));

      for (ActInterface intf : talker_device.GetInterfaces()) {
        if (intf.GetInterfaceId() == traffic_stream_talker.GetInterfaceId()) {
          talker.setInterfaceName(UaString(intf.GetInterfaceName().toStdString().c_str()));
        }
      }

      streamResult.setTalker(talker);

      // Set listeners
      int index = 0;
      MoxaClassBased::TsnInterfaceConfigurationListenerDataTypes listeners;
      listeners.create(OpcUa_UInt32(traffic_stream.GetListeners().size()));
      for (ActTrafficStreamInterface traffic_stream_listener : traffic_stream.GetListeners()) {
        ActDevice device;
        act_status = project.GetDeviceById(device, traffic_stream_listener.GetDeviceId());
        if (!IsActStatusSuccess(act_status)) {
          errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
          errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
          qDebug() << errorMessage.toUtf8();
          return ret;
        }

        MoxaClassBased::TsnInterfaceConfigurationListenerDataType listener;
        listener.setIpAddress(UaString(device.GetIpv4().GetIpAddress().toStdString().c_str()));

        for (ActInterface intf : device.GetInterfaces()) {
          if (intf.GetInterfaceId() == traffic_stream_listener.GetInterfaceId()) {
            listener.setInterfaceName(UaString(intf.GetInterfaceName().toStdString().c_str()));
          }
        }

        listener.copyTo(&listeners[index++]);
      }
      streamResult.setListeners(listeners);
    }
  }

  return ret;
}
}  // namespace ClassBased