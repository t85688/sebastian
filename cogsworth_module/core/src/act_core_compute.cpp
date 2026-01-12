#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QSet>
#include <QStack>
#include <thread>

#include "act_algorithm.hpp"
#include "act_core.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::GetComputedResult(qint64 &project_id, ActComputedResult &computed_rlt) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  computed_rlt = project.GetComputedResult();

  return act_status;
}

ACT_STATUS ActCore::DeleteComputedResult(qint64 &project_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActComputedResult empty_rlt;
  project.SetComputedResult(empty_rlt);

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActComputedResultPatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, project_id, empty_rlt, true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project_id);

  return act_status;
}

ACT_STATUS ActCore::ComputeTopologySetting(ActProject &project) {
  ACT_STATUS_INIT();

  ActDeviceConfig &device_config = project.GetDeviceConfig();
  for (qint64 device_id : device_config.GetVlanTables().keys()) {
    ActDevice device;
    act_status = project.GetDeviceById(device, device_id);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    ActVlanTable &vlan_table = device_config.GetVlanTables()[device_id];

    QSet<ActVlanStaticEntry> vlan_static_entries;
    for (ActVlanStaticEntry vlan_static_entry : vlan_table.GetVlanStaticEntries()) {
      if ((vlan_static_entry.GetVlanPriority() == ActVlanPriorityEnum::kNonTSN) ||
          device.GetDeviceProperty().GetReservedVlan().contains(vlan_static_entry.GetVlanId())) {
        vlan_static_entries.insert(vlan_static_entry);
      }
    }
    vlan_table.SetVlanStaticEntries(vlan_static_entries);

    QSet<ActPortVlanEntry> port_vlan_entries;
    for (ActPortVlanEntry port_vlan_entry : vlan_table.GetPortVlanEntries()) {
      if (port_vlan_entry.GetVlanPriority() != ActVlanPriorityEnum::kNonTSN) {
        port_vlan_entry.SetPVID(ACT_VLAN_INIT_PVID);
        port_vlan_entry.SetVlanPriority(ActVlanPriorityEnum::kNonTSN);
      }
      port_vlan_entries.insert(port_vlan_entry);
    }
    vlan_table.SetPortVlanEntries(port_vlan_entries);

    QSet<ActVlanPortTypeEntry> vlan_port_type_entries;
    for (ActVlanPortTypeEntry vlan_port_type_entry : vlan_table.GetVlanPortTypeEntries()) {
      if (vlan_port_type_entry.GetVlanPriority() != ActVlanPriorityEnum::kNonTSN) {
        vlan_port_type_entry.SetVlanPortType(ActVlanPortTypeEnum::kAccess);
        vlan_port_type_entry.SetVlanPriority(ActVlanPriorityEnum::kNonTSN);
      }
      vlan_port_type_entries.insert(vlan_port_type_entry);
    }
    vlan_table.SetVlanPortTypeEntries(vlan_port_type_entries);
  }

  // update vlan group
  // act_status = this->ComputeVlanConfig(project);
  // if (!IsActStatusSuccess(act_status)) {
  //   qCritical() << "Compute VLAN config failed";
  //   return act_status;
  // }

  // update redundant group
  act_status = this->ComputeRedundantSwift(project);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  if (!project.GetTopologySetting().GetIntelligentVlanGroup().isEmpty() ||
      !project.GetTopologySetting().GetRedundantGroup().GetRSTP().isEmpty()) {
    for (ActDevice device : project.GetDevices()) {
      // Send device update msg to temp
      InsertDeviceMsgToNotificationTmp(
          ActDevicePatchUpdateMsg(ActPatchUpdateActionEnum::kUpdate, project.GetId(), device, true));
    }

    for (ActLink link : project.GetLinks()) {
      // Send link update msg to temp
      InsertLinkMsgToNotificationTmp(
          ActLinkPatchUpdateMsg(ActPatchUpdateActionEnum::kUpdate, project.GetId(), link, true));
    }
  }

  return ACT_STATUS_SUCCESS;
}

void ActCore::OpcUaStartComputeThread(qint64 project_id, std::future<void> signal_receiver,
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
  // Trigger compute procedure
  ActAlgorithm computer(project.GetProjectName());

  act_status = computer.Start(project);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start compute failed";
    cb_func(act_status, arg);
    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kComputing;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = computer.GetStatus();

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
    computer.Stop();
    qCritical() << project.GetProjectName() << "Abort compute";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

    cb_func(ACT_STATUS_STOP, arg);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // [bugfix:2601] OPC UA - After Compute finished need to generate the DeviceConfig
  // Generate the DeviceConfig
  ActDeviceConfig device_config;
  this->deploy_available[project_id] = false;
  act_status = this->GenerateDeviceConfigByComputeResult(project, device_config);
  if (IsActStatusSuccess(act_status)) {
    // Update DeviceConfig
    act_status = this->UpdateDeviceConfig(project, device_config);
    if (IsActStatusSuccess(act_status)) {
      this->deploy_available[project_id] = true;
      // Send update msg
      ActDeviceConfigPatchUpdateMsg msg_device(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetDeviceConfig(),
                                               true);
      this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg_device, project_id);

      ActComputedResultPatchUpdateMsg msg_compute_result(ActPatchUpdateActionEnum::kUpdate, project_id,
                                                         project.GetComputedResult(), false);
      this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg_compute_result, project_id);
    } else {
      qCritical() << "Update DeviceConfig failed with project id:" << project.GetId();
    }
  } else {
    qCritical() << "GenerateDeviceConfigByComputeResult() failed.";
  }

  // [bugfix:3196] project 計算成功後沒辦法按 deploy
  // Send Websocket features available status msg
  ActFeaturesAvailableStatusResult available_result(CanUndoProject(project_id), CanRedoProject(project_id),
                                                    CanDeployProject(project_id));
  ActFeaturesAvailableStatusWSResponse ws_features_available_status_msg(available_result);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_features_available_status_msg, project_id);

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

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  // Send finished status reply to client
  act_status = computer.GetStatus();
  cb_func(act_status, arg);

  return;
}

ACT_STATUS ActCore::OpcUaStartCompute(qint64 &project_id, std::future<void> signal_receiver,
                                      void (*cb_func)(ACT_STATUS status, void *arg), void *arg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kComputing, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  qDebug() << project_name << "Spawn OPC UA compute thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::OpcUaStartComputeThread, this, project_id,
                                             std::move(signal_receiver), cb_func, arg);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"OpcUaStartComputeThread";
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

void ActCore::StartComputeThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartCompute, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  auto backup_project = project;
  // Trigger compute procedure
  ActAlgorithm computer(project.GetProjectName());

  act_status = computer.Start(project);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start compute failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartCompute, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kComputing;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    act_status = computer.GetStatus();
    if (act_status->GetStatus() != ActStatusType::kFinished) {
      if (act_status->GetStatus() != ActStatusType::kRunning) {  // error

        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartCompute, *act_status);
        qDebug() << __func__ << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      } else {
        ActProgressWSResponse ws_resp(ActWSCommandEnum::kStartCompute, dynamic_cast<ActProgressStatus &>(*act_status));
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
    computer.Stop();
    qCritical() << project.GetProjectName() << "Abort compute";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  // Generate the DeviceConfig
  ActDeviceConfig device_config;
  this->deploy_available[project_id] = false;
  act_status = this->GenerateDeviceConfigByComputeResult(project, device_config);
  if (IsActStatusSuccess(act_status)) {
    // Update DeviceConfig
    act_status = this->UpdateDeviceConfig(project, device_config);
    if (IsActStatusSuccess(act_status)) {
      this->deploy_available[project_id] = true;
      // Send update msg
      ActDeviceConfigPatchUpdateMsg msg_device(ActPatchUpdateActionEnum::kUpdate, project_id, project.GetDeviceConfig(),
                                               true);
      this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg_device, project_id);

      ActComputedResultPatchUpdateMsg msg_compute_result(ActPatchUpdateActionEnum::kUpdate, project_id,
                                                         project.GetComputedResult(), false);
      this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg_compute_result, project_id);

    } else {
      qCritical() << "Update DeviceConfig failed with project id:" << project.GetId();
    }
  } else {
    qCritical() << "GenerateDeviceConfigByComputeResult() failed.";
  }

  // [bugfix:3196] project 計算成功後沒辦法按 deploy
  // Send features available status msg
  ActFeaturesAvailableStatusResult available_result(CanUndoProject(project_id), CanRedoProject(project_id),
                                                    CanDeployProject(project_id));
  ActFeaturesAvailableStatusWSResponse ws_features_available_status_msg(available_result);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_features_available_status_msg, project_id);

  // Write back the project status to the core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << project.GetProjectName() << "Cannot update the project";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartCompute, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    // If update failed would update the backup_project
    act_status = this->UpdateProject(backup_project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << project.GetProjectName() << "Cannot update the backup project";
    }

    return;
  }

  // Send finished status reply to client
  ActProgressStatus finished_status;
  finished_status.SetProgress(100);
  finished_status.SetStatus(ActStatusType::kFinished);

  ActProgressWSResponse ws_resp(ActWSCommandEnum::kStartCompute, finished_status);
  qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartCompute(qint64 &project_id, const qint64 &ws_listener_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;

  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kComputing, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartCompute, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << project_name << "Spawn compute thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::StartComputeThread, this, std::cref(ws_listener_id),
                                             std::move(signal_receiver), project_id);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"StartComputeThread";
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
