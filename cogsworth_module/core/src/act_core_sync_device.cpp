#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QSet>
#include <thread>

#include "act_core.hpp"
#include "act_sync.hpp"

namespace act {
namespace core {

void ActCore::StartSyncDevicesThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id,
                                     QList<qint64> dev_id_list) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartSyncDevices, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }
  auto backup_project = project;

  // Trigger deploy procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::topology::ActSync sync(profiles);

  act_status = sync.Start(project, dev_id_list);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start sync failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartSyncDevices, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kSyncing;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = sync.GetStatus();

    // Dequeue
    while (!sync.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = sync.result_queue_.dequeue();
      ActDeviceConfigResultWSResponse ws_resp(ActWSCommandEnum::kStartSyncDevices, ActStatusType::kRunning,
                                              dev_config_result);
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartSyncDevices, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }

      break;
    } else {  // update pure progress
      ActProgressWSResponse ws_resp(ActWSCommandEnum::kStartSyncDevices,
                                    dynamic_cast<ActProgressStatus &>(*act_status));
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    }
  }

  qDebug() << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    sync.Stop();
    qCritical() << project.GetProjectName() << "Abort deploy ini";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartSyncDevices, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << project.GetProjectName() << "Cannot update the backup project";
    }
    return;
  }

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartSyncDevices, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToWSListener(ws_listener_id, ws_resp.ToString(ws_resp.key_order_).toStdString().c_str());

  return;
}

ACT_STATUS ActCore::StartSyncDevices(qint64 &project_id, const qint64 &ws_listener_id,
                                     const QList<qint64> &dev_id_list) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kSyncing, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartSyncDevices, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << project_name << "Spawn sync devices thread...";
  std::shared_ptr<std::thread> thread_ptr;

  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartSyncDevicesThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, dev_id_list);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartSyncDevicesThread";
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

void ActCore::OpcUaStartSyncDevicesThread(qint64 project_id, QList<qint64> dev_id_list,
                                          std::future<void> signal_receiver,
                                          void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    cb_func(act_status, arg);
    return;
  }
  auto backup_project = project;

  // Trigger deploy procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::topology::ActSync sync(profiles);

  act_status = sync.Start(project, dev_id_list);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start sync failed";
    cb_func(act_status, arg);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kSyncing;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = sync.GetStatus();

    // Wait 0.1 second
    signal_receiver.wait_for(std::chrono::milliseconds(100));

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    sync.Stop();
    qCritical() << project.GetProjectName() << "Abort deploy ini";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  // Dequeue
  while (!sync.result_queue_.isEmpty()) {
    ActDeviceConfigureResult dev_config_result = sync.result_queue_.dequeue();

    act_status->SetErrorMessage(dev_config_result.GetErrorMessage());
    act_status->SetStatus(dev_config_result.GetStatus());
    cb_func(act_status, arg);
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    cb_func(act_status, arg);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      cb_func(act_status, arg);
    }
    return;
  }

  return;
}

ACT_STATUS ActCore::OpcUaStartSyncDevices(qint64 &project_id, const QList<qint64> &dev_id_list,
                                          std::future<void> signal_receiver,
                                          void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kSyncing, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << project_name << "Spawn OPC UA sync devices thread...";
  std::shared_ptr<std::thread> thread_ptr;

  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartSyncDevicesThread, this, project_id,
                                             dev_id_list, std::move(signal_receiver), cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartSyncDevicesThread";
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
