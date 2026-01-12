#include "classbased_ipsetting.h"

namespace ClassBased {

UaStatus setIpSettingMethod(const UaNodeId& ipSettingNodeId, const MoxaClassBased::IpSettingDataType& configuration,
                            OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(ipSettingNodeId);
  qint64 device_id = pMoxaNodeManager->getDeviceId(ipSettingNodeId);

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

  ActIpv4& ipv4 = device.GetIpv4();
  if (configuration.isIpAddressSet()) {
    ipv4.SetIpAddress(configuration.getIpAddress().toUtf8());
  }
  if (configuration.isSubnetMaskSet()) {
    ipv4.SetSubnetMask(configuration.getSubnetMask().toUtf8());
  }
  if (configuration.isGatewaySet()) {
    ipv4.SetGateway(configuration.getGateway().toUtf8());
  }
  if (configuration.isDNS1Set()) {
    ipv4.SetDNS1(configuration.getDNS1().toUtf8());
  }
  if (configuration.isDNS2Set()) {
    ipv4.SetDNS2(configuration.getDNS2().toUtf8());
  }

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.UpdateDevice(project_id, device);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}

}  // namespace ClassBased