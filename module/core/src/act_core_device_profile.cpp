#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {
ACT_STATUS ActCore::InitDeviceProfiles() {
  ACT_STATUS_INIT();

  // Read device profiles from configuration folder
  QSet<ActDeviceProfile> device_profile_set;
  qint64 last_assigned_device_profile_id = -1;

  device_profile_set.clear();

  // Retrieve the data from database
  // kene+
  /*
  QDir dir(ACT_DEVICE_PROFILE_FOLDER);
  */
  QDir dir(GetDeviceProfilePath());
  // kene-
  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActDeviceProfile data;
      act_status = act::database::ReadFromDB(data, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "ReadFromDB() failed: device profile";
        return act_status;
      }

      // [feat:2849] Deploy - Identify device feature for different firmware
      // Update support firmware version set
      if (data.GetSupportFirmwareVersions().isEmpty()) {
        data.GetSupportFirmwareVersions().insert(data.GetLatestFirmwareVersion());
      }

      // Assign data to output argument
      device_profile_set.insert(data);

      // Update the id to improve the first create performance
      last_assigned_device_profile_id = data.GetId();
    }
  }

  this->SetDeviceProfileSet(device_profile_set);
  this->last_assigned_device_profile_id_ = last_assigned_device_profile_id;

  return act_status;
}

ACT_STATUS ActCore::InitPowerDeviceProfiles() {
  ACT_STATUS_INIT();

  // Read power device profiles from configuration folder
  QSet<ActPowerDeviceProfile> power_device_profile_set;

  power_device_profile_set.clear();

  // Retrieve the data from database
  QDir dir(GetPowerDeviceProfilePath());
  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActPowerDeviceProfile data;
      act_status = act::database::ReadFromDB(data, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "ReadFromDB() failed: power device profile";
        return act_status;
      }

      // Assign data to output argument
      power_device_profile_set.insert(data);
    }
  }

  this->SetPowerDeviceProfileSet(power_device_profile_set);

  return act_status;
}

ACT_STATUS ActCore::InitSoftwareLicenseProfiles() {
  ACT_STATUS_INIT();

  // Read software license profiles from configuration folder
  QSet<ActSoftwareLicenseProfile> software_license_profile_set;

  software_license_profile_set.clear();

  // Retrieve the data from database
  QDir dir(GetSoftwareLicenseProfilePath());
  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActSoftwareLicenseProfile data;
      act_status = act::database::ReadFromDB(data, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "ReadFromDB() failed: software license profile";
        return act_status;
      }

      // Assign data to output argument
      software_license_profile_set.insert(data);
    }
  }

  this->SetSoftwareLicenseProfileSet(software_license_profile_set);

  return act_status;
}

ACT_STATUS ActCore::UploadDeviceProfile(ActDeviceProfile &device_profile) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QSet<ActDeviceProfile> device_profile_set = this->GetDeviceProfileSet();

  // Check the model name empty
  if (device_profile.GetModelName().isEmpty()) {
    QString error_msg = "The Model Name is empty";
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Validate Model Name length
  const int model_length = device_profile.GetModelName().length();
  if (model_length < 1 || model_length > 64) {
    QString error_msg = QString("Model Name length must be 1~64 (got %1)").arg(model_length);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Validate Description length
  const int desc_length = device_profile.GetDescription().length();
  if (desc_length < 1 || desc_length > 512) {
    QString error_msg = QString("Description length must be 1~512 (got %1)").arg(desc_length);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Validate Vendor length
  const int vendor_length = device_profile.GetVendor().length();
  if (vendor_length < 1 || vendor_length > 64) {
    QString error_msg = QString("Vendor length must be 1~64 (got %1)").arg(vendor_length);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check model name is valid
  // Check if contains special characters (\ / : * ? “ < > |) that can not use in windows
  const QString model_name = device_profile.GetModelName();
  const QString forbidden_char = "\\/:*?\"<>|";
  for (const QChar &c : forbidden_char) {
    if (model_name.contains(c)) {
      QString error_msg = QString("Model name contains forbidden characters, the name is %1").arg(model_name);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Check if trailing space and multiple consecutive spaces
  if (model_name.endsWith(" ") || model_name.contains(QRegExp("\\s{2,}"))) {
    QString error_msg =
        QString("Model name contains multiple consecutive spaces or end with space, the name is %1").arg(model_name);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check the model name does not exist
  for (auto dp : device_profile_set) {
    if (dp.GetModelName() == device_profile.GetModelName()) {
      qCritical() << "The model name" << device_profile.GetModelName() << "is duplicated";
      return std::make_shared<ActDuplicatedError>(QString("%1").arg(device_profile.GetModelName()));
    }
  }

  // Generate a new unique id
  qint64 id;
  act_status = this->GenerateUniqueId<ActDeviceProfile>(device_profile_set, this->last_assigned_device_profile_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot get an available unique id";
    return act_status;
  }

  device_profile.SetId(id);
  device_profile.SetDataVersion(ACT_DEVICE_PROFILE_DATA_VERSION);
  device_profile.SetBuiltIn(false);

  // [feat:2849] Deploy - Identify device feature for different firmware
  // Update support firmware version set
  if (device_profile.GetSupportFirmwareVersions().isEmpty()) {
    device_profile.GetSupportFirmwareVersions().insert(device_profile.GetLatestFirmwareVersion());
  }

  // Insert the device_profile to core set
  device_profile_set.insert(device_profile);

  // Write to configuration folder
  // act_status = act::database::device_profile::WriteData(device_profile);
  // [feat: 2795] Organize file names in DB
  // kene+
  /*
  QString file_name(ACT_DEVICE_PROFILE_FOLDER);
  */
  QString file_name(GetDeviceProfilePath());
  // kene-
  file_name.append("/");
  file_name.append(QString::number(id));
  file_name.append("_");
  file_name.append(device_profile.GetModelName());
  file_name.append(".json");

  QFile file(file_name);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Cannot open the file for writing:" << qPrintable(file.errorString());
    return std::make_shared<ActStatusInternalError>("Device Profile");
  }

  // Write to the device profile folder
  // Reset key_order by device_type
  device_profile.ResetKeyOrderByDeviceType();
  QString content = device_profile.ToString(device_profile.key_order_);
  file.write(content.toUtf8());

  // Close the JSON file
  file.close();

  this->SetDeviceProfileSet(device_profile_set);

  // Send update msg
  ActDeviceProfilePatchUpdateMsg msg(ActPatchUpdateActionEnum::kCreate, device_profile, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, msg);

  return act_status;
}

ACT_STATUS ActCore::DeleteDeviceProfile(qint64 &id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QSet<ActDeviceProfile> device_profile_set = this->GetDeviceProfileSet();

  // Check the item does exist by id
  typename QSet<ActDeviceProfile>::const_iterator iterator;
  iterator = device_profile_set.find(ActDeviceProfile(id));

  if (iterator == device_profile_set.end()) {
    QString error_msg = QString("Delete device profile failed, cannot found device profile id %1").arg(id);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  ActDeviceProfile dev_prop = *iterator;
  if (dev_prop.GetBuiltIn()) {
    QString message = QString("Cannot remove built-in device profile");
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  bool project_used = false;
  bool topology_used = false;
  bool baseline_used = false;
  QMap<QString, QSet<QString>> project_used_info;
  QMap<QString, QSet<QString>> topology_used_info;
  QMap<QString, QSet<QString>> baseline_used_info;

  // Check the device profile is unused for any project
  for (ActProject project : this->GetProjectSet()) {
    QSet<QString> used_device;
    for (ActDevice device : project.GetDevices()) {
      if (device.GetDeviceProfileId() == dev_prop.GetId()) {
        used_device.insert(device.GetIpv4().GetIpAddress());
        project_used = true;
      }
    }

    if (used_device.size() > 0) {
      project_used_info[project.GetProjectName()] = used_device;
    }
  }

  // [feat:2241] Check the device profile is unused for any topology
  for (ActTopology topology : this->GetTopologySet()) {
    QSet<QString> used_device;
    for (ActDevice device : topology.GetDevices()) {
      if (device.GetDeviceProfileId() == dev_prop.GetId()) {
        used_device.insert(device.GetIpv4().GetIpAddress());
        topology_used = true;
      }
    }

    if (used_device.size() > 0) {
      topology_used_info[topology.GetTopologyName()] = used_device;
    }
  }

  // Check the device profile is unused for any baseline
  for (ActNetworkBaseline baseline : this->GetDesignBaselineSet()) {
    ActProject project = baseline.GetProject();
    QSet<QString> used_device;
    for (ActDevice device : project.GetDevices()) {
      if (device.GetDeviceProfileId() == dev_prop.GetId()) {
        used_device.insert(device.GetIpv4().GetIpAddress());
        baseline_used = true;
      }
    }

    if (used_device.size() > 0) {
      baseline_used_info[baseline.GetName()] = used_device;
    }
  }

  if (project_used || topology_used || baseline_used) {
    ActDeviceProfileIsUsedFailedParameter bad_request_parameters;
    QList<ActProjectUsedDeviceIpList> used_projects;
    QList<ActTopologyUsedDeviceIpList> used_topologies;
    QList<ActBaselineUsedDeviceIpList> used_baselines;

    QString error_msg("The device profile is used by the following ");
    error_msg.append("Projects:");
    if (project_used) {
      for (QString project_name : project_used_info.keys()) {
        QSet<QString> device_list = project_used_info[project_name];

        error_msg.append(project_name);
        error_msg.append(": ");
        int count = device_list.size();
        for (QString device_ip : device_list) {
          error_msg.append(device_ip);
          if (count > 1) {
            error_msg.append(", ");
          }
          count--;
        }
        error_msg.append(";");

        ActProjectUsedDeviceIpList used_device_ip_list;
        used_device_ip_list.SetProjectName(project_name);
        used_device_ip_list.SetDevices(device_list);
        used_projects.push_back(used_device_ip_list);
      }
    }

    if (topology_used) {
      error_msg.append(" Topologies:");
      for (QString topology_name : topology_used_info.keys()) {
        QSet<QString> device_list = topology_used_info[topology_name];

        error_msg.append(topology_name);
        error_msg.append(": ");
        int count = device_list.size();
        for (QString device_ip : device_list) {
          error_msg.append(device_ip);
          if (count > 1) {
            error_msg.append(", ");
          }
          count--;
        }
        error_msg.append(";");

        ActTopologyUsedDeviceIpList used_device_ip_list;
        used_device_ip_list.SetTopologyName(topology_name);
        used_device_ip_list.SetDevices(device_list);
        used_topologies.push_back(used_device_ip_list);
      }
    }

    error_msg.append("Baselines:");
    if (baseline_used) {
      for (QString baseline_name : baseline_used_info.keys()) {
        QSet<QString> device_list = baseline_used_info[baseline_name];

        error_msg.append(baseline_name);
        error_msg.append(": ");
        int count = device_list.size();
        for (QString device_ip : device_list) {
          error_msg.append(device_ip);
          if (count > 1) {
            error_msg.append(", ");
          }
          count--;
        }
        error_msg.append(";");

        ActBaselineUsedDeviceIpList used_device_ip_list;
        used_device_ip_list.SetBaselineName(baseline_name);
        used_device_ip_list.SetDevices(device_list);
        used_baselines.push_back(used_device_ip_list);
      }
    }

    bad_request_parameters.SetUsedProjects(used_projects);
    bad_request_parameters.SetUsedTopologies(used_topologies);
    bad_request_parameters.SetUsedBaselines(used_baselines);

    std::shared_ptr<ActDeviceProfileIsUsedFailed> bad_request = std::make_shared<ActDeviceProfileIsUsedFailed>();
    bad_request->SetErrorMessage(error_msg);
    bad_request->SetParameter(bad_request_parameters);

    qDebug() << "bad_request:" << bad_request->ToString().toStdString().c_str();

    return bad_request;
  }

  // Piece the device profile filename
  // [feat: 2795] Organize file names in DB
  // kene+
  /*
  QString device_profile_filename(ACT_DEVICE_PROFILE_FOLDER);
  */
  QString deviceProfilePath = GetDeviceProfilePath();
  QString device_profile_filename(deviceProfilePath);
  // kene-
  device_profile_filename.append("/");
  device_profile_filename.append(QString::number(id));
  device_profile_filename.append("_");
  device_profile_filename.append((*iterator).GetModelName());
  device_profile_filename.append(".json");

  // Delete the file
  if (!QFile::remove(device_profile_filename)) {
    // kene+
    /*
    qCritical() << "Cannot remove device profile from configuration folder:" << ACT_DEVICE_PROFILE_FOLDER;
    */
    qCritical() << "Cannot remove device profile from configuration folder:" << deviceProfilePath;
    // kene-
  }

  // Delete it
  // qDebug() << "Delete item id:" << QString::number(id);
  device_profile_set.erase(iterator);
  this->SetDeviceProfileSet(device_profile_set);

  // Send update msg
  ActDeviceProfilePatchUpdateMsg msg(ActPatchUpdateActionEnum::kDelete, dev_prop, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, msg);

  return act_status;
}

ACT_STATUS ActCore::UpdateDeviceProfile(ActDeviceProfile &device_profile) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // qDebug() << "WriteToDB()";
  // Check the database folder is exist
  // kene+
  /*
  if (!QDir(ACT_DEVICE_PROFILE_FOLDER).exists()) {
  */
  QString deviceProfilePath = GetDeviceProfilePath();
  if (!QDir(deviceProfilePath).exists()) {
    // kene-
    return std::make_shared<ActStatusInternalError>("The device profile folder doesn't exist");
  }

  // Check model name is valid
  // Check if contains special characters (\ / : * ? “ < > |) that can not use in windows
  const QString model_name = device_profile.GetModelName();

  // Validate Model Name length
  const int model_length = model_name.length();
  if (model_length < 1 || model_length > 64) {
    QString error_msg = QString("Model Name length must be 1~64 (got %1)").arg(model_length);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Validate Description length
  const int desc_length = device_profile.GetDescription().length();
  if (desc_length < 1 || desc_length > 512) {
    QString error_msg = QString("Description length must be 1~512 (got %1)").arg(desc_length);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Validate Vendor length
  const int vendor_length = device_profile.GetVendor().length();
  if (vendor_length < 1 || vendor_length > 64) {
    QString error_msg = QString("Vendor length must be 1~64 (got %1)").arg(vendor_length);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  const QString forbidden_char = "\\/:*?\"<>|";
  for (const QChar &c : forbidden_char) {
    if (model_name.contains(c)) {
      QString error_msg = QString("Model name contains forbidden characters, the name is %1").arg(model_name);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Check if trailing space and multiple consecutive spaces
  if (model_name.endsWith(" ") || model_name.contains(QRegExp("\\s{2,}"))) {
    QString error_msg =
        QString("Model name contains multiple consecutive spaces or end with space, the name is %1").arg(model_name);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // [feat:2849] Deploy - Identify device feature for different firmware
  // Update support firmware version set
  if (device_profile.GetSupportFirmwareVersions().isEmpty()) {
    device_profile.GetSupportFirmwareVersions().insert(device_profile.GetLatestFirmwareVersion());
  }

  // [feat: 2795] Organize file names in DB
  // kene+
  /*
  QString device_profile_filename(ACT_DEVICE_PROFILE_FOLDER);
  */
  QString device_profile_filename(deviceProfilePath);
  // kene+
  device_profile_filename.append("/");
  device_profile_filename.append(QString::number(device_profile.GetId()));
  device_profile_filename.append("_");
  device_profile_filename.append(device_profile.GetModelName());
  device_profile_filename.append(".json");

  QFile file(device_profile_filename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Cannot open the file for writing:" << qPrintable(file.errorString());
    return std::make_shared<ActStatusInternalError>("Can't write the device profile");
  }

  // Write the system db file
  QString content = device_profile.ToString(device_profile.key_order_);
  file.write(content.toUtf8());

  // Close the JSON file
  file.close();

  // Send update msg
  ActDeviceProfilePatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, device_profile, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, msg);

  return act_status;
}

}  // namespace core
}  // namespace act
