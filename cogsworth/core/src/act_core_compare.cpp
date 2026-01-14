#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QSet>
#include <thread>

#include "act_compare.hpp"
#include "act_core.hpp"

namespace act {
namespace core {

void ActCore::OpcUaStartCompareThread(qint64 project_id, std::future<void> signal_receiver,
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
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }
  auto backup_project = project;
  // Trigger compare procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::topology::ActCompare comparer(profiles);
  act::topology::ActCompareControl compare_ctrl;
  compare_ctrl.SetTopologyConsistent(false);
  compare_ctrl.SetInterfaceStatus(false);
  compare_ctrl.SetPropagationDelay(false);

  QList<qint64> dev_id_list;
  // Generate the device_id_list when that is empty
  if (dev_id_list.isEmpty()) {
    for (auto dev : project.GetDevices()) {
      // Check device can be deploy by device type
      if (ActDevice::CheckDeviceCanBeDeploy(dev)) {
        dev_id_list.append(dev.GetId());
      }
    }
  }

  act_status = comparer.Start(project, dev_id_list, compare_ctrl);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start compare failed";
    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kComparing;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = comparer.GetStatus();

    if (act_status->GetStatus() != ActStatusType::kFinished) {
      qDebug() << act_status->ToString().toStdString().c_str();
      cb_func(act_status, arg);
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    comparer.Stop();
    qCritical() << project.GetProjectName() << "Abort compare";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Write back the project status to the core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";
    cb_func(act_status, arg);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << project.GetProjectName() << "Cannot update the backup project";
    }

    return;
  }

  // Send finished status reply to client
  act_status = comparer.GetStatus();
  cb_func(act_status, arg);

  return;
}

ACT_STATUS ActCore::OpcUaStartCompare(qint64 &project_id, std::future<void> signal_receiver,
                                      void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kComparing, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << project_name << "Spawn OPC UA compare thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartCompareThread, this, project_id,
                                             std::move(signal_receiver), cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartCompareThread";
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

void ActCore::StartCompareThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartCompare, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }
  auto backup_project = project;
  // Trigger compare procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::topology::ActCompare comparer(profiles);
  act::topology::ActCompareControl compare_ctrl;
  compare_ctrl.SetTopologyConsistent(false);
  compare_ctrl.SetInterfaceStatus(false);
  compare_ctrl.SetPropagationDelay(false);

  QList<qint64> dev_id_list;
  // Generate the device_id_list when that is empty
  if (dev_id_list.isEmpty()) {
    if (dev_id_list.isEmpty()) {
      for (auto dev : project.GetDevices()) {
        // Check device can be compared by device type
        if (ActDevice::CheckDeviceCanBeDeploy(dev)) {
          dev_id_list.append(dev.GetId());
        }
      }
    }
  }

  act_status = comparer.Start(project, dev_id_list, compare_ctrl);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start compare failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartCompare, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kComparing;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = comparer.GetStatus();

    if (act_status->GetStatus() != ActStatusType::kFinished) {
      if (act_status->GetStatus() != ActStatusType::kRunning) {  // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartCompare, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      } else {
        ActProgressWSResponse ws_resp(ActWSCommandEnum::kStartCompare, dynamic_cast<ActProgressStatus &>(*act_status));
        qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    comparer.Stop();
    qCritical() << project.GetProjectName() << "Abort compare";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Send finished status reply to client
  ActProgressStatus finished_status;
  finished_status.SetProgress(100);
  finished_status.SetStatus(ActStatusType::kFinished);

  ActProgressWSResponse ws_resp(ActWSCommandEnum::kStartCompare, finished_status);
  qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartCompare(qint64 &project_id, const qint64 &ws_listener_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kComparing, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartCompare, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << project_name << "Spawn compare thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::StartCompareThread, this, std::cref(ws_listener_id),
                                             std::move(signal_receiver), project_id);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartCompareThread";
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
