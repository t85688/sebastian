
#include "act_auto_probe.hpp"

#include <unistd.h>

#include "act_core.hpp"
#include "act_grpc_server_process.hpp"
#include "act_status.hpp"
#include "act_unit_test.hpp"
#include "act_utilities.hpp"

namespace act {
namespace auto_probe {

class ActAutoProbeTest : public ActQuickTest {
 protected:
  QSet<ActFeatureProfile> feature_profiles;
  QSet<ActDeviceProfile> dev_profiles;
  QSet<ActDeviceProfile> default_dev_profiles;

  ACT_STATUS_INIT();

  void SetUp() override {
    // feature_profiles = ReadAllSameClassFile<ActFeatureProfile>(ACT_FEATURE_PROFILE_FOLDER);
    // default_dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(ACT_DEFAULT_DEVICE_PROFILE_FOLDER);

    act::core::g_core.InitDefaultDeviceProfiles();
    act::core::g_core.InitFeatureProfiles();
    act::core::g_core.InitDeviceProfiles();

    default_dev_profiles = act::core::g_core.GetDefaultDeviceProfileSet();
    feature_profiles = act::core::g_core.GetFeatureProfileSet();
    dev_profiles = act::core::g_core.GetDeviceProfileSet();
  }
  void TearDown() override {}
};

TEST_F(ActAutoProbeTest, AutoProbe) {
  act_status = ActGrpcServerProcess::StartGrpcServer();  // Start grpc server
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";
  sleep(2);

  // act::auto_probe::ActAutoProbe auto_probe(feature_profiles, default_dev_profiles);

  ActDevice dev("192.168.127.252");
  dev.SetId(404);

  // restful
  ActRestfulConfiguration restful_cfg("admin", "moxa", ActRestfulProtocolEnum::kHTTP, 80);

  // snmp
  ActSnmpConfiguration snmp_cfg("public", "private");

  // netconf
  ActNetconfOverSSH netconf_over_ssh_cfg("admin", "moxa");
  ActNetconfConfiguration netconf_cfg(false, netconf_over_ssh_cfg);

  // Set connection config
  dev.SetRestfulConfiguration(restful_cfg);
  dev.SetSnmpConfiguration(snmp_cfg);
  dev.SetNetconfConfiguration(netconf_cfg);

  // dev.SetDeviceProfileId(101);

  // act::core::g_core.MatchDeviceProfile(dev);
  // qDebug() << "Init Device:" << dev.ToString(dev.key_order_).toStdString().c_str();
  // for (auto dev_profile : core_dev_profiles) {
  //   qDebug() << "~~!!!dev_profile:" << dev_profile.ToString(dev_profile.key_order_).toStdString().c_str();
  // }

  // Start call auto_probe
  ActAutoProbeWarning probe_warning;
  ActDeviceProfile probe_dev_profile;
  qint64 probe_dev_icon_id;
  act_status = auto_probe.StartAutoProbe(dev, dev_profiles, probe_warning, probe_dev_profile, probe_dev_icon_id);
  qint8 times = 0;
  while (IsActStatusRunning(act_status) && (times < 120)) {
    times++;
    sleep(2);
    act_status = auto_probe.GetStatus();
    qDebug() << QString("act_status(%1)::%2")
                    .arg(QTime::currentTime().toString("hh:mm:ss"))
                    .arg(act_status->ToString())
                    .toStdString()
                    .c_str();
  }

  if (IsActStatusFinished(act_status)) {
    qDebug() << __func__
             << "probe_dev_profile:" << probe_dev_profile.ToString(probe_dev_profile.key_order_).toStdString().c_str();
    qDebug() << __func__ << "probe_warning:" << probe_warning.ToString().toStdString().c_str();
    qDebug() << __func__ << "Device:" << dev.ToString(dev.key_order_).toStdString().c_str();

    if (dev_profiles.contains(probe_dev_profile)) {
      qDebug() << __func__
               << QString("The device_profile(%1) has existed.").arg(probe_dev_profile.GetId()).toStdString().c_str();
    } else {  // upload
      // If ReIdentify(DeviceProfileId != -1) would skip upload empty DeviceProfile(Disable all features)
      if ((dev.GetDeviceProfileId() == -1) || (dev.GetDeviceProfileId() != probe_dev_profile.GetId())) {
        act::core::g_core.UploadDeviceProfile(probe_dev_profile, false);
      }
    }
  }

  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "AutoProbe failed";
}

}  // namespace auto_probe
}  // namespace act
