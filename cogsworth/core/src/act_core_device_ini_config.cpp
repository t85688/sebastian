#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"
#include "act_offline_config.hpp"
#include "device_configuration/act_maf_offline_config_export_file.hpp"

namespace act {
namespace core {

ACT_STATUS GetExportConfigFileContent(QList<ActMafExportConfigFileContent> &export_config_file_content_list) {
  ACT_STATUS_INIT();
  export_config_file_content_list.clear();

  // Read files and find the sub zip
  QString device_config_file_path = GetDeviceConfigFilePath();
  QString files_file_path(QString("%1/files.json").arg(device_config_file_path));
  QFile files_file(files_file_path);
  if (!files_file.open(QIODevice::ReadOnly)) {
    QString err_msg = QString("Cannot read file: %1").arg(files_file_path);
    return std::make_shared<ActBadRequest>(err_msg);
  }
  // Read all contents at one time
  QByteArray ba = files_file.readAll();
  // Close the JSON file
  files_file.close();

  QJsonParseError e;
  QJsonDocument json_doc = QJsonDocument::fromJson(ba, &e);
  if (e.error != QJsonParseError::NoError) {
    QString err_msg = e.errorString();
    qCritical() << err_msg;
    return std::make_shared<ActBadRequest>(err_msg);
  }

  if (!json_doc.isArray()) {
    QString err_msg = QString("JSON Root is not a JSON array!");
    qCritical() << err_msg;
    return std::make_shared<ActBadRequest>(err_msg);
  }

  QJsonArray arr = json_doc.array();
  for (const QJsonValue &val : arr) {
    if (!val.isObject()) continue;
    QJsonObject obj = val.toObject();
    QString json_str = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    ActMafExportConfigFileContent export_config_file_content;
    export_config_file_content.FromString(json_str);
    export_config_file_content_list.append(export_config_file_content);
  }

  return act_status;
}

ACT_STATUS ActCore::GenerateDeviceIniConfigZipFile(qint64 &project_id, ActDeviceBackupFile &zip_file) {
  ACT_STATUS_INIT();

  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QList<qint64> dev_id_list;

  // Generate the all can deploy device
  for (auto dev : project.GetDevices()) {
    if (dev.CheckCanDeploy()) {
      dev_id_list.append(dev.GetId());
    }
  }

  // Clear DeviceConfig tmp folder
  QString deviceConfigFilePath = GetDeviceConfigFilePath();
  act_status = ClearFolder(deviceConfigFilePath);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Generate the ini file
  ActDeviceOfflineConfigFileMap device_offline_config_file_map;
  act_status = GenerateDeviceIniConfigFile(project, dev_id_list, device_offline_config_file_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Check map[deviceId] has value (generated offline config)
  act_status = CheckDeviceConfigValuesNotEmpty(project, dev_id_list, device_offline_config_file_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  act_status = act::offline_config::SaveOfflineConfigToTmpFolder(dev_id_list);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("There is no device available to generate an offline configuration");
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  QString file_name(QString(MAF_EXPORT_CONFIG_FILE_NAME));
  QString file_path(QString("%1/%2").arg(deviceConfigFilePath).arg(file_name));

  QByteArray file_content;
  act_status = this->ReadFileContent(file_path, file_content);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Read file content failed:" << act_status->GetErrorMessage().toStdString().c_str();
    return act_status;
  }

  zip_file.SetFileName(file_name);
  zip_file.SetBackupFile(file_content.toBase64());

  return act_status;
}

ACT_STATUS ActCore::CheckDeviceConfigValuesNotEmpty(
    const ActProject &project, const QList<qint64> &dev_id_list,
    const ActDeviceOfflineConfigFileMap &device_offline_config_file_map) {
  ACT_STATUS_INIT();

  // Check folder exist
  QString deviceConfigFilePath = GetDeviceConfigFilePath();
  QDir dir(deviceConfigFilePath);
  if (!dir.exists()) {
    return std::make_shared<ActStatusNotFound>(QString("%1 folder").arg(deviceConfigFilePath));
  }

  // Check each devices in device id list has file id (generated offline config)
  for (auto dev_id : dev_id_list) {
    // Check exist
    QString file_id = device_offline_config_file_map.GetDeviceOfflineConfigFileMap()[dev_id];

    if (file_id.isEmpty()) {
      // get device
      ActDevice dev;
      act_status = project.GetDeviceById(dev, dev_id);
      if (IsActStatusNotFound(act_status)) {  // not found device
        return act_status;
      }

      qCritical() << __func__ << QString("Device(%1) config file is not found").arg(dev.GetIpv4().GetIpAddress());
      return std::make_shared<ActStatusNotFound>(QString("Device(%1) configfile").arg(dev.GetIpv4().GetIpAddress()));
    }
  }

  return act_status;
}

ACT_STATUS ActCore::GenerateDesignBaselineDeployDeviceIniConfigFile(
    const qint64 &project_id, const qint64 &design_baseline_id, const QList<qint64> &dev_id_list,
    ActDeviceOfflineConfigFileMap &device_offline_config_file_map) {
  ACT_STATUS_INIT();
  // If dev_id_list is empty, it would generate all deployable devices

  ActProject project;
  if (design_baseline_id == -1) {  // Handle CURRENT(id == -1)
    act_status = this->GetProject(project_id, project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Get project failed with project id:" << project_id;
      return act_status;
    }
  } else {
    act_status = this->GetDesignBaselineProject(qint64(project_id), design_baseline_id, project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << QString("Get the Design Baseline's project failed.(ProjectID: %1, DesignBaselineID: %2)")
                         .arg(project_id)
                         .arg(design_baseline_id);
      return act_status;
    }
  }

  QList<qint64> gen_dev_id_list;
  // Generate the all can deploy device
  for (auto dev : project.GetDevices()) {
    // Skip not in target device
    if (!dev_id_list.isEmpty()) {
      if (!dev_id_list.contains(dev.GetId())) {
        continue;
      }
    }

    if (dev.CheckCanDeploy()) {
      gen_dev_id_list.append(dev.GetId());
    }
  }

  // Clear DeviceConfig tmp folder
  QString deviceConfigFilePath = GetDeviceConfigFilePath();
  act_status = ClearFolder(deviceConfigFilePath);
  // kene-
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Generate the ini file
  act_status = GenerateDeviceIniConfigFile(project, gen_dev_id_list, device_offline_config_file_map);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GenerateDeviceIniConfigFile() failed";
    return act_status;
  }
  qDebug() << __func__
           << "device_offline_config_file_map:" << device_offline_config_file_map.ToString().toStdString().c_str();
  // Check map[deviceId] has value (generated offline config)
  act_status = CheckDeviceConfigValuesNotEmpty(project, gen_dev_id_list, device_offline_config_file_map);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "CheckDeviceConfigValuesNotEmpty() failed";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::GenerateDeviceIniConfigFile(const ActProject &project, const QList<qint64> &dev_id_list,
                                                ActDeviceOfflineConfigFileMap &device_offline_config_file_map) {
  ACT_STATUS_INIT();
  device_offline_config_file_map.GetDeviceOfflineConfigFileMap().clear();

  // clear MAF fileDB
  act_status = act::offline_config::ClearMafFileDb();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << QString("clear maf fileDB fail");

    return act_status;
  }

  QString deviceConfigFilePath = GetDeviceConfigFilePath();
  QDir dir(deviceConfigFilePath);

  if (!dir.exists()) {
    return std::make_shared<ActStatusNotFound>(QString("%1 folder").arg(deviceConfigFilePath));
  }

  for (auto dev_id : dev_id_list) {
    // Check device exists
    ActDevice dev;
    act_status = project.GetDeviceById(dev, dev_id);
    if (IsActStatusNotFound(act_status)) {
      return act_status;
    }

    QString file_id;
    act_status = act::offline_config::GenerateOfflineConfig(project, dev_id, file_id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__
                  << QString("GenerateOfflineConfig() failed. Device(%1).").arg(dev.GetIpv4().GetIpAddress());

      return act_status;
    }
    device_offline_config_file_map.GetDeviceOfflineConfigFileMap()[dev_id] = file_id;
  }

  return act_status;
}

ACT_STATUS ActCore::ExportAndUnzipConfigFile(const ActProject &project, const QList<qint64> &dev_id_list,
                                             ActDeviceOfflineConfigFileMap &device_offline_config_file_map) {
  ACT_STATUS_INIT();
  device_offline_config_file_map.GetDeviceOfflineConfigFileMap().clear();

  act_status = act::offline_config::SaveOfflineConfigToTmpFolder(dev_id_list);
  if (!IsActStatusSuccess(act_status)) {
    QString error_msg = QString("There is no device available to generate an offline configuration");
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  QString device_config_file_path = GetDeviceConfigFilePath();
  QString root_zip_file_name(QString(MAF_EXPORT_CONFIG_FILE_NAME));
  QString root_zip_file_path(QString("%1/%2").arg(device_config_file_path).arg(root_zip_file_name));

  // Unzip root folder
  act_status = act::core::g_core.UnZipFile(root_zip_file_path, device_config_file_path);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  QList<ActMafExportConfigFileContent> export_config_file_content_list;
  act_status = GetExportConfigFileContent(export_config_file_content_list);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  for (auto export_file_content : export_config_file_content_list) {
    qDebug() << __func__ << "export_file_content:" << export_file_content.ToString().toStdString().c_str();
    auto pro_device_id_str = export_file_content.Getproperties().GetdeviceID();
    qint64 device_id = pro_device_id_str.toLongLong();
    if (dev_id_list.contains(device_id)) {
      qDebug() << __func__ << "Uzip subfile:" << pro_device_id_str;

      QString zip_file_path = export_file_content.Getpath();
      act_status = act::core::g_core.UnZipFile(zip_file_path, device_config_file_path);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      QFileInfo fi(zip_file_path);
      // QString ini_file_path = fi.absolutePath() + "/" + fi.completeBaseName() + ".ini";
      QString ini_file_path(QString("%1\\%2.ini").arg(device_config_file_path).arg(fi.completeBaseName()));
      qDebug() << __func__ << "ini_file_path:" << ini_file_path;

      device_offline_config_file_map.GetDeviceOfflineConfigFileMap()[device_id] = ini_file_path;
    }
  }

  return act_status;
}

}  // namespace core
}  // namespace act
