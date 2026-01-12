#include "classbased_syslogserver.h"

namespace ClassBased {

static ACT_STATUS checkSyslogServerNode(ActSyslogSettingTable& syslog_setting_table) {
  ACT_STATUS_INIT();

  if (syslog_setting_table.GetPort1() < 1 || syslog_setting_table.GetPort1() > 65535) {
    QString error_msg = QString("Invalid: The valid range of Port1 is 1 to 65535");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (syslog_setting_table.GetPort2() < 1 || syslog_setting_table.GetPort2() > 65535) {
    QString error_msg = QString("Invalid: The valid range of Port2 is 1 to 65535");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (syslog_setting_table.GetPort3() < 1 || syslog_setting_table.GetPort3() > 65535) {
    QString error_msg = QString("Invalid: The valid range of Port3 is 1 to 65535");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if ((syslog_setting_table.GetAddress1() != QString() &&
       syslog_setting_table.GetAddress1() == syslog_setting_table.GetAddress2()) ||
      (syslog_setting_table.GetAddress2() != QString() &&
       syslog_setting_table.GetAddress2() == syslog_setting_table.GetAddress3()) ||
      (syslog_setting_table.GetAddress3() != QString() &&
       syslog_setting_table.GetAddress3() == syslog_setting_table.GetAddress1())) {
    QString error_msg = QString("Invalid: The syslog server address cannot duplicate");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

UaStatus updateSyslogServerNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                UaString& errorMessage) {
  UaStatus ret;

  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSyslogSetting()) {
    return ret;
  }

  UaNodeId deviceNodeId = pMoxaNodeManager->getDeviceNodeId(project.GetId(), device.GetId());
  MoxaClassBased::BridgeType* pBridgeType = (MoxaClassBased::BridgeType*)pMoxaNodeManager->getNode(deviceNodeId);
  MoxaClassBased::SyslogServerType* pSyslogServerType = pBridgeType->getDeviceConfig()->getSyslogServer();

  const ActSyslogSettingTable& syslog_setting_table =
      project.GetDeviceConfig().GetSyslogSettingTables()[device.GetId()];

  pSyslogServerType->setEnabled(syslog_setting_table.GetEnabled());
  pSyslogServerType->setSyslogServer1(syslog_setting_table.GetSyslogServer1());
  pSyslogServerType->setAddress1(syslog_setting_table.GetAddress1().toStdString().c_str());
  pSyslogServerType->setPort1(syslog_setting_table.GetPort1());
  pSyslogServerType->setSyslogServer2(syslog_setting_table.GetSyslogServer2());
  pSyslogServerType->setAddress2(syslog_setting_table.GetAddress2().toStdString().c_str());
  pSyslogServerType->setPort2(syslog_setting_table.GetPort2());
  pSyslogServerType->setSyslogServer3(syslog_setting_table.GetSyslogServer3());
  pSyslogServerType->setAddress3(syslog_setting_table.GetAddress3().toStdString().c_str());
  pSyslogServerType->setPort3(syslog_setting_table.GetPort3());

  return ret;
}

UaStatus setSyslogServerMethod(const UaNodeId& nodeId, const MoxaClassBased::SyslogServerDataType& configuration,
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
  ActSyslogSettingTable& syslog_setting_table = device_config.GetSyslogSettingTables()[device_id];

  if (configuration.isEnabledSet()) {
    syslog_setting_table.SetEnabled(configuration.getEnabled());
  }
  if (configuration.isSyslogServer1Set()) {
    syslog_setting_table.SetSyslogServer1(configuration.getSyslogServer1());
  }
  if (configuration.isAddress1Set()) {
    syslog_setting_table.SetAddress1(configuration.getAddress1().toUtf8());
  }
  if (configuration.isPort1Set()) {
    syslog_setting_table.SetPort1(configuration.getPort1());
  }
  if (configuration.isSyslogServer2Set()) {
    syslog_setting_table.SetSyslogServer2(configuration.getSyslogServer2());
  }
  if (configuration.isAddress2Set()) {
    syslog_setting_table.SetAddress2(configuration.getAddress2().toUtf8());
  }
  if (configuration.isPort2Set()) {
    syslog_setting_table.SetPort2(configuration.getPort2());
  }
  if (configuration.isSyslogServer3Set()) {
    syslog_setting_table.SetSyslogServer3(configuration.getSyslogServer3());
  }
  if (configuration.isAddress3Set()) {
    syslog_setting_table.SetAddress3(configuration.getAddress3().toUtf8());
  }
  if (configuration.isPort3Set()) {
    syslog_setting_table.SetPort3(configuration.getPort3());
  }

  act_status = checkSyslogServerNode(syslog_setting_table);
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