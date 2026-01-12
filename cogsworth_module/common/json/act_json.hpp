/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

// kene+
/*
#include "qserializer.hpp"
*/
// kene-
#include "qserializer.h"

/**
 * @brief Create JSON property and getter/setter for enumerations field
 *
 * Example:
 * Step1: Declare the enum class first
 *    enum class TestEnum { kEnum1, kEnum2, kEnum3 };
 *
 * Step2: Declare the string mapping map
 *    static const QMap<QString, TestEnum> kTestEnumMap = {
 *      {"Enum 1", TestEnum::kEnum1},
 *      {"Enum 2", TestEnum::kEnum2},
 *      {"Enum 3", TestEnum::kEnum3}
 *    };
 *
 * Step3: Declare the macro in the user defined class
 *    class XXX : public QSerializer {
 *      ACT_JSON_ENUM(TestEnum, my_test_enum, MyTestEnum);
 *    }
 *
 * Step4: Use the enum type in the source code
 *    XXX x;
 *    x.SetMyTestEnum(MyTestEnum::kEnum1);
 *
 * Step5: Check the output in the JSON of the XXX class
 *    {
 *      "MyTestEnum": "Enum 1"
 *    }
 */
#define ACT_JSON_ENUM(type, name, key)                             \
 public:                                                           \
  inline void Set##key(const type &name) { this->name##_ = name; } \
  inline const type &Get##key() const { return this->name##_; }    \
  inline type &Get##key() { return this->name##_; }                \
  QS_JSON_ENUM(type, name##_, key)                                 \
                                                                   \
 private:                                                          \
  type name##_;

/**
 * @brief Make collection of enum type objects [collectionType<itemType> name] and generate serializable properties
 * for this collection
 *
 * This collection must be provide method append(T) (it's can be QList, QVector)
 *
 * Example:
 * Step1: Declare the enum class first
 *    enum class TestEnum { kEnum1, kEnum2, kEnum3 };
 *
 * Step2: Declare the string mapping map
 *    static const QMap<QString, TestEnum> kTestEnumMap = {
 *      {"Enum 1", TestEnum::kEnum1},
 *      {"Enum 2", TestEnum::kEnum2},
 *      {"Enum 3", TestEnum::kEnum3}
 *    };
 *
 * Step3: Declare the macro in the user defined class
 *    class XXX : public QSerializer {
 *      ACT_JSON_COLLECTION_ENUM(QList, TestEnum, my_test_enum_list, MyTestEnumList);
 *    }
 *
 * Step4: Use the enum type in the source code *
 *    QList<TestEnum> my_test_enum_list;
 *    my_test_enum_list.push_back(TestEnum::kEnum1);
 *    my_test_enum_list.push_back(TestEnum::kEnum1);
 *    my_test_enum_list.push_back(TestEnum::kEnum2);
 *    my_test_enum_list.push_back(TestEnum::kEnum3);

 *    XXX x;
 *    x.SetMyTestEnumList(my_test_enum_list);
 *
 * Step5: Check the output in the JSON of the XXX class
 *    {
 *      "MyTestEnumList": [
 *          "Enum 1",
 *          "Enum 1",
 *          "Enum 2",
 *          "Enum 3"
 *      ]
 *    }
 */
#define ACT_JSON_COLLECTION_ENUM(collectionType, type, name, key)                  \
 public:                                                                           \
  inline void Set##key(const collectionType<type> &name) { this->name##_ = name; } \
  inline const collectionType<type> &Get##key() const { return this->name##_; }    \
  inline collectionType<type> &Get##key() { return this->name##_; }                \
  QS_JSON_ENUM_ARRAY(type, name##_, key)                                           \
                                                                                   \
 private:                                                                          \
  collectionType<type> name##_ = collectionType<type>();

/**
 * @brief Create JSON property and getter/setter for primitive type field
 *
 */
#define ACT_JSON_FIELD(type, name, key)                            \
 public:                                                           \
  inline void Set##key(const type &name) { this->name##_ = name; } \
  inline const type &Get##key() const { return this->name##_; }    \
  inline type &Get##key() { return this->name##_; }                \
  QS_JSON_FIELD(type, name##_, key)                                \
                                                                   \
 private:                                                          \
  type name##_;

/**
 * @brief Create JSON property and getter/setter for some custom class
 *
 * Custom type must be provide methods fromJson and toJson or inherit from QSerializer
 */
#define ACT_JSON_OBJECT(type, name, key)                           \
 public:                                                           \
  inline void Set##key(const type &name) { this->name##_ = name; } \
  inline const type &Get##key() const { return this->name##_; }    \
  inline type &Get##key() { return this->name##_; }                \
  QS_JSON_OBJECT(type, name##_, key)                               \
                                                                   \
 private:                                                          \
  type name##_;

/**
 * @brief Make collection of primitive type objects [collectionType<itemType> name] and generate serializable properties
 * for this collection
 *
 * This collection must be provide method append(T) (it's can be QList, QVector)
 *
 * Example:
 *    ACT_JSON_COLLECTION(QList, QString, test_list_str, TestListStr);
 */
#define ACT_JSON_COLLECTION(collectionType, type, name, key)                       \
 public:                                                                           \
  inline void Set##key(const collectionType<type> &name) { this->name##_ = name; } \
  inline const collectionType<type> &Get##key() const { return this->name##_; }    \
  inline collectionType<type> &Get##key() { return this->name##_; }                \
  QS_JSON_ARRAY(type, name##_, key)                                                \
                                                                                   \
 private:                                                                          \
  collectionType<type> name##_ = collectionType<type>();

/**
 * @brief Make collection of custom class objects [collectionType<itemType> name] and bind serializable properties
 *
 * This collection must be provide method append(T) (it's can be QList, QVector)
 *
 * Example:
 *    ACT_JSON_COLLECTION_OBJECTS(QList, ActModule, test_list_module, TestListModule);
 */
#define ACT_JSON_COLLECTION_OBJECTS(collectionType, type, name, key)               \
 public:                                                                           \
  inline void Set##key(const collectionType<type> &name) { this->name##_ = name; } \
  inline const collectionType<type> &Get##key() const { return this->name##_; }    \
  inline collectionType<type> &Get##key() { return this->name##_; }                \
  QS_JSON_ARRAY_OBJECTS(type, name##_, key)                                        \
                                                                                   \
 private:                                                                          \
  collectionType<type> name##_ = collectionType<type>();

/**
 * @brief Make dictionary collection of simple types [dictionary<key, itemType> name] and bind serializable
 properties
 *
 * This collection must be QT DICTIONARY TYPE
 *
 * Example:
 * Step1: Declare the enum class first
 *    enum class TestEnum { kEnum1, kEnum2, kEnum3 };
 *
 * Step2: Declare the string mapping map
 *    static const QMap<QString, TestEnum> kTestEnumMap = {
 *      {"Enum 1", TestEnum::kEnum1},
 *      {"Enum 2", TestEnum::kEnum2},
 *      {"Enum 3", TestEnum::kEnum3}
 *    };
 *
 * Step3: Declare the macro in the user defined class
 *    class XXX : public QSerializer {
 *      ACT_JSON_QT_DICT_ENUM(QHash, QString, TestEnum, my_test_enum_qt_dict, MyTestEnumQtDict);
 *    }
 *
 * Step4: Use the enum type in the source code *
 *    QHash<QString, TestEnum> my_test_enum_qt_dict;
 *    my_test_enum_qt_dict.insert("abc", TestEnum::kEnum1);
 *    my_test_enum_qt_dict.insert("def", TestEnum::kEnum2);
 *    my_test_enum_qt_dict.insert("xyz", TestEnum::kEnum3);

 *    XXX x;
 *    x.SetMyTestEnumQtDict(my_test_enum_qt_dict);
 *
 * Step5: Check the output in the JSON of the XXX class
 *    {
 *      "MyTestEnumQtDict": {
 *          "abc": "Enum 1",
 *          "def": "Enum 2",
 *          "xyz": "Enum 3"
 *      }
 *    }
 */
#define ACT_JSON_QT_DICT_ENUM(map, first, second, name, key)                  \
 public:                                                                      \
  typedef map<first, second> dict_##name##_t;                                 \
  inline void Set##key(const dict_##name##_t &name) { this->name##_ = name; } \
  inline const dict_##name##_t &Get##key() const { return this->name##_; }    \
  inline dict_##name##_t &Get##key() { return this->name##_; }                \
  QS_JSON_QT_DICT_ENUM(dict_##name##_t, second, name##_, key)                 \
                                                                              \
 private:                                                                     \
  dict_##name##_t name##_ = dict_##name##_t();

/**
 * @brief Make dictionary collection of simple types [dictionary<key, itemType> name] and bind serializable
 properties
 *
 * This collection must be QT DICTIONARY TYPE
 *
 * Example:
 *    ACT_JSON_QT_DICT(QHash, QString, QString, qt_hash, QtHash);
 */
#define ACT_JSON_QT_DICT(map, first, second, name, key)                       \
 public:                                                                      \
  typedef map<first, second> dict_##name##_t;                                 \
  inline void Set##key(const dict_##name##_t &name) { this->name##_ = name; } \
  inline const dict_##name##_t &Get##key() const { return this->name##_; }    \
  inline dict_##name##_t &Get##key() { return this->name##_; }                \
  QS_JSON_QT_DICT(dict_##name##_t, name##_, key)                              \
                                                                              \
 private:                                                                     \
  dict_##name##_t name##_ = dict_##name##_t();

/**
 * @brief Make dictionary collection of custom class objects [dictionary<key, itemType> name] and bind serializable
 * properties
 *
 * This collection must be QT DICTIONARY TYPE
 *
 * Example:
 *    ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActModule, qt_map_objects, QtMapObjects);
 */
#define ACT_JSON_QT_DICT_OBJECTS(map, first, second, name, key)               \
 public:                                                                      \
  typedef map<first, second> dict_##name##_t;                                 \
  inline void Set##key(const dict_##name##_t &name) { this->name##_ = name; } \
  inline const dict_##name##_t &Get##key() const { return this->name##_; }    \
  inline dict_##name##_t &Get##key() { return this->name##_; }                \
  QS_JSON_QT_DICT_OBJECTS(dict_##name##_t, name##_, key)                      \
                                                                              \
 private:                                                                     \
  dict_##name##_t name##_ = dict_##name##_t();

/**
 * @brief Make dictionary collection of set types [dictionary<key, QSet<itemType> name] and bind serializable
 properties
 *
 * This collection must be QT DICTIONARY TYPE
 *
 * Example:
 *    ACT_JSON_QT_DICT_SET(QMap, QString, QSet<qint64>, qt_map_set, QtMapSet);
 */
// #define ACT_JSON_QT_DICT_SET(map, first, second, name, key)                   \
//  public:                                                                      \
//   typedef map<first, second> dict_##name##_t;                                 \
//   inline void Set##key(const dict_##name##_t &name) { this->name##_ = name; } \
//   inline const dict_##name##_t &Get##key() const { return this->name##_; }    \
//   QS_JSON_QT_DICT_SET(dict_##name##_t, name##_, key)                          \
//                                                                               \
//  private:                                                                     \
//   dict_##name##_t name##_ = dict_##name##_t();
#define ACT_JSON_QT_DICT_SET(keyType, valueType, name, jsonkey)                            \
  Q_PROPERTY(QJsonValue jsonkey READ GET(json, name) WRITE SET(json, name))                \
 public:                                                                                   \
  inline void Set##jsonkey(const QMap<keyType, valueType> &name) { this->name##_ = name; } \
  inline const QMap<keyType, valueType> &Get##jsonkey() const { return this->name##_; }    \
  inline QMap<keyType, valueType> &Get##jsonkey() { return this->name##_; }                \
                                                                                           \
 private:                                                                                  \
  QMap<keyType, valueType> name##_;                                                        \
  QJsonValue GET(json, name)() const {                                                     \
    QJsonObject json;                                                                      \
    for (auto key : name##_.keys()) {                                                      \
      QJsonArray jsonArray;                                                                \
      for (auto item : name##_.value(key)) {                                               \
        jsonArray.push_back(item);                                                         \
      }                                                                                    \
      json.insert(QString::number(key), jsonArray);                                        \
    }                                                                                      \
    return json;                                                                           \
  }                                                                                        \
  void SET(json, name)(const QJsonValue &varname) {                                        \
    name##_.clear();                                                                       \
    QJsonObject json = varname.toObject();                                                 \
    QStringList keys = json.keys();                                                        \
    for (auto key : keys) {                                                                \
      QString keyStr = key;                                                                \
      bool ok;                                                                             \
      keyType k = static_cast<keyType>(keyStr.toInt(&ok));                                 \
      if (!ok) {                                                                           \
        continue;                                                                          \
      }                                                                                    \
      valueType v;                                                                         \
      QJsonArray jsonArray = json.value(key).toArray();                                    \
      for (auto item : jsonArray) {                                                        \
        v.insert(static_cast<keyType>(item.toInt()));                                      \
      }                                                                                    \
      name##_.insert(k, v);                                                                \
    }                                                                                      \
  }

/**
 * @brief Make dictionary collection of simple types [dictionary<key, itemType> name] and bind serializable properties
 *
 * This collection must be STL DICTIONARY TYPE
 *
 * Example:
 * Step1: Declare the enum class first
 *    enum class TestEnum { kEnum1, kEnum2, kEnum3 };
 *
 * Step2: Declare the string mapping map
 *    static const QMap<QString, TestEnum> kTestEnumMap = {
 *      {"Enum 1", TestEnum::kEnum1},
 *      {"Enum 2", TestEnum::kEnum2},
 *      {"Enum 3", TestEnum::kEnum3}
 *    };
 *
 * Step3: Declare the macro in the user defined class
 *    class XXX : public QSerializer {
 *      ACT_JSON_STL_DICT_ENUM(std::map, QString, TestEnum, my_test_enum_stl_dict, MyTestEnumSTLDict);
 *    }
 *
 * Step4: Use the enum type in the source code *
 *    std::map<QString, TestEnum> my_test_enum_stl_dict;
 *    my_test_enum_stl_dict.insert({"abc", TestEnum::kEnum1});
 *    my_test_enum_stl_dict.insert({"def", TestEnum::kEnum2});
 *    my_test_enum_stl_dict.insert({"xyz", TestEnum::kEnum3});
 *
 *    XXX x;
 *    x.SetMyTestEnumSTLDict(my_test_enum_stl_dict);
 *
 * Step5: Check the output in the JSON of the XXX class
 *    {
 *      "MyTestEnumSTLDict": {
 *          "abc": "Enum 1",
 *          "def": "Enum 2",
 *          "xyz": "Enum 3"
 *      }
 *    }
 */
#define ACT_JSON_STL_DICT_ENUM(map, first, second, name, key)                 \
 public:                                                                      \
  typedef map<first, second> dict_##name##_t;                                 \
  inline void Set##key(const dict_##name##_t &name) { this->name##_ = name; } \
  inline const dict_##name##_t &Get##key() const { return this->name##_; }    \
  inline dict_##name##_t &Get##key() { return this->name##_; }                \
  QS_JSON_STL_DICT_ENUM(dict_##name##_t, second, name##_, key)                \
                                                                              \
 private:                                                                     \
  dict_##name##_t name##_ = dict_##name##_t();

/**
 * @brief Make dictionary collection of simple types [dictionary<key, itemType> name] and bind serializable properties
 *
 * This collection must be STL DICTIONARY TYPE
 *
 * Example:
 *    ACT_JSON_STL_DICT(std::map, qint8, QString, std_map, StdMap)
 */
#define ACT_JSON_STL_DICT(map, first, second, name, key)                      \
 public:                                                                      \
  typedef map<first, second> dict_##name##_t;                                 \
  inline void Set##key(const dict_##name##_t &name) { this->name##_ = name; } \
  inline const dict_##name##_t &Get##key() const { return this->name##_; }    \
  inline dict_##name##_t &Get##key() { return this->name##_; }                \
  QS_JSON_STL_DICT(dict_##name##_t, name##_, key)                             \
                                                                              \
 private:                                                                     \
  dict_##name##_t name##_ = dict_##name##_t();

/**
 * @brief  Make dictionary collection of custom class objects [dictionary<key, itemType> name] and bind serializable
 * properties
 *
 * This collection must be STL DICTIONARY TYPE
 *
 * Example:
 *    ACT_JSON_STL_DICT_OBJECTS(std::map, QString, ActModule, std_map_objects, StdMapObjects);
 */
#define ACT_JSON_STL_DICT_OBJECTS(map, first, second, name, key)              \
 public:                                                                      \
  typedef map<first, second> dict_##name##_t;                                 \
  inline void Set##key(const dict_##name##_t &name) { this->name##_ = name; } \
  inline const dict_##name##_t &Get##key() const { return this->name##_; }    \
  inline dict_##name##_t &Get##key() { return this->name##_; }                \
  QS_JSON_STL_DICT_OBJECTS(dict_##name##_t, name##_, key)                     \
                                                                              \
 private:                                                                     \
  dict_##name##_t name##_ = dict_##name##_t();

/**
 * @brief Make set collection of custom class objects [QSet<itemType>] and bind serializable
 * properties
 *
 * This collection must be QSet TYPE and the itemType must be ENUM TYPE
 *
 * Example:
 * Step1: Declare the enum class first
 *    enum class TestEnum { kEnum1, kEnum2, kEnum3 };
 *
 * Step2: Declare the string mapping map
 *    static const QMap<QString, TestEnum> kTestEnumMap = {
 *      {"Enum 1", TestEnum::kEnum1},
 *      {"Enum 2", TestEnum::kEnum2},
 *      {"Enum 3", TestEnum::kEnum3}
 *    };
 *
 * Step3: Declare the qHash() function for QSet
 *     inline uint qHash(TestEnum key, uint seed) { return qHash(static_cast<uint>(key), seed); }  ///< For QSet
 *
 * Step4: Declare the macro in the user defined class
 *    class XXX : public QSerializer {
 *      ACT_JSON_QT_SET_ENUM(TestEnum, qt_enum_set, QtEnumSet);
 *    }
 *
 * Step5: Use the enum type in the source code *
 *    QSet<TestEnum> qt_enum_set;
 *    qt_enum_set.insert(TestEnum::kEnum1);
 *    qt_enum_set.insert(TestEnum::kEnum1);
 *    qt_enum_set.insert(TestEnum::kEnum2);
 *    qt_enum_set.insert(TestEnum::kEnum3);
 *
 *    XXX x;
 *    x.SetQtEnumSet(qt_enum_set);
 *
 * Step6: Check the output in the JSON of the XXX class
 *    {
 *      "QtEnumSet": [
 *          "Enum 2",
 *          "Enum 1",
 *          "Enum 3"
 *      ]
 *    }
 */
#define ACT_JSON_QT_SET_ENUM(type, name, key)                                \
 public:                                                                     \
  typedef QSet<type> set_##name##_t;                                         \
  inline void Set##key(const set_##name##_t &name) { this->name##_ = name; } \
  inline const set_##name##_t &Get##key() const { return this->name##_; }    \
  inline set_##name##_t &Get##key() { return this->name##_; }                \
  QS_JSON_QT_SET_ENUM(type, name##_, key)                                    \
                                                                             \
 private:                                                                    \
  set_##name##_t name##_ = set_##name##_t();

/**
 * @brief Make set collection of simple types [QSet<itemType>] and bind serializable
 * properties
 *
 * This collection must be QSet TYPE
 *
 * Example:
 *    ACT_JSON_QT_SET(qint8, qt_set, QtSet);
 *
 */
#define ACT_JSON_QT_SET(type, name, key)                                     \
 public:                                                                     \
  typedef QSet<type> set_##name##_t;                                         \
  inline void Set##key(const set_##name##_t &name) { this->name##_ = name; } \
  inline const set_##name##_t &Get##key() const { return this->name##_; }    \
  inline set_##name##_t &Get##key() { return this->name##_; }                \
  QS_JSON_QT_SET(type, name##_, key)                                         \
                                                                             \
 private:                                                                    \
  set_##name##_t name##_ = set_##name##_t();

/**
 * @brief Make set collection of custom class objects [QSet<itemType>] and bind serializable
 * properties
 *
 * This collection must be QSet TYPE
 *
 * This collection must implement operator==, for example:
 *    friend bool operator==(const ActModule& x, const ActModule& y) {
 *      return x.lic_activate_ == y.lic_activate_ && x.user_enable_ == y.user_enable_;
 *    }
 *
 * This collection must implement qHash(), for example:
 *    friend uint qHash(const ActModule& mod) {
 *      return qHash(mod.lic_activate_ & mod.user_enable_, 0xa03f);  // arbitrary value
 *    }
 *
 * Example:
 *    ACT_JSON_QT_SET_OBJECTS(ActModule, qt_set_objs, QtSetObjects);
 *
 */
#define ACT_JSON_QT_SET_OBJECTS(type, name, key)                             \
 public:                                                                     \
  typedef QSet<type> set_##name##_t;                                         \
  inline void Set##key(const set_##name##_t &name) { this->name##_ = name; } \
  inline const set_##name##_t &Get##key() const { return this->name##_; }    \
  inline set_##name##_t &Get##key() { return this->name##_; }                \
  QS_JSON_QT_SET_OBJECTS(type, name##_, key)                                 \
                                                                             \
 private:                                                                    \
  set_##name##_t name##_ = set_##name##_t();

/**
 * @brief Make set collection of custom class objects [QSet<itemType>] and bind serializable
 * properties
 *
 * This collection must be QSet TYPE and the itemType must be ENUM TYPE
 *
 * Example:
 * Step1: Declare the enum class first
 *    enum class TestEnum { kEnum1, kEnum2, kEnum3 };
 *
 * Step2: Declare the string mapping map
 *    static const QMap<QString, TestEnum> kTestEnumMap = {
 *      {"Enum 1", TestEnum::kEnum1},
 *      {"Enum 2", TestEnum::kEnum2},
 *      {"Enum 3", TestEnum::kEnum3}
 *    };
 *
 * Step3: Declare the macro in the user defined class
 *    class XXX : public QSerializer {
 *      ACT_JSON_STL_SET_ENUM(TestEnum, stl_enum_set, STLEnumSet);
 *    }
 *
 * Step4: Use the enum type in the source code *
 *    std::set<TestEnum> stl_enum_set;
 *    stl_enum_set.insert(TestEnum::kEnum1);
 *    stl_enum_set.insert(TestEnum::kEnum1);
 *    stl_enum_set.insert(TestEnum::kEnum2);
 *    stl_enum_set.insert(TestEnum::kEnum3);
 *
 *    XXX x;
 *    x.SetQtEnumSet(stl_enum_set);
 *
 * Step5: Check the output in the JSON of the XXX class
 *    {
 *      "STLEnumSet": [
 *          "Enum 1",
 *          "Enum 2",
 *          "Enum 3"
 *      ]
 *    }
 */
#define ACT_JSON_STL_SET_ENUM(type, name, key)                               \
 public:                                                                     \
  typedef std::set<type> set_##name##_t;                                     \
  inline void Set##key(const set_##name##_t &name) { this->name##_ = name; } \
  inline const set_##name##_t &Get##key() const { return this->name##_; }    \
  inline set_##name##_t &Get##key() { return this->name##_; }                \
  QS_JSON_STL_SET_ENUM(type, name##_, key)                                   \
                                                                             \
 private:                                                                    \
  set_##name##_t name##_ = set_##name##_t();

/**
 * @brief Make set collection of simple types [std::set<itemType>] and bind serializable
 * properties
 *
 * This collection must be std::set TYPE
 *
 * Example:
 *    ACT_JSON_STL_SET(qint8, std_set, StdSet);
 */
#define ACT_JSON_STL_SET(type, name, key)                                    \
 public:                                                                     \
  typedef std::set<type> set_##name##_t;                                     \
  inline void Set##key(const set_##name##_t &name) { this->name##_ = name; } \
  inline const set_##name##_t &Get##key() const { return this->name##_; }    \
  inline set_##name##_t &Get##key() { return this->name##_; }                \
  QS_JSON_STL_SET(type, name##_, key)                                        \
                                                                             \
 private:                                                                    \
  set_##name##_t name##_ = set_##name##_t();

/**
 * @brief Make set collection of custom class objects [std::set<itemType>] and bind serializable
 * properties
 *
 * This collection must be std::set TYPE
 *
 * Must implement operator<, for example:
 *   friend bool operator<(const ActModule& x, const ActModule& y) {
 *      return std::tie(x.lic_activate_, x.user_enable_) <
 *             std::tie(y.lic_activate_, y.user_enable_);  // keep the same order
 *   }
 *
 * Example:
 *    ACT_JSON_STL_SET_OBJECTS(ActModule, std_set_objs, StdSetObjects);
 */
#define ACT_JSON_STL_SET_OBJECTS(type, name, key)                            \
 public:                                                                     \
  typedef std::set<type> set_##name##_t;                                     \
  inline void Set##key(const set_##name##_t &name) { this->name##_ = name; } \
  inline const set_##name##_t &Get##key() const { return this->name##_; }    \
  inline set_##name##_t &Get##key() { return this->name##_; }                \
  QS_JSON_STL_SET_OBJECTS(type, name##_, key)                                \
                                                                             \
 private:                                                                    \
  set_##name##_t name##_ = set_##name##_t();
