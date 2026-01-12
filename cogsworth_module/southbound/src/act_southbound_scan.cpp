
#include "act_core.hpp"
#include "act_southbound.hpp"
#include "act_utilities.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

ACT_STATUS ActSouthbound::UpdateDeviceConnectByScanFeature(ActDevice &device) {
  ACT_STATUS_INIT();
  ActDeviceConnectStatusControl total_control(false, false, false, false);
  ActDeviceConnectStatusControl config_control(false, false, false, false);
  ActDeviceConnectStatusControl scan_control(false, false, false, false);

  // Get Configuration feature
  ActFeatureProfile config_feature_profile;
  auto config_status = GetDeviceFeature(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                        ActFeatureEnum::kConfiguration, config_feature_profile);
  if (!IsActStatusSuccess(config_status)) {
    qCritical() << __func__
                << QString("Device(%1) GetDeviceFeature(Configuration) failed.")
                       .arg(device.GetIpv4().GetIpAddress())
                       .toStdString()
                       .c_str();
  } else {
    // Find Feature used connect protocols
    GetFeaturesUsedConnectProtocol(config_feature_profile, config_control);
  }

  // Get AutoScan feature
  ActFeatureProfile auto_feature_profile;
  auto auto_status = GetDeviceFeature(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                      ActFeatureEnum::kAutoScan, auto_feature_profile);
  if (!IsActStatusSuccess(auto_status)) {
    qCritical() << __func__
                << QString("Device(%1) GetDeviceFeature(AutoScan) failed.")
                       .arg(device.GetIpv4().GetIpAddress())
                       .toStdString()
                       .c_str();
  } else {
    // Find Feature used connect protocols
    GetFeaturesUsedConnectProtocol(auto_feature_profile, scan_control);
  }

  // Aggregate control
  if (scan_control.GetRESTful() || config_control.GetRESTful()) {
    total_control.SetRESTful(true);
  }
  if (scan_control.GetSNMP() || config_control.GetSNMP()) {
    total_control.SetSNMP(true);
  }
  if (scan_control.GetNETCONF() || config_control.GetNETCONF()) {
    total_control.SetNETCONF(true);
  }
  if (scan_control.GetNewMOXACommand() || config_control.GetNewMOXACommand()) {
    total_control.SetNewMOXACommand(true);
  }

  // Update Device connect
  act_status = FeatureAssignDeviceStatus(false, device, total_control);
  if (!IsActStatusSuccess(act_status)) {
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::AssignDeviceConfigs(ActDevice &device, ActDeviceConfig &device_config) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;

  // Network Setting
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetNetworkSetting()) {
    ActNetworkSettingTable network_setting_table(device);
    device_config.GetNetworkSettingTables()[device.GetId()] = network_setting_table;
  }

  // // User Account
  // ActUserAccountTable user_account_table(device.GetId());
  // act_status = ScanUserAccountTable(device, user_account_table);
  // if (!IsActStatusSuccess(act_status)) {
  //   qCritical() << __func__ << "ScanUserAccountTable() failed.";
  // } else {
  //   device_config.GetUserAccountTables()[device.GetId()] = user_account_table;
  // }
  // if (stop_flag_) {
  //   return ACT_STATUS_STOP;
  // }

  // LoginPolicy
  ActLoginPolicyTable login_policy_table(device.GetId());
  act_status = ScanLoginPolicyTable(device, login_policy_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanLoginPolicyTable() failed.";
  } else {
    device_config.GetLoginPolicyTables()[device.GetId()] = login_policy_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Information Setting
  ActInformationSettingTable info_setting_table(device.GetId());
  act_status = ScanInformationSettingTable(device, info_setting_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanInformationSettingTable() failed.";
  } else {
    // Update Device's DeviceName & Location & Description
    device.SetDeviceName(info_setting_table.GetDeviceName());
    device.GetDeviceInfo().SetLocation(info_setting_table.GetLocation());
    device.GetDeviceProperty().SetDescription(info_setting_table.GetDescription());

    // Update DeviceConfig
    device_config.GetInformationSettingTables()[device.GetId()] = info_setting_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // ManagementInterface
  ActManagementInterfaceTable mgmt_interface_table(device.GetId());
  act_status = ScanManagementInterfaceTable(device, mgmt_interface_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanManagementInterfaceTable() failed.";
  } else {
    device_config.GetManagementInterfaceTables()[device.GetId()] = mgmt_interface_table;
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Loop Protection
  ActLoopProtectionTable lp_table(device.GetId());
  act_status = ScanLoopProtectionTable(device, lp_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanLoopProtectionTable() failed.";
  } else {
    device_config.GetLoopProtectionTables()[device.GetId()] = lp_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Snmp Trap setting
  ActSnmpTrapSettingTable snmp_trap_table(device.GetId());
  act_status = ScanSnmpTrapSettingTable(device, snmp_trap_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanSnmpTrapSettingTable() failed.";
  } else {
    device_config.GetSnmpTrapSettingTables()[device.GetId()] = snmp_trap_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Syslog setting
  ActSyslogSettingTable syslog_table(device.GetId());
  act_status = ScanSyslogSettingTable(device, syslog_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanSyslogSettingTable() failed.";
  } else {
    device_config.GetSyslogSettingTables()[device.GetId()] = syslog_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Time setting
  ActTimeSettingTable time_table(device.GetId());
  act_status = ScanTimeSettingTable(device, time_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanTimeSettingTable() failed.";
  } else {
    device_config.GetTimeSettingTables()[device.GetId()] = time_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Port setting
  ActPortSettingTable port_setting_table(device.GetId());
  act_status = ScanPortSettingTable(device, port_setting_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanPortSettingTable() failed.";
  } else {
    device_config.GetPortSettingTables()[device.GetId()] = port_setting_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // VLAN
  ActVlanTable vlan_table(device.GetId());
  act_status = ScanVlanTable(device, vlan_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanVlanTable() failed.";
  } else {
    device_config.GetVlanTables()[device.GetId()] = vlan_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // PCP
  ActDefaultPriorityTable pcp_table(device.GetId());
  act_status = ScanPortDefaultPCPTable(device, pcp_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanPortDefaultPCPTable() failed.";
  } else {
    device_config.GetPortDefaultPCPTables()[device.GetId()] = pcp_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // StreamPriority Ingress
  ActStadPortTable stad_port_table(device.GetId());
  act_status = ScanStreamPriorityIngressTable(device, stad_port_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanStreamPriorityIngressTable() failed.";
  } else {
    device_config.GetStreamPriorityIngressTables()[device.GetId()] = stad_port_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // StreamPriority Egress
  ActStadConfigTable stad_config_table(device.GetId());
  act_status = ScanStreamPriorityEgressTable(device, stad_config_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanStreamPriorityEgressTable() failed.";
  } else {
    device_config.GetStreamPriorityEgressTables()[device.GetId()] = stad_config_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // StaticForward Unicast
  ActStaticForwardTable unicast_static_forward_table(device.GetId());
  act_status = ScanUnicastStaticTable(device, unicast_static_forward_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanUnicastStaticTable() failed.";
  } else {
    device_config.GetUnicastStaticForwardTables()[device.GetId()] = unicast_static_forward_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // StaticForward Multicast
  ActStaticForwardTable multicast_static_forward_table(device.GetId());
  act_status = ScanMulticastStaticTable(device, multicast_static_forward_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanMulticastStaticTable() failed.";
  } else {
    device_config.GetMulticastStaticForwardTables()[device.GetId()] = multicast_static_forward_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // TimeAwareShaper
  ActGclTable gcl_table(device.GetId());
  act_status = ScanTimeAwareShaperTable(device, gcl_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanTimeAwareShaperTable() failed.";
  } else {
    device_config.GetGCLTables()[device.GetId()] = gcl_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // TimeSync Setting
  ActTimeSyncTable time_sync_table(device.GetId());
  act_status = ScanTimeSyncTable(device, time_sync_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanTimeSyncTable() failed.";
  } else {
    device_config.GetTimeSyncTables()[device.GetId()] = time_sync_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // RSTP
  ActRstpTable rstp_table(device.GetId());
  act_status = ScanRstpTable(device, rstp_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanRstpTable() failed.";
  } else {
    device_config.GetRstpTables()[device.GetId()] = rstp_table;
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::AssignDeviceLldpData(ActDevice &device) {
  ACT_STATUS_INIT();

  // Get LLDP Data
  ActLLDPData lldp_data;
  ActFeatureSubItem sub_item;

  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetLLDP()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "LLDP", "Basic", sub_item);

    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetLldpData(device, sub_item, lldp_data);
      if (stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetLldpData() failed.";
        return act_status;
      }
    }

    device.lldp_data_ = lldp_data;
  }

  return act_status;
}

ACT_STATUS ActSouthbound::AssignDeviceMacTable(ActDevice &device) {
  ACT_STATUS_INIT();

  // Get LLDP Data
  ActFeatureSubItem sub_item;
  QMap<qint64, QString> mac_table;  // <PortID, MAC> Port_MAC_map

  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetMACTable()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "DeviceInformation", "MACTable", sub_item);

    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetSingleEntryMacTable(device, sub_item, mac_table);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetSingleEntryMacTable() failed.";
        return act_status;
      }
    }

    device.single_entry_mac_table_ = mac_table;
  }

  return act_status;
}

ACT_STATUS ActSouthbound::AssignDeviceInterfacesInfo(ActDevice &device) {
  ACT_STATUS_INIT();

  QList<ActInterface> dev_if_list;

  // Use LLDPLocalPortID
  if (device.GetInterfaces().isEmpty()) {
    if (!device.lldp_data_.GetLocPortIdMap().isEmpty()) {
      ActInterfaceProperty default_interface_prop;
      for (auto if_id : device.lldp_data_.GetLocPortIdMap().keys()) {
        ActInterface new_interface(if_id);
        new_interface.SetDeviceId(device.GetId());
        new_interface.SetInterfaceName(QString::number(if_id));
        new_interface.SetSupportSpeeds(default_interface_prop.GetSupportSpeeds());
        device.GetInterfaces().append(new_interface);
      }
    }
  }

  // TODO: remove
  // [feat:3661] Remove fake MAC
  // // Get Interfaces MAC
  // QMap<qint64, QString> interface_mac_map;
  // if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetInterfaceMAC()) {
  //   ActFeatureSubItem if_mac_sub_item;
  //   act_status =
  //       GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
  //                               ActFeatureEnum::kAutoScan, "DeviceInformation", "InterfaceMAC", if_mac_sub_item);
  //   if (IsActStatusSuccess(act_status)) {
  //     act_status = ActionGetInterfaceMac(device, if_mac_sub_item, interface_mac_map);
  //     if (!IsActStatusSuccess(act_status)) {
  //       qDebug()
  //           << __func__
  //           << QString("Device(%1) get InterfaceMac
  //           failed").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();
  //     }
  //   }
  // }

  // // Find Interface & Set MAC
  // for (auto interface_id : interface_mac_map.keys()) {
  //   ActInterface target_interface(interface_id);
  //   auto interface_index = device.GetInterfaces().indexOf(target_interface);
  //   if (interface_index != -1) {  // find
  //     target_interface = device.GetInterfaces().at(interface_index);
  //     QString mac_addr = interface_mac_map[interface_id];
  //     target_interface.SetMacAddress(mac_addr);
  //     device.GetInterfaces()[interface_index] = target_interface;  //  update back to device
  //   }
  // }

  // // Interfaces is empty use southbound result
  // // Use InterfaceMAC or LLDPLocalPortID
  // if (device.GetInterfaces().isEmpty()) {
  //   ActInterfaceProperty default_interface_prop;
  //   if (!interface_mac_map.isEmpty()) {
  //     for (auto if_id : interface_mac_map.keys()) {
  //       ActInterface new_interface(if_id);
  //       new_interface.SetDeviceId(device.GetId());
  //       new_interface.SetMacAddress(interface_mac_map[if_id]);
  //       new_interface.SetInterfaceName(QString::number(if_id));
  //       new_interface.SetSupportSpeeds(default_interface_prop.GetSupportSpeeds());
  //       device.GetInterfaces().append(new_interface);
  //     }
  //   } else if (!device.lldp_data_.GetLocPortIdMap().isEmpty()) {
  //     for (auto if_id : device.lldp_data_.GetLocPortIdMap().keys()) {
  //       ActInterface new_interface(if_id);
  //       new_interface.SetDeviceId(device.GetId());
  //       new_interface.SetInterfaceName(QString::number(if_id));
  //       new_interface.SetSupportSpeeds(default_interface_prop.GetSupportSpeeds());
  //       device.GetInterfaces().append(new_interface);
  //     }
  //   }
  // }

  return ACT_STATUS_SUCCESS;
}

// ACT_STATUS ActSouthbound::AssignSFPtoModule(ActDevice &device,
//                                                          const QMap<qint64, ActMonitorFiberCheckEntry>
//                                                          &port_fiber_map, const QMap<qint64, ActDevicePortInfoEntry>
//                                                          &port_info_map) {
//   ACT_STATUS_INIT();

//   if (port_fiber_map.size() == 0) {
//     return ACT_STATUS_SUCCESS;
//   }

//   // Transfer ActMonitorFiberCheckEntry to ActSFPInfo
//   QMap<qint64, ActSFPInfo> port_sfp_map;
//   for (auto port_id : port_fiber_map.keys()) {
//     ActSFPInfo sfp_info(port_id);
//     sfp_info.SetInterfaceName(port_fiber_map[port_id].GetInterfaceName());
//     sfp_info.SetModelName(port_fiber_map[port_id].GetModelName());
//     sfp_info.SetSerialNumber(port_fiber_map[port_id].GetSerialNumber());
//     sfp_info.SetExist(port_fiber_map[port_id].GetExist());

//     // // Skip not exist SFP
//     // if (!sfp_info.GetExist()) {
//     //   continue;
//     // }
//     port_sfp_map[port_id] = sfp_info;
//   }

//   if (device.GetModularInfo().GetEthernet().isEmpty()) {
//     device.GetModularInfo().SetSFP(port_sfp_map);
//     return ACT_STATUS_SUCCESS;
//   }

//   // Assign SFP to Ethernet module
//   for (auto port_id : port_sfp_map.keys()) {
//     if (port_info_map.contains(port_id)) {
//       auto slot = port_info_map[port_id].GetModuleSlot();
//       if (device.GetModularInfo().GetEthernet().contains(slot)) {
//         device.GetModularInfo().GetEthernet()[slot].GetSFP()[port_id] = port_sfp_map[port_id];
//       } else {
//         qWarning() << __func__
//                    << QString("Device(%1) not found the slot %2 ethernet module")
//                           .arg(device.GetIpv4().GetIpAddress())
//                           .arg(slot)
//                           .toStdString()
//                           .c_str();
//       }
//     } else {
//       qWarning() << __func__
//                  << QString("Device(%1) not found the Port %2 info at the port_info_map")
//                         .arg(device.GetIpv4().GetIpAddress())
//                         .arg(port_id)
//                         .toStdString()
//                         .c_str();
//     }
//   }

//   return ACT_STATUS_SUCCESS;
// }

ACT_STATUS ActSouthbound::AssignDeviceIPv4(ActDevice &device) {
  ACT_STATUS_INIT();
  ActFeatureSubItem sub_item;

  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetIPConfiguration()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "DeviceInformation", "IPConfiguration", sub_item);
    ActIpv4 ipv4;
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetIPConfiguration(device, sub_item, ipv4);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetIPConfiguration() failed.";
        return act_status;
      }
      device.GetIpv4().SetGateway(ipv4.GetGateway());
      device.GetIpv4().SetSubnetMask(ipv4.GetSubnetMask());
      device.GetIpv4().SetDNS1(ipv4.GetDNS1());
      device.GetIpv4().SetDNS2(ipv4.GetDNS2());
    }
  }

  return act_status;
}

ACT_STATUS ActSouthbound::AssignDeviceName(ActDevice &device) {
  ACT_STATUS_INIT();
  ActFeatureSubItem sub_item;
  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetDeviceName()) {
    QString device_name;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "DeviceInformation", "DeviceName", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetDeviceName(device, sub_item, device_name);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetDeviceName() failed.";
        return act_status;
      }
      device.SetDeviceName(device_name);
    }
  }

  return act_status;
}

ACT_STATUS ActSouthbound::AssignDeviceSerialNumber(ActDevice &device) {
  ACT_STATUS_INIT();
  ActFeatureSubItem sub_item;

  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetSerialNumber()) {
    QString serial_number;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "DeviceInformation", "SerialNumber", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetSerialNumber(device, sub_item, serial_number);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetSerialNumber() failed.";
        return act_status;
      }
      device.GetDeviceInfo().SetSerialNumber(serial_number);
    }
  }

  return act_status;
}

ACT_STATUS ActSouthbound::AssignDeviceSystemUptime(ActDevice &device) {
  ACT_STATUS_INIT();
  ActFeatureSubItem sub_item;

  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetSystemUptime()) {
    QString uptime;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "DeviceInformation", "SystemUptime", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetSystemUptime(device, sub_item, uptime);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetSystemUptime() failed.";
        return act_status;
      }
      device.GetDeviceInfo().SetSystemUptime(uptime);
    }
  }

  return act_status;
}

ACT_STATUS ActSouthbound::AssignDeviceProductRevision(ActDevice &device) {
  ACT_STATUS_INIT();
  ActFeatureSubItem sub_item;

  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetProductRevision()) {
    QString revision;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "DeviceInformation", "ProductRevision", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetProductRevision(device, sub_item, revision);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetProductRevision() failed.";
        return act_status;
      }
      device.GetDeviceInfo().SetProductRevision(revision);
    }
  }

  return act_status;
}

ACT_STATUS ActSouthbound::AssignDeviceRedundantProtocol(ActDevice &device) {
  ACT_STATUS_INIT();
  ActFeatureSubItem sub_item;

  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetRedundantProtocol()) {
    QString protocol;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "DeviceInformation", "RedundantProtocol", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetRedundantProtocol(device, sub_item, protocol);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetRedundantProtocol() failed.";
        return act_status;
      }
      device.GetDeviceInfo().SetRedundantProtocol(protocol);
    }
  }

  return act_status;
}

ACT_STATUS ActSouthbound::AssignDeviceLocation(ActDevice &device) {
  ACT_STATUS_INIT();
  ActFeatureSubItem sub_item;

  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetLocation()) {
    QString location;
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "DeviceInformation", "Location", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetLocation(device, sub_item, location);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetLocation() failed.";
        return act_status;
      }
      device.GetDeviceInfo().SetLocation(location);
    }
  }

  return act_status;
}

ACT_STATUS ActSouthbound::AssignDeviceModularConfiguration(ActDevice &device) {
  ACT_STATUS_INIT();
  ActFeatureSubItem sub_item;
  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetModularInfo()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "DeviceInformation", "ModularInfo", sub_item);
    ActDeviceModularInfo modular_info;
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetModularInfo(device, sub_item, modular_info);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetModularInfo() failed.";
        return act_status;
      } else {
        // qInfo() << "modular_info(" << device.GetIpv4().GetIpAddress()
        //         << "):" << modular_info.ToString().toStdString().c_str();

        device.GetModularConfiguration().GetEthernet().clear();
        device.GetModularConfiguration().GetPower().clear();

        QMap<QString, qint64> orig_eth_module_name_id_map;
        QMap<QString, qint64> orig_power_module_name_id_map;
        act::core::g_core.GetEthernetModuleNameIdMap(orig_eth_module_name_id_map);
        act::core::g_core.GetPowerModuleNameIdMap(orig_power_module_name_id_map);

        // Transfer ModuleName to upper
        QMap<QString, qint64> eth_module_name_id_map;
        QMap<QString, qint64> power_module_name_id_map;
        for (auto it = orig_eth_module_name_id_map.constBegin(); it != orig_eth_module_name_id_map.constEnd(); ++it) {
          eth_module_name_id_map[it.key().toUpper()] = it.value();
        }
        for (auto it = orig_power_module_name_id_map.constBegin(); it != orig_power_module_name_id_map.constEnd();
             ++it) {
          power_module_name_id_map[it.key().toUpper()] = it.value();
        }

        // Filter no module Slot & builtin Ethernet Slot
        for (auto slot_id : modular_info.GetEthernet().keys()) {
          // Skip assign the builtin slot
          if (slot_id == ACT_BUILTIN_LINE_MODULE_SLOT) {
            continue;
          }

          if (!modular_info.GetEthernet()[slot_id].GetExist()) {
            continue;
          }

          auto module_name = modular_info.GetEthernet()[slot_id].GetModuleName().toUpper();
          if (eth_module_name_id_map.contains(module_name)) {
            device.GetModularConfiguration().GetEthernet()[slot_id] = eth_module_name_id_map[module_name];
          } else {
            qWarning() << __func__
                       << QString("Device(%1) Ethernet Module(%2:%3) not found")
                              .arg(device.GetIpv4().GetIpAddress())
                              .arg(slot_id)
                              .arg(module_name)
                              .toStdString()
                              .c_str();
          }
        }

        for (auto slot_id : modular_info.GetPower().keys()) {
          if (!modular_info.GetPower()[slot_id].GetExist()) {
            continue;
          }
          auto module_name = modular_info.GetPower()[slot_id].GetModuleName().toUpper();
          if (power_module_name_id_map.contains(module_name)) {
            device.GetModularConfiguration().GetPower()[slot_id] = power_module_name_id_map[module_name];
          } else {
            qWarning() << __func__
                       << QString("Device(%1) Power Module(%2:%3) not found")
                              .arg(device.GetIpv4().GetIpAddress())
                              .arg(slot_id)
                              .arg(module_name)
                              .toStdString()
                              .c_str();
          }
        }

        //  For manufacture mapping report
        device.modular_info_ = modular_info;
      }
    }
  }

  return act_status;
}

ACT_STATUS ActSouthbound::AssignInterfacesAndBuiltinPowerByModular(ActDevice &device) {
  ACT_STATUS_INIT();

  // Assign the Builtin Power & Interfaces
  ActProject tmp_project;
  act_status = act::core::g_core.MatchDeviceProfile(tmp_project, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Match device profile failed with device id:" << device.GetId();
    return act_status;
  }

  return act_status;
}

// ACT_STATUS ActSouthbound::AssignDeviceModularInfo(ActDevice &device) {
//   ACT_STATUS_INIT();
//   ActFeatureSubItem sub_item;
//   if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetModularInfo()) {
//     act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(),
//     profiles_.GetDeviceProfiles(),
//                                          ActFeatureEnum::kAutoScan, "DeviceInformation", "ModularInfo", sub_item);
//     ActDeviceModularInfo modular_info;
//     if (IsActStatusSuccess(act_status)) {
//       act_status = ActionGetModularInfo(device, sub_item, modular_info);
//       if (!IsActStatusSuccess(act_status)) {
//         qCritical() << __func__ << "ActionGetModularInfo() failed.";
//         return act_status;
//       } else {
//         device.SetModularInfo(modular_info);
//       }
//     }
//   }

//   return act_status;
// }

ACT_STATUS ActSouthbound::ScanFiberCheck(const ActDevice &device,
                                         QMap<qint64, ActMonitorFiberCheckEntry> &port_fiber_map) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  port_fiber_map.clear();

  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetBasicStatus().GetFiberCheck()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kMonitor, "BasicStatus", "FiberCheck", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetFiberCheck(device, sub_item, port_fiber_map);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetFiberCheck() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanPortInfo(const ActDevice &device, QMap<qint64, ActDevicePortInfoEntry> &port_info_map) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  port_info_map.clear();

  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetPortInfo()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "DeviceInformation", "PortInfo", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetPortInfo(device, sub_item, port_info_map);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetPortInfo() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanNetworkSettingTable(const ActDevice &device,
                                                  ActNetworkSettingTable &result_network_setting) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_network_setting.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetIPConfiguration()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "DeviceInformation", "IPConfiguration", sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActIpv4 ipv4;
      act_status = ActionGetIPConfiguration(device, sub_item, ipv4);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetLoopProtection() failed.";
        return act_status;
      }

      result_network_setting.SetIpAddress(ipv4.GetIpAddress());
      result_network_setting.SetSubnetMask(ipv4.GetSubnetMask());
      result_network_setting.SetGateway(ipv4.GetGateway());
      result_network_setting.SetDNS1(ipv4.GetDNS1());
      result_network_setting.SetDNS2(ipv4.GetDNS2());
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanUserAccountTable(const ActDevice &device, ActUserAccountTable &result_user_account) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_user_account.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetUserAccount()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "UserAccount", "Basic", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetUserAccount(device, sub_item, result_user_account);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetUserAccount() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanLoginPolicyTable(const ActDevice &device, ActLoginPolicyTable &result_login_policy) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_login_policy.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetLoginPolicy()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "LoginPolicy", "Basic", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetLoginPolicy(device, sub_item, result_login_policy);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetLoginPolicy() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanManagementInterfaceTable(const ActDevice &device,
                                                       ActManagementInterfaceTable &result_mgmt_interface) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_mgmt_interface.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetManagementInterface()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "ManagementInterface", "Basic", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetManagementInterface(device, sub_item, result_mgmt_interface);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetManagementInterface() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanSnmpTrapSettingTable(const ActDevice &device,
                                                   ActSnmpTrapSettingTable &result_snmp_trap_setting) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_snmp_trap_setting.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSNMPTrapSetting()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "SNMPTrapSetting", "Basic", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetSnmpTrapSetting(device, sub_item, result_snmp_trap_setting);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetSnmpTrapSetting() failed.";
        return act_status;
      }
    }
  }

  // qDebug() << "result_snmp_trap_setting(" << device.GetIpv4().GetIpAddress()
  //          << "):" << result_snmp_trap_setting.ToString().toStdString().c_str();

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanSyslogSettingTable(const ActDevice &device,
                                                 ActSyslogSettingTable &result_syslog_setting) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_syslog_setting.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSyslogSetting()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "SyslogSetting", "Basic", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetSyslogSetting(device, sub_item, result_syslog_setting);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetSyslogSetting() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanLoopProtectionTable(const ActDevice &device,
                                                  ActLoopProtectionTable &result_loop_protection) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_loop_protection.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetLoopProtection()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "LoopProtection", "Basic", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetLoopProtection(device, sub_item, result_loop_protection);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetLoopProtection() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanInformationSettingTable(const ActDevice &device,
                                                      ActInformationSettingTable &result_info_setting) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_info_setting.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetInformationSetting()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "InformationSetting", "Basic", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetInformationSetting(device, sub_item, result_info_setting);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetInformationSetting() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanTimeSettingTable(const ActDevice &device, ActTimeSettingTable &result_time_setting) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_time_setting.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSetting().GetSystemTime()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "TimeSetting", "Basic", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetTimeSetting(device, sub_item, result_time_setting);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetTimeSetting() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanPortSettingTable(const ActDevice &device, ActPortSettingTable &result_port_setting) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_port_setting.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetPortSetting().GetAdminStatus()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "PortSetting", "AdminStatus", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetPortSettingAdminStatus(device, sub_item, result_port_setting);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetPortSettingAdminStatus() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanVlanTable(const ActDevice &device, ActVlanTable &result_vlan_table) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_vlan_table.SetDeviceId(device.GetId());

  // VLAN configuration (VLAN PortType, PVID, VLAN Static)
  // VLAN PortType
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetAccessTrunkMode()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "VLANSetting", "AccessTrunkMode", sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActVlanPortTypeTable vlan_port_type_table(device.GetId());
      act_status = ActionGetVLANPortType(device, sub_item, vlan_port_type_table);
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__ << "ActionGetVLANPortType() failed.";
        return act_status;
      } else {  // update result
        result_vlan_table.SetVlanPortTypeEntries(vlan_port_type_table.GetVlanPortTypeEntries());
      }
    }
  }
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // PVID
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetDefaultPVID()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "VLANSetting", "DefaultPVID", sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActPortVlanTable port_vlan_table(device.GetId());
      act_status = ActionGetPortPVID(device, sub_item, port_vlan_table);
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__ << "ActionGetPortPVID() failed.";
        return act_status;
      } else {  // update result
        result_vlan_table.SetPortVlanEntries(port_vlan_table.GetPortVlanEntries());
      }
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // VLAN Static
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetAccessTrunkMode()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "VLANSetting", "VLANMethod", sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActVlanStaticTable vlan_static_table(device.GetId());
      act_status = ActionGetVLAN(device, sub_item, vlan_static_table);
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__ << "ActionGetPortPVID() failed.";
        return act_status;
      } else {  // update result
        result_vlan_table.SetVlanStaticEntries(vlan_static_table.GetVlanStaticEntries());
      }
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // Management VLAN
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetManagementVLAN()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "VLANSetting", "ManagementVLAN", sub_item);
    if (IsActStatusSuccess(act_status)) {
      qint32 mgmt_vlan = 0;
      act_status = ActionGetManagementVlan(device, sub_item, mgmt_vlan);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetManagementVlan() failed.";
        return act_status;
      } else {  // update result
        result_vlan_table.SetManagementVlan(mgmt_vlan);
      }
    }
  }

  // qDebug() << "result_vlan_table(" << device.GetIpv4().GetIpAddress()
  //          << "):" << result_vlan_table.ToString().toStdString().c_str();
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanPortDefaultPCPTable(const ActDevice &device,
                                                  ActDefaultPriorityTable &result_default_priority_table) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_default_priority_table.SetDeviceId(device.GetId());
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetDefaultPCP()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "VLANSetting", "DefaultPCP", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetPortDefaultPCP(device, sub_item, result_default_priority_table);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetPortDefaultPCP() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanStreamPriorityIngressTable(const ActDevice &device,
                                                         ActStadPortTable &result_stad_port_table) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_stad_port_table.SetDeviceId(device.GetId());
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetPerStreamPriorityV2()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "VLANSetting", "PerStreamPriorityV2", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetStreamPriorityIngress(device, sub_item, result_stad_port_table);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetStreamPriorityIngress() failed.";
        return act_status;
      }
    }
  } else if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetPerStreamPriority()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "VLANSetting", "PerStreamPriority", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetStreamPriorityIngress(device, sub_item, result_stad_port_table);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetStreamPriorityIngress() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanStreamPriorityEgressTable(const ActDevice &device,
                                                        ActStadConfigTable &result_stad_config_table) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_stad_config_table.SetDeviceId(device.GetId());
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetPerStreamPriorityV2()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "VLANSetting", "PerStreamPriorityV2", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetStreamPriorityEgress(device, sub_item, result_stad_config_table);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetStreamPriorityEgress() failed.";
        return act_status;
      }
    }
  } else if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetPerStreamPriority()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "VLANSetting", "PerStreamPriority", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetStreamPriorityEgress(device, sub_item, result_stad_config_table);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetStreamPriorityEgress() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanUnicastStaticTable(const ActDevice &device,
                                                 ActStaticForwardTable &result_static_forward) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_static_forward.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetStaticForwardSetting().GetUnicast()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "StaticForwardSetting", "Unicast", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetStaticUnicast(device, sub_item, result_static_forward);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetStaticUnicast() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanMulticastStaticTable(const ActDevice &device,
                                                   ActStaticForwardTable &result_static_forward) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_static_forward.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetStaticForwardSetting().GetMulticast()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "StaticForwardSetting", "Multicast", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetStaticMulticast(device, sub_item, result_static_forward);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetStaticMulticast() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanTimeAwareShaperTable(const ActDevice &device, ActGclTable &result_gcl_table) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_gcl_table.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTSN().GetIEEE802Dot1Qbv()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "TSN", "IEEE802Dot1Qbv", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGet802Dot1Qbv(device, sub_item, result_gcl_table);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGet802Dot1Qbv() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanRstpTable(const ActDevice &device, ActRstpTable &result_rstp_table) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_rstp_table.SetDeviceId(device.GetId());
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetRSTP()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "STPRSTP", "RSTPMethod", sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = ActionGetSpanningTree(device, sub_item, result_rstp_table);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetSpanningTree() failed.";
        return act_status;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::ScanTimeSyncTable(const ActDevice &device, ActTimeSyncTable &result_time_sync_table) {
  ACT_STATUS_INIT();

  ActFeatureSubItem sub_item;
  result_time_sync_table.SetDeviceId(device.GetId());

  // TimeSync Setting:
  // - Base Config(Enable, Profile)
  // - IEEE_802Dot1AS_2011
  // - IEEE_1588_2008
  // - IEC_61850_2016
  // - IEEE_C37Dot238_2017

  // Base Config(Enable, Profile)
  auto time_sync_setting_item = device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSyncSetting();
  if (time_sync_setting_item.CheckSupportAnyOne()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "TimeSyncSetting", "TimeSyncMethod", sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActTimeSyncBaseConfig time_sync_base_config;
      act_status = ActionGetTimeSyncBaseConfig(device, sub_item, time_sync_base_config);
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__ << "ActionGetTimeSyncBaseConfig() failed.";
        return act_status;
      } else {  // update result
        result_time_sync_table.SetEnabled(time_sync_base_config.GetEnabled());
        result_time_sync_table.SetProfile(time_sync_base_config.GetProfile());
      }
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // IEEE_802Dot1AS_2011
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSyncSetting().GetIEEE802Dot1AS_2011()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "TimeSyncSetting", "IEEE802Dot1AS_2011", sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActTimeSync802Dot1ASConfig time_sync_config;
      act_status = ActionGetTimeSync802Dot1ASConfig(device, sub_item, time_sync_config);
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__ << "ActionGetTimeSync802Dot1ASConfig() failed.";
        return act_status;
      } else {  // update result
        result_time_sync_table.SetIEEE_802Dot1AS_2011(time_sync_config);
      }
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // IEEE_1588_2008
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSyncSetting().GetIEEE1588_2008()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "TimeSyncSetting", "IEEE1588_2008", sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActTimeSync1588Config time_sync_config;
      act_status = ActionGetTimeSync1588Config(device, sub_item, time_sync_config);
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__ << "ActionGetTimeSync1588Config() failed.";
        return act_status;
      } else {  // update result
        result_time_sync_table.SetIEEE_1588_2008(time_sync_config);
      }
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // IEC_61850_2016
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSyncSetting().GetIEC61850_2016()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kConfiguration, "TimeSyncSetting", "IEC61850_2016", sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActTimeSyncIec61850Config time_sync_config;
      act_status = ActionGetTimeSyncIec61850Config(device, sub_item, time_sync_config);
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__ << "ActionGetTimeSyncIec61850Config() failed.";
        return act_status;
      } else {  // update result
        result_time_sync_table.SetIEC_61850_2016(time_sync_config);
      }
    }
  }

  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  // IEEE_C37Dot238_2017
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSyncSetting().GetIEEEC37Dot238_2017()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kConfiguration, "TimeSyncSetting", "IEEEC37Dot238_2017", sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActTimeSyncC37Dot238Config time_sync_config;
      act_status = ActionGetTimeSyncC37Dot238Config(device, sub_item, time_sync_config);
      if (!IsActStatusSuccess(act_status)) {
        qWarning() << __func__ << "ActionGetTimeSyncC37Dot238Config() failed.";
        return act_status;
      } else {  // update result
        result_time_sync_table.SetIEEE_C37Dot238_2017(time_sync_config);
      }
    }
  }

  // qDebug() << "result_time_sync_table(" << device.GetIpv4().GetIpAddress()
  //          << "):" << result_time_sync_table.ToString().toStdString().c_str();
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::IdentifyDevice(ActDevice &device) {
  ACT_STATUS_INIT();

  // [feat:2396] Refactor - AutoScan performance enhance
  ActDeviceProfile device_profile;
  ActFirmwareFeatureProfile fw_feature_profile;
  QString firmware;
  act_status = FeatureIdentifyDeviceAndGetProfiles(true, device, profiles_, device_profile, fw_feature_profile,
                                                   firmware);  // function would update device's FirmwareVersion
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__
                << QString("FeatureIdentifyDeviceAndGetProfiles() failed. Device(%1)")
                       .arg(device.GetIpv4().GetIpAddress())
                       .toStdString()
                       .c_str();
    return act_status;
  }
  // qDebug() << __func__
  //          << QString("Device(%1) find DeviceProfile(%2), FirmwareFeatureProfile(%3) ")
  //                 .arg(device.GetIpv4().GetIpAddress())
  //                 .arg(device_profile.GetId())
  //                 .arg(fw_feature_profile.GetId())
  //                 .toStdString()
  //                 .c_str();

  // Set Device DeviceProfileId
  device.SetDeviceProfileId(device_profile.GetId());

  // Set Device FirmwareFeatureProfileId
  device.SetFirmwareFeatureProfileId(fw_feature_profile.GetId());

  // Set Device FirmwareVersion
  device.SetFirmwareVersion(firmware);

  // Set DeviceType
  device.SetDeviceType(device_profile.GetDeviceType());

  // Set DeviceName by DeviceProfile
  device.SetDeviceName(device_profile.GetDeviceName());

  // Set Firmware version to device configuration
  device.SetFirmwareVersion(firmware);

  // Set FeatureGroup
  if (fw_feature_profile.GetId() == -1) {  // not found
    device.GetDeviceProperty().SetFeatureGroup(device_profile.GetFeatureGroup());
  } else {
    device.GetDeviceProperty().SetFeatureGroup(fw_feature_profile.GetFeatureGroup());
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSouthbound::CreateDeviceLink(const QMap<QString, QString> &ip_mac_table, const ActDevice &device,
                                           const QSet<ActDevice> &alive_devices, ActScanLinksResult &scan_link_result) {
  ACT_STATUS_INIT();

  scan_link_result.GetUpdateDevices().clear();
  scan_link_result.GetScanLinks().clear();

  // Get device's links (use lldp & mac_table)
  QSet<ActLink> south_links;

  // Check FeatureGroup >> LLDP
  if (!device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetLLDP()) {
    return ACT_STATUS_SUCCESS;
  }

  // - Generate links by Device's LLDP data
  ActScanLinksResult lldp_scan_links_result;
  act_status = GenerateLinkSetBylldpInfo(device, alive_devices.values(), lldp_scan_links_result);
  if (!IsActStatusSuccess(act_status)) {
    qCritical()
        << __func__
        << QString("Device(%1) ScanLinksByLLDP failed").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();
    return act_status;
  }
  south_links = lldp_scan_links_result.GetScanLinks();
  // Assign the unknown LLDP Device to result update_devices
  scan_link_result.SetUpdateDevices(lldp_scan_links_result.GetUpdateDevices());

  // Use switch mac_table to get not lldp links (optional)
  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetMACTable()) {
    ActScanLinksResult mac_scan_links_result;
    // - Generate links by Device's MAC table
    act_status =
        GenerateLinkSetByMacTable(device, alive_devices.values(), south_links, ip_mac_table, mac_scan_links_result);

    if (!IsActStatusSuccess(act_status)) {
      qDebug() << __func__
               << QString("Device(%1) ScanLinksByMacTable failed")
                      .arg(device.GetIpv4().GetIpAddress())
                      .toStdString()
                      .c_str();
    } else {
      // Assign the unknown MACtable Device to result_update_devices
      // The Device's interface has been assign "Port"
      for (auto update_device : mac_scan_links_result.GetUpdateDevices()) {
        scan_link_result.GetUpdateDevices().remove(update_device);
        scan_link_result.GetUpdateDevices().insert(update_device);
      }

      for (auto south_link : mac_scan_links_result.GetScanLinks()) {
        south_links.insert(south_link);
      }
    }
  }

  // Check link hasn't same Source & Destination.
  // [bugfix:1729] Auto Scan - The response is wrong when the topology has the same IP devices.
  for (auto south_link : south_links) {
    if (south_link.GetSourceDeviceId() == south_link.GetDestinationDeviceId()) {
      qCritical() << __func__
                  << QString("Device(%1) get the same Source & Destination link. Link: %2")
                         .arg(device.GetIpv4().GetIpAddress())
                         .arg(south_link.ToString())
                         .toStdString()
                         .c_str();
      return std::make_shared<ActDuplicatedError>(QString("Device IP %1").arg(device.GetIpv4().GetIpAddress()));
    }
  }

  // Get Speed (Optional)
  QMap<qint64, qint64> south_port_speed_map;
  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetPortSpeed()) {
    // - Get Sub-Item (DeviceInformation  > PortSpeed)

    ActFeatureSubItem port_speed_sub_item;
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kAutoScan, "DeviceInformation", "PortSpeed", port_speed_sub_item);

    if (IsActStatusSuccess(act_status)) {  // (Optional)
      // - Access Speed
      act_status = ActionGetPortSpeed(device, port_speed_sub_item, south_port_speed_map);
      if (!IsActStatusSuccess(act_status)) {
        qDebug()
            << __func__
            << QString("Device(%1) GetPortSpeed failed").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();
      }
    }
  }

  // Assign linkInfo
  for (auto south_link : south_links) {
    // Assign port speed
    // If the device returns one port would set all ports same speed
    if (south_port_speed_map.size() == 1) {
      south_link.SetSpeed(south_port_speed_map.begin().value());
    } else {
      // Assign port speed by link direction
      if (south_link.GetDestinationDeviceId() == device.GetId()) {
        south_link.SetSpeed(south_port_speed_map[south_link.GetDestinationInterfaceId()]);
      } else if (south_link.GetSourceDeviceId() == device.GetId()) {
        south_link.SetSpeed(south_port_speed_map[south_link.GetSourceInterfaceId()]);
      }
    }

    // Re-assign id to alive_links
    south_link.SetId(scan_link_result.GetScanLinks().size() + 1);
    // qDebug() << __func__ << "South link:" << south_link.ToString().toStdString().c_str();

    // Insert to Result links
    scan_link_result.GetScanLinks().insert(south_link);

    // Update UpdateDevice Interface speed
    ActDevice target_device;
    // Destination Device
    auto found_status = ActGetItemById<ActDevice>(scan_link_result.GetUpdateDevices(),
                                                  south_link.GetDestinationDeviceId(), target_device);
    if (IsActStatusSuccess(found_status)) {
      // Find Interface
      ActInterface target_interface(south_link.GetDestinationInterfaceId());
      auto interface_index = target_device.GetInterfaces().indexOf(target_interface);
      if (interface_index != -1) {
        target_interface = target_device.GetInterfaces().at(interface_index);
        if (!target_interface.GetSupportSpeeds().contains(south_link.GetSpeed())) {
          target_interface.GetSupportSpeeds().append(south_link.GetSpeed());
          target_device.GetInterfaces()[interface_index] = target_interface;  //  update back to device
          scan_link_result.GetUpdateDevices().remove(target_device);          // update back to result
          scan_link_result.GetUpdateDevices().insert(target_device);
        }
      } else {
        QString not_found_ele = QString("Update Device(%1) > interface(%2)")
                                    .arg(target_device.GetId())
                                    .arg(target_interface.GetInterfaceId());
        qCritical() << __func__ << "Not found " << not_found_ele;
        return std::make_shared<ActStatusNotFound>(not_found_ele);
      }
    }

    // Source Device
    found_status =
        ActGetItemById<ActDevice>(scan_link_result.GetUpdateDevices(), south_link.GetSourceDeviceId(), target_device);
    if (IsActStatusSuccess(found_status)) {
      // Find Interface
      ActInterface target_interface(south_link.GetSourceInterfaceId());
      auto interface_index = target_device.GetInterfaces().indexOf(target_interface);
      if (interface_index != -1) {
        target_interface = target_device.GetInterfaces().at(interface_index);
        if (!target_interface.GetSupportSpeeds().contains(south_link.GetSpeed())) {
          target_interface.GetSupportSpeeds().append(south_link.GetSpeed());
          target_device.GetInterfaces()[interface_index] = target_interface;  // update back to device
          scan_link_result.GetUpdateDevices().remove(target_device);          // update back to result
          scan_link_result.GetUpdateDevices().insert(target_device);
        }
      } else {
        QString not_found_ele = QString("Update Device(%1) > interface(%2)")
                                    .arg(target_device.GetId())
                                    .arg(target_interface.GetInterfaceId());
        qCritical() << __func__ << "Not found " << not_found_ele;
        return std::make_shared<ActStatusNotFound>(not_found_ele);
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}
