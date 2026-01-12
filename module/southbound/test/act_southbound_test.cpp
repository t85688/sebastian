#include "act_southbound.hpp"

#include "act_grpc_server_process.hpp"
#include "act_snmp_handler.h"
#include "act_status.hpp"
#include "act_system.hpp"
#include "act_unit_test.hpp"
#include "act_utilities.hpp"
#include "gmock/gmock.h"

// #include "act_status.hpp"

// #include "deploy_entry/act_vlan_static_entry.hpp"
// #include "topology/act_device.hpp"
#define FAKE_FOLDER "southbound_fake"

// class mockTest : public ActSnmpHandler {
//  public:
//   MOCK_METHOD2(GetInterfaceStadPort, ACT_STATUS(const ActDevice &device, ActStadPortTable &stad_port_table));
// };

class ActSouthboundTest : public ActQuickTest {
 protected:
  QSet<ActDeviceProfile> dev_profiles;
  ActSouthbound southbound;
  ActDevice device;
  ACT_STATUS_INIT();
  void SetUp() override {
    // ::testing::InitGoogleMock();
    // kene+
    /*
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(ACT_DEVICE_PROFILE_FOLDER);
    */
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(GetDeviceProfilePath());
    // kene-
    device.fromJson(ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("devices.json")).object());
  }

  void TearDown() override {}
};

TEST_F(ActSouthboundTest, ActSouthboundTest1) {
  // TODO: Need to study how to use MOCK.
  // qDebug() << "device:" << device.ToString(device.key_order_).toStdString().c_str();
  // ActStadPortTable stad_port_table(device.GetId());
  // southbound.GetStadPortTable(device, stad_port_table);
  // qDebug() << "stad_port_table:" << stad_port_table.ToString().toStdString().c_str();
  // EXPECT_CALL(mockTest, GetInterfaceStadPort(device, stad_port_table)).Times(1).WillOnce(act_status);
  // EXPECT_CALL(mockTest, GetInterfaceStadPort(device, stad_port_table));
  // act_status = southbound.GetStadPortTable(device, stad_port_table);
  // qDebug() << "act_status:" << act_status.ToString().toStdString().c_str();
}

TEST_F(ActSouthboundTest, GetGateParameters) {
  act_status = ActGrpcServerProcess::StartGrpcServer();  // Start grpc server
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";

  SLEEP_MS(3000);

  ActDevice dev("192.168.127.253", "TSN-G5008-2GTXSFP");
  dev.SetDeviceProfileId(1);
  QMap<qint64, QString> interfaces_name_map;
  interfaces_name_map.insert(1, "1");
  interfaces_name_map.insert(2, "2");
  interfaces_name_map.insert(3, "3");
  interfaces_name_map.insert(4, "4");
  QSet<QString> capabilities;
  act_status = southbound.GetDeviceNetconfCapabilities(dev, capabilities);
  for (auto revision : capabilities) {  // print
    qDebug() << __func__
             << QString("Device: %1(%2). Support YANG Revision: %3")
                    .arg(dev.GetIpv4().GetIpAddress())
                    .arg(dev.GetId())
                    .arg(revision)
                    .toStdString()
                    .c_str();
  }

  //   ActDevice dev("192.168.127.253", "TSN-G5008-2GTXSFP");
  //   dev.SetDeviceProfileId(1);
  //   QMap<qint64, QString> interfaces_name_map;
  //   interfaces_name_map.insert(1, "1");
  //   interfaces_name_map.insert(2, "2");
  //   interfaces_name_map.insert(3, "3");
  //   interfaces_name_map.insert(4, "4");
  //   QSet<QString> capabilities;
  //   // act_status = southbound.GetDeviceNetconfCapabilities(dev, capabilities);
  //   for (auto revision : capabilities) {  // print
  //     qDebug() << __func__
  //              << QString("Device: %1(%2). Support YANG Revision: %3")
  //                     .arg(dev.GetIpv4().GetIpAddress())
  //                     .arg(dev.GetId())
  //                     .arg(revision)
  //                     .toStdString()
  //                     .c_str();
  //   }

  //   EXPECT_EQ(IsActStatusSuccess(act_status), true);

  //   act_status = ActGrpcServerProcess::StopGrpcServer();
  //   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Close GRPC server failed";
  // }

  // TEST_F(ActSouthboundTest, GetDeviceVlanConfigVlanPortType) {
  //   ActDevice dev("192.168.127.253", "G-5008");
  //   dev.SetDeviceProfileId(1);
  //   // QSet<ActInterfaceIdValue> interface_id_value_set;
  //   // act_status = southbound.GetDeviceVlanConfigVlanPortType(dev, interface_id_value_set);

  //   QSet<ActVlanPortTypeEntry> vlan_port_type_entries;
  //   act_status = southbound.GetDeviceVlanConfigVlanPortType(dev, vlan_port_type_entries);

  //   for (auto entry : vlan_port_type_entries) {  // print
  //     qDebug() << __func__
  //              << QString("Port: %1, VlanPortType: %2")
  //                     .arg(entry.GetPortId())
  //                     .arg(entry.GetVlanPortType())
  //                     .toStdString()
  //                     .c_str();
  //   }

  //   EXPECT_EQ(IsActStatusSuccess(act_status), true);
  // }

  // TEST_F(ActSouthboundTest, AssignDevicesStatus) {
  //   act_status = ActGrpcServerProcess::StartGrpcServer();  // Start grpc server
  //   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";
  //   sleep(3);

  //   ActDevice dev1("192.168.127.253", "G-5008");
  //   ActDevice dev2("192.168.127.252", "G-5004");

  //   ActRestfulConfiguration restful_cfg1("admin", "moxa", ActRestfulProtocolEnum::kHTTPS, 443);
  //   ActRestfulConfiguration restful_cfg2("admin", "moxa404", ActRestfulProtocolEnum::kHTTPS, 443);
  //   dev1.SetRestfulConfiguration(restful_cfg1);
  //   dev2.SetRestfulConfiguration(restful_cfg2);

  //   ActSnmpConfiguration snmp_cfg1;
  //   snmp_cfg1.SetReadCommunity("123");
  //   dev1.SetSnmpConfiguration(snmp_cfg1);

  //   ActNetconfOverSSH netconf_over_ssh_cfg1;
  //   netconf_over_ssh_cfg1.SetAccount("404");
  //   ActNetconfConfiguration netconf_cfg1;
  //   netconf_cfg1.SetNetconfOverSSH(netconf_over_ssh_cfg1);
  //   dev1.SetNetconfConfiguration(netconf_cfg1);

  //   QSet<ActDevice> dev_set;
  //   dev_set.insert(dev1);
  //   dev_set.insert(dev2);

  //   for (auto dev : dev_set) {  // print
  //     // act_status = southbound.AssignDeviceStatus(dev);

  //     qDebug() << __func__
  //              << QString("Dev: %1, Status: %2")
  //                     .arg(dev.GetId())
  //                     .arg(dev.GetDeviceStatus().ToString().toStdString().c_str())
  //                     .toStdString()
  //                     .c_str();
  //   }
  //   EXPECT_EQ(IsActStatusSuccess(act_status), true);

  //   act_status = ActGrpcServerProcess::StopGrpcServer();
  //   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Close GRPC server failed";
  // }

  // TEST_F(ActSouthboundTest, DeleteLocalhostArpEntry) {
  //   ActDevice dev("192.168.127.253");
  //   act_status = southbound.DeleteLocalhostArpEntry(dev);
  //   EXPECT_EQ(IsActStatusSuccess(act_status), true);
  // }

  // TEST_F(ActSouthboundTest, GetLocalhostArpTable) {
  //   QMap<QString, QString> arp_table;
  //   QSet<QString> arp_entry_types;
  //   arp_entry_types << ACT_ARP_ENTRY_DYNAMIC  // dynamic
  //                   << ACT_ARP_ENTRY_STATIC;  // static
  //   act_status = southbound.GetLocalhostArpTable(arp_entry_types, arp_table);
  //   qDebug() << __func__ << "arp_table(kDynamic & kStatic):";
  //   foreach (auto ip, arp_table.keys()) {
  //     qDebug() << __func__ << QString("ip: %1, mac:%2").arg(ip).arg(arp_table[ip]).toStdString().c_str();
  //   }

  //   arp_entry_types.clear();
  //   arp_entry_types << ACT_ARP_ENTRY_STATIC;  // static
  //   act_status = southbound.GetLocalhostArpTable(arp_entry_types, arp_table);
  //   qDebug() << __func__ << "arp_table(kStatic):";
  //   foreach (auto ip, arp_table.keys()) {
  //     qDebug() << __func__ << QString("ip: %1, mac:%2").arg(ip).arg(arp_table[ip]).toStdString().c_str();
  //   }

  //   EXPECT_EQ(IsActStatusSuccess(act_status), true);
  // }
  // TEST_F(ActSouthboundTest, GetLocalhostArpTable) {
  //   QMap<QString, QString> adapter_table;
  //   act_status = southbound.GetLocalhostAdapterTable(adapter_table);
  //   qDebug() << __func__ << "adapter_table():";
  //   foreach (auto ip, adapter_table.keys()) {
  //     qDebug() << __func__ << QString("ip: %1, mac:%2").arg(ip).arg(adapter_table[ip]).toStdString().c_str();
  //   }

  //   EXPECT_EQ(IsActStatusSuccess(act_status), true);
  // }

  // TEST_F(ActSouthboundTest, GetLocalhostAdapterIpDeviceTable) {
  //   QMap<QString, QString> ip_dev_map_result;
  //   act_status = southbound.GetLocalhostAdapterIpDeviceTable(ip_dev_map_result);
  //   qDebug() << __func__ << "ip_dev_map_result():";
  //   foreach (auto ip, ip_dev_map_result.keys()) {
  //     qDebug() << __func__ << QString("IP: %1, Name: %2").arg(ip).arg(ip_dev_map_result[ip]).toStdString().c_str();
  //   }

  //   EXPECT_EQ(IsActStatusSuccess(act_status), true);
  // }

  // TEST_F(ActSouthboundTest, SetLocalhostArpTable) {
  //   ActDevice dev("192.168.127.211");
  //   dev.SetMacAddress("00-90-E8-11-22-99");
  //   // dev.SetMacAddress("00:90:e8:11:22:33");

  //   // act_status = southbound.AddArpEntry("enx00e04c680029", dev.GetIpv4().GetIpAddress(), dev.GetMacAddress());
  //   // act_status = southbound.SetLocalhostArpTable(dev, "192.168.127.242");
  //   act_status = southbound.SetLocalhostArpTable(dev, "10.123.33.21");

  //   EXPECT_EQ(IsActStatusSuccess(act_status), true);
  // }

  // TEST_F(ActSouthboundTest, DeleteLocalhostArpEntry) {
  //   ActDevice dev("192.168.127.211");

  //   // act_status = southbound.DeleteLocalhostArpEntry(dev, "192.168.127.242");
  //   act_status = southbound.DeleteLocalhostArpEntry(dev);
  //   EXPECT_EQ(IsActStatusSuccess(act_status), true);
  // }
