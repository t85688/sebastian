#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QSet>
#include <thread>

#include "act_device_configuration.hpp"

namespace act {
namespace core {

void ActCore::StartExportDeviceConfigThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                            qint64 project_id, QList<qint64> dev_id_list) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  bool is_operation = true;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get Operation project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartExportDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  // Clear DeviceConfig folder
  // kene+
  /*
  act_status = ClearFolder(ACT_DEVICE_CONFIG_FILE_FOLDER);
  */
  QString deviceConfigFilePath = GetDeviceConfigFilePath();
  act_status = ClearFolder(deviceConfigFilePath);
  // kene-
  if (!IsActStatusSuccess(act_status)) {
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartDeploy, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  // Trigger Export procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  // kene+
  /*
  QString path = QString(ACT_DEVICE_CONFIG_FILE_FOLDER);
  */
  QString path = QString(deviceConfigFilePath);
  // kene-
  act_status = device_configuration.StartExportConfig(project, dev_id_list, path);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start Export config failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartExportDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's Export result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Dequeue
    while (!device_configuration.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

      ActDeviceConfigResultWSResponse ws_resp(ActWSCommandEnum::kStartExportDeviceConfig, ActStatusType::kRunning,
                                              dev_config_result);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartExportDeviceConfig, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort Export Config";

    return;
  }

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartExportDeviceConfig, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartExportDeviceConfig(qint64 &project_id, const qint64 &ws_listener_id,
                                            QList<qint64> &dev_id_list) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support the Export";
    qCritical() << __func__ << error_msg;

    act_status = std::make_shared<ActLicenseNotActiveFailed>("DeviceConfig");
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartExportDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartExportDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << project_name << "Spawn Export thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartExportDeviceConfigThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, dev_id_list);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartExportDeviceConfigThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartImportDeviceConfigThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                            qint64 project_id, QList<qint64> dev_id_list) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  bool is_operation = true;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get Operation project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartImportDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  ActDevice device;
  act_status = project.GetDeviceById(device, dev_id_list.first());
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get device failed with device id:" << dev_id_list.first();

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartImportDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  // Trigger Import procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  QString file_path = QString("%1/%2_%3.ini")
                          // kene+
                          /*
                          .arg(ACT_DEVICE_CONFIG_FILE_FOLDER)
                          */
                          .arg(GetDeviceConfigFilePath())
                          // kene-
                          .arg(project.GetProjectName())
                          .arg(device.GetIpv4().GetIpAddress());
  act_status = device_configuration.StartImportConfig(project, dev_id_list, file_path);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start Import config failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartImportDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's Import result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Dequeue
    while (!device_configuration.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

      ActDeviceConfigResultWSResponse ws_resp(ActWSCommandEnum::kStartImportDeviceConfig, ActStatusType::kRunning,
                                              dev_config_result);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartImportDeviceConfig, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort Import Config";

    return;
  }

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartImportDeviceConfig, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartImportDeviceConfig(qint64 &project_id, const qint64 &ws_listener_id,
                                            QList<qint64> &dev_id_list) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support the Import";
    qCritical() << __func__ << error_msg;

    act_status = std::make_shared<ActLicenseNotActiveFailed>("DeviceConfig");
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartImportDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartImportDeviceConfig, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << project_name << "Spawn Import thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartImportDeviceConfigThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, dev_id_list);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartImportDeviceConfigThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::OpcUaStartExportDeviceConfigThread(qint64 project_id, QList<qint64> dev_id_list, QString export_path,
                                                 std::future<void> signal_receiver,
                                                 void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    cb_func(act_status, arg);
    return;
  }

  // Trigger Export procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartExportConfig(project, dev_id_list, export_path);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start Export config failed";

    cb_func(act_status, arg);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kDeviceConfiguring;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's Export result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Wait 0.1 second
    signal_receiver.wait_for(std::chrono::milliseconds(100));

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort Export Config";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  // Dequeue
  while (!device_configuration.result_queue_.isEmpty()) {
    ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

    act_status->SetErrorMessage(dev_config_result.GetErrorMessage());
    act_status->SetStatus(dev_config_result.GetStatus());
    cb_func(act_status, arg);
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  return;
}

ACT_STATUS ActCore::OpcUaStartExportDeviceConfig(qint64 &project_id, QList<qint64> &dev_id_list, QString &export_path,
                                                 std::future<void> signal_receiver,
                                                 void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support the Export";
    qCritical() << __func__ << error_msg;

    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << __func__ << project_name << "Spawn OPC UA Export thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartExportDeviceConfigThread, this, project_id,
                                             dev_id_list, export_path, std::move(signal_receiver), cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartExportDeviceConfigThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::OpcUaStartImportDeviceConfigThread(qint64 project_id, QList<qint64> dev_id_list, QString import_path,
                                                 std::future<void> signal_receiver,
                                                 void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    cb_func(act_status, arg);
    return;
  }

  ActDevice device;
  act_status = project.GetDeviceById(device, dev_id_list.first());
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get device failed with device id:" << dev_id_list.first();
    cb_func(act_status, arg);
    return;
  }

  // Trigger Import procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartImportConfig(project, dev_id_list, import_path);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start Import config failed";
    cb_func(act_status, arg);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kDeviceConfiguring;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's Import result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Wait 0.1 second
    signal_receiver.wait_for(std::chrono::milliseconds(100));

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort Import Config";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  // Dequeue
  while (!device_configuration.result_queue_.isEmpty()) {
    ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

    act_status->SetErrorMessage(dev_config_result.GetErrorMessage());
    act_status->SetStatus(dev_config_result.GetStatus());
    cb_func(act_status, arg);
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  return;
}

ACT_STATUS ActCore::OpcUaStartImportDeviceConfig(qint64 &project_id, QList<qint64> &dev_id_list, QString &import_path,
                                                 std::future<void> signal_receiver,
                                                 void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support the Import";
    qCritical() << __func__ << error_msg;

    act_status = std::make_shared<ActLicenseNotActiveFailed>("DeviceConfig");
    return act_status;
  } */

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kDeviceConfiguring, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << __func__ << project_name << "Spawn OPC UA Import thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartImportDeviceConfigThread, this, project_id,
                                             dev_id_list, import_path, std::move(signal_receiver), cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartImportDeviceConfigThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Insert thread handler to pools
  qDebug() << __func__ << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

}  // namespace core
}  // namespace act
