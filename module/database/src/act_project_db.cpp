#include "act_db.hpp"
#include "act_json.hpp"

// Namespace names are all lower-case, with words separated by underscores.
// https://google.github.io/styleguide/cppguide.html#Namespace_Names
namespace act {
namespace database {
namespace project {
ACT_STATUS Init(bool &project_db_exist) {
  ACT_STATUS_INIT();

  // kene+
  /*
  QDir dir(ACT_PROJECT_DB_FOLDER);
  */
  QString projectDbFolder = act::database::GetProjectDbFolder();
  QDir dir(projectDbFolder);
  // kene-
  if (dir.exists()) {
    project_db_exist = true;
    return act_status;
  }

  // kene+
  /*
  if (!QDir().mkpath(ACT_PROJECT_DB_FOLDER)) {
    qDebug() << "mkpath() failed:" << ACT_PROJECT_DB_FOLDER;
  */
  if (!QDir().mkpath(projectDbFolder)) {
    qDebug() << "mkpath() failed:" << projectDbFolder;
    // kene-
    return std::make_shared<ActStatusInternalError>("Database");
  }

  project_db_exist = false;

  return act_status;
}

ACT_STATUS RetrieveData(QSet<ActProject> &project_set, qint64 &last_assigned_project_id) {
  ACT_STATUS_INIT();

  project_set.clear();

  // Retrieve the data from database
  // kene+
  /*
  QDir dir(ACT_PROJECT_DB_FOLDER);
  */
  QDir dir(act::database::GetProjectDbFolder());
  // kene-
  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActProject p;

      act_status = act::database::ReadFromDB(p, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "ReadFromDB() failed: project config database";
        return act_status;
      }

      // [bugfix:2514] AutoScan can not identify device
      p.DecryptPassword();
      project_set.insert(p);

      // Update the id to improve the first create performance
      last_assigned_project_id = p.GetId();
    }
  }

  return act_status;
}

ACT_STATUS WriteData(const ActProject &project) {
  if (project.GetId() == 0) {
    qCritical() << "WriteData() failed: project id is 0";
    return std::make_shared<ActStatusInternalError>("Database");
  }
  // [bugfix:2514] AutoScan can not identify device
  ActProject copy_project = project;
  copy_project.EncryptPassword();

  // [feat: 2795] Organize file names in DB
  QString file_name = QString::number(project.GetId());
  file_name.append("_");
  file_name.append(project.GetProjectSetting().GetProjectName());

  // kene+
  /*
  return act::database::WriteToDBFolder<ActProject>(ACT_PROJECT_DB_FOLDER, file_name, copy_project, project.key_order_);
  */
  return act::database::WriteToDBFolder<ActProject>(act::database::GetProjectDbFolder(), file_name, copy_project,
                                                    project.key_order_);
  // kene-
}

ACT_STATUS DeleteProjectFile(const qint64 &id, QString project_name) {
  QString file_name = QString::number(id);
  file_name.append("_");
  file_name.append(project_name);

  // kene+
  /*
  return act::database::DeleteFromFolder(ACT_PROJECT_DB_FOLDER, file_name);
  */
  return act::database::DeleteFromFolder(act::database::GetProjectDbFolder(), file_name);
  // kene-
}

ACT_STATUS UpdateProjectFileName(const qint64 &id, QString old_project_name, QString new_project_name) {
  QString old_file_name = QString::number(id);
  old_file_name.append("_");
  old_file_name.append(old_project_name);

  QString new_file_name = QString::number(id);
  new_file_name.append("_");
  new_file_name.append(new_project_name);

  // kene+
  /*
  return act::database::UpdateFileName(ACT_PROJECT_DB_FOLDER, old_file_name, new_file_name);
  */
  return act::database::UpdateFileName(act::database::GetProjectDbFolder(), old_file_name, new_file_name);
  // kene-
}

}  // namespace project
}  // namespace database
}  // namespace act
