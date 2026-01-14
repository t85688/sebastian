
#include "act_deploy.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QFile>
#include <QtGlobal>

#include "act_deploy_path_data.hpp"
#include "act_grpc_server_process.hpp"
#include "act_project.hpp"
#include "act_unit_test.hpp"
#include "json_utils.hpp"
// #include "moc_deploy_test.cpp"
#include "act_computed_result.hpp"
#include "act_utilities.hpp"

#define FAKE_FOLDER "deploy_fake"

namespace act {
namespace deploy {

class ActDeployMulticastTest : public ActQuickTest {
 protected:
  ActProject act_project;
  QSet<ActDeviceProfile> dev_profiles;
  act::deploy::ActDeployControl deploy_ctrl;
  act::deploy::ActDeployPathData deploy_path_data;
  act::deploy::ActDeploy deploy;

  ACT_STATUS_INIT();
  void SetUp() override {
    // kene+
    /*
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(ACT_DEVICE_PROFILE_FOLDER);
    */
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(GetDeviceProfilePath());
    // kene-
    act_project.fromJson(ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("multicast_project.json")).object());
    deploy_ctrl.SetVlan(true);
    deploy_ctrl.SetUnicastStaticForward(true);
    deploy_ctrl.SetMulticastStaticForward(true);
    deploy_ctrl.SetStreamPriority(true);
    deploy_ctrl.SetGateControl(true);
    deploy_ctrl.SetIeee802Dot1Cb(true);
    deploy_ctrl.SetSpanningTree(true);

    deploy_path_data = act::deploy::ActDeployPathData(deploy_ctrl, act_project, dev_profiles);
    deploy_path_data.GenerateData();
  }

  void TearDown() override {}
};

class ActDeployMulticastTest22 : public ActQuickTest {
  // Switch's VlanHybridCapable is true
 protected:
  ActProject act_project;
  QSet<ActDeviceProfile> dev_profiles;
  act::deploy::ActDeployControl deploy_ctrl;
  act::deploy::ActDeployPathData deploy_path_data;
  act::deploy::ActDeploy deploy;

  ACT_STATUS_INIT();
  void SetUp() override {
    // kene+
    /*
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(ACT_DEVICE_PROFILE_FOLDER);
    */
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(GetDeviceProfilePath());
    // kene-
    act_project.fromJson(ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("multicast_project_2.2.json")).object());
    deploy_ctrl.SetVlan(true);
    deploy_ctrl.SetUnicastStaticForward(true);
    deploy_ctrl.SetMulticastStaticForward(true);
    deploy_ctrl.SetStreamPriority(true);
    deploy_ctrl.SetGateControl(true);
    deploy_ctrl.SetIeee802Dot1Cb(true);
    deploy_ctrl.SetSpanningTree(true);
  }

  void TearDown() override {}
};

class ActDeployMulticastInactiveStadTest : public ActQuickTest {
 protected:
  ActProject act_project;
  QSet<ActDeviceProfile> dev_profiles;
  act::deploy::ActDeployControl deploy_ctrl;
  act::deploy::ActDeployPathData deploy_path_data;
  act::deploy::ActDeploy deploy;

  ACT_STATUS_INIT();
  void SetUp() override {
    // kene+
    /*
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(ACT_DEVICE_PROFILE_FOLDER);
    */
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(GetDeviceProfilePath());
    // kene-
    act_project.fromJson(
        ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("multicast_inactive_stad_project.json")).object());
    deploy_ctrl.SetVlan(true);
    deploy_ctrl.SetUnicastStaticForward(true);
    deploy_ctrl.SetMulticastStaticForward(true);
    deploy_ctrl.SetStreamPriority(true);
    deploy_ctrl.SetGateControl(true);
    deploy_ctrl.SetIeee802Dot1Cb(true);
    deploy_ctrl.SetSpanningTree(true);
  }
  void TearDown() override {}
};

class ActDeployUnicastTest : public ActQuickTest {
 protected:
  ActProject act_project;
  QSet<ActDeviceProfile> dev_profiles;
  act::deploy::ActDeployControl deploy_ctrl;
  act::deploy::ActDeployPathData deploy_path_data;

  ACT_STATUS_INIT();
  void SetUp() override {
    // kene+
    /*
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(ACT_DEVICE_PROFILE_FOLDER);
    */
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(GetDeviceProfilePath());
    // kene-
    act_project.fromJson(ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("unitcast_project.json")).object());
    deploy_ctrl.SetVlan(true);
    deploy_ctrl.SetUnicastStaticForward(true);
    deploy_ctrl.SetMulticastStaticForward(true);
    deploy_ctrl.SetStreamPriority(true);
    deploy_ctrl.SetGateControl(true);
    deploy_ctrl.SetIeee802Dot1Cb(true);
    deploy_ctrl.SetSpanningTree(true);
    deploy_path_data = act::deploy::ActDeployPathData(deploy_ctrl, act_project, dev_profiles);
    deploy_path_data.GenerateData();
  }

  void TearDown() override {}
};

class ActDeployUnicastTestUntag : public ActQuickTest {
 protected:
  ActProject act_project;
  QSet<ActDeviceProfile> dev_profiles;
  act::deploy::ActDeployControl deploy_ctrl;
  act::deploy::ActDeployPathData deploy_path_data;
  act::deploy::ActDeploy deploy;

  ACT_STATUS_INIT();
  void SetUp() override {
    // kene+
    /*
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(ACT_DEVICE_PROFILE_FOLDER);
    */
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(GetDeviceProfilePath());
    // kene-
    act_project.fromJson(ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("unitcast_untag_project.json")).object());
    deploy_ctrl.SetVlan(true);
    deploy_ctrl.SetUnicastStaticForward(true);
    deploy_ctrl.SetMulticastStaticForward(true);
    deploy_ctrl.SetStreamPriority(true);
    deploy_ctrl.SetGateControl(true);
    deploy_ctrl.SetIeee802Dot1Cb(true);
    deploy_ctrl.SetSpanningTree(true);
  }

  void TearDown() override {}
};

class ActDeployUnicastTestUntag22 : public ActQuickTest {
 protected:
  ActProject act_project;
  QSet<ActDeviceProfile> dev_profiles;
  act::deploy::ActDeployControl deploy_ctrl;
  act::deploy::ActDeployPathData deploy_path_data;
  act::deploy::ActDeploy deploy;

  ACT_STATUS_INIT();
  void SetUp() override {
    // kene+
    /*
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(ACT_DEVICE_PROFILE_FOLDER);
    */
    dev_profiles = ReadAllSameClassFile<ActDeviceProfile>(GetDeviceProfilePath());
    // kene-
    act_project.fromJson(
        ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("unitcast_untag_project_2.2.json")).object());
    deploy_ctrl.SetVlan(true);
    deploy_ctrl.SetUnicastStaticForward(true);
    deploy_ctrl.SetMulticastStaticForward(true);
    deploy_ctrl.SetStreamPriority(true);
    deploy_ctrl.SetGateControl(true);
    deploy_ctrl.SetIeee802Dot1Cb(true);
    deploy_ctrl.SetSpanningTree(true);
  }

  void TearDown() override {}
};
// TEST_F(ActDeployMulticastTest, GenerateData_GclResult) {
//   QSet<ActGclTable> gate_control_tables = deploy_path_data.GetGclTables();
//   QJsonArray gcl_array = ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("multicast_generate_gcl.json")).array();
//   ActGclTable answer_gcl_table;
//   for (auto gcl_j : gcl_array) {  // for each answer table
//     answer_gcl_table.fromJson(gcl_j);
//     QSet<ActGclTable>::iterator gcl_table_iter = gate_control_tables.find(answer_gcl_table);
//     ASSERT_NE(gcl_table_iter, gate_control_tables.end())
//         << "Table not found. DeviceID:" << answer_gcl_table.GetDeviceId();
//     EXPECT_EQ(gcl_table_iter->ToString(), answer_gcl_table.ToString())
//         << "Compare table failed. Failed table:" << gcl_table_iter->ToString().toStdString().c_str();
//   }
// }

// TEST_F(ActDeployMulticastTest, GenerateData_VlanStaticResult) {
//   QSet<ActVlanStaticTable> vlan_tables = deploy_path_data.GetVlanTables();
//   // qDebug() << "ActVlanStaticTables:";
//   // for (auto table : vlan_tables) {
//   //   qDebug() << table.ToString().toStdString().c_str();
//   // }
//   QJsonArray vlan_array =
//   ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("multicast_generate_vlan.json")).array(); ActVlanStaticTable
//   answer_vlan_table; for (auto vlan_j : vlan_array) {  // for each answer table
//     answer_vlan_table.fromJson(vlan_j);
//     QSet<ActVlanStaticTable>::iterator vlan_table_iter = vlan_tables.find(answer_vlan_table);
//     ASSERT_NE(vlan_table_iter, vlan_tables.end()) << "Table not found. DeviceID:" << answer_vlan_table.GetDeviceId();
//     EXPECT_EQ(vlan_table_iter->ToString(), answer_vlan_table.ToString())
//         << "Compare table failed. Failed table:" << vlan_table_iter->ToString().toStdString().c_str();
//   }
// }

// TEST_F(ActDeployMulticastTest, GenerateData_MulticastStaticForwardResult) {
//   QSet<ActStaticForwardTable> static_forward_tables = deploy_path_data.GetMulticastStaticForwardTables();
//   QJsonArray static_forward_array =
//       ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("multicast_generate_static_forward.json")).array();
//   ActStaticForwardTable answer_static_forward_table;
//   // for (auto table : static_forward_tables) {
//   //   qDebug() << "static_forward_tables:" << table.ToString().toStdString().c_str();
//   // }

//   for (auto static_forward_j : static_forward_array) {  // for each answer table
//     answer_static_forward_table.fromJson(static_forward_j);
//     QSet<ActStaticForwardTable>::iterator static_forward_table_iter =
//         static_forward_tables.find(answer_static_forward_table);

//     ASSERT_NE(static_forward_table_iter, static_forward_tables.end())
//         << "Table not found. DeviceID:" << answer_static_forward_table.GetDeviceId();
//     EXPECT_EQ(static_forward_table_iter->ToString(), answer_static_forward_table.ToString())
//         << "Compare table failed. Failed table:" << static_forward_table_iter->ToString().toStdString().c_str();
//   }
// }

// TEST_F(ActDeployMulticastTest, GenerateData_StreamPriorityResult) {
//   // StreamPriority Port (Ingress)
//   QSet<ActStadPortTable> stad_port_tables = deploy_path_data.GetStadPortTables();
//   // qDebug() << "ActStadPortTables:";
//   // for (auto table : stad_port_tables) {
//   //   qDebug() << table.ToString().toStdString().c_str();
//   // }

//   QJsonArray stad_port_array =
//       ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("multicast_generate_stad_port.json")).array();
//   ActStadPortTable answer_stad_port_table;
//   for (auto stad_port_j : stad_port_array) {  // for each answer table
//     answer_stad_port_table.fromJson(stad_port_j);
//     QSet<ActStadPortTable>::iterator stad_port_table_iter = stad_port_tables.find(answer_stad_port_table);
//     ASSERT_NE(stad_port_table_iter, stad_port_tables.end())
//         << "Table not found. DeviceID:" << answer_stad_port_table.GetDeviceId();
//     EXPECT_EQ(stad_port_table_iter->ToString(), answer_stad_port_table.ToString())
//         << "Compare table failed. Failed table:" << stad_port_table_iter->ToString().toStdString().c_str();
//   }

//   // StreamPriority Config (Egress)
//   QSet<ActStadConfigTable> stad_config_tables = deploy_path_data.GetStadConfigTables();
//   // qDebug() << "ActStadConfigTables:";
//   // for (auto table : stad_config_tables) {
//   //   qDebug() << table.ToString().toStdString().c_str();
//   // }

//   QJsonArray stad_config_array =
//       ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("multicast_generate_stad_config.json")).array();
//   ActStadConfigTable answer_stad_config_table;
//   for (auto stad_config_j : stad_config_array) {  // for each answer table
//     answer_stad_config_table.fromJson(stad_config_j);
//     QSet<ActStadConfigTable>::iterator stad_config_table_iter = stad_config_tables.find(answer_stad_config_table);
//     ASSERT_NE(stad_config_table_iter, stad_config_tables.end())
//         << "Table not found. DeviceID:" << answer_stad_config_table.GetDeviceId();
//     EXPECT_EQ(stad_config_table_iter->ToString(), answer_stad_config_table.ToString())
//         << "Compare table failed. Failed table:" << stad_config_table_iter->ToString().toStdString().c_str();
//   }
// }

// // // [feat:529] untag - hybrid capable
// TEST_F(ActDeployMulticastTest22, GenerateData_VlanStaticResult) {
// deploy_path_data = act::deploy::ActDeployPathData(deploy_ctrl, act_project, dev_profiles);
// deploy_path_data.GenerateData();

//   QSet<ActVlanStaticTable> vlan_tables = deploy_path_data.GetVlanTables();
//   // qDebug() << "ActVlanStaticTables:";
//   // for (auto table : vlan_tables) {
//   //   qDebug() << table.ToString().toStdString().c_str();
//   // }
//   QJsonArray vlan_array =
//       ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("multicast_generate_vlan_2.2.json")).array();
//   ActVlanStaticTable answer_vlan_table;
//   for (auto vlan_j : vlan_array) {  // for each answer table
//     answer_vlan_table.fromJson(vlan_j);
//     QSet<ActVlanStaticTable>::iterator vlan_table_iter = vlan_tables.find(answer_vlan_table);
//     ASSERT_NE(vlan_table_iter, vlan_tables.end()) << "Table not found. DeviceID:" << answer_vlan_table.GetDeviceId();
//     EXPECT_EQ(vlan_table_iter->ToString(), answer_vlan_table.ToString())
//         << "Compare table failed. Failed table:" << vlan_table_iter->ToString().toStdString().c_str();
//   }
// }

// // // [feat:529] untag - hybrid capable
// TEST_F(ActDeployMulticastTest22, GenerateData_StreamPriorityResult) {
// deploy_path_data = act::deploy::ActDeployPathData(deploy_ctrl, act_project, dev_profiles);
// deploy_path_data.GenerateData();
//   // StreamPriority Config (Egress)
//   QSet<ActStadConfigTable> stad_config_tables = deploy_path_data.GetStadConfigTables();
//   EXPECT_EQ(stad_config_tables.size(), 0) << "Compare table failed.";
// }

// // // [feat:529] untag - hybrid capable
// TEST_F(ActDeployUnicastTestUntag22, GenerateData_Untag) {

// deploy_path_data = act::deploy::ActDeployPathData(deploy_ctrl, act_project, dev_profiles);
// deploy_path_data.GenerateData();
//   QSet<ActVlanStaticTable> vlan_tables = deploy_path_data.GetVlanTables();
//   // qDebug() << "ActVlanStaticTables:";
//   // for (auto table : vlan_tables) {
//   //   qDebug() << table.ToString().toStdString().c_str();
//   // }
//   EXPECT_NE(vlan_tables.size(), 0) << "Compare table failed.";

//   QJsonArray vlan_array =
//       ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("unicast_untag_generate_vlan_2.2.json")).array();
//   ActVlanStaticTable answer_vlan_table;
//   for (auto vlan_j : vlan_array) {  // for each answer table
//     answer_vlan_table.fromJson(vlan_j);
//     QSet<ActVlanStaticTable>::iterator vlan_table_iter = vlan_tables.find(answer_vlan_table);
//     ASSERT_NE(vlan_table_iter, vlan_tables.end()) << "Table not found. DeviceID:" << answer_vlan_table.GetDeviceId();
//     EXPECT_EQ(vlan_table_iter->ToString(), answer_vlan_table.ToString())
//         << "Compare table failed. Failed table:" << vlan_table_iter->ToString().toStdString().c_str();
//   }

//   QSet<ActStadConfigTable> stad_config_tables = deploy_path_data.GetStadConfigTables();
//   EXPECT_EQ(stad_config_tables.size(), 0) << "Compare table failed.";
// }

// // // [feat:529] untag - hybrid none capable
// TEST_F(ActDeployUnicastTestUntag, GenerateData_Untag) {
// deploy_path_data = act::deploy::ActDeployPathData(deploy_ctrl, act_project, dev_profiles);
// deploy_path_data.GenerateData();
//   QSet<ActVlanStaticTable> vlan_tables = deploy_path_data.GetVlanTables();
//   // qDebug() << "ActVlanStaticTables:";
//   // for (auto table : vlan_tables) {
//   //   qDebug() << table.ToString().toStdString().c_str();
//   // }
//   EXPECT_NE(vlan_tables.size(), 0) << "Compare ActVlanStaticTable failed.";

//   QJsonArray vlan_array =
//       ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("unicast_untag_generate_vlan.json")).array();
//   ActVlanStaticTable answer_vlan_table;
//   for (auto vlan_j : vlan_array) {  // for each answer table
//     answer_vlan_table.fromJson(vlan_j);
//     QSet<ActVlanStaticTable>::iterator vlan_table_iter = vlan_tables.find(answer_vlan_table);
//     ASSERT_NE(vlan_table_iter, vlan_tables.end()) << "Table not found. DeviceID:" << answer_vlan_table.GetDeviceId();
//     EXPECT_EQ(vlan_table_iter->ToString(), answer_vlan_table.ToString())
//         << "Compare table failed. Failed table:" << vlan_table_iter->ToString().toStdString().c_str();
//   }

//   QSet<ActStadConfigTable> stad_config_tables = deploy_path_data.GetStadConfigTables();
//   EXPECT_NE(vlan_tables.size(), 0) << "Compare ActStadConfigTable failed.";

//   QJsonArray stad_config_array =
//       ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("unicast_untag_generate_stad_config.json")).array();
//   ActStadConfigTable answer_stad_config_table;
//   for (auto stad_config_j : stad_config_array) {  // for each answer table
//     answer_stad_config_table.fromJson(stad_config_j);
//     QSet<ActStadConfigTable>::iterator stad_config_table_iter = stad_config_tables.find(answer_stad_config_table);
//     ASSERT_NE(stad_config_table_iter, stad_config_tables.end())
//         << "Table not found. DeviceID:" << answer_stad_config_table.GetDeviceId();
//     EXPECT_EQ(stad_config_table_iter->ToString(), answer_stad_config_table.ToString())
//         << "Compare table failed. Failed table:" << stad_config_table_iter->ToString().toStdString().c_str();
//   }
//   // qDebug() << "ActStadConfigTable:";
//   // for (auto table : stad_config_tables) {
//   //   qDebug() << table.ToString().toStdString().c_str();
//   // }
// }

// TEST_F(ActDeployMulticastInactiveStadTest, GenerateData_StreamPriorityResult) {
// deploy_path_data = act::deploy::ActDeployPathData(deploy_ctrl, act_project, dev_profiles);
// deploy_path_data.GenerateData();

//   // StreamPriority Port (Ingress)
//   QSet<ActStadPortTable> stad_port_tables = deploy_path_data.GetStadPortTables();
//   ASSERT_EQ(stad_port_tables.isEmpty(), true) << "StadPortTables doesn't empty.";

//   // StreamPriority Config (Egress)
//   QSet<ActStadConfigTable> stad_config_tables = deploy_path_data.GetStadConfigTables();
//   QJsonArray stad_config_array =
//       ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("multicast_generate_inactive_stad_config.json")).array();
//   ActStadConfigTable answer_stad_config_table;
//   for (auto stad_config_j : stad_config_array) {  // for each answer table
//     answer_stad_config_table.fromJson(stad_config_j);
//     QSet<ActStadConfigTable>::iterator stad_config_table_iter = stad_config_tables.find(answer_stad_config_table);
//     ASSERT_NE(stad_config_table_iter, stad_config_tables.end())
//         << "Table not found. DeviceID:" << answer_stad_config_table.GetDeviceId();
//     EXPECT_EQ(stad_config_table_iter->ToString(), answer_stad_config_table.ToString())
//         << "Compare table failed. Failed table:" << stad_config_table_iter->ToString().toStdString().c_str();
//   }
// }

// TEST_F(ActDeployUnicastTest, GenerateData_UnicastStaticForwardResult) {
//   QSet<ActVlanStaticTable> vlan_tables = deploy_path_data.GetVlanTables();
//   QSet<ActStaticForwardTable> static_forward_tables = deploy_path_data.GetUnicastStaticForwardTables();
//   QJsonArray static_forward_array =
//       ReadJsonFile(QString("%1/%2").arg(FAKE_FOLDER).arg("unicast_generate_static_forward.json")).array();
//   ActStaticForwardTable answer_static_forward_table;
//   for (auto static_forward_j : static_forward_array) {  // for each answer table
//     answer_static_forward_table.fromJson(static_forward_j);
//     QSet<ActStaticForwardTable>::iterator static_forward_table_iter =
//         static_forward_tables.find(answer_static_forward_table);
//     ASSERT_NE(static_forward_table_iter, static_forward_tables.end())
//         << "Table not found. DeviceID:" << answer_static_forward_table.GetDeviceId();
//     EXPECT_EQ(static_forward_table_iter->ToString(), answer_static_forward_table.ToString())
//         << "Compare table failed. Failed table:" << static_forward_table_iter->ToString().toStdString().c_str();
//   }
// }

// TEST_F(ActDeployUnicastTestUntag22, DeployerThread) {
//   act_status = ActGrpcServerProcess::StartGrpcServer();  // Start grpc server
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";

//   act_status = deploy.Start(act_project, dev_profiles, deploy_ctrl);
//   qDebug() << "Start act_status::" << act_status->ToString().toStdString().c_str();

//   qint8 times = 0;
//   while (IsActStatusRunning(act_status) && (times < 30)) {
//     times++;
//     act_status = deploy.GetStatus();
//     qDebug() << "act_status::" << act_status->ToString().toStdString().c_str();
//   }
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "Deployer failed";

//   act_status = ActGrpcServerProcess::StopGrpcServer();
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Close GRPC server failed";
// }

TEST_F(ActDeployMulticastTest22, DeployerThread) {
  act_status = ActGrpcServerProcess::StartGrpcServer();  // Start grpc server
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";

  // qDebug() << __func__ << act_project.ToString().toStdString().c_str();

  act_status = deploy.Start(act_project, dev_profiles, deploy_ctrl);
  qDebug() << "Start act_status::" << act_status->ToString().toStdString().c_str();

  qint8 times = 0;
  while (IsActStatusRunning(act_status) && (times < 30)) {
    times++;

    SLEEP_MS(2000);

    act_status = deploy.GetStatus();
    qDebug() << "act_status::" << act_status->ToString().toStdString().c_str();
  }
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "Deployer failed";

  act_status = ActGrpcServerProcess::StopGrpcServer();
  EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Close GRPC server failed";
}

// TEST_F(ActDeployMulticastTest, DeployerThreadStop) {
//   act_status = ActGrpcServerProcess::StartGrpcServer();  // Start grpc server
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";

//   act_status = deploy.Start(act_project, dev_profiles, deploy_ctrl);
//   qDebug() << "Start act_status::" << act_status->ToString().toStdString().c_str();
//   sleep(2);

//   act_status = deploy.Stop();
//   qDebug() << "Stop act_status::" << act_status->ToString().toStdString().c_str();
//   EXPECT_NE(act_status->GetStatus(), ActStatusType::kRunning) << "Deployer stop failed";

//   act_status = ActGrpcServerProcess::StopGrpcServer();
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Close GRPC server failed";
// }

// TEST_F(ActDeployMulticastTest, DeployerThread) {
//   act_status = ActGrpcServerProcess::StartGrpcServer();  // Start grpc server
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Start GRPC server failed";

//   act_status = deploy.Start(act_project, dev_profiles, deploy_ctrl);
//   qDebug() << "Start act_status::" << act_status->ToString().toStdString().c_str();

//   qint8 times = 0;
//   while (IsActStatusRunning(act_status) && (times < 30)) {
//     times++;
//     sleep(2);
//     act_status = deploy.GetStatus();
//     qDebug() << "act_status::" << act_status->ToString().toStdString().c_str();
//   }
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kFinished) << "Deployer failed";

//   act_status = ActGrpcServerProcess::StopGrpcServer();
//   EXPECT_EQ(act_status->GetStatus(), ActStatusType::kSuccess) << "Close GRPC server failed";
// }

}  // namespace deploy
}  // namespace act
