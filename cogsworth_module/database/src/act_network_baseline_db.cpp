#include "act_db.hpp"
#include "act_network_baseline.hpp"

// Namespace names are all lower-case, with words separated by underscores.
// https://google.github.io/styleguide/cppguide.html#Namespace_Names
namespace act {
namespace database {
namespace networkbaseline {

ACT_STATUS Init() {
  ACT_STATUS_INIT();

  // Design
  QString designBaselineDbFolder = act::database::GetDesignBaselineDbFolder();
  QDir design_dir(designBaselineDbFolder);
  if (design_dir.exists()) {
    return act_status;
  }

  if (!QDir().mkpath(designBaselineDbFolder)) {
    qDebug() << "mkpath() failed:" << designBaselineDbFolder;
    return std::make_shared<ActStatusInternalError>("Database");
  }

  // Operation
  QString operationBaselineDbFolder = act::database::GetOperationBaselineDbFolder();
  QDir operation_dir(operationBaselineDbFolder);
  if (operation_dir.exists()) {
    return act_status;
  }

  if (!QDir().mkpath(operationBaselineDbFolder)) {
    qDebug() << "mkpath() failed:" << operationBaselineDbFolder;
    return std::make_shared<ActStatusInternalError>("Database");
  }

  return act_status;
}

ACT_STATUS RetrieveData(ActBaselineModeEnum mode, QSet<ActNetworkBaseline> &network_baseline_set,
                        qint64 &last_assigned_baseline_id) {
  ACT_STATUS_INIT();

  network_baseline_set.clear();

  auto db_folder = mode == ActBaselineModeEnum::kDesign ? act::database::GetDesignBaselineDbFolder()
                                                        : act::database::GetOperationBaselineDbFolder();
  QDir dir(db_folder);
  // kene-
  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActNetworkBaseline network_baseline;
      act_status = act::database::ReadFromDB(network_baseline, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "ReadFromDB() failed: network_baseline database";
        return act_status;
      }

      network_baseline.DecryptPassword();

      // Assign data to output argument
      network_baseline_set.insert(network_baseline);

      // Update the id to improve the first create performance
      last_assigned_baseline_id = network_baseline.GetId();
    }
  }

  return act_status;
}

ACT_STATUS WriteData(ActBaselineModeEnum mode, const ActNetworkBaseline &network_baseline) {
  // [bugfix:2514] AutoScan can not identify device
  ActNetworkBaseline copy_network_baseline = network_baseline;
  copy_network_baseline.EncryptPassword();

  // [feat: 2795] Organize file names in DB
  QString file_name = QString::number(network_baseline.GetId());
  file_name.append("_");
  file_name.append(network_baseline.GetName());

  auto db_folder = mode == ActBaselineModeEnum::kDesign ? act::database::GetDesignBaselineDbFolder()
                                                        : act::database::GetOperationBaselineDbFolder();

  return act::database::WriteToDBFolder<ActNetworkBaseline>(db_folder, file_name, copy_network_baseline,
                                                            copy_network_baseline.write_db_key_order_);
  // kene-
}

ACT_STATUS DeleteBaselineFile(ActBaselineModeEnum mode, const qint64 &id, QString network_baseline_name) {
  QString file_name = QString::number(id);
  file_name.append("_");
  file_name.append(network_baseline_name);

  auto db_folder = mode == ActBaselineModeEnum::kDesign ? act::database::GetDesignBaselineDbFolder()
                                                        : act::database::GetOperationBaselineDbFolder();
  return act::database::DeleteFromFolder(db_folder, file_name);
}

ACT_STATUS UpdateNetworkBaselineFileName(ActBaselineModeEnum mode, const qint64 &id, QString old_name,
                                         QString new_name) {
  QString old_file_name = QString::number(id);
  old_file_name.append("_");
  old_file_name.append(old_name);

  QString new_file_name = QString::number(id);
  new_file_name.append("_");
  new_file_name.append(new_name);

  auto db_folder = mode == ActBaselineModeEnum::kDesign ? act::database::GetDesignBaselineDbFolder()
                                                        : act::database::GetOperationBaselineDbFolder();
  return act::database::UpdateFileName(db_folder, old_file_name, new_file_name);
}

}  // namespace networkbaseline
}  // namespace database
}  // namespace act
