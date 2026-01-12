#include <QHostAddress>
#include <QRegExp>
#include <QRegExpValidator>
#include <QSet>

#include "act_core.hpp"
#include "act_device.hpp"
#include "act_device_profile.hpp"
#include "deploy_entry/act_deploy_table.hpp"

namespace act {
namespace core {

extern QSet<ActDeviceCoordinate>
    g_dev_coordinate_set;  // device coordinate set, prevent coordinate conflict w/ monitor device status update

ACT_STATUS ActCore::CheckDevice(ActProject &project, ActDevice &device) {
  ACT_STATUS_INIT();
  static_cast<void>(project);

  // Check IP address format
  QHostAddress ip_address(device.GetIpv4().GetIpAddress());
  if (ip_address.isNull()) {
    QString error_msg = QString("The IP address is not valid: %1").arg(ip_address.toString());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // [bugfix:2148] 0.0.0.0 & 255.255.255.255 IP address is invalid
  if (ip_address.toString() == "0.0.0.0" || ip_address.toString() == "255.255.255.255") {
    QString error_msg = QString("The IP address cannot be 0.0.0.0 or 255.255.255.255");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // XX-XX-XX-XX-XX-XX or XX:XX:XX:XX:XX:XX
  QRegularExpression re("([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$");
  for (ActInterface intf : device.GetInterfaces()) {
    if (!intf.GetActive()) {
      continue;
    }

    act_status = CheckMacAddress(intf.GetMacAddress());
    if (!IsActStatusSuccess(act_status)) {
      QString error_msg = QString("Device (%1) - Interface(%2) - %3")
                              .arg(device.GetIpv4().GetIpAddress())
                              .arg(intf.GetInterfaceName())
                              .arg(act_status->GetErrorMessage());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    if (intf.GetSupportSpeeds().isEmpty()) {
      QString error_msg = QString("Device (%1) - Interface(%2) - Support speed is empty.")
                              .arg(device.GetIpv4().GetIpAddress())
                              .arg(intf.GetInterfaceName());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Check connection parameter
  act_status = this->CheckConnectionConfigField(device, QString("Device (%1)").arg(ip_address.toString()));

  return act_status;
}

static void UpdateDeviceConfigTable(ActProject &project, ActDevice &device,
                                    const QSet<ActDeviceProfile> &device_profiles) {
  qint64 device_id = device.GetId();
  ActDeviceConfig &device_config = project.GetDeviceConfig();

  // Get Default Config in the DeviceProfile
  ActDeviceProfile device_profile;
  ActGetItemById<ActDeviceProfile>(device_profiles, device.GetDeviceProfileId(), device_profile);
  auto default_device_config = device_profile.GetDefaultDeviceConfig();

  QSet<qint64> port_ids;
  for (ActInterface interface : device.GetInterfaces()) {
    port_ids.insert(interface.GetInterfaceId());
  }

  // TopologyMapping IP setting
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetNetworkSetting()) {
    device_config.GetMappingDeviceIpSettingTables().remove(device_id);
  }

  // NetworkSetting
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetNetworkSetting()) {
    device_config.GetNetworkSettingTables().remove(device_id);
  } else {
    ActNetworkSettingTable &network_setting = device_config.GetNetworkSettingTables()[device_id];
    network_setting.SetDeviceId(device_id);
    network_setting.SetIpAddress(device.GetIpv4().GetIpAddress());
    network_setting.SetSubnetMask(device.GetIpv4().GetSubnetMask());
    network_setting.SetGateway(device.GetIpv4().GetGateway());
    network_setting.SetDNS1(device.GetIpv4().GetDNS1());
    network_setting.SetDNS2(device.GetIpv4().GetDNS2());
  }

  // UserAccount
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetUserAccount()) {
    device_config.GetUserAccountTables().remove(device_id);
  } else if (!device_config.GetUserAccountTables().contains(device_id)) {
    ActUserAccount user_account;
    ActUserAccountTable user_account_table;
    if (!device_profile.GetDefaultDeviceConfig().GetUserAccountTables().isEmpty()) {
      user_account_table = device_profile.GetDefaultDeviceConfig().GetUserAccountTables()[0];
    } else {
      user_account_table.GetAccounts().insert(user_account.GetUsername(), user_account);
    }
    user_account_table.SetSyncConnectionAccount(user_account_table.GetAccounts().keys().first());
    device_config.GetUserAccountTables().insert(device_id, user_account_table);
  }

  // TimeSync
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSyncSetting().CheckSupportAnyOne()) {
    device_config.GetTimeSyncTables().remove(device_id);
  } else {
    // Insert default config from the device profile when table as empty
    if (!device_config.GetTimeSyncTables().contains(device.GetId())) {
      if (!default_device_config.GetTimeSyncTables().isEmpty()) {
        device_config.GetTimeSyncTables()[device_id] = default_device_config.GetTimeSyncTables().first();
      }
    }

    ActTimeSyncTable &time_sync_table = device_config.GetTimeSyncTables()[device_id];
    time_sync_table.SetDeviceId(device_id);

    // IEEE_802Dot1AS_2011
    if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSyncSetting().GetIEEE802Dot1AS_2011()) {
      QSet<ActTimeSync802Dot1ASPortEntry> default_802dot1as_entries =
          time_sync_table.GetIEEE_802Dot1AS_2011().GetPortEntries();
      QSet<ActTimeSync802Dot1ASPortEntry> new_default_802dot1as_entries;
      for (qint64 port_id : port_ids) {
        ActTimeSync802Dot1ASPortEntry default_802dot1as_entry(port_id);
        QSet<ActTimeSync802Dot1ASPortEntry>::iterator default_802dot1as_entry_iter =
            default_802dot1as_entries.find(default_802dot1as_entry);
        if (default_802dot1as_entry_iter != default_802dot1as_entries.end()) {
          default_802dot1as_entry = *default_802dot1as_entry_iter;
        }
        new_default_802dot1as_entries.insert(default_802dot1as_entry);
      }
      time_sync_table.GetIEEE_802Dot1AS_2011().SetPortEntries(new_default_802dot1as_entries);
    } else {
      time_sync_table.GetIEEE_802Dot1AS_2011().GetPortEntries().clear();
    }

    // IEEE_1588_2008
    if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSyncSetting().GetIEEE1588_2008()) {
      QSet<ActTimeSync1588PortEntry> default_1588_entries = time_sync_table.GetIEEE_1588_2008().GetPortEntries();
      QSet<ActTimeSync1588PortEntry> new_default_1588_entries;
      for (qint64 port_id : port_ids) {
        ActTimeSync1588PortEntry default_1588_entry(port_id);
        QSet<ActTimeSync1588PortEntry>::iterator default_1588_entry_iter =
            default_1588_entries.find(default_1588_entry);
        if (default_1588_entry_iter != default_1588_entries.end()) {
          default_1588_entry = *default_1588_entry_iter;
        }
        new_default_1588_entries.insert(default_1588_entry);
      }
      time_sync_table.GetIEEE_1588_2008().SetPortEntries(new_default_1588_entries);
    } else {
      time_sync_table.GetIEEE_1588_2008().GetPortEntries().clear();
    }

    // IEC_61850_2016
    if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSyncSetting().GetIEC61850_2016()) {
      QSet<ActTimeSyncDefaultPortEntry> default_61850_entries = time_sync_table.GetIEC_61850_2016().GetPortEntries();
      QSet<ActTimeSyncDefaultPortEntry> new_default_61850_entries;
      for (qint64 port_id : port_ids) {
        ActTimeSyncDefaultPortEntry default_61850_entry(port_id);
        QSet<ActTimeSyncDefaultPortEntry>::iterator default_61850_entry_iter =
            default_61850_entries.find(default_61850_entry);
        if (default_61850_entry_iter != default_61850_entries.end()) {
          default_61850_entry = *default_61850_entry_iter;
        }
        new_default_61850_entries.insert(default_61850_entry);
      }
      time_sync_table.GetIEC_61850_2016().SetPortEntries(new_default_61850_entries);
    } else {
      time_sync_table.GetIEC_61850_2016().GetPortEntries().clear();
    }

    // IEEE_C37Dot238_2017
    if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSyncSetting().GetIEEEC37Dot238_2017()) {
      QSet<ActTimeSyncDefaultPortEntry> default_c37dot238_entries =
          time_sync_table.GetIEEE_C37Dot238_2017().GetPortEntries();
      QSet<ActTimeSyncDefaultPortEntry> new_default_c37dot238_entries;
      for (qint64 port_id : port_ids) {
        ActTimeSyncDefaultPortEntry default_c37dot238_entry(port_id);
        QSet<ActTimeSyncDefaultPortEntry>::iterator default_c37dot238_entry_iter =
            default_c37dot238_entries.find(default_c37dot238_entry);
        if (default_c37dot238_entry_iter != default_c37dot238_entries.end()) {
          default_c37dot238_entry = *default_c37dot238_entry_iter;
        }
        new_default_c37dot238_entries.insert(default_c37dot238_entry);
      }
      time_sync_table.GetIEEE_C37Dot238_2017().SetPortEntries(new_default_c37dot238_entries);
    } else {
      time_sync_table.GetIEEE_C37Dot238_2017().GetPortEntries().clear();
    }
  }

  // LoginPolicy
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetLoginPolicy()) {
    device_config.GetLoginPolicyTables().remove(device_id);
  }

  // LoopProtection
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetLoopProtection()) {
    device_config.GetLoopProtectionTables().remove(device_id);
  }

  // SNMP Trap server setting
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSNMPTrapSetting()) {
    device_config.GetSnmpTrapSettingTables().remove(device_id);
  }

  // Syslog server setting
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSyslogSetting()) {
    device_config.GetSyslogSettingTables().remove(device_id);
  }

  // Time setting
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTimeSetting().GetSystemTime()) {
    device_config.GetTimeSettingTables().remove(device_id);
  }

  // InformationSetting
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetInformationSetting()) {
    device_config.GetInformationSettingTables().remove(device_id);
  }

  // ManagementInterface
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetManagementInterface()) {
    device_config.GetManagementInterfaceTables().remove(device_id);
  }

  // PortSetting
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetPortSetting().GetAdminStatus()) {
    device_config.GetPortSettingTables().remove(device_id);
  } else {
    // Insert default config from the device profile when table as empty
    if (!device_config.GetPortSettingTables().contains(device.GetId())) {
      if (!default_device_config.GetPortSettingTables().isEmpty()) {
        auto cfg = default_device_config.GetPortSettingTables().first();
        cfg.SetDeviceId(device_id);
        device_config.GetPortSettingTables()[device_id] = cfg;
      }
    }

    ActPortSettingTable &port_setting_table = device_config.GetPortSettingTables()[device_id];
    port_setting_table.SetDeviceId(device_id);
    QSet<ActPortSettingEntry> port_setting_entries = port_setting_table.GetPortSettingEntries();
    QSet<ActPortSettingEntry> new_port_setting_entries;
    for (qint64 port_id : port_ids) {
      ActPortSettingEntry port_setting_entry(port_id);
      QSet<ActPortSettingEntry>::iterator port_setting_entry_iter = port_setting_entries.find(port_setting_entry);
      if (port_setting_entry_iter != port_setting_entries.end()) {
        port_setting_entry = *port_setting_entry_iter;
      }
      new_port_setting_entries.insert(port_setting_entry);
    }
    port_setting_table.SetPortSettingEntries(new_port_setting_entries);
  }

  // VLAN (VlanStatic & PortType & PVID)
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().CheckSupportAnyOne()) {
    device_config.GetVlanTables().remove(device_id);
  } else {
    QMap<qint64, ActVlanTable> &vlan_tables = device_config.GetVlanTables();
    if (!vlan_tables.contains(device_id)) {
      if (!default_device_config.GetVlanTables().isEmpty()) {
        // Insert default config from the device profile
        auto cfg = default_device_config.GetVlanTables().first();
        cfg.SetDeviceId(device_id);
        vlan_tables[device_id] = cfg;
      } else {
        ActVlanTable vlan_table(device_id);
        ActVlanStaticEntry vlan_static_entry(vlan_table.GetManagementVlan());
        for (qint64 port_id : port_ids) {
          vlan_static_entry.GetEgressPorts().insert(port_id);
          vlan_static_entry.GetUntaggedPorts().insert(port_id);

          vlan_table.GetPortVlanEntries().insert(ActPortVlanEntry(port_id));
          vlan_table.GetVlanPortTypeEntries().insert(ActVlanPortTypeEntry(port_id));
        }
        vlan_table.GetVlanStaticEntries().insert(vlan_static_entry);

        vlan_tables.insert(device_id, vlan_table);
      }
    } else {
      ActVlanTable &vlan_table = vlan_tables[device_id];

      QSet<ActVlanStaticEntry> new_vlan_static_entries;
      for (ActVlanStaticEntry vlan_static_entry : vlan_table.GetVlanStaticEntries()) {
        QSet<qint64> egress_ports;
        QSet<qint64> untagged_ports;
        for (qint64 port_id : port_ids) {
          if (!vlan_table.GetPortVlanEntries().contains(
                  ActPortVlanEntry(port_id))) {  // The port on the new device that does not exist on the old device
            if (!default_device_config.GetVlanTables().isEmpty()) {
              ActVlanTable default_vlan_table = default_device_config.GetVlanTables().first();
              ActVlanStaticEntry default_vlan_static_entry;
              QSet<ActVlanStaticEntry>::iterator default_vlan_static_entry_iter =
                  default_vlan_table.GetVlanStaticEntries().find(vlan_static_entry);
              if (default_vlan_static_entry_iter !=
                  default_vlan_table.GetVlanStaticEntries().end()) {  // found the default config
                default_vlan_static_entry = *default_vlan_static_entry_iter;
                if (default_vlan_static_entry.GetEgressPorts().contains(port_id)) {
                  egress_ports.insert(port_id);
                }
                if (default_vlan_static_entry.GetUntaggedPorts().contains(port_id)) {
                  untagged_ports.insert(port_id);
                }
              } else {  // does't found the default config
                if (vlan_static_entry.GetVlanId() == vlan_table.GetManagementVlan()) {
                  untagged_ports.insert(port_id);
                }
                egress_ports.insert(port_id);
              }
            } else {  // does't found the default config
              if (vlan_static_entry.GetVlanId() == vlan_table.GetManagementVlan()) {
                untagged_ports.insert(port_id);
              }
              egress_ports.insert(port_id);
            }
          } else {  // the port exist on the old device
            if (vlan_static_entry.GetEgressPorts().contains(port_id)) {
              egress_ports.insert(port_id);
            }
            if (vlan_static_entry.GetUntaggedPorts().contains(port_id)) {
              untagged_ports.insert(port_id);
            }
          }
        }
        vlan_static_entry.SetEgressPorts(egress_ports);
        vlan_static_entry.SetUntaggedPorts(untagged_ports);
        new_vlan_static_entries.insert(vlan_static_entry);
      }
      if (!default_device_config.GetVlanTables()
               .isEmpty()) {  // insert the default vlan static entry that doesn't exist on the origin config
        ActVlanTable default_vlan_table = default_device_config.GetVlanTables().first();
        for (ActVlanStaticEntry vlan_static_entry : default_vlan_table.GetVlanStaticEntries()) {
          if (!new_vlan_static_entries.contains(vlan_static_entry)) {
            new_vlan_static_entries.insert(vlan_static_entry);
          }
        }
      }
      vlan_table.SetVlanStaticEntries(new_vlan_static_entries);

      QSet<ActPortVlanEntry> new_port_vlan_entries;
      for (qint64 port_id : port_ids) {
        ActPortVlanEntry port_vlan_entry(port_id, vlan_table.GetManagementVlan(), ActVlanPriorityEnum::kNonTSN);
        QSet<ActPortVlanEntry>::iterator port_vlan_entry_iter = vlan_table.GetPortVlanEntries().find(port_vlan_entry);
        if (port_vlan_entry_iter != vlan_table.GetPortVlanEntries().end()) {
          port_vlan_entry = *port_vlan_entry_iter;
        } else if (!default_device_config.GetVlanTables().isEmpty()) {
          ActVlanTable default_vlan_table = default_device_config.GetVlanTables().first();
          QSet<ActPortVlanEntry>::iterator default_port_vlan_entry_iter =
              default_vlan_table.GetPortVlanEntries().find(port_vlan_entry);
          if (default_port_vlan_entry_iter != default_vlan_table.GetPortVlanEntries().end()) {
            port_vlan_entry = *default_port_vlan_entry_iter;
          }
        }
        new_port_vlan_entries.insert(port_vlan_entry);
      }
      vlan_table.SetPortVlanEntries(new_port_vlan_entries);

      QSet<ActVlanPortTypeEntry> new_vlan_port_type_entries;
      for (qint64 port_id : port_ids) {
        ActVlanPortTypeEntry vlan_port_type_entry(port_id);
        QSet<ActVlanPortTypeEntry>::iterator vlan_port_type_entry_iter =
            vlan_table.GetVlanPortTypeEntries().find(vlan_port_type_entry);
        if (vlan_port_type_entry_iter != vlan_table.GetVlanPortTypeEntries().end()) {
          vlan_port_type_entry = *vlan_port_type_entry_iter;
        } else if (!default_device_config.GetVlanTables().isEmpty()) {
          ActVlanTable default_vlan_table = default_device_config.GetVlanTables().first();
          QSet<ActVlanPortTypeEntry>::iterator default_vlan_port_type_entry_iter =
              default_vlan_table.GetVlanPortTypeEntries().find(vlan_port_type_entry);
          if (default_vlan_port_type_entry_iter != default_vlan_table.GetVlanPortTypeEntries().end()) {
            vlan_port_type_entry = *default_vlan_port_type_entry_iter;
          }
        }
        new_vlan_port_type_entries.insert(vlan_port_type_entry);
      }
      vlan_table.SetVlanPortTypeEntries(new_vlan_port_type_entries);
    }

    // Keep the Reserved VLANs
    // [feat:3930] The MELCO 06 device must not delete VLAN 2
    ActVlanTable &vlan_table = vlan_tables[device_id];
    for (auto reserved_vlan_id : device.GetDeviceProperty().GetReservedVlan()) {
      ActVlanStaticEntry vlan_static_entry(reserved_vlan_id);
      auto vlan_static_entry_iter = vlan_table.GetVlanStaticEntries().find(vlan_static_entry);
      if (vlan_static_entry_iter == vlan_table.GetVlanStaticEntries().end()) {  // vlan entry not found
        // // Check has the default vlan entry, would used it name
        // if ((!default_device_config.isEmpty()) && (!default_device_config.GetVlanTables().isEmpty())) {
        //   auto default_vlan_static_entry_set =
        //       default_device_config.GetVlanTables().first().GetVlanStaticEntries();
        //   auto default_vlan_static_entry_iter = default_vlan_static_entry_set.find(vlan_static_entry);
        //   if (default_vlan_static_entry_iter != default_vlan_static_entry_set.end()) {  // find default vlan entry
        //     vlan_static_entry.SetName(default_vlan_static_entry_iter->GetName());
        //   }
        // }
        // Insert reserved vlan entry
        vlan_table.GetVlanStaticEntries().insert(vlan_static_entry);
      }
    }
  }

  // StaticForward(Unicast & Multiple)
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetStaticForwardSetting().GetUnicast()) {
    device_config.GetUnicastStaticForwardTables().remove(device_id);
  } else {
    // Insert default config from the device profile when table as empty
    if (!device_config.GetUnicastStaticForwardTables().contains(device.GetId())) {
      if (!default_device_config.GetUnicastStaticForwardTables().isEmpty()) {
        device_config.GetUnicastStaticForwardTables()[device_id] =
            default_device_config.GetUnicastStaticForwardTables().first();
      }
    }

    ActStaticForwardTable &unicast_static_forward_table = device_config.GetUnicastStaticForwardTables()[device_id];
    unicast_static_forward_table.SetDeviceId(device_id);
    QSet<ActStaticForwardEntry> new_static_forward_entries;
    for (ActStaticForwardEntry static_forward_entry : unicast_static_forward_table.GetStaticForwardEntries()) {
      if (device_config.GetVlanTables().contains(device_id)) {
        ActVlanTable &vlan_table = device_config.GetVlanTables()[device_id];
        if (vlan_table.GetVlanStaticEntries().contains(ActVlanStaticEntry(static_forward_entry.GetVlanId()))) {
          QSet<qint64> egress_ports = static_forward_entry.GetEgressPorts().intersect(port_ids);
          static_forward_entry.SetEgressPorts(egress_ports);
          new_static_forward_entries.insert(static_forward_entry);
        } else {
          // vlan table doesn't contain this vlan ID, so remove from static forward table
        }
      } else {
        // no vlan table, so clear all the static forward table
      }
    }
    unicast_static_forward_table.SetStaticForwardEntries(new_static_forward_entries);
  }

  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetStaticForwardSetting().GetMulticast()) {
    device_config.GetMulticastStaticForwardTables().remove(device_id);
  } else {
    // Insert default config from the device profile when table as empty
    if (!device_config.GetMulticastStaticForwardTables().contains(device.GetId())) {
      if (!default_device_config.GetMulticastStaticForwardTables().isEmpty()) {
        device_config.GetMulticastStaticForwardTables()[device_id] =
            default_device_config.GetMulticastStaticForwardTables().first();
      }
    }

    ActStaticForwardTable &multicast_static_forward_table = device_config.GetMulticastStaticForwardTables()[device_id];
    multicast_static_forward_table.SetDeviceId(device_id);
    QSet<ActStaticForwardEntry> new_static_forward_entries;
    for (ActStaticForwardEntry static_forward_entry : multicast_static_forward_table.GetStaticForwardEntries()) {
      if (device_config.GetVlanTables().contains(device_id)) {
        ActVlanTable &vlan_table = device_config.GetVlanTables()[device_id];
        if (vlan_table.GetVlanStaticEntries().contains(ActVlanStaticEntry(static_forward_entry.GetVlanId()))) {
          QSet<qint64> egress_ports = static_forward_entry.GetEgressPorts().intersect(port_ids);
          static_forward_entry.SetEgressPorts(egress_ports);
          new_static_forward_entries.insert(static_forward_entry);
        } else {
          // vlan table doesn't contain this vlan ID, so remove from static forward table
        }
      } else {
        // no vlan table, so clear all the static forward table
      }
    }
    multicast_static_forward_table.SetStaticForwardEntries(new_static_forward_entries);
  }

  // Port Default PCP
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetDefaultPCP()) {
    device_config.GetPortDefaultPCPTables().remove(device_id);
  } else {
    // Insert default config from the device profile when table as empty
    if (!device_config.GetPortDefaultPCPTables().contains(device.GetId())) {
      if (!default_device_config.GetPortDefaultPCPTables().isEmpty()) {
        device_config.GetPortDefaultPCPTables()[device_id] = default_device_config.GetPortDefaultPCPTables().first();
      }
    }

    ActDefaultPriorityTable &port_default_pcp_table = device_config.GetPortDefaultPCPTables()[device_id];
    port_default_pcp_table.SetDeviceId(device_id);
    QSet<ActDefaultPriorityEntry> default_priority_entries = port_default_pcp_table.GetDefaultPriorityEntries();
    QSet<ActDefaultPriorityEntry> new_default_priority_entries;
    for (qint64 port_id : port_ids) {
      ActDefaultPriorityEntry default_priority_entry(port_id);
      QSet<ActDefaultPriorityEntry>::iterator default_priority_entry_iter =
          default_priority_entries.find(default_priority_entry);
      if (default_priority_entry_iter != default_priority_entries.end()) {
        default_priority_entry = *default_priority_entry_iter;
      }
      new_default_priority_entries.insert(default_priority_entry);
    }
    port_default_pcp_table.SetDefaultPriorityEntries(new_default_priority_entries);
  }

  // StreamPriority(Ingress & Egress)
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetPerStreamPriority() &&
      !device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetPerStreamPriorityV2()) {
    device_config.GetStreamPriorityIngressTables().remove(device_id);
    device_config.GetStreamPriorityEgressTables().remove(device_id);
  } else {
    // Ingress

    // Insert default config from the device profile when table as empty
    if (!device_config.GetStreamPriorityIngressTables().contains(device.GetId())) {
      if (!default_device_config.GetStreamPriorityIngressTables().isEmpty()) {
        device_config.GetStreamPriorityIngressTables()[device_id] =
            default_device_config.GetStreamPriorityIngressTables().first();
      }
    }

    ActStadPortTable &stream_priority_ingress_table = device_config.GetStreamPriorityIngressTables()[device_id];
    stream_priority_ingress_table.SetDeviceId(device_id);
    QSet<ActInterfaceStadPortEntry> interface_stad_port_entries =
        stream_priority_ingress_table.GetInterfaceStadPortEntries();
    QSet<ActInterfaceStadPortEntry> new_interface_stad_port_entries;
    for (qint64 port_id : port_ids) {
      ActInterfaceStadPortEntry interface_stad_port_entry(port_id);
      QSet<ActInterfaceStadPortEntry>::iterator interface_stad_port_entry_iter =
          interface_stad_port_entries.find(interface_stad_port_entry);
      if (interface_stad_port_entry_iter != interface_stad_port_entries.end()) {
        interface_stad_port_entry = *interface_stad_port_entry_iter;
      } else if (!default_device_config.GetStreamPriorityIngressTables().isEmpty()) {
        ActStadPortTable default_stream_priority_ingress_table =
            default_device_config.GetStreamPriorityIngressTables().first();
        QSet<ActInterfaceStadPortEntry>::iterator default_interface_stad_port_entry_iter =
            default_stream_priority_ingress_table.GetInterfaceStadPortEntries().find(interface_stad_port_entry);
        if (default_interface_stad_port_entry_iter !=
            default_stream_priority_ingress_table.GetInterfaceStadPortEntries().end()) {
          interface_stad_port_entry = *default_interface_stad_port_entry_iter;
        }
      }
      new_interface_stad_port_entries.insert(interface_stad_port_entry);
    }
    stream_priority_ingress_table.SetInterfaceStadPortEntries(new_interface_stad_port_entries);

    // Egress
    // Insert default config from the device profile when table as empty
    if (!device_config.GetStreamPriorityEgressTables().contains(device.GetId())) {
      if (!default_device_config.GetStreamPriorityEgressTables().isEmpty()) {
        device_config.GetStreamPriorityEgressTables()[device_id] =
            default_device_config.GetStreamPriorityEgressTables().first();
      }
    }

    ActStadConfigTable &stream_priority_egress_table = device_config.GetStreamPriorityEgressTables()[device_id];
    stream_priority_egress_table.SetDeviceId(device_id);
    QSet<ActStadConfigEntry> stad_config_entries = stream_priority_egress_table.GetStadConfigEntries();
    QSet<ActStadConfigEntry> new_stad_config_entries;
    for (qint64 port_id : port_ids) {
      ActStadConfigEntry stad_config_entry(port_id);
      QSet<ActStadConfigEntry>::iterator stad_config_entry_iter = stad_config_entries.find(stad_config_entry);
      if (stad_config_entry_iter != stad_config_entries.end()) {
        stad_config_entry = *stad_config_entry_iter;
      } else if (!default_device_config.GetStreamPriorityEgressTables().isEmpty()) {
        ActStadConfigTable default_stream_priority_egress_table =
            default_device_config.GetStreamPriorityEgressTables().first();
        QSet<ActStadConfigEntry>::iterator default_stad_config_entry_iter =
            default_stream_priority_egress_table.GetStadConfigEntries().find(stad_config_entry);
        if (default_stad_config_entry_iter != default_stream_priority_egress_table.GetStadConfigEntries().end()) {
          stad_config_entry = *default_stad_config_entry_iter;
        }
      }
      new_stad_config_entries.insert(stad_config_entry);
    }
    stream_priority_egress_table.SetStadConfigEntries(new_stad_config_entries);
  }

  // GCL
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetTSN().GetIEEE802Dot1Qbv()) {
    device_config.GetGCLTables().remove(device_id);
  } else {
    // Insert default config from the device profile when table as empty
    if (!device_config.GetGCLTables().contains(device.GetId())) {
      if (!default_device_config.GetGCLTables().isEmpty()) {
        device_config.GetGCLTables()[device_id] = default_device_config.GetGCLTables().first();
      }
    }

    ActGclTable &gcl_table = device_config.GetGCLTables()[device_id];
    gcl_table.SetDeviceId(device_id);
    QSet<ActInterfaceGateParameters> interfaces_gate_parameters = gcl_table.GetInterfacesGateParameters();
    QSet<ActInterfaceGateParameters> new_interfaces_gate_parameters;
    for (qint64 port_id : port_ids) {
      ActInterfaceGateParameters interfaces_gate_parameter(port_id);
      QSet<ActInterfaceGateParameters>::iterator interfaces_gate_parameter_iter =
          interfaces_gate_parameters.find(interfaces_gate_parameter);
      if (interfaces_gate_parameter_iter != interfaces_gate_parameters.end()) {
        interfaces_gate_parameter = *interfaces_gate_parameter_iter;
      } else if (!default_device_config.GetStreamPriorityEgressTables().isEmpty()) {
        ActGclTable default_gcl_table = default_device_config.GetGCLTables().first();
        QSet<ActInterfaceGateParameters>::iterator default_interfaces_gate_parameter_iter =
            default_gcl_table.GetInterfacesGateParameters().find(interfaces_gate_parameter);
        if (default_interfaces_gate_parameter_iter != default_gcl_table.GetInterfacesGateParameters().end()) {
          interfaces_gate_parameter = *default_interfaces_gate_parameter_iter;
        }
      }
      new_interfaces_gate_parameters.insert(interfaces_gate_parameter);
    }
    gcl_table.SetInterfacesGateParameters(new_interfaces_gate_parameters);
  }

  // CB
  // 因為裡面存的 port 是 port name，所以更動 device 後基本上要重新 compute 才會正確，所以有更動 device 會清空 CB table
  device_config.GetCbTables().clear();

  // RSTP
  if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetSTPRSTP().GetRSTP()) {
    device_config.GetRstpTables().remove(device_id);
  } else {
    // Insert default config from the device profile when table as empty
    if (!device_config.GetRstpTables().contains(device.GetId())) {
      if (!default_device_config.GetRstpTables().isEmpty()) {
        device_config.GetRstpTables()[device_id] = default_device_config.GetRstpTables().first();
      }
    }

    ActRstpTable &rstp_table = device_config.GetRstpTables()[device_id];
    rstp_table.SetDeviceId(device_id);
    QSet<ActRstpPortEntry> rstp_port_entries = rstp_table.GetRstpPortEntries();
    QSet<ActRstpPortEntry> new_rstp_port_entries;
    for (qint64 port_id : port_ids) {
      ActRstpPortEntry rstp_port_entry(port_id);
      QSet<ActRstpPortEntry>::iterator rstp_port_entry_iter = rstp_port_entries.find(rstp_port_entry);
      if (rstp_port_entry_iter != rstp_port_entries.end()) {
        rstp_port_entry = *rstp_port_entry_iter;
      }
      new_rstp_port_entries.insert(rstp_port_entry);
    }
    rstp_table.SetRstpPortEntries(new_rstp_port_entries);
  }
}

ACT_STATUS ActCore::MatchDeviceProfile(ActProject &project, ActDevice &device) {
  ACT_STATUS_INIT();

  // Get device profile first
  QSet<ActDeviceProfile> device_profile_set = act::core::g_core.GetDeviceProfileSet();
  QSet<ActDeviceProfile>::const_iterator iterator =
      device_profile_set.find(ActDeviceProfile(device.GetDeviceProfileId()));
  if (iterator == device_profile_set.end()) {
    QString log_msg = QString("Device (%1) - The device profile %2 is not found")
                          .arg(device.GetIpv4().GetIpAddress())
                          .arg(device.GetDeviceProfileId());
    qCritical() << log_msg;
    QString error_msg = QString("Device profile id %1").arg(device.GetDeviceProfileId());
    return std::make_shared<ActStatusNotFound>(error_msg);
  }
  ActDeviceProfile profile = *iterator;

  device.SetDeviceType(profile.GetDeviceType());

  // Update FirmwareVersion when it is empty
  if (device.GetFirmwareVersion().isEmpty() && (!profile.GetLatestFirmwareVersion().isEmpty())) {
    device.SetFirmwareVersion(profile.GetLatestFirmwareVersion());
  }

  // Assign device properties from device profile if firmware changed
  ActDeviceProperty dev_prop;
  if (profile.GetDeviceType() == ActDeviceTypeEnum::kTSNSwitch ||
      profile.GetDeviceType() == ActDeviceTypeEnum::kBridgedEndStation ||
      profile.GetDeviceType() == ActDeviceTypeEnum::kSwitch) {
    dev_prop.SetGateControlListLength(profile.GetGateControlListLength());
    dev_prop.SetNumberOfQueue(profile.GetNumberOfQueue());
    dev_prop.SetTickGranularity(profile.GetTickGranularity());
    dev_prop.SetStreamPriorityConfigIngressIndexMax(profile.GetStreamPriorityConfigIngressIndexMax());
    dev_prop.SetProcessingDelayMap(profile.GetProcessingDelayMap());
    dev_prop.SetGCLOffsetMinDuration(profile.GetGCLOffsetMinDuration());
    dev_prop.SetGCLOffsetMaxDuration(profile.GetGCLOffsetMaxDuration());
    dev_prop.SetMaxVlanCfgSize(profile.GetMaxVlanCfgSize());
    dev_prop.SetPerQueueSize(profile.GetPerQueueSize());
    dev_prop.SetPtpQueueId(profile.GetPtpQueueId());
    dev_prop.SetMountType(profile.GetMountType());
    dev_prop.SetReservedVlan(profile.GetReservedVlan());
  }

  if (profile.GetDeviceType() == ActDeviceTypeEnum::kEndStation ||
      profile.GetDeviceType() == ActDeviceTypeEnum::kBridgedEndStation) {
    dev_prop.SetTrafficSpecification(profile.GetTrafficSpecification());
    dev_prop.SetIeee802VlanTag(profile.GetIeee802VlanTag());
  }

  dev_prop.SetModelName(profile.GetModelName());
  dev_prop.SetPhysicalModelName(profile.GetPhysicalModelName());
  dev_prop.SetCertificate(profile.GetCertificate());
  dev_prop.SetVendor(profile.GetVendor());
  dev_prop.SetDeviceCluster(profile.GetDeviceCluster());

  // Match Firmware Feature Profile
  // [feat:2849] Deploy - Identify device feature for different firmware
  // Find FirmwareFeatureProfile (ModelName & FirmwareVersion)
  ActFirmwareFeatureProfile fw_feat_profile;
  auto find_status = ActFirmwareFeatureProfile::GetFirmwareFeatureProfile(
      this->GetFirmwareFeatureProfileSet(), profile.GetModelName(), device.GetFirmwareVersion(), fw_feat_profile);
  if (IsActStatusSuccess(find_status)) {  // find specific version
    // Update FeatureGroup by FirmwareFeature profile
    dev_prop.SetFeatureGroup(fw_feat_profile.GetFeatureGroup());

    // Update FirmwareFeatureProfile ID
    device.SetFirmwareFeatureProfileId(fw_feat_profile.GetId());
  } else {
    // The latest firmware needs to set DeviceProfile's value
    dev_prop.SetFeatureGroup(profile.GetFeatureGroup());

    // Update FirmwareFeatureProfile ID(-1)
    device.SetFirmwareFeatureProfileId(-1);
  }

  device.SetDeviceProperty(dev_prop);

  // Set Device builtin Power Modular ID for default value
  if (profile.GetBuiltInPower() && device.GetModularConfiguration().GetPower().isEmpty() &&
      !profile.GetSupportPowerModules().isEmpty()) {
    device.GetModularConfiguration().GetPower().insert(ACT_BUILTIN_POWER_MODULE_SLOT,
                                                       profile.GetSupportPowerModules().begin().key());
  }

  // Set Device Ethernet Modular ID for default value
  // if (device.GetModularConfiguration().GetEthernet().isEmpty()) {
  //   qint64 eth_module_id = (profile.GetL2Family() == "MDS")   ? eth_module_name_id_map_[ACT_DEFAULT_MDS_LINE_MODULE]
  //                          : (profile.GetL2Family() == "RKS") ? eth_module_name_id_map_[ACT_DEFAULT_RKS_LINE_MODULE]
  //                                                             : 0;
  //   for (qint64 slot_id = 2; slot_id < profile.GetSupportEthernetSlots() + 2; slot_id++) {
  //     device.GetModularConfiguration().GetEthernet().insert(slot_id, eth_module_id);
  //   }
  // }

  QList<ActInterface> new_dev_intf_list;
  QList<ActInterface> dev_intf_list = device.GetInterfaces();
  QList<ActInterfaceProperty> profile_intf_list = profile.GetInterfaces();

  // Create ethernet module interface
  qint64 num_of_buildin_intf = profile.GetInterfaces().size();
  qint64 num_of_default_intf = profile.GetDefaultEthernetSlotOccupiedIntfs();
  qint64 num_of_support_slot = profile.GetSupportEthernetSlots() + 1;
  QMap<qint64, ActEthernetModule> eth_module_map = act::core::g_core.GetEthernetModuleMap();
  QMap<qint64, qint64> dev_eth_module_map = device.GetModularConfiguration().GetEthernet();

  qint64 intf_id = 0;
  for (qint64 slot_id = 1; slot_id <= num_of_support_slot; slot_id++) {
    QList<ActInterfaceProperty> profile_intf_list;
    if (slot_id == 1) {
      profile_intf_list = profile.GetInterfaces();
    } else if (slot_id > 1 && dev_eth_module_map.contains(slot_id)) {
      qint64 eth_module_id = dev_eth_module_map[slot_id];
      if (!eth_module_map.contains(eth_module_id)) {
        QString error_msg = QString("Ethernet module id %1 is not found").arg(eth_module_id);
        qCritical() << error_msg;
        return std::make_shared<ActStatusNotFound>(error_msg);
      }
      ActEthernetModule eth_module = eth_module_map[eth_module_id];

      profile_intf_list = eth_module.GetInterfaces();
    }

    for (ActInterfaceProperty intf_prop : profile_intf_list) {
      ActInterface intf(++intf_id);

      auto dev_intf_idx = dev_intf_list.indexOf(intf);
      if (dev_intf_idx != -1) {  // contains
        intf = dev_intf_list.at(dev_intf_idx);
      }

      // Use the profile interface name if the interface name is empty
      if (intf.GetInterfaceName().isEmpty() || (profile.GetDeviceType() != ActDeviceTypeEnum::kEndStation &&
                                                profile.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation)) {
        QString intf_name = slot_id == 1 ? intf_prop.GetInterfaceName()
                                         : QString("Eth%1/%2").arg(slot_id).arg(intf_prop.GetInterfaceName());
        intf.SetInterfaceName(intf_name);
      }

      intf.SetActive(true);
      intf.SetIpAddress(device.GetIpv4().GetIpAddress());
      intf.SetDeviceId(device.GetId());
      intf.SetSupportSpeeds(intf_prop.GetSupportSpeeds());
      intf.SetCableTypes(intf_prop.GetCableTypes());

      new_dev_intf_list.append(intf);
    }

    qint64 max_intf_id = num_of_buildin_intf + num_of_default_intf * (slot_id - 1);
    while (intf_id < max_intf_id) {
      ActInterface intf(++intf_id);
      qint64 eth_id = (intf_id - num_of_buildin_intf - 1) % num_of_default_intf + 1;
      intf.SetInterfaceName(QString("Eth%1/%2").arg(slot_id).arg(eth_id));
      intf.SetIpAddress(device.GetIpv4().GetIpAddress());
      intf.SetDeviceId(device.GetId());
      intf.SetActive(false);

      new_dev_intf_list.append(intf);
    }
  }

  // Append the interface not defined in the DeviceProfile(ICMP, Unknown, Moxa device)
  // [bugfix:3249] Auto Scan - Interface can not be created when it not be defined in DeviceProfile
  if (profile.GetDeviceType() == ActDeviceTypeEnum::kICMP || profile.GetDeviceType() == ActDeviceTypeEnum::kUnknown ||
      profile.GetDeviceType() == ActDeviceTypeEnum::kMoxa) {
    for (auto intf : dev_intf_list) {
      if (!new_dev_intf_list.contains(intf)) {
        intf.SetIpAddress(device.GetIpv4().GetIpAddress());
        intf.SetDeviceId(device.GetId());
      }
      new_dev_intf_list.append(intf);
    }
  }

  // Sorting the interface list by interface id
  std::sort(new_dev_intf_list.begin(), new_dev_intf_list.end(), InterfaceIdThan);
  device.SetInterfaces(new_dev_intf_list);

  return act_status;
}

ACT_STATUS ActCore::CreateDevice(qint64 &project_id, ActDevice &device, const bool from_bag, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->CreateDevice(project, device, from_bag);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Create device failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::CreateDevice(ActProject &project, ActDevice &device, const bool from_bag) {
  ACT_STATUS_INIT();

  QSet<ActDevice> &device_set = project.GetDevices();

  // Add license control
  /* quint16 lic_device_qty = this->GetLicense().GetSize().GetDeviceQty();
  if ((device_set.size() >= lic_device_qty) &&
      (lic_device_qty != 0)) {  // [feat:2852] License - device size = 0 means unlimited
    QString error_msg = QString("Device (%1) - The device size exceeds the limit: %2")
                            .arg(device.GetIpv4().GetIpAddress())
                            .arg(lic_device_qty);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActLicenseSizeFailedRequest>("Device Size", lic_device_qty);
  } */

  if (device.GetIpv4().GetIpAddress() == "") {
    project.AutoAssignIP<ActDevice>(device);
  }

  // Check the device does not exist with duplicated IP address
  quint32 ip_num = 0;
  ActIpv4::AddressStrToNumber(device.GetIpv4().GetIpAddress(), ip_num);
  if (project.used_ip_addresses_.contains(ip_num)) {
    qCritical() << "The IP address" << device.GetIpv4().GetIpAddress() << "is duplicated";
    return std::make_shared<ActDuplicatedError>(device.GetIpv4().GetIpAddress());
  }

  // Generate a new unique id
  if (device.GetId() == -1) {
    qint64 id;
    act_status = this->GenerateUniqueId<ActDevice>(device_set, project.last_assigned_device_id_, id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Cannot get an available unique id";
      return act_status;
    }
    device.SetId(id);
  }

  // Find the device profile to fill the interface property by model name
  act_status = MatchDeviceProfile(project, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get device profile failed with device id:" << device.GetId();
    return act_status;
  }

  // If the input is empty, which means it should use default setting
  this->HandleConnectionConfigField(device, project.GetProjectSetting(), project.GetProjectSetting());

  act_status = this->CheckDevice(project, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check device failed with device id:" << device.GetId();
    return act_status;
  }

  // Set device role to unknown
  device.SetDeviceRole(ActDeviceRoleEnum::kUnknown);

  // Check coordinate duplicated w/ other device
  // If duplicated, assign a new coordinate w/ x + 100, y + 100
  ActCoordinate new_coordinate = device.GetCoordinate();
  quint8 retry = 0;
  while (project.used_coordinates_.contains(new_coordinate)) {
    if (retry >= 4) {
      new_coordinate.SetX(device.GetCoordinate().GetX());
      new_coordinate.SetY(new_coordinate.GetY() + 150);
      retry = 0;
    } else {
      new_coordinate.SetX(new_coordinate.GetX() + 300);
    }
    retry++;
  }
  device.SetCoordinate(new_coordinate);
  project.used_coordinates_.insert(new_coordinate);

  // Add device to group if assigned
  if (device.GetGroupId() != -1) {
    QMap<qint64, ActGroup> &groups = project.GetTopologySetting().GetGroups();
    if (groups.contains(device.GetGroupId())) {
      groups[device.GetGroupId()].GetDeviceIds().insert(device.GetId());
    } else {
      QString error_msg = QString("Device (%1) - The group id %2 is not found")
                              .arg(device.GetIpv4().GetIpAddress())
                              .arg(device.GetGroupId());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Insert the device to project
  device_set.insert(device);
  project.used_ip_addresses_.insert(ip_num);

  // Create device in RSTP group
  if (device.GetDeviceType() == ActDeviceTypeEnum::kTSNSwitch ||
      device.GetDeviceType() == ActDeviceTypeEnum::kBridgedEndStation ||
      device.GetDeviceType() == ActDeviceTypeEnum::kSwitch) {
    QSet<ActRSTP> rstp_groups = project.GetTopologySetting().GetRedundantGroup().GetRSTP();
    for (ActRSTP rstp : rstp_groups) {
      QSet<qint64> &devices = rstp.GetDevices();
      devices.insert(device.GetId());

      act_status = act::core::g_core.UpdateRedundantRSTP(project, rstp);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Update rstp failed" << rstp.ToString().toStdString().c_str();
        return act_status;
      }
    }
  }

  // Find the device profile to fill the interface property by model name
  UpdateDeviceConfigTable(project, device, this->GetDeviceProfileSet());

  // Send update msg to temp
  InsertDeviceMsgToNotificationTmp(
      ActDevicePatchUpdateMsg(ActPatchUpdateActionEnum::kCreate, project.GetId(), device, true));

  // Update SKU quantities & in_topology_count (used) with device model and module information
  act_status = this->UpdateSkuQuantityByAddDevice(project, device, from_bag);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "UpdateSkuQuantityByAddDevice() failed for project:"
                << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::CreateDevices(qint64 &project_id, QList<ActDevice> &device_list, const bool from_bag,
                                  QList<qint64> &created_device_ids, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->CreateDevices(project, device_list, from_bag, created_device_ids);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Create Devices failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::CreateDevices(ActProject &project, QList<ActDevice> &device_list, const bool from_bag,
                                  QList<qint64> &created_device_ids) {
  ACT_STATUS_INIT();
  QList<ActDevice> new_device_list;

  // Check device list is empty
  if (device_list.isEmpty()) {
    QString error_msg = QString("Device list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Take each device out to generate ActDevice
  for (ActDevice device : device_list) {
    act_status = this->CreateDevice(project, device, from_bag);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Create device failed";
      return act_status;
    }

    created_device_ids.append(device.GetId());
    new_device_list.append(device);
  }

  device_list = new_device_list;
  return act_status;
}

ACT_STATUS ActCore::GetDevice(const qint64 &project_id, const qint64 &device_id, ActDevice &device, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QSet<ActDevice> device_set = project.GetDevices();
  act_status = ActGetItemById<ActDevice>(device_set, device_id, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Device id:" << device_id << "not found";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateDevice(qint64 &project_id, ActDevice &device, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateDevice(project, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update device failed with device id:" << device.GetId();
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateDevice(ActProject &project, ActDevice &device) {
  ACT_STATUS_INIT();

  QSet<ActDevice> &device_set = project.GetDevices();

  // Create new device to find, because the duplicated IP address would found failed
  ActDevice deleted_device;
  typename QSet<ActDevice>::const_iterator iterator;
  iterator = device_set.find(ActDevice(device.GetId()));
  if (iterator != device_set.end()) {
    deleted_device = *iterator;
    device_set.erase(iterator);

    quint32 del_ip_num = 0;
    ActIpv4::AddressStrToNumber(deleted_device.GetIpv4().GetIpAddress(), del_ip_num);
    project.used_ip_addresses_.remove(del_ip_num);

    project.used_coordinates_.remove(deleted_device.GetCoordinate());
  }

  for (ActInterface &interface : device.GetInterfaces()) {
    ActLink link;
    if (IsActStatusNotFound(project.GetLinkByInterfaceId(link, device.GetId(), interface.GetInterfaceId()))) {
      interface.SetUsed(false);
    } else {
      interface.SetUsed(true);
    }

    interface.SetDeviceId(device.GetId());
  }

  // Check the device does not exist with duplicated IP address
  quint32 ip_num = 0;
  ActIpv4::AddressStrToNumber(device.GetIpv4().GetIpAddress(), ip_num);
  if (project.used_ip_addresses_.contains(ip_num)) {
    qCritical() << "The IP address" << device.GetIpv4().GetIpAddress() << "is duplicated";
    return std::make_shared<ActDuplicatedError>(device.GetIpv4().GetIpAddress());
  }
  project.used_ip_addresses_.insert(ip_num);

  // Generate a new unique id
  if (device.GetId() == -1) {
    qint64 id;
    act_status = this->GenerateUniqueId<ActDevice>(device_set, project.last_assigned_device_id_, id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Cannot get an available unique id";
      return act_status;
    }
    device.SetId(id);
  }

  // Find the device profile to fill the interface property by model name
  act_status = MatchDeviceProfile(project, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get device profile failed with device:" << device.GetIpv4().GetIpAddress();
    return act_status;
  }

  // Check DeviceCluster
  if (deleted_device.GetDeviceProperty().GetDeviceCluster() != device.GetDeviceProperty().GetDeviceCluster()) {
    QString error_msg = QString("The %1 cannot be changed to the %2")
                            .arg(deleted_device.GetDeviceProperty().GetModelName())
                            .arg(device.GetDeviceProperty().GetModelName());
    qWarning() << __func__ << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Write back the password item
  this->HandleConnectionConfigField(device, deleted_device, project.GetProjectSetting());

  act_status = this->CheckDevice(project, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check device failed with device id:" << device.GetId();
    return act_status;
  }

  // Check the g_dev_coordinate_set contains the device coordinate
  // If yes, replace the device coordinate with the new one
  if (g_dev_coordinate_set.contains(ActDeviceCoordinate(device))) {
    // Fetch the device coordinate from the QSet
    ActDeviceCoordinate x_y;
    QSet<ActDeviceCoordinate>::const_iterator iterator = g_dev_coordinate_set.find(ActDeviceCoordinate(device));
    if (iterator == g_dev_coordinate_set.end()) {  // not found device coordinate
      qCritical() << __func__ << "coordinate list has no device id:" << device.GetId();
      return act_status;
    }

    x_y = *iterator;
    g_dev_coordinate_set.erase(iterator);

    device.SetCoordinate(x_y.GetCoordinate());
  }

  project.used_coordinates_.insert(device.GetCoordinate());

  // Update Group membership
  qint64 old_group_id = deleted_device.GetGroupId();
  qint64 new_group_id = device.GetGroupId();

  if (old_group_id != new_group_id) {
    QMap<qint64, ActGroup> &groups = project.GetTopologySetting().GetGroups();

    // Remove from old group
    if (old_group_id != -1 && groups.contains(old_group_id)) {
      groups[old_group_id].GetDeviceIds().remove(device.GetId());
    }

    // Add to new group
    if (new_group_id != -1) {
      if (groups.contains(new_group_id)) {
        groups[new_group_id].GetDeviceIds().insert(device.GetId());
      } else {
        QString error_msg = QString("Device (%1) - The group id %2 is not found")
                                .arg(device.GetIpv4().GetIpAddress())
                                .arg(device.GetGroupId());
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }
  }

  // Insert the device to project
  device_set.insert(device);

  // Update device in RSTP group
  QSet<ActRSTP> rstp_groups = project.GetTopologySetting().GetRedundantGroup().GetRSTP();
  for (ActRSTP rstp : rstp_groups) {
    QSet<qint64> &devices = rstp.GetDevices();
    if (device.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation &&
        device.GetDeviceType() != ActDeviceTypeEnum::kSwitch) {
      devices.remove(device.GetId());
    } else {
      devices.insert(device.GetId());
    }

    if (devices.isEmpty() ||
        (!devices.contains(rstp.GetRootDevice()) && !devices.contains(rstp.GetBackupRootDevice()))) {
      act_status = act::core::g_core.DeleteRedundantRSTP(project, rstp.GetId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Delete rstp failed" << rstp.ToString().toStdString().c_str();
        return act_status;
      }
      continue;
    } else if (!devices.contains(rstp.GetRootDevice()) && devices.contains(rstp.GetBackupRootDevice())) {
      rstp.SetRootDevice(rstp.GetBackupRootDevice());
      rstp.SetBackupRootDevice(-1);
    } else if (devices.contains(rstp.GetRootDevice()) && !devices.contains(rstp.GetBackupRootDevice())) {
      rstp.SetBackupRootDevice(-1);
    }

    act_status = act::core::g_core.UpdateRedundantRSTP(project, rstp);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Update rstp failed" << rstp.ToString().toStdString().c_str();
      return act_status;
    }
  }

  // Update device in VLAN group
  QSet<ActIntelligentVlan> intelligent_vlan_group = project.GetTopologySetting().GetIntelligentVlanGroup();
  if (device.GetDeviceType() != ActDeviceTypeEnum::kEndStation &&
      device.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
    for (ActIntelligentVlan intelligent_vlan : intelligent_vlan_group) {
      if (intelligent_vlan.GetEndStationList().remove(device.GetId())) {
        if (intelligent_vlan.GetEndStationList().isEmpty()) {
          act_status = act::core::g_core.DeleteVlanGroup(project, intelligent_vlan.GetVlanId());
          if (!IsActStatusSuccess(act_status)) {
            qCritical() << "Delete VLAN group failed" << intelligent_vlan.ToString().toStdString().c_str();
            return act_status;
          }
        } else {
          act_status = act::core::g_core.UpdateVlanGroup(project, intelligent_vlan);
          if (!IsActStatusSuccess(act_status)) {
            qCritical() << "Update VLAN group failed" << intelligent_vlan.ToString().toStdString().c_str();
            return act_status;
          }
        }
      }
    }
  }

  // Device interface size less than original size
  QSet<ActLink> link_set = project.GetLinks();
  for (ActLink link : link_set) {
    qint64 interface_id = (link.GetSourceDeviceId() == device.GetId())        ? link.GetSourceInterfaceId()
                          : (link.GetDestinationDeviceId() == device.GetId()) ? link.GetDestinationInterfaceId()
                                                                              : -1;
    if (interface_id == -1) {
      continue;
    }

    // Find original device interface is exist or not
    int intf_idx = device.GetInterfaces().indexOf(ActInterface(interface_id));
    if (intf_idx == -1 || !device.GetInterfaces()[intf_idx].GetActive()) {
      // Find unused interface to update link to connect
      bool find_unused_interface = false;
      for (ActInterface &interface : device.GetInterfaces()) {
        if (!interface.GetActive() || interface.GetUsed()) {
          continue;
        }
        interface.SetUsed(true);
        // Update connected device interface
        if (link.GetSourceDeviceId() == device.GetId()) {
          link.SetSourceInterfaceId(interface.GetInterfaceId());
        } else {
          link.SetDestinationInterfaceId(interface.GetInterfaceId());
        }
        intf_idx = device.GetInterfaces().indexOf(interface);
        find_unused_interface = true;
        break;
      }

      // Not found unused interface
      if (!find_unused_interface) {
        // Delete link
        act_status = this->DeleteLink(project, link.GetId());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Delete link failed";
          return act_status;
        }
        continue;
      }
    }

    // Find neighbor device interface
    qint64 neighbor_id;
    qint64 neighbor_intf_id;
    if (link.GetSourceDeviceId() == device.GetId()) {
      neighbor_id = link.GetDestinationDeviceId();
      neighbor_intf_id = link.GetDestinationInterfaceId();
    } else {
      neighbor_id = link.GetSourceDeviceId();
      neighbor_intf_id = link.GetSourceInterfaceId();
    }

    ActDevice neighbor;
    if (IsActStatusNotFound(project.GetDeviceById(neighbor, neighbor_id))) {
      continue;
    }

    int neighbor_intf_idx = neighbor.GetInterfaces().indexOf(ActInterface(neighbor_intf_id));
    ActInterface neighbor_intf = neighbor.GetInterfaces()[neighbor_intf_idx];
    ActInterface device_intf = device.GetInterfaces()[intf_idx];

    QList<qint64> device_support_speeds = device_intf.GetSupportSpeeds();
    QList<qint64> neighbor_support_speeds = neighbor_intf.GetSupportSpeeds();

    // Find the largest speed of the interface supports between the devices link connected.
    while (1) {
      if (device_support_speeds.isEmpty() || neighbor_support_speeds.isEmpty()) {
        // Delete link
        act_status = this->DeleteLink(project, link.GetId());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Delete link failed";
          return act_status;
        }
        break;
      }

      if (device_support_speeds.last() > neighbor_support_speeds.last()) {
        device_support_speeds.pop_back();
      } else if (device_support_speeds.last() < neighbor_support_speeds.last()) {
        neighbor_support_speeds.pop_back();
      } else {
        link.SetSpeed(device_support_speeds.last());
        act_status = this->UpdateLink(project, link);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Update link failed";
          return act_status;
        }
        break;
      }
    }
  }

  // Find the device profile to fill the interface property by model name
  UpdateDeviceConfigTable(project, device, this->GetDeviceProfileSet());

  // Send update msg to temp
  InsertDeviceMsgToNotificationTmp(
      ActDevicePatchUpdateMsg(ActPatchUpdateActionEnum::kUpdate, project.GetId(), device, true));

  act_status = this->UpdateSkuQuantityByUpdateDevice(project, deleted_device, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "UpdateSkuQuantity() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateDevices(qint64 &project_id, QList<ActDevice> &device_list, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Patch update devices
  act_status = this->UpdateDevices(project, device_list);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Update devices failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateDevices(ActProject &project, QList<ActDevice> &device_list) {
  ACT_STATUS_INIT();

  // Check device list is empty
  if (device_list.isEmpty()) {
    QString error_msg = QString("Device list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (ActDevice device : device_list) {
    act_status = this->UpdateDevice(project, device);
    if (!IsActStatusSuccess(act_status)) {  // fail to update device
      qCritical() << "Update device failed with device id:" << device.GetId();
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateDeviceCoordinates(qint64 &project_id, QSet<ActDeviceCoordinate> &device_set,
                                            bool sync_to_websocket, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateDeviceCoordinates(project, device_set);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Update device coordinate failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, sync_to_websocket, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  // this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateDeviceCoordinates(ActProject &project, QSet<ActDeviceCoordinate> &dev_coordinate_set) {
  ACT_STATUS_INIT();

  QSet<ActDevice> &device_set = project.GetDevices();

  for (ActDeviceCoordinate dev_coordinate : dev_coordinate_set) {
    ActDevice device;
    QSet<ActDevice>::const_iterator iterator = device_set.find(ActDevice(dev_coordinate.GetId()));
    if (iterator == device_set.end()) {  // not found device
      qCritical() << __func__ << "Project has no device id:" << dev_coordinate.GetId();
      return act_status;
    }
    device = *iterator;
    device_set.erase(iterator);
    project.used_coordinates_.remove(device.GetCoordinate());

    device.SetCoordinate(dev_coordinate.GetCoordinate());
    project.used_coordinates_.insert(device.GetCoordinate());
    device_set.insert(device);
  }

  return act_status;
}

ACT_STATUS ActCore::DeleteDevice(qint64 &project_id, qint64 &device_id, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteDevice(project, device_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete device failed with device id:" << device_id;
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::DeleteDevice(ActProject &project, qint64 &device_id) {
  ACT_STATUS_INIT();

  QSet<ActDevice> &device_set = project.GetDevices();

  // Check the item does exist by id
  ActDevice device;
  act_status = project.GetDeviceById(device, device_id);
  if (IsActStatusNotFound(act_status)) {
    QString error_msg = QString("Delete device failed, cannot found device id %1").arg(device_id);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check the device does not exist with duplicated IP address
  quint32 ip_num = 0;
  ActIpv4::AddressStrToNumber(device.GetIpv4().GetIpAddress(), ip_num);
  project.used_ip_addresses_.remove(ip_num);

  project.used_coordinates_.remove(device.GetCoordinate());

  // Remove device from group
  if (device.GetGroupId() != -1) {
    QMap<qint64, ActGroup> &groups = project.GetTopologySetting().GetGroups();
    if (groups.contains(device.GetGroupId())) {
      groups[device.GetGroupId()].GetDeviceIds().remove(device.GetId());
    }
  }

  device_set.remove(device);

  // Send update msg to temp
  InsertDeviceMsgToNotificationTmp(
      ActDevicePatchUpdateMsg(ActPatchUpdateActionEnum::kDelete, project.GetId(), device, true));

  // [bugfix:2740] Delete device not removed the ManagementInterface
  // Delete ManagementInterface
  QList<ActManagementInterface> management_interface_list = project.GetTopologySetting().GetManagementInterfaces();
  for (ActManagementInterface management_intf : management_interface_list) {
    if (management_intf.GetDeviceId() == device_id) {  // Found the ManagementInterface at DeviceConfig
      act_status = act::core::g_core.DeleteManagementInterface(project, management_intf.GetDeviceId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__
                    << "Delete management_interface failed with device id:" << management_intf.GetDeviceId();
        return act_status;
      }
    }
  }

  // Delete device in RSTP group
  QSet<ActRSTP> rstp_groups = project.GetTopologySetting().GetRedundantGroup().GetRSTP();
  for (ActRSTP rstp : rstp_groups) {
    QSet<qint64> &devices = rstp.GetDevices();
    devices.remove(device_id);

    if (devices.isEmpty() || (rstp.GetRootDevice() == device_id && rstp.GetBackupRootDevice() == -1)) {
      act_status = act::core::g_core.DeleteRedundantRSTP(project, rstp.GetId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Delete rstp failed" << rstp.ToString().toStdString().c_str();
        return act_status;
      }
    } else {
      if (rstp.GetRootDevice() == device_id && rstp.GetBackupRootDevice() != -1) {
        rstp.SetRootDevice(rstp.GetBackupRootDevice());
        rstp.SetBackupRootDevice(-1);
      } else if (rstp.GetBackupRootDevice() == device_id) {
        rstp.SetBackupRootDevice(-1);
      }
      act_status = act::core::g_core.UpdateRedundantRSTP(project, rstp);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Update rstp failed" << rstp.ToString().toStdString().c_str();
        return act_status;
      }
    }
  }

  // Delete device in VLAN group
  QSet<ActIntelligentVlan> intelligent_vlan_group = project.GetTopologySetting().GetIntelligentVlanGroup();
  for (ActIntelligentVlan intelligent_vlan : intelligent_vlan_group) {
    QSet<qint64> &end_devices = intelligent_vlan.GetEndStationList();
    end_devices.remove(device_id);

    if (end_devices.size() < 2) {
      act_status = act::core::g_core.DeleteVlanGroup(project, intelligent_vlan.GetVlanId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Delete VLAN group failed" << intelligent_vlan.ToString().toStdString().c_str();
        return act_status;
      }
    } else {
      act_status = act::core::g_core.UpdateVlanGroup(project, intelligent_vlan);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Update VLAN group failed" << intelligent_vlan.ToString().toStdString().c_str();
        return act_status;
      }
    }
  }

  // Delete DeviceConfig table
  act_status = act::core::g_core.DeleteDeviceDeviceConfig(project, device_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete Device DeviceConfig failed. Device:" << device_id;
    return act_status;
  }

  // Delete the connected link & update opposite device
  QSet<ActLink> link_set = project.GetLinks();
  for (ActLink link : link_set) {
    // Use device id to filter the link
    if (link.GetSourceDeviceId() == device_id || link.GetDestinationDeviceId() == device_id) {
      act_status = act::core::g_core.DeleteLink(project, link.GetId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Cannot delete related link:" << link.ToString().toStdString().c_str();
        return act_status;
      }
    }
  }

  // Update SKU quantities with device model and module information
  const bool to_bag = false;  // northbound call deleteDevices instead of deleteDevice
  act_status = this->UpdateSkuQuantityByDeleteDevice(project, device, to_bag);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "UpdateSkuQuantity() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::DeleteDevices(qint64 &project_id, QList<qint64> &dev_ids, const bool to_bag, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteDevices(project, dev_ids, to_bag);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete devices failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::DeleteDevices(ActProject &project, QList<qint64> &dev_ids, const bool to_bag) {
  ACT_STATUS_INIT();

  QSet<ActDevice> &device_set = project.GetDevices();

  // Check device list is empty
  if (dev_ids.isEmpty()) {
    QString error_msg = QString("Device id list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check the item does exist by id
  for (qint64 device_id : dev_ids) {
    ActDevice device;
    act_status = project.GetDeviceById(device, device_id);
    if (IsActStatusNotFound(act_status)) {
      QString error_msg = QString("Delete device failed, cannot found device id %1").arg(device_id);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    // Check the device does not exist with duplicated IP address
    quint32 ip_num = 0;
    ActIpv4::AddressStrToNumber(device.GetIpv4().GetIpAddress(), ip_num);
    project.used_ip_addresses_.remove(ip_num);
    project.used_coordinates_.remove(device.GetCoordinate());

    // Remove device from group
    if (device.GetGroupId() != -1) {
      QMap<qint64, ActGroup> &groups = project.GetTopologySetting().GetGroups();
      if (groups.contains(device.GetGroupId())) {
        groups[device.GetGroupId()].GetDeviceIds().remove(device.GetId());
      }
    }

    device_set.remove(device);

    // Send update msg to temp
    InsertDeviceMsgToNotificationTmp(
        ActDevicePatchUpdateMsg(ActPatchUpdateActionEnum::kDelete, project.GetId(), device, true));

    // Update SKU quantities with device model and module information
    act_status = this->UpdateSkuQuantityByDeleteDevice(project, device, to_bag);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "UpdateSkuQuantity() failed for project:" << project.GetProjectName().toStdString().c_str();
      return act_status;
    }
  }

  // [bugfix:2740] Delete device not removed the ManagementInterface
  // Delete ManagementInterface
  QList<ActManagementInterface> management_interface_list = project.GetTopologySetting().GetManagementInterfaces();
  for (ActManagementInterface management_intf : management_interface_list) {
    if (dev_ids.contains(management_intf.GetDeviceId())) {  // Found the ManagementInterface at DeviceConfig
      act_status = act::core::g_core.DeleteManagementInterface(project, management_intf.GetDeviceId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__
                    << "Delete management_interface failed with device id:" << management_intf.GetDeviceId();
        return act_status;
      }
    }
  }

  // Delete device in RSTP group
  QSet<ActRSTP> rstp_groups = project.GetTopologySetting().GetRedundantGroup().GetRSTP();
  for (ActRSTP rstp : rstp_groups) {
    QSet<qint64> &devices = rstp.GetDevices();
    for (qint64 device_id : dev_ids) {
      if (!devices.contains(device_id)) {
        continue;
      }
      devices.remove(device_id);
    }
    if (devices.isEmpty() || (dev_ids.contains(rstp.GetRootDevice()) &&
                              (dev_ids.contains(rstp.GetBackupRootDevice()) || rstp.GetBackupRootDevice() == -1))) {
      act_status = act::core::g_core.DeleteRedundantRSTP(project, rstp.GetId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Delete rstp failed" << rstp.ToString().toStdString().c_str();
        return act_status;
      }
    } else {
      if (dev_ids.contains(rstp.GetRootDevice()) && rstp.GetBackupRootDevice() != -1) {
        rstp.SetRootDevice(rstp.GetBackupRootDevice());
        rstp.SetBackupRootDevice(-1);
      } else if (dev_ids.contains(rstp.GetBackupRootDevice())) {
        rstp.SetBackupRootDevice(-1);
      }
      act_status = act::core::g_core.UpdateRedundantRSTP(project, rstp);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Update rstp failed" << rstp.ToString().toStdString().c_str();
        return act_status;
      }
    }
  }

  // Delete device in VLAN group
  QSet<ActIntelligentVlan> intelligent_vlan_group = project.GetTopologySetting().GetIntelligentVlanGroup();
  for (ActIntelligentVlan intelligent_vlan : intelligent_vlan_group) {
    QSet<qint64> &end_devices = intelligent_vlan.GetEndStationList();
    for (qint64 device_id : dev_ids) {
      if (!end_devices.contains(device_id)) {
        continue;
      }
      end_devices.remove(device_id);
    }
    if (end_devices.size() < 2) {
      act_status = act::core::g_core.DeleteVlanGroup(project, intelligent_vlan.GetVlanId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Delete VLAN group failed" << intelligent_vlan.ToString().toStdString().c_str();
        return act_status;
      }
    } else {
      act_status = act::core::g_core.UpdateVlanGroup(project, intelligent_vlan);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Update VLAN group failed" << intelligent_vlan.ToString().toStdString().c_str();
        return act_status;
      }
    }
  }

  // Delete DeviceConfig table
  for (qint64 device_id : dev_ids) {
    act_status = act::core::g_core.DeleteDeviceDeviceConfig(project, device_id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Delete Device DeviceConfig failed. Device:" << device_id;
      return act_status;
    }
  }

  // Delete the connected link & update opposite device
  QSet<ActLink> link_set = project.GetLinks();
  for (ActLink link : link_set) {
    // Use device id to filter the link
    if (dev_ids.contains(link.GetSourceDeviceId()) || dev_ids.contains(link.GetDestinationDeviceId())) {
      act_status = act::core::g_core.DeleteLink(project, link.GetId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Cannot delete related link:" << link.ToString().toStdString().c_str();
        return act_status;
      }
    }
  }

  return act_status;
}

ACT_STATUS ActCore::GetDeviceBag(qint64 &project_id, ActDeviceBag &device_bag) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Get device bag
  act_status = this->GetDeviceBag(project, device_bag);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get device bag failed";
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetDeviceBag(ActProject &project, ActDeviceBag &device_bag) {
  ACT_STATUS_INIT();

  QMap<QString, ActSkuQuantity> sku_map = project.GetSkuQuantitiesMap();
  auto device_profile_set = this->GetDeviceProfileSet();
  auto general_profile_map = this->GetGeneralProfileMap();

  QMap<QString, ActDeviceBagItem> device_bag_map = device_bag.GetDeviceBagMap();

  for (auto it = sku_map.begin(); it != sku_map.end(); it++) {
    QString sku_name = it.key();
    ActSkuQuantity sku_info = it.value();

    ActL1CategoryEnum l1_category = general_profile_map[sku_name].GetL1Category();

    if (l1_category == ActL1CategoryEnum::kSwitches) {
      qint64 remaining_quantity = sku_info.GetQuantity() - sku_info.GetInTopologyCount();
      device_bag_map[sku_name].SetRemainingQuantity(remaining_quantity);

      bool in_device_profile = device_profile_set.contains(ActDeviceProfile(sku_name));
      device_bag_map[sku_name].SetCanAddToTopology(in_device_profile);
    }
  }

  device_bag.SetDeviceBagMap(device_bag_map);
  return act_status;
}

}  // namespace core
}  // namespace act
