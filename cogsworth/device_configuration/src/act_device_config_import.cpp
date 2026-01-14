#include "act_device_configuration.hpp"

ACT_STATUS ActDeviceConfiguration::StartImportConfig(const ActProject &project, const QList<qint64> &dev_id_list,
                                                     const QString &file_path) {
  // Checking has the thread is running
  if (IsActStatusRunning(device_config_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("DeviceConfiguration");
  }

  // init ActDeviceConfiguration status
  progress_ = 0;
  stop_flag_ = false;
  device_config_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to start ImportConfig
  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((device_config_thread_ != nullptr) && (device_config_thread_->joinable())) {
      device_config_thread_->join();
    }
    device_config_act_status_->SetStatus(ActStatusType::kRunning);
    device_config_thread_ =
        std::make_unique<std::thread>(&ActDeviceConfiguration::TriggerImportConfigForThread, this, std::cref(project),
                                      std::cref(dev_id_list), std::cref(file_path));

    qDebug() << "Start ImportConfig thread.";

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(ImportConfig) failed. Error:" << e.what();
    device_config_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("DeviceConfiguration");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*device_config_act_status_), progress_);
}

void ActDeviceConfiguration::TriggerImportConfigForThread(const ActProject &project, const QList<qint64> &dev_id_list,
                                                          const QString &file_path) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the ImportConfig and wait for the return, and update device_config_thread_.
  try {
    device_config_act_status_ = ImportConfig(project, dev_id_list, file_path);
  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(interrupt) failed. Error:" << e.what();
    device_config_act_status_ = std::make_shared<ActStatusInternalError>("DeviceConfiguration");
  }
}

ACT_STATUS ActDeviceConfiguration::ImportConfig(const ActProject &project, const QList<qint64> &dev_id_list,
                                                const QString &file_path) {
  ACT_STATUS_INIT();

  // Create device list
  QList<qint64> sorted_dev_id_list = dev_id_list;
  SortDeviceIdList(project, sorted_dev_id_list);

  QList<ActDevice> dev_list;
  for (auto &dev_id : sorted_dev_id_list) {
    ActDevice dev;
    act_status = project.GetDeviceById(dev, dev_id);
    if (!IsActStatusSuccess(act_status)) {
      DeviceConfigurationErrorHandler(__func__, "Device not found in the project", dev);
      continue;
    }
    dev_list.append(dev);
  }

  // Update Devices ICMP status to check alive
  southbound_.UpdateDevicesIcmpStatus(dev_list);
  if (stop_flag_) {
    return ACT_STATUS_STOP;
  }

  UpdateProgress(10);

  for (auto dev : dev_list) {
    if (stop_flag_) {  // stop flag
      return ACT_STATUS_STOP;
    }
    act_status = ACT_STATUS_SUCCESS;

    // Check ICMP status
    if (!dev.GetDeviceStatus().GetICMPStatus()) {
      DeviceConfigurationErrorHandler(__func__, "ICMP status is false(not alive)", dev);
      continue;
    }

    // Update connect status to true for southbound
    dev.GetDeviceStatus().SetAllConnectStatus(true);

    // Try to get FirmwareVersion to find firmware_feature_profile
    // ActFeatureSubItem fw_feature_sub_item;
    // act_status = GetDeviceFeatureSubItem(dev, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
    //                                      ActFeatureEnum::kAutoScan, "Identify", "FirmwareVersion",
    //                                      fw_feature_sub_item);
    // if (IsActStatusSuccess(act_status)) {
    //   QString firmware_version;
    //   act_status = southbound_.ActionGetFirmwareVersion(dev, fw_feature_sub_item, firmware_version);
    //   if (!IsActStatusSuccess(act_status)) {
    //     DeviceConfigurationErrorHandler(__func__, "Get device Firmware version failed", dev);
    //     continue;
    //   }
    //   // Try to find firmware_feature_profile
    //   ActFirmwareFeatureProfile fw_feat_profile;
    //   auto find_status = ActFirmwareFeatureProfile::GetFirmwareFeatureProfile(profiles_.GetFirmwareFeatureProfiles(),
    //                                                                           dev.GetDeviceProperty().GetModelName(),
    //                                                                           firmware_version, fw_feat_profile);
    //   if (IsActStatusSuccess(find_status)) {
    //     dev.SetFirmwareFeatureProfileId(fw_feat_profile.GetId());
    //   }
    // }

    // Feature > Item > SubItem
    ActFeatureSubItem feature_sub_item;
    act_status = GetDeviceFeatureSubItem(dev, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kOperation, "ImportExport", "Basic", feature_sub_item);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetDeviceFeatureSubItem() failed.";
      DeviceConfigurationErrorHandler(__func__, act_status->GetErrorMessage(), dev);
      continue;
    }

    // Import device config
    act_status = southbound_.ActionImportConfig(dev, feature_sub_item, file_path);
    if (!IsActStatusSuccess(act_status)) {
      DeviceConfigurationErrorHandler(__func__, act_status->GetErrorMessage(), dev);
      continue;
    }

    // Add success result to result_queue_
    result_queue_.enqueue(ActDeviceConfigureResult(dev.GetId(), ActStatusType::kSuccess));

    // Update progress(10~90/100)
    quint8 new_progress = progress_ + (80 / dev_list.size());
    if (new_progress >= 90) {
      UpdateProgress(90);
    } else {
      UpdateProgress(new_progress);
    }
  }

  SLEEP_MS(1000);
  UpdateProgress(100);
  act_status = ACT_STATUS_SUCCESS;
  return act_status;
}