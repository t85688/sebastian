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

ACT_STATUS act::deploy::ActDeploy::GetIniConfigFileName(const ActDevice &dev, QString &file_name) {
  ACT_STATUS_INIT();

  // kene+
  /*
  QDir dir(ACT_DEVICE_CONFIG_FILE_FOLDER);
  */
  QDir dir(GetDeviceConfigFilePath());
  // kene-

  QStringList filter;
  filter << QString("%1*").arg(dev.GetIpv4().GetIpAddress());  // add regular
  QStringList matching_files = dir.entryList(filter, QDir::Files);
  if (matching_files.isEmpty()) {
    qCritical() << __func__ << QString("Device(%1) config file is not found").arg(dev.GetIpv4().GetIpAddress());
    return std::make_shared<ActStatusNotFound>(QString("Device(%1) config file").arg(dev.GetIpv4().GetIpAddress()));
  }

  file_name = matching_files.first();
  return act_status;
}

ACT_STATUS act::deploy::ActDeploy::StartIniDeployer(const ActProject &project, const QList<qint64> &dev_id_list,
                                                    const bool &skip_mapping_dev) {
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
    deployer_thread_ =
        std::make_unique<std::thread>(&act::deploy::ActDeploy::TriggeredIniDeployerForThread, this, std::cref(project),
                                      std::cref(dev_id_list), std::cref(skip_mapping_dev));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggeredIniDeployerForThread";
    HRESULT hr = SetThreadDescription(deployer_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(ini deployer) failed. Error:" << e.what();
    deployer_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("Deploy");
  }

  qDebug() << "Start deployer thread.";
  return std::make_shared<ActProgressStatus>(ActStatusBase(*deployer_act_status_), progress_);
}

void act::deploy::ActDeploy::TriggeredIniDeployerForThread(const ActProject &project, const QList<qint64> &dev_id_list,
                                                           const bool &skip_mapping_dev) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the deployer and wait for the return, and update deployer_act_status_.
  try {
    deployer_act_status_ = IniDeployer(project, dev_id_list, skip_mapping_dev);
  } catch (std::exception &e) {
    qCritical() << __func__ << "Deployer() failed. Error:" << e.what();
    deployer_act_status_ = std::make_shared<ActStatusInternalError>("Deploy");
  }
}

ACT_STATUS act::deploy::ActDeploy::IniDeployer(const ActProject &project, const QList<qint64> &dev_id_list,
                                               const bool &skip_mapping_dev) {
  ACT_STATUS_INIT();
  qInfo() << __func__ << QString("Deploy list size:%1").arg(dev_id_list.size()).toStdString().c_str();
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

  // Check feature & module & user account
  bool devices_checked_result = true;
  for (auto &dev : deploy_dev_list) {
    if (skip_mapping_dev) {
      // Enable SNMP
      if (dev.GetEnableSnmpSetting() &&
          dev.GetDeviceProperty().GetFeatureGroup().GetOperation().GetEnableSNMPService()) {
        southbound_.FeatureEnableDeviceSnmp(true, dev, false);
      }
    }

    // Mapping Device firmware (Update FeatureGroup and FirmwareFeatureProfileId)
    act_status = MappingDeviceFirmware(dev, dev_config, skip_mapping_dev);
    if (!IsActStatusSuccess(act_status)) {
      DeployIniErrorHandler(__func__, "Get Device information failed", dev);
      devices_checked_result = false;
      continue;
    }

    // Check it support the ImportExport feature
    if (!dev.GetDeviceProperty().GetFeatureGroup().GetOperation().GetImportExport()) {
      DeployIniErrorHandler(__func__, "Device not support the ImportExport feature", dev);
      devices_checked_result = false;
      continue;
    }

    // Check UserAccount
    if (dev_config.GetUserAccountTables().contains(dev.GetId())) {
      // Check it support the UserAccount feature
      if (!dev.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetUserAccount()) {
        DeployIniErrorHandler(__func__, "Device not support the UserAccount feature", dev);
        devices_checked_result = false;
        continue;
      }
    }

    // Check the UserAccount password not empty(except the default account)
    auto user_account_table = dev_config.GetUserAccountTables()[dev.GetId()];
    for (auto account : user_account_table.GetAccounts().keys()) {
      ActUserAccount default_account;
      // qCritical() << __func__ << "user_account_table:" << user_account_table.ToString().toStdString().c_str();

      if (default_account.GetUsername() != account) {  // new account
        if (user_account_table.GetAccounts()[account].GetPassword().isEmpty()) {
          QString error_msg = QString("The UserAccount \"%1\" has an empty password").arg(account);
          DeployIniErrorHandler(__func__, error_msg, dev);
          devices_checked_result = false;
          continue;
        }
      }
    }
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

    // Get ini file
    QString file_name;
    act_status = GetIniConfigFileName(dev, file_name);
    if (!IsActStatusSuccess(act_status)) {
      DeployIniErrorHandler(__func__, "The configuration file(.ini) not found", dev);
      continue;
    }

    bool set_arp_bool = false;
    // Get online device
    ActDevice online_dev(dev);
    if (skip_mapping_dev == false) {
      auto ip_setting_tables = project.GetDeviceConfig().GetMappingDeviceIpSettingTables();
      ActMappingDeviceIpSettingTable ip_setting_table;
      if (ip_setting_tables.contains(dev.GetId())) {  // use old IP to import device
        ip_setting_table = ip_setting_tables[dev.GetId()];
        online_dev.GetIpv4().SetIpAddress(ip_setting_table.GetOnlineIP());
        online_dev.SetMacAddress(ip_setting_table.GetMacAddress());

        set_arp_bool = true;
      }
    }

    // Set ArpTable
    QString host = "";
    if (set_arp_bool) {
      // Check Map has device's mac
      if (!mac_host_map_.contains(online_dev.GetMacAddress())) {  // hasn't
        QString error_msg = QString("MAC table not found the MAC(%1)").arg(online_dev.GetMacAddress());
        DeployIniErrorHandler(__func__, error_msg, dev);
        continue;
      }
      host = mac_host_map_[online_dev.GetMacAddress()];
      act_status = southbound_.SetLocalhostArpTable(online_dev, host);
      if (!IsActStatusSuccess(act_status)) {
        DeployIniErrorHandler(__func__, "Add the ARP entry failed", online_dev);
        continue;
      }
    }

    // Update connect status to true for southbound
    online_dev.GetDeviceStatus().SetAllConnectStatus(true);

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
    auto import_status = ImportConfig(online_dev, QString("%1/%2").arg(GetDeviceConfigFilePath()).arg(file_name));
    // kene-
    //  Delete Arp entry
    if (set_arp_bool) {
      auto act_status_arp = southbound_.DeleteLocalhostArpEntry(online_dev);
      if (!IsActStatusSuccess(act_status_arp)) {
        qWarning() << __func__ << "DeleteLocalhostArpEntry() failed.";
      }
    }

    if (!IsActStatusSuccess(import_status)) {
      DeployIniErrorHandler(__func__, import_status->GetErrorMessage(), dev);
      qCritical() << __func__ << "Failed account" << dev.GetAccount().ToString().toStdString().c_str();
      continue;
    }

    SLEEP_MS(500);

    // Update online_dev for dev's ipv4
    online_dev.SetIpv4(dev.GetIpv4());

    // Update Device Account to default
    ActDeviceAccount default_account;
    // [bugfix:4050] Deployment failed after adding a user account.
    act_status = ActDeviceProfile::GetInitDefaultAccount(dev, profiles_.GetDeviceProfiles(), default_account);
    if (!IsActStatusSuccess(act_status)) {
      DeployIniErrorHandler(__func__, "Get the DeviceProfile failed", dev);
      continue;
    }

    // Set UserAccount
    bool check_with_user_account = dev_config.GetUserAccountTables().contains(dev.GetId()) &&
                                   (!dev_config.GetUserAccountTables()[dev.GetId()].GetAccounts().isEmpty());
    if (check_with_user_account) {
      auto user_account_table = dev_config.GetUserAccountTables()[dev.GetId()];
      auto user_name = user_account_table.GetSyncConnectionAccount();
      if (!user_account_table.GetAccounts().contains(user_name)) {
        DeployIniErrorHandler(__func__, QString("The specified connection account() does not exist").arg(user_name),
                              online_dev);
        continue;
      }

      // [bugfix] Invalid: Your new password can not be the same as your original password, please enter a new password.
      // Check default account password
      if (user_account_table.GetAccounts().contains(default_account.GetUsername())) {
        if (user_account_table.GetAccounts()[default_account.GetUsername()].GetPassword() ==
            default_account.GetPassword()) {
          // Set Default account's password as empty
          // If the password is empty, the southbound will skip modifying the password
          user_account_table.GetAccounts()[default_account.GetUsername()].SetPassword("");
        }
      }

      // Update online_dev account
      online_dev.SetAccount(default_account);

      if (set_arp_bool) {
        act_status = southbound_.SetLocalhostArpTable(online_dev, host);
        if (!IsActStatusSuccess(act_status)) {
          DeployIniErrorHandler(__func__, "Add the ARP entry failed", online_dev);
          continue;
        }
      }
      // user_account_table.HidePassword();
      // qCritical() << __func__ << "user_account_table:" << user_account_table.ToString().toStdString().c_str();

      // [bugfix:4514] Deploy - TSN 設備無法穩定 deploy
      SLEEP_MS(1000);
      act_status = southbound_.ConfigureUserAccount(online_dev, user_account_table);
      // Delete Arp entry
      if (set_arp_bool) {
        auto act_status_arp = southbound_.DeleteLocalhostArpEntry(online_dev);
        if (!IsActStatusSuccess(act_status_arp)) {
          qWarning() << __func__ << "DeleteLocalhostArpEntry() failed.";
        }
      }

      if (!IsActStatusSuccess(act_status)) {
        DeployIniErrorHandler(__func__, act_status->GetErrorMessage(), online_dev);
        continue;
      }
    }

    // Sync User Account
    if (check_with_user_account) {
      // Sync the user-specified account to the device's connection
      ActDeviceAccount sync_account;
      auto user_account_table = dev_config.GetUserAccountTables()[dev.GetId()];
      auto user_name = user_account_table.GetSyncConnectionAccount();
      if (!user_account_table.GetAccounts().contains(user_name)) {
        DeployIniErrorHandler(__func__, QString("The specified connection account() does not exist").arg(user_name),
                              online_dev);
        continue;
      }
      sync_account.SetDefaultSetting(false);
      sync_account.SetUsername(user_name);
      sync_account.SetPassword(user_account_table.GetAccounts()[user_name].GetPassword());
      update_account_queue_.enqueue(qMakePair(online_dev.GetId(), sync_account));

    } else {
      // Sync default account to the device's connection
      default_account.SetDefaultSetting(false);
      update_account_queue_.enqueue(qMakePair(online_dev.GetId(), default_account));
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
