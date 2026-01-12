// #include "act_algorithm_structure.hpp"
// #include "act_algorithm_flow.h"
#include "act_project.hpp"
// #include "act_shortest_path.hpp"
#include "act_algorithm.hpp"
#include "act_status.hpp"
#include "act_unit_test.hpp"
#include "gcl/act_gcl_result.hpp"
#include "json/json_utils.hpp"
#include "routing/act_routing_result.hpp"

// class DoShortestPathRoutingForStreamTest : public ActQuickTest {
//  protected:
//   QMap<qint64, QList<ActRoutingLink>> links_from_nodes;

//   qint64 source_node = 0;
//   qint64 destination_node = 4;
//   qint64 stream_id = 101;
//   ActProject act_project;
//   ACT_STATUS_INIT();

//   void SetUp() override {
//     act_project.SetProjectName("Test project");
//     QList<ActRoutingLink> links = {ActRoutingLink(1, 0, 6.0),
//                                                   ActRoutingLink(2, 0, 3.0),
//                                                   ActRoutingLink(3, 0, 5.0)};
//     links_from_nodes.insert(0, links);
//     links = {ActRoutingLink(0, 0, 6.0), ActRoutingLink(4, 0, 4.0)};
//     links_from_nodes.insert(1, links);
//     links = {ActRoutingLink(0, 0, 3.0), ActRoutingLink(3, 0, 1.0),
//              ActRoutingLink(4, 0, 7.0), ActRoutingLink(5, 0, 2.0)};
//     links_from_nodes.insert(2, links);
//     links = {ActRoutingLink(0, 0, 5.0), ActRoutingLink(2, 0, 1.0)};
//     links_from_nodes.insert(3, links);
//     links = {ActRoutingLink(1, 0, 4.0), ActRoutingLink(2, 0, 7.0),
//              ActRoutingLink(5, 0, 1.0)};
//     links_from_nodes.insert(4, links);
//     links = {ActRoutingLink(2, 0, 2.0), ActRoutingLink(4, 0, 1.0)};
//     links_from_nodes.insert(5, links);
//     // print the added content
//     // qDebug() << "topology:";
//     // for (QString link_src_node : links_from_nodes.keys()) {
//     //   for (ActRoutingLink link : links_from_nodes.value(link_src_node)) {
//     //     qDebug() << link_src_node << "to" << link.GetDestinationNode() << "distance =" << link.GetDistance();
//     //   }
//     // }
//   }

//   void TearDown() override {}
// };

// TEST_F(DoShortestPathRoutingForStreamTest, InvalidSourceNode) {
//   source_node = 100;
//   QList<qint64> list;
//   ActHostPair host_pair(source_node, destination_node);

//   act_status =
//       DoShortestPathRoutingForStream(list, links_from_nodes, host_pair, stream_id, act_project);
//   ActStatusInternalError internal_error = dynamic_cast<ActStatusInternalError &>(*act_status);
//   EXPECT_EQ(internal_error.GetStatus(), ActStatusType::kInternalError);
//   EXPECT_EQ(internal_error.GetSeverity(), ActSeverity::kCritical);
//   EXPECT_EQ(internal_error.GetModule(), "Routing");
// }

// TEST_F(DoShortestPathRoutingForStreamTest, InvalidDestinationNode) {
//   destination_node = 100;
//   QList<qint64> list;
//   ActHostPair host_pair(source_node, destination_node);
//   act_status =
//       DoShortestPathRoutingForStream(list, links_from_nodes, host_pair, stream_id, act_project);
//   ActStatusInternalError internal_error = dynamic_cast<ActStatusInternalError &>(*act_status);
//   EXPECT_EQ(internal_error.GetStatus(), ActStatusType::kInternalError);
//   EXPECT_EQ(internal_error.GetSeverity(), ActSeverity::kCritical);
//   EXPECT_EQ(internal_error.GetModule(), "Routing");
// }

// TEST_F(DoShortestPathRoutingForStreamTest, NoHop) {
//   destination_node = 0;
//   QList<qint64> list;
//   ActHostPair host_pair(source_node, destination_node);

//   act_status =
//       DoShortestPathRoutingForStream(list, links_from_nodes, host_pair, stream_id, act_project);
//   ASSERT_EQ(list.size(), 1);
//   EXPECT_EQ(list[0], 0);
// }

// TEST_F(DoShortestPathRoutingForStreamTest, OneHop) {
//   destination_node = 1;
//   QList<qint64> list;
//   ActHostPair host_pair(source_node, destination_node);
//   ActHostPair host_pair(source_node, destination_node);

//   act_status =
//       DoShortestPathRoutingForStream(list, links_from_nodes, host_pair, stream_id, act_project);
//   ASSERT_EQ(list.size(), 2);
//   EXPECT_EQ(list[0], 0);
//   EXPECT_EQ(list[1], 1);
// }

// TEST_F(DoShortestPathRoutingForStreamTest, GeneralCases) {
//   QList<qint64> list;
//   ActHostPair host_pair(source_node, destination_node);
//   act_status =
//       DoShortestPathRoutingForStream(list, links_from_nodes, host_pair, stream_id, act_project);
//   ASSERT_EQ(list.size(), 4);
//   EXPECT_EQ(list[0], 0);
//   EXPECT_EQ(list[1], 2);
//   EXPECT_EQ(list[2], 5);
//   EXPECT_EQ(list[3], 4);

//   source_node = 5;
//   destination_node = 0;
//   list.clear();
//   act_status =
//       DoShortestPathRoutingForStream(list, links_from_nodes, host_pair, stream_id, act_project);
//   ASSERT_EQ(list.size(), 3);
//   EXPECT_EQ(list[0], 5);
//   EXPECT_EQ(list[1], 2);
//   EXPECT_EQ(list[2], 0);
// }

// TEST_F(DoShortestPathRoutingForStreamTest, DestinationUnreachable) {
//   QList<ActRoutingLink> links = {};
//   links_from_nodes.insert(6, links);
//   destination_node = 6;
//   QList<qint64> list;
//   ActHostPair host_pair(source_node, destination_node);
//   act_status =
//       DoShortestPathRoutingForStream(list, links_from_nodes, host_pair, stream_id, act_project);
//   ActStatusRoutingDestinationUnreachable unreachable =
//       dynamic_cast<ActStatusRoutingDestinationUnreachable &>(*act_status);

//   EXPECT_EQ(unreachable.GetStatus(), ActStatusType::kRoutingDestinationUnreachable);
//   EXPECT_EQ(unreachable.GetSeverity(), ActSeverity::kWarning);
//   EXPECT_EQ(unreachable.GetDestinationIp(), "-1.-1.-1.-1");
//   EXPECT_EQ(unreachable.GetStreamName(), "StreamNameNotFound");
// }

// class DataStructureHasNoEntryInternalErrorTest : public DoShortestPathRoutingForStreamTest {
//  protected:
//   qint64 target_node = 0;
//   QString node_description = "candidate node";
//   QList<qint64> device_list;
//   QString target_container = "record board";

//   void SetUp() override {
//     DoShortestPathRoutingForStreamTest::SetUp();
//     device_list = links_from_nodes.keys();
//   }

//   void TearDown() override {}
// };

// TEST_F(DataStructureHasNoEntryInternalErrorTest, ErrorInRecordBoard) {
//   act_status = DataStructureHasNoEntryInternalError(target_node, node_description, stream_id,
//                                                                     device_list, target_container);

//   // ActStatusBase* act_status_base = act_status.release();
//   // ActStatusInternalError* internal_error = dynamic_cast<ActStatusInternalError*>(act_status_base);
//   ActStatusInternalError internal_error = dynamic_cast<ActStatusInternalError &>(*act_status);
//   EXPECT_EQ(internal_error.GetStatus(), ActStatusType::kInternalError);
//   EXPECT_EQ(internal_error.GetSeverity(), ActSeverity::kCritical);
//   EXPECT_EQ(internal_error.GetModule(), "Routing");
// }

// class ShortestPathAndClassBasedSchedulingTest : public ActQuickTest {
//  protected:
//   ActProject act_project;
//   ACT_STATUS_INIT();

//   void SetUp() override {
//     QString file_path = "act_algorithm_test_projects/line_topology.json";

//     QJsonDocument json_doc = ReadJsonFile(file_path);
//     act_project.fromJson(json_doc.object());
//   }

//   void TearDown() override {}
// };

// TEST_F(ShortestPathAndClassBasedSchedulingTest, SimpleTest) {
//   ActComputedResult computed_result;
//   quint8 progress = 0;
//   act_status = ShortestPathAndClassBasedScheduling(computed_result, act_project, progress);
//   if (!IsActStatusSuccess(act_status)) {
//     qDebug() << "act_status:" << act_status->ToString().toStdString().c_str();
//   } else {
//     qDebug() << "routing_results:";
//     for (const ActRoutingResult &routing_result : computed_result.GetRoutingResults()) {
//       qDebug() << routing_result.ToString().toStdString().c_str();
//     }
//     qDebug() << "gcl_results:";
//     for (const ActGclResult &gcl_result : computed_result.GetGclResults()) {
//       qDebug() << gcl_result.ToString().toStdString().c_str();
//     }
//   }
// }

class AssignDeviceDistanceByShortestPathTest : public ActQuickTest {
 protected:
  QList<ActDevice> dev_list;
  QList<ActLink> link_list;

  ACT_STATUS_INIT();

  void SetUp() override {}
  void TearDown() override {}
};