#include "act_southbound.hpp"

ACT_STATUS ActSouthbound::ConfigurationCheckErrorHandler(QString called_func, const ActDevice &device,
                                                         const QString &item) {
  QString error_msg = QString("This configuration is invalid(%1). Device:%2(%3)")
                          .arg(item)
                          .arg(device.GetIpv4().GetIpAddress())
                          .arg(device.GetId());

  return std::make_shared<ActStatusSouthboundFailed>(error_msg);
}

ACT_STATUS ActSouthbound::DeviceConnectionFalseErrorHandler(QString called_func, const ActDevice &device,
                                                            const ActConnectProtocolTypeEnum &protocol_enum) {
  QString error_msg = QString("Device %1 connection status is False. Device: %2(%3)")
                          .arg(kActConnectProtocolTypeEnumMap.key(protocol_enum))
                          .arg(device.GetIpv4().GetIpAddress())
                          .arg(device.GetId());
  qCritical() << called_func.toStdString().c_str() << error_msg.toStdString().c_str();
  return std::make_shared<ActStatusSouthboundFailed>(error_msg);
}

ACT_STATUS ActSouthbound::SetConfigFailProtocolErrorHandler(QString called_func, const ActDevice &device,
                                                            const ActConnectProtocolTypeEnum &protocol_enum,
                                                            const QString &item) {
  QString error_msg = QString("Set %1 failed. Device: %2(%3), Protocol: %4")
                          .arg(item)
                          .arg(device.GetIpv4().GetIpAddress())
                          .arg(device.GetId())
                          .arg(kActConnectProtocolTypeEnumMap.key(protocol_enum));

  qCritical() << called_func.toStdString().c_str() << error_msg.toStdString().c_str();
  return std::make_shared<ActStatusSetConfigFailed>(item, device.GetIpv4().GetIpAddress());
}

ACT_STATUS ActSouthbound::GetDataEmptyFailErrorHandler(QString called_func, const ActDevice &device,
                                                       const QString &item) {
  QString error_msg =
      QString("Get %1 data is empty. Device:%2(%3)").arg(item).arg(device.GetIpv4().GetIpAddress()).arg(device.GetId());

  qCritical() << called_func.toStdString().c_str() << error_msg.toStdString().c_str();
  return std::make_shared<ActStatusGetDeviceDataFailed>(item, device.GetIpv4().GetIpAddress());
}

ACT_STATUS ActSouthbound::GenerateDataFailErrorHandler(QString called_func, const ActDevice &device,
                                                       const QString &item) {
  QString error_msg =
      QString("Generate %1 failed. Device:%2(%3)").arg(item).arg(device.GetIpv4().GetIpAddress()).arg(device.GetId());

  qCritical() << called_func.toStdString().c_str() << error_msg.toStdString().c_str();
  return std::make_shared<ActStatusSouthboundFailed>(error_msg);
}

ACT_STATUS ActSouthbound::MethodsEmptyErrorHandler(QString called_func, const ActDevice &device, const QString &feat) {
  QString error_msg = QString("The %1 methods are Empty.\n Device: %2(%3)")
                          .arg(feat)
                          .arg(device.GetIpv4().GetIpAddress())
                          .arg(device.GetId());

  // qCritical() << called_func.toStdString().c_str() << error_msg.toStdString().c_str();
  return std::make_shared<ActStatusSouthboundFailed>(error_msg);
}

ACT_STATUS ActSouthbound::SequenceItemNotFoundErrorHandler(QString called_func, const QString &sequence_item,
                                                           const QString &feat) {
  QString not_found_elem = QString("Method(%1) by SequenceItem").arg(sequence_item);

  qCritical() << called_func.toStdString().c_str() << QString("The %1 is not found").arg(not_found_elem);
  return std::make_shared<ActStatusNotFound>(not_found_elem);
}

ACT_STATUS ActSouthbound::MethodNotImplementedErrorHandler(QString called_func, const ActDevice &device,
                                                           const QString &feat, const QString &method) {
  QString error_msg = QString("The method(%1) not implemented at %2 feature.\n Device: %3(%4)")
                          .arg(method)
                          .arg(feat)
                          .arg(device.GetIpv4().GetIpAddress())
                          .arg(device.GetId());

  qCritical() << called_func.toStdString().c_str() << error_msg.toStdString().c_str();

  return std::make_shared<ActStatusSouthboundFailed>(error_msg);
}

ACT_STATUS ActSouthbound::NotFoundProtocolErrorHandler(QString called_func, const ActDevice &device,
                                                       const QString &feat, const QString &method,
                                                       const QString &protocol) {
  QString error_msg = QString("The %1 protocol not found at %2 method of the %3 feature.\n Device: %4(%5)")
                          .arg(protocol)
                          .arg(method)
                          .arg(feat)
                          .arg(device.GetIpv4().GetIpAddress())
                          .arg(device.GetId());

  qCritical() << called_func.toStdString().c_str() << error_msg.toStdString().c_str();

  return std::make_shared<ActStatusSouthboundFailed>(error_msg);
}

ACT_STATUS ActSouthbound::ProbeFailErrorHandler(QString called_func, const ActDevice &device, const QString &feat) {
  QString error_msg = QString("Probe the %1 feature failed. Device:%2(%3)")
                          .arg(feat)
                          .arg(device.GetIpv4().GetIpAddress())
                          .arg(device.GetId());

  qCritical() << called_func.toStdString().c_str() << error_msg.toStdString().c_str();

  return std::make_shared<ActStatusSouthboundFailed>(error_msg);
}

ACT_STATUS ActSouthbound::AccessFailErrorHandler(QString called_func, const ActDevice &device, const QString &feat,
                                                 ACT_STATUS &act_status) {
  QString error_msg = act_status->GetErrorMessage();
  if (error_msg.isEmpty()) {
    error_msg = QString("Access the %1 failed. Device:%2(%3)")
                    .arg(feat)
                    .arg(device.GetIpv4().GetIpAddress())
                    .arg(device.GetId());

    qWarning() << called_func.toStdString().c_str() << error_msg.toStdString().c_str();
  }

  return std::make_shared<ActStatusSouthboundFailed>(error_msg);
}

ACT_STATUS ActSouthbound::NotFoundMethodErrorHandler(QString called_func, ActActionMethod method) {
  qCritical() << called_func.toStdString().c_str()
              << QString("Not found Method(%1): %2").arg(method.GetKey()).arg(method.ToString()).toStdString().c_str();

  return std::make_shared<ActStatusNotFound>(QString("Method(%1)").arg(method.GetKey()));
}
