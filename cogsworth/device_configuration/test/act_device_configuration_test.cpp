
#include "act_device_configuration.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif
#include "act_status.hpp"
#include "act_unit_test.hpp"
namespace act {
namespace device_configuration {

class ActDeviceConfigurationTest : public ActQuickTest {
 protected:
  // ActDeviceConfiguration device_configuration;

  QSet<ActFeatureProfile> feat_profiles;
  ACT_STATUS_INIT();

  void SetUp() override {
    act::core::g_core.InitFeatureProfiles();
    feat_profiles = act::core::g_core.GetFeatureProfileSet();

    act::core::g_core.InitDeviceProfiles();
    device_profiles = act::core::g_core.GetDeviceProfileSet();
  }
  void TearDown() override {}
};

TEST_F(ActDeviceConfigurationTest, SetNetworkSetting) {
  ActDeviceConfiguration device_configuration(feat_profiles, device_profiles);

  QList<ActDeviceIpConfiguration> dev_ip_config_list;
  ActDeviceIpConfiguration dev_ip_config;
  dev_ip_config.SetId(1);
  dev_ip_config.SetMacAddress("00-90-E8-11-22-09");
  dev_ip_config.SetOriginIp("192.168.127.253");
  dev_ip_config.SetNewIp("192.168.127.251");
  dev_ip_config.SetSubnetMask("255.255.255.0");
  dev_ip_config.SetGateway("");
  dev_ip_config.SetDNS1("");
  dev_ip_config.SetDNS2("");

  ActRestfulConfiguration restful_config;
  dev_ip_config.SetRestfulConfiguration(restful_config);
  dev_ip_config_list.append(dev_ip_config);

  QMap<QString, QString> mac_host_map;
  mac_host_map["00-90-E8-11-22-09"] = "192.168.127.241";  // 5004
  mac_host_map["00-90-E8-11-22-44"] = "192.168.127.241";  // 5008

  act_status = device_configuration.StartSetNetworkSetting(dev_ip_config_list, true);
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "SetNetworkSetting failed";
}

// TEST_F(ActDeviceConfigurationTest, SetNetworkSettingThread) {
//   QList<ActDeviceIpConfiguration> dev_ip_config_list;
//   ActDeviceIpConfiguration dev_ip_config;
//   dev_ip_config.SetId(1);
//   dev_ip_config.SetMacAddress("00-90-E8-11-22-09");
//   dev_ip_config.SetOriginIp("192.168.127.253");
//   dev_ip_config.SetNewIp("192.168.127.252");
//   dev_ip_config.SetSubnetMask("255.255.255.0");
//   dev_ip_config.SetGateway("");
//   dev_ip_config.SetDNS1("");
//   dev_ip_config.SetDNS2("");
//   ActRestfulConfiguration restful_config;
//   dev_ip_config.SetRestfulConfiguration(restful_config);
//   dev_ip_config_list.append(dev_ip_config);

//   QMap<QString, QString> mac_host_map;
//   mac_host_map["00-90-E8-11-22-09"] = "192.168.127.241";  // 5004
//   mac_host_map["00-90-E8-11-22-44"] = "192.168.127.241";  // 5008
//   act::core::g_core.SetMacHostMap(mac_host_map);

//   for (int i = 2; i <= 5; i++) {
//     ActDeviceIpConfiguration dev_ip_config_i;
//     dev_ip_config_i.SetId(i);
//     dev_ip_config_list.append(dev_ip_config_i);
//   }

//   act_status = device_configuration.StartSetNetworkSetting(dev_ip_config_list, true);
//   qint8 times = 0;
//   while (IsActStatusRunning(act_status) && (times < 60)) {
//     times++;
//     sleep(2);
//     act_status = device_configuration.GetStatus();
//     qDebug() << QString("act_status(%1)::%2")
//                     .arg(QTime::currentTime().toString("hh:mm:ss"))
//                     .arg(act_status->ToString())
//                     .toStdString()
//                     .c_str();

//     qDebug() << "Queue size::::::::" << device_configuration.result_queue_.size();
//     while (!device_configuration.result_queue_.isEmpty()) {
//       ActDeviceConfigureResult result_status = device_configuration.result_queue_.dequeue();
//       qDebug() << result_status.ToString().toStdString().c_str();
//       qDebug() << "Queue size::::::::(dequeue after)" << device_configuration.result_queue_.size();
//     }
//   }

//   // while (!device_configuration.result_queue_.isEmpty()) {
//   //   ActDeviceConfigureResult result_status = device_configuration.result_queue_.dequeue();
//   //   qDebug() << result_status.ToString().toStdString().c_str();
//   // }

//   qDebug() << "Queue size::" << device_configuration.result_queue_.size();

//   // qDebug() << "Queue size::::::::" << device_configuration.result_queue_.size();
//   // while (!device_configuration.result_queue_.isEmpty()) {
//   //   ActDeviceConfigureResult result_status = device_configuration.result_queue_.dequeue();
//   //   qDebug() << result_status.ToString().toStdString().c_str();
//   //   qDebug() << "Queue size::::::::(dequeue after)" << device_configuration.result_queue_.size();
//   // }

//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "SetNetworkSetting failed";
// }

// TEST_F(ActDeviceConfigurationTest, FirmwareUpgrade) {
//   QList<ActDevice> dev_list;

//   ActDevice dev("192.168.127.253");
//   ActRestfulConfiguration restful_cfg("admin", "moxa", ActRestfulProtocolEnum::kHTTPS,443);
//   dev.SetRestfulConfiguration(restful_cfg);
//   dev_list.append(dev);

//   act_status = device_configuration.StartFirmwareUpgrade(dev_list, "FWR_TSN-G5000_v2.2_2022_0712_2011.rom");
//   qint8 times = 0;
//   while (IsActStatusRunning(act_status) && (times < 120)) {
//     times++;
//     sleep(2);
//     act_status = device_configuration.GetStatus();
//     qDebug() << QString("act_status(%1)::%2")
//                     .arg(QTime::currentTime().toString("hh:mm:ss"))
//                     .arg(act_status->ToString())
//                     .toStdString()
//                     .c_str();

//     qDebug() << "Queue size::::::::" << device_configuration.result_queue_.size();
//     while (!device_configuration.result_queue_.isEmpty()) {
//       ActDeviceConfigureResult result_status = device_configuration.result_queue_.dequeue();
//       qDebug() << result_status.ToString().toStdString().c_str();
//       qDebug() << "Queue size::::::::(dequeue after)" << device_configuration.result_queue_.size();
//     }
//   }

//   qDebug() << "Queue size::" << device_configuration.result_queue_.size();

//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "FirmwareUpgrade failed";
// }

}  // namespace device_configuration

}  // namespace act
