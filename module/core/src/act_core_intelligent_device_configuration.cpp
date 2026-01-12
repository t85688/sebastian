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

void ActCore::StartIntelligentConfigImportThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                                 qint64 project_id, QList<QString> dev_ip_list, QString path) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    // Send failed reply to client
    ActIntelligentResponse sys_response;
    sys_response.Getresponse().Setrole("Assistant");
    sys_response.Getresponse().Getreply().Setreply("An error occurred, please try again.");
    sys_response.Getresponse().Getreply().Settoolbar(false);
    sys_response.Getstatus().Setcode(400);
    sys_response.Getstatus().Setmessage("OK");
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, sys_response),
                                ws_listener_id);
    // Reply finish
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActBaseResponse(ActWSCommandEnum::kStartIntelligentRequest, ActStatusType::kFinished),
                                ws_listener_id);
    return;
  }

  // Create dev_id_list
  QList<qint64> dev_id_list;
  QMap<qint64, QString> dev_id_ip_map;  // <DeviceID, DeviceIP>
  // Create device list from Project's devices
  for (auto dev_ip : dev_ip_list) {
    ActDevice dev;
    act_status = project.GetDeviceByIp(dev_ip, dev);  // find project's device by id
    if (IsActStatusNotFound(act_status)) {            // not found device
      qCritical() << __func__ << "Project has no device IP:" << dev_ip;
      // Send device failed result to client
      ActIntelligentResponse sys_response;
      sys_response.Getresponse().Setrole("Assistant");
      sys_response.Getresponse().Getreply().Setreply(QString("Backup %1 %2.").arg(dev_ip).arg("failed"));
      sys_response.Getresponse().Getreply().Settoolbar(false);
      sys_response.Getstatus().Setcode(400);
      sys_response.Getstatus().Setmessage("OK");

      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                  ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                   ActStatusType::kRunning, sys_response),
                                  ws_listener_id);
      continue;
    }
    dev_id_ip_map[dev.GetId()] = dev_ip;
    dev_id_list.append(dev.GetId());
  }

  // Trigger Import procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartImportConfig(project, dev_id_list, path);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start Intelligent Import config failed";

    // Send failed reply to client
    ActIntelligentResponse sys_response;
    sys_response.Getresponse().Setrole("Assistant");
    sys_response.Getresponse().Getreply().Setreply("An error occurred, please try again.");
    sys_response.Getresponse().Getreply().Settoolbar(false);
    sys_response.Getstatus().Setcode(400);
    sys_response.Getstatus().Setmessage("OK");
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, sys_response),
                                ws_listener_id);
    // Reply finish
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActBaseResponse(ActWSCommandEnum::kStartIntelligentRequest, ActStatusType::kFinished),
                                ws_listener_id);

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kDeviceConfiguring;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's Import result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Dequeue
    while (!device_configuration.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

      // Send device result status reply to client
      QString result_status_str = dev_config_result.GetStatus() == ActStatusType::kSuccess ? "success" : "failed";
      ActIntelligentResponse sys_response;
      sys_response.Getresponse().Setrole("Assistant");
      sys_response.Getresponse().Getreply().Setreply(
          QString("Restore %1 %2.").arg(dev_id_ip_map[dev_config_result.GetId()]).arg(result_status_str));
      sys_response.Getresponse().Getreply().Settoolbar(false);
      sys_response.Getstatus().Setcode(400);
      sys_response.Getstatus().Setmessage("OK");

      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                  ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                   ActStatusType::kRunning, sys_response),
                                  ws_listener_id);

      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
      }
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // Call abort
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort Export Config";
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    // Send last status reply to client
    ActIntelligentResponse sys_response;
    sys_response.Getresponse().Setrole("Assistant");
    sys_response.Getresponse().Getreply().Setreply("Restore task was aborted.");
    sys_response.Getresponse().Getreply().Settoolbar(false);
    sys_response.Getstatus().Setcode(200);
    sys_response.Getstatus().Setmessage("OK");
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, sys_response),
                                ws_listener_id);

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Send finished status reply to client
  ActIntelligentResponse sys_response;
  sys_response.Getresponse().Setrole("Assistant");
  sys_response.Getresponse().Getreply().Setreply("Restore task completed.");
  sys_response.Getresponse().Getreply().Settoolbar(false);
  sys_response.Getstatus().Setcode(200);
  sys_response.Getstatus().Setmessage("OK");
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                              ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                               ActStatusType::kRunning, sys_response),
                              ws_listener_id);

  // Send finished status reply to client
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                              ActBaseResponse(ActWSCommandEnum::kStartIntelligentRequest, ActStatusType::kFinished),
                              ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartIntelligentConfigImport(qint64 &project_id, const qint64 &ws_listener_id,
                                                 QList<QString> &dev_ip_list, QString &path) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support Intelligent Import";
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

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << project_name << "Spawn Intelligent ConfigImport thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::StartIntelligentConfigImportThread, this,
                                             std::cref(ws_listener_id), std::move(signal_receiver), project_id,
                                             dev_ip_list, path);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartIntelligentConfigImportThread";
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

void ActCore::StartIntelligentConfigExportThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                                 qint64 project_id, QList<QString> dev_ip_list, QString path) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    // Send failed reply to client
    ActIntelligentResponse sys_response;
    sys_response.Getresponse().Setrole("Assistant");
    sys_response.Getresponse().Getreply().Setreply("An error occurred, please try again.");
    sys_response.Getresponse().Getreply().Settoolbar(false);
    sys_response.Getstatus().Setcode(400);
    sys_response.Getstatus().Setmessage("OK");
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, sys_response),
                                ws_listener_id);
    // Reply finish
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActBaseResponse(ActWSCommandEnum::kStartIntelligentRequest, ActStatusType::kFinished),
                                ws_listener_id);
    return;
  }

  // Create dev_id_list
  QList<qint64> dev_id_list;
  QMap<qint64, QString> dev_id_ip_map;  // <DeviceID, DeviceIP>
  if (dev_ip_list.isEmpty()) {          // Add all project devices when it FeatureGroup(ImportExport) is true
    for (auto dev : project.GetDevices()) {
      if (dev.GetDeviceProperty().GetFeatureGroup().GetOperation().GetImportExport()) {
        dev_id_ip_map[dev.GetId()] = dev.GetIpv4().GetIpAddress();
        dev_id_list.append(dev.GetId());
      }
    }
  } else {
    // Create device list from Project's devices
    for (auto dev_ip : dev_ip_list) {
      ActDevice dev;
      act_status = project.GetDeviceByIp(dev_ip, dev);  // find project's device by id
      if (IsActStatusNotFound(act_status)) {            // not found device
        qCritical() << __func__ << "Project has no device IP:" << dev_ip;
        // Send device failed result to client
        ActIntelligentResponse sys_response;
        sys_response.Getresponse().Setrole("Assistant");
        sys_response.Getresponse().Getreply().Setreply(QString("Backup %1 %2.").arg(dev_ip).arg("failed"));
        sys_response.Getresponse().Getreply().Settoolbar(false);
        sys_response.Getstatus().Setcode(400);
        sys_response.Getstatus().Setmessage("OK");

        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                    ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                     ActStatusType::kRunning, sys_response),
                                    ws_listener_id);
        continue;
      }
      dev_id_ip_map[dev.GetId()] = dev_ip;
      dev_id_list.append(dev.GetId());
    }
  }

  // Trigger Export procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  ActDeviceConfiguration device_configuration(profiles);
  act_status = device_configuration.StartExportConfig(project, dev_id_list, path);

  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << project.GetProjectName() << "Start Intelligent Export config failed";
    // Send failed reply to client
    ActIntelligentResponse sys_response;
    sys_response.Getresponse().Setrole("Assistant");
    sys_response.Getresponse().Getreply().Setreply("An error occurred, please try again.");
    sys_response.Getresponse().Getreply().Settoolbar(false);
    sys_response.Getstatus().Setcode(400);
    sys_response.Getstatus().Setmessage("OK");
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, sys_response),
                                ws_listener_id);

    // Reply finish
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActBaseResponse(ActWSCommandEnum::kStartIntelligentRequest, ActStatusType::kFinished),
                                ws_listener_id);

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kDeviceConfiguring;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's Export result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = device_configuration.GetStatus();

    // Dequeue
    while (!device_configuration.result_queue_.isEmpty()) {
      ActDeviceConfigureResult dev_config_result = device_configuration.result_queue_.dequeue();

      // Send device result status reply to client
      QString result_status_str = dev_config_result.GetStatus() == ActStatusType::kSuccess ? "success" : "failed";
      ActIntelligentResponse sys_response;
      sys_response.Getresponse().Setrole("Assistant");
      sys_response.Getresponse().Getreply().Setreply(
          QString("Backup %1 %2.").arg(dev_id_ip_map[dev_config_result.GetId()]).arg(result_status_str));
      sys_response.Getresponse().Getreply().Settoolbar(false);
      sys_response.Getstatus().Setcode(400);
      sys_response.Getstatus().Setmessage("OK");

      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                  ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                   ActStatusType::kRunning, sys_response),
                                  ws_listener_id);

      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
      }
      break;
    }
  }

  qDebug() << __func__ << project.GetProjectName() << "Thread is going to close";

  // Call abort
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    device_configuration.Stop();
    qCritical() << project.GetProjectName() << "Abort Export Config";
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    // Send last status reply to client
    ActIntelligentResponse sys_response;
    sys_response.Getresponse().Setrole("Assistant");
    sys_response.Getresponse().Getreply().Setreply("Backup task was aborted.");
    sys_response.Getresponse().Getreply().Settoolbar(false);
    sys_response.Getstatus().Setcode(200);
    sys_response.Getstatus().Setmessage("OK");
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, sys_response),
                                ws_listener_id);

    return;
  }

  // Normal finished

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;
  // Send last status reply to client
  ActIntelligentResponse sys_response;
  sys_response.Getresponse().Setrole("Assistant");
  sys_response.Getresponse().Getreply().Setreply("Backup task completed.");
  sys_response.Getresponse().Getreply().Settoolbar(false);
  sys_response.Getstatus().Setcode(200);
  sys_response.Getstatus().Setmessage("OK");
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                              ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                               ActStatusType::kRunning, sys_response),
                              ws_listener_id);

  // Send finished status reply to client
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                              ActBaseResponse(ActWSCommandEnum::kStartIntelligentRequest, ActStatusType::kFinished),
                              ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartIntelligentConfigExport(qint64 &project_id, const qint64 &ws_listener_id,
                                                 QList<QString> &dev_ip_list, QString &path) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check device config license
  /* if (!this->GetLicense().GetFeature().GetDeviceConfig().GetEnabled()) {
    QString error_msg = "The license does not support Intelligent Export";
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

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << project_name << "Spawn Intelligent ConfigExport thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::StartIntelligentConfigExportThread, this,
                                             std::cref(ws_listener_id), std::move(signal_receiver), project_id,
                                             dev_ip_list, path);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartIntelligentConfigExportThread";
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
