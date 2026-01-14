#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QHash>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::CheckTopology(const ActTopology &topology) {
  ACT_STATUS_INIT();

  QSet<ActDevice> devices = topology.GetDevices();
  QSet<ActLink> links = topology.GetLinks();
  QSet<ActStream> streams = topology.GetStreams();
  QSet<ActTopology> &topologies = this->GetTopologySet();

  // Check topology name is valid
  // Check if contains special characters (\ / : * ? “ < > |) that can not use in windows
  const QString topology_name = topology.GetTopologyName();
  const QString forbidden_char = "\\/:*?\"<>|";
  for (const QChar &c : forbidden_char) {
    if (topology_name.contains(c)) {
      QString error_msg = QString("Topology name contains forbidden characters, the name is %1").arg(topology_name);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Check if trailing space and multiple consecutive spaces
  if (topology_name.endsWith(" ") || topology_name.contains(QRegExp("\\s{2,}"))) {
    QString error_msg = QString("Topology name contains multiple consecutive spaces or end with space, the name is %1")
                            .arg(topology_name);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // [feat:2722] Check the topology name is unique
  for (ActTopology t : topologies) {
    if (t.GetTopologyName() == topology.GetTopologyName()) {
      qCritical() << "The topology name " << topology.GetTopologyName() << "is duplicated";
      return std::make_shared<ActDuplicatedError>(topology.GetTopologyName());
    }
  }

  // Both side devices of the link should be saved
  for (ActLink link : links) {
    if (!devices.contains(ActDevice(link.GetSourceDeviceId())) ||
        !devices.contains(ActDevice(link.GetDestinationDeviceId()))) {
      QString error_msg = QString("Topology (%1) - Both side devices of the link %2 should be saved")
                              .arg(topology.GetTopologyName())
                              .arg(QString::number(link.GetId()));
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Both talker & listener of the stream should be saved
  for (ActStream stream : streams) {
    if (!devices.contains(ActDevice(stream.GetTalker().GetEndStationInterface().GetDeviceId()))) {
      QString error_msg = QString("Topology (%1) - The stream %2's talker device %3 should be saved")
                              .arg(topology.GetTopologyName())
                              .arg(stream.GetStreamName())
                              .arg(QString::number(stream.GetTalker().GetEndStationInterface().GetDeviceId()));
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    QList<ActListener> listeners = stream.GetListeners();
    for (ActListener listener : listeners) {
      if (!devices.contains(ActDevice(listener.GetEndStationInterface().GetDeviceId()))) {
        QString error_msg = QString("Topology (%1) - The stream %2's listener device %3 should be saved")
                                .arg(topology.GetTopologyName())
                                .arg(stream.GetStreamName())
                                .arg(QString::number(listener.GetEndStationInterface().GetDeviceId()));
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }
  }

  return act_status;
}

ACT_STATUS ActCore::GetTopology(const qint64 &id, ActTopology &topology) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  act_status = ActGetItemById<ActTopology>(this->GetTopologySet(), id, topology);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Topology id:" << id << "not found";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::SaveTopology(const ActTopologyCreateParam &topology_param) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QSet<ActDevice> devices;
  QSet<ActLink> links;
  QSet<ActStream> streams;

  // Fetch the project first
  ActProject project;
  act_status = ActGetItemById<ActProject>(this->GetProjectSet(), topology_param.GetProjectId(), project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "The project id" << QString::number(topology_param.GetProjectId())
                << "matches no project configuration";
    return std::make_shared<ActStatusNotFound>(QString::number(topology_param.GetProjectId()));
  }

  // Fetch the device, link, stream from the project
  for (qint64 device_id : topology_param.GetDevices()) {
    ActDevice device;
    act_status = ActGetItemById<ActDevice>(project.GetDevices(), device_id, device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "The device id" << QString::number(device_id) << "matches no device configuration";
      return std::make_shared<ActStatusNotFound>(QString::number(device_id));
    }
    devices.insert(device);
  }

  for (qint64 link_id : topology_param.GetLinks()) {
    ActLink link;
    act_status = ActGetItemById<ActLink>(project.GetLinks(), link_id, link);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "The link id" << QString::number(link_id) << "matches no link configuration";
      return std::make_shared<ActStatusNotFound>(QString::number(link_id));
    }
    links.insert(link);
  }

  for (qint64 stream_id : topology_param.GetStreams()) {
    ActStream stream;
    act_status = ActGetItemById<ActStream>(project.GetStreams(), stream_id, stream);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "The stream id" << QString::number(stream_id) << "matches no stream configuration";
      return std::make_shared<ActStatusNotFound>(QString::number(stream_id));
    }
    streams.insert(stream);
  }

  // Save it to the topology instance
  ActTopology topology;
  topology.SetTopologyName(topology_param.GetTopologyName());
  topology.SetDevices(devices);
  topology.SetLinks(links);
  topology.SetStreams(streams);

  // check the topology configuration is correct
  act_status = ActCore::CheckTopology(topology);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check topology failed with topology:" << topology.GetTopologyName().toStdString().c_str();
    return act_status;
  }

  // Generate a new unique id
  QSet<ActTopology> &topology_set = this->GetTopologySet();
  qint64 id;
  act_status = this->GenerateUniqueId<ActTopology>(topology_set, this->last_assigned_topology_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot get an available unique id";
    return act_status;
  }
  topology.SetId(id);

  topology.SetDataVersion(ACT_TOPOLOGY_DATA_VERSION);

  // Save it to the topology management
  topology_set.insert(topology);

  // Write to db
  act_status = act::database::topology::WriteData(topology);
  act_status = act::database::topology::SaveTopologyIcon(id, topology_param.GetImage());

  // Send update msg
  ActTopologyPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kCreate, topology, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_msg);

  return act_status;
}

ACT_STATUS ActCore::AppendStream(ActProject &project, ActStream &topo_stream) {
  ACT_STATUS_INIT();

  QSet<ActStream> &stream_set = project.GetStreams();

  // check the stream is unique
  for (ActStream s : stream_set) {
    if (s.GetStreamName() == topo_stream.GetStreamName()) {
      qCritical() << "The stream name" << topo_stream.GetStreamName() << "is duplicated";
      return std::make_shared<ActDuplicatedError>(topo_stream.GetStreamName());
    }
  }

  // Generate a new unique id
  qint64 id;
  act_status = this->GenerateUniqueId<ActStream>(stream_set, project.last_assigned_stream_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot get an available unique id";
    return act_status;
  }
  topo_stream.SetId(id);

  // [feat:793] Set the stream status to planned (802.1Qdj)
  topo_stream.SetStreamStatus(ActStreamStatusEnum::kPlanned);

  // interface capability in the listener???
  // stream rank????

  // Insert the stream to project
  stream_set.insert(topo_stream);

  // Send update msg to temp
  InsertStreamMsgToNotificationTmp(
      ActStreamPatchUpdateMsg(ActPatchUpdateActionEnum::kCreate, project.GetId(), topo_stream, true));

  return act_status;
}

ACT_STATUS ActCore::AppendLink(ActProject &project, ActLink &topo_link) {
  ACT_STATUS_INIT();

  act_status = this->CheckLink(project, topo_link);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check link failed";
    return act_status;
  }

  QSet<ActLink> &link_set = project.GetLinks();

  // Generate a new unique id
  qint64 id;
  act_status = this->GenerateUniqueId<ActLink>(link_set, project.last_assigned_link_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot get an available unique id";
    return act_status;
  }
  topo_link.SetId(id);

  // Insert the link to project
  link_set.insert(topo_link);

  // Send update msg to temp
  InsertLinkMsgToNotificationTmp(
      ActLinkPatchUpdateMsg(ActPatchUpdateActionEnum::kCreate, project.GetId(), topo_link, true));

  return act_status;
}

ACT_STATUS ActCore::AppendDevice(ActProject &project, ActDevice &topo_device) {
  ACT_STATUS_INIT();

  QSet<ActDevice> device_set = project.GetDevices();

  // Add license control
  /* quint16 lic_device_qty = this->GetLicense().GetSize().GetDeviceQty();
  if ((device_set.size() >= lic_device_qty) &&
      (lic_device_qty != 0)) {  // [feat:2852] License - device size = 0 means unlimited
    qCritical() << "Append device" << topo_device.GetIpv4().GetIpAddress() << "failed";
    QString error_msg = QString("Project (%1) - The device size exceeds the limit: %2")
                            .arg(project.GetProjectName())
                            .arg(lic_device_qty);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActLicenseSizeFailedRequest>("Device Size", lic_device_qty);
  } */

  // Check the device does not exist with duplicated Ipaddress
  for (auto other_dev : device_set) {
    if (other_dev.GetIpv4().GetIpAddress() == topo_device.GetIpv4().GetIpAddress()) {
      qCritical() << "The IP address" << topo_device.GetIpv4().GetIpAddress() << "is duplicated";
      return std::make_shared<ActDuplicatedError>(topo_device.GetIpv4().GetIpAddress());
    }
  }

  // Generate a new unique id
  qint64 id;
  act_status = this->GenerateUniqueId<ActDevice>(device_set, project.last_assigned_device_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot get an available unique id";
    return act_status;
  }
  topo_device.SetId(id);

  QList<ActInterface> &dev_intf_list = topo_device.GetInterfaces();
  for (ActInterface &intf : dev_intf_list) {
    // Renew device id in the interface
    intf.SetDeviceId(id);
    intf.SetIpAddress(topo_device.GetIpv4().GetIpAddress());
  }

  act_status = this->CheckDevice(project, topo_device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check device failed with device id:" << topo_device.GetId();
    return act_status;
  }

  // Send update msg to temp
  InsertDeviceMsgToNotificationTmp(
      ActDevicePatchUpdateMsg(ActPatchUpdateActionEnum::kCreate, project.GetId(), topo_device, true));

  // Insert the device to project
  device_set.insert(topo_device);
  project.SetDevices(device_set);

  return act_status;
}

ACT_STATUS ActCore::AppendTopology(qint64 &project_id, const ActTopology &topology_cfg) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Get Topology by id
  ActTopology topology;
  act_status = this->GetTopology(topology_cfg.GetId(), topology);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get topology failed with topology id:" << topology_cfg.GetId();
    return act_status;
  }

  QSet<ActDevice> topo_cfg_device_set = topology_cfg.GetDevices();
  QSet<ActStream> topo_cfg_stream_set = topology_cfg.GetStreams();

  QSet<ActDevice> &topo_device_set = topology.GetDevices();
  QSet<ActLink> &topo_link_set = topology.GetLinks();
  QSet<ActStream> &topo_stream_set = topology.GetStreams();

  QHash<qint64, qint64> new_device_id_mapping_tbl;   // <Old Device Id, New Device Id>
  QHash<qint64, QString> new_device_ip_mapping_tbl;  // <Old Device Id, New Device IP>

  QMap<qint64, QMap<qint64, QPair<QString, qint64>>>
      new_mac_mapping_tbl;  // [feat:2454] new_mac_mapping_tbl[old dev id][interface id] = <"00-00-00-00-00-01",
                            // 000000000001>;

  for (ActDevice topo_device : topo_device_set) {
    qint64 old_device_id = topo_device.GetId();
    ActDevice device_cfg;
    act_status = ActGetItemById<ActDevice>(topo_cfg_device_set, old_device_id, device_cfg);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Device id:" << old_device_id << "not found";
      return act_status;
    }

    // Assign parameters to the saved items
    topo_device.SetCoordinate(device_cfg.GetCoordinate());
    topo_device.SetIpv4(device_cfg.GetIpv4());

    // Append the device to the project
    act_status = ActCore::AppendDevice(project, topo_device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Append device :" << topo_device.GetIpv4().GetIpAddress().toStdString().c_str() << "failed";
      return act_status;
    }

    // Update the new device id to replace the related configurations
    new_device_id_mapping_tbl[old_device_id] = topo_device.GetId();
    new_device_ip_mapping_tbl[old_device_id] = topo_device.GetIpv4().GetIpAddress();

    QMap<qint64, QPair<QString, qint64>> intf_info;
    QPair<QString, qint64> mac_pair = qMakePair(topo_device.GetMacAddress(), topo_device.mac_address_int);
    intf_info[0] = mac_pair;  // device MAC

    QList<ActInterface> intf_list = topo_device.GetInterfaces();
    for (ActInterface intf : intf_list) {
      mac_pair = qMakePair(intf.GetMacAddress(), intf.mac_address_int);
      intf_info[intf.GetInterfaceId()] = mac_pair;
    }

    new_mac_mapping_tbl[old_device_id] = intf_info;
  }

  for (ActLink topo_link : topo_link_set) {
    qint64 new_src_dev_id = new_device_id_mapping_tbl[topo_link.GetSourceDeviceId()];
    qint64 new_dst_dev_id = new_device_id_mapping_tbl[topo_link.GetDestinationDeviceId()];
    topo_link.SetSourceDeviceId(new_src_dev_id);
    topo_link.SetDestinationDeviceId(new_dst_dev_id);

    // Append the link to the project
    act_status = ActCore::AppendLink(project, topo_link);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Append link failed";
      return act_status;
    }
  }

  for (ActStream topo_stream : topo_stream_set) {
    ActStream stream_cfg;
    act_status = ActGetItemById<ActStream>(topo_cfg_stream_set, topo_stream.GetId(), stream_cfg);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Stream id:" << topo_stream.GetId() << "not found";
      return act_status;
    }

    // Assign parameters to the saved items
    topo_stream.SetStreamName(stream_cfg.GetStreamName());

    // Replace talker device id
    ActTalker &talker = topo_stream.GetTalker();
    ActInterface &talker_intf = talker.GetEndStationInterface();
    qint64 old_talker_device_id = talker_intf.GetDeviceId();
    talker_intf.SetDeviceId(new_device_id_mapping_tbl[old_talker_device_id]);
    talker_intf.SetIpAddress(new_device_ip_mapping_tbl[old_talker_device_id]);

    // [feat:2495] Replace MAC address
    QPair<QString, qint64> mac_pair = new_mac_mapping_tbl[old_talker_device_id][talker_intf.GetInterfaceId()];
    talker_intf.SetMacAddress(mac_pair.first);
    talker_intf.mac_address_int = mac_pair.second;

    ActStreamId &stream_id = topo_stream.GetStreamId();
    stream_id.SetMacAddress(mac_pair.first);

    QList<ActListener> &listener_list = topo_stream.GetListeners();
    for (ActListener &listener : listener_list) {
      ActInterface &listener_intf = listener.GetEndStationInterface();
      qint64 old_listener_device_id = listener_intf.GetDeviceId();
      listener_intf.SetDeviceId(new_device_id_mapping_tbl[old_listener_device_id]);
      listener_intf.SetIpAddress(new_device_ip_mapping_tbl[old_listener_device_id]);

      // [feat:2495] Replace MAC address
      QPair<QString, qint64> listener_mac_pair =
          new_mac_mapping_tbl[old_listener_device_id][listener_intf.GetInterfaceId()];
      listener_intf.SetMacAddress(listener_mac_pair.first);
      listener_intf.mac_address_int = listener_mac_pair.second;
    }

    act_status = ActCore::AppendStream(project, topo_stream);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Append stream failed";
      return act_status;
    }
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateTopology(ActTopology &topology) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // check the topology configuration is correct
  act_status = ActCore::CheckTopology(topology);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check topology failed with topology:" << topology.GetTopologyName().toStdString().c_str();
    return act_status;
  }

  QSet<ActTopology> &topology_set = this->GetTopologySet();

  // Check the item does exist by id
  typename QSet<ActTopology>::const_iterator iterator;
  iterator = topology_set.find(topology);
  if (iterator != topology_set.end()) {
    // If yes, delete it
    topology_set.erase(iterator);
  }

  if (topology.GetId() == -1) {
    // Generate a new unique id
    qint64 id;
    act_status = this->GenerateUniqueId<ActTopology>(topology_set, this->last_assigned_topology_id_, id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Cannot get an available unique id";
      return act_status;
    }
    topology.SetId(id);
  }

  topology.SetDataVersion(ACT_TOPOLOGY_DATA_VERSION);

  // Insert the topology to core set
  topology_set.insert(topology);

  // Write to db
  act_status = act::database::topology::WriteData(topology);

  // Send update msg
  ActTopologyPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, topology, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_msg);

  return act_status;
}

ACT_STATUS ActCore::DeleteTopology(const qint64 &topology_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QSet<ActTopology> &topology_set = this->GetTopologySet();

  // Check the item does exist by id
  typename QSet<ActTopology>::const_iterator iterator;
  iterator = topology_set.find(ActTopology(topology_id));
  if (iterator == topology_set.end()) {
    QString error_msg = QString("Delete topology failed, cannot found topology id %1").arg(topology_id);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  ActTopology topology = (*iterator);
  QString topology_name = topology.GetTopologyName();

  // If yes, delete it
  topology_set.erase(iterator);

  // Write to db
  act_status = act::database::topology::DeleteTopologyFile(topology_id, topology_name);

  // Send update msg
  ActTopologyPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kDelete, topology, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_msg);

  return act_status;
}

ACT_STATUS ActCore::CopyTopology(qint64 &project_id, QList<qint64> &dev_ids, QList<qint64> &copied_dev_ids,
                                 QList<qint64> &copied_link_ids) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Copy device
  act_status = this->CopyTopology(project, dev_ids, copied_dev_ids, copied_link_ids);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Copy device failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActProjectPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, project, true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project.GetId());

  return act_status;
}

ACT_STATUS ActCore::CopyTopology(ActProject &project, QList<qint64> &dev_ids, QList<qint64> &copied_dev_ids,
                                 QList<qint64> &copied_link_ids) {
  ACT_STATUS_INIT();

  QHash<qint64, qint64> new_dev_id_mapping_tbl;  // <Old Device Id, New Device Id>

  QSet<ActDevice> dev_set = project.GetDevices();
  QSet<ActLink> link_set = project.GetLinks();

  QSet<ActDevice> new_dev_set;
  QSet<ActLink> new_link_set;

  // For each copied device id, fetch the device config
  for (qint64 old_dev_id : dev_ids) {
    // Get old_device by id
    ActDevice old_dev;
    act_status = project.GetDeviceById(old_dev, old_dev_id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Get old device failed with device id:" << old_dev_id;
      return act_status;
    }

    ActDevice new_dev = old_dev;

    // Generate a new unique id
    qint64 new_dev_id;
    act_status = this->GenerateUniqueId<ActDevice>(dev_set, project.last_assigned_device_id_, new_dev_id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Cannot get an available unique id";
      return act_status;
    }
    new_dev.SetId(new_dev_id);

    qint64 x = old_dev.GetCoordinate().GetX() + 100;
    qint64 y = old_dev.GetCoordinate().GetY() + 100;
    ActCoordinate new_coordinate(x, y);
    while (project.used_coordinates_.contains(new_coordinate)) {
      x += 100;
      y += 100;
      new_coordinate.SetX(x);
      new_coordinate.SetY(y);
    }
    new_dev.SetCoordinate(new_coordinate);

    project.AutoAssignIP<ActDevice>(new_dev);

    // Handle the Connection config
    this->HandleConnectionConfigField(new_dev, old_dev, project.GetProjectSetting());

    // Set interface fake MAC address & clear used flag of all interface
    // If there are any link be copied, it will be set later
    QList<ActInterface> new_dev_intf_list;
    QList<ActInterface> dev_intf_list = new_dev.GetInterfaces();
    for (ActInterface intf : dev_intf_list) {
      intf.SetUsed(false);
      intf.SetManagement(false);
      intf.SetDeviceId(new_dev_id);
      intf.SetIpAddress(new_dev.GetIpv4().GetIpAddress());
      new_dev_intf_list.append(intf);
    }

    // Insert the interface list to the new copied device
    new_dev.SetInterfaces(new_dev_intf_list);

    // Set device role to unknown
    new_dev.SetDeviceRole(ActDeviceRoleEnum::kUnknown);

    // Update the new device id to the mapping table
    new_dev_id_mapping_tbl[old_dev.GetId()] = new_dev.GetId();

    // for send ws notification
    new_dev_set.insert(new_dev);
  }

  // For each link, check the both side device id are in the copied device list
  // If yes, create a new link for the copied new devices
  for (ActLink link : link_set) {
    if (dev_ids.contains(link.GetSourceDeviceId()) && dev_ids.contains(link.GetDestinationDeviceId())) {
      link.SetSourceDeviceId(new_dev_id_mapping_tbl[link.GetSourceDeviceId()]);
      link.SetDestinationDeviceId(new_dev_id_mapping_tbl[link.GetDestinationDeviceId()]);

      // Get source/destination device & interface by link
      ActDevice new_src_dev;
      ActDevice new_dst_dev;
      ActInterface new_src_intf;
      ActInterface new_dst_intf;
      act_status = GetDeviceAndInterfacePair(new_dev_set, link, new_src_dev, new_dst_dev, new_src_intf, new_dst_intf);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Get link pair failed";
        return act_status;
      }

      // Generate a new unique id
      qint64 new_link_id;
      act_status = this->GenerateUniqueId<ActLink>(link_set, project.last_assigned_link_id_, new_link_id);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Cannot get an available unique id";
        return act_status;
      }
      link.SetId(new_link_id);
      link.SetSourceDeviceIp(new_src_dev.GetIpv4().GetIpAddress());
      link.SetDestinationDeviceIp(new_dst_dev.GetIpv4().GetIpAddress());

      // For send ws notification
      new_link_set.insert(link);
    }
  }

  // Insert the device to project
  QList<ActDevice> device_list = new_dev_set.toList();
  const bool from_bag = false;
  act_status = this->CreateDevices(project, device_list, from_bag, copied_dev_ids);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Create Devices failed";
    return act_status;
  }

  // Insert the link to project
  QList<ActLink> link_list = new_link_set.toList();
  if (!link_list.isEmpty()) {
    act_status = this->CreateLinks(project, link_list, copied_link_ids);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Create Devices failed";
      return act_status;
    }
  }

  // Update copy device in RSTP group
  QSet<ActRSTP> rstp_groups = project.GetTopologySetting().GetRedundantGroup().GetRSTP();
  for (ActRSTP rstp : rstp_groups) {
    QSet<qint64> &devices = rstp.GetDevices();
    for (ActDevice new_dev : new_dev_set) {
      if (new_dev.GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
          new_dev.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation &&
          new_dev.GetDeviceType() != ActDeviceTypeEnum::kSwitch) {
        continue;
      }
      devices.insert(new_dev.GetId());
    }
    act_status = act::core::g_core.UpdateRedundantRSTP(project, rstp);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Update rstp failed" << rstp.ToString().toStdString().c_str();
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS ActCore::CheckTopologyLoop(qint64 &project_id, ActTopologyStatus &topology_status) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Check topology loop
  if (project.GetDevices().isEmpty()) {
    return ACT_STATUS_SUCCESS;
  }
  ActDevice device = *project.GetDevices().begin();

  QStack<qint64> DFS_stack;
  DFS_stack.push(device.GetId());

  QSet<qint64> visited;
  while (!DFS_stack.isEmpty()) {
    qint64 device_id = DFS_stack.pop();
    ActDevice device;
    project.GetDeviceById(device, device_id);

    for (ActInterface intf : device.GetInterfaces()) {
      ActLink link;
      act_status = project.GetLinkByInterfaceId(link, device_id, intf.GetInterfaceId());
      if (IsActStatusNotFound(act_status)) {
        continue;
      }

      ActDevice neighbor;
      qint64 neighbor_id =
          (link.GetSourceDeviceId() == device_id) ? link.GetDestinationDeviceId() : link.GetSourceDeviceId();
      project.GetDeviceById(neighbor, neighbor_id);

      if (neighbor.GetDeviceType() == ActDeviceTypeEnum::kEndStation || visited.contains(neighbor_id)) {
        continue;
      }

      if (DFS_stack.contains(neighbor_id)) {
        topology_status.SetLoop(true);
        return ACT_STATUS_SUCCESS;
      }

      DFS_stack.push(neighbor_id);
    }
    visited.insert(device_id);

    if (DFS_stack.isEmpty() && visited.count() != project.GetDevices().count()) {
      for (ActDevice device : project.GetDevices()) {
        if (visited.contains(device.GetId())) {
          continue;
        }
        DFS_stack.push(device.GetId());
        break;
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

qint64 GetCommonHighestSpeed(ActInterface &src_intf, ActInterface &dst_intf) {
  // Set the link speed to the both supported maximum speed of the interfaces
  QList<qint64> src_intf_support_speeds = src_intf.GetSupportSpeeds();
  QList<qint64> dst_intf_support_speeds = dst_intf.GetSupportSpeeds();

  // Find the common supported speeds

  // Convert the interface supported speeds list to a QSet
  QSet<qint64> src_set(src_intf_support_speeds.begin(), src_intf_support_speeds.end());
  QSet<qint64> dst_set(dst_intf_support_speeds.begin(), dst_intf_support_speeds.end());

  // Find the intersection of the two sets to get the common supported speeds
  QSet<qint64> common_speeds = src_set.intersect(dst_set);

  // Find the maximum common speed
  qint64 max_common_speed = 0;
  if (!common_speeds.isEmpty()) {
    max_common_speed = *std::max_element(common_speeds.begin(), common_speeds.end());
  }

  return max_common_speed;
}

ACT_STATUS ActCore::CreateTopologyTemplate(qint64 &project_id, ActTopologyTemplate &topology_template,
                                           QList<qint64> &created_device_ids, QList<qint64> &created_link_ids) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QList<ActDevice> device_list;
  QList<ActLink> link_list;

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Get coordinate from the template
  ActCoordinate x_y = topology_template.GetCoordinate();

  // Get device count from the template
  qint64 device_count = topology_template.GetDeviceCount();

  // Get IP list from the template
  QList<QString> ip_list = topology_template.GetIPList();

  // Check the all the IP in the list is valid
  for (QString ip : ip_list) {
    ActIpv4 ipv4(ip);
    if (!ipv4.IsValidIpAddress()) {
      QString error_msg = QString("The IP address %1 is invalid").arg(ip);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    // Check the device does not exist with duplicated IP address
    quint32 ip_num = 0;
    ActIpv4::AddressStrToNumber(ip, ip_num);
    if (project.used_ip_addresses_.contains(ip_num)) {
      qCritical() << "The IP address" << ip << "is duplicated";
      return std::make_shared<ActDuplicatedError>(ip);
    }
  }

  // Get device profile from the template
  qint64 device_profile_id = topology_template.GetDeviceProfileId();

  // Get device profile from the project for checking the interface count
  const QSet<ActDeviceProfile> &device_profiles = this->GetDeviceProfileSet();
  ActDeviceProfile device_profile;
  act_status = ActGetItemById<ActDeviceProfile>(device_profiles, device_profile_id, device_profile);
  if (!IsActStatusSuccess(act_status)) {
    QString not_found_elem = QString("DeviceProfile(%1)").arg(device_profile_id);

    qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
    qCritical() << __func__ << QString("The device_profiles size: %1").arg(device_profiles.size());
    return std::make_shared<ActStatusNotFound>(not_found_elem);
  }

  // Check interface count is enough
  switch (topology_template.GetType()) {
    case ActTopologyTemplateTypeEnum::kLine: {
      if (device_count > 2) {
        if (device_profile.GetInterfaces().size() < 2) {
          QString err_msg = QString("The device model does not have enough interfaces");
          qCritical() << err_msg.toStdString().c_str();
          return std::make_shared<ActBadRequest>(err_msg);
        }
      }
    } break;
    case ActTopologyTemplateTypeEnum::kRing: {
      if (device_count > 2) {
        if (device_profile.GetInterfaces().size() < 2) {
          QString err_msg = QString("The device model does not have enough interfaces");
          qCritical() << err_msg.toStdString().c_str();
          return std::make_shared<ActBadRequest>(err_msg);
        }
      }
    } break;
    case ActTopologyTemplateTypeEnum::kStar: {
      if (device_profile.GetInterfaces().size() < (device_count - 1)) {
        QString err_msg = QString("The device model does not have enough interfaces");
        qCritical() << err_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(err_msg);
      }
    } break;
    default: {
      qCritical() << "Unknown topology template type";
      return std::make_shared<ActBadRequest>("Unknown topology template type");
    }
  }

  for (int i = 0; i < device_count; i++) {
    // Create switch 1
    ActDevice device;
    device.SetDeviceProfileId(device_profile_id);

    // Auto assign IP from the project setting
    if (i < ip_list.size()) {
      ActIpv4 ipv4;
      ipv4.AutoAssignIPSubnetMask(ip_list[i]);
      device.SetIpv4(ipv4);
    }

    device_list.append(device);
  }

  // Create devices to project
  const bool from_bag = false;
  act_status = this->CreateDevices(project, device_list, from_bag, created_device_ids);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Create Devices failed";
    return act_status;
  }

  // According to the topology template type, create the links between devices
  // And also calculate the coordinate of the devices to prevent device icon overlap
  // Assuming ActDeviceCoordinate is a class that can be used in a QSet
  QSet<ActDeviceCoordinate> device_x_y_set;
  switch (topology_template.GetType()) {
    case ActTopologyTemplateTypeEnum::kLine: {
      // Create devices in a line topology
      std::vector<std::vector<int>> used_intfs(device_list.size());

      for (int i = 0; i < device_list.size() - 1; ++i) {
        ActDevice &src_dev = device_list[i];
        ActDevice &dst_dev = device_list[i + 1];

        // Create link
        ActLink link;
        link.SetSourceDeviceId(src_dev.GetId());
        link.SetSourceDeviceIp(src_dev.GetIpv4().GetIpAddress());
        link.SetDestinationDeviceId(dst_dev.GetId());
        link.SetDestinationDeviceIp(dst_dev.GetIpv4().GetIpAddress());

        // Get the next available port for source and destination devices
        int src_port_index = static_cast<int>(used_intfs[i].size());
        int dst_port_index = static_cast<int>(used_intfs[i + 1].size());

        ActInterface &src_intf = src_dev.GetInterfaces()[src_port_index];
        src_intf.SetUsed(true);
        ActInterface &dst_intf = dst_dev.GetInterfaces()[dst_port_index];
        dst_intf.SetUsed(true);

        // Interface IDs start from 1
        link.SetSourceInterfaceId(src_port_index + 1);
        link.SetDestinationInterfaceId(dst_port_index + 1);

        qint64 max_common_speed = GetCommonHighestSpeed(src_intf, dst_intf);
        if (max_common_speed == 0) {
          QString err_msg = QString("Can't find the common highest speed between the interfaces");
          qCritical() << err_msg.toStdString().c_str();
          return std::make_shared<ActBadRequest>(err_msg);
        }
        link.SetSpeed(max_common_speed);

        // Update used ports
        used_intfs[i].push_back(src_port_index);
        used_intfs[i + 1].push_back(dst_port_index);

        link_list.push_back(link);
      }

      // Calculate the coordinate of the devices to prevent device icon overlap
      // The center of the first device is from template coordinate
      // The center of the second device is 150px right from the first device
      // The center of the third device is 150px right from the second device
      // ...

      // Initialize starting coordinates
      int startX = x_y.GetX();
      int startY = x_y.GetY();
      int offsetX = 300;  // Distance between devices

      for (ActDevice &dev : device_list) {
        ActCoordinate coord;
        coord.SetX(startX);
        coord.SetY(startY);

        // Update x-coordinate for the next device
        startX += offsetX;

        ActDeviceCoordinate device_x_y(coord.GetX(), coord.GetY());
        device_x_y.SetId(dev.GetId());
        device_x_y_set.insert(device_x_y);
      }

    } break;
    case ActTopologyTemplateTypeEnum::kRing: {
      // Create devices in a ring topology
      std::vector<std::vector<int>> used_intfs(device_list.size());

      int num_devices = device_list.size();
      double angle_increment = 2 * M_PI / num_devices;
      int radius = 300;  // Radius of the ring

      // Initialize starting coordinates
      int startX = x_y.GetX();
      int startY = x_y.GetY();

      // Calculate the center of the circle based on the starting point
      int centerX = startX + radius;
      int centerY = startY;

      for (int i = 0; i < num_devices; ++i) {
        ActDevice &src_dev = device_list[i];
        ActDevice &dst_dev = device_list[(i + 1) % num_devices];

        // Calculate coordinates in a circular pattern
        int x = centerX + radius * cos(i * angle_increment - M_PI);
        int y = centerY + radius * sin(i * angle_increment - M_PI);

        ActCoordinate coord;
        coord.SetX(x);
        coord.SetY(y);

        ActDeviceCoordinate device_x_y(coord.GetX(), coord.GetY());
        device_x_y.SetId(src_dev.GetId());
        device_x_y_set.insert(device_x_y);

        // Create link
        ActLink link;
        link.SetSourceDeviceId(src_dev.GetId());
        link.SetSourceDeviceIp(src_dev.GetIpv4().GetIpAddress());
        link.SetDestinationDeviceId(dst_dev.GetId());
        link.SetDestinationDeviceIp(dst_dev.GetIpv4().GetIpAddress());

        // Get the next available port for source and destination devices
        int src_port_index = static_cast<int>(used_intfs[i].size());
        int dst_port_index = static_cast<int>(used_intfs[(i + 1) % num_devices].size());

        ActInterface &src_intf = src_dev.GetInterfaces()[src_port_index];
        src_intf.SetUsed(true);
        ActInterface &dst_intf = dst_dev.GetInterfaces()[dst_port_index];
        dst_intf.SetUsed(true);

        // Interface IDs start from 1
        link.SetSourceInterfaceId(src_port_index + 1);
        link.SetDestinationInterfaceId(dst_port_index + 1);

        qint64 max_common_speed = GetCommonHighestSpeed(src_intf, dst_intf);
        if (max_common_speed == 0) {
          QString err_msg = QString("Can't find the common highest speed between the interfaces");
          qCritical() << err_msg.toStdString().c_str();
          return std::make_shared<ActBadRequest>(err_msg);
        }
        link.SetSpeed(max_common_speed);

        // Update used ports
        used_intfs[i].push_back(src_port_index);
        used_intfs[(i + 1) % num_devices].push_back(dst_port_index);

        link_list.push_back(link);
      }
    } break;
    case ActTopologyTemplateTypeEnum::kStar: {
      // Create devices in a star topology
      std::vector<std::vector<int>> used_intfs(device_list.size());

      int num_devices = device_list.size();
      int radius = 300;  // Radius of the star

      // Initialize starting coordinates
      int centerX = x_y.GetX();
      int centerY = x_y.GetY();

      // Central device
      ActDevice &central_dev = device_list[0];
      ActCoordinate central_coord;
      central_coord.SetX(centerX);
      central_coord.SetY(centerY);

      ActDeviceCoordinate central_device_x_y(central_coord.GetX(), central_coord.GetY());
      central_device_x_y.SetId(central_dev.GetId());
      device_x_y_set.insert(central_device_x_y);

      // Check if central device has enough interfaces
      int max_interfaces = central_dev.GetInterfaces().size();
      if (num_devices - 1 > max_interfaces) {
        num_devices = max_interfaces + 1;  // Adjust number of devices to the maximum available interfaces
      }

      for (int i = 1; i < num_devices; ++i) {
        ActDevice &peripheral_dev = device_list[i];

        // Calculate coordinates in a circular pattern around the central device
        double angle = 2 * M_PI * (i - 1) / (num_devices - 1);
        int x = centerX + radius * cos(angle);
        int y = centerY + radius * sin(angle);

        ActCoordinate coord;
        coord.SetX(x);
        coord.SetY(y);

        ActDeviceCoordinate device_x_y(coord.GetX(), coord.GetY());
        device_x_y.SetId(peripheral_dev.GetId());
        device_x_y_set.insert(device_x_y);

        // Create link
        ActLink link;
        link.SetSourceDeviceId(central_dev.GetId());
        link.SetSourceDeviceIp(central_dev.GetIpv4().GetIpAddress());
        link.SetDestinationDeviceId(peripheral_dev.GetId());
        link.SetDestinationDeviceIp(peripheral_dev.GetIpv4().GetIpAddress());

        // Check if peripheral device has enough interfaces
        if (used_intfs[i].size() >= peripheral_dev.GetInterfaces().size()) {
          qCritical() << "Peripheral device does not have enough interfaces.";
          continue;
        }

        // Get the next available port for central and peripheral devices
        int central_port_index = static_cast<int>(used_intfs[0].size());
        int peripheral_port_index = static_cast<int>(used_intfs[i].size());

        ActInterface &central_intf = central_dev.GetInterfaces()[central_port_index];
        central_intf.SetUsed(true);
        ActInterface &peripheral_intf = peripheral_dev.GetInterfaces()[peripheral_port_index];
        peripheral_intf.SetUsed(true);

        // Interface IDs start from 1
        link.SetSourceInterfaceId(central_port_index + 1);
        link.SetDestinationInterfaceId(peripheral_port_index + 1);

        qint64 max_common_speed = GetCommonHighestSpeed(central_intf, peripheral_intf);
        if (max_common_speed == 0) {
          QString err_msg = QString("Can't find the common highest speed between the interfaces");
          qCritical() << err_msg.toStdString().c_str();
          return std::make_shared<ActBadRequest>(err_msg);
        }
        link.SetSpeed(max_common_speed);

        // Update used ports
        used_intfs[0].push_back(central_port_index);
        used_intfs[i].push_back(peripheral_port_index);

        link_list.push_back(link);
      }
    } break;
    default:
      break;
  }

  act_status = this->UpdateDeviceCoordinates(project, device_x_y_set);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update Device Coordinates failed";
    return act_status;
  }

  // Create links to project
  if (!link_list.isEmpty()) {
    act_status = this->CreateLinks(project, link_list, created_link_ids);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Create Links failed";
      return act_status;
    }
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return ACT_STATUS_SUCCESS;
}
}  // namespace core
}  // namespace act
