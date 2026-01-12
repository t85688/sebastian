#include "act_json.hpp"
#include "act_unit_test.hpp"

enum class TestEnum { kEnum1, kEnum2, kEnum3 };

static const QMap<QString, TestEnum> kTestEnumMap = {
    {"Enum 1", TestEnum::kEnum1}, {"Enum 2", TestEnum::kEnum2}, {"Enum 3", TestEnum::kEnum3}};
// Declare the qHash() function for QSet
inline uint qHash(TestEnum key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

class DummyObject : public QSerializer {
  // We don't need to call toJson & fromJson here
  // Q_GADGET
  // QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, quint8_field, QUint8Field);

 public:
  DummyObject() {}
  DummyObject(quint8 val) : quint8_field_(val) {}

  /**
   * @brief For QHash, QSet
   *
   * @param other
   * @return true
   * @return false
   */
  inline friend bool operator==(const DummyObject &x, const DummyObject &y) {
    return (x.quint8_field_ == y.quint8_field_);
  }

  /**
   * @brief For QMap
   *
   * @param other
   * @return true
   * @return false
   */
  inline bool operator<(const DummyObject &other) const { return (quint8_field_ < other.quint8_field_); }

  /**
   * @brief For QSet
   *
   * @param x
   * @return uint
   */
  inline friend uint qHash(const DummyObject &x) {
    return qHash(x.quint8_field_, 0xa03f);  // arbitrary value
  }
};

class DummyClass : public QSerializer {
  // We don't need to call toJson & fromJson here
  // Q_GADGET
  // QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(QDate, date_field, DateField);
  ACT_JSON_FIELD(quint16, quint16_field, QUint16Field);
  ACT_JSON_FIELD(qint16, qint16_field, QInt16Field);
  ACT_JSON_FIELD(bool, bool_field, BoolField);
  ACT_JSON_FIELD(QString, string_field, StringField);

  ACT_JSON_COLLECTION(QList, quint8, quint8_list, QUint8List);
  ACT_JSON_COLLECTION_OBJECTS(QList, DummyObject, object_list, ObjectList);
  ACT_JSON_QT_DICT(QHash, quint8, QString, qt_hash, QtHash);
  // ACT_JSON_QT_DICT_SET(QMap, QString, QSet<qint8>, test_qamp_set, TestQMapSet);
  ACT_JSON_QT_DICT_SET(QString, QSet<qint8>, test_qamp_set, TestQMapSet);
  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, DummyObject, object_qt_map, ObjectQtMap);
  ACT_JSON_STL_DICT(std::map, quint8, QString, std_map, StdMap);
  ACT_JSON_STL_DICT_OBJECTS(std::map, QString, DummyObject, object_std_map, ObjectStdMap);
  ACT_JSON_QT_SET(qint8, qt_set, QtSet);
  ACT_JSON_QT_SET_OBJECTS(DummyObject, qt_set_objs, QtObjectSet);
  ACT_JSON_STL_SET(qint8, std_set, StdSet);
  ACT_JSON_STL_SET_OBJECTS(DummyObject, std_set_objs, StdObjectSet);

  ACT_JSON_ENUM(TestEnum, my_test_enum, TestEnum);
  ACT_JSON_COLLECTION_ENUM(QList, TestEnum, test_enum_list, TestEnumList);
  ACT_JSON_QT_DICT_ENUM(QHash, QString, TestEnum, test_enum_qt_dict, TestEnumQtDict);
  ACT_JSON_STL_DICT_ENUM(std::map, QString, TestEnum, test_enum_stl_dict, TestEnumSTLDict);
  ACT_JSON_QT_SET_ENUM(TestEnum, qt_enum_set, QtEnumSet);
  ACT_JSON_STL_SET_ENUM(TestEnum, stl_enum_set, STLEnumSet);
};

// We derive a fixture named IntegerFunctionTest from the ActQuickTest
// fixture.  All tests using this fixture will be automatically
// required to be quick.
class ActJsonTest : public ActQuickTest {
 protected:  // You should make the members protected s.t. they can be
             // accessed from sub-classes.
  // virtual void SetUp() will be called before each test is run.  You
  // should define it if you need to initialize the variables.
  // Otherwise, this can be skipped.
  void SetUp() override {}

  // virtual void TearDown() will be called after each test is run.
  // You should define it if there is cleanup work to do.  Otherwise,
  // you don't have to provide it.
  void TearDown() override {}

  // Declares the variables your tests want to use.
  DummyClass dummy;
};

// When you have a test fixture, you define a test using TEST_F
// instead of TEST.

// Demonstrate some basic assertions.
TEST_F(ActJsonTest, TestActJsonField) {
  dummy.SetQInt16Field(-1234);
  EXPECT_EQ(-1234, dummy.GetQInt16Field());

  dummy.SetQUint16Field(9527u);
  EXPECT_EQ(9527u, dummy.GetQUint16Field());

  dummy.SetDateField(QDate::fromString("2021/12/25", "yyyy/MM/dd"));
  EXPECT_STREQ("2021/12/25", dummy.GetDateField().toString("yyyy/MM/dd").toStdString().c_str());

  dummy.SetBoolField(false);
  EXPECT_EQ(false, dummy.GetBoolField());

  dummy.SetStringField("Moxa");
  EXPECT_EQ(4, dummy.GetStringField().size());
  EXPECT_STREQ("Moxa", dummy.GetStringField().toStdString().c_str());
}

TEST_F(ActJsonTest, TestActJsonCollection) {
  QList<quint8> my_test_list;
  my_test_list.push_back(1);
  my_test_list.push_back(2);
  my_test_list.push_back(3);
  my_test_list.push_back(4);
  my_test_list.push_back(5);
  dummy.SetQUint8List(my_test_list);

  EXPECT_EQ(5, dummy.GetQUint8List().size());
  EXPECT_EQ(1, dummy.GetQUint8List().first());
  EXPECT_EQ(5, dummy.GetQUint8List().last());
}

TEST_F(ActJsonTest, TestActJsonCollectionObjects) {
  QList<DummyObject> my_test_obj_list;
  DummyObject o1(1);
  DummyObject o2(2);
  DummyObject o3(3);
  my_test_obj_list.push_back(o1);
  my_test_obj_list.push_back(o2);
  my_test_obj_list.push_back(o3);
  dummy.SetObjectList(my_test_obj_list);

  EXPECT_EQ(3, dummy.GetObjectList().size());
  EXPECT_EQ(1, dummy.GetObjectList().first().GetQUint8Field());
  EXPECT_EQ(3, dummy.GetObjectList().last().GetQUint8Field());
}

TEST_F(ActJsonTest, TestActJsonQtDict) {
  QHash<quint8, QString> my_test_qt_dict;
  my_test_qt_dict.insert(10, "abc");
  my_test_qt_dict.insert(20, "def");
  my_test_qt_dict.insert(30, "xyz");

  dummy.SetQtHash(my_test_qt_dict);

  EXPECT_EQ(3, dummy.GetQtHash().size());
  EXPECT_STREQ("abc", dummy.GetQtHash().value(10).toStdString().c_str());
  EXPECT_STREQ("def", dummy.GetQtHash().value(20).toStdString().c_str());
  EXPECT_STREQ("xyz", dummy.GetQtHash().value(30).toStdString().c_str());
}

// TEST_F(ActJsonTest, TestActJsonQtDictSet) {
//   QMap<QString, QSet<qint8>> my_test_qt_dict_set;
//   QSet<qint8> s1;
//   s1.insert(10);
//   s1.insert(20);
//   s1.insert(30);
//   my_test_qt_dict_set.insert("abc", s1);
//   QSet<qint8> s2;
//   s2.insert(10);
//   s2.insert(20);
//   s2.insert(30);
//   s2.insert(30);
//   my_test_qt_dict_set.insert("def", s2);

//   dummy.SetTestQMapSet(my_test_qt_dict_set);

//   EXPECT_EQ(2, dummy.GetTestQMapSet().size());
//   EXPECT_EQ(3, dummy.GetTestQMapSet().value("abc").size());
//   EXPECT_EQ(3, dummy.GetTestQMapSet().value("def").size());
// }

TEST_F(ActJsonTest, TestActJsonQtDictObjects) {
  QMap<QString, DummyObject> my_test_qt_map;
  DummyObject o1(1);
  DummyObject o2(2);
  DummyObject o3(3);
  my_test_qt_map.insert("abc", o1);
  my_test_qt_map.insert("def", o2);
  my_test_qt_map.insert("xyz", o3);

  dummy.SetObjectQtMap(my_test_qt_map);

  EXPECT_EQ(3, dummy.GetObjectQtMap().size());
  EXPECT_EQ(1, dummy.GetObjectQtMap().value("abc"));
  EXPECT_EQ(2, dummy.GetObjectQtMap().value("def"));
  EXPECT_EQ(3, dummy.GetObjectQtMap().value("xyz"));
}

TEST_F(ActJsonTest, TestActJsonSTLDict) {
  std::map<quint8, QString> my_test_std_map;
  my_test_std_map.insert({10, "abc"});
  my_test_std_map.insert({20, "def"});
  my_test_std_map.insert({30, "xyz"});

  dummy.SetStdMap(my_test_std_map);

  std::map<quint8, QString> x = dummy.GetStdMap();
  EXPECT_EQ(3, x.size());
  EXPECT_STREQ("abc", x[10].toStdString().c_str());
  EXPECT_STREQ("def", x[20].toStdString().c_str());
  EXPECT_STREQ("xyz", x[30].toStdString().c_str());
}

TEST_F(ActJsonTest, TestActJsonSTLDictObjects) {
  std::map<QString, DummyObject> my_test_std_map;
  DummyObject o1(1);
  DummyObject o2(2);
  DummyObject o3(3);
  my_test_std_map.insert({"abc", o1});
  my_test_std_map.insert({"def", o2});
  my_test_std_map.insert({"xyz", o3});

  dummy.SetObjectStdMap(my_test_std_map);

  std::map<QString, DummyObject> x = dummy.GetObjectStdMap();
  EXPECT_EQ(3, x.size());
  EXPECT_EQ(1, x["abc"]);
  EXPECT_EQ(2, x["def"]);
  EXPECT_EQ(3, x["xyz"]);
}

TEST_F(ActJsonTest, TestActJsonQtSet) {
  QSet<qint8> qt_set;
  qt_set.insert(1);
  qt_set.insert(1);
  qt_set.insert(2);
  qt_set.insert(3);

  dummy.SetQtSet(qt_set);
  EXPECT_EQ(3, dummy.GetQtSet().size());
}

TEST_F(ActJsonTest, TestActJsonQtObjectSet) {
  QSet<DummyObject> qt_set;
  DummyObject o1(1);
  DummyObject o2(2);
  DummyObject o3(3);
  DummyObject o4(3);
  qt_set.insert(o1);
  qt_set.insert(o2);
  qt_set.insert(o3);
  qt_set.insert(o4);

  dummy.SetQtObjectSet(qt_set);
  EXPECT_EQ(3, dummy.GetQtObjectSet().size());
}

TEST_F(ActJsonTest, TestActJsonStdSet) {
  std::set<qint8> std_set;
  std_set.insert(1);
  std_set.insert(1);
  std_set.insert(2);
  std_set.insert(3);

  dummy.SetStdSet(std_set);
  EXPECT_EQ(3, dummy.GetStdSet().size());
}

TEST_F(ActJsonTest, TestActJsonStdObjectSet) {
  std::set<DummyObject> std_set;
  DummyObject o1(1);
  DummyObject o2(2);
  DummyObject o3(3);
  DummyObject o4(3);
  std_set.insert(o1);
  std_set.insert(o2);
  std_set.insert(o3);
  std_set.insert(o4);

  dummy.SetStdObjectSet(std_set);
  EXPECT_EQ(3, dummy.GetStdObjectSet().size());
}

TEST_F(ActJsonTest, TestEnum) {
  dummy.SetTestEnum(TestEnum::kEnum2);
  EXPECT_EQ(TestEnum::kEnum2, dummy.GetTestEnum());
  EXPECT_STREQ("Enum 2", GetStringFromEnum<TestEnum>(dummy.GetTestEnum(), kTestEnumMap).toStdString().c_str());
  EXPECT_EQ(TestEnum::kEnum2, GetEnumFromString<TestEnum>("Enum 2", kTestEnumMap));
}

TEST_F(ActJsonTest, TestActJsonEnumList) {
  QList<TestEnum> test_enum_list;
  test_enum_list.push_back(TestEnum::kEnum1);
  test_enum_list.push_back(TestEnum::kEnum1);
  test_enum_list.push_back(TestEnum::kEnum1);
  test_enum_list.push_back(TestEnum::kEnum2);
  test_enum_list.push_back(TestEnum::kEnum3);
  dummy.SetTestEnumList(test_enum_list);

  EXPECT_EQ(5, dummy.GetTestEnumList().size());
  EXPECT_EQ(TestEnum::kEnum1, dummy.GetTestEnumList().first());
  EXPECT_EQ(TestEnum::kEnum3, dummy.GetTestEnumList().last());
}

TEST_F(ActJsonTest, TestActJsonEnumQtDict) {
  QHash<QString, TestEnum> test_enum_qt_dict;
  test_enum_qt_dict.insert("abc", TestEnum::kEnum1);
  test_enum_qt_dict.insert("def", TestEnum::kEnum2);
  test_enum_qt_dict.insert("xyz", TestEnum::kEnum3);
  dummy.SetTestEnumQtDict(test_enum_qt_dict);

  EXPECT_EQ(3, dummy.GetTestEnumQtDict().size());
  EXPECT_EQ(TestEnum::kEnum1, dummy.GetTestEnumQtDict().value("abc"));
  EXPECT_EQ(TestEnum::kEnum2, dummy.GetTestEnumQtDict().value("def"));
  EXPECT_EQ(TestEnum::kEnum3, dummy.GetTestEnumQtDict().value("xyz"));
}

TEST_F(ActJsonTest, TestActJsonEnumSTLDict) {
  std::map<QString, TestEnum> test_enum_stl_dict;
  test_enum_stl_dict.insert({"abc", TestEnum::kEnum1});
  test_enum_stl_dict.insert({"def", TestEnum::kEnum2});
  test_enum_stl_dict.insert({"xyz", TestEnum::kEnum3});
  dummy.SetTestEnumSTLDict(test_enum_stl_dict);

  std::map<QString, TestEnum> x = dummy.GetTestEnumSTLDict();
  EXPECT_EQ(3, x.size());
  EXPECT_EQ(TestEnum::kEnum1, x["abc"]);
  EXPECT_EQ(TestEnum::kEnum2, x["def"]);
  EXPECT_EQ(TestEnum::kEnum3, x["xyz"]);
}

TEST_F(ActJsonTest, TestActJsonQtEnumSet) {
  QSet<TestEnum> qt_enum_set;
  qt_enum_set.insert(TestEnum::kEnum1);
  qt_enum_set.insert(TestEnum::kEnum1);
  qt_enum_set.insert(TestEnum::kEnum2);
  qt_enum_set.insert(TestEnum::kEnum3);
  dummy.SetQtEnumSet(qt_enum_set);

  EXPECT_EQ(3, dummy.GetQtEnumSet().size());
}

TEST_F(ActJsonTest, TestActJsonSTLEnumSet) {
  std::set<TestEnum> stl_enum_set;
  stl_enum_set.insert(TestEnum::kEnum1);
  stl_enum_set.insert(TestEnum::kEnum1);
  stl_enum_set.insert(TestEnum::kEnum2);
  stl_enum_set.insert(TestEnum::kEnum3);
  dummy.SetSTLEnumSet(stl_enum_set);

  EXPECT_EQ(3, dummy.GetSTLEnumSet().size());
}
