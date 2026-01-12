#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QSet>
#include <thread>

#include "act_broadcast_search.hpp"
#include "act_core.hpp"
#include "act_deploy.hpp"
#include "act_manufacture_result.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::InitManufactureResult(qint64 &project_id, ActManufactureResult &manufacture_result) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = InitManufactureResult(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Init the ManufactureResult failed.";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  manufacture_result = project.GetManufactureResult();

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::InitManufactureResult(ActProject &project) {
  ACT_STATUS_INIT();

  QList<ActDevice> new_dev_list;
  for (auto dev : project.GetDevices()) {
    if (!dev.CheckCanDeploy()) {
      continue;
    }

    new_dev_list.append(dev);
  }

  // Init by device list
  project.GetManufactureResult().GetManufactureReport().clear();
  project.GetManufactureResult().GetReady().GetDevices().clear();
  project.GetManufactureResult().GetRemain().GetDevices().clear();

  for (auto dev : new_dev_list) {
    if (dev.CheckCanDeploy()) {
      ActManufactureResultDevice manufacture_dev(-1, dev);

      // Handle Ethernet Module
      for (auto slot : dev.GetModularConfiguration().GetEthernet().keys()) {
        ActDeviceSimpleEthernetModule simple_module;
        auto module_id = dev.GetModularConfiguration().GetEthernet()[slot];
        if (this->eth_modules_.contains(module_id)) {
          simple_module.GetModuleName() = this->eth_modules_[module_id].GetModuleName();
        }
        manufacture_dev.GetEthernetModule()[slot] = simple_module;
      }

      // Handle Power Module
      for (auto slot : dev.GetModularConfiguration().GetPower().keys()) {
        ActDeviceSimplePowerModule simple_module;
        auto module_id = dev.GetModularConfiguration().GetPower()[slot];
        if (this->power_modules_.contains(module_id)) {
          simple_module.GetModuleName() = this->power_modules_[module_id].GetModuleName();
        }
        manufacture_dev.GetPowerModule()[slot] = simple_module;
      }

      project.GetManufactureResult().GetRemain().GetDevices().append(manufacture_dev);
    }
  }

  std::sort(project.GetManufactureResult().GetRemain().GetDevices().begin(),
            project.GetManufactureResult().GetRemain().GetDevices().end());

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateManufactureResultOrder(qint64 &project_id, const QString &order,
                                                 ActManufactureResult &manufacture_result) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  project.GetManufactureResult().SetOrder(order);

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  manufacture_result = project.GetManufactureResult();

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateReManufactureDevices(qint64 &project_id, const QList<qint64> &dev_ids,
                                               ActManufactureResult &manufacture_result) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = UpdateReManufactureDevices(project, dev_ids);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update the Re-manufacture devices failed.";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  manufacture_result = project.GetManufactureResult();

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UpdateReManufactureDevices(ActProject &project, const QList<qint64> &dev_ids) {
  ACT_STATUS_INIT();

  for (auto dev_id : dev_ids) {
    for (auto &result_item : project.GetManufactureResult().GetManufactureReport()) {
      auto it = std::find_if(result_item.GetDevices().begin(), result_item.GetDevices().end(),
                             [&dev_id](const ActManufactureResultDevice &manufacture_dev) {
                               return manufacture_dev.GetDeviceId() == dev_id;
                             });

      if (it != result_item.GetDevices().end()) {  // found
        // Clear Order
        it->UpdateToRemainStatus();
        project.GetManufactureResult().GetRemain().GetDevices().append(*it);
        result_item.GetDevices().erase(it);
        break;
      }
    }
  }

  // Re-sort Remain's devices
  std::sort(project.GetManufactureResult().GetRemain().GetDevices().begin(),
            project.GetManufactureResult().GetRemain().GetDevices().end());

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetManufactureResult(qint64 &project_id, ActManufactureResult &manufacture_result) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Init manufacture_result_ when first execute mapper
  if (project.GetManufactureResult().CheckIsFirstTime()) {
    InitManufactureResult(project);
  }

  // Set Index
  qint64 dev_index = 0;
  for (auto &result_item : project.GetManufactureResult().GetManufactureReport()) {
    for (auto &result_dev : result_item.GetDevices()) {
      dev_index += 1;
      result_dev.SetIndex(dev_index);
    }
  }
  for (auto &remain_dev : project.GetManufactureResult().GetRemain().GetDevices()) {
    dev_index += 1;
    remain_dev.SetIndex(dev_index);
  }

  manufacture_result = project.GetManufactureResult();

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::GetTotalManufactureResult(qint64 &project_id, ActTotalManufactureResult &total_manufacture_result) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  qint64 dev_index = 0;
  for (auto result_item : project.GetManufactureResult().GetManufactureReport()) {
    if (result_item.GetStatus() != ActManufactureStatus::kSuccess) {
      continue;
    }

    for (auto result_dev : result_item.GetDevices()) {
      dev_index += 1;
      result_dev.SetIndex(dev_index);
      total_manufacture_result.GetTotalReport().append(result_dev);
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS UpdateManufactureResultByDeploy(ActProject &project, const QSet<qint64> &failed_dev_id_set) {
  ACT_STATUS_INIT();

  // Check all devices success
  if (failed_dev_id_set.isEmpty()) {
    // Check ready devices not empty
    auto ready_devices = project.GetManufactureResult().GetReady().GetDevices();
    if (!ready_devices.isEmpty()) {
      // Update device order
      for (auto &ready_device : ready_devices) {
        ready_device.SetOrder(project.GetManufactureResult().GetOrder());
      }

      // Add the ready_item to ManufactureReport
      qint64 batch_id = project.GetManufactureResult().GetManufactureReport().size() + 1;
      ActManufactureResultItem result_item(batch_id, ready_devices, ActManufactureStatus::kSuccess);
      result_item.SetTimeStamp(QDateTime::currentSecsSinceEpoch());
      project.GetManufactureResult().GetManufactureReport().append(result_item);

      // Remove the remain device when it deployed
      for (auto ready_dev : ready_devices) {
        if (project.GetManufactureResult().GetRemain().GetDevices().contains(ready_dev)) {
          project.GetManufactureResult().GetRemain().GetDevices().removeOne(ready_dev);
        }

        // Clear ready devices
        project.GetManufactureResult().GetReady().GetDevices().clear();
      }
    }
  }
  return ACT_STATUS_SUCCESS;
}

void ActCore::StartManufactureDeployIniThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                              qint64 project_id) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartManufactureDeploy, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }
  auto backup_project = project;

  // Get dev_id_list
  QList<qint64> dev_id_list;
  for (auto manufacture_dev : project.GetManufactureResult().GetReady().GetDevices()) {
    dev_id_list.append(manufacture_dev.GetDeviceId());
  }

  // Check device exist
  for (auto dev_id : dev_id_list) {
    ActDevice dev;
    act_status = project.GetDeviceById(dev, dev_id);  // find project's device by id
    if (IsActStatusNotFound(act_status)) {            // not found device
      qCritical() << __func__ << "Project has no device id:" << dev_id;

      std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
          ActWSResponseErrorTransfer(ActWSCommandEnum::kStartManufactureDeploy, *act_status);
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      return;
    }
  }

  // TODO: temporary skip check
  // // Check map[deviceId] has value (generated offline config)
  // act_status = CheckDeviceConfigValuesNotEmpty(project, dev_id_list, device_offline_config_file_map);
  // if (!IsActStatusSuccess(act_status)) {
  //   std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
  //       ActWSResponseErrorTransfer(ActWSCommandEnum::kStartManufactureDeploy, *act_status);
  //   this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
  //   return;
  // }

  // Trigger deploy procedure
  QMap<QString, QString> mac_host_map;
  this->GetMacHostMap(mac_host_map);
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::deploy::ActDeploy deployer(profiles, mac_host_map);

  bool skip_mapping_dev = false;
  act_status = deployer.StartIniDeployer(project, dev_id_list, skip_mapping_dev);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start ini deployer failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartManufactureDeploy, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kDeploying;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = deployer.GetStatus();

    // Dequeue
    while (!deployer.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = deployer.result_queue_.dequeue();
      ActDeviceConfigResultWSResponse ws_resp(ActWSCommandEnum::kStartManufactureDeploy, ActStatusType::kRunning,
                                              dev_config_result);
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartManufactureDeploy, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }

      break;
    } else {  // update pure progress
      ActProgressWSResponse ws_resp(ActWSCommandEnum::kStartManufactureDeploy,
                                    dynamic_cast<ActProgressStatus &>(*act_status));
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    }
  }

  qDebug() << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    deployer.Stop();
    qCritical() << project.GetProjectName() << "Abort deploy ini";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // [feat:793] Update the stream status of all streams to scheduled (802.1Qdj)
  act_status = this->UpdateAllStreamStatus(project, ActStreamStatusEnum::kScheduled);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the stream status";
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartManufactureDeploy, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  // Update Manufacture result by deploy
  UpdateManufactureResultByDeploy(project, deployer.failed_device_id_set_);

  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartManufactureDeploy, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << project.GetProjectName() << "Cannot update the backup project";
    }
    return;
  }

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartManufactureDeploy, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartManufactureDeployIni(qint64 &project_id, const qint64 &ws_listener_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeploying, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartManufactureDeploy, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << project_name << "Spawn Manufacture deploy ini thread...";
  std::shared_ptr<std::thread> thread_ptr;

  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::StartManufactureDeployIniThread, this,
                                             std::cref(ws_listener_id), std::move(signal_receiver), project_id);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartManufactureDeployIniThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

}  // namespace core
}  // namespace act
