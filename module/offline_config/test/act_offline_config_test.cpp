#include "act_offline_config.hpp"

int main(void) {
  ACT_STATUS_INIT();

  const qint64 DEVICE_ID = 100;

  ActProject project;

  // add device into project
  ActDeviceProperty device_property;
  device_property.SetModelName("TSN-G5004");

  ActDevice device(DEVICE_ID);
  device.SetDeviceProperty(device_property);

  ActIpv4 ipv4("192.168.127.252");
  device.SetIpv4(ipv4);

  ActInterface interface_1;
  interface_1.SetInterfaceId(1);
  interface_1.SetInterfaceName("1");
  ActInterface interface_2;
  interface_2.SetInterfaceId(2);
  interface_2.SetInterfaceName("Eth1/2");
  QList<ActInterface> interfaces = {interface_1, interface_2};

  device.SetInterfaces(interfaces);

  QSet<ActDevice> devices;
  devices.insert(device);
  project.SetDevices(devices);

  // set device config

  // network setting
  ActNetworkSettingTable network_setting_table(DEVICE_ID);
  network_setting_table.SetIpAddress("192.168.127.252");

  QMap<qint64, ActNetworkSettingTable> network_setting_tables;
  network_setting_tables[DEVICE_ID] = network_setting_table;

  ActDeviceConfig device_config;
  device_config.SetNetworkSettingTables(network_setting_tables);

  // login message
  ActLoginPolicyTable login_policy_table(DEVICE_ID);
  login_policy_table.SetLoginMessage("Hello.");

  QMap<qint64, ActLoginPolicyTable> login_policy_tables;
  login_policy_tables[DEVICE_ID] = login_policy_table;

  device_config.SetLoginPolicyTables(login_policy_tables);

  // snmp trap event
  ActSnmpTrapHostEntry host_entry;
  host_entry.SetHostName("192.168.127.100");

  QList<ActSnmpTrapHostEntry> snmp_trap_settings;
  snmp_trap_settings.append(host_entry);

  ActSnmpTrapSettingTable snmp_trap_setting_table(DEVICE_ID);
  snmp_trap_setting_table.SetHostList(snmp_trap_settings);

  QMap<qint64, ActSnmpTrapSettingTable> snmp_trap_setting_tables;
  snmp_trap_setting_tables[DEVICE_ID] = snmp_trap_setting_table;

  device_config.SetSnmpTrapSettingTables(snmp_trap_setting_tables);

  // stream adapter
  ActStadPortEntry entry;
  entry.SetPortId(1);
  entry.SetIngressIndex(0);
  QSet<ActStreamPriorityTypeEnum> type_set = {ActStreamPriorityTypeEnum::kEthertype};
  entry.SetType(type_set);
  entry.SetEthertypeValue(18);
  entry.SetSubtypeEnable(1);
  entry.SetSubtypeValue(3);
  entry.SetVlanId(5);
  entry.SetVlanPcp(3);

  ActStadPortEntry entry_2;
  entry_2.SetPortId(2);
  entry_2.SetIngressIndex(1);
  QSet<ActStreamPriorityTypeEnum> type_set_2 = {ActStreamPriorityTypeEnum::kEthertype};
  entry_2.SetType(type_set);
  entry_2.SetEthertypeValue(100);
  entry_2.SetVlanId(10);
  entry_2.SetVlanPcp(2);

  QSet<ActStadPortEntry> stad_port_entries;
  stad_port_entries.insert(entry);
  stad_port_entries.insert(entry_2);

  ActInterfaceStadPortEntry port_entry;
  port_entry.SetInterfaceId(1);
  port_entry.SetStadPortEntries(stad_port_entries);

  QSet<ActInterfaceStadPortEntry> port_entries;
  port_entries.insert(port_entry);

  ActStadPortTable stream_adapter_table(DEVICE_ID);
  stream_adapter_table.SetInterfaceStadPortEntries(port_entries);

  QMap<qint64, ActStadPortTable> stream_adapter_tables;
  stream_adapter_tables[DEVICE_ID] = stream_adapter_table;

  device_config.SetStreamPriorityIngressTables(stream_adapter_tables);

  // set device config
  project.SetDeviceConfig(device_config);

  ActProject &project_ref = project;

  QString file_name;
  act_status = act::offline_config::GenerateOfflineConfig(project_ref, DEVICE_ID, file_name);

  return 0;
}
