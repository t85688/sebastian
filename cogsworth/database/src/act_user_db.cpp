#include "act_db.hpp"
#include "act_user.hpp"

// Namespace names are all lower-case, with words separated by underscores.
// https://google.github.io/styleguide/cppguide.html#Namespace_Names
namespace act {
namespace database {
namespace user {

ACT_STATUS Init() {
  ACT_STATUS_INIT();

  // kene+
  /*
  QDir dir(ACT_USER_DB_FOLDER);
  */
  QString userDbFolder = act::database::GetUserDbFolder();
  QDir dir(userDbFolder);
  // kene-
  if (dir.exists()) {
    return act_status;
  }

  // kene+
  /*
  if (!QDir().mkpath(ACT_USER_DB_FOLDER)) {
    qDebug() << "mkpath() failed:" << ACT_USER_DB_FOLDER;
  */
  if (!QDir().mkpath(userDbFolder)) {
    qDebug() << "mkpath() failed:" << userDbFolder;
    // kene-
    return std::make_shared<ActStatusInternalError>("Database");
  }

  // Create built-in user configuration
  ActUser admin;
  admin.SetId(0);
  admin.SetUsername("admin");
  admin.SetPassword("moxa");
  admin.SetRole(ActRoleEnum::kAdmin);

  QList<ActServiceProfileForLicenseEnum> admin_profiles;
  admin_profiles.append(ActServiceProfileForLicenseEnum::kSelfPlanning);
  admin_profiles.append(ActServiceProfileForLicenseEnum::kGeneral);
  admin_profiles.append(ActServiceProfileForLicenseEnum::kFoxboro);
  admin.SetProfiles(admin_profiles);

  act_status = WriteData(admin);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot save the user configuration";
    return std::make_shared<ActStatusInternalError>("Database");
  }

  ActUser basic;
  basic.SetId(1);
  basic.SetUsername("basic");
  basic.SetPassword("moxa");
  basic.SetRole(ActRoleEnum::kAdmin);

  QList<ActServiceProfileForLicenseEnum> basic_profiles;
  basic_profiles.append(ActServiceProfileForLicenseEnum::kSelfPlanning);
  basic.SetProfiles(basic_profiles);

  act_status = WriteData(basic);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot save the user configuration";
    return std::make_shared<ActStatusInternalError>("Database");
  }

  qInfo() << "Init Success: Create built-in user";

  return act_status;
}

ACT_STATUS RetrieveData(QSet<ActUser> &user_set, qint64 &last_assigned_user_id) {
  ACT_STATUS_INIT();

  user_set.clear();

  // Retrieve the data from database
  // kene+
  /*
  QDir dir(ACT_USER_DB_FOLDER);
  */
  QDir dir(act::database::GetUserDbFolder());
  // kene-
  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActUser u;
      act_status = act::database::ReadFromDB(u, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "ReadFromDB() failed: user config database";
        return act_status;
      }

      // [bugfix:2514] AutoScan can not identify device
      u.DecryptPassword();

      // Assign data to output argument
      user_set.insert(u);

      // Update the id to improve the first create performance
      last_assigned_user_id = u.GetId();
    }
  }

  return act_status;
}

ACT_STATUS WriteData(const ActUser &user) {
  // [bugfix:2514] AutoScan can not identify device
  ActUser copy_user = user;
  copy_user.EncryptPassword();

  // [feat: 2795] Organize file names in DB
  QString file_name = QString::number(user.GetId());
  file_name.append("_");
  file_name.append(user.GetUsername());

  // kene+
  /*
  return act::database::WriteToDBFolder<ActUser>(ACT_USER_DB_FOLDER, file_name, copy_user, user.key_order_);
  */
  return act::database::WriteToDBFolder<ActUser>(act::database::GetUserDbFolder(), file_name, copy_user,
                                                 user.key_order_);
  // kene-
}

ACT_STATUS DeleteUserFile(const qint64 &id, QString username) {
  QString file_name = QString::number(id);
  file_name.append("_");
  file_name.append(username);

  // kene+
  /*
  return act::database::DeleteFromFolder(ACT_USER_DB_FOLDER, file_name);
  */
  return act::database::DeleteFromFolder(act::database::GetUserDbFolder(), file_name);
  // kene-
}

}  // namespace user
}  // namespace database
}  // namespace act
