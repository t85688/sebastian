#include "act_auto_probe.hpp"

ACT_STATUS act::auto_probe::ActAutoProbe::DisconnectReturnHandler(const ActDevice &device,
                                                                  const ActFeatureEnum &feature,
                                                                  QList<ActFeatureWarning> &features_warning,
                                                                  ActAutoProbeWarning &result_probe_warning) {
  QString error_msg = QString("The device(%1) disconnection").arg(device.GetIpv4().GetIpAddress());

  ActFeatureWarning feat_warning(feature, error_msg);
  features_warning.append(feat_warning);
  result_probe_warning.SetWarningReason(features_warning);

  qDebug() << __func__ << error_msg.toStdString().c_str();
  return std::make_shared<ActStatusBase>(ActStatusType::kFailed, ActSeverity::kDebug);
}

ACT_STATUS act::auto_probe::ActAutoProbe::ProbeFeatures(const ActDevice &device, ActDeviceProfile &device_profile,
                                                        ActAutoProbeWarning &result_probe_warning) {
  ACT_STATUS_INIT();

  // Init Southbound's probe catch
  southbound_.InitProbeCache();

  // Set WarningReport
  result_probe_warning.SetModelName(device_profile.GetModelName());

  auto feature_capability_set = device_profile.GetFeatureCapability();
  QList<ActFeatureWarning> features_warning;
  ACT_STATUS act_status_find = std::make_shared<ActStatusBase>();

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Base feature
  ActFeatureProfile base_probe_feat_profile;
  ActFeatureWarning base_probe_feat_warning;
  BaseDetect(device, base_probe_feat_profile, base_probe_feat_warning);
  feature_capability_set.insert(base_probe_feat_profile);
  if (!base_probe_feat_warning.GetItems().isEmpty()) {  // has warning
    // Check connect
    act_status = southbound_.PingIpAddress(device.GetIpv4().GetIpAddress(), ACT_PING_REPEAT_TIMES);
    if (!IsActStatusSuccess(act_status)) {
      return DisconnectReturnHandler(device, ActFeatureEnum::kBase, features_warning, result_probe_warning);
    }
    features_warning.append(base_probe_feat_warning);
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // AutoScan feature
  ActFeatureProfile auto_scan_probe_feat_profile;
  ActFeatureWarning auto_scan_probe_feat_warning;
  AutoScanDetect(device, auto_scan_probe_feat_profile, auto_scan_probe_feat_warning);
  feature_capability_set.insert(auto_scan_probe_feat_profile);
  if (!auto_scan_probe_feat_warning.GetItems().isEmpty()) {  // has warning
    // Check connect
    act_status = southbound_.PingIpAddress(device.GetIpv4().GetIpAddress(), ACT_PING_REPEAT_TIMES);
    if (!IsActStatusSuccess(act_status)) {
      return DisconnectReturnHandler(device, ActFeatureEnum::kAutoScan, features_warning, result_probe_warning);
    }
    features_warning.append(auto_scan_probe_feat_warning);
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Operation feature
  ActFeatureProfile operation_probe_feat_profile;
  ActFeatureWarning operation_probe_feat_warning;
  OperationDetect(device, operation_probe_feat_profile, operation_probe_feat_warning);
  feature_capability_set.insert(operation_probe_feat_profile);
  if (!operation_probe_feat_warning.GetItems().isEmpty()) {  // has warning
    // Check connect
    act_status = southbound_.PingIpAddress(device.GetIpv4().GetIpAddress(), ACT_PING_REPEAT_TIMES);
    if (!IsActStatusSuccess(act_status)) {
      return DisconnectReturnHandler(device, ActFeatureEnum::kOperation, features_warning, result_probe_warning);
    }
    features_warning.append(operation_probe_feat_warning);
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Configuration feature
  ActFeatureProfile configuration_probe_feat_profile;
  ActFeatureWarning configuration_probe_feat_warning;
  ConfigurationDetect(device, configuration_probe_feat_profile, configuration_probe_feat_warning);
  feature_capability_set.insert(configuration_probe_feat_profile);
  if (!configuration_probe_feat_warning.GetItems().isEmpty()) {  // has warning
    // Check connect
    act_status = southbound_.PingIpAddress(device.GetIpv4().GetIpAddress(), ACT_PING_REPEAT_TIMES);
    if (!IsActStatusSuccess(act_status)) {
      return DisconnectReturnHandler(device, ActFeatureEnum::kConfiguration, features_warning, result_probe_warning);
    }
    features_warning.append(configuration_probe_feat_warning);
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Set WarningReport
  if (!features_warning.isEmpty()) {
    result_probe_warning.SetWarningReason(features_warning);
  }

  // Set FeatureCapability to device_profile
  device_profile.SetFeatureCapability(feature_capability_set);
  return act_status;
}

ACT_STATUS act::auto_probe::ActAutoProbe::AppendAction(
    const ActFeatureEnum &feature_enum, const QString &item_key, const QString &sub_item_key, const ActDevice &device,
    ActFeatureProfile &result_feature_profile, ActFeatureWarning &result_feature_warning,
    ACT_STATUS (ActSouthbound::*probe_func)(const ActDevice &, const ActFeatureSubItem &, ActFeatureSubItem &,
                                            ActFeatureSubItemWarning &)) {
  ACT_STATUS_INIT();

  // Get sub-item at the feature_profiles_
  ActFeatureSubItem sub_item_elem;
  ActFeatureSubItem result_sub_item_elem;
  ActFeatureSubItemWarning result_sub_item_warning;
  act_status = GetFeatureProfileFeatureSubItem(profiles_.GetFeatureProfiles(), feature_enum, item_key, sub_item_key,
                                               sub_item_elem);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetFeatureProfileFeatureSubItem() failed";
    // Append to warning report
    if (!result_feature_warning.GetItems().contains(item_key)) {
      result_feature_warning.GetItems()[item_key] = ActFeatureItemWarning();
    }
    result_feature_warning.GetItems()[item_key].GetSubItems()[sub_item_key] =
        ActFeatureSubItemWarning(act_status->GetErrorMessage());

    return act_status;
  }

  // Call southbound probe function
  act_status = (southbound_.*probe_func)(device, sub_item_elem, result_sub_item_elem, result_sub_item_warning);
  if (!IsActStatusSuccess(act_status)) {  // probe failed
    // Append to warning report
    if (!result_feature_warning.GetItems().contains(item_key)) {
      result_feature_warning.GetItems()[item_key] = ActFeatureItemWarning();
    }
    result_feature_warning.GetItems()[item_key].GetSubItems()[sub_item_key] = result_sub_item_warning;
  }

  // Append to result_feature_profile
  if (!result_feature_profile.GetItems().contains(item_key)) {
    result_feature_profile.GetItems()[item_key] = ActFeatureItem();
  }
  result_feature_profile.GetItems()[item_key].GetSubItems()[sub_item_key] = result_sub_item_elem;

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS act::auto_probe::ActAutoProbe::BaseDetect(const ActDevice &device, ActFeatureProfile &result_feature_profile,
                                                     ActFeatureWarning &result_feature_warning) {
  ACT_STATUS_INIT();
  auto feat_enum = ActFeatureEnum::kBase;
  auto feat_str = kActFeatureEnumMap.key(feat_enum);

  // Init Result
  result_feature_profile = ActFeatureProfile(feat_enum);
  result_feature_warning = ActFeatureWarning(feat_enum);

  // Probe CheckConnection > SNMP
  act_status = AppendAction(feat_enum, "CheckConnection", "SNMP", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionCheckSnmpConnect);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__
            << QString("Probe CheckConnection(SNMP) failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe CheckConnection > RESTful
  act_status = AppendAction(feat_enum, "CheckConnection", "RESTful", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionCheckRestfulConnect);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__
            << QString("Probe CheckConnection(RESTful) failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // qInfo() << __func__
  //         << QString("result_feature_profile: %1").arg(result_feature_profile.ToString()).toStdString().c_str();

  return act_status;
}

ACT_STATUS act::auto_probe::ActAutoProbe::AutoScanDetect(const ActDevice &device,
                                                         ActFeatureProfile &result_feature_profile,
                                                         ActFeatureWarning &result_feature_warning) {
  ACT_STATUS_INIT();
  auto feat_enum = ActFeatureEnum::kAutoScan;
  auto feat_str = kActFeatureEnumMap.key(feat_enum);

  // Init Result
  result_feature_profile = ActFeatureProfile(feat_enum);
  result_feature_warning = ActFeatureWarning(feat_enum);

  // Probe BroadcastSearch > Basic
  act_status = AppendAction(feat_enum, "BroadcastSearch", "Basic", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionBroadcastSearch);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe BroadcastSearch failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe Identify > ModelName
  act_status = AppendAction(feat_enum, "Identify", "ModelName", device, result_feature_profile, result_feature_warning,
                            &ActSouthbound::ProbeActionModelName);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe ModelName failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe Identify > VendorId
  act_status = AppendAction(feat_enum, "Identify", "VendorID", device, result_feature_profile, result_feature_warning,
                            &ActSouthbound::ProbeActionVendorId);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe VendorID failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe Identify > FirmwareVersion
  act_status = AppendAction(feat_enum, "Identify", "FirmwareVersion", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionFirmwareVersion);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe FirmwareVersion failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe LLDP
  act_status = AppendAction(feat_enum, "LLDP", "Basic", device, result_feature_profile, result_feature_warning,
                            &ActSouthbound::ProbeActionLLDP);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe LLDP failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe DeviceInformation > DeviceName
  act_status = AppendAction(feat_enum, "DeviceInformation", "DeviceName", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionDeviceName);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe DeviceName failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe DeviceInformation > MACTable
  act_status = AppendAction(feat_enum, "DeviceInformation", "MACTable", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionMacTable);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe MACTable failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe DeviceInformation > InterfaceName
  act_status = AppendAction(feat_enum, "DeviceInformation", "InterfaceName", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionInterfaceName);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe InterfaceName failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // TODO: remove
  // [feat:3661] Remove fake MAC
  // // Probe DeviceInformation > InterfaceMAC
  // act_status = AppendAction(feat_enum, "DeviceInformation", "InterfaceMAC", device, result_feature_profile,
  //                           result_feature_warning, &ActSouthbound::ProbeActionInterfaceMac);
  // if (!IsActStatusSuccess(act_status)) {
  //   qInfo() << __func__ << QString("Probe InterfaceMAC failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  // }

  // Probe DeviceInformation > PortSpeed
  act_status = AppendAction(feat_enum, "DeviceInformation", "PortSpeed", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionPortSpeed);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe PortSpeed failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  if (!result_feature_warning.GetItems().isEmpty()) {  // print warnings
    qInfo() << __func__
            << QString("result_feature_warning: %1").arg(result_feature_warning.ToString()).toStdString().c_str();
  }

  // qInfo() << __func__
  //         << QString("result_feature_profile: %1").arg(result_feature_profile.ToString()).toStdString().c_str();

  return act_status;
}

ACT_STATUS act::auto_probe::ActAutoProbe::OperationDetect(const ActDevice &device,
                                                          ActFeatureProfile &result_feature_profile,
                                                          ActFeatureWarning &result_feature_warning) {
  ACT_STATUS_INIT();
  auto feat_enum = ActFeatureEnum::kOperation;
  auto feat_str = kActFeatureEnumMap.key(feat_enum);

  // Init Result
  result_feature_profile = ActFeatureProfile(feat_enum);
  result_feature_warning = ActFeatureWarning(feat_enum);

  // Probe Reboot
  act_status = AppendAction(feat_enum, "Reboot", "Basic", device, result_feature_profile, result_feature_warning,
                            &ActSouthbound::ProbeActionReboot);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe Reboot failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe FactoryDefault
  act_status = AppendAction(feat_enum, "FactoryDefault", "Basic", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionFactoryDefault);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe FactoryDefault failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe FirmwareUpgrade
  act_status = AppendAction(feat_enum, "FirmwareUpgrade", "Basic", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionFirmwareUpgrade);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe FirmwareUpgrade failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe EnableSNMPService
  act_status = AppendAction(feat_enum, "EnableSNMPService", "Basic", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionEnableSnmpService);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe EnableSNMPService failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  if (!result_feature_warning.GetItems().isEmpty()) {  // print warnings
    qInfo() << __func__
            << QString("result_feature_warning: %1").arg(result_feature_warning.ToString()).toStdString().c_str();
  }

  // qInfo() << __func__
  //         << QString("result_feature_profile: %1").arg(result_feature_profile.ToString()).toStdString().c_str();

  return act_status;
}

ACT_STATUS act::auto_probe::ActAutoProbe::ConfigurationDetect(const ActDevice &device,
                                                              ActFeatureProfile &result_feature_profile,
                                                              ActFeatureWarning &result_feature_warning) {
  ACT_STATUS_INIT();
  auto feat_enum = ActFeatureEnum::kConfiguration;
  auto feat_str = kActFeatureEnumMap.key(feat_enum);

  // Init Result
  result_feature_profile = ActFeatureProfile(feat_enum);
  result_feature_warning = ActFeatureWarning(feat_enum);

  // Probe NetworkSetting
  act_status = AppendAction(feat_enum, "NetworkSetting", "Basic", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionNetworkSetting);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe NetworkSetting failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe VLANSetting > VLANMethod
  act_status = AppendAction(feat_enum, "VLANSetting", "VLANMethod", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionVLAN);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe VLANMethod failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe VLANSetting > AccessTrunkMode
  act_status = AppendAction(feat_enum, "VLANSetting", "AccessTrunkMode", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionVLANPortType);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe AccessTrunkMode failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe VLANSetting > TEMSTID
  act_status = AppendAction(feat_enum, "VLANSetting", "TEMSTID", device, result_feature_profile, result_feature_warning,
                            &ActSouthbound::ProbeActionTEMSTID);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe TEMSTID failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe VLANSetting > DefaultPVID
  act_status = AppendAction(feat_enum, "VLANSetting", "DefaultPVID", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionPortPVID);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe DefaultPVID failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe VLANSetting > DefaultPCP
  act_status = AppendAction(feat_enum, "VLANSetting", "DefaultPCP", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionPortDefaultPCP);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe DefaultPCP failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe VLANSetting > PerStreamPriority
  act_status = AppendAction(feat_enum, "VLANSetting", "PerStreamPriority", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionStreamPriority);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe PerStreamPriority failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe StaticForwardSetting > Unicast
  act_status = AppendAction(feat_enum, "StaticForwardSetting", "Unicast", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionStaticUnicast);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__
            << QString("Probe StaticForward(Unicast) failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe StaticForwardSetting > Multicast
  act_status = AppendAction(feat_enum, "StaticForwardSetting", "Multicast", device, result_feature_profile,
                            result_feature_warning, &ActSouthbound::ProbeActionStaticMulticast);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__
            << QString("Probe StaticForward(Multicast) failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe STPRSTP > RSTPMethod
  act_status = AppendAction(feat_enum, "STPRSTP", "RSTPMethod", device, result_feature_profile, result_feature_warning,
                            &ActSouthbound::ProbeActionSpanningTreeMethod);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe RSTPMethod failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe STPRSTP > HelloTime
  act_status = AppendAction(feat_enum, "STPRSTP", "HelloTime", device, result_feature_profile, result_feature_warning,
                            &ActSouthbound::ProbeActionSpanningTreeHelloTime);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe STPRSTP(HelloTime) failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe STPRSTP > Priority
  act_status = AppendAction(feat_enum, "STPRSTP", "Priority", device, result_feature_profile, result_feature_warning,
                            &ActSouthbound::ProbeActionSpanningTreePriority);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe STPRSTP(Priority) failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe STPRSTP > RootGuard
  act_status = AppendAction(feat_enum, "STPRSTP", "RootGuard", device, result_feature_profile, result_feature_warning,
                            &ActSouthbound::ProbeActionSpanningTreeRootGuard);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe STPRSTP(RootGuard) failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe TSN > 802.1Qbv (Time-aware Shaper)
  act_status = AppendAction(feat_enum, "TSN", "IEEE802Dot1Qbv", device, result_feature_profile, result_feature_warning,
                            &ActSouthbound::ProbeAction802Dot1Qbv);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe IEEE802Dot1Qbv failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  // Probe TSN > 802.1CB
  act_status = AppendAction(feat_enum, "TSN", "IEEE802Dot1CB", device, result_feature_profile, result_feature_warning,
                            &ActSouthbound::ProbeAction802Dot1CB);
  if (!IsActStatusSuccess(act_status)) {
    qInfo() << __func__ << QString("Probe IEEE802Dot1CB failed. Device: %1").arg(device.GetIpv4().GetIpAddress());
  }

  if (!result_feature_warning.GetItems().isEmpty()) {  // print warnings
    qInfo() << __func__
            << QString("result_feature_warning: %1").arg(result_feature_warning.ToString()).toStdString().c_str();
  }

  // qInfo() << __func__
  //         << QString("result_feature_profile: %1").arg(result_feature_profile.ToString()).toStdString().c_str();

  return act_status;
}
