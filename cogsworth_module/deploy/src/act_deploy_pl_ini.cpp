#include "act_deploy.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QJsonDocument>
#include <QJsonObject>

#include "act_algorithm.hpp"

ACT_STATUS act::deploy::ActDeploy::StartPLIniDeployer(const ActProject &project) {
  ACT_STATUS_INIT();

  // Checking has the thread is running
  if (IsActStatusRunning(deployer_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("Deploy");
  }

  // init deployer status
  progress_ = 0;
  deployer_stop_flag_ = false;
  deployer_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to triggered the deployer
  try {
    // check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((deployer_thread_ != nullptr) && (deployer_thread_->joinable())) {
      deployer_thread_->join();
    }

    deployer_act_status_->SetStatus(ActStatusType::kRunning);
    deployer_thread_ = std::make_unique<std::thread>(&act::deploy::ActDeploy::TriggeredPLIniDeployerForThread, this,
                                                     std::cref(project));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggeredPLIniDeployerForThread";
    HRESULT hr = SetThreadDescription(deployer_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(pl ini deployer) failed. Error:" << e.what();
    deployer_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("Deploy");
  }

  qDebug() << "Start PL Ini deployer thread.";
  return std::make_shared<ActProgressStatus>(ActStatusBase(*deployer_act_status_), progress_);
}

void act::deploy::ActDeploy::TriggeredPLIniDeployerForThread(const ActProject &project) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the deployer and wait for the return, and update deployer_act_status_.
  try {
    deployer_act_status_ = PLIniDeployer(project, dev_id_list);
  } catch (std::exception &e) {
    qCritical() << __func__ << "PLIniDeployer() failed. Error:" << e.what();
    deployer_act_status_ = std::make_shared<ActStatusInternalError>("Deploy");
  }
}

ACT_STATUS act::deploy::ActDeploy::PLIniDeployer(const ActProject &project) {
  ACT_STATUS_INIT();

  auto dev_config = project.GetDeviceConfig();

  // Generate the Deploy device list
  QList<ActDevice> deploy_dev_list;
  for (auto &dev_id : dev_id_list) {
    // Get project device by dev_id
    ActDevice dev;
    act_status = project.GetDeviceById(dev, dev_id);
    if (!IsActStatusSuccess(act_status)) {
      DeployIniErrorHandler(__func__, "Device not found in the project", dev);
      continue;
    }

    deploy_dev_list.append(dev);
  }

  // Check feature & module
  bool devices_checked_result = true;
  for (auto &dev : deploy_dev_list) {
    // Mapping Device firmware (Update FeatureGroup and FirmwareFeatureProfileId)
    act_status = MappingDeviceFirmware(dev, dev_config);
    if (!IsActStatusSuccess(act_status)) {
      DeployIniErrorHandler(__func__, "Mapping Device firmware failed", dev);
      devices_checked_result = false;
      continue;
    }
    // Check it support the ImportExport feature
    if (!dev.GetDeviceProperty().GetFeatureGroup().GetOperation().GetImportExport()) {
      DeployIniErrorHandler(__func__, "Device not support the ImportExport feature", dev);
      devices_checked_result = false;
      continue;
    }

    // TODO: Check module (power & ethernet)
  }

  // Any one device check failed would stop & another devices would response stop status
  if (!devices_checked_result) {
    for (auto &dev : deploy_dev_list) {
      if (!failed_device_id_set_.contains(dev.GetId())) {
        result_queue_.enqueue(ActDeviceConfigureResult(dev.GetId(), progress_, ActStatusType::kStop, "Stop configure",
                                                       "Some devices check the features failed"));
      }
    }

    SLEEP_MS(100);
    UpdateProgress(100);
    act_status = ACT_STATUS_SUCCESS;
    return act_status;
  }

  UpdateProgress(10);

  // Start deploy (20%~90%)
  for (auto &dev : deploy_dev_list) {
    if (deployer_stop_flag_) {
      return ACT_STATUS_STOP;
    }

    bool set_arp_bool = false;

    // Get ini file
    QString file_name;
    act_status = GetIniConfigFileName(dev, file_name);
    if (!IsActStatusSuccess(act_status)) {
      DeployIniErrorHandler(__func__, "The configuration file(.ini) not found", dev);
      continue;
    }

    // Get online device
    ActDevice online_dev(dev);
    auto ip_setting_tables = project.GetDeviceConfig().GetMappingDeviceIpSettingTables();
    ActMappingDeviceIpSettingTable ip_setting_table;
    if (ip_setting_tables.contains(dev.GetId())) {  // use old IP to import device
      ip_setting_table = ip_setting_tables[dev.GetId()];
      online_dev.GetIpv4().SetIpAddress(ip_setting_table.GetOnlineIP());
      online_dev.SetMacAddress(ip_setting_table.GetMacAddress());

      set_arp_bool = true;
    }

    // Set ArpTable
    if (set_arp_bool) {
      // Check Map has device's mac
      if (!mac_host_map_.contains(online_dev.GetMacAddress())) {  // hasn't
        QString error_msg = QString("MAC table not found the MAC(%1)").arg(online_dev.GetMacAddress());
        DeployIniErrorHandler(__func__, error_msg, dev);
        continue;
      }
      QString host = mac_host_map_[online_dev.GetMacAddress()];
      act_status = southbound_.SetLocalhostArpTable(online_dev, host);
      if (!IsActStatusSuccess(act_status)) {
        DeployIniErrorHandler(__func__, "Add the ARP entry failed", dev);
        continue;
      }
    }

    // Import
    qInfo() << __func__
            << QString("Import %1 to Device(%2)")
                   .arg(file_name)
                   .arg(online_dev.GetIpv4().GetIpAddress())
                   .toStdString()
                   .c_str();
    // kene+
    /*
    auto import_status = ImportConfig(online_dev, QString("%1/%2").arg(ACT_DEVICE_CONFIG_FILE_FOLDER).arg(file_name));
    */
    auto import_status = ImportConfig(online_dev, QString("%1/%2").arg(getDeviceConfigFilePath()).arg(file_name));
    // kene-
    //  Delete Arp entry
    if (set_arp_bool) {
      auto act_status_arp = southbound_.DeleteLocalhostArpEntry(online_dev);
      if (!IsActStatusSuccess(act_status_arp)) {
        qWarning() << __func__ << "DeleteLocalhostArpEntry() failed.";
      }
    }

    if (!IsActStatusSuccess(import_status)) {
      DeployIniErrorHandler(__func__, "Execute the Import config failed", dev);
      continue;
    }

    // Add success result to result_queue_
    result_queue_.enqueue(ActDeviceConfigureResult(dev.GetId(), progress_, ActStatusType::kSuccess));

    // Update progress(10~90/100)
    quint8 new_progress = progress_ + (80 / deploy_dev_list.size());
    if (new_progress >= 90) {
      UpdateProgress(90);
    } else {
      UpdateProgress(new_progress);
    }
  }

  SLEEP_MS(100);
  UpdateProgress(100);
  act_status = ACT_STATUS_SUCCESS;
  return act_status;
}
