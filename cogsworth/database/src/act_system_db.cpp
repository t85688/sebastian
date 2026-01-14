
#include "act_db.hpp"
#include "act_json.hpp"
#include "act_system.hpp"

// Namespace names are all lower-case, with words separated by underscores.
// https://google.github.io/styleguide/cppguide.html#Namespace_Names
namespace act {
namespace database {
namespace system {

ACT_STATUS Init() {
  ACT_STATUS_INIT();
  // Check the database folder is exist
  // kene+
  /*
  if (!QDir(ACT_DATABASE_FOLDER).exists()) {
    qDebug() << "database folder does not exist:" << ACT_DATABASE_FOLDER;
  */
  QString databaseFolder = act::database::GetDatabaseFolder();
  if (!QDir(databaseFolder).exists()) {
    qDebug() << "database folder does not exist:" << databaseFolder;
    // kene-
    return std::make_shared<ActStatusInternalError>("Database");
  }

  // kene+
  /*
  QFile file(ACT_SYSTEM_DB_NAME);
  */
  QFile file(act::database::GetSystemDbName());
  // kene-
  if (file.exists()) {
    return act_status;
  }

  // Create built-in system configuration
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Cannot open the file for writing:" << qPrintable(file.errorString());
    return std::make_shared<ActStatusInternalError>("Database");
  }

  // Write to the file
  ActSystem sys;

  // Only write the configuration to the db file
  QString content = sys.ToString(sys.key_order_);
  file.write(content.toUtf8());

  // Close the JSON file
  file.close();

  return act_status;
}

ACT_STATUS RetrieveData(ActSystem &sys) {
  ACT_STATUS_INIT();

  // Retrieve the data from database
  // The format of output argument and the content in the db are the same,
  // So we just use "sys" here
  // kene+
  /*
  QString db_name(ACT_SYSTEM_DB_NAME);
  */
  QString db_name(act::database::GetSystemDbName());
  // kene-
  act_status = act::database::ReadFromDB(sys, db_name);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ReadFromDB() failed: system database";
    return act_status;
  }

  return act_status;
}

}  // namespace system
}  // namespace database
}  // namespace act
