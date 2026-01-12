#include "act_core.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::InitNotificationTmp() {
  ACT_STATUS_INIT();

  this->notification_tmp_.SetDeviceUpdateMsgs(QList<ActDevicePatchUpdateMsg>());
  this->notification_tmp_.SetLinkUpdateMsgs(QList<ActLinkPatchUpdateMsg>());
  this->notification_tmp_.SetStreamUpdateMsgs(QList<ActStreamPatchUpdateMsg>());

  return act_status;
}

ACT_STATUS ActCore::InsertLinkMsgToNotificationTmp(const ActLinkPatchUpdateMsg &update_msg) {
  ACT_STATUS_INIT();

  QList<ActLinkPatchUpdateMsg> &update_msg_list = this->notification_tmp_.GetLinkUpdateMsgs();
  for (ActLinkPatchUpdateMsg &update_link_msg : update_msg_list) {
    if (update_link_msg.GetPath() == update_msg.GetPath()) {
      update_link_msg.SetData(update_msg.GetData());
      update_link_msg.SetSyncToWebsocket(update_msg.GetSyncToWebsocket());
      return ACT_STATUS_SUCCESS;
    }
  }

  // Remove previous Update Msg
  update_msg_list.append(update_msg);

  return act_status;
}

ACT_STATUS ActCore::InsertDeviceMsgToNotificationTmp(const ActDevicePatchUpdateMsg &update_msg) {
  ACT_STATUS_INIT();

  QList<ActDevicePatchUpdateMsg> &update_msg_list = this->notification_tmp_.GetDeviceUpdateMsgs();
  for (ActDevicePatchUpdateMsg &update_device_msg : update_msg_list) {
    if (update_device_msg.GetPath() == update_msg.GetPath()) {
      update_device_msg.SetData(update_msg.GetData());
      update_device_msg.SetSyncToWebsocket(update_msg.GetSyncToWebsocket());
      return ACT_STATUS_SUCCESS;
    }
  }

  // Remove previous Update Msg
  update_msg_list.append(update_msg);

  return act_status;
}

ACT_STATUS ActCore::InsertStreamMsgToNotificationTmp(const ActStreamPatchUpdateMsg &update_msg) {
  ACT_STATUS_INIT();

  QList<ActStreamPatchUpdateMsg> &update_msg_list = this->notification_tmp_.GetStreamUpdateMsgs();
  for (ActStreamPatchUpdateMsg &update_stream_msg : update_msg_list) {
    if (update_stream_msg.GetPath() == update_msg.GetPath()) {
      update_stream_msg.SetData(update_msg.GetData());
      update_stream_msg.SetSyncToWebsocket(update_msg.GetSyncToWebsocket());
      return ACT_STATUS_SUCCESS;
    }
  }

  // Remove previous Update Msg
  update_msg_list.append(update_msg);

  return act_status;
}

// XXX: add into ActCore::SendMessageToListener(), for unify the entrance function
// ACT_STATUS ActCore::SendNotificationTmpMsgs(const qint64 &project_id) {
//   ACT_STATUS_INIT();
//   auto local_notification_tmp = this->notification_tmp_;

//   // Devices
//   for (auto msg : local_notification_tmp.GetDeviceUpdateMsgs().values()) {
//     auto message = msg.ToString(msg.key_order_).toStdString().c_str();
//     act_status = this->SendMessageToListener(ActWSTypeEnum::kProject, message, project_id);
//   }

//   // Links
//   for (auto msg : local_notification_tmp.GetLinkUpdateMsgs().values()) {
//     auto message = msg.ToString(msg.key_order_).toStdString().c_str();
//     act_status = this->SendMessageToListener(ActWSTypeEnum::kProject, message, project_id);
//   }

//   // Streams
//   for (auto msg : local_notification_tmp.GetStreamUpdateMsgs().values()) {
//     auto message = msg.ToString(msg.key_order_).toStdString().c_str();
//     act_status = this->SendMessageToListener(ActWSTypeEnum::kProject, message, project_id);
//   }

//   return act_status;
// }

}  // namespace core
}  // namespace act
