#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::UploadFirmware(ActFirmware &firmware) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QSet<ActFirmware> firmware_set = this->GetFirmwareSet();

  // Check the firmware_file exists
  QString file_path = QString("%1/%2").arg(ACT_FIRMWARE_FILE_FOLDER).arg(firmware.GetFirmwareName());
  QFile file(file_path);
  if (!file.exists()) {
    auto not_found_str = QString("Firmware file(%1)").arg(firmware.GetFirmwareName());
    qCritical() << __func__ << QString("%1 not found").arg(not_found_str).toStdString().c_str();
    return std::make_shared<ActStatusNotFound>(not_found_str);
  }

  // Check the firmware does not duplicated
  for (auto fw : firmware_set) {
    if (fw.GetFirmwareName() == firmware.GetFirmwareName()) {
      qDebug() << __func__
               << QString("The firmware %1 is duplicated, would directly replace file")
                      .arg(firmware.GetFirmwareName())
                      .toStdString()
                      .c_str();

      // Directly replace file so return success
      return act_status;
    }
  }

  // Check firmware name is valid
  // Check if contains special characters (\ / : * ? “ < > |) that can not use in windows
  const QString firmware_name = firmware.GetFirmwareName();
  const QString forbidden_char = "\\/:*?\"<>|";
  for (const QChar &c : forbidden_char) {
    if (firmware_name.contains(c)) {
      QString error_msg = QString("Model name contains forbidden characters, the name is %1").arg(firmware_name);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Check if trailing space and multiple consecutive spaces
  if (firmware_name.endsWith(" ") || firmware_name.contains(QRegExp("\\s{2,}"))) {
    QString error_msg = QString("Firmware name contains multiple consecutive spaces or end with space, the name is %1")
                            .arg(firmware_name);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Generate a new unique id
  qint64 id;
  act_status = this->GenerateUniqueId<ActFirmware>(firmware_set, this->last_assigned_firmware_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Cannot get an available unique id";
    return act_status;
  }

  firmware.SetId(id);

  firmware.SetDataVersion(ACT_FIRMWARE_DATA_VERSION);

  // Insert the device_profile to core set
  firmware_set.insert(firmware);
  this->SetFirmwareSet(firmware_set);

  // Write to db
  act_status = act::database::firmware::WriteData(firmware);

  return act_status;
}

ACT_STATUS ActCore::DeleteFirmware(const qint64 &id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QSet<ActFirmware> firmware_set = this->GetFirmwareSet();

  // Check the item does exist by id
  typename QSet<ActFirmware>::const_iterator fw_iterator;
  fw_iterator = firmware_set.find(ActFirmware(id));

  if (fw_iterator == firmware_set.end()) {
    QString error_msg = QString("Delete firmware failed, cannot found firmware id %1").arg(id);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }
  QString firmware_name = fw_iterator->GetFirmwareName();
  QString model_name = fw_iterator->GetModelName();

  // Delete firmware
  // qDebug() << __func__ << "Delete item id:" << QString::number(id);
  firmware_set.erase(fw_iterator);

  this->SetFirmwareSet(firmware_set);

  // Delete firmware at database
  act_status = act::database::firmware::DeleteFirmwareFile(id, model_name, firmware_name);
  if (!IsActStatusSuccess(act_status)) {
    qCritical()
        << __func__
        << QString("Delete firmware failed, cannot delete database's firmware(%1.json)").arg(id).toStdString().c_str();
    return act_status;
  }

  // Delete firmware_file
  QString file_path = QString("%1/%2").arg(ACT_FIRMWARE_FILE_FOLDER).arg(firmware_name);
  QFile file(file_path);
  if (file.exists()) {  // check the firmware_file exists
    if (!file.remove()) {
      qCritical() << __func__ << "Cannot remove firmware_file from firmware file folder:" << ACT_FIRMWARE_FILE_FOLDER;
      return std::make_shared<ActStatusInternalError>("Delete Firmware");
    }
  }

  return act_status;
}
}  // namespace core
}  // namespace act
