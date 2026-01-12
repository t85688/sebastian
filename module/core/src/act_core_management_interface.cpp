#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::CheckManagementInterface(const ActProject &project,
                                             const ActManagementInterface &management_interface) {
  ACT_STATUS_INIT();

  // Check device
  auto device_id = management_interface.GetDeviceId();
  ActDevice device;
  act_status = project.GetDeviceById(device, device_id);
  if (!IsActStatusSuccess(act_status)) {  // not found
    qCritical() << __func__ << QString("Device(%1) not found").arg(device_id);
    return act_status;
  }

  // Check interfaces
  for (auto mgmt_interface_id : management_interface.GetInterfaces()) {
    if (device.GetInterfaces().indexOf(ActInterface(mgmt_interface_id)) == -1) {  // not found
      qCritical() << __func__ << QString("Interface(%1) not found in Device(%2)").arg(mgmt_interface_id).arg(device_id);
      return std::make_shared<ActStatusNotFound>(
          QString("Interface(%1) at Device(%2)").arg(device_id).arg(mgmt_interface_id));
    }
  }

  return act_status;
}

ACT_STATUS ActCore::GetManagementInterface(qint64 &project_id, qint64 &device_id,
                                           ActManagementInterface &management_interface) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {  // not found
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  ActManagementInterface target_mgmt_interface(device_id);
  auto target_mgmt_interface_idx =
      project.GetTopologySetting().GetManagementInterfaces().indexOf(target_mgmt_interface);
  if (target_mgmt_interface_idx == -1) {  // not found
    qCritical() << __func__ << QString("Device(%1) not found in ManagementInterfaces").arg(device_id);
    return std::make_shared<ActStatusNotFound>(QString("Device(%1) at ManagementInterfaces").arg(device_id));
  }

  management_interface = project.GetTopologySetting().GetManagementInterfaces().at(target_mgmt_interface_idx);
  return act_status;
}

ACT_STATUS ActCore::CreateManagementInterface(qint64 &project_id, ActManagementInterface &management_interface) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {  // not found
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->CreateManagementInterface(project, management_interface);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__
                << "Create management_interface failed with device id:" << management_interface.GetDeviceId();
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp(Device)
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  // Send update msg (ManagementInterface)
  ActManagementInterfacePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kCreate, project_id, management_interface,
                                              true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return act_status;
}

ACT_STATUS ActCore::CreateManagementInterface(ActProject &project, ActManagementInterface &management_interface) {
  ACT_STATUS_INIT();

  // Check ManagementInterface duplicated target_mgmt_interface);
  ActManagementInterface target_mgmt_interface(management_interface.GetDeviceId());
  if (project.GetTopologySetting().GetManagementInterfaces().indexOf(target_mgmt_interface) !=
      -1) {  // found(duplicated)
    qCritical()
        << __func__
        << QString("The ManagementInterface(DeviceId:%1) is duplicated").arg(management_interface.GetDeviceId());
    QString duplicated = QString("ManagementInterface(DeviceId:%1)").arg(management_interface.GetDeviceId());
    return std::make_shared<ActDuplicatedError>(duplicated);
  }

  // Check ManagementInterfaces
  act_status = this->CheckManagementInterface(project, management_interface);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Check the ManagementInterface failed";
    return act_status;
  }

  // Append ManagementInterface to ManagementInterfaces list
  project.GetTopologySetting().GetManagementInterfaces().append(management_interface);

  // Update Management flag of the Device's Interfaces
  ActDevice device;
  project.GetDeviceById(device, management_interface.GetDeviceId());
  // Set Management flag
  for (auto &interface : device.GetInterfaces()) {
    if (management_interface.GetInterfaces().contains(interface.GetInterfaceId())) {
      // Device's interface is the management_interface
      interface.SetManagement(true);
    } else {
      interface.SetManagement(false);
    }
  }
  // Update device
  act_status = this->UpdateDevice(project, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update device failed with device id:" << device.GetId();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateManagementInterface(qint64 &project_id, ActManagementInterface &management_interface) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {  // not found
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateManagementInterface(project, management_interface);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__
                << "Update management_interface failed with device id:" << management_interface.GetDeviceId();
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp(Device)
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  // Send Websocket update msg(ManagementInterface)
  ActManagementInterfacePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, management_interface,
                                              true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateManagementInterface(ActProject &project, ActManagementInterface &management_interface) {
  ACT_STATUS_INIT();

  // Check ManagementInterfaces
  act_status = this->CheckManagementInterface(project, management_interface);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Check the ManagementInterface failed";
    return act_status;
  }

  // [bugfix:2840] Modify network management endpoint, the topology not update.
  // Multiple ManagementInterface (wait UI supported)
  // // Check the item does exist by device_id
  // ActManagementInterface target_mgmt_interface(management_interface.GetDeviceId());
  // if (project.GetTopologySetting().GetManagementInterfaces().indexOf(target_mgmt_interface) != -1) {  // found
  //   // If yes, delete it
  //   project.GetTopologySetting().GetManagementInterfaces().removeOne(target_mgmt_interface);
  // }

  // [bugfix:2840] Modify network management endpoint, the topology not update.
  // Clear old ManagementInterface(only one ManagementInterface)
  for (auto mgmt_interface : project.GetTopologySetting().GetManagementInterfaces()) {
    act_status = this->DeleteManagementInterface(project, mgmt_interface.GetDeviceId());
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Delete management_interface failed with device id:" << mgmt_interface.GetDeviceId();
      return act_status;
    }
  }

  // Append new ManagementInterface to ManagementInterfaces list
  project.GetTopologySetting().GetManagementInterfaces().append(management_interface);

  // Update Management flag of the Device's Interfaces
  ActDevice device;
  project.GetDeviceById(device, management_interface.GetDeviceId());
  // Set Management flag
  for (auto &interface : device.GetInterfaces()) {
    if (management_interface.GetInterfaces().contains(interface.GetInterfaceId())) {
      // Device's interface is the management_interface
      interface.SetManagement(true);
    } else {
      interface.SetManagement(false);
    }
  }

  // Update device
  act_status = this->UpdateDevice(project, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update device failed with device id:" << device.GetId();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::DeleteManagementInterface(qint64 &project_id, const qint64 &device_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Keep the deleted management interface
  ActManagementInterface management_interface(device_id);

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {  // not found
    qCritical() << __func__ << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteManagementInterface(project, device_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Delete management_interface failed with device id:" << device_id;
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp(Device)
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  // Send Websocket update msg
  ActManagementInterfacePatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kDelete, project_id, management_interface,
                                              true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return act_status;
}

ACT_STATUS ActCore::DeleteManagementInterface(ActProject &project, const qint64 &device_id) {
  ACT_STATUS_INIT();

  // Check the item does exist by device_id
  ActManagementInterface target_mgmt_interface(device_id);
  if (project.GetTopologySetting().GetManagementInterfaces().indexOf(target_mgmt_interface) == -1) {  // not found
    return act_status;
  }

  // If yes, delete it
  project.GetTopologySetting().GetManagementInterfaces().removeOne(target_mgmt_interface);

  // Update Management flag of the Device's Interfaces (If device exists)
  ActDevice device;
  if (!IsActStatusNotFound(project.GetDeviceById(device, device_id))) {
    // Set Management flag
    for (auto &interface : device.GetInterfaces()) {
      interface.SetManagement(false);
    }

    // Update device
    act_status = this->UpdateDevice(project, device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Update device failed with device id:" << device.GetId();
      return act_status;
    }
  }

  return act_status;
}

}  // namespace core
}  // namespace act