#include "classbased_projectsetting.h"

namespace ClassBased {

UaStatus updateProjectSettingNode(const ActProject& project, OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;

  const ActProjectSetting& projectSetting = project.GetProjectSetting();

  UaNodeId projectNodeId = pMoxaNodeManager->getProjectNodeId(project.GetId());
  MoxaClassBased::ProjectType* pProjectType = (MoxaClassBased::ProjectType*)pMoxaNodeManager->getNode(projectNodeId);
  MoxaClassBased::ProjectSettingType* pProjectSettingType = pProjectType->getProjectSetting();

  // Assign the input data
  pProjectSettingType->setProjectName(project.GetProjectName().toStdString().c_str());

  // Assign base ip setting
  MoxaClassBased::IpSettingType* pBaseIpSetting = pProjectSettingType->getBaseIpSetting();
  pBaseIpSetting->setIpAddress(projectSetting.GetProjectStartIp().GetIpAddress().toStdString().c_str());
  pBaseIpSetting->setSubnetMask(projectSetting.GetProjectStartIp().GetSubnetMask().toStdString().c_str());
  pBaseIpSetting->setGateway(projectSetting.GetProjectStartIp().GetGateway().toStdString().c_str());
  pBaseIpSetting->setDNS1(projectSetting.GetProjectStartIp().GetDNS1().toStdString().c_str());
  pBaseIpSetting->setDNS2(projectSetting.GetProjectStartIp().GetDNS2().toStdString().c_str());

  // Get Device Account
  MoxaClassBased::ConnectionAccountType* pConnectionAccount = pProjectSettingType->getConnectionAccount();
  pConnectionAccount->setUserName(UaString(projectSetting.GetAccount().GetUsername().toStdString().c_str()));

  // Get NETCONF
  MoxaClassBased::NETCONFType* pNETCONF = pProjectSettingType->getNETCONF();
  pNETCONF->setSSHPort(OpcUa_UInt16(projectSetting.GetNetconfConfiguration().GetNetconfOverSSH().GetSSHPort()));

  // Get SNMP
  MoxaClassBased::SNMPType* pSNMP = pProjectSettingType->getSNMP();
  pSNMP->setPort(OpcUa_UInt16(projectSetting.GetSnmpConfiguration().GetPort()));
  pSNMP->setVersion(pMoxaNodeManager->getSnmpVersion(projectSetting.GetSnmpConfiguration().GetVersion()));

  // Get RESTful
  MoxaClassBased::RESTfulType* pRESTful = pProjectSettingType->getRESTful();
  pRESTful->setPort(OpcUa_UInt16(projectSetting.GetRestfulConfiguration().GetPort()));

  errorCode = M_UA_NO_ERROR;
  errorMessage = UaString();
  return ret;
}

UaStatus setProjectSettingMethod(const UaNodeId& projectNodeId,
                                 const MoxaClassBased::ProjectSettingDataType& configuration, OpcUa_UInt32& errorCode,
                                 UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  qint64 project_id = pMoxaNodeManager->getProjectId(projectNodeId);

  ActProject project;
  act_status = act::core::g_core.GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ActProjectSetting project_setting = project.GetProjectSetting();

  if (configuration.isProjectNameSet()) {
    project_setting.SetProjectName(QString(configuration.getProjectName().toUtf8()));
  }

  // Set Base IP Setting
  ActIpv4& ipv4 = project_setting.GetProjectStartIp();
  MoxaClassBased::IpSettingDataType baseIpSetting = configuration.getBaseIpSetting();
  if (baseIpSetting.isIpAddressSet()) {
    ipv4.SetIpAddress(configuration.getBaseIpSetting().getIpAddress().toUtf8());
  }
  if (baseIpSetting.isSubnetMaskSet()) {
    ipv4.SetSubnetMask(configuration.getBaseIpSetting().getSubnetMask().toUtf8());
  }
  if (baseIpSetting.isGatewaySet()) {
    ipv4.SetGateway(configuration.getBaseIpSetting().getGateway().toUtf8());
  }
  if (baseIpSetting.isDNS1Set()) {
    ipv4.SetDNS1(configuration.getBaseIpSetting().getDNS1().toUtf8());
  }
  if (baseIpSetting.isDNS2Set()) {
    ipv4.SetDNS2(configuration.getBaseIpSetting().getDNS2().toUtf8());
  }

  // SetConnection Account
  ActDeviceAccount& device_account = project_setting.GetAccount();
  if (configuration.getConnectionAccount().isUserNameSet()) {
    device_account.SetUsername(configuration.getConnectionAccount().getUserName().toUtf8());
  }
  if (configuration.getConnectionAccount().isPasswordSet()) {
    device_account.SetPassword(configuration.getConnectionAccount().getPassword().toUtf8());
  }

  // Save NETCONF
  ActNetconfConfiguration& actNetconfConfiguration = project_setting.GetNetconfConfiguration();
  actNetconfConfiguration.SetTLS(false);
  ActNetconfOverSSH& actNetconfOverSSH = actNetconfConfiguration.GetNetconfOverSSH();
  if (configuration.getNETCONF().isSSHPortSet()) {
    actNetconfOverSSH.SetSSHPort(quint16(configuration.getNETCONF().getSSHPort()));
  }

  // Save SNMP
  ActSnmpConfiguration& actSnmpConfiguration = project_setting.GetSnmpConfiguration();
  if (configuration.getSNMP().isPortSet()) {
    actSnmpConfiguration.SetPort(configuration.getSNMP().getPort());
  }
  if (configuration.getSNMP().isReadCommunitySet()) {
    actSnmpConfiguration.SetReadCommunity(QString(configuration.getSNMP().getReadCommunity().toUtf8()));
  }
  if (configuration.getSNMP().isWriteCommunitySet()) {
    actSnmpConfiguration.SetWriteCommunity(QString(configuration.getSNMP().getWriteCommunity().toUtf8()));
  }
  if (configuration.getSNMP().isVersionSet()) {
    actSnmpConfiguration.SetVersion(pMoxaNodeManager->setSnmpVersion(configuration.getSNMP().getVersion()));
  }

  // Save RESTful
  ActRestfulConfiguration& actRestfulConfiguration = project_setting.GetRestfulConfiguration();
  if (configuration.getRESTful().isPortSet()) {
    actRestfulConfiguration.SetPort(quint16(configuration.getRESTful().getPort()));
  }

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.UpdateProjectSetting(project_id, project_setting);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}

}  // namespace ClassBased