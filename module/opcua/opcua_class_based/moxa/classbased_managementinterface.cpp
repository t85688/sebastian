#include "classbased_managementinterface.h"

namespace ClassBased {

static ACT_STATUS checkManagementInterfaceNode(ActManagementInterfaceTable& management_interface_table) {
  ACT_STATUS_INIT();

  if (management_interface_table.GetHttpService().GetPort() != 80 &&
      (management_interface_table.GetHttpService().GetPort() < 1024 ||
       management_interface_table.GetHttpService().GetPort() > 65535)) {
    QString error_msg = QString("Invalid: The valid range of HttpTcpPort is 1024 to 65535 or 80");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (management_interface_table.GetHttpsService().GetPort() != 443 &&
      (management_interface_table.GetHttpsService().GetPort() < 1024 ||
       management_interface_table.GetHttpsService().GetPort() > 65535)) {
    QString error_msg = QString("Invalid: The valid range of HttpsTcpPort is 1024 to 65535 or 443");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (management_interface_table.GetTelnetService().GetPort() != 23 &&
      (management_interface_table.GetTelnetService().GetPort() < 1024 ||
       management_interface_table.GetTelnetService().GetPort() > 65535)) {
    QString error_msg = QString("Invalid: The valid range of TelnetTcpPort is 1024 to 65535 or 23");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (management_interface_table.GetSSHService().GetPort() != 22 &&
      (management_interface_table.GetSSHService().GetPort() < 1024 ||
       management_interface_table.GetSSHService().GetPort() > 65535)) {
    QString error_msg = QString("Invalid: The valid range of SshTcpPort is 1024 to 65535 or 22");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (management_interface_table.GetSnmpService().GetPort() != 161 &&
      (management_interface_table.GetSnmpService().GetPort() < 1024 ||
       management_interface_table.GetSnmpService().GetPort() > 65535)) {
    QString error_msg = QString("Invalid: The valid range of SnmpPort is 1024 to 65535 or 161");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (management_interface_table.GetHttpMaxLoginSessions() < 1 ||
      management_interface_table.GetHttpMaxLoginSessions() > 10) {
    QString error_msg = QString("Invalid: The valid range of NumberOfHttpAndHttpsLoginSessions is 1 to 10");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (management_interface_table.GetTerminalMaxLoginSessions() < 1 ||
      management_interface_table.GetTerminalMaxLoginSessions() > 5) {
    QString error_msg = QString("Invalid: The valid range of NumberOfTelnetAndSshLoginSessions is 1 to 5");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

UaStatus updateManagementInterfaceNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                       UaString& errorMessage) {
  UaStatus ret;

  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetManagementInterface()) {
    return ret;
  }

  UaNodeId deviceNodeId = pMoxaNodeManager->getDeviceNodeId(project.GetId(), device.GetId());
  MoxaClassBased::BridgeType* pBridgeType = (MoxaClassBased::BridgeType*)pMoxaNodeManager->getNode(deviceNodeId);
  MoxaClassBased::ManagementInterfaceType* pManagementInterfaceType =
      pBridgeType->getDeviceConfig()->getManagementInterface();

  const ActManagementInterfaceTable& management_interface_table =
      project.GetDeviceConfig().GetManagementInterfaceTables()[device.GetId()];

  pManagementInterfaceType->setNumberOfHttpAndHttpsLoginSessions(
      OpcUa_UInt16(management_interface_table.GetHttpMaxLoginSessions()));
  pManagementInterfaceType->setNumberOfTelnetAndSshLoginSessions(
      OpcUa_UInt16(management_interface_table.GetTerminalMaxLoginSessions()));

  // HTTP service
  const ActMgmtHttpService& http_service = management_interface_table.GetHttpService();
  pManagementInterfaceType->setHttpActive(OpcUa_Boolean(http_service.GetEnable()));
  pManagementInterfaceType->setHttpTcpPort(OpcUa_UInt16(http_service.GetPort()));

  // HTTPs service
  const ActMgmtHttpsService& https_service = management_interface_table.GetHttpsService();
  pManagementInterfaceType->setHttpsActive(https_service.GetEnable());
  pManagementInterfaceType->setHttpsTcpPort(https_service.GetPort());

  // SNMP service
  const ActMgmtSnmpService& snmp_service = management_interface_table.GetSnmpService();
  pManagementInterfaceType->setSnmpPort(snmp_service.GetPort());
  switch (snmp_service.GetMode()) {
    case ActMgmtSnmpServiceModeEnum::kEnabled:
      pManagementInterfaceType->setSnmpActive(MoxaClassBased::ActiveSnmpEnumType::ActiveSnmpEnumType_Enabled);
      break;
    case ActMgmtSnmpServiceModeEnum::kDisabled:
      pManagementInterfaceType->setSnmpActive(MoxaClassBased::ActiveSnmpEnumType::ActiveSnmpEnumType_Disabled);
      break;
    case ActMgmtSnmpServiceModeEnum::kReadOnly:
      pManagementInterfaceType->setSnmpActive(MoxaClassBased::ActiveSnmpEnumType::ActiveSnmpEnumType_ReadOnly);
      break;
  }
  switch (snmp_service.GetTransportLayerProtocol()) {
    case 1:
      pManagementInterfaceType->setSnmpTransportProtocol(MoxaClassBased::TransportProtocol::TransportProtocol_UDP);
      break;
    case 2:
      pManagementInterfaceType->setSnmpTransportProtocol(MoxaClassBased::TransportProtocol::TransportProtocol_TCP);
      break;
  }

  // SSH service
  const ActMgmtSshService& ssh_service = management_interface_table.GetSSHService();
  pManagementInterfaceType->setSshActive(ssh_service.GetEnable());
  pManagementInterfaceType->setSshTcpPort(OpcUa_UInt16(ssh_service.GetPort()));

  // Telnet service
  const ActMgmtTelnetService& telnet_service = management_interface_table.GetTelnetService();
  pManagementInterfaceType->setTelnetActive(telnet_service.GetEnable());
  pManagementInterfaceType->setTelnetTcpPort(OpcUa_UInt16(telnet_service.GetPort()));

  return ret;
}

UaStatus setManagementInterfaceMethod(const UaNodeId& nodeId,
                                      const MoxaClassBased::ManagementInterfaceDataType& configuration,
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
  ActManagementInterfaceTable& management_interface_table = device_config.GetManagementInterfaceTables()[device_id];

  if (configuration.isNumberOfHttpAndHttpsLoginSessionsSet()) {
    management_interface_table.SetHttpMaxLoginSessions(quint16(configuration.getNumberOfHttpAndHttpsLoginSessions()));
  }
  if (configuration.isNumberOfTelnetAndSshLoginSessionsSet()) {
    management_interface_table.SetTerminalMaxLoginSessions(
        quint16(configuration.getNumberOfTelnetAndSshLoginSessions()));
  }

  // HTTP service
  ActMgmtHttpService& http_service = management_interface_table.GetHttpService();
  if (configuration.isHttpActiveSet()) {
    http_service.SetEnable(configuration.getHttpActive());
  }
  if (configuration.isHttpTcpPortSet()) {
    http_service.SetPort(quint16(configuration.getHttpTcpPort()));
  }

  // HTTPs service
  ActMgmtHttpsService& https_service = management_interface_table.GetHttpsService();
  if (configuration.isHttpsActiveSet()) {
    https_service.SetEnable(configuration.getHttpsActive());
  }
  if (configuration.isHttpsTcpPortSet()) {
    https_service.SetPort(quint16(configuration.getHttpsTcpPort()));
  }

  // SNMP service
  ActMgmtSnmpService& snmp_service = management_interface_table.GetSnmpService();
  if (configuration.isSnmpActiveSet()) {
    switch (configuration.getSnmpActive()) {
      case MoxaClassBased::ActiveSnmpEnumType::ActiveSnmpEnumType_Enabled:
        snmp_service.SetMode(ActMgmtSnmpServiceModeEnum::kEnabled);
        break;
      case MoxaClassBased::ActiveSnmpEnumType::ActiveSnmpEnumType_Disabled:
        snmp_service.SetMode(ActMgmtSnmpServiceModeEnum::kDisabled);
        break;
      case MoxaClassBased::ActiveSnmpEnumType::ActiveSnmpEnumType_ReadOnly:
        snmp_service.SetMode(ActMgmtSnmpServiceModeEnum::kReadOnly);
        break;
    }
  }
  if (configuration.isSnmpPortSet()) {
    snmp_service.SetPort(quint16(configuration.getSnmpPort()));
  }
  if (configuration.isSnmpTransportProtocolSet()) {
    switch (configuration.getSnmpTransportProtocol()) {
      case MoxaClassBased::TransportProtocol::TransportProtocol_UDP:
        snmp_service.SetTransportLayerProtocol(ActMgmtSnmpServiceTransLayerProtoEnum::kUDP);
        break;
      case MoxaClassBased::TransportProtocol::TransportProtocol_TCP:
        snmp_service.SetTransportLayerProtocol(ActMgmtSnmpServiceTransLayerProtoEnum::kTCP);
        break;
    }
  }

  // SSH service
  ActMgmtSshService& ssh_service = management_interface_table.GetSSHService();
  if (configuration.isSshActiveSet()) {
    ssh_service.SetEnable(configuration.getSshActive());
  }
  if (configuration.isSshTcpPortSet()) {
    ssh_service.SetPort(quint16(configuration.getSshTcpPort()));
  }

  // Telnet service
  ActMgmtTelnetService& telnet_service = management_interface_table.GetTelnetService();
  if (configuration.isTelnetActiveSet()) {
    telnet_service.SetEnable(configuration.getTelnetActive());
  }
  if (configuration.isTelnetTcpPortSet()) {
    telnet_service.SetPort(quint16(configuration.getTelnetTcpPort()));
  }

  act_status = checkManagementInterfaceNode(management_interface_table);
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