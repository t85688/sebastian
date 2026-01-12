#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QSet>
#include <thread>

#include "act_auto_probe.hpp"
#include "act_core.hpp"

namespace act {
namespace core {

void ActCore::StartProbeDeviceProfileThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                            QList<ActScanIpRangeEntry> scan_ip_ranges) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Set default Connect Config

  // Account
  ActDeviceAccount account(ACT_DEFAULT_DEVICE_ACCOUNT_USERNAME, ACT_DEFAULT_DEVICE_ACCOUNT_PASSWORD);
  // NETCONF
  ActNetconfOverSSH netconf_over_ssh_config;
  ActNetconfConfiguration netconf_config(false, netconf_over_ssh_config);
  // SNMP
  ActSnmpConfiguration snmp_config(ACT_DEFAULT_SNMP_READ_COMMUNITY, ACT_DEFAULT_SNMP_WRITE_COMMUNITY);
  // RESTful
  ActRestfulConfiguration restful_config(ACT_DEFAULT_RESTFUL_PROTOCOL, ACT_DEFAULT_RESTFUL_PORT);
  ActDeviceConnectConfig default_connect_config(account, netconf_config, snmp_config, restful_config);

  // Check the input password, if it is empty would assign default value
  QList<ActScanIpRangeEntry> new_scan_ip_ranges;
  for (auto scan_ip_range_entry : scan_ip_ranges) {
    auto new_scan_ip_range_entry = scan_ip_range_entry;
    this->HandleConnectionConfigField(new_scan_ip_range_entry, default_connect_config, default_connect_config);
    new_scan_ip_ranges.append(new_scan_ip_range_entry);
  }

  // Trigger AutoProbe procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::auto_probe::ActAutoProbe auto_probe(profiles);

  act_status = auto_probe.StartAutoProbeByScanIpRange(new_scan_ip_ranges);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << __func__ << "System - Start AutoProbeByScanIpRange failed";
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartProbeDeviceProfile, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    // Write back the system status to the core memory
    this->SetSystemStatus(ActSystemStatusEnum::kIdle);
    return;
  }

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // Return each device's EnableSnmp result by stream
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = auto_probe.GetStatus();

    // Dequeue
    while (!auto_probe.result_queue_.isEmpty()) {
      act::auto_probe::ActAutoProbeResult auto_probe_result = auto_probe.result_queue_.dequeue();
      auto dev_profile_result = auto_probe_result.GetDeviceProfile();
      ActProbeDeviceResult probe_dev_result;

      // Success: upload DeviceProfile to system
      if (auto_probe_result.GetStatus() == ActStatusType::kSuccess) {
        // Upload device profile

        auto act_status_upload = act::core::g_core.UploadDeviceProfile(dev_profile_result);
        if (!IsActStatusSuccess(act_status_upload)) {  // failed
          qCritical() << __func__ << "Upload device profile failed";
          probe_dev_result = ActProbeDeviceResult(auto_probe_result.GetIpAddress(), dev_profile_result.GetModelName(),
                                                  dev_profile_result.GetId(), act_status_upload->GetStatus());
        } else {  // return last_assigned_device_profile_id
          probe_dev_result =
              ActProbeDeviceResult(auto_probe_result.GetIpAddress(), dev_profile_result.GetModelName(),
                                   this->last_assigned_device_profile_id_, auto_probe_result.GetStatus());
        }
      } else {
        // Failed or Skip: directly reply
        probe_dev_result = ActProbeDeviceResult(auto_probe_result.GetIpAddress(), dev_profile_result.GetModelName(),
                                                dev_profile_result.GetId(), auto_probe_result.GetStatus());
      }

      // WS response
      ActProbeDeviceResultWSResponse ws_resp(ActWSCommandEnum::kStartProbeDeviceProfile, ActStatusType::kRunning,
                                             probe_dev_result);

      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

      // Wait 0.1 second
      signal_receiver.wait_for(std::chrono::milliseconds(100));
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartProbeDeviceProfile, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }

      break;
    }
  }

  qDebug() << __func__ << "System thread job is going to close";
  bool finished_flag = false;
  if (act_status->GetStatus() == ActStatusType::kFinished) {
    finished_flag = true;
  } else {  // receive stop signal
    auto_probe.Stop();
  }

  // Write back the system status to the core memory
  this->SetSystemStatus(ActSystemStatusEnum::kIdle);

  // Send finished status reply to client
  if (finished_flag) {
    ActBaseResponse ws_resp(ActWSCommandEnum::kStartProbeDeviceProfile, ActStatusType::kFinished);
    qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
  }
  return;
}

ACT_STATUS ActCore::StartProbeDeviceProfile(const qint64 &ws_listener_id, QList<ActScanIpRangeEntry> &scan_ip_ranges) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check auto probe license
  /* if (!this->GetLicense().GetFeature().GetAutoProbe()) {
    QString error_msg = "The license does not support auto probe";
    qCritical() << __func__ << error_msg;

    act_status = std::make_shared<ActBadRequest>(error_msg);
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartProbeDeviceProfile, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  } */

  // Check system thread status
  if (this->GetSystemStatus() != ActSystemStatusEnum::kIdle) {
    QString error_msg = "The system has the process running";
    qCritical() << __func__ << error_msg;

    act_status = std::make_shared<ActBadRequest>(error_msg);
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartProbeDeviceProfile, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Check project threads pool, all status of the Websocket thread pool
  QMapIterator<qint64, std::pair<std::shared_ptr<std::promise<void>>, std::shared_ptr<std::thread>>> map_it(
      ws_thread_handler_pools);
  while (map_it.hasNext()) {
    map_it.next();
    auto project_id = map_it.key();

    // Get project by id
    ActProject project;
    act_status = this->GetProject(project_id, project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Get project failed with project id:" << project_id;
      std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
          ActWSResponseErrorTransfer(ActWSCommandEnum::kStartProbeDeviceProfile, *act_status);
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      return act_status;
    }

    // Check the exist threads are reasonable
    qDebug() << __func__ << "Project status:" << kActProjectStatusEnumMap.key(this->project_status_list[project_id]);
    if (this->project_status_list[project_id] != ActProjectStatusEnum::kFinished &&
        this->project_status_list[project_id] != ActProjectStatusEnum::kAborted) {
      QString error_msg = QString("The system has the project(%1) process running").arg(project.GetProjectName());
      qCritical() << __func__ << error_msg;

      act_status = std::make_shared<ActBadRequest>(error_msg);
      std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
          ActWSResponseErrorTransfer(ActWSCommandEnum::kStartProbeDeviceProfile, *act_status);

      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

      return act_status;
    }
  }

  // Update system status
  this->SetSystemStatus(ActSystemStatusEnum::kAutoProbing);

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << __func__ << "Spawn StartProbeDeviceProfile thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::StartProbeDeviceProfileThread, this,
                                             std::cref(ws_listener_id), std::move(signal_receiver), scan_ip_ranges);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"ProbeDeviceProfileThread";
  HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  // Set system thread handler to memory
  qDebug() << "Set system thread handler to memory";
  // Remove previous thread pointer
  if (system_promise_thread_pair_.second != nullptr && system_promise_thread_pair_.second->joinable()) {
    system_promise_thread_pair_.second->join();
  }
  this->system_promise_thread_pair_ = make_pair(signal_sender, thread_ptr);  // set to memory

  return act_status;
}

}  // namespace core
}  // namespace act
