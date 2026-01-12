#include "act_restful_client_handler.h"
#include "act_status.hpp"
#include "act_system.hpp"
#include "act_unit_test.hpp"

class ActRestfulClientTest : public ActQuickTest {
 protected:
  ActRestfulClientHandler restful_client;

  ACT_STATUS_INIT();

  ActDevice device;
  void SetUp() override {
    ActDevice new_device("192.168.127.253", "TSN-G5004");

    ActRestfulConfiguration restful_cfg;
    restful_cfg.SetUsername("admin");
    restful_cfg.SetPassword("moxa");
    // restful_cfg.SetPort(443);
    new_device.SetRestfulConfiguration(restful_cfg);
    device = new_device;
  }
  void TearDown() override {}
};
// TEST_F(ActRestfulClientTest, SetSnmpServiceMoxaIEIMoxa) {
//   ActClientSnmpService snmp_service_config;
//   snmp_service_config.Setmode(3);
//   act_status = restful_client.SetSnmpServiceMoxaIEIMoxa(device, snmp_service_config);

//   EXPECT_EQ(IsActStatusSuccess(act_status), true);
// }

// TEST_F(ActRestfulClientTest, GetModelNameMoxa) {
//   act_status = restful_client.GetModelNameMoxa(device);

//   EXPECT_EQ(IsActStatusSuccess(act_status), true);
// }

// TEST_F(ActRestfulClientTest, GetSerialNumber) {
//   QString result;

//   act_status = restful_client.GetStringRequestUseToken(
//       device, ActMoxaRequestTypeEnum::kGetSerialNumber, result);

//   qDebug() << __func__ << QString("Result:%1").arg(result).toStdString().c_str();
//   EXPECT_EQ(IsActStatusSuccess(act_status), true);
// }

// TEST_F(ActRestfulClientTest, GetNetmask) {
//   QString result;

//   act_status = restful_client.GetStringRequestUseToken(
//       device, ActMoxaRequestTypeEnum::kGetNetmask, result);

//   qDebug() << __func__ << QString("Result:%1").arg(result).toStdString().c_str();
//   EXPECT_EQ(IsActStatusSuccess(act_status), true);
// }

// TEST_F(ActRestfulClientTest, GetGateway) {
//   QString result;

//   act_status = restful_client.GetStringRequestUseToken(
//       device, ActMoxaRequestTypeEnum::kGetGateway, result);

//   qDebug() << __func__ << QString("Result:%1").arg(result).toStdString().c_str();
//   EXPECT_EQ(IsActStatusSuccess(act_status), true);
// }

// TEST_F(ActRestfulClientTest, RebootMoxa) {
//   QString result;

//   act_status = restful_client.RebootMoxa(device);
//   EXPECT_EQ(IsActStatusSuccess(act_status), true);
// }

TEST_F(ActRestfulClientTest, FactoryDefaultMoxa) {
  QString result;

  act_status = restful_client.FactoryDefaultMoxa(device);
  EXPECT_EQ(IsActStatusSuccess(act_status), true);
}

// TEST_F(ActRestfulClientTest, SetNetworkSettingsMoxa) {
//   QString result;

//   ActIpv4 mod_ipv4("192.168.127.251");
//   mod_ipv4.SetSubnetMask("255.255.0.0");
//   // // mod_ipv4.SetGateway("192.168.127.254");

//   act_status = restful_client.SetNetworkSettingsMoxa(device, mod_ipv4);
//   EXPECT_EQ(IsActStatusSuccess(act_status), true);
// }
