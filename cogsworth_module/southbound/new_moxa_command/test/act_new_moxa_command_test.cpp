
#include "act_new_moxa_command_handler.h"
#include "act_status.hpp"
#include "act_unit_test.hpp"

class ActNewMoxaCommandTest : public ActQuickTest {
 protected:
  ActNewMoxaCommandHandler new_moxa_command;
  ACT_STATUS_INIT();

  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ActNewMoxaCommandTest, FirmwareUpgrade) {
  ActDevice dev("192.168.127.253");
  ActRestfulConfiguration restful_cfg("admin", "moxa", ActRestfulProtocolEnum::kHTTP, 80);
  qDebug() << __func__ << QString("RestfulConfiguration11: %1").arg(restful_cfg.ToString()).toStdString().c_str();

  dev.SetRestfulConfiguration(restful_cfg);

  qDebug() << __func__
           << QString("Dev->RestfulConfiguration: %1").arg(dev.ToString("RestfulConfiguration")).toStdString().c_str();

  QString fw_file = "FWR_TSN-G5000_v2.2_2022_0720_1430.rom";
  act_status = new_moxa_command.FirmwareUpgrade(dev, fw_file);
  EXPECT_EQ(1, 1);
}
