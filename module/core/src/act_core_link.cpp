#include <QSet>

#include "act_algorithm.hpp"
#include "act_core.hpp"
namespace act {
namespace core {

static void UpdateDeviceByLink(ActLink &link, ActProject &project) {
  project.SetUsedInterface(link.GetSourceDeviceId(), link.GetSourceInterfaceId());
  project.SetUsedInterface(link.GetDestinationDeviceId(), link.GetDestinationInterfaceId());

  ActDevice src_device;
  if (!IsActStatusNotFound(project.GetDeviceById(src_device, link.GetSourceDeviceId()))) {
    act::core::g_core.InsertDeviceMsgToNotificationTmp(
        ActDevicePatchUpdateMsg(ActPatchUpdateActionEnum::kUpdate, project.GetId(), src_device, true));
  }

  ActDevice dst_device;
  if (!IsActStatusNotFound(project.GetDeviceById(dst_device, link.GetDestinationDeviceId()))) {
    act::core::g_core.InsertDeviceMsgToNotificationTmp(
        ActDevicePatchUpdateMsg(ActPatchUpdateActionEnum::kUpdate, project.GetId(), dst_device, true));
  }
}

static ACT_STATUS UpdateStreamByLink(ActLink &ori_link, ActLink &update_link, ActProject &project) {
  ACT_STATUS_INIT();

  // Check talker or listeners is the same with the deleted link
  QSet<ActTrafficStream> stream_setting = project.GetTrafficDesign().GetStreamSetting();
  for (ActTrafficStream stream : stream_setting) {
    ActTrafficStreamInterface &talker = stream.GetTalker();
    if ((talker.GetDeviceId() == ori_link.GetSourceDeviceId() &&
         talker.GetInterfaceId() == ori_link.GetSourceInterfaceId()) ||
        (talker.GetDeviceId() == ori_link.GetDestinationDeviceId() &&
         talker.GetInterfaceId() == ori_link.GetDestinationInterfaceId())) {
      ActDevice update_talker;
      if (IsActStatusSuccess(project.GetDeviceById(update_talker, update_link.GetSourceDeviceId())) &&
          (update_talker.GetDeviceType() == ActDeviceTypeEnum::kEndStation ||
           update_talker.GetDeviceType() == ActDeviceTypeEnum::kBridgedEndStation)) {
        for (ActInterface intf : update_talker.GetInterfaces()) {
          if (intf.GetInterfaceId() == update_link.GetSourceInterfaceId()) {
            talker.SetDeviceId(intf.GetDeviceId());
            talker.SetInterfaceId(intf.GetInterfaceId());
            act_status = act::core::g_core.UpdateTrafficStreamSetting(project, stream);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << "Update stream failed:" << stream.ToString().toStdString().c_str();
              return act_status;
            }
            break;
          }
        }
      } else if (IsActStatusSuccess(project.GetDeviceById(update_talker, update_link.GetDestinationDeviceId())) &&
                 (update_talker.GetDeviceType() == ActDeviceTypeEnum::kEndStation ||
                  update_talker.GetDeviceType() == ActDeviceTypeEnum::kBridgedEndStation)) {
        for (ActInterface intf : update_talker.GetInterfaces()) {
          if (intf.GetInterfaceId() == update_link.GetDestinationInterfaceId()) {
            talker.SetDeviceId(intf.GetDeviceId());
            talker.SetInterfaceId(intf.GetInterfaceId());
            act_status = act::core::g_core.UpdateTrafficStreamSetting(project, stream);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << "Update stream failed:" << stream.ToString().toStdString().c_str();
              return act_status;
            }
            break;
          }
        }
      } else {
        // delete the stream
        act_status = act::core::g_core.DeleteTrafficStreamSetting(project, stream.GetId());
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Cannot delete related stream:" << stream.ToString().toStdString().c_str();
          return act_status;
        }
      }
    } else {
      for (ActTrafficStreamInterface listener : stream.GetListeners()) {
        if ((listener.GetDeviceId() == ori_link.GetSourceDeviceId() &&
             listener.GetInterfaceId() == ori_link.GetSourceInterfaceId()) ||
            (listener.GetDeviceId() == ori_link.GetDestinationDeviceId() &&
             listener.GetInterfaceId() == ori_link.GetDestinationInterfaceId())) {
          ActDevice update_listener;
          if (IsActStatusSuccess(project.GetDeviceById(update_listener, update_link.GetSourceDeviceId())) &&
              (update_listener.GetDeviceType() == ActDeviceTypeEnum::kEndStation ||
               update_listener.GetDeviceType() == ActDeviceTypeEnum::kBridgedEndStation)) {
            for (ActInterface intf : update_listener.GetInterfaces()) {
              if (intf.GetInterfaceId() == update_link.GetSourceInterfaceId()) {
                listener.SetDeviceId(intf.GetDeviceId());
                listener.SetInterfaceId(intf.GetInterfaceId());
                act_status = act::core::g_core.UpdateTrafficStreamSetting(project, stream);
                if (!IsActStatusSuccess(act_status)) {
                  qCritical() << "Update stream failed:" << stream.ToString().toStdString().c_str();
                  return act_status;
                }
                break;
              }
            }
          } else if (IsActStatusSuccess(project.GetDeviceById(update_listener, update_link.GetDestinationDeviceId())) &&
                     (update_listener.GetDeviceType() == ActDeviceTypeEnum::kEndStation ||
                      update_listener.GetDeviceType() == ActDeviceTypeEnum::kBridgedEndStation)) {
            for (ActInterface intf : update_listener.GetInterfaces()) {
              if (intf.GetInterfaceId() == update_link.GetDestinationInterfaceId()) {
                listener.SetDeviceId(intf.GetDeviceId());
                listener.SetInterfaceId(intf.GetInterfaceId());
                act_status = act::core::g_core.UpdateTrafficStreamSetting(project, stream);
                if (!IsActStatusSuccess(act_status)) {
                  qCritical() << "Update stream failed:" << stream.ToString().toStdString().c_str();
                  return act_status;
                }
                break;
              }
            }
          } else {
            stream.GetListeners().remove(listener);
            if (stream.GetListeners().isEmpty()) {
              act_status = act::core::g_core.DeleteTrafficStreamSetting(project, stream.GetId());
              if (!IsActStatusSuccess(act_status)) {
                qCritical() << "Cannot delete related stream:" << stream.ToString().toStdString().c_str();
                return act_status;
              }
            } else {
              act_status = act::core::g_core.UpdateTrafficStreamSetting(project, stream);
              if (!IsActStatusSuccess(act_status)) {
                qCritical() << "Update stream failed:" << stream.ToString().toStdString().c_str();
                return act_status;
              }
            }
          }
          break;
        }
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::CheckLink(ActProject &project, ActLink &link) {
  ACT_STATUS_INIT();

  // Get source/destination device by id first
  ActDevice src_dev;
  ActDevice dst_dev;
  ActInterface src_intf;
  ActInterface dst_intf;
  act_status = GetDeviceAndInterfacePairFromProject(project, link, src_dev, dst_dev, src_intf, dst_intf);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get link pair failed";
    return act_status;
  }

  // ICMP, Unknown, Moxa device would skip check the link speed
  if (src_dev.GetDeviceType() == ActDeviceTypeEnum::kICMP || dst_dev.GetDeviceType() == ActDeviceTypeEnum::kICMP ||
      src_dev.GetDeviceType() == ActDeviceTypeEnum::kUnknown ||
      dst_dev.GetDeviceType() == ActDeviceTypeEnum::kUnknown || src_dev.GetDeviceType() == ActDeviceTypeEnum::kMoxa ||
      dst_dev.GetDeviceType() == ActDeviceTypeEnum::kMoxa) {
    return act_status;
  }

  // Only stream based need to check the link speed
  if (!src_intf.GetSupportSpeeds().contains(link.GetSpeed()) ||
      !dst_intf.GetSupportSpeeds().contains(link.GetSpeed())) {
    QString message = QString("Project (%1) - The link speed %2 doesn't supported by both side %3(%4) & %5(%6)")
                          .arg(project.GetProjectName())
                          .arg(link.GetSpeed())
                          .arg(src_dev.GetIpv4().GetIpAddress())
                          .arg(src_intf.GetInterfaceName())
                          .arg(dst_dev.GetIpv4().GetIpAddress())
                          .arg(dst_intf.GetInterfaceName());
    qCritical() << message.toStdString().c_str();

    qDebug() << QString("Device(%1): Src intf: %2")
                    .arg(src_dev.GetIpv4().GetIpAddress())
                    .arg(src_intf.ToString())
                    .toStdString()
                    .c_str();
    qDebug() << QString("Device(%1): Dst intf: %2")
                    .arg(dst_dev.GetIpv4().GetIpAddress())
                    .arg(dst_intf.ToString())
                    .toStdString()
                    .c_str();

    return std::make_shared<ActBadRequest>(message);
  }

  return act_status;
}

ACT_STATUS ActCore::CreateLink(qint64 &project_id, ActLink &link, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->CreateLink(project, link);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Create link failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::CreateLink(ActProject &project, ActLink &link) {
  ACT_STATUS_INIT();

  act_status = this->CheckLink(project, link);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check link failed";
    return act_status;
  }

  // Generate a new unique id
  QSet<ActLink> &link_set = project.GetLinks();
  if (link.GetId() == -1) {
    qint64 id;
    act_status = this->GenerateUniqueId<ActLink>(link_set, project.last_assigned_link_id_, id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Cannot get an available unique id";
      return act_status;
    }
    link.SetId(id);
  }

  link.SetSourceDeviceIp(project.GetDeviceIp(link.GetSourceDeviceId()));
  link.SetDestinationDeviceIp(project.GetDeviceIp(link.GetDestinationDeviceId()));

  // Check link port is already used or not
  for (ActLink exist_link : link_set) {
    if (exist_link == link) {
      QString error_msg = QString("Link already exists between %1:%2 and %3:%4")
                              .arg(exist_link.GetSourceDeviceIp())
                              .arg(exist_link.GetSourceInterfaceId())
                              .arg(exist_link.GetDestinationDeviceIp())
                              .arg(exist_link.GetDestinationInterfaceId());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  link_set.insert(link);

  UpdateDeviceByLink(link, project);

  // Send update msg to temp
  InsertLinkMsgToNotificationTmp(ActLinkPatchUpdateMsg(ActPatchUpdateActionEnum::kCreate, project.GetId(), link, true));

  return act_status;
}

ACT_STATUS ActCore::CreateLinks(qint64 &project_id, QList<ActLink> &link_list, QList<qint64> &created_link_ids,
                                bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->CreateLinks(project, link_list, created_link_ids);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Create links failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActProjectPatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, project, true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project.GetId());

  return act_status;
}

ACT_STATUS ActCore::CreateLinks(ActProject &project, QList<ActLink> &link_list, QList<qint64> &created_link_ids) {
  ACT_STATUS_INIT();

  // Check link list is empty
  if (link_list.isEmpty()) {
    QString error_msg = QString("Link list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (ActLink link : link_list) {
    act_status = this->CreateLink(project, link);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Create link failed";
      return act_status;
    }

    created_link_ids.append(link.GetId());
  }

  return act_status;
}

ACT_STATUS ActCore::GetLink(qint64 &project_id, qint64 &link_id, ActLink &link, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QSet<ActLink> link_set = project.GetLinks();
  act_status = ActGetItemById<ActLink>(link_set, link_id, link);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Link id:" << link_id << "not found";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateLink(qint64 &project_id, ActLink &link, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateLink(project, link);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update link failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateLink(ActProject &project, ActLink &link) {
  ACT_STATUS_INIT();

  act_status = this->CheckLink(project, link);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check link failed";
    return act_status;
  }

  QSet<ActLink> &link_set = project.GetLinks();

  link.SetSourceDeviceIp(project.GetDeviceIp(link.GetSourceDeviceId()));
  link.SetDestinationDeviceIp(project.GetDeviceIp(link.GetDestinationDeviceId()));

  // Check the item does exist by id
  typename QSet<ActLink>::const_iterator iterator;
  iterator = link_set.find(link);
  if (iterator != link_set.end()) {
    // Get original source/destination device by id
    ActLink orig_link = *iterator;

    // Insert the link to project
    link_set.erase(iterator);

    // Check link port is already used or not
    for (ActLink exist_link : link_set) {
      if (exist_link == link) {
        QString error_msg = QString("Link already exists between %1:%2 and %3:%4")
                                .arg(exist_link.GetSourceDeviceIp())
                                .arg(exist_link.GetSourceInterfaceId())
                                .arg(exist_link.GetDestinationDeviceIp())
                                .arg(exist_link.GetDestinationInterfaceId());
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }

    link_set.insert(link);

    UpdateDeviceByLink(orig_link, project);
    UpdateDeviceByLink(link, project);

    act_status = UpdateStreamByLink(orig_link, link, project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Cannot update stream by link:" << link.ToString().toStdString().c_str();
      return act_status;
    }

  } else {
    act_status = this->CreateLink(project, link);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Create link failed";
      return act_status;
    }
  }

  // Send update msg to temp
  InsertLinkMsgToNotificationTmp(ActLinkPatchUpdateMsg(ActPatchUpdateActionEnum::kUpdate, project.GetId(), link, true));

  return act_status;
}

ACT_STATUS ActCore::UpdateLinks(qint64 &project_id, QList<ActLink> &link_list, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateLinks(project, link_list);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update links failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateLinks(ActProject &project, QList<ActLink> &link_list) {
  ACT_STATUS_INIT();

  // Check link list is empty
  if (link_list.isEmpty()) {
    QString error_msg = QString("Link list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (auto link : link_list) {
    act_status = this->UpdateLink(project, link);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Update link failed with link id:" << link.GetId();
      return act_status;
    }
  }
  return act_status;
}

ACT_STATUS ActCore::DeleteLink(qint64 &project_id, qint64 &link_id, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteLink(project, link_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete link failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::DeleteLink(ActProject &project, qint64 &link_id) {
  ACT_STATUS_INIT();

  QSet<ActLink> &link_set = project.GetLinks();

  // Check the item does exist by id
  ActLink link;
  act_status = project.GetLinkById(link, link_id);
  if (IsActStatusNotFound(act_status)) {
    QString error_msg = QString("Delete link failed, cannot found link id %1").arg(link_id);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  link_set.remove(link);

  act_status = UpdateStreamByLink(link, ActLink(), project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update device by link:" << link.ToString().toStdString().c_str();
    return act_status;
  }

  UpdateDeviceByLink(link, project);

  // Send update msg to temp
  InsertLinkMsgToNotificationTmp(ActLinkPatchUpdateMsg(ActPatchUpdateActionEnum::kDelete, project.GetId(), link, true));

  return act_status;
}

ACT_STATUS ActCore::DeleteLinks(qint64 &project_id, QList<qint64> &link_ids, bool is_operation) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteLinks(project, link_ids);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete links failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project, false, is_operation);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::DeleteLinks(ActProject &project, QList<qint64> &link_ids) {
  ACT_STATUS_INIT();

  // Check link list is empty
  if (link_ids.isEmpty()) {
    QString error_msg = QString("Link id list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (auto link_id : link_ids) {
    act_status = this->DeleteLink(project, link_id);
    if (!IsActStatusSuccess(act_status)) {  // fail to delete link
      qCritical() << "Delete link failed with link id:" << link_id;
      return act_status;
    }
  }

  return act_status;
}

}  // namespace core
}  // namespace act
