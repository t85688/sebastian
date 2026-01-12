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
#include "act_topology_mapping.hpp"

namespace act {
namespace core {

void ActCore::StartTopologyMappingThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                         qint64 project_id, qint64 design_baseline_id) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get the design baseline
  ActNetworkBaseline design_baseline;
  act_status = this->GetDesignBaseline(project_id, design_baseline_id, design_baseline);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << QString("Get the DesignBaseline failed with project_id: %1 & design_baseline_id: %2")
                       .arg(project_id)
                       .arg(design_baseline_id);

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartTopologyMapping, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
    return;
  }
  auto project = design_baseline.GetProject();

  // Clear DeviceConfig's MappingDeviceIpSettingTables
  project.GetDeviceConfig().GetMappingDeviceIpSettingTables().clear();

  // Trigger topology_mapping procedure
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());
  act::topology::ActTopologyMapping topology_mapping(profiles);

  ActTopologyMappingResult mapping_result;
  act_status = topology_mapping.Start(project, mapping_result);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start topology_mapping failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartTopologyMapping, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kTopologyMapping;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = topology_mapping.GetStatus();

    if (act_status->GetStatus() != ActStatusType::kFinished) {
      if (act_status->GetStatus() != ActStatusType::kRunning) {  // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartTopologyMapping, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

      } else {
        ActProgressWSResponse ws_resp(ActWSCommandEnum::kStartTopologyMapping,
                                      dynamic_cast<ActProgressStatus &>(*act_status));
        qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
        this->SendMessageToWSListener(ws_listener_id, ws_resp.ToString(ws_resp.key_order_).toStdString().c_str());
      }
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }

  qInfo() << project.GetProjectName() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    topology_mapping.Stop();
    qCritical() << project.GetProjectName() << "Abort topology mapping";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Write back the project to design baseline & update to core memory
  design_baseline.SetProject(project);
  act_status = this->UpdateDesignBaseline(project_id, design_baseline);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the DesignBaseline. baseline_id:" << design_baseline_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartTopologyMapping, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

    return;
  }

  // Send finished status reply to client
  // Update core mac_host_map
  SetMacHostMap(topology_mapping.GetMACHostMap());

  ActTopologyMappingResultWSResponse ws_resp(ActWSCommandEnum::kStartTopologyMapping, mapping_result);
  qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToWSListener(ws_listener_id, ws_resp.ToString(ws_resp.key_order_).toStdString().c_str());

  return;
}

ACT_STATUS ActCore::StartTopologyMapping(qint64 &project_id, qint64 &design_baseline_id, const qint64 &ws_listener_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kTopologyMapping, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartTopologyMapping, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << project_name << "Spawn topology mapping thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartTopologyMappingThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, design_baseline_id);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartTopologyMappingThread";
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
