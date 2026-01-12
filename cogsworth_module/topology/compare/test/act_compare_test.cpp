#include "act_compare.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include "act_grpc_server_process.hpp"
#include "act_project.hpp"
#include "act_status.hpp"
#include "act_system.hpp"
#include "act_unit_test.hpp"
#include "act_utilities.hpp"
#define FAKE_FOLDER "compare_fake"

namespace act {

namespace topology {}  // namespace topology

class ActCompareTest : public ActQuickTest {
 protected:
  act::topology::ActCompare compare;
  ActProject act_project;
  QSet<ActDeviceProfile> dev_profiles;
  act::topology::ActCompareControl compare_ctrl;

  ACT_STATUS_INIT();
  void SetUp() override {
    // act_project.fromJson(ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("compare_project_single.json")).object());
    act_project.fromJson(ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("compare_project.json")).object());

    // kene+
    /*
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(ACT_DEVICE_PROFILE_FOLDER);
    */
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(GetDeviceProfilePath());
    // kene-
  }
  void TearDown() override {}
};

TEST_F(ActCompareTest, CompareIntegrationThread) {
  act_status = ActGrpcServerProcess::StartGrpcServer();  // Start grpc server
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";

  act_status = compare.Start(act_project, dev_profiles, compare_ctrl);
  act_status = compare.GetStatus();
  qDebug() << QString("Start act_status(%1)::%2")
                  .arg(QTime::currentTime().toString("hh:mm:ss"))
                  .arg(act_status->ToString())
                  .toStdString()
                  .c_str();

  qint8 times = 0;
  while (IsActStatusRunning(act_status) && (times < 60)) {
    times++;
    sleep(2);
    act_status = compare.GetStatus();
    qDebug() << QString("act_status(%1)::%2")
                    .arg(QTime::currentTime().toString("hh:mm:ss"))
                    .arg(act_status->ToString())
                    .toStdString()
                    .c_str();
  }
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "CompareTopology failed";

  act_status = ActGrpcServerProcess::StopGrpcServer();
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Close GRPC server failed";
}

// TEST_F(ActCompareTest, CompareIntegrationStop) {
//   // Start grpc server
//   act_status = ActGrpcServerProcess::StartGrpcServer();
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";

//   // Start Compare
//   act_status = compare.Start(act_project, dev_profiles, compare_ctrl);
//   act_status = compare.GetStatus();
//   qDebug() << QString("Start act_status(%1)::%2")
//                   .arg(QTime::currentTime().toString("hh:mm:ss"))
//                   .arg(act_status->ToString())
//                   .toStdString()
//                   .c_str();

//   sleep(2);  // wait 2 seconds
//   // Stop Compare
//   act_status = compare.Stop();
//   qDebug() << QString("Stop act_status(%1)::%2")
//                   .arg(QTime::currentTime().toString("hh:mm:ss"))
//                   .arg(act_status->ToString())
//                   .toStdString()
//                   .c_str();
//   EXPECT_NE(act_status->GetStatus(), ActStatusType::kRunning) << "CompareTopology stop failed";

//   // Stop grpc server
//   act_status = ActGrpcServerProcess::StopGrpcServer();
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Close GRPC server failed";
// }

}  // namespace act
