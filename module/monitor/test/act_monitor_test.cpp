#include "act_monitor.hpp"

#include "act_status.hpp"
// #include "act_unit_test.hpp"
#include "act_utilities.hpp"

int main(void) {
  qint8 times = 0;
  while ((times < 120)) {
    times++;
    sleep(2);
    qDebug() << "times:" << times;

    // act_status = monitor.GetStatus();
    // qDebug() << QString("act_status(%1)::%2")
    //                 .arg(QTime::currentTime().toString("hh:mm:ss"))
    //                 .arg(act_status->ToString())
    //                 .toStdString()
    //                 .c_str();

    // qDebug() << "Current Queue size(in queue):" << monitor.result_queue_.size();
    // while (!monitor.result_queue_.isEmpty()) {
    //   auto queue_result_status = monitor.result_queue_.dequeue();
    //   qDebug() << queue_result_status.ToString().toStdString().c_str();
    //   qDebug() << "Current Queue size(dequeue after):" << monitor.result_queue_.size();
    // }
  }

  return 0;
}

// class ActMonitorTest : public ActQuickTest {
//  protected:
//   ActMonitor monitor;
//   // QSet<ActDeviceProfile> dev_profiles;
//   ACT_STATUS_INIT();

//   void SetUp() override {
//     // dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(ACT_DEVICE_PROFILE_FOLDER);

//     // act::core::g_core.InitDeviceProfiles();
//   }
//   void TearDown() override {}
// };

// TEST_F(ActMonitorTest, Test1) {
//   // act_status = ActGrpcServerProcess::StartGrpcServer();  // Start grpc server
//   // EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";
//   // sleep(2);

//   // // Create test device
//   // ActDevice dev("192.168.127.253");
//   // dev.SetId(404);
//   // dev.SetDeviceProfileId(1001);  // TSN-G5008-2GTXSFP, v2.2
//   // // restful
//   // ActRestfulConfiguration restful_cfg("admin", "moxa", ActRestfulProtocolEnum::kHTTP, 80);

//   // // snmp
//   // ActSnmpConfiguration snmp_cfg("public", "private");

//   // // netconf
//   // ActNetconfOverSSH netconf_over_ssh_cfg("admin", "moxa");
//   // ActNetconfConfiguration netconf_cfg(false, netconf_over_ssh_cfg);

//   // // Set connection config
//   // dev.SetRestfulConfiguration(restful_cfg);
//   // dev.SetSnmpConfiguration(snmp_cfg);
//   // dev.SetNetconfConfiguration(netconf_cfg);

//   // // Set Device status
//   // ActDeviceStatus dev_status;
//   // dev_status.SetAllConnectStatus(true);

//   // // Match device profile
//   // act::core::g_core.MatchDeviceProfile(dev);
//   // // qDebug() << "Init Device:" << dev.ToString(dev.key_order_).toStdString().c_str();

//   // QList<ActDevice> dev_list;
//   // dev_list.append(dev);

//   // // act_status = monitor.Start(dev_list, dev_profiles);
//   // act_status = monitor.Start(dev_list);

//   // qint8 times = 0;
//   // while (IsActStatusRunning(act_status) && (times < 120)) {
//   //   times++;
//   //   sleep(2);
//   //   act_status = monitor.GetStatus();
//   //   qDebug() << QString("act_status(%1)::%2")
//   //                   .arg(QTime::currentTime().toString("hh:mm:ss"))
//   //                   .arg(act_status->ToString())
//   //                   .toStdString()
//   //                   .c_str();

//   //   qDebug() << "Current Queue size(in queue):" << monitor.result_queue_.size();
//   //   while (!monitor.result_queue_.isEmpty()) {
//   //     auto queue_result_status = monitor.result_queue_.dequeue();
//   //     qDebug() << queue_result_status.ToString().toStdString().c_str();
//   //     qDebug() << "Current Queue size(dequeue after):" << monitor.result_queue_.size();
//   //   }
//   // }

//   // qDebug() << "Queue size(Finished):" << monitor.result_queue_.size();
//   // EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "Failed";
// }