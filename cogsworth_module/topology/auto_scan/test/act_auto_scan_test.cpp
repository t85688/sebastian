#include "act_auto_scan.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QTime>

#include "act_core.hpp"
#include "act_device.hpp"
#include "act_scan_ip_range.hpp"
#include "act_status.hpp"
#include "act_system.hpp"
#include "act_unit_test.hpp"
#include "act_utilities.hpp"
namespace act {

namespace topology {

class ActAutoScanTest : public ActQuickTest {
 protected:
  QSet<ActDeviceProfile> dev_profiles;
  QSet<ActDeviceProfile> default_dev_profiles;
  QSet<ActFeatureProfile> feature_profiles;

  ACT_STATUS_INIT();
  void SetUp() override {
    act::core::g_core.InitDeviceProfiles();
    act::core::g_core.InitDefaultDeviceProfiles();
    act::core::g_core.InitFeatureProfiles();

    dev_profiles = act::core::g_core.GetDeviceProfileSet();
    default_dev_profiles = act::core::g_core.GetDefaultDeviceProfileSet();
    feature_profiles = act::core::g_core.GetFeatureProfileSet();
  }
  void TearDown() override {}
};

TEST_F(ActAutoScanTest, AutoScanIntegrationThread) {
  act::topology::ActAutoScan auto_scan(feature_profiles, default_dev_profiles);

  QList<ActScanIpRangeEntry> scan_ip_range_entry_list;

  // Set connection config
  ActRestfulConfiguration restful_cfg("admin", "moxa", ActRestfulProtocolEnum::kHTTP, 80);  // restful
  ActSnmpConfiguration snmp_cfg("public", "private");                                       // snmp
  ActNetconfOverSSH netconf_over_ssh_cfg("admin", "moxa");                                  // netconf
  ActNetconfConfiguration netconf_cfg(false, netconf_over_ssh_cfg);

  scan_ip_range_entry_list.append(
      ActScanIpRangeEntry(1, "192.168.127.240", "192.168.127.245", snmp_cfg, netconf_cfg, restful_cfg, true, true));
  scan_ip_range_entry_list.append(
      ActScanIpRangeEntry(2, "192.168.127.250", "192.168.127.255", snmp_cfg, netconf_cfg, restful_cfg, true, true));

  ActScanIpRange scan_ip_range;
  scan_ip_range.SetScanIpRangeEntries(scan_ip_range_entry_list);

  ActAutoScanResult auto_scan_result;

  QMap<qint64, ActDeviceIpConnectConfig> dev_ip_conn_cfg_map;
  act_status = auto_scan.Start(scan_ip_range, false, dev_ip_conn_cfg_map, dev_profiles, true, auto_scan_result);
  act_status = auto_scan.GetStatus();
  qDebug() << QString("Start act_status(%1)::%2")
                  .arg(QTime::currentTime().toString("hh:mm:ss"))
                  .arg(act_status->ToString())
                  .toStdString()
                  .c_str();

  qint8 times = 0;
  while (IsActStatusRunning(act_status) && (times < 60)) {
    times++;
    sleep(2);
    act_status = auto_scan.GetStatus();
    qDebug() << QString("act_status(%1)::%2")
                    .arg(QTime::currentTime().toString("hh:mm:ss"))
                    .arg(act_status->ToString())
                    .toStdString()
                    .c_str();
  }
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "ScanTopology failed";
}

// TEST_F(ActAutoScanTest, AutoScanIntegrationThreadStop) {
//   QSet<ActScanIpRangeEntry> scan_ip_range_entry_list;

//   ActSnmpConfiguration snmp_cfg;
//   ActNetconfConfiguration netconf_cfg;
//   ActRestfulConfiguration restful_cfg;
//   scan_ip_range_entry_list.insert(ActScanIpRangeEntry(1, ActIpv4("192.168.127.240"),
//   ActIpv4("192.168.127.255"),
//                                                 snmp_cfg, netconf_cfg, restful_cfg));

//   // scan_ip_range_entry_list.insert(ActScanIpRangeEntry(1, ActIpv4("192.168.127.250"),
//   ActIpv4("192.168.127.255"),
//   //                                               snmp_cfg, netconf_cfg, restful_cfg));

//   ActScanIpRange scan_ip_range;
//   scan_ip_range.SetScanIpRangeEntries(scan_ip_range_entry_list);

//   ActAutoScanResult auto_scan_result;

// QMap<qint64, ActDeviceIpConnectConfig> dev_ip_conn_cfg_map;
// act_status = auto_scan.Start(scan_ip_range, false, dev_ip_conn_cfg_map, dev_profiles, true, auto_scan_result);
//   act_status = auto_scan.GetStatus();
//   qDebug() << QString("Start act_status(%1)::%2")
//                   .arg(QTime::currentTime().toString("hh:mm:ss"))
//                   .arg(act_status->ToString())
//                   .toStdString()
//                   .c_str();

//   sleep(5);
//   act_status = auto_scan.Stop();
//   qDebug() << QString("Stop act_status(%1)::%2")
//                   .arg(QTime::currentTime().toString("hh:mm:ss"))
//                   .arg(act_status->ToString())
//                   .toStdString()
//                   .c_str();
//   EXPECT_NE(act_status->GetStatus(), ActStatusType::kRunning) << "ScanTopology stop failed";
// }

}  // namespace topology
}  // namespace act
