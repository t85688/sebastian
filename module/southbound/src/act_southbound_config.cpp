
#include "act_southbound.hpp"
#include "act_utilities.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

ACT_STATUS ActSouthbound::ConfigureNetworkSetting(const ActDevice &device,
                                                  const ActNetworkSettingTable &network_setting_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  // Set NetworkSetting
  // Get Sub-item
  ActFeatureSubItem feature_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "NetworkSetting", "Basic", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetDeviceFeatureSubItem() failed";
    return act_status;
  }

  act_status = ActionSetNetworkSetting(device, feature_sub_item, network_setting_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "NetworkSetting failed.";
    return act_status;
  }

  act_status = ClearArpCache();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ClearArpCache() failed.";
    return act_status;
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Check icmp status
  SLEEP_MS(1000);
  act_status = PingIpAddress(network_setting_table.GetIpAddress(), ACT_PING_REPEAT_TIMES);
  if (!IsActStatusSuccess(act_status)) {  // not alive
    qCritical() << __func__ << "Check the New NetworkSetting connect failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureVlan(const ActDevice &device, const ActVlanTable &vlan_config_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  qint64 start = QDateTime::currentMSecsSinceEpoch();
  qint64 end;

  QSet<qint32> vlan_id_set;
  for (auto entry : vlan_config_table.GetVlanStaticEntries()) {
    vlan_id_set.insert(entry.GetVlanId());
  }

  auto dev_feat_group_cfg = device.GetDeviceProperty().GetFeatureGroup().GetConfiguration();

  // Get Feature Property
  ActFeatureSubItem vlan_sub_item;
  ActFeatureSubItem vlan_port_type_sub_item;
  ActFeatureSubItem port_pvid_sub_item;
  ActFeatureSubItem mgmt_vlan_sub_item;
  ActFeatureSubItem static_fwd_uni_sub_item;
  ActFeatureSubItem static_fwd_mul_sub_item;

  // VLANMethod
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "VLANSetting", "VLANMethod", vlan_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // AccessTrunkMode
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "VLANSetting", "AccessTrunkMode",
                                       vlan_port_type_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  //  DefaultPVID
  if (dev_feat_group_cfg.GetVLANSetting().GetDefaultPVID()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "VLANSetting", "DefaultPVID", port_pvid_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Management VLAN
  if (dev_feat_group_cfg.GetVLANSetting().GetManagementVLAN()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "VLANSetting", "ManagementVLAN", mgmt_vlan_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  //  StaticForwardSetting > Unicast
  if (dev_feat_group_cfg.GetStaticForwardSetting().GetUnicast()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "StaticForwardSetting", "Unicast",
                                         static_fwd_uni_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  //  StaticForwardSetting > Multicast
  if (dev_feat_group_cfg.GetStaticForwardSetting().GetMulticast()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "StaticForwardSetting", "Multicast",
                                         static_fwd_mul_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Check VLAN method use the NETCONF or not
  ActConnectProtocolTypeEnum method_protocol = ActConnectProtocolTypeEnum::kEmpty;
  if (vlan_sub_item.GetMethods().begin().value().GetProtocols().contains("NETCONF")) {
    method_protocol = ActConnectProtocolTypeEnum::kNETCONF;
  } else if (vlan_sub_item.GetMethods().begin().value().GetProtocols().contains("RESTful")) {
    method_protocol = ActConnectProtocolTypeEnum::kRESTful;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  end = QDateTime::currentMSecsSinceEpoch();
  qDebug() << __func__ << "Get Feature Property duration:" << end - start << "ms";

  // Get RemoveStaticForwardTable(Unicast)
  ActStaticForwardTable uni_remove_static_forward_table;
  if (!static_fwd_uni_sub_item.GetMethods().isEmpty()) {
    start = QDateTime::currentMSecsSinceEpoch();
    act_status =
        GenerateRemoveStaticForwardTableByVlanTable(device, vlan_config_table, true, uni_remove_static_forward_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Generate RemoveStaticForwardTableByVlans(Unicast) failed.";
      return act_status;
    }

    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __func__ << "Generate RemoveStaticForwardTableByVlans(Unicast) duration:" << end - start << "ms";
  }

  // Get RemoveStaticForwardTable(Multicast)

  ActStaticForwardTable mul_remove_static_forward_table;
  if (!static_fwd_mul_sub_item.GetMethods().isEmpty()) {
    start = QDateTime::currentMSecsSinceEpoch();
    act_status =
        GenerateRemoveStaticForwardTableByVlanTable(device, vlan_config_table, false, mul_remove_static_forward_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Generate RemoveStaticForwardTableByVlans(Multicast) failed.";
      return act_status;
    }

    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __func__ << "Generate RemoveStaticForwardTableByVlans(Multicast) duration:" << end - start << "ms";
  }

  // Remove StaticForward(Unicast) entry
  if (!static_fwd_uni_sub_item.GetMethods().isEmpty()) {
    start = QDateTime::currentMSecsSinceEpoch();
    act_status = ActionSetStaticUnicast(device, static_fwd_uni_sub_item, uni_remove_static_forward_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Delete StaticForward(Unicast) entry failed.";
      return act_status;
    }

    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __func__ << "Delete StaticForward(Unicast) entry duration:" << end - start << "ms";
  }

  // Remove StaticForward(Multicast) entry
  if (!static_fwd_mul_sub_item.GetMethods().isEmpty()) {
    start = QDateTime::currentMSecsSinceEpoch();
    act_status = ActionSetStaticMulticast(device, static_fwd_mul_sub_item, mul_remove_static_forward_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Delete StaticForward(Multicast) entry failed.";
      return act_status;
    }

    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __func__ << "Delete StaticForward(Multicast) entry duration:" << end - start << "ms";
  }

  // Set all Port PVID as ACT_VLAN_INIT_PVID(1)
  // Generate the port_vlan_entry_set(PVID = 1)
  if (!port_pvid_sub_item.GetMethods().isEmpty()) {
    start = QDateTime::currentMSecsSinceEpoch();
    QSet<ActPortVlanEntry> init_port_vlan_entry_set;
    for (auto intf : device.GetInterfaces()) {
      init_port_vlan_entry_set.insert(
          ActPortVlanEntry(intf.GetInterfaceId(), ACT_VLAN_INIT_PVID, ActVlanPriorityEnum::kNonTSN));
    }
    ActPortVlanTable default_port_vlan_table(device.GetId(), init_port_vlan_entry_set);
    act_status = GenerateTargetConfigPortVlanTable(device, default_port_vlan_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenerateTargetConfigPortVlanTable() failed.";
      return act_status;
    }
    act_status = ActionSetPortPVID(device, port_pvid_sub_item, default_port_vlan_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Set all ports PVID as ACT_VLAN_INIT_PVID(1) failed.";
      return act_status;
    }

    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __func__ << "Set all ports PVID as ACT_VLAN_INIT_PVID(1) duration:" << end - start << "ms";
  }

  // [bugfix:2860] Deploy - 802.1CB(FRER) - It should clear old stream id before configure new stream id
  if (method_protocol == ActConnectProtocolTypeEnum::kNETCONF) {
    SLEEP_MS(3000);  // wait switch sync to NETCONF VLAN DB
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }
  }

  // Get Add & Remove VlanStaticTable
  ActEditVlanStaticTable edit_vlan_table;
  if (!vlan_sub_item.GetMethods().isEmpty()) {
    start = QDateTime::currentMSecsSinceEpoch();

    // [feat:2766] Device Integration - APL
    // eCos switch would remove the not use VLAN when modifies the PVID
    ActVlanStaticTable vlan_static_table(device.GetId(), vlan_config_table.GetVlanStaticEntries());
    act_status = GenerateEditVlanTable(device, vlan_static_table, edit_vlan_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Generate EditVlanTable failed.";
      return act_status;
    }

    end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __func__ << "Generate EditVlanTable duration:" << end - start << "ms";
    // qDebug() << __func__ << "edit_vlan_table:" << edit_vlan_table.ToString().toStdString().c_str();

    start = QDateTime::currentMSecsSinceEpoch();

    // Remove Vlan member
    act_status = ActionDeleteVLANMember(device, vlan_sub_item, edit_vlan_table.GetDeleteVlanId());
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Delete VLAN member failed.";
      return act_status;
    }
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __func__ << "Delete VLAN member duration:" << end - start << "ms";

    // Add Vlan member
    act_status = ActionAddVLANMember(device, vlan_sub_item, edit_vlan_table.GetAddVlanId());
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Add VLAN member failed.";
      return act_status;
    }
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __func__ << "Add VLAN member duration:" << end - start << "ms";
  }

  // Set Vlan Port Type
  if (!vlan_port_type_sub_item.GetMethods().isEmpty()) {
    start = QDateTime::currentMSecsSinceEpoch();
    ActVlanPortTypeTable vlan_port_type_table(device.GetId(), vlan_config_table.GetVlanPortTypeEntries());
    act_status = GenerateTargetConfigVLANPortTypeTable(device, vlan_port_type_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenerateTargetConfigVLANPortTypeTable() failed.";
      return act_status;
    }
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __func__ << "GenerateTargetConfigVLANPortTypeTable duration:" << end - start << "ms";
    // SLEEP_MS(1000);  // test

    start = QDateTime::currentMSecsSinceEpoch();

    act_status = ActionSetVLANPortType(device, vlan_port_type_sub_item, vlan_port_type_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Set VLAN PortType failed.";
      return act_status;
    }
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __func__ << "Set VLAN PortType duration:" << end - start << "ms";
  }

  if (method_protocol == ActConnectProtocolTypeEnum::kNETCONF) {
    SLEEP_MS(3000);  // wait switch sync to NETCONF VLAN DB
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }
  }

  // Set PortPVID by DeviceConfig
  if (!port_pvid_sub_item.GetMethods().isEmpty()) {
    start = QDateTime::currentMSecsSinceEpoch();
    ActPortVlanTable port_vlan_table(device.GetId(), vlan_config_table.GetPortVlanEntries());
    act_status = GenerateTargetConfigPortVlanTable(device, port_vlan_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenerateTargetConfigPortVlanTable() failed.";
      return act_status;
    }
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    act_status = ActionSetPortPVID(device, port_pvid_sub_item, port_vlan_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Set PortPVID failed.";
      return act_status;
    }

    end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __func__ << "Set PortPVID duration:" << end - start << "ms";
  }

  // Set VlanStatic entry
  if (!vlan_sub_item.GetMethods().isEmpty()) {
    start = QDateTime::currentMSecsSinceEpoch();
    act_status = ActionSetVLAN(device, vlan_sub_item, edit_vlan_table.GetSetVlanStaticTable());
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Set VLAN static entry failed.";
      return act_status;
    }
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __func__ << "Set VLAN static entry duration:" << end - start << "ms";
  }

  // Set Management VLAN
  if (!mgmt_vlan_sub_item.GetMethods().isEmpty()) {
    start = QDateTime::currentMSecsSinceEpoch();
    act_status = ActionSetManagementVlan(device, mgmt_vlan_sub_item, vlan_config_table.GetManagementVlan());
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Set Management VLAN failed.";
      return act_status;
    }
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << __func__ << "Set Management VLAN duration:" << end - start << "ms";
  }
  return act_status;
}
ACT_STATUS ActSouthbound::ConfigureStaticForward(const ActDevice &device,
                                                 const ActStaticForwardTable &static_forward_table,
                                                 const bool &unicast) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  // feature group disable would return (skip default configuration)
  if (unicast) {
    if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetStaticForwardSetting().GetUnicast()) {
      qDebug() << __func__
               << QString("Skip config the unicast static_forward_table. Device: %1(%2)")
                      .arg(device.GetIpv4().GetIpAddress())
                      .arg(device.GetId())
                      .toStdString()
                      .c_str();
      return act_status;
    }
  } else {  // multicast
    if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetStaticForwardSetting().GetMulticast()) {
      qDebug() << __func__
               << QString("Skip config the multicast static_forward_table. Device: %1(%2)")
                      .arg(device.GetIpv4().GetIpAddress())
                      .arg(device.GetId())
                      .toStdString()
                      .c_str();
      return act_status;
    }
  }

  // Check VLAN entries exist
  QSet<qint32> vlan_id_set;
  for (auto entry : static_forward_table.GetStaticForwardEntries()) {
    vlan_id_set.insert(entry.GetVlanId());
  }
  act_status = CheckDeviceVlanEntriesExistByVlans(device, vlan_id_set);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "CheckDeviceVlanEntriesExistByVlans() failed";
    return act_status;
  }

  // Get Add & Remove StaticForwardTable
  ActAddRemoveStaticForwardTable add_remove_table;
  act_status = GenerateAddRemoveStaticForwardTable(device, static_forward_table, unicast, add_remove_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GenerateAddRemoveStaticForwardTable()  failed.";
    return act_status;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Remove StaticForward entries
  ActFeatureSubItem static_fwd_sub_item;
  QString sub_item_str_key = unicast ? "Unicast" : "Multicast";
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "StaticForwardSetting", sub_item_str_key,
                                       static_fwd_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  act_status =
      unicast ? ActionSetStaticUnicast(device, static_fwd_sub_item, add_remove_table.GetRemoveStaticForwardTable())
              : ActionSetStaticMulticast(device, static_fwd_sub_item, add_remove_table.GetRemoveStaticForwardTable());
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__
                << QString("Delete StaticForward(%1) entry failed.")
                       .arg(unicast ? "Unicast" : "Multicast")
                       .toStdString()
                       .c_str();
    return act_status;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Add StaticForward entries
  act_status = unicast
                   ? ActionSetStaticUnicast(device, static_fwd_sub_item, add_remove_table.GetAddStaticForwardTable())
                   : ActionSetStaticMulticast(device, static_fwd_sub_item, add_remove_table.GetAddStaticForwardTable());
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Add StaticForward entry failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureStreamPriorityIngress(const ActDevice &device,
                                                         const ActStadPortTable &stad_port_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  const auto vlan_feat_group = device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting();
  // feature group disable would return (skip default configuration)
  if ((!vlan_feat_group.GetPerStreamPriority()) && (!vlan_feat_group.GetPerStreamPriorityV2())) {
    qDebug() << __func__
             << QString("Skip config the Per-Stream Priority (Ingress). Device: %1(%2)")
                    .arg(device.GetIpv4().GetIpAddress())
                    .arg(device.GetId())
                    .toStdString()
                    .c_str();

    return act_status;
  }

  // Get sub-item
  ActFeatureSubItem feature_sub_item;
  if (vlan_feat_group.GetPerStreamPriorityV2()) {  // PerStreamPriorityV2
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "VLANSetting", "PerStreamPriorityV2", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  } else {  // PerStreamPriorityV1
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "VLANSetting", "PerStreamPriority", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Add StadPort entries(Ingress)
  qDebug() << __func__ << "Add StreamPriorityPort(Ingress)";
  act_status = ActionSetStreamPriorityIngress(device, feature_sub_item, stad_port_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Add ActionSetStreamPriorityIngress entry failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureStreamPriorityEgress(const ActDevice &device,
                                                        const ActStadConfigTable &stad_config_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  const auto vlan_feat_group = device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting();

  // feature group disable would return (skip default configuration)
  if ((!vlan_feat_group.GetPerStreamPriority()) && (!vlan_feat_group.GetPerStreamPriorityV2())) {
    qDebug() << __func__
             << QString("Skip config the Per-Stream Priority (Egress). Device: %1(%2)")
                    .arg(device.GetIpv4().GetIpAddress())
                    .arg(device.GetId())
                    .toStdString()
                    .c_str();

    return act_status;
  }

  // Get sub-item
  ActFeatureSubItem feature_sub_item;
  if (vlan_feat_group.GetPerStreamPriorityV2()) {  // PerStreamPriorityV2
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "VLANSetting", "PerStreamPriorityV2", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  } else {  // PerStreamPriorityV1

    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "VLANSetting", "PerStreamPriority", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Deploy StreamPriorityConfig(Egress)
  qDebug() << __func__ << "Add StreamPriorityPortV2(Egress)";
  act_status = ActionSetStreamPriorityEgress(device, feature_sub_item, stad_config_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Deploy StreamPriorityV2(Egress) failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSouthbound::ConfigurePortDefaultPCP(const ActDevice &device,
                                                  const ActDefaultPriorityTable &default_priority_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  // feature group disable would return (skip default configuration)
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetDefaultPCP()) {
    qDebug() << __func__
             << QString("Skip config the default PCP. Device: %1(%2)")
                    .arg(device.GetIpv4().GetIpAddress())
                    .arg(device.GetId())
                    .toStdString()
                    .c_str();

    return act_status;
  }

  // Get sub-item
  ActFeatureSubItem feature_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "VLANSetting", "DefaultPCP", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Deploy PortDefaultPCP
  act_status = ActionSetPortDefaultPCP(device, feature_sub_item, default_priority_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Deploy PortDefaultPCP failed";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureGCL(const ActDevice &device, const ActGclTable &gcl_table) {
  ACT_STATUS_INIT();

  // Get sub-item
  ActFeatureSubItem feature_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "TSN", "IEEE802Dot1Qbv", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Deploy GCL
  act_status = ActionSet802Dot1Qbv(device, feature_sub_item, gcl_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Deploy 802.1Qbv(GateControlList) failed";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureFRER(const ActDevice &device, const ActCbTable &ieee_802_1cb_table) {
  ACT_STATUS_INIT();

  // Get sub-item
  ActFeatureSubItem feature_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "TSN", "IEEE802Dot1CB", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Deploy CB
  act_status = ActionSet802Dot1CB(device, feature_sub_item, ieee_802_1cb_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Deploy 802.1CB failed";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureSpanningTree(const ActDevice &device, const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  // Deploy SpanningTree(Active, Version)
  ActFeatureSubItem rstp_method_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "STPRSTP", "RSTPMethod", rstp_method_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  act_status = ActionSetSpanningTreeMethod(device, rstp_method_sub_item, rstp_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Deploy SpanningTree failed.";
    return act_status;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Deploy SpanningTree(RSTP)
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetRSTP()) {
    ActFeatureSubItem method_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "STPRSTP", "RSTP", method_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    act_status = ActionSetSpanningTreeBasicRstp(device, method_sub_item, rstp_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy SpanningTree(RSTP) failed.";
      qCritical() << __func__ << "rstp_table:" << rstp_table.ToString().toStdString().c_str();

      return act_status;
    }
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Deploy SpanningTree(Port RSTPEnable)
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetPortRSTPEnable()) {
    ActFeatureSubItem method_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "STPRSTP", "PortRSTPEnable", method_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    act_status = ActionSetSpanningTreePortRSTPEnable(device, method_sub_item, rstp_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy SpanningTree(RstpEnable) failed.";
      qCritical() << __func__ << "rstp_table:" << rstp_table.ToString().toStdString().c_str();
      return act_status;
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Deploy SpanningTree(ErrorRecoveryTime)
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetErrorRecoveryTime()) {
    ActFeatureSubItem method_sub_item;
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "STPRSTP", "ErrorRecoveryTime", method_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    act_status = ActionSetSpanningTreeErrorRecoveryTime(device, method_sub_item, rstp_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy SpanningTree(ErrorRecoveryTime) failed.";
      qCritical() << __func__ << "rstp_table:" << rstp_table.ToString().toStdString().c_str();

      return act_status;
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Deploy SpanningTree(LinkType)
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetLinkType()) {
    ActFeatureSubItem method_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "STPRSTP", "LinkType", method_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    act_status = ActionSetSpanningTreeLinkType(device, method_sub_item, rstp_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy SpanningTree(LinkType) failed.";
      qCritical() << __func__ << "rstp_table:" << rstp_table.ToString().toStdString().c_str();
      return act_status;
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Init the RootGuard & LoopGuard.
  ActRstpTable init_rstp_table(device);
  // Init SpanningTree RootGuard
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetRootGuard()) {
    ActFeatureSubItem method_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "STPRSTP", "RootGuard", method_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    act_status = ActionSetSpanningTreeRootGuard(device, method_sub_item, init_rstp_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Init SpanningTree RootGuard failed.";
      qCritical() << __func__ << "init_rstp_table:" << init_rstp_table.ToString().toStdString().c_str();
      return act_status;
    }
  }

  // Init SpanningTree LoopGuard
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetLoopGuard()) {
    ActFeatureSubItem method_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "STPRSTP", "LoopGuard", method_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    act_status = ActionSetSpanningTreeLoopGuard(device, method_sub_item, init_rstp_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Init SpanningTree LoopGuard failed.";
      qCritical() << __func__ << "init_rstp_table:" << init_rstp_table.ToString().toStdString().c_str();
      return act_status;
    }
  }

  // Deploy SpanningTree(BPDUGuard)
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetBPDUGuard()) {
    ActFeatureSubItem method_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "STPRSTP", "BPDUGuard", method_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    act_status = ActionSetSpanningTreeBPDUGuard(device, method_sub_item, rstp_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy SpanningTree(BPDUGuard) failed.";
      qCritical() << __func__ << "rstp_table:" << rstp_table.ToString().toStdString().c_str();
      return act_status;
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Deploy SpanningTree(RootGuard)
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetRootGuard()) {
    ActFeatureSubItem method_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "STPRSTP", "RootGuard", method_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    act_status = ActionSetSpanningTreeRootGuard(device, method_sub_item, rstp_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy SpanningTree(RootGuard) failed.";
      qCritical() << __func__ << "rstp_table:" << rstp_table.ToString().toStdString().c_str();
      return act_status;
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Deploy SpanningTree(LoopGuard)
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetLoopGuard()) {
    ActFeatureSubItem method_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "STPRSTP", "LoopGuard", method_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    act_status = ActionSetSpanningTreeLoopGuard(device, method_sub_item, rstp_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy SpanningTree(LoopGuard) failed.";
      qCritical() << __func__ << "rstp_table:" << rstp_table.ToString().toStdString().c_str();
      return act_status;
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Deploy SpanningTree(BPDUFilter)
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetBPDUFilter()) {
    ActFeatureSubItem method_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "STPRSTP", "BPDUFilter", method_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    act_status = ActionSetSpanningTreeBPDUFilter(device, method_sub_item, rstp_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy SpanningTree(BPDUFilter) failed.";
      qCritical() << __func__ << "rstp_table:" << rstp_table.ToString().toStdString().c_str();
      return act_status;
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Deploy SpanningTree(Swift)
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetSwift()) {
    ActFeatureSubItem method_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "STPRSTP", "Swift", method_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    act_status = ActionSetSpanningTreeSwift(device, method_sub_item, rstp_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy SpanningTree(Swift) failed.";
      qCritical() << __func__ << "rstp_table:" << rstp_table.ToString().toStdString().c_str();
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureUserAccount(const ActDevice &device, const ActUserAccountTable &user_account_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetUserAccount()) {
    ActFeatureSubItem feature_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "UserAccount", "Basic", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    // Set UserAccount()
    act_status = ActionSetUserAccount(device, feature_sub_item, user_account_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy UserAccount() failed.";
      ActUserAccountTable tmp_table = user_account_table;
      tmp_table.HidePassword();
      qCritical() << __func__ << "user_account_table:" << tmp_table.ToString().toStdString().c_str();
      return act_status;
    }
  }
  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureLoginPolicy(const ActDevice &device, const ActLoginPolicyTable &login_policy_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetLoginPolicy()) {
    ActFeatureSubItem feature_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "LoginPolicy", "Basic", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    // Set LoginPolicy()
    act_status = ActionSetLoginPolicy(device, feature_sub_item, login_policy_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy LoginPolicy() failed.";
      qCritical() << __func__ << "login_policy_table:" << login_policy_table.ToString().toStdString().c_str();
      return act_status;
    }
  }
  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureSnmpTrapSetting(const ActDevice &device,
                                                   const ActSnmpTrapSettingTable &snmp_trap_setting_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSNMPTrapSetting()) {
    ActFeatureSubItem feature_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "SNMPTrapSetting", "Basic", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    // Set SnmpTrapSetting()
    act_status = ActionSetSnmpTrapSetting(device, feature_sub_item, snmp_trap_setting_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy SnmpTrapSetting() failed.";
      qCritical() << __func__ << "snmp_trap_setting_table:" << snmp_trap_setting_table.ToString().toStdString().c_str();
      return act_status;
    }
  }
  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureSyslogSetting(const ActDevice &device,
                                                 const ActSyslogSettingTable &syslog_setting_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSyslogSetting()) {
    ActFeatureSubItem feature_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "SyslogSetting", "Basic", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    // Set SyslogSetting()
    act_status = ActionSetSyslogSetting(device, feature_sub_item, syslog_setting_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy SyslogSetting() failed.";
      qCritical() << __func__ << "syslog_setting_table:" << syslog_setting_table.ToString().toStdString().c_str();
      return act_status;
    }
  }
  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureTimeSetting(const ActDevice &device, const ActTimeSettingTable &time_setting_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSetting().GetSystemTime()) {
    ActFeatureSubItem feature_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "TimeSetting", "Basic", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    // Set TimeSetting()
    act_status = ActionSetTimeSetting(device, feature_sub_item, time_setting_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy TimeSetting() failed.";
      qCritical() << __func__ << "time_setting_table:" << time_setting_table.ToString().toStdString().c_str();
      return act_status;
    }
  }
  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureInformationSetting(const ActDevice &device,
                                                      const ActInformationSettingTable &info_setting_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetInformationSetting()) {
    ActFeatureSubItem feature_sub_item;
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "InformationSetting", "Basic", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    // Set InformationSetting
    act_status = ActionSetInformationSetting(device, feature_sub_item, info_setting_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy InformationSetting() failed.";
      qCritical() << __func__ << "info_setting_table:" << info_setting_table.ToString().toStdString().c_str();
      return act_status;
    }
  }
  return act_status;
}

ACT_STATUS ActSouthbound::ConfigurePortSetting(const ActDevice &device, const ActPortSettingTable &port_setting_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetPortSetting().GetAdminStatus()) {
    ActFeatureSubItem feature_sub_item;
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "PortSetting", "AdminStatus", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    // Set PortSetting(AdminStatus)
    act_status = ActionSetPortSettingAdminStatus(device, feature_sub_item, port_setting_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy PortSetting(AdminStatus) failed.";
      qCritical() << __func__ << "port_setting_table:" << port_setting_table.ToString().toStdString().c_str();
      return act_status;
    }

    // if (stop_flag_) {
    //   return ACT_STATUS_STOP;
    // }

    // // Get PortSetting(AdminStatus)
    // ActPortSettingTable south_port_setting_table(device.GetId());
    // act_status = ActionGetPortSettingAdminStatus(device, feature_sub_item, south_port_setting_table);
    // if (!IsActStatusSuccess(act_status)) {
    //   qCritical() << __func__ << "Get PortSetting(AdminStatus) failed.";
    //   return act_status;
    // }
    // qDebug() << __func__ << "south_port_setting_table:" << south_port_setting_table.ToString().toStdString().c_str();
  }
  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureLoopProtection(const ActDevice &device,
                                                  const ActLoopProtectionTable &loop_protection_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetLoopProtection()) {
    ActFeatureSubItem feature_sub_item;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "LoopProtection", "Basic", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    // Set LoopProtection()
    act_status = ActionSetLoopProtection(device, feature_sub_item, loop_protection_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy LoopProtection() failed.";
      qCritical() << __func__ << "loop_protection_table:" << loop_protection_table.ToString().toStdString().c_str();
      return act_status;
    }
  }
  return act_status;
}

ACT_STATUS ActSouthbound::ConfigureManagementInterface(const ActDevice &device,
                                                       const ActManagementInterfaceTable &mgmt_interface_table) {
  ACT_STATUS_INIT();

  qDebug() << __func__
           << QString("Device(%1) is under processing.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetManagementInterface()) {
    ActFeatureSubItem feature_sub_item;
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "ManagementInterface", "Basic", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    // Set ManagementInterface
    act_status = ActionSetManagementInterface(device, feature_sub_item, mgmt_interface_table);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Deploy ManagementInterface failed.";
      qCritical() << __func__ << "ManagementInterface table:" << mgmt_interface_table.ToString().toStdString().c_str();
      return act_status;
    }
  }
  return act_status;
}

// ACT_STATUS ActSouthbound::ActDeploy::ModifyVlanPortTypeFromTrunkToHybrid(
//     const ActDevice &device, const QMap<QString, ActFeatureActionCapability> &action_capa_map) {
//   ACT_STATUS_INIT();

//   if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode() != true) {
//     return act_status;
//   }

//   QSet<ActVlanPortTypeEntry> south_vlan_port_type_entries;
//   // Get switch DeviceVlanConfigVlanPortType by southbound
//   QList<ActActionMethod> get_vlan_port_type_action_methods;
//   get_vlan_port_type_action_methods.append(action_capa_map["GetVlanPortType"].GetMethod());
//   act_status = ActionGetVlanPortType(device, action_capa_map["GetVlanPortType"].GetAction(),
//                                                  get_vlan_port_type_action_methods, south_vlan_port_type_entries);
//   if (!IsActStatusSuccess(act_status)) {
//     return act_status;
//   }

//   QSet<ActVlanPortTypeEntry> mod_vlan_port_type_entries;
//   for (auto entry : south_vlan_port_type_entries) {
//     if (entry.GetVlanPortType() == ActVlanPortTypeEnum::kTrunk) {  // trunk (2)
//       // mod_vlan_port_type_entries.insert(ActVlanPortTypeEntry(entry.GetPortId(), ActVlanPortTypeEnum::kAccess)); //
//       // access(1)
//       mod_vlan_port_type_entries.insert(
//           ActVlanPortTypeEntry(entry.GetPortId(), ActVlanPortTypeEnum::kHybrid));  // hybrid(3)
//     }
//   }

//   // Start transfer
//   if (mod_vlan_port_type_entries.size() != 0) {
//     QList<ActActionMethod> set_vlan_port_type_action_methods;
//     set_vlan_port_type_action_methods.append(action_capa_map["SetVlanPortType"].GetMethod());

//     // Set Port vlan type
//     ActVlanPortTypeTable vlan_port_type_table(device.GetId(), mod_vlan_port_type_entries);
//     act_status = ActionSetVlanPortType(device, action_capa_map["SetVlanPortType"].GetAction(),
//                                                    set_vlan_port_type_action_methods, vlan_port_type_table);
//     if (!IsActStatusSuccess(act_status)) {
//       return act_status;
//     }

//     // Set Vlan entry untagged
//     QList<ActActionMethod> set_vlan_action_methods;
//     set_vlan_action_methods.append(action_capa_map["SetVLAN"].GetMethod());

//     QSet<ActVlanStaticEntry> vlan_static_entries;
//     ActVlanStaticEntry vlan_static_entry(ACT_VLAN_INIT_PVID);
//     for (auto interface : device.GetInterfaces()) {
//       vlan_static_entry.InsertUntaggedPorts(interface.GetInterfaceId());
//     }
//     vlan_static_entries.insert(vlan_static_entry);
//     ActVlanStaticTable vlan_static_table(device.GetId(), vlan_static_entries);
//     act_status = ActionSetVlanUntaggedPorts(device, action_capa_map["SetVLAN"].GetAction(),
//                                                         set_vlan_action_methods, vlan_static_table);
//     if (!IsActStatusSuccess(act_status)) {
//       return act_status;
//     }
//   }

//   return act_status;
// }

ACT_STATUS ActSouthbound::GenerateEditVlanTable(const ActDevice &device, const ActVlanStaticTable &vlan_static_table,
                                                ActEditVlanStaticTable &edit_vlan_static_table) {
  ACT_STATUS_INIT();

  edit_vlan_static_table = ActEditVlanStaticTable();

  QSet<ActVlanStaticEntry> set_entries = vlan_static_table.GetVlanStaticEntries();
  QList<qint32> add_vlan_list;
  QList<qint32> delete_vlan_list;

  // Add all vlan_static_entry to the add_vlan_list
  for (auto entry : vlan_static_table.GetVlanStaticEntries()) {
    add_vlan_list.append(entry.GetVlanId());
  }

  // Get sub-item
  ActFeatureSubItem vlan_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "VLANSetting", "VLANMethod", vlan_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Get switch vlan_static_table by southbound
  ActVlanStaticTable south_vlan_static_table(device.GetId());
  act_status = ActionGetVLAN(device, vlan_sub_item, south_vlan_static_table);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Compare entries
  for (auto entry : south_vlan_static_table.GetVlanStaticEntries()) {
    // if (!vlan_range.InVlanRange(entry.GetVlanId())) {
    //   // Only handle the vlan_id in the vlan_range
    //   continue;
    // }

    // Find set VLAN
    auto entry_iter = set_entries.find(entry);  // by vlan_id
    if (entry_iter == set_entries.end()) {      // not found
      // The switch's entries not in the target_deploy_entries but their vlan_id in the vlan_range.
      // These entries would be remove from the switch.

      // PVID(1) & MELCO VID(2) not be removed
      if (device.GetDeviceProperty().GetReservedVlan().contains(entry.GetVlanId())) {
        // Skip add again
        if (add_vlan_list.contains(entry.GetVlanId())) {
          add_vlan_list.removeAll(entry.GetVlanId());
        }
        continue;
      }

      if (!delete_vlan_list.contains(entry.GetVlanId())) {
        delete_vlan_list.append(entry.GetVlanId());
      }

    } else {  // found
      ActVlanStaticEntry find_entry(*entry_iter);

      // [feat:529] untag
      if (find_entry.GetEgressPorts() == entry.GetEgressPorts() &&
          find_entry.GetUntaggedPorts() == entry.GetUntaggedPorts() && find_entry.GetTeMstid() == entry.GetTeMstid() &&
          find_entry.GetName() == entry.GetName()) {
        set_entries.erase(entry_iter);  // not deploy again when the entry is already in the switch
        if (add_vlan_list.contains(entry.GetVlanId())) {
          add_vlan_list.removeAll(entry.GetVlanId());
        }
        continue;
      }

      // PVID(1) & MELCO VID(2) not be removed
      if (device.GetDeviceProperty().GetReservedVlan().contains(entry.GetVlanId())) {
        // Skip add again
        if (add_vlan_list.contains(entry.GetVlanId())) {
          add_vlan_list.removeAll(entry.GetVlanId());
        }
        continue;
      }

      // Delete & Add again to init the port settings
      if (!delete_vlan_list.contains(entry.GetVlanId())) {
        delete_vlan_list.append(entry.GetVlanId());
      }
    }
  }

  // Sort add & delete vlan list
  std::sort(add_vlan_list.begin(), add_vlan_list.end());
  std::sort(delete_vlan_list.begin(), delete_vlan_list.end());

  edit_vlan_static_table.SetSetVlanStaticTable(ActVlanStaticTable(device.GetId(), set_entries));
  edit_vlan_static_table.SetAddVlanId(add_vlan_list);
  edit_vlan_static_table.SetDeleteVlanId(delete_vlan_list);

  return act_status;
}

ACT_STATUS ActSouthbound::CheckDeviceVLANConfiguration(const ActDevice &device, const ActVlanTable &vlan_config_table,
                                                       bool &result) {
  ACT_STATUS_INIT();
  result = false;

  // Check VLAN entries exist
  QSet<qint32> vlan_id_set;
  for (auto entry : vlan_config_table.GetVlanStaticEntries()) {
    vlan_id_set.insert(entry.GetVlanId());
  }

  ActVlanStaticTable south_vlan_static_table(device.GetId());

  // Get sub-item
  ActFeatureSubItem vlan_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "VLANSetting", "VLANMethod", vlan_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Get switch vlan_static_table by southbound
  act_status = ActionGetVLAN(device, vlan_sub_item, south_vlan_static_table);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Create south_vlan_id_set
  QSet<qint32> south_vlan_id_set;
  for (auto entry : south_vlan_static_table.GetVlanStaticEntries()) {
    south_vlan_id_set.insert(entry.GetVlanId());
  }

  // Check vlan number
  if (vlan_id_set == south_vlan_id_set) {
    result = true;
  }
  return act_status;
}

ACT_STATUS ActSouthbound::GenerateRemoveStaticForwardTableByVlans(const ActDevice &device,
                                                                  const QSet<qint32> &vlan_id_set, const bool &unicast,
                                                                  ActStaticForwardTable &remove_static_forward_table) {
  ACT_STATUS_INIT();

  ActStaticForwardTable south_static_forward_table(device.GetId());
  QSet<ActStaticForwardEntry> del_entries;

  // Get Sub-item
  ActFeatureSubItem static_fwd_sub_item;
  QString sub_item_str_key = unicast ? "Unicast" : "Multicast";
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "StaticForwardSetting", sub_item_str_key,
                                       static_fwd_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Get switch static_forward_table by southbound
  act_status = unicast ? ActionGetStaticUnicast(device, static_fwd_sub_item, south_static_forward_table)
                       : ActionGetStaticMulticast(device, static_fwd_sub_item, south_static_forward_table);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Compare entries
  for (auto entry : south_static_forward_table.GetStaticForwardEntries()) {
    // if (!vlan_range.InVlanRange(entry.GetVlanId())) {
    //   // Only handle the vlan_id in the vlan_range
    //   continue;
    // }

    // Remove PVID(1) because would let config vlan failed
    if (entry.GetVlanId() == ACT_VLAN_INIT_PVID) {
      continue;
    }

    auto entry_iter = vlan_id_set.find(entry.GetVlanId());
    if (!vlan_id_set.contains(entry.GetVlanId())) {
      ActStaticForwardEntry del_entry = entry;
      // del_entry.SetRowStatus(6);    // destroy(6)
      del_entry.SetDot1qStatus(2);  // invalid(2)
      del_entries.insert(del_entry);
    }
  }

  remove_static_forward_table = ActStaticForwardTable(device.GetId(), del_entries);

  return act_status;
}

ACT_STATUS ActSouthbound::GenerateRemoveStaticForwardTableByVlanTable(
    const ActDevice &device, const ActVlanTable &vlan_config_table, const bool &unicast,
    ActStaticForwardTable &remove_static_forward_table) {
  ACT_STATUS_INIT();

  ActStaticForwardTable south_static_forward_table(device.GetId());
  QSet<ActStaticForwardEntry> del_entries;

  // Get Sub-item
  ActFeatureSubItem static_fwd_sub_item;
  QString sub_item_str_key = unicast ? "Unicast" : "Multicast";
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "StaticForwardSetting", sub_item_str_key,
                                       static_fwd_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Get switch static_forward_table by southbound
  act_status = unicast ? ActionGetStaticUnicast(device, static_fwd_sub_item, south_static_forward_table)
                       : ActionGetStaticMulticast(device, static_fwd_sub_item, south_static_forward_table);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Compare entries
  for (auto static_fwd_entry : south_static_forward_table.GetStaticForwardEntries()) {
    ActVlanStaticEntry vlan_entry(static_fwd_entry.GetVlanId());
    auto vlan_entry_iter = vlan_config_table.GetVlanStaticEntries().find(vlan_entry);
    if (vlan_entry_iter == vlan_config_table.GetVlanStaticEntries().end()) {  // not found
      // Remove the static_forward_entry
      ActStaticForwardEntry del_entry = static_fwd_entry;
      // del_entry.SetRowStatus(6);    // destroy(6)
      del_entry.SetDot1qStatus(2);  // invalid(2)
      del_entries.insert(del_entry);
    } else {  // found
      // Check vlan member port
      ActVlanStaticEntry find_vlan_entry(*vlan_entry_iter);
      for (auto port : static_fwd_entry.GetEgressPorts()) {
        if (!find_vlan_entry.GetEgressPorts().contains(port)) {
          // Remove the static_forward_entry
          ActStaticForwardEntry del_entry = static_fwd_entry;
          // del_entry.SetRowStatus(6);    // destroy(6)
          del_entry.SetDot1qStatus(2);  // invalid(2)
          del_entries.insert(del_entry);
        }
      }
    }
  }

  remove_static_forward_table = ActStaticForwardTable(device.GetId(), del_entries);

  return act_status;
}

ACT_STATUS ActSouthbound::GenerateTargetConfigVLANPortTypeTable(const ActDevice &device,
                                                                ActVlanPortTypeTable &vlan_port_type_table) {
  ACT_STATUS_INIT();

  QSet<ActVlanPortTypeEntry> result_entries;

  // Get sub-item
  ActFeatureSubItem vlan_port_type_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "VLANSetting", "AccessTrunkMode",
                                       vlan_port_type_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Get switch vlan_port_type_table by southbound
  ActVlanPortTypeTable south_vlan_port_type_table(device.GetId());
  act_status = ActionGetVLANPortType(device, vlan_port_type_sub_item, south_vlan_port_type_table);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // qCritical() << __func__
  //             << "south_vlan_port_type_table:" << south_vlan_port_type_table.ToString().toStdString().c_str();

  // Compare entries
  for (auto target_entry : vlan_port_type_table.GetVlanPortTypeEntries()) {
    auto south_entry_iter = south_vlan_port_type_table.GetVlanPortTypeEntries().find(target_entry);  // by vlan_id
    if (south_entry_iter != south_vlan_port_type_table.GetVlanPortTypeEntries().end()) {             // found
      // If the same of the PortType would skip config
      if (south_entry_iter->GetVlanPortType() == target_entry.GetVlanPortType()) {
        continue;
      }
    }

    result_entries.insert(target_entry);
  }

  vlan_port_type_table.SetVlanPortTypeEntries(result_entries);
  return act_status;
}

ACT_STATUS ActSouthbound::GenerateTargetConfigPortVlanTable(const ActDevice &device,
                                                            ActPortVlanTable &port_vlan_table) {
  ACT_STATUS_INIT();

  QSet<ActPortVlanEntry> result_entries;

  // Get sub-item
  ActFeatureSubItem port_pvid_sub_item;
  act_status =
      GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                              ActFeatureEnum::kConfiguration, "VLANSetting", "DefaultPVID", port_pvid_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Get switch port_vlan_table by southbound
  ActPortVlanTable south_port_vlan_table(device.GetId());
  act_status = ActionGetPortPVID(device, port_pvid_sub_item, south_port_vlan_table);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Compare entries
  for (auto target_entry : port_vlan_table.GetPortVlanEntries()) {
    auto south_entry_iter = south_port_vlan_table.GetPortVlanEntries().find(target_entry);  // by vlan_id

    if (south_entry_iter != south_port_vlan_table.GetPortVlanEntries().end()) {  // found
      // If the same of the PVID would skip config
      if (south_entry_iter->GetPVID() == target_entry.GetPVID()) {
        continue;
      }
    }

    result_entries.insert(target_entry);
  }

  port_vlan_table.SetPortVlanEntries(result_entries);
  return act_status;
}

ACT_STATUS ActSouthbound::GenerateAddRemoveStaticForwardTable(
    const ActDevice &device, const ActStaticForwardTable &static_forward_table, const bool &unicast,
    ActAddRemoveStaticForwardTable &add_remove_static_forward_table) {
  ACT_STATUS_INIT();

  add_remove_static_forward_table = ActAddRemoveStaticForwardTable();

  QSet<ActStaticForwardEntry> add_entries;
  QSet<ActStaticForwardEntry> del_entries;
  add_entries = static_forward_table.GetStaticForwardEntries();

  // Get Unicast / Multicast sub-item
  ActFeatureSubItem static_fwd_sub_item;
  QString sub_item_str_key = unicast ? "Unicast" : "Multicast";
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "StaticForwardSetting", sub_item_str_key,
                                       static_fwd_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Get switch static_forward_table by southbound
  ActStaticForwardTable south_static_forward_table(device.GetId());
  act_status = unicast ? ActionGetStaticUnicast(device, static_fwd_sub_item, south_static_forward_table)
                       : ActionGetStaticMulticast(device, static_fwd_sub_item, south_static_forward_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get ActionGetStaticUnicast(Multicast)  failed.";
    return act_status;
  }

  // Compare entries
  for (auto entry : south_static_forward_table.GetStaticForwardEntries()) {
    // if (!vlan_range.InVlanRange(entry.GetVlanId())) {
    //   // Only handle the vlan_id in the vlan_range
    //   continue;
    // }

    // Skip PVID(1)
    if (entry.GetVlanId() == ACT_VLAN_INIT_PVID) {
      continue;
    }

    auto entry_iter = add_entries.find(entry);  // by vlan_id + mac
    if ((entry_iter != add_entries.end()) &&
        (ActStaticForwardEntry(*entry_iter).GetEgressPorts() == entry.GetEgressPorts())) {
      add_entries.erase(entry_iter);  // not deploy again when the entry is already in the switch
    } else {
      ActStaticForwardEntry del_entry = entry;
      // del_entry.SetRowStatus(6);    // destroy(6)
      del_entry.SetDot1qStatus(2);  // invalid(2)
      del_entries.insert(del_entry);
    }
  }

  add_remove_static_forward_table.SetAddStaticForwardTable(ActStaticForwardTable(device.GetId(), add_entries));
  add_remove_static_forward_table.SetRemoveStaticForwardTable(ActStaticForwardTable(device.GetId(), del_entries));

  return act_status;
}

// ACT_STATUS ActSouthbound::GenerateAddRemoveVlanTable(
//     const ActDevice &device, const ActVlanStaticTable &vlan_static_table,
//     ActAddRemoveVlanStaticTable &add_remove_vlan_static_table) {
//   ACT_STATUS_INIT();

//   add_remove_vlan_static_table = ActAddRemoveVlanStaticTable();

//   QSet<ActVlanStaticEntry> add_entries;
//   QSet<ActVlanStaticEntry> del_entries;
//   add_entries = vlan_static_table.GetVlanStaticEntries();

//   // Get sub-item
//   ActFeatureSubItem vlan_sub_item;
//   act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
//                                        ActFeatureEnum::kConfiguration, "VLANSetting", "VLANMethod", vlan_sub_item);
//   if (!IsActStatusSuccess(act_status)) {
//     return act_status;
//   }

//   // Get switch vlan_static_table by southbound
//   ActVlanStaticTable south_vlan_static_table(device.GetId());
//   act_status = ActionGetVLAN(device, vlan_sub_item, south_vlan_static_table);
//   if (!IsActStatusSuccess(act_status)) {
//     return act_status;
//   }

//   // Compare entries
//   for (auto entry : south_vlan_static_table.GetVlanStaticEntries()) {
//     // if (!vlan_range.InVlanRange(entry.GetVlanId())) {
//     //   // Only handle the vlan_id in the vlan_range
//     //   continue;
//     // }

//     auto entry_iter = add_entries.find(entry);  // by vlan_id
//     if (entry_iter == add_entries.end()) {      // not found
//       // The switch's entries not in the target_deploy_entries but their vlan_id in the vlan_range.
//       // These entries would be remove from the switch.

//       // PVID(1) & MELCO VID(2) not be removed
//       if (device.GetDeviceProperty().GetReservedVlan().contains(entry.GetVlanId())) {
//         continue;
//       }

//       ActVlanStaticEntry del_entry = entry;
//       del_entry.SetRowStatus(6);  // destroy(6)
//       del_entries.insert(del_entry);
//     } else {  // found
//       ActVlanStaticEntry find_entry(*entry_iter);

//       // [feat:529] untag
//       if (find_entry.GetEgressPorts() == entry.GetEgressPorts() &&
//           find_entry.GetUntaggedPorts() == entry.GetUntaggedPorts() && find_entry.GetTeMstid() == entry.GetTeMstid())
//           {
//         add_entries.erase(entry_iter);  // not deploy again when the entry is already in the switch
//       } else if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) {
//         // PVID(1) & MELCO VID(2) not be removed
//         if (device.GetDeviceProperty().GetReservedVlan().contains(entry.GetVlanId())) {
//           // Modify add the SetRowStatus(0) // snmp would skip active it
//           find_entry.SetRowStatus(0);
//           add_entries.erase(entry_iter);
//           add_entries.insert(find_entry);
//           continue;
//         }

//         // [bugfix:2331] Modify the VLAN untag port before must remove the VLAN entry
//         // [bugfix:1652] Vlan untag port can't change to egress port
//         // hybrid can't directly modify another's untag port, so this need to delete the entry & add deploy again
//         ActVlanStaticEntry del_entry = entry;
//         del_entry.SetRowStatus(6);  // destroy(6)
//         del_entries.insert(del_entry);
//       }
//     }
//   }

//   add_remove_vlan_static_table.SetAddVlanStaticTable(ActVlanStaticTable(device.GetId(), add_entries));
//   add_remove_vlan_static_table.SetRemoveVlanStaticTable(ActVlanStaticTable(device.GetId(), del_entries));

//   return act_status;
// }

ACT_STATUS ActSouthbound::CheckDeviceVlanEntriesExistByVlans(const ActDevice &device, const QSet<qint32> &vlan_id_set) {
  ACT_STATUS_INIT();

  ActVlanStaticTable south_vlan_static_table(device.GetId());

  // Get sub-item
  ActFeatureSubItem vlan_sub_item;
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kConfiguration, "VLANSetting", "VLANMethod", vlan_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Get switch vlan_static_table by southbound
  act_status = ActionGetVLAN(device, vlan_sub_item, south_vlan_static_table);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Create south_vlan_id_set
  QSet<qint32> south_vlan_id_set;
  for (auto entry : south_vlan_static_table.GetVlanStaticEntries()) {
    south_vlan_id_set.insert(entry.GetVlanId());
  }

  // Check vlan exists
  for (auto vlan_id : vlan_id_set) {
    if (!south_vlan_id_set.contains(vlan_id)) {
      qCritical()
          << __func__
          << QString("Device(%1) not found the VLAN(%2) entry").arg(device.GetIpv4().GetIpAddress()).arg(vlan_id);
      return std::make_shared<ActStatusNotFound>(QString("VLAN(%1) entry").arg(vlan_id));
    }
  }

  return act_status;
}

ACT_STATUS ActSouthbound::GenerateAddRemoveStreamPriorityIngressTable(
    const ActDevice &device, const ActStadPortTable &stad_port_table,
    ActAddRemoveStadPortTable &add_remove_stad_port_table) {
  ACT_STATUS_INIT();

  add_remove_stad_port_table = ActAddRemoveStadPortTable();

  QSet<ActInterfaceStadPortEntry> add_if_stad_port_entries;
  QSet<ActInterfaceStadPortEntry> del_if_stad_port_entries;
  QSet<ActInterfaceStadPortEntry> new_south_if_stad_port_entries;

  add_if_stad_port_entries = stad_port_table.GetInterfaceStadPortEntries();

  // Get sub-item
  ActFeatureSubItem feature_sub_item;
  act_status =
      GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                              ActFeatureEnum::kConfiguration, "VLANSetting", "PerStreamPriority", feature_sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Get switch stad_port_table by southbound
  ActStadPortTable south_stad_port_table(device.GetId());
  act_status = ActionGetStreamPriorityIngress(device, feature_sub_item, south_stad_port_table);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Compare entries
  for (auto if_stad_port_entry : south_stad_port_table.GetInterfaceStadPortEntries()) {
    QSet<ActStadPortEntry> del_stad_port_entries;
    QSet<ActStadPortEntry> south_stad_port_entries;

    for (auto stad_port_entry : if_stad_port_entry.GetStadPortEntries()) {  // for entry
      // if (!vlan_range.InVlanRange(stad_port_entry.GetVlanId())) {
      //   south_stad_port_entries.insert(stad_port_entry);
      //   // Only handle the vlan_id in the vlan_range
      //   continue;
      // }

      // Skip PVID(1)
      if (stad_port_entry.GetVlanId() == ACT_VLAN_INIT_PVID) {
        south_stad_port_entries.insert(stad_port_entry);
        continue;
      }

      // Remove other entries
      stad_port_entry.SetIndexEnable(2);
      del_stad_port_entries.insert(stad_port_entry);
    }

    // Update new_south_if_stad_port_entries
    if_stad_port_entry.SetStadPortEntries(south_stad_port_entries);
    if (!if_stad_port_entry.GetStadPortEntries().isEmpty()) {
      new_south_if_stad_port_entries.insert(if_stad_port_entry);
    }

    // Update del_if_stad_port_entries
    ActInterfaceStadPortEntry del_if_stad_port_entry(if_stad_port_entry.GetInterfaceId(), del_stad_port_entries);
    if (!del_if_stad_port_entry.GetStadPortEntries().isEmpty()) {
      del_if_stad_port_entries.insert(del_if_stad_port_entry);
    }
  }

  // Update remove_stad_port_table
  add_remove_stad_port_table.SetRemoveStadPortTable(ActStadPortTable(device.GetId(), del_if_stad_port_entries));

  // Start to Update the add_if_stad_port_entries
  for (auto deploy_if_stad_port_entry : stad_port_table.GetInterfaceStadPortEntries()) {  // for interface
    // Find the new_south_if_stad_port_entry by if_entry
    auto new_south_if_stad_port_entry_iter =
        new_south_if_stad_port_entries.find(ActInterfaceStadPortEntry(deploy_if_stad_port_entry.GetInterfaceId()));

    if (new_south_if_stad_port_entry_iter == new_south_if_stad_port_entries.end()) {  // not found
      // Directly to add deploy_if_stad_port_entry to add_if_stad_port_entries
      add_if_stad_port_entries.insert(deploy_if_stad_port_entry);
      continue;
    }

    QSet<ActStadPortEntry> add_stad_port_entries;
    for (auto deploy_stad_port_entry : deploy_if_stad_port_entry.GetStadPortEntries()) {  // for entry
      // Find idle index at new_south_if_stad_port_entries's stad_port_entries
      qint32 port_index;
      act_status = FindIdleStadPortIndex(device.GetDeviceProperty().GetStreamPriorityConfigIngressIndexMax(),
                                         new_south_if_stad_port_entry_iter->GetStadPortEntries(),
                                         deploy_stad_port_entry, port_index);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__
                    << QString("FindIdleStadPortIndex() Failed. Device:%1(%2)")
                           .arg(device.GetIpv4().GetIpAddress())
                           .arg(device.GetId())
                           .toStdString()
                           .c_str();
        return act_status;
      }
      deploy_stad_port_entry.SetIngressIndex(port_index);  // update IngressIndex
      add_stad_port_entries.insert(deploy_stad_port_entry);
      ActInterfaceStadPortEntry add_if_stad_port_entry(deploy_if_stad_port_entry.GetInterfaceId(),
                                                       add_stad_port_entries);
      add_if_stad_port_entries.insert(add_if_stad_port_entry);
    }
  }

  // Update add_stad_port_table
  add_remove_stad_port_table.SetAddStadPortTable(ActStadPortTable(device.GetId(), add_if_stad_port_entries));

  return act_status;
}
ACT_STATUS ActSouthbound::FindIdleStadPortIndex(const quint16 &ingress_index_max,
                                                const QSet<ActStadPortEntry> &stad_port_entries,
                                                const ActStadPortEntry &target_stad_port_entry, qint32 &ingress_index) {
  ACT_STATUS_INIT();

  for (qint32 index = 0; index < ingress_index_max; index++) {
    ActStadPortEntry find_stad_port_entry(target_stad_port_entry.GetPortId(), index);
    auto stad_port_entry_iter = stad_port_entries.find(find_stad_port_entry);
    if (stad_port_entry_iter != stad_port_entries.end()) {  // found , the index has be used

      if (index == ingress_index_max) {  // all indexes have been used.
        qCritical() << __func__ << "All StreamPriority's IngressIndexes have been used. Interface:"
                    << target_stad_port_entry.GetPortId();
        return std::make_shared<ActStatusInternalError>("Deploy");
      }
      // Continue to find next idle index
      continue;
    }

    // Not found would use this index
    ingress_index = index;
    break;
  }
  return act_status;
}
