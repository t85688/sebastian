#include "act_db.hpp"
#include "act_firmware.hpp"

namespace act {
namespace database {
namespace firmware {

ACT_STATUS Init() {
  ACT_STATUS_INIT();

  QSet<ActFirmware> firmware_set;
  // QSet<ActFirmware> new_firmware_set;
  qint64 last_assigned_firmware_id = -1;

  // kene+
  /*
  QDir fw_db_dir(ACT_FIRMWARE_DB_FOLDER);
  */
  QString firmwareDbFolder = act::database::GetFirmwareDbFolder();
  QDir fw_db_dir(firmwareDbFolder);
  // kene-
  if (fw_db_dir.exists()) {
    // return act_status;
    // Retrive the firmware configuration
    act_status = RetrieveData(firmware_set, last_assigned_firmware_id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Cannot retrieve the firmware configuration";
      return std::make_shared<ActStatusInternalError>("Database");
    }
  } else {
    // kene+
    /*
    if (!QDir().mkpath(ACT_FIRMWARE_DB_FOLDER)) {
      qDebug() << __func__ << "mkpath() failed:" << ACT_FIRMWARE_DB_FOLDER;
    */
    if (!QDir().mkpath(firmwareDbFolder)) {
      qDebug() << __func__ << "mkpath() failed:" << firmwareDbFolder;
      // kene-
      return std::make_shared<ActStatusInternalError>("Database");
    }
  }

  // Create built-in firmware configuration

  // Check the firmware_file exists
  for (auto firmware : firmware_set) {
    QString file_path = QString("%1/%2").arg(ACT_FIRMWARE_FILE_FOLDER).arg(firmware.GetFirmwareName());

    // qDebug() << __func__ << "file_path:" << file_path;
    QFile file(file_path);
    if (file.exists()) {
      continue;
    }

    // Delete firmware from database
    qDebug() << __func__
             << QString("Delete firmware which non exists firmware_file: %1(%2).")
                    .arg(firmware.GetFirmwareName())
                    .arg(firmware.GetId())
                    .toStdString()
                    .c_str();

    act_status = DeleteFirmwareFile(firmware.GetId(), firmware.GetModelName(), firmware.GetFirmwareName());
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "DeleteFirmwareFile() failed: Delete firmware from database";
      return act_status;
    }
  }

  // Append the new firmware that non exists the database
  QDir fw_file_dir(ACT_FIRMWARE_FILE_FOLDER);
  QFileInfoList list = fw_file_dir.entryInfoList(QDir::Files);  // get firmware_file list
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      last_assigned_firmware_id = last_assigned_firmware_id + 1;
      QString fw_file_name = list.at(i).fileName();
      ActFirmware new_firmware(last_assigned_firmware_id, fw_file_name);

      // If firmware already exits in the database
      if (firmware_set.contains(new_firmware)) {
        last_assigned_firmware_id = last_assigned_firmware_id - 1;
        continue;
      }

      act_status = WriteData(new_firmware);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "Cannot save the firmware configuration";
        return std::make_shared<ActStatusInternalError>("Database");
      }
    }
  }

  // qDebug() << __func__ << "Success: Create built-in firmware";

  return act_status;
}

ACT_STATUS RetrieveData(QSet<ActFirmware> &firmware_set, qint64 &last_assigned_firmware_id) {
  ACT_STATUS_INIT();

  firmware_set.clear();

  // Retrieve the data from database
  // kene+
  /*
  QDir dir(ACT_FIRMWARE_DB_FOLDER);
  */
  QDir dir(act::database::GetFirmwareDbFolder());
  // kene-
  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActFirmware fw;
      act_status = act::database::ReadFromDB(fw, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ReadFromDB() failed: firmware config database";
        return act_status;
      }

      // Assign data to output argument
      firmware_set.insert(fw);

      // Update the id to improve the first create performance
      last_assigned_firmware_id = fw.GetId();
    }
  }

  return act_status;
}

ACT_STATUS WriteData(const ActFirmware &firmware) {
  // [feat: 2795] Organize file names in DB
  QString file_name = QString::number(firmware.GetId());
  file_name.append("_");
  file_name.append(firmware.GetModelName());
  file_name.append("_");
  file_name.append(firmware.GetFirmwareName());

  // kene+
  /*
  return act::database::WriteToDBFolder<ActFirmware>(ACT_FIRMWARE_DB_FOLDER, file_name, firmware, firmware.key_order_);
  */
  return act::database::WriteToDBFolder<ActFirmware>(act::database::GetFirmwareDbFolder(), file_name, firmware,
                                                     firmware.key_order_);
  // kene-
}

ACT_STATUS DeleteFirmwareFile(const qint64 &id, QString model_name, QString firmware_name) {
  QString file_name = QString::number(id);
  file_name.append("_");
  file_name.append(model_name);
  file_name.append("_");
  file_name.append(firmware_name);

  // kene+
  /*
  return act::database::DeleteFromFolder(ACT_FIRMWARE_DB_FOLDER, file_name);
  */
  return act::database::DeleteFromFolder(act::database::GetFirmwareDbFolder(), file_name);
  // kene-
}

}  // namespace firmware
}  // namespace database
}  // namespace act
