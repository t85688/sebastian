/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <QDir>
#include <QMutex>

#include "act_firmware_db.hpp"
#include "act_network_baseline_db.hpp"
#include "act_project_db.hpp"
#include "act_status.hpp"
#include "act_system_db.hpp"
#include "act_topology_db.hpp"
#include "act_user_db.hpp"

namespace act {
namespace database {

extern QMutex db_mutex;

#define ACT_DATABASE_FOLDER "db"  ///< The file name of the encrypted license

// #define ACT_USER_DB_NAME \

#define ACT_SYSTEM_DB_NAME \
  ACT_DATABASE_FOLDER      \
  "/"                      \
  "system.json"  ///< The name of user JSON db file

#define ACT_USER_DB_FOLDER \
  ACT_DATABASE_FOLDER      \
  "/"                      \
  "users"  ///< The name of user JSON db folder

#define ACT_PROJECT_DB_FOLDER \
  ACT_DATABASE_FOLDER         \
  "/"                         \
  "projects"  ///< The name of project JSON db folder

#define ACT_FIRMWARE_DB_FOLDER \
  ACT_DATABASE_FOLDER          \
  "/"                          \
  "firmwares"  ///< The name of firmware JSON db folder

#define ACT_NETWORK_BASELINE_DB_FOLDER \
  ACT_DATABASE_FOLDER                  \
  "/"                                  \
  "network_baseline"  ///< The name of network_baseline JSON db folder

#define ACT_TOPOLOGY_BASE_DB_FOLDER \
  ACT_DATABASE_FOLDER               \
  "/"                               \
  "topologies"  ///< The name of topology JSON db folder

#define ACT_TOPOLOGY_DB_FOLDER \
  ACT_TOPOLOGY_BASE_DB_FOLDER  \
  "/"                          \
  "configurations"  ///< The name of topology JSON db folder

#define ACT_TOPOLOGY_ICON_DB_FOLDER \
  ACT_TOPOLOGY_BASE_DB_FOLDER       \
  "/"                               \
  "icons"  ///< The name of topology icon db folder

// kene+
static QString GetDatabaseFolder() {
  // ACT_DATABASE_FOLDER
  QString defaultPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return defaultPath + QDir::separator() + "db";
}
static QString GetSystemDbName() {
  // ACT_SYSTEM_DB_NAME
  return GetDatabaseFolder() + QDir::separator() + "system.json";
}
static QString GetUserDbFolder() {
  // ACT_USER_DB_FOLDER
  return GetDatabaseFolder() + QDir::separator() + "users";
}
static QString GetProjectDbFolder() {
  // ACT_PROJECT_DB_FOLDER
  return GetDatabaseFolder() + QDir::separator() + "projects";
}
static QString GetFirmwareDbFolder() {
  // ACT_FIRMWARE_DB_FOLDER
  return GetDatabaseFolder() + QDir::separator() + "firmwares";
}
static QString GetDesignBaselineDbFolder() {
  // ACT_DESIGN_BASELINE_DB_FOLDER
  return GetDatabaseFolder() + QDir::separator() + "design_baseline";
}
static QString GetOperationBaselineDbFolder() {
  // ACT_OPERATION_BASELINE_DB_FOLDER
  return GetDatabaseFolder() + QDir::separator() + "operation_baseline";
}
static QString GetTopologyBaseDbFolder() {
  // ACT_TOPOLOGY_BASE_DB_FOLDER
  return GetDatabaseFolder() + QDir::separator() + "topologies";
}
static QString GetTopologyDbFolder() {
  // ACT_TOPOLOGY_DB_FOLDER
  return GetTopologyBaseDbFolder() + QDir::separator() + "configurations";
}
static QString GetTopologyIconDbFolder() {
  // ACT_TOPOLOGY_ICON_DB_FOLDER
  return GetTopologyBaseDbFolder() + QDir::separator() + "icons";
}
// kene-

// Making sure to declare it inline to avoid breaking the one definition rule:
// https://en.wikipedia.org/wiki/One_Definition_Rule
inline ACT_STATUS Init(bool &project_db_exist) {
  ACT_STATUS_INIT();
  // Check the database folder is exist
  // kene+
  /*
  if (!QDir(ACT_DATABASE_FOLDER).exists()) {
  */
  QString databaseFolder = GetDatabaseFolder();
  if (!QDir(databaseFolder).exists()) {
    // Create the database folder
    /*
    if (!QDir().mkdir(ACT_DATABASE_FOLDER)) {
      qDebug() << "mkdir() failed:" << ACT_DATABASE_FOLDER;
    */
    if (!QDir().mkpath(databaseFolder)) {
      qDebug() << "mkpath() failed:" << databaseFolder;
      // kene-
      return std::make_shared<ActStatusInternalError>("Database");
    }
  }

  act_status = act::database::system::Init();
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Initial system database failed";
    return act_status;
  }

  act_status = act::database::user::Init();
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Initial user database failed";
    return act_status;
  }

  act_status = act::database::project::Init(project_db_exist);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Initial project database failed";
    return act_status;
  }

  act_status = act::database::firmware::Init();
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Initial firmware database failed";
    return act_status;
  }

  act_status = act::database::topology::Init();
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Initial topology database failed";
    return act_status;
  }

  act_status = act::database::networkbaseline::Init();
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Initial network_baseline database failed";
    return act_status;
  }

  return act_status;
}

template <class T>
inline ACT_STATUS ReadFromDB(T &type, QString &db_name) {
  ACT_STATUS_INIT();

  // qDebug() << "ReadFromDB()";

  // kene+
  /*
  if (!QDir(ACT_DATABASE_FOLDER).exists()) {
    qCritical() << "ReadFromDB() failed: folder not exist:" << ACT_DATABASE_FOLDER;
  */
  QString databaseFolder = GetDatabaseFolder();
  if (!QDir(databaseFolder).exists()) {
    qCritical() << "ReadFromDB() failed: folder not exist:" << databaseFolder;
    // kene-
    return std::make_shared<ActStatusInternalError>("Database");
  }

  QFile file(db_name);

  // Open the JSON file
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Open db file failed:" << db_name;
    return std::make_shared<ActStatusInternalError>("Database");
  }

  // Read all contents at one time
  QByteArray ba = file.readAll();

  // Close the JSON file
  file.close();

  // If the database content is empty, just return
  if (ba.length() == 0) {
    return std::make_shared<ActStatusInternalError>("Database");
  }

  QString err_msg;
  QJsonParseError e;
  QJsonDocument json_doc = QJsonDocument::fromJson(ba, &e);
  if (e.error != QJsonParseError::NoError) {
    err_msg = e.errorString();
    qCritical() << "Error in file:" << db_name << "at offset:" << e.offset;

    // Optional: Convert offset to line and column
    int line = 1, column = 1;
    for (int i = 0; i < e.offset; ++i) {
      if (ba[i] == '\n') {
        ++line;
        column = 1;
      } else {
        ++column;
      }
    }
    qCritical() << "Error location: Line" << line << ", Column" << column;

    return std::make_shared<ActStatusInternalError>("Database");
  }

  if (json_doc.isNull()) {
    err_msg = "The JSON is empty";
    qCritical() << err_msg;
    return std::make_shared<ActStatusInternalError>("Database");
  }

  if (!json_doc.isObject()) {
    err_msg = "The JSON is not an object";
    qCritical() << err_msg;
    return std::make_shared<ActStatusInternalError>("Database");
  }

  // Convert QJsonDocument to QJsonObject
  QJsonObject root_obj = json_doc.object();

  // Parse Json to object
  type.fromJson(root_obj);

  return act_status;
}

template <class T>
inline ACT_STATUS WriteToDB(const T &type, const QString &db_name) {
  ACT_STATUS_INIT();

  // qDebug() << "WriteToDB()";
  // Check the database folder is exist
  // kene+
  /*
  if (!QDir(ACT_DATABASE_FOLDER).exists()) {
  */
  if (!QDir(GetDatabaseFolder()).exists()) {
    // kene-
    return std::make_shared<ActStatusInternalError>("Database");
  }

  QFile file(db_name);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Cannot open the file for writing:" << qPrintable(file.errorString());
    return std::make_shared<ActStatusInternalError>("Database");
  }

  // Write the system db file
  QString content = type.ToString();
  file.write(content.toUtf8());

  // Close the JSON file
  file.close();

  return act_status;
}

template <class T>
inline ACT_STATUS WriteToDB(const T &type, const QString &db_name, const QList<QString> &key_order_) {
  ACT_STATUS_INIT();

  QMutexLocker locker(&act::database::db_mutex);

  // qDebug() << "WriteToDB()";
  // Check the database folder is exist
  // kene+
  /*
  if (!QDir(ACT_DATABASE_FOLDER).exists()) {
  */
  if (!QDir(GetDatabaseFolder()).exists()) {
    // kene-
    return std::make_shared<ActStatusInternalError>("Database");
  }
  QFile file(db_name);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Cannot open the file for writing:" << qPrintable(file.errorString());
    return std::make_shared<ActStatusInternalError>("Database");
  }
  // Write the system db file
  QString content = type.ToString(key_order_);
  if (content.size() == 0) {
    qFatal("WriteToDB() failed: content is empty");
  }
  file.write(content.toUtf8());
  // Close the JSON file
  file.close();

  return act_status;
}

/**
 * @brief Write data to database folder
 *
 * @tparam T The class type of the item should be wrote
 * @param folder
 * @param file_name
 * @param item
 * @return ACT_STATUS
 */
template <class T>
ACT_STATUS WriteToDBFolder(const QString &folder, const QString file_name, const T &item) {
  ACT_STATUS_INIT();

  // Write to the folder
  QString db_name(folder);
  db_name.append("/");
  db_name.append(file_name);
  db_name.append(".json");
  act_status = act::database::WriteToDB(item, db_name);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot write data to database folder:" << folder;
    return std::make_shared<ActStatusInternalError>("Database");
  }

  return act_status;
}

/**
 * @brief Write data to database folder
 *
 * @tparam T The class type of the item should be wrote
 * @param folder
 * @param file_name
 * @param item
 * @param key_order_
 * @return ACT_STATUS
 */
template <class T>
ACT_STATUS WriteToDBFolder(const QString &folder, const QString file_name, const T &item,
                           const QList<QString> &key_order_) {
  ACT_STATUS_INIT();

  // Write to the folder
  QString db_name(folder);
  db_name.append("/");
  db_name.append(file_name);
  db_name.append(".json");
  act_status = act::database::WriteToDB(item, db_name, key_order_);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot write data to database folder:" << folder;
    return std::make_shared<ActStatusInternalError>("Database");
  }

  return act_status;
}

/**
 * @brief Delete from database folder
 *
 * @param folder
 * @param file_name
 * @return ACT_STATUS
 */
inline ACT_STATUS DeleteFromFolder(const QString &folder, const QString file_name) {
  ACT_STATUS_INIT();

  QString db_name(folder);
  db_name.append("/");
  db_name.append(file_name);
  db_name.append(".json");

  // Delete the file
  if (!QFile::remove(db_name)) {
    qCritical() << "Cannot remove data from database folder:" << folder;
    return std::make_shared<ActStatusInternalError>("Database");
  }

  return act_status;
}

/**
 * @brief Update file name in database folder
 *
 * @param folder
 * @param old_file_name
 * @param new_file_name
 * @return ACT_STATUS
 */
inline ACT_STATUS UpdateFileName(const QString &folder, const QString old_file_name, const QString new_file_name) {
  ACT_STATUS_INIT();

  // Old file path
  QString old_file(folder);
  old_file.append("/");
  old_file.append(old_file_name);
  old_file.append(".json");

  // New file path
  QString new_file(folder);
  new_file.append("/");
  new_file.append(new_file_name);
  new_file.append(".json");

  // Update the file
  if (!QFile::rename(old_file, new_file)) {
    qCritical() << "Cannot update data from database folder:" << folder;
    return std::make_shared<ActStatusInternalError>("Database");
  }

  return act_status;
}

}  // namespace database
}  // namespace act
