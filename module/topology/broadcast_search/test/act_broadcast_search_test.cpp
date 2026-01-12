
#include "act_broadcast_search.hpp"

#include "act_core.hpp"
#include "act_grpc_server_process.hpp"
#include "act_status.hpp"
#include "act_unit_test.hpp"
#include "act_utilities.hpp"

namespace act {

namespace topology {

class ActBroadcastSearchTest : public ActQuickTest {
 protected:
  ActDeviceDiscoveryConfig discover_cfg;
  ActRetryConnectConfig retry_connect_cfg;
  QSet<ActFeatureProfile> feat_profiles;
  QSet<ActDeviceProfile> dev_profiles;

  ACT_STATUS_INIT();

  void SetUp() override {
    // dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(ACT_DEVICE_PROFILE_FOLDER);
    act::core::g_core.InitFeatureProfiles();
    feat_profiles = act::core::g_core.GetFeatureProfileSet();

    act::core::g_core.InitDeviceProfiles();
    dev_profiles = act::core::g_core.GetDeviceProfileSet();
  }
  void TearDown() override {}
};

// TEST_F(ActBroadcastSearchTest, ActBroadcastSearchTest1) {
//   broadcast_search.LinkSequenceDetect();
//   EXPECT_EQ(1, 1);
// }

// TEST_F(ActBroadcastSearchTest, DeviceDiscoveryBroadcastSearchAndRetry) {

//   // DeviceDiscoveryBroadcastSearch

//   QList<ActDevice> devices;
//   // act_status = broadcast_search.DeviceDiscoveryBroadcastSearch(define_device_type, define_network_interface,
//   //                                                              restful_cfg, snmp_cfg, netconf_cfg,
//   devices); act_status = broadcast_search.DeviceDiscoveryBroadcastSearch(discover_cfg, devices);
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "DeviceDiscoveryBroadcastSearch failed";

//   // RetryConnect
//   QList<qint64> devices_id;
//   devices_id << 1;
//   devices_id << 2;
//   QList<ActDevice> result_devices;

//   act_status = broadcast_search.RetryConnect(retry_connect_cfg,
// result_devices);

// EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "RetryConnect failed";
// }

// TEST_F(ActBroadcastSearchTest, DeviceDiscoveryThread) {
//   // Start grpc server
//   act_status = ActGrpcServerProcess::StartGrpcServer();
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";

//   act::topology::ActBroadcastSearch broadcast_search(feat_profiles);

//   // Set discover_cfg
//   // restful
//   ActRestfulConfiguration restful_cfg("admin", "moxa",ActRestfulProtocolEnum::kHTTP, 80);
//   // snmp
//   ActSnmpConfiguration snmp_cfg("public", "private");
//   // netconf
//   ActNetconfOverSSH netconf_over_ssh_cfg("admin", "moxa");
//   ActNetconfConfiguration netconf_cfg(false, netconf_over_ssh_cfg);
//   discover_cfg.SetRestfulConfiguration(restful_cfg);
//   discover_cfg.SetSnmpConfiguration(snmp_cfg);
//   discover_cfg.SetNetconfConfiguration(netconf_cfg);

//   QList<ActDevice> devices;

//   act_status = broadcast_search.StartDeviceDiscovery(discover_cfg, dev_profiles, devices);
//   act_status = broadcast_search.GetStatus();
//   qDebug() << QString("Start act_status(%1)::%2")
//                   .arg(QTime::currentTime().toString("hh:mm:ss"))
//                   .arg(act_status->ToString())
//                   .toStdString()
//                   .c_str();

//   qint8 times = 0;
//   while (IsActStatusRunning(act_status) && (times < 60)) {
//     times++;
//     sleep(2);
//     act_status = broadcast_search.GetStatus();
//     qDebug() << QString("act_status(%1)::%2")
//                     .arg(QTime::currentTime().toString("hh:mm:ss"))
//                     .arg(act_status->ToString())
//                     .toStdString()
//                     .c_str();
//   }
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "DeviceDiscovery failed";

//   // Stop grpc server
//   act_status = ActGrpcServerProcess::StopGrpcServer();
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Close GRPC server failed";

//   for (auto dev : devices) {
//     qDebug() << dev.ToString().toStdString().c_str();
//   }
// }

// TEST_F(ActBroadcastSearchTest, RetryConnectThread) {
//   // RetryConnect
//   QList<qint64> devices_id;
//   devices_id << 2;
//   QList<ActDevice> result_devices;
//   act_status =
//       broadcast_search.StartRetryConnect(retry_connect_cfg, result_devices);

//   qint8 times = 0;
//   while (IsActStatusRunning(act_status) && (times < 60)) {
//     times++;
//     sleep(2);
//     act_status = broadcast_search.GetStatus();
//     qDebug() << QString("act_status(%1)::%2")
//                     .arg(QTime::currentTime().toString("hh:mm:ss"))
//                     .arg(act_status->ToString())
//                     .toStdString()
//                     .c_str();
//   }
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "RetryConnect failed";
// }

// TEST_F(ActBroadcastSearchTest, DeviceDiscoveryThreadAndRetryConnect) {
//   // Start grpc server
//   act_status = ActGrpcServerProcess::StartGrpcServer();
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";

//   // DeviceDiscovery
//   QList<ActDevice> devices;
//   ActRestfulConfiguration restful_cfg_error("admin", "moxa404",ActRestfulProtocolEnum::kHTTP, 80);
//   act_status = broadcast_search.StartDeviceDiscovery(discover_cfg, dev_profiles, devices);

//   qint8 times = 0;
//   while (IsActStatusRunning(act_status) && (times < 60)) {
//     times++;
//     sleep(2);
//     act_status = broadcast_search.GetStatus();
//     qDebug() << QString("act_status(%1)::%2")
//                     .arg(QTime::currentTime().toString("hh:mm:ss"))
//                     .arg(act_status->ToString())
//                     .toStdString()
//                     .c_str();
//   }
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "DeviceDiscovery failed";

//   // RetryConnect
//   QList<qint64> devices_id;
//   devices_id << 2;
//   QList<ActDevice> result_devices;
//   act_status =
//       broadcast_search.StartRetryConnect(retry_connect_cfg, result_devices);

//   times = 0;
//   while (IsActStatusRunning(act_status) && (times < 60)) {
//     times++;
//     sleep(2);
//     act_status = broadcast_search.GetStatus();
//     qDebug() << QString("act_status(%1)::%2")
//                     .arg(QTime::currentTime().toString("hh:mm:ss"))
//                     .arg(act_status->ToString())
//                     .toStdString()
//                     .c_str();
//   }
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "RetryConnect failed";

//   // Stop grpc server
//   act_status = ActGrpcServerProcess::StopGrpcServer();
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Close GRPC server failed";
// }

// TEST_F(ActBroadcastSearchTest, DeviceDiscoveryStopThread) {
//   // DeviceDiscovery
//   QList<ActDevice> devices;
//   ActRestfulConfiguration restful_cfg_error("admin", "moxa404",ActRestfulProtocolEnum::kHTTP, 80);
//   act_status = broadcast_search.StartDeviceDiscovery(discover_cfg, dev_profiles, devices);

//   sleep(2);  // wait 2 seconds
//   // Stop
//   act_status = broadcast_search.Stop();
//   qDebug() << QString("Stop act_status(%1)::%2")
//                   .arg(QTime::currentTime().toString("hh:mm:ss"))
//                   .arg(act_status->ToString())
//                   .toStdString()
//                   .c_str();
//   sleep(2);
//   EXPECT_NE(act_status->GetStatus(), ActStatusType::kRunning) << "DeviceDiscovery stop failed";
// }

// TEST_F(ActBroadcastSearchTest, LinkDistanceDetect) {
//   // Start grpc server
//   act_status = ActGrpcServerProcess::StartGrpcServer();
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";

//   QList<ActDevice> devices;

//   act_status = broadcast_search.StartDeviceDiscovery(discover_cfg, dev_profiles, devices);
//   act_status = broadcast_search.GetStatus();
//   qDebug() << QString("Start act_status(%1)::%2")
//                   .arg(QTime::currentTime().toString("hh:mm:ss"))
//                   .arg(act_status->ToString())
//                   .toStdString()
//                   .c_str();

//   qint8 times = 0;
//   while (IsActStatusRunning(act_status) && (times < 60)) {
//     times++;
//     sleep(2);
//     act_status = broadcast_search.GetStatus();
//     qDebug() << QString("act_status(%1)::%2")
//                     .arg(QTime::currentTime().toString("hh:mm:ss"))
//                     .arg(act_status->ToString())
//                     .toStdString()
//                     .c_str();
//   }
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "DeviceDiscovery failed";

//   QList<ActDeviceDistanceEntry> distance_entry_list;
//   act_status = broadcast_search.LinkDistanceDetect(distance_entry_list);
//   qDebug() << "Result distance_entry_list:";
//   for (auto entry : distance_entry_list) {
//     qDebug() << entry.ToString().toStdString().c_str();
//   }

//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "LinkDistanceDetect failed";

//   // Stop grpc server
//   act_status = ActGrpcServerProcess::StopGrpcServer();
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Close GRPC server failed";
// }

TEST_F(ActBroadcastSearchTest, DeviceDiscoveryThreadAndLinkDistanceDetect) {
  // Start grpc server
  act_status = ActGrpcServerProcess::StartGrpcServer();
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";

  act::topology::ActBroadcastSearch broadcast_search(feat_profiles);

  // DeviceDiscovery
  QList<ActDevice> devices;
  ActDeviceDiscoveryConfig discover_cfg;
  // Set discover_cfg
  // restful
  ActRestfulConfiguration restful_cfg("admin", "moxa", ActRestfulProtocolEnum::kHTTP, 80);
  // snmp
  ActSnmpConfiguration snmp_cfg("public", "private");
  // netconf
  ActNetconfOverSSH netconf_over_ssh_cfg("admin", "moxa");
  ActNetconfConfiguration netconf_cfg(false, netconf_over_ssh_cfg);
  discover_cfg.SetRestfulConfiguration(restful_cfg);
  discover_cfg.SetSnmpConfiguration(snmp_cfg);
  discover_cfg.SetNetconfConfiguration(netconf_cfg);

  // access
  act_status = broadcast_search.StartDeviceDiscovery(discover_cfg, dev_profiles, devices);

  qint8 times = 0;
  while (IsActStatusRunning(act_status) && (times < 60)) {
    times++;
    sleep(2);
    act_status = broadcast_search.GetStatus();
    qDebug() << QString("DeviceDiscovery act_status(%1)::%2")
                    .arg(QTime::currentTime().toString("hh:mm:ss"))
                    .arg(act_status->ToString())
                    .toStdString()
                    .c_str();
  }
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "DeviceDiscovery failed";

  // LinkDistanceDetect
  QList<ActDeviceDistanceEntry> distance_entry_list;
  act_status = broadcast_search.StartLinkDistanceDetect(distance_entry_list);
  times = 0;
  while (IsActStatusRunning(act_status) && (times < 60)) {
    times++;
    sleep(2);
    act_status = broadcast_search.GetStatus();
    qDebug() << QString("LinkDistanceDetect act_status(%1)::%2")
                    .arg(QTime::currentTime().toString("hh:mm:ss"))
                    .arg(act_status->ToString())
                    .toStdString()
                    .c_str();
  }
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "LinkDistanceDetect failed";
  for (auto entry : distance_entry_list) {
    qDebug() << entry.ToString().toStdString().c_str();
  }

  // Stop grpc server
  act_status = ActGrpcServerProcess::StopGrpcServer();
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Close GRPC server failed";
}

// TEST_F(ActBroadcastSearchTest, ActDeviceDistanceEntryTest) {
//   QList<ActDeviceDistanceEntry> result;
//   result.append(ActDeviceDistanceEntry(1, 5));
//   result.append(ActDeviceDistanceEntry(2, 4));
//   result.append(ActDeviceDistanceEntry(5, 1));
//   result.append(ActDeviceDistanceEntry(3, 3));
//   result.append(ActDeviceDistanceEntry(3, 2));
//   result.append(ActDeviceDistanceEntry(13, 2));
//   result.append(ActDeviceDistanceEntry(11, 2));
//   result.append(ActDeviceDistanceEntry(4, 2));

//   // std::sort(result.begin(), result.end(), ActDeviceDistanceEntry::SortByDistance);
//   std::sort(result.begin(), result.end());

//   for (auto entry : result) {
//     qDebug() << entry.ToString().toStdString().c_str();
//   }
// }

}  // namespace topology

}  // namespace act