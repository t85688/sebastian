/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_auto_scan_result.hpp"
#include "act_device_profile.hpp"
#include "act_model.hpp"
#include "act_topology.hpp"
#include "act_topology_mapping_result.hpp"
#include "act_user.hpp"
#include "act_ws_cmd.hpp"

// ------------------------------------------------- Base Response ------------------------------------------------- //

/**
 * @brief The patch update action enum class
 *
 */
enum class ActPatchUpdateActionEnum { kCreate = 1, kDelete = 2, kUpdate = 3 };

/**
 * @brief The QMap for patch update action enum mapping
 *
 */
static const QMap<QString, ActPatchUpdateActionEnum> kActPatchUpdateActionEnumMap = {
    {"Create", ActPatchUpdateActionEnum::kCreate},
    {"Delete", ActPatchUpdateActionEnum::kDelete},
    {"Update", ActPatchUpdateActionEnum::kUpdate}};

class ActBaseResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(qint64, op_code, OpCode);
  ACT_JSON_FIELD(qint64, status_code, StatusCode);
  ACT_JSON_ENUM(ActPatchUpdateActionEnum, action, Action);
  ACT_JSON_FIELD(QString, data, Data);
  ACT_JSON_FIELD(bool, sync_to_opcua, SyncToOpcua);
  ACT_JSON_FIELD(bool, sync_to_websocket, SyncToWebsocket);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act Response Base object
   *
   */
  ActBaseResponse() {
    this->key_order_ = QList<QString>({QString("OpCode"), QString("StatusCode"), QString("Data")});

    // default value
    this->SetOpCode(static_cast<qint64>(ActWSCommandEnum::kTestStart));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kSuccess));
    this->SetAction(ActPatchUpdateActionEnum::kUpdate);
    this->SetData("");
    this->SetSyncToOpcua(false);
    this->SetSyncToWebsocket(true);
  }

  ActBaseResponse(const ActWSCommandEnum &op_code_enum, const ActStatusBase &status_base) : ActBaseResponse() {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(status_base.GetStatus()));
  }
};

// ------------------------------------------- Patch Update Response------------------------------------------------ //

// WS Data Structure
// {
//   "OpCode": 65534,  // TestStart
//   "StatusCode": 200, // Success
//   "Data": "資料" progress/device...等
// }

// WS Error Structure
// {
//   "OpCode": 65534,  // TestStart
//   "StatusCode": 2000, // RoutingDestinationUnreachable
//   "ErrorMessage": "xxx failed" // 當有 error 的時候
//   "Parameter": {
//     "StreamName: "Stream 1",
//     "DestinationIP": "192.168.127.100"
//   }
// }

// PatchUpdate
// {
//   "OpCode": 4097,  // PatchUpdate
//   "Path": "Projects/1001/Devices/1001",
//   "Action": "Update", // Create, Delete
//   "Data": {
//     // Device Object
//     "DeviceName": "",
//     "DeviceType": "TSNSwitch",
//     ...
//   }
// }

/**
 * @brief A base temp notification message class
 *
 */
class ActBasePatchUpdateMsg : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, path, Path);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act WebSocket Response Base object
   *
   */
  ActBasePatchUpdateMsg() {
    this->SetOpCode(static_cast<qint64>(ActWSCommandEnum::kPatchUpdate));
    this->key_order_ = QList<QString>({QString("OpCode"), QString("Path"), QString("Action"), QString("Data")});
  }
};

class ActSimpleDesignBaselinePatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActSimpleNetworkBaseline, data, Data);

 public:
  ActSimpleDesignBaselinePatchUpdateMsg() {
    this->SetPath("");
    this->SetAction(ActPatchUpdateActionEnum::kUpdate);
  }

  ActSimpleDesignBaselinePatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                                        ActNetworkBaseline network_baseline, bool sync_to_websocket) {
    ActSimpleNetworkBaseline simple_nb(network_baseline);

    this->SetPath(QString("Projects/%1/NetworkBaselines/Design/%2").arg(project_id).arg(simple_nb.GetId()));
    this->SetAction(action);
    this->SetData(simple_nb);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActSimpleDesignBaselinePatchUpdateMsg &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActSimpleDesignBaselinePatchUpdateMsg &x,
                         const ActSimpleDesignBaselinePatchUpdateMsg &y) {
    return (x.GetAction() == y.GetAction()) && (x.data_.GetId() == y.data_.GetId());
  }
};

class ActSimpleProjectPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActSimpleProject, data, Data);

 public:
  ActSimpleProjectPatchUpdateMsg() {
    this->SetPath("");
    this->SetAction(ActPatchUpdateActionEnum::kUpdate);
  }

  ActSimpleProjectPatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                                 ActSimpleProject simple_project, bool sync_to_websocket) {
    this->SetPath(QString("simple-projects/%1").arg(project_id));
    this->SetAction(action);
    this->SetData(simple_project);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActSimpleProjectPatchUpdateMsg &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActSimpleProjectPatchUpdateMsg &x, const ActSimpleProjectPatchUpdateMsg &y) {
    return (x.GetAction() == y.GetAction()) && (x.data_.GetId() == y.data_.GetId());
  }
};

class ActSimpleOperationBaselinePatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActSimpleNetworkBaseline, data, Data);

 public:
  ActSimpleOperationBaselinePatchUpdateMsg() {
    this->SetPath("");
    this->SetAction(ActPatchUpdateActionEnum::kUpdate);
  }

  ActSimpleOperationBaselinePatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                                           ActNetworkBaseline network_baseline, bool sync_to_websocket) {
    ActSimpleNetworkBaseline simple_nb(network_baseline);

    this->SetPath(QString("Projects/%1/NetworkBaselines/Operation/%2").arg(project_id).arg(simple_nb.GetId()));
    this->SetAction(action);
    this->SetData(simple_nb);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActSimpleOperationBaselinePatchUpdateMsg &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActSimpleOperationBaselinePatchUpdateMsg &x,
                         const ActSimpleOperationBaselinePatchUpdateMsg &y) {
    return (x.GetAction() == y.GetAction()) && (x.data_.GetId() == y.data_.GetId());
  }
};

class ActDevicePatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDevice, data, Data);

 public:
  ActDevicePatchUpdateMsg() {
    this->SetPath("");
    this->SetAction(ActPatchUpdateActionEnum::kUpdate);
  }

  ActDevicePatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id, ActDevice device,
                          bool sync_to_websocket) {
    device.HidePassword();

    this->SetPath(QString("Projects/%1/Devices/%2").arg(project_id).arg(device.GetId()));
    this->SetAction(action);
    this->SetData(device);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActDevicePatchUpdateMsg &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActDevicePatchUpdateMsg &x, const ActDevicePatchUpdateMsg &y) {
    return (x.GetAction() == y.GetAction()) && (x.data_.GetId() == y.data_.GetId());
  }
};

class ActLinkPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActLink, data, Data);

 public:
  ActLinkPatchUpdateMsg() {
    this->SetPath("");
    this->SetAction(ActPatchUpdateActionEnum::kUpdate);
  }

  ActLinkPatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id, ActLink link,
                        bool sync_to_websocket) {
    this->SetPath(QString("Projects/%1/Links/%2").arg(project_id).arg(link.GetId()));
    this->SetAction(action);
    this->SetData(link);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActLinkPatchUpdateMsg &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActLinkPatchUpdateMsg &x, const ActLinkPatchUpdateMsg &y) {
    return (x.GetAction() == y.GetAction()) && (x.data_.GetId() == y.data_.GetId());
  }
};

class ActStreamPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStream, data, Data);

 public:
  ActStreamPatchUpdateMsg() {
    this->SetPath("");
    this->SetAction(ActPatchUpdateActionEnum::kUpdate);
  }

  ActStreamPatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id, ActStream stream,
                          bool sync_to_websocket) {
    this->SetPath(QString("Projects/%1/Streams/%2").arg(project_id).arg(stream.GetId()));
    this->SetAction(action);
    this->SetData(stream);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActStreamPatchUpdateMsg &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActStreamPatchUpdateMsg &x, const ActStreamPatchUpdateMsg &y) {
    return (x.GetAction() == y.GetAction()) && (x.data_.GetId() == y.data_.GetId());
  }
};

/**
 * @brief A temp notification message class
 *
 */
class ActNotificationMsgTmp : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActStreamPatchUpdateMsg, stream_update_msgs, StreamUpdateMsgs);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActDevicePatchUpdateMsg, device_update_msgs, DeviceUpdateMsgs);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActLinkPatchUpdateMsg, link_update_msgs, LinkUpdateMsgs);
};

class ActCycleSettingPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActCycleSetting, data, Data);

 public:
  ActCycleSettingPatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                                const ActCycleSetting &cycle_setting, bool sync_to_websocket) {
    this->SetPath(QString("Projects/%1/CycleSetting").arg(project_id));
    this->SetAction(action);
    this->SetData(cycle_setting);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActCycleSettingPatchUpdateMsg() {}
};

class ActProjectSettingPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActProjectSetting, data, Data);

 public:
  ActProjectSettingPatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                                  ActProjectSetting project_setting, bool sync_to_websocket) {
    project_setting.HidePassword();

    this->SetPath(QString("Projects/%1/ProjectSetting").arg(project_id));
    this->SetAction(action);
    this->SetData(project_setting);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActProjectSettingPatchUpdateMsg() {}
};

class ActManagementInterfacePatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActManagementInterface, data, Data);

 public:
  ActManagementInterfacePatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                                       const ActManagementInterface &management_interface, bool sync_to_websocket) {
    this->SetPath(QString("Projects/%1/TopologySetting/ManagementInterfaces/%2")
                      .arg(project_id)
                      .arg(management_interface.GetDeviceId()));
    this->SetAction(action);
    this->SetData(management_interface);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActManagementInterfacePatchUpdateMsg() {}
};

class ActDeviceConfigPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDeviceConfig, data, Data);

 public:
  ActDeviceConfigPatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                                const ActDeviceConfig &device_config, bool sync_to_websocket) {
    this->SetPath(QString("Projects/%1/DeviceConfig").arg(project_id));
    this->SetAction(action);
    this->SetData(device_config);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActDeviceConfigPatchUpdateMsg() {}
};

class ActTrafficDesignPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActTrafficDesign, data, Data);

 public:
  ActTrafficDesignPatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                                 const ActTrafficDesign &traffic_design, bool sync_to_websocket) {
    this->SetPath(QString("Projects/%1/TrafficDesign").arg(project_id));
    this->SetAction(action);
    this->SetData(traffic_design);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActTrafficDesignPatchUpdateMsg() {}
};

class ActComputedResultPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActComputedResult, data, Data);

 public:
  ActComputedResultPatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                                  ActComputedResult compute_result, bool sync_to_websocket) {
    compute_result.HidePassword();

    this->SetPath(QString("Projects/%1/ComputedResult").arg(project_id));
    this->SetAction(action);
    this->SetData(compute_result);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActComputedResultPatchUpdateMsg() {}
};

class ActStreamStatusPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActStreamStatusEnum, data, Data);

 public:
  ActStreamStatusPatchUpdateMsg(const ActPatchUpdateActionEnum &action, ActStreamStatusEnum status,
                                bool sync_to_websocket) {
    this->SetPath("");
    this->SetAction(action);
    this->SetData(status);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActStreamStatusPatchUpdateMsg() {}
};

class ActProjectPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActProject, data, Data);

 public:
  /**
   * @brief Construct a new Act Project Patch Update Msg object
   *
   * @param action
   * @param project
   * @param sync_to_opcua
   * @param sync_to_websocket
   */
  ActProjectPatchUpdateMsg(const ActPatchUpdateActionEnum &action, ActProject project, bool sync_to_websocket) {
    project.HidePassword();

    this->SetPath(QString("Projects/%1").arg(project.GetId()));
    this->SetAction(action);
    this->SetData(project);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActProjectPatchUpdateMsg() {}
};

class ActTopologyPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActTopology, data, Data);

 public:
  ActTopologyPatchUpdateMsg(const ActPatchUpdateActionEnum &action, ActTopology topology, bool sync_to_websocket) {
    topology.HidePassword();

    this->SetPath(QString("Topologies/%1").arg(topology.GetId()));
    this->SetAction(action);
    this->SetData(topology);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActTopologyPatchUpdateMsg() {}
};

class ActUserPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActUser, data, Data);

 public:
  ActUserPatchUpdateMsg(const ActPatchUpdateActionEnum &action, ActUser user, bool sync_to_websocket) {
    user.HidePassword();

    this->SetPath(QString("Users/%1").arg(user.GetId()));
    this->SetAction(action);
    this->SetData(user);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActUserPatchUpdateMsg() {}
};

class ActDeviceProfilePatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDeviceProfile, data, Data);

 public:
  ActDeviceProfilePatchUpdateMsg(const ActPatchUpdateActionEnum &action, const ActDeviceProfile &device_profile,
                                 bool sync_to_websocket) {
    this->SetPath(QString("DeviceProfiles/%1").arg(device_profile.GetId()));
    this->SetAction(action);
    this->SetData(device_profile);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActDeviceProfilePatchUpdateMsg() {}
};

class ActSystemPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActSystem, data, Data);

 public:
  ActSystemPatchUpdateMsg(const ActPatchUpdateActionEnum &action, const ActSystem &system, bool sync_to_websocket) {
    this->SetPath(QString("System"));
    this->SetAction(action);
    this->SetData(system);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActSystemPatchUpdateMsg() {}
};

class ActDeviceBagPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDeviceBag, data, Data);

 public:
  ActDeviceBagPatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                             const ActDeviceBag &device_bag, bool sync_to_websocket) {
    this->SetPath(QString("Projects/%1/DeviceBag").arg(project_id));
    this->SetAction(action);
    this->SetData(device_bag);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActDeviceBagPatchUpdateMsg() {}
};

class AckDeviceBagDeleteMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActModelList, data, Data);

 public:
  AckDeviceBagDeleteMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                        const ActModelList &model_list, bool sync_to_websocket) {
    this->SetPath(QString("Projects/%1/DeviceBag").arg(project_id));
    this->SetAction(action);
    this->SetData(model_list);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  AckDeviceBagDeleteMsg() {}
};

class ActSkuQuantityPatchUpdateMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActSkuQuantities, data, Data);

 public:
  ActSkuQuantityPatchUpdateMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                               const ActSkuQuantities &sku_quantities, bool sync_to_websocket) {
    this->SetPath(QString("Projects/%1/SkuQuantities").arg(project_id));
    this->SetAction(action);
    this->SetData(sku_quantities);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActSkuQuantityPatchUpdateMsg() {}
};

class AckSkuQuantityDeleteMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActSkuList, data, Data);

 public:
  AckSkuQuantityDeleteMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id, const ActSkuList &sku_list,
                          bool sync_to_websocket) {
    this->SetPath(QString("Projects/%1/SkuQuantities").arg(project_id));
    this->SetAction(action);
    this->SetData(sku_list);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  AckSkuQuantityDeleteMsg() {}
};

/*************
 *  Monitor  *
 *************/
/**
 * @brief The ACT Device ICMP Status class
 *
 */
class ActMonitorDeviceStatusData : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(bool, alive, Alive);
  ACT_JSON_FIELD(QString, ip_address, IpAddress);

 public:
  QList<QString> key_order_;

  ActMonitorDeviceStatusData() {
    this->id_ = -1;
    this->key_order_.append(QList<QString>({QString("Id"), QString("Alive"), QString("IpAddress")}));
  }

  ActMonitorDeviceStatusData(const qint64 &id, const QString &ip_address, const bool &alive)
      : ActMonitorDeviceStatusData() {
    this->id_ = id;
    this->ip_address_ = ip_address;
    this->alive_ = alive;
  }

  ActMonitorDeviceStatusData(ActPingDevice ping_device) : ActMonitorDeviceStatusData() {
    this->id_ = ping_device.GetId();
    this->alive_ = ping_device.GetAlive();
    this->ip_address_ = ping_device.GetIpAddress();
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActMonitorDeviceStatusData &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActMonitorDeviceStatusData &x, const ActMonitorDeviceStatusData &y) {
    return x.id_ == y.id_;
  }
};

/**
 * @brief The ACT Device Monitor Message class
 *
 */
class ActMonitorDeviceMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActMonitorDeviceStatusData, data, Data);

 public:
  ActMonitorDeviceMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                      const QSet<ActMonitorDeviceStatusData> &data, bool sync_to_websocket) {
    this->SetOpCode(static_cast<qint64>(ActWSCommandEnum::kMonitorAliveUpdate));
    this->SetPath(QString("Projects/%1/Devices").arg(project_id));
    this->SetAction(action);
    this->SetData(data);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActMonitorDeviceMsg() {}
};

/**
 * @brief The ACT Monitor Link Status class
 *
 */
class ActMonitorLinkStatusData : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);

  ACT_JSON_FIELD(bool, alive, Alive);

  ACT_JSON_FIELD(bool, redundancy, Redundancy);

  ACT_JSON_FIELD(qint64, source_device_id, SourceDeviceId);
  ACT_JSON_FIELD(QString, source_device_ip, SourceDeviceIp);
  ACT_JSON_FIELD(qint64, source_interface_id, SourceInterfaceId);

  ACT_JSON_FIELD(qint64, destination_device_id, DestinationDeviceId);
  ACT_JSON_FIELD(QString, destination_device_ip, DestinationDeviceIp);
  ACT_JSON_FIELD(qint64, destination_interface_id, DestinationInterfaceId);

 public:
  QList<QString> key_order_;

  ActMonitorLinkStatusData() {
    this->id_ = -1;
    this->key_order_.append(QList<QString>({QString("Id"), QString("Alive"), QString("Redundancy")}));
  }

  ActMonitorLinkStatusData(ActLink link) : ActMonitorLinkStatusData() {
    this->id_ = link.GetId();
    this->alive_ = link.GetAlive();
    this->redundancy_ = false;
    this->source_device_id_ = link.GetSourceDeviceId();
    this->source_device_ip_ = link.GetSourceDeviceIp();
    this->source_interface_id_ = link.GetSourceInterfaceId();
    this->destination_device_id_ = link.GetDestinationDeviceId();
    this->destination_device_ip_ = link.GetDestinationDeviceIp();
    this->destination_interface_id_ = link.GetDestinationInterfaceId();
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActMonitorLinkStatusData &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActMonitorLinkStatusData &x, const ActMonitorLinkStatusData &y) {
    return x.id_ == y.id_;
  }
};

/**
 * @brief The ACT Link Monitor Message class
 *
 */
class ActMonitorLinkMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActMonitorLinkStatusData, data, Data);

 public:
  ActMonitorLinkMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                    const QSet<ActMonitorLinkStatusData> &data, bool sync_to_websocket) {
    this->SetOpCode(static_cast<qint64>(ActWSCommandEnum::kMonitorAliveUpdate));
    this->SetPath(QString("Projects/%1/Links").arg(project_id));
    this->SetAction(action);
    this->SetData(data);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActMonitorLinkMsg() {}
};

class ActMonitorDeviceSystemStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);
  ACT_JSON_FIELD(qreal, cpu_usage, CPUUsage);
  ACT_JSON_FIELD(qreal, memory_usage, MemoryUsage);
  ACT_JSON_FIELD(QString, system_uptime, SystemUptime);
  ACT_JSON_FIELD(QString, redundant_protocol, RedundantProtocol);
  ACT_JSON_FIELD(QString, product_revision, ProductRevision);
  ACT_JSON_ENUM(ActRstpPortRoleEnum, role, Role);
  ACT_JSON_FIELD(QString, mac_address, MacAddress);
  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(QString, firmware_version, FirmwareVersion);
  ACT_JSON_FIELD(QString, serial_number, SerialNumber);
  ACT_JSON_FIELD(QString, device_name, DeviceName);
  ACT_JSON_FIELD(QString, alias, Alias);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActInterface, interfaces, Interfaces);
  ACT_JSON_OBJECT(ActDeviceModularInfo, modular_info, ModularInfo);

 public:
  ActMonitorDeviceSystemStatus() {
    this->cpu_usage_ = 0;
    this->memory_usage_ = 0;
    this->system_uptime_ = "";
    this->redundant_protocol_ = "";
    this->product_revision_ = "";
    this->role_ = ActRstpPortRoleEnum::kNone;
    this->mac_address_ = "";
    this->model_name_ = "";
    this->firmware_version_ = "";
    this->serial_number_ = "";
    this->device_name_ = "";
    this->alias_ = "";
  }

  ActMonitorDeviceSystemStatus(ActMonitorBasicStatus basic_status) : ActMonitorDeviceSystemStatus() {
    this->device_id_ = basic_status.GetDeviceId();
    this->device_ip_ = basic_status.GetDeviceIp();
    this->cpu_usage_ = basic_status.GetSystemUtilization().GetCPUUsage();
    this->memory_usage_ = basic_status.GetSystemUtilization().GetMemoryUsage();
    this->system_uptime_ = basic_status.GetSystemUptime();
    this->redundant_protocol_ = basic_status.GetRedundantProtocol();
    this->product_revision_ = basic_status.GetProductRevision();
    this->mac_address_ = basic_status.GetMacAddress();
    this->firmware_version_ = basic_status.GetFirmwareVersion();
    this->serial_number_ = basic_status.GetSerialNumber();
    this->device_name_ = basic_status.GetDeviceName();
    this->mac_address_ = basic_status.GetMacAddress();
    this->modular_info_ = basic_status.GetModularInfo();
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActMonitorDeviceSystemStatus &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActMonitorDeviceSystemStatus &x, const ActMonitorDeviceSystemStatus &y) {
    return x.device_id_ == y.device_id_;
  }
};

/**
 * @brief The ACT Monitor Traffic View class
 *
 */
class ActMonitorDeviceSystemStatusMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActMonitorDeviceSystemStatus, data, Data);

 public:
  ActMonitorDeviceSystemStatusMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                                  const QSet<ActMonitorDeviceSystemStatus> &data, bool sync_to_websocket) {
    this->SetOpCode(static_cast<qint64>(ActWSCommandEnum::kMonitorStatusUpdate));
    this->SetPath(QString("Projects/%1/Devices").arg(project_id));
    this->SetAction(action);
    this->SetData(data);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActMonitorDeviceSystemStatusMsg() {}
};

/**
 * @brief The ACT Device Monitor Traffic View class
 *
 */
class ActDeviceMonitorTrafficMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActDeviceMonitorTraffic, data, Data);

 public:
  ActDeviceMonitorTrafficMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                             const QSet<ActDeviceMonitorTraffic> &data, bool sync_to_websocket) {
    this->SetOpCode(static_cast<qint64>(ActWSCommandEnum::kMonitorTrafficUpdate));
    this->SetPath(QString("Projects/%1/Devices").arg(project_id));
    this->SetAction(action);
    this->SetData(data);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActDeviceMonitorTrafficMsg() {}
};

/**
 * @brief The ACT Link Monitor Traffic View class
 *
 */
class ActLinkMonitorTrafficMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActLinkMonitorTraffic, data, Data);

 public:
  ActLinkMonitorTrafficMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                           const QSet<ActLinkMonitorTraffic> &data, bool sync_to_websocket) {
    this->SetOpCode(static_cast<qint64>(ActWSCommandEnum::kMonitorTrafficUpdate));
    this->SetPath(QString("Projects/%1/Links").arg(project_id));
    this->SetAction(action);
    this->SetData(data);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActLinkMonitorTrafficMsg() {}
};

/**
 * @brief The ACT Monitor Time Status Message class
 *
 */
class ActMonitorTimeStatusMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActMonitorTimeStatus, data, Data);

 public:
  ActMonitorTimeStatusMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                          const QSet<ActMonitorTimeStatus> &data, bool sync_to_websocket) {
    this->SetOpCode(static_cast<qint64>(ActWSCommandEnum::kMonitorTimeStatusUpdate));
    this->SetPath(QString("Projects/%1/Devices").arg(project_id));
    this->SetAction(action);
    this->SetData(data);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActMonitorTimeStatusMsg() {}
};

/**
 * @brief The ACT Device Monitor Message class
 *
 */
class ActMonitorEndpointMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActSourceDevice, data, Data);

 public:
  ActMonitorEndpointMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                        const ActSourceDevice &endpoint, bool sync_to_websocket) {
    this->SetOpCode(static_cast<qint64>(ActWSCommandEnum::kMonitorEndpointUpdate));
    this->SetPath(QString("Projects/%1/Endpoint").arg(project_id));
    this->SetAction(action);
    this->SetData(endpoint);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActMonitorEndpointMsg() {}
};

/**
 * @brief The ACT Monitor Swift Status Message class
 *
 */
class ActMonitorSwiftStatusMsg : public ActBasePatchUpdateMsg {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActMonitorSwiftStatus, data, Data);

 public:
  ActMonitorSwiftStatusMsg(const ActPatchUpdateActionEnum &action, const qint64 &project_id,
                           const QSet<ActMonitorSwiftStatus> &data, bool sync_to_websocket) {
    this->SetOpCode(static_cast<qint64>(ActWSCommandEnum::kMonitorSwiftStatusUpdate));
    this->SetPath(QString("Projects/%1/Devices").arg(project_id));
    this->SetAction(action);
    this->SetData(data);
    this->SetSyncToWebsocket(sync_to_websocket);
  }

 private:
  ActMonitorSwiftStatusMsg() {}
};

/****************************
 *  Error Message Response  *
 ****************************/

class ActBaseErrorMessageResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, error_message, ErrorMessage);
  ACT_JSON_FIELD(QString, parameter, Parameter);

 public:
  ActBaseErrorMessageResponse() {
    this->key_order_ =
        QList<QString>({QString("OpCode"), QString("StatusCode"), QString("ErrorMessage"), QString("Parameter")});

    // default value
    this->SetOpCode(static_cast<qint64>(ActWSCommandEnum::kTestStart));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kFailed));
    this->SetErrorMessage("");
    this->SetParameter("");
    this->SetSyncToOpcua(false);
    this->SetSyncToWebsocket(true);
  }

  ActBaseErrorMessageResponse(const ActWSCommandEnum &op_code_enum, const ActStatusBase &status_base)
      : ActBaseErrorMessageResponse() {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(status_base.GetStatus()));
    this->SetErrorMessage(status_base.GetErrorMessage());
  }

  ActBaseErrorMessageResponse(const qint64 &op_code, const ActStatusBase &status_base) : ActBaseErrorMessageResponse() {
    this->SetOpCode(op_code);
    this->SetStatusCode(static_cast<qint64>(status_base.GetStatus()));
    this->SetErrorMessage(status_base.GetErrorMessage());
  }
};

class ActDuplicatedErrorWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDuplicatedErrorParameter, parameter, Parameter);

 public:
  ActDuplicatedErrorWSResponse(const ActWSCommandEnum &op_code_enum, const ActDuplicatedError &act_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kDuplicated));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActDuplicatedErrorWSResponse() {}
};

class ActDeployFailedWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusDeployFailedParameter, parameter, Parameter);

 public:
  ActDeployFailedWSResponse(const ActWSCommandEnum &op_code_enum, const ActStatusDeployFailed &act_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kDeployFailed));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActDeployFailedWSResponse() {}
};

class ActWSResponseDataDeployIniResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, progress, Progress);
  // ACT_JSON_FIELD(qint64, monitor_project_id, MonitorProjectId);

 public:
  ActWSResponseDataDeployIniResult() {
    this->SetProgress(100);
    // this->SetMonitorProjectId(-1);
  }
  ActWSResponseDataDeployIniResult(const quint8 &progress) {
    this->SetProgress(progress);
    // this->SetMonitorProjectId(monitor_project_id);
  }
};

class ActDeployIniResultWSResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, progress, Progress);
  ACT_JSON_OBJECT(ActWSResponseDataDeployIniResult, data, Data);

 public:
  ActDeployIniResultWSResponse() {
    this->SetProgress(100);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kFinished));
  }

  ActDeployIniResultWSResponse(const ActWSCommandEnum &op_code_enum) : ActDeployIniResultWSResponse() {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetData(ActWSResponseDataDeployIniResult(100));
  }
};

class ActCompareFailedWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusCompareFailedParameter, parameter, Parameter);

 public:
  ActCompareFailedWSResponse(const ActWSCommandEnum &op_code_enum, const ActStatusCompareFailed &act_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kCompareFailed));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActCompareFailedWSResponse() {}
};

class ActCompareTopologyFailedWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusCompareTopologyFailedParameter, parameter, Parameter);

 public:
  ActCompareTopologyFailedWSResponse(const ActWSCommandEnum &op_code_enum,
                                     const ActStatusCompareTopologyFailed &act_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kCompareTopologyFailed));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActCompareTopologyFailedWSResponse() {}
};

class ActUpdateProjectTopologyFailedWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusUpdateProjectTopologyFailedParameter, parameter, Parameter);

 public:
  ActUpdateProjectTopologyFailedWSResponse(const ActWSCommandEnum &op_code_enum,
                                           const ActStatusUpdateProjectTopologyFailed &act_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kUpdateProjectTopologyFailed));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActUpdateProjectTopologyFailedWSResponse() {}
};

class ActLicenseSizeFailedWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActLicenseSizeFailedRequestParameter, parameter, Parameter);

 public:
  ActLicenseSizeFailedWSResponse(const ActWSCommandEnum &op_code_enum, const ActLicenseSizeFailedRequest &act_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kLicenseSizeFailed));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActLicenseSizeFailedWSResponse() {}
};

class ActNotFoundWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusNotFoundParameter, parameter, Parameter);

 public:
  ActNotFoundWSResponse(const ActWSCommandEnum &op_code_enum, const ActStatusNotFound &act_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kNotFound));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActNotFoundWSResponse() {}
};

class ActGetDeviceDataFailedWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusGetDeviceDataFailedParameter, parameter, Parameter);

 public:
  ActGetDeviceDataFailedWSResponse(const ActWSCommandEnum &op_code_enum, const ActStatusGetDeviceDataFailed &act_status)
      : ActGetDeviceDataFailedWSResponse() {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kGetDeviceDataFailed));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActGetDeviceDataFailedWSResponse() {}
};

class ActSetConfigFailedWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusSetConfigFailedParameter, parameter, Parameter);

 public:
  ActSetConfigFailedWSResponse(const ActWSCommandEnum &op_code_enum, const ActStatusSetConfigFailed &act_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kSetConfigFailed));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActSetConfigFailedWSResponse() {}
};

class ActInternalErrorWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusInternalErrorParameter, parameter, Parameter);

 public:
  ActInternalErrorWSResponse(const ActWSCommandEnum &op_code_enum, const ActStatusInternalError &act_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kInternalError));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActInternalErrorWSResponse() {}
};

class ActPcpInsufficientForTimeSlotWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusPcpInsufficientForTimeSlotParameter, parameter, Parameter);

 public:
  ActPcpInsufficientForTimeSlotWSResponse(const ActWSCommandEnum &op_code_enum,
                                          const ActStatusPcpInsufficientForTimeSlot &act_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kPcpInsufficientForTimeSlot));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActPcpInsufficientForTimeSlotWSResponse() {}
};

class ActStatusFeasibilityCheckFailedWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusFeasibilityCheckFailedParameter, parameter, Parameter);

 public:
  ActStatusFeasibilityCheckFailedWSResponse(const ActWSCommandEnum &op_code_enum,
                                            const ActStatusFeasibilityCheckFailed &act_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kFeasibilityCheckFailed));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActStatusFeasibilityCheckFailedWSResponse() {}
};

class ActRoutingDestinationUnreachableWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusRoutingDestinationUnreachableParameter, parameter, Parameter);

 public:
  ActRoutingDestinationUnreachableWSResponse(const ActWSCommandEnum &op_code_enum,
                                             const ActStatusRoutingDestinationUnreachable &act_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kRoutingDestinationUnreachable));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActRoutingDestinationUnreachableWSResponse() {}
};

class ActRoutingDeviceTypeIncapableWSResponse : public ActBaseErrorMessageResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusRoutingDeviceTypeIncapableParameter, parameter, Parameter);

 public:
  ActRoutingDeviceTypeIncapableWSResponse(const ActWSCommandEnum &op_code_enum,
                                          const ActStatusRoutingDeviceTypeIncapable &act_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kRoutingDeviceTypeIncapable));
    this->SetErrorMessage(act_status.GetErrorMessage());
    this->SetParameter(act_status.GetParameter());
  }

 private:
  ActRoutingDeviceTypeIncapableWSResponse() {}
};

// ------------------------------------------ Response Error Transfer ----------------------------------------------- //

inline std::shared_ptr<ActBaseErrorMessageResponse> ActWSResponseErrorTransfer(const ActWSCommandEnum &op_code_enum,
                                                                               ActStatusBase &act_status) {
  // qDebug() << act_status.ToString().toStdString().c_str();

  switch (act_status.GetStatus()) {
    case ActStatusType::kDuplicated:
      return std::make_shared<ActDuplicatedErrorWSResponse>(op_code_enum,
                                                            dynamic_cast<ActDuplicatedError &>(act_status));
    case ActStatusType::kRoutingDestinationUnreachable:
      return std::make_shared<ActRoutingDestinationUnreachableWSResponse>(
          op_code_enum, dynamic_cast<ActStatusRoutingDestinationUnreachable &>(act_status));

    case ActStatusType::kPcpInsufficientForTimeSlot:
      return std::make_shared<ActPcpInsufficientForTimeSlotWSResponse>(
          op_code_enum, dynamic_cast<ActStatusPcpInsufficientForTimeSlot &>(act_status));

    case ActStatusType::kRoutingDeviceTypeIncapable:
      return std::make_shared<ActRoutingDeviceTypeIncapableWSResponse>(
          op_code_enum, dynamic_cast<ActStatusRoutingDeviceTypeIncapable &>(act_status));

    case ActStatusType::kFeasibilityCheckFailed:
      return std::make_shared<ActStatusFeasibilityCheckFailedWSResponse>(
          op_code_enum, dynamic_cast<ActStatusFeasibilityCheckFailed &>(act_status));

    case ActStatusType::kInternalError:
      return std::make_shared<ActInternalErrorWSResponse>(op_code_enum,
                                                          dynamic_cast<ActStatusInternalError &>(act_status));
    case ActStatusType::kSetConfigFailed:
      return std::make_shared<ActSetConfigFailedWSResponse>(op_code_enum,
                                                            dynamic_cast<ActStatusSetConfigFailed &>(act_status));
    case ActStatusType::kGetDeviceDataFailed:
      return std::make_shared<ActGetDeviceDataFailedWSResponse>(
          op_code_enum, dynamic_cast<ActStatusGetDeviceDataFailed &>(act_status));

    case ActStatusType::kNotFound:
      return std::make_shared<ActNotFoundWSResponse>(op_code_enum, dynamic_cast<ActStatusNotFound &>(act_status));

    case ActStatusType::kDeployFailed:
      return std::make_shared<ActDeployFailedWSResponse>(op_code_enum,
                                                         dynamic_cast<ActStatusDeployFailed &>(act_status));
    case ActStatusType::kCompareFailed:
      return std::make_shared<ActCompareFailedWSResponse>(op_code_enum,
                                                          dynamic_cast<ActStatusCompareFailed &>(act_status));
    case ActStatusType::kCompareTopologyFailed:
      return std::make_shared<ActCompareTopologyFailedWSResponse>(
          op_code_enum, dynamic_cast<ActStatusCompareTopologyFailed &>(act_status));
    case ActStatusType::kUpdateProjectTopologyFailed:
      return std::make_shared<ActUpdateProjectTopologyFailedWSResponse>(
          op_code_enum, dynamic_cast<ActStatusUpdateProjectTopologyFailed &>(act_status));
    case ActStatusType::kLicenseSizeFailed:
      return std::make_shared<ActLicenseSizeFailedWSResponse>(op_code_enum,
                                                              dynamic_cast<ActLicenseSizeFailedRequest &>(act_status));

    default:
      return std::make_shared<ActBaseErrorMessageResponse>(op_code_enum, act_status);
  }
};

// ------------------------------------------------- Other Responses ------------------------------------------------ //

class ActWSResponseDataVersion : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, data_version, DataVersion);

 public:
  ActWSResponseDataVersion() { this->SetDataVersion(""); }
  ActWSResponseDataVersion(const QString &data_version) { this->SetDataVersion(data_version); }
};

class ActDataVersionWSResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActWSResponseDataVersion, data, Data);

 public:
  ActDataVersionWSResponse(const QString &data_version) {
    this->SetOpCode(static_cast<qint64>(ActWSCommandEnum::kGetProjectDataVersion));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kFinished));
    this->SetData(ActWSResponseDataVersion(data_version));
  }

 private:
  ActDataVersionWSResponse() {}
};

class ActProgressWSResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActProgressStatusParameter, data, Data);

 public:
  ActProgressWSResponse(const ActWSCommandEnum &op_code_enum, const ActStatusBase &status_base,
                        const quint8 &progress) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(status_base.GetStatus()));
    this->SetData(ActProgressStatusParameter(progress));
  }

  ActProgressWSResponse(const ActWSCommandEnum &op_code_enum, const ActProgressStatus &progress_status) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(progress_status.GetStatus()));
    this->SetData(ActProgressStatusParameter(progress_status.GetProgress()));
  }

 private:
  ActProgressWSResponse() {}
};

class ActWSResponseDataDevices : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, progress, Progress);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActDevice, devices, Devices);

 public:
  ActWSResponseDataDevices() { this->SetProgress(100); }
  ActWSResponseDataDevices(const quint8 &progress, const QList<ActDevice> &devices) {
    this->SetProgress(progress);
    this->SetDevices(devices);
  }
};

class ActDevicesResultWSResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActWSResponseDataDevices, data, Data);

 public:
  ActDevicesResultWSResponse(const ActWSCommandEnum &op_code_enum, const QList<ActDevice> &devices) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kFinished));
    this->SetData(ActWSResponseDataDevices(100, devices));
  }

  // XXX: unused
  // ActDevicesResultWSResponse(const ActWSCommandEnum& op_code_enum, const QSet<ActDevice>& devices) {
  //   this->SetOpCode(static_cast<qint64>(op_code_enum));
  //   this->SetStatusCode(static_cast<qint64>(ActStatusType::kFinished));
  //   QList<ActDevice> dev_list(devices.begin(), devices.end());
  //   this->SetData(ActWSResponseDataDevices(100, dev_list));
  // }

 private:
  ActDevicesResultWSResponse() {}
};

class ActWSResponseDataTopologyMappingResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, progress, Progress);
  ACT_JSON_OBJECT(ActTopologyMappingResult, topology_mapping_result, TopologyMappingResult);

 public:
  ActWSResponseDataTopologyMappingResult() { this->SetProgress(100); }
  ActWSResponseDataTopologyMappingResult(const quint8 &progress,
                                         const ActTopologyMappingResult &topology_mapping_result) {
    this->SetProgress(progress);
    this->SetTopologyMappingResult(topology_mapping_result);
  }
};

class ActTopologyMappingResultWSResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, progress, Progress);
  ACT_JSON_OBJECT(ActWSResponseDataTopologyMappingResult, data, Data);

 public:
  ActTopologyMappingResultWSResponse() {
    this->SetProgress(100);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kFinished));
  }

  ActTopologyMappingResultWSResponse(const ActWSCommandEnum &op_code_enum,
                                     const ActTopologyMappingResult &topology_mapping_result)
      : ActTopologyMappingResultWSResponse() {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetData(ActWSResponseDataTopologyMappingResult(100, topology_mapping_result));
  }
};

class ActWSResponseDataScanMappingResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, progress, Progress);
  ACT_JSON_OBJECT(ActScanMappingResult, scan_mapping_result, ScanMappingResult);

 public:
  ActWSResponseDataScanMappingResult() { this->SetProgress(100); }
  ActWSResponseDataScanMappingResult(const quint8 &progress, const ActScanMappingResult &scan_mapping_result) {
    this->SetProgress(progress);
    this->SetScanMappingResult(scan_mapping_result);
  }
};

class ActScanMappingResultWSResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, progress, Progress);
  ACT_JSON_OBJECT(ActWSResponseDataScanMappingResult, data, Data);

 public:
  ActScanMappingResultWSResponse() {
    this->SetProgress(100);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kFinished));
  }

  ActScanMappingResultWSResponse(const ActWSCommandEnum &op_code_enum, const ActScanMappingResult &scan_mapping_result)
      : ActScanMappingResultWSResponse() {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetData(ActWSResponseDataScanMappingResult(100, scan_mapping_result));
  }
};

class ActWSResponseDataScanResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, progress, Progress);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActAutoScanResultItem, scan_result, ScanResult);

 public:
  ActWSResponseDataScanResult() { this->SetProgress(100); }
  ActWSResponseDataScanResult(const quint8 &progress, const QList<ActAutoScanResultItem> &scan_result) {
    this->SetProgress(progress);
    this->SetScanResult(scan_result);
  }
};

class ActScanResultWSResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActWSResponseDataScanResult, data, Data);

 public:
  ActScanResultWSResponse(const ActWSCommandEnum &op_code_enum, const QList<ActAutoScanResultItem> &scan_result) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kFinished));
    this->SetData(ActWSResponseDataScanResult(100, scan_result));
  }

 private:
  ActScanResultWSResponse() {}
};

class ActWSResponseDataDevicesSequence : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, progress, Progress);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceDistanceEntry, sorted_result, SortedResult);

 public:
  ActWSResponseDataDevicesSequence() { this->SetProgress(100); }
  ActWSResponseDataDevicesSequence(const quint8 &progress, const QList<ActDeviceDistanceEntry> &dev_dist_entries) {
    this->SetProgress(progress);
    this->SetSortedResult(dev_dist_entries);
  }
};

class ActDevicesSequenceResultWSResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActWSResponseDataDevicesSequence, data, Data);

 public:
  ActDevicesSequenceResultWSResponse(const ActWSCommandEnum &op_code_enum,
                                     const QList<ActDeviceDistanceEntry> &dev_dist_entries) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kFinished));
    this->SetData(ActWSResponseDataDevicesSequence(100, dev_dist_entries));
  }

 private:
  ActDevicesSequenceResultWSResponse() {}
};

class ActDeviceConfigResultWSResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDeviceConfigureResult, data, Data);

 public:
  ActDeviceConfigResultWSResponse(const ActWSCommandEnum &op_code_enum, const ActStatusType &status_type,
                                  const ActDeviceConfigureResult &dev_config_result) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(status_type));
    this->SetData(dev_config_result);
  }

 private:
  ActDeviceConfigResultWSResponse() {}
};

class ActDeviceEventLogResultWSResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDeviceEventLogResult, data, Data);

 public:
  QList<QString> key_order_;
  ActDeviceEventLogResultWSResponse(const ActWSCommandEnum &op_code_enum, const ActStatusType &status_type,
                                    const ActDeviceEventLogResult &dev_event_log_result) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(status_type));
    this->SetData(dev_event_log_result);
  }

 private:
  ActDeviceEventLogResultWSResponse() { this->key_order_ = QList<QString>({QString("OpCode"), QString("StatusCode")}); }
};

class ActIntelligentResponseWSResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActIntelligentResponse, data, Data);

 public:
  ActIntelligentResponseWSResponse(const ActWSCommandEnum &op_code_enum, const ActStatusType &status_type,
                                   const ActIntelligentResponse &intelligent_response) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(status_type));
    this->SetData(intelligent_response);
  }

 private:
  ActIntelligentResponseWSResponse() {}
};

class ActProbeDeviceResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, ip_address, IpAddress);
  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(qint64, device_profile_id, DeviceProfileId);
  ACT_JSON_ENUM(ActStatusType, status, Status);

 public:
  /**
   * @brief Construct a new Act Ipv 4 object
   *
   */
  ActProbeDeviceResult() {
    ip_address_ = "";
    model_name_ = "";
    device_profile_id_ = -1;
    status_ = ActStatusType::kFailed;
  }

  ActProbeDeviceResult(const QString &ip_address, const ActStatusType &status) : ActProbeDeviceResult() {
    ip_address_ = ip_address;
    status_ = status;
  }

  ActProbeDeviceResult(const QString &ip_address, const QString &model_name, const qint64 &device_profile_id,
                       const ActStatusType &status) {
    ip_address_ = ip_address;
    model_name_ = model_name;
    device_profile_id_ = device_profile_id;
    status_ = status;
  }
};

class ActProbeDeviceResultWSResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActProbeDeviceResult, data, Data);

 public:
  ActProbeDeviceResultWSResponse(const ActWSCommandEnum &op_code_enum, const ActStatusType &status_type,
                                 const ActProbeDeviceResult &dev_ip_result) {
    this->SetOpCode(static_cast<qint64>(op_code_enum));
    this->SetStatusCode(static_cast<qint64>(status_type));
    this->SetData(dev_ip_result);
  }

 private:
  ActProbeDeviceResultWSResponse() {}
};

class ActFeaturesAvailableStatusResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, undo, Undo);
  ACT_JSON_FIELD(bool, redo, Redo);
  ACT_JSON_FIELD(bool, deploy, Deploy);

 public:
  /**
   * @brief Construct a new Act Ipv 4 object
   *
   */
  ActFeaturesAvailableStatusResult() {
    undo_ = false;
    redo_ = false;
    deploy_ = false;
  }

  ActFeaturesAvailableStatusResult(const bool &undo, const bool &redo, const bool &deploy)
      : ActFeaturesAvailableStatusResult() {
    undo_ = undo;
    redo_ = redo;
    deploy_ = deploy;
  }
};

class ActFeaturesAvailableStatusWSResponse : public ActBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActFeaturesAvailableStatusResult, data, Data);

 public:
  ActFeaturesAvailableStatusWSResponse(const ActFeaturesAvailableStatusResult &status) {
    this->SetOpCode(static_cast<qint64>(ActWSCommandEnum::kFeaturesAvailableStatus));
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kSuccess));
    this->SetData(status);
  }

 private:
  ActFeaturesAvailableStatusWSResponse() {}
};

/**
 * @brief The websocket type enum class
 *
 */
enum class ActWSTypeEnum { kSystem = 1, kProject = 2, kSpecified = 3 };