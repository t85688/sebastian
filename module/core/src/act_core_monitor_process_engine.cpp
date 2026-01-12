
#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QQueue>
#include <algorithm>  // for std::min

#include "act_core.hpp"
#include "act_monitor.hpp"
#include "act_mqtt_client.hpp"

namespace act {
namespace core {  // namespace core

quint64 g_last_fake_record_timestamp = 0;
QSet<QString> g_busy_device_set;  // Used to prevent multiple jobs for the same device
QMutex g_busy_device_set_mutex;

QMap<qint64, ActMonitorDeviceStatus> g_monitor_device_status;  // <device_id, device_status>
QSet<ActMonitorSwiftStatus> g_swift_status_set;
QSet<ActMonitorDeviceStatusData> g_device_status_ws_data_set;
QMap<qint64, bool> g_monitor_link_status;  // <link_id, link_status>
QSet<ActMonitorLinkStatusData> g_link_status_ws_data_set;
QSet<ActMonitorDeviceSystemStatus> g_device_system_status_ws_data_set;
QMap<qint64, ActDeviceMonitorTraffic> g_monitor_device_traffic;  // <device_id, device_traffic>
QMap<qint64, ActLinkMonitorTraffic> g_monitor_link_traffic;      // <link_id, link_traffic>
QMap<qint64, ActMonitorTimeStatus> g_monitor_time_status;        // <device_id, time_status>
QMap<qint64, ActMonitorBasicStatus> g_monitor_basic_status;      // <device_id, basic_status>
QMap<qint64, ActMonitorRstpStatus> g_monitor_rstp_status;        // <device_id, rstp_status>

QList<QString> g_baseline_device_ip_list;  // offline device IP list
QList<qint64> g_baseline_device_id_list;   // offline device id list
QList<qint64> g_baseline_link_id_list;     // offline link id list

QSet<ActDeviceCoordinate>
    g_dev_coordinate_set;  // device coordinate set, prevent coordinate conflict w/ device status update

void ActCore::DistributeMonitorData(ActMonitorData &data) {
  if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
    return;
  }

  // qDebug() << "[] enqueue monitor data:" << data.ToString().toStdString().c_str();
  QMutexLocker locker(monitor_process_queue_mutex_.get());

  // Dump the queue size
  if (monitor_process_queue_.size() > 100) {
    qWarning() << "Monitor process queue size:" << monitor_process_queue_.size();
    monitor_process_queue_.clear();

    QMutexLocker locker(&g_busy_device_set_mutex);
    g_busy_device_set.clear();
  }

  monitor_process_queue_.enqueue(data);
}

void ActCore::HandlePortLinkEvent(const ActMqttEventTopicEnum topic, const ActMqttMessage &message,
                                  bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  // Get the related device & link
  ActDevice device;
  act_status = monitor_project_.GetDeviceByIp(message.GetsourceIp(), device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get device by IP failed. IP:" << message.GetsourceIp();
    return;
  }

  const qint64 &port_id = message.Getvariables()[0].toLongLong();

  // QMutexLocker locker(&job_queue_mutex_);
  // job_queue_.clear();

  // If you do this, the device_handle flag should also be set
  // QMutexLocker locker(monitor_process_queue_mutex_.get());
  // monitor_process_queue_.clear();

  // QMutexLocker locker(&g_busy_device_set_mutex);
  // g_busy_device_set.remove(device.GetIpv4().GetIpAddress());

  const qint64 &dev_id = device.GetId();

  // Find out the related link and notify the user the link status
  QSet<ActLink> link_set = monitor_project_.GetLinks();
  for (ActLink link : link_set) {
    const qint64 &source_device_id = link.GetSourceDeviceId();
    const qint64 &source_intf_id = link.GetSourceInterfaceId();
    const qint64 &destination_device_id = link.GetDestinationDeviceId();
    const qint64 &destination_intf_id = link.GetDestinationInterfaceId();

    ActMonitorLinkStatusData prev_link_status_data;
    if ((source_device_id == dev_id && source_intf_id == port_id) ||
        (destination_device_id == dev_id && destination_device_id == port_id)) {
      bool found = false;
      // Remove the list status notification from g_link_status_ws_data_set
      for (auto it = g_link_status_ws_data_set.begin(); it != g_link_status_ws_data_set.end(); ++it) {
        if (it->GetId() == link.GetId()) {
          prev_link_status_data = *it;
          g_link_status_ws_data_set.erase(it);
          found = true;
          break;
        }
      }

      if (found) {
        prev_link_status_data.SetAlive(topic == ActMqttEventTopicEnum::kPortLinkUp);
        g_link_status_ws_data_set.insert(prev_link_status_data);
      } else {
        link.SetAlive(topic == ActMqttEventTopicEnum::kPortLinkUp);
        g_link_status_ws_data_set.insert(ActMonitorLinkStatusData(link));
      }
      g_monitor_link_status[link.GetId()] = (topic == ActMqttEventTopicEnum::kPortLinkUp);
    } else {
      // [bugfix:3733] If the link is not related to the device, it should be added to the g_link_status_ws_data_set
      link.SetAlive(g_monitor_link_status[link.GetId()]);
      g_link_status_ws_data_set.insert(ActMonitorLinkStatusData(link));
    }
  }

  // Notify the user that the link status immediately
  if (!g_link_status_ws_data_set.isEmpty()) {
    ActMonitorLinkMsg link_msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(), g_link_status_ws_data_set,
                               sync_to_websocket);

    // qDebug() << "trap link msg:" << link_msg.ToString().toStdString().c_str();

    this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, link_msg, monitor_project_.GetId());

    g_link_status_ws_data_set.clear();

    SLEEP_MS(50);
  }

  // Update the port status in the g_monitor_basic_status map
  ActMonitorBasicStatus dev_status = g_monitor_basic_status[dev_id];
  QMap<qint64, ActMonitorPortStatusEntry> port_status_map = dev_status.GetPortStatus();
  ActMonitorPortStatusEntry port_status_entry = port_status_map[port_id];
  port_status_entry.SetLinkStatus(topic == ActMqttEventTopicEnum::kPortLinkUp ? ActLinkStatusTypeEnum::kUp
                                                                              : ActLinkStatusTypeEnum::kDown);
  port_status_map[port_id] = port_status_entry;
  dev_status.SetPortStatus(port_status_map);
  g_monitor_basic_status[dev_id] = dev_status;
}

ACT_STATUS ActCore::HandleTrapMessage(ActMqttMessage &message, bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  qDebug() << "Trap message:" << message.ToString().toStdString().c_str();

  // transfer code w/ qint64 format to ActMqttEventTopicEnum
  ActMqttEventTopicEnum topic = static_cast<ActMqttEventTopicEnum>(message.Getcode());

  switch (topic) {
    case ActMqttEventTopicEnum::kPortLinkDown: {
      HandlePortLinkEvent(topic, message, sync_to_websocket, send_tmp);
      break;
    }
    case ActMqttEventTopicEnum::kPortLinkUp: {
      HandlePortLinkEvent(topic, message, sync_to_websocket, send_tmp);
      break;
    }
    default:
      break;
  }

  return act_status;
}

ACT_STATUS ActCore::HandlePingResult(ActPingDevice &ping_device, bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  if (ping_device.GetAlive()) {
    ActDevice identify_device;
    act_status = monitor_project_.GetDeviceById(identify_device, ping_device.GetId());
    if (IsActStatusNotFound(act_status)) {
      // If the device is not found in the project, create a new device
      identify_device = ActDevice(ping_device);
    } else {
      ActMonitorDeviceStatus device_status;
      device_status.SetAlive(true);
      g_monitor_device_status[identify_device.GetId()] = device_status;

      // Notify the user that the device is alive
      ActMonitorDeviceStatusData device_status_data(ping_device);
      g_device_status_ws_data_set.insert(device_status_data);
    }
    act_status = ACT_STATUS_SUCCESS;

    // Generate identify job
    ActIdentifyJob identify_job(identify_device);
    QList<ActJob> job_list;
    ActJob job;
    job.AssignJob<ActIdentifyJob>(monitor_project_.GetId(), ActJobTypeEnum::kIdentify, identify_job);
    job_list.push_back(job);
    this->DistributeWorkerJobs(job_list);
  } else {
    // If the device does not reply ICMP, which means the device is not alive
    // if the device is from original project topology, it need to notify the user that the device is not
    // alive

    // Remove keep connect flag
    if (monitor_project_.keep_connect_status_devices_.contains(ping_device.GetId())) {
      monitor_project_.keep_connect_status_devices_.remove(ping_device.GetId());
    }

    // if the device is the endpoint of the project, it should be deleted from the project
    if (monitor_project_.monitor_endpoint_.GetDeviceId() == ping_device.GetId()) {
      ActMonitorEndpointMsg msg(ActPatchUpdateActionEnum::kDelete, monitor_project_.GetId(),
                                monitor_project_.monitor_endpoint_, sync_to_websocket);
      this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());
    }

    if (g_baseline_device_ip_list.contains(ping_device.GetIpAddress())) {
      bool not_found = true;
      ActDevice device;
      // Find device by IP from the project
      for (ActDevice dev : monitor_project_.GetDevices()) {
        if (dev.GetIpv4().GetIpAddress() == ping_device.GetIpAddress()) {
          not_found = false;
          device = dev;
          break;
        }
      }

      if (not_found) {
        qCritical() << __func__ << "Project has no device IP" << ping_device.GetIpAddress();
        // Dump the project devices with ids in one line
        QStringList ip_list;
        for (const ActDevice device : monitor_project_.GetDevices()) {
          ip_list << device.GetIpv4().GetIpAddress();
        }
        QString debug_msg = ip_list.join(", ");
        qCritical() << "Project devices:" << debug_msg;
        return act_status;
      }

      // Notify the user that the device is not alive
      ActMonitorDeviceStatusData device_status_data(ping_device);
      g_device_status_ws_data_set.insert(device_status_data);

      ActMonitorDeviceStatus device_status;
      device_status.SetAlive(false);
      g_monitor_device_status[device.GetId()] = device_status;

      // Find out the related link and notify the user the link is not alive
      QSet<ActLink> link_set = monitor_project_.GetLinks();
      for (ActLink link : link_set) {
        int source_device_id = link.GetSourceDeviceId();
        int destination_device_id = link.GetDestinationDeviceId();

        if (source_device_id == ping_device.GetId() || destination_device_id == ping_device.GetId()) {
          link.SetAlive(false);
          g_link_status_ws_data_set.insert(ActMonitorLinkStatusData(link));
          g_monitor_link_status[link.GetId()] = false;
        }
      }

      // Update the port status in the g_monitor_basic_status map
      ActMonitorBasicStatus dev_status = g_monitor_basic_status[device.GetId()];
      QMap<qint64, ActMonitorPortStatusEntry> port_status_map = dev_status.GetPortStatus();
      for (auto port_idx : port_status_map.keys()) {
        ActMonitorPortStatusEntry port_status_entry = port_status_map[port_idx];
        port_status_entry.SetLinkStatus(ActLinkStatusTypeEnum::kDown);
        port_status_map[port_idx] = port_status_entry;
      }
      dev_status.SetPortStatus(port_status_map);
      g_monitor_basic_status[device.GetId()] = dev_status;

    } else {  // Online device
      bool not_found = true;
      ActDevice device;
      // Find device by IP from the project
      for (ActDevice dev : monitor_project_.GetDevices()) {
        if (dev.GetIpv4().GetIpAddress() == ping_device.GetIpAddress()) {
          not_found = false;
          device = dev;
          break;
        }
      }

      if (not_found) {
        act_status->SetActStatus(ActStatusType::kSkip);
        return act_status;
      }

      // Delete the device & related links from the project
      QSet<ActLink> link_set = monitor_project_.GetLinks();
      for (ActLink link : link_set) {
        int source_device_id = link.GetSourceDeviceId();
        int destination_device_id = link.GetDestinationDeviceId();

        if (source_device_id == device.GetId() || destination_device_id == device.GetId()) {
          // Delete the related link from the project
          act_status = DeleteLink(monitor_project_, link.GetId());
          if (!IsActStatusSuccess(act_status)) {
            qCritical() << __func__ << "Delete link failed. Link ID:" << link.GetId();
          }

          // Notify the user that the device be created
          ActLinkPatchUpdateMsg msg(ActPatchUpdateActionEnum::kDelete, monitor_project_.GetId(), link, true);
          this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

          g_monitor_link_status.remove(link.GetId());

          // Remove the link status notification from g_link_status_ws_data_set
          for (auto it = g_link_status_ws_data_set.begin(); it != g_link_status_ws_data_set.end(); ++it) {
            if (it->GetId() == link.GetId()) {
              g_link_status_ws_data_set.erase(it);
              break;
            }
          }

          // Remove the link status notification from g_monitor_link_traffic
          for (auto it = g_monitor_link_traffic.begin(); it != g_monitor_link_traffic.end(); ++it) {
            if (it->GetLinkId() == link.GetId()) {
              g_monitor_link_traffic.erase(it);
              break;
            }
          }
        }
      }

      act_status = DeleteDevice(monitor_project_, device.GetId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "Delete device failed. Device IP:" << device.GetIpv4().GetIpAddress();
      }

      qDebug() << __func__ << "Online device is not alive. Delete it. Device IP:" << device.GetIpv4().GetIpAddress();

      // Notify the user that the device be created
      ActDevicePatchUpdateMsg msg(ActPatchUpdateActionEnum::kDelete, monitor_project_.GetId(), device, true);
      this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

      g_monitor_device_status.remove(device.GetId());

      // Remove the device status notification from g_device_status_ws_data_set
      for (auto it = g_device_status_ws_data_set.begin(); it != g_device_status_ws_data_set.end(); ++it) {
        if (it->GetId() == device.GetId()) {
          g_device_status_ws_data_set.erase(it);
          break;
        }
      }

      // Remove the device status notification from g_device_system_status_ws_data_set
      for (auto it = g_device_system_status_ws_data_set.begin(); it != g_device_system_status_ws_data_set.end(); ++it) {
        if (it->GetDeviceId() == device.GetId()) {
          g_device_system_status_ws_data_set.erase(it);
          break;
        }
      }

      // Remove the device status notification from g_monitor_basic_status
      for (auto it = g_monitor_basic_status.begin(); it != g_monitor_basic_status.end(); ++it) {
        if (it->GetDeviceId() == device.GetId()) {
          g_monitor_basic_status.erase(it);
          break;
        }
      }

      // Remove the device status notification from g_monitor_time_status
      for (auto it = g_monitor_time_status.begin(); it != g_monitor_time_status.end(); ++it) {
        if (it->GetDeviceId() == device.GetId()) {
          g_monitor_time_status.erase(it);
          break;
        }
      }

      // Remove the device status notification from g_monitor_device_traffic
      for (auto it = g_monitor_device_traffic.begin(); it != g_monitor_device_traffic.end(); ++it) {
        if (it->GetDeviceId() == device.GetId()) {
          g_monitor_device_traffic.erase(it);
          break;
        }
      }
    }

    act_status->SetActStatus(ActStatusType::kSkip);
  }

  return act_status;
}

ACT_STATUS ActCore::HandleSwiftStatus(ActPingDevice &ping_device, bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActSwift swift = monitor_project_.GetTopologySetting().GetRedundantGroup().GetSwift();
  if (!swift.GetDeviceTierMap().contains(ping_device.GetId())) {
    return act_status;
  }

  qint64 root_dev_id = swift.GetRootDevice();
  qint64 back_root_dev_id = swift.GetBackupRootDevice();

  bool offline = true, online = true;
  bool alive = g_monitor_device_status[ping_device.GetId()].GetAlive();

  if (!alive && (ping_device.GetId() == root_dev_id || ping_device.GetId() == back_root_dev_id)) {
    // If the root device or backup root device is not alive, all the swift status is offline
    online = false;

    for (qint64 device_id : swift.GetDeviceTierMap().keys()) {
      ActDevice device;
      act_status = monitor_project_.GetDeviceById(device, device_id);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      ActMonitorSwiftStatus swift_status(device.GetId(), device.GetIpv4().GetIpAddress(), offline, online);
      g_swift_status_set.insert(swift_status);
    }

  } else {
    // Update the each device's swift status with device ping status
    for (qint64 device_id : swift.GetDeviceTierMap().keys()) {
      ActDevice device;
      act_status = monitor_project_.GetDeviceById(device, device_id);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      // [bugfix:3778] Root switch still show green from swift view when its priority is 32768
      ActRstpTable &rstp_table = monitor_project_.GetDeviceConfig().GetRstpTables()[device_id];
      online =
          g_monitor_device_status[device_id].GetAlive() && rstp_table.GetActive() && (rstp_table.GetHelloTime() == 1) &&
          ((rstp_table.GetRstpConfigRevert() && rstp_table.GetRstpConfigSwift() && rstp_table.GetPriority() == 4096) ||
           (!rstp_table.GetRstpConfigRevert() && rstp_table.GetRstpConfigSwift() && rstp_table.GetPriority() == 8192) ||
           (!rstp_table.GetRstpConfigRevert() && !rstp_table.GetRstpConfigSwift() &&
            rstp_table.GetPriority() == 32768));

      ActMonitorSwiftStatus swift_status(device.GetId(), device.GetIpv4().GetIpAddress(), offline, online);
      g_swift_status_set.insert(swift_status);
    }
  }

  // if (!g_swift_status_set.isEmpty()) {
  //   ActMonitorSwiftStatusMsg msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(), g_swift_status_set,
  //                                 sync_to_websocket);
  //   this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());
  // }

  return act_status;
}

ACT_STATUS ActCore::HandleBasicStatusResult(ActMonitorBasicStatus &device_basic_status, bool sync_to_websocket,
                                            bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  // Find device from the project
  ActDevice device;
  act_status = monitor_project_.GetDeviceById(device, device_basic_status.GetDeviceId());
  if (IsActStatusNotFound(act_status)) {
    qCritical() << __func__ << "Project has no device id" << QString::number(device_basic_status.GetDeviceId());
    // Dump the project devices with ids in one line
    QStringList id_list;
    for (const ActDevice device : monitor_project_.GetDevices()) {
      id_list << QString::number(device.GetId());
    }
    QString debug_msg = id_list.join(", ");
    qCritical() << "Project devices:" << debug_msg;
    return act_status;
  }

  ActMonitorDeviceSystemStatus device_system_status(device_basic_status);
  device_system_status.SetModelName(device.GetDeviceProperty().GetModelName());
  device_system_status.SetAlias(device.GetDeviceAlias());
  device_system_status.SetInterfaces(device.GetInterfaces());
  device_system_status.SetModularInfo(device_basic_status.GetModularInfo());
  g_device_system_status_ws_data_set.insert(device_system_status);

  if (!g_monitor_basic_status.contains(device.GetId())) {
    g_monitor_basic_status[device.GetId()] = ActMonitorBasicStatus();
  }

  g_monitor_basic_status[device.GetId()] = device_basic_status;

  // Save the fiber check status
  for (ActMonitorFiberCheckEntry &entry : device_basic_status.GetFiberCheck()) {
    monitor_project_.fiber_check_entries_.insert(entry);
  }

  // [bugfix:3466] Monitor - Import/Export Device Config fields error
  device.GetDeviceInfo().SetSerialNumber(device_basic_status.GetSerialNumber());
  device.GetDeviceInfo().SetLocation(device_basic_status.GetLocation());
  device.GetDeviceInfo().SetProductRevision(device_basic_status.GetProductRevision());
  device.GetDeviceInfo().SetSystemUptime(device_basic_status.GetSystemUptime());
  device.GetDeviceInfo().SetRedundantProtocol(device_basic_status.GetRedundantProtocol());
  device.SetDeviceName(device_basic_status.GetDeviceName());
  device.GetDeviceProperty().SetDescription(device_basic_status.GetDescription());
  act_status = act::core::g_core.UpdateDevice(monitor_project_, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update device:" << device.ToString().toStdString().c_str();
    return act_status;
    // TODO: Allen How to handle the error?
  }

  // Notify the user that the device be updated ?
  // basic_status already updated by 1540
  // ActDevicePatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(), device,
  // true); this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

  return act_status;
}

ACT_STATUS ActCore::HandleTrafficResult(ActDeviceMonitorTraffic &traffic, bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  // qDebug() << __func__ << traffic.ToString().toStdString().c_str();

  // Find device from the project
  ActDevice device;
  act_status = monitor_project_.GetDeviceById(device, traffic.GetDeviceId());
  if (IsActStatusNotFound(act_status)) {
    qCritical() << __func__ << "Project has no device id" << QString::number(traffic.GetDeviceId());
    // Dump the project devices with ids in one line
    QStringList id_list;
    for (const ActDevice device : monitor_project_.GetDevices()) {
      id_list << QString::number(device.GetId());
    }
    QString debug_msg = id_list.join(", ");
    qCritical() << "Project devices:" << debug_msg;
    return act_status;
  }

  if (!g_monitor_device_traffic.contains(device.GetId())) {
    g_monitor_device_traffic[device.GetId()] = ActDeviceMonitorTraffic();
  }

  ActDeviceMonitorTraffic prev_traffic = g_monitor_device_traffic[device.GetId()];
  QMap<qint64, ActDeviceMonitorTrafficEntry> prev_traffic_view_map = prev_traffic.GetTrafficMap();
  QMap<qint64, ActDeviceMonitorTrafficEntry> &traffic_view_map = traffic.GetTrafficMap();

  // Find out the related link and notify the user the link's utilization
  QSet<ActLink> link_set = monitor_project_.GetLinks();

  // If this is the first time, the previous traffic view is empty, just save it
  if (prev_traffic_view_map.size()) {
    // Calculate the utilization
    quint64 old_time = prev_traffic.GetTimestamp();
    quint64 new_time = traffic.GetTimestamp();
    qreal precise_elapsed_time = new_time - old_time;

    for (qint64 port_id : traffic_view_map.keys()) {
      ActDeviceMonitorTrafficEntry prev_traffic_view_entry = prev_traffic_view_map.value(port_id);
      ActDeviceMonitorTrafficEntry traffic_entry = traffic_view_map.value(port_id);

      /*
      http://mhqnetgitlab.moxa.com/LinuxPlatform/tcst/status_moxa_tcst/-/issues/3#note_18221

      Pkts增加量 * (9.6 + 6.4) + (Octets增加量 * 0.8)
      ------------------------------------
             Interval * speed

      (speed: 10M = 10,000 ; 100M = 100,000  ; 1G = 1,000,000
      */

      quint64 prev_tx_total_octets = prev_traffic_view_entry.GetTxTotalOctets();
      quint64 tx_total_octets = traffic_entry.GetTxTotalOctets();
      quint64 tx_octets;
      if (tx_total_octets >= prev_tx_total_octets) {
        tx_octets = tx_total_octets - prev_tx_total_octets;
      } else {
        // Handle overflow: add the maximum value of a 32-bit counter plus 1
        tx_octets = (tx_total_octets + (1ULL << 32)) - prev_tx_total_octets;
      }

      quint64 prev_tx_total_pkts = prev_traffic_view_entry.GetTxTotalPackets();
      quint64 tx_total_pkts = traffic_entry.GetTxTotalPackets();
      quint64 tx_pkts;
      if (tx_total_pkts >= prev_tx_total_pkts) {
        tx_pkts = tx_total_pkts - prev_tx_total_pkts;
      } else {
        // Handle overflow: add the maximum value of a 32-bit counter plus 1
        tx_pkts = (tx_total_pkts + (1ULL << 32)) - prev_tx_total_pkts;
      }

      quint64 port_speed = traffic_entry.GetPortSpeed() * 1000 / 8;

      // Calculate the utilization
      qreal utilization = 0.0;
      if (precise_elapsed_time > 0) {
        // Calculate the numerator of the formula
        qreal numerator = static_cast<qreal>((tx_octets + tx_pkts * 20));  // 20 bytes L3 headers per packet
        // Calculate the denominator of the formula
        qreal denominator = static_cast<qreal>(precise_elapsed_time) * static_cast<qreal>(port_speed);

        // Calculate utilization
        utilization = (numerator / denominator) * 100.0;

        // Format utilization to two decimal places
        QString formattedUtilization = QString::number(utilization, 'f', 3);

        // Convert the formatted string back to qreal
        utilization = formattedUtilization.toDouble();

        // Round up to the nearest integer
        utilization = qCeil(utilization);

        // Ensure utilization is between 0 and 100
        if (utilization < 0) {
          utilization = 0;
        } else if (utilization > 100) {
          utilization = 100;
        }
      }

      // Update the traffic view
      traffic_entry.SetTrafficUtilization(utilization);
      traffic_view_map[port_id] = traffic_entry;

      for (ActLink link : link_set) {
        // Ignore the link that is not related to the device
        if (!((link.GetSourceDeviceId() == traffic.GetDeviceId() && link.GetSourceInterfaceId() == port_id) ||
              (link.GetDestinationDeviceId() == traffic.GetDeviceId() &&
               link.GetDestinationInterfaceId() == port_id))) {
          continue;
        }

        if (!g_monitor_link_traffic.contains(link.GetId())) {
          g_monitor_link_traffic[link.GetId()] = ActLinkMonitorTraffic();
        }

        ActLinkMonitorTraffic link_traffic = g_monitor_link_traffic[link.GetId()];
        link_traffic.SetLinkId(link.GetId());
        link_traffic.SetSourceDeviceId(link.GetSourceDeviceId());
        link_traffic.SetSourceInterfaceId(link.GetSourceInterfaceId());
        link_traffic.SetDestinationDeviceId(link.GetDestinationDeviceId());
        link_traffic.SetDestinationInterfaceId(link.GetDestinationInterfaceId());
        link_traffic.SetSpeed(traffic_entry.GetPortSpeed());
        link_traffic.SetTimestamp(traffic.GetTimestamp());

        if (link.GetSourceDeviceId() == traffic_entry.GetDeviceId() && link.GetSourceInterfaceId() == port_id) {
          link_traffic.SetSourceTrafficUtilization(traffic_entry.GetTrafficUtilization());
        }

        if (link.GetDestinationDeviceId() == traffic_entry.GetDeviceId() &&
            link.GetDestinationInterfaceId() == port_id) {
          link_traffic.SetDestinationTrafficUtilization(traffic_entry.GetTrafficUtilization());
        }

        g_monitor_link_traffic[link.GetId()] = link_traffic;
      }
    }
  }

  // Notify the traffic information of the device
  // ActDeviceMonitorTrafficMsg msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(), traffic,
  //
  //                                sync_to_websocket);
  // this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

  g_monitor_device_traffic[device.GetId()] = traffic;

  return act_status;
}

ACT_STATUS ActCore::HandleTimeStatusResult(ActMonitorTimeStatus &time_synchronization_data, bool sync_to_websocket,
                                           bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  // Notify the time synchronization information of the device
  ActMonitorTimeStatusMsg msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(), {time_synchronization_data},
                              sync_to_websocket);
  this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

  // TODO: Allen save the device status for compare

  return act_status;
}

ACT_STATUS ActCore::HandleDeviceConnectionStatus(ActDevice &device, bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDevice project_device;
  act_status = monitor_project_.GetDeviceById(project_device, device.GetId());
  if (IsActStatusSuccess(act_status)) {  // Only update exists device
    project_device.SetDeviceStatus(device.GetDeviceStatus());
    act_status = act::core::g_core.UpdateDevice(monitor_project_, project_device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Cannot update device:" << project_device.ToString().toStdString().c_str();
      // TODO: Allen How to handle the error?
      return act_status;
    }

    // Check needs connect are success
    ActDeviceConnectStatusControl connect_control(false, false, false, false);
    ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(),
                         this->GetDeviceProfileSet(), this->GetDefaultDeviceProfileSet());
    ActMonitor monitor(profiles);
    auto status = monitor.GetConnectControlByFeature(device, connect_control);
    if (IsActStatusSuccess(status)) {
      if (connect_control.GetRESTful()) {
        if (!device.GetDeviceStatus().GetRESTfulStatus()) {
          return ACT_STATUS_SUCCESS;
        }
      }

      if (connect_control.GetSNMP()) {
        if (!device.GetDeviceStatus().GetSNMPStatus()) {
          return ACT_STATUS_SUCCESS;
        }
      }

      if (connect_control.GetNETCONF()) {
        if (!device.GetDeviceStatus().GetNETCONFStatus()) {
          return ACT_STATUS_SUCCESS;
        }
      }

      monitor_project_.keep_connect_status_devices_.insert(device.GetId());
    }
  }

  return act_status;
}

ACT_STATUS ActCore::HandleModuleConfigAndInterfaces(ActDevice &device, bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDevice project_device;
  act_status = monitor_project_.GetDeviceById(project_device, device.GetId());
  if (IsActStatusSuccess(act_status)) {  // Only update exists device
    project_device.SetInterfaces(device.GetInterfaces());
    project_device.SetModularConfiguration(device.GetModularConfiguration());

    act_status = act::core::g_core.UpdateDevice(monitor_project_, project_device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Cannot update device:" << project_device.ToString().toStdString().c_str();
      // TODO: Allen How to handle the error?
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS ActCore::HandleIdentifyDevice(ActDevice &device, bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  // Find device from the projectport
  ActDevice project_device;
  act_status = monitor_project_.GetDeviceById(project_device, device.GetId());
  if (IsActStatusNotFound(act_status)) {
    // New device
    const bool from_bag = false;
    act_status = act::core::g_core.CreateDevice(monitor_project_, device, from_bag);
    if (!IsActStatusSuccess(act_status)) {
      QList<QString> key_order = {QString("DeviceProfileId"), QString("DeviceType"), QString("Ipv4")};
      qCritical() << "Cannot create device:" << device.ToString(key_order).toStdString().c_str();
      // TODO: Allen How to handle the error?
      return act_status;
    }

    // Notify the user that the device be created
    ActDevicePatchUpdateMsg msg(ActPatchUpdateActionEnum::kCreate, monitor_project_.GetId(), device, true);
    this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

    g_monitor_device_status[device.GetId()].SetAlive(true);

    ActMonitorDeviceStatusData device_status_data;
    device_status_data.SetAlive(true);
    device_status_data.SetId(device.GetId());
    device_status_data.SetIpAddress(device.GetIpv4().GetIpAddress());
    g_device_status_ws_data_set.insert(device_status_data);

  } else {  // Old device
    // Check DeviceProfile same as the old device
    // If different would notify user or update device (MONITOR-1101 - device alive status > 4 )
    if (project_device.GetDeviceProfileId() != device.GetDeviceProfileId()) {
      if (baseline_project_.GetDevices().contains(device)) {
        // If the device is from offline
        // Notify the user that the device is not alive (MONITOR-1101 - device alive status > 4 > a )
        ActMonitorDeviceStatusData device_status_data(device.GetId(), device.GetIpv4().GetIpAddress(), false);
        ActMonitorDeviceMsg msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(), {device_status_data},
                                sync_to_websocket);
        this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

        // Skip the following job
        act_status->SetActStatus(ActStatusType::kSkip);
        return act_status;
      } else {
        // Device from the online
        // Notify the user that the device be changed  (MONITOR-1101 - device alive status > 4 > b )
        act_status = act::core::g_core.UpdateDevice(monitor_project_, device);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "Cannot update device:" << device.ToString().toStdString().c_str();
          // TODO: Allen How to handle the error?
          return act_status;
        }

        // Notify the user that the device be update
        ActDevicePatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(), device, true);

        this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

        // If the device does not support the monitor or auto scan feature, just return
        if (!device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetLLDP()) {
          // Skip the following job
          act_status->SetActStatus(ActStatusType::kSkip);
          return act_status;
        }
      }
    }
  }

  // If the device is ICMP device, moxa device or unknown device, just return
  if (device.GetDeviceType() == ActDeviceTypeEnum::kICMP || device.GetDeviceType() == ActDeviceTypeEnum::kMoxa ||
      device.GetDeviceType() == ActDeviceTypeEnum::kUnknown) {
    // Skip the following job
    act_status->SetActStatus(ActStatusType::kSkip);
    qDebug() << device.GetIpv4().GetIpAddress() << ": Skip the following job for ICMP/Moxa/Unknown device";
    return act_status;
  }

  // Generate scan job to
  ActScanJob scan_job(device);
  if (monitor_project_.keep_connect_status_devices_.contains(device.GetId())) {
    scan_job.SetKeepConnectStatus(true);
  } else {
    scan_job.SetKeepConnectStatus(false);
  }

  QList<ActJob> job_list;
  ActJob job;
  job.AssignJob<ActScanJob>(monitor_project_.GetId(), ActJobTypeEnum::kScan, scan_job);
  job_list.push_back(job);
  this->DistributeWorkerJobs(job_list);

  return act_status;
}

ACT_STATUS ActCore::HandleAssignScanLinkData(ActDevice &update_device, bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  // Find device from the monitor_project
  ActDevice device;
  act_status = monitor_project_.GetDeviceById(device, update_device.GetId());
  if (IsActStatusNotFound(act_status)) {
    qCritical() << __func__ << "Project has no device id" << QString::number(update_device.GetId());
    // Dump the project devices with ids in one line
    QStringList id_list;
    for (const ActDevice device : monitor_project_.GetDevices()) {
      id_list << QString::number(device.GetId());
    }
    QString debug_msg = id_list.join(", ");
    qCritical() << "Project devices:" << debug_msg;
    return act_status;
  }

  // Sync ScanLink data to Monitor project device
  device.lldp_data_ = update_device.lldp_data_;
  device.single_entry_mac_table_ = update_device.single_entry_mac_table_;
  device.mac_address_int = update_device.mac_address_int;
  device.SetMacAddress(update_device.GetMacAddress());

  act_status = act::core::g_core.UpdateDevice(monitor_project_, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update device:" << device.ToString().toStdString().c_str();
    // TODO: Allen How to handle the error?
    return act_status;
  }

  // Generate create link job to
  ActScanLinkJob scan_link_job(device, monitor_project_.GetDevices());

  QList<ActJob> job_list;
  ActJob job;
  job.AssignJob<ActScanLinkJob>(monitor_project_.GetId(), ActJobTypeEnum::kScanLink, scan_link_job);
  job_list.push_back(job);
  this->DistributeWorkerJobs(job_list);

  return act_status;
}

ACT_STATUS ActCore::HandleManagementEndpoint(ActSourceDevice &src_device, bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  // qDebug() << __func__ << QString("ActSourceDevice: %1").arg(src_device.ToString()).toStdString().c_str();

  // Check src_device exists
  ActDevice device;
  act_status = monitor_project_.GetDeviceById(device, src_device.GetDeviceId());
  if (IsActStatusNotFound(act_status)) {
    qCritical() << __func__ << "Project has no device id" << QString::number(src_device.GetDeviceId());
    // Dump the project devices with ids in one line
    QStringList id_list;
    for (const ActDevice device : monitor_project_.GetDevices()) {
      id_list << QString::number(device.GetId());
    }
    QString debug_msg = id_list.join(", ");
    qCritical() << "Project devices:" << debug_msg;

    ActMonitorEndpointMsg msg(ActPatchUpdateActionEnum::kDelete, monitor_project_.GetId(), src_device,
                              sync_to_websocket);
    this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

    return act_status;
  } else {
    // Check interface exists
    if (!device.GetInterfaces().contains(ActInterface(src_device.GetInterfaceId()))) {
      qCritical() << __func__
                  << QString("Not found interface(%1) on the device(%2)")
                         .arg(src_device.GetInterfaceId())
                         .arg(src_device.GetDeviceId())
                         .toStdString()
                         .c_str();

      ActMonitorEndpointMsg msg(ActPatchUpdateActionEnum::kDelete, monitor_project_.GetId(), src_device,
                                sync_to_websocket);
      this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

      return std::make_shared<ActStatusNotFound>("Endpoint Interface in the monitor project");
    }
  }

  // If the port is not alive, just return
  // The arp table doesn't update so fast, so we need to prevent the monitor endpoint from being updated when the port
  // is down
  ActMonitorBasicStatus monitor_basic_status = g_monitor_basic_status[src_device.GetDeviceId()];
  QMap<qint64, ActMonitorPortStatusEntry> port_status_map = monitor_basic_status.GetPortStatus();
  if (port_status_map[src_device.GetInterfaceId()].GetLinkStatus() == ActLinkStatusTypeEnum::kDown) {
    qDebug() << __func__ << "Device:" << device.GetIpv4().GetIpAddress() << "Port:" << src_device.GetInterfaceId()
             << "is down, skip the management endpoint update";
    return act_status;
  }

  ActSourceDevice &monitor_endpoint = monitor_project_.monitor_endpoint_;

  monitor_endpoint = src_device;

  ActMonitorEndpointMsg msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(), src_device, sync_to_websocket);
  this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::HandleDeviceScanLinks(ActScanLinksResult &scan_links_result, bool sync_to_websocket,
                                          bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  // qDebug() << __func__ << QString("ActScanLinksResult:
  // %1").arg(scan_links_result.ToString()).toStdString().c_str();

  // Sync device's interfaces to Monitor project device
  for (auto update_device : scan_links_result.GetUpdateDevices()) {
    // Find device from the monitor_project
    ActDevice device;
    act_status = monitor_project_.GetDeviceById(device, update_device.GetId());
    if (IsActStatusNotFound(act_status)) {
      qCritical() << __func__ << "Project has no device id" << QString::number(update_device.GetId());
      // Dump the project devices with ids in one line
      QStringList id_list;
      for (const ActDevice device : monitor_project_.GetDevices()) {
        id_list << QString::number(device.GetId());
      }
      QString debug_msg = id_list.join(", ");
      qCritical() << "Project devices:" << debug_msg;
      return act_status;
    }

    // Check monitor's device interfaces number
    if (device.GetInterfaces().isEmpty()) {
      device.SetInterfaces(update_device.GetInterfaces());
    } else {  // Check update interfaces in the monitor device
      for (auto update_interface : update_device.GetInterfaces()) {
        if (!device.GetInterfaces().contains(update_interface)) {
          // Not found would append to monitor device
          device.GetInterfaces().append(update_interface);
        }
        // Sort interfaces by id
        std::sort(device.GetInterfaces().begin(), device.GetInterfaces().end());
      }
    }

    act_status = act::core::g_core.UpdateDevice(monitor_project_, device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Cannot update device:" << device.ToString().toStdString().c_str();
      // TODO: Allen How to handle the error?
    }
    // Notify the user that the device be updated
    ActDevicePatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(), device, true);
    this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());
  }

  QSet<ActLink> link_set = monitor_project_.GetLinks();

  // Find new create link & add to monitor_project_
  for (auto link : scan_links_result.GetScanLinks()) {
    ActLink src_entry = ActLink(-1, link.GetSourceDeviceId(), -1, link.GetSourceInterfaceId(), -1);
    ActLink dst_entry = ActLink(-1, -1, link.GetDestinationDeviceId(), -1, link.GetDestinationInterfaceId());

    ActLink old_link;
    bool src_found = false;
    bool dst_found = false;
    if (link_set.contains(src_entry)) {
      // Fetch the source entry from the link_set
      typename QSet<ActLink>::const_iterator iterator;
      iterator = link_set.find(src_entry);
      src_found = true;
      old_link = *iterator;
    }

    if (link_set.contains(dst_entry)) {
      // Fetch the destination entry from the link_set
      typename QSet<ActLink>::const_iterator iterator;
      iterator = link_set.find(dst_entry);
      dst_found = true;
      old_link = *iterator;
    }

    if (src_found ^ dst_found) {  // One of the side doesn't exist. remove the link
      act_status = act::core::g_core.DeleteLink(monitor_project_, old_link.GetId());
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Cannot delete link:" << link.ToString().toStdString().c_str();
      }

      // Notify the user that the link be deleted
      ActLinkPatchUpdateMsg msg(ActPatchUpdateActionEnum::kDelete, monitor_project_.GetId(), old_link, true);
      this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

      // Remove the link status
      g_monitor_link_status.remove(old_link.GetId());
      g_monitor_link_traffic.remove(old_link.GetId());

      // Remove the list status notification from g_monitor_link_status_ws_data_set
      for (auto it = g_link_status_ws_data_set.begin(); it != g_link_status_ws_data_set.end(); ++it) {
        if (it->GetId() == old_link.GetId()) {
          g_link_status_ws_data_set.erase(it);
          break;
        }
      }
    }

    if (!(src_found && dst_found)) {  // Create the link

      // Re-assign the link id as -1, avoid check duplicated failed
      link.SetId(-1);
      link.SetAlive(true);

      // Check link not duplicated
      act_status = act::core::g_core.CreateLink(monitor_project_, link);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Cannot create link:" << link.ToString().toStdString().c_str();
        // TODO: Allen How to handle the error?
        // return act_status;
      }

      // Notify the user that the link be created
      ActLinkPatchUpdateMsg msg(ActPatchUpdateActionEnum::kCreate, monitor_project_.GetId(), link, true);
      this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

      g_monitor_link_status[link.GetId()] = true;

      ActMonitorLinkStatusData link_status_data(link);
      g_link_status_ws_data_set.insert(link_status_data);
    }
  }
  return act_status;
}

void UpdateRedundantLinkStatus(const qint64 &link_id, bool isRedundant) {
  QSet<ActMonitorLinkStatusData> new_set;
  for (ActMonitorLinkStatusData entry : g_link_status_ws_data_set) {
    if (entry.GetId() == link_id) {
      entry.SetRedundancy(isRedundant);
    }
    new_set.insert(entry);
  }
  g_link_status_ws_data_set = new_set;
}

ACT_STATUS ActCore::HandleRSTPResult(ActMonitorRstpStatus &rstp_status, bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  g_monitor_rstp_status[rstp_status.GetDeviceId()] = rstp_status;

  return act_status;
}

ACT_STATUS ActCore::HandleVLANResult(ActVlanTable &vlan_table, bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetVlanTables()[vlan_table.GetDeviceId()] = vlan_table;

  return act_status;
}

ACT_STATUS ActCore::HandlePortDefaultPCPResult(ActDefaultPriorityTable &pcp_table, bool sync_to_websocket,
                                               bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetPortDefaultPCPTables()[pcp_table.GetDeviceId()] = pcp_table;

  return act_status;
}

ACT_STATUS ActCore::HandleNetworkSettingResult(ActNetworkSettingTable &network_setting_table, bool sync_to_websocket,
                                               bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  // Update Device's ipv4
  ActDevice device;
  act_status = monitor_project_.GetDeviceById(device, network_setting_table.GetDeviceId());
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Project has no device id" << QString::number(network_setting_table.GetDeviceId());
    return act_status;
  }

  device.GetIpv4().SetSubnetMask(network_setting_table.GetSubnetMask());
  device.GetIpv4().SetGateway(network_setting_table.GetGateway());
  device.GetIpv4().SetDNS1(network_setting_table.GetDNS1());
  device.GetIpv4().SetDNS2(network_setting_table.GetDNS2());
  act_status = act::core::g_core.UpdateDevice(monitor_project_, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update device:" << device.ToString().toStdString().c_str();
    return act_status;
  }

  // Update DeviceConfig
  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetNetworkSettingTables()[network_setting_table.GetDeviceId()] = network_setting_table;

  return act_status;
}

ACT_STATUS ActCore::HandleUserAccountResult(ActUserAccountTable &user_account_table, bool sync_to_websocket,
                                            bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetUserAccountTables()[user_account_table.GetDeviceId()] = user_account_table;

  return act_status;
}

ACT_STATUS ActCore::HandleLoginPolicyResult(ActLoginPolicyTable &login_policy_table, bool sync_to_websocket,
                                            bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetLoginPolicyTables()[login_policy_table.GetDeviceId()] = login_policy_table;

  return act_status;
}

ACT_STATUS ActCore::HandleSnmpTrapSettingResult(ActSnmpTrapSettingTable &snmp_trap_setting_table,
                                                bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetSnmpTrapSettingTables()[snmp_trap_setting_table.GetDeviceId()] = snmp_trap_setting_table;

  return act_status;
}

ACT_STATUS ActCore::HandleSyslogSettingResult(ActSyslogSettingTable &syslog_setting_table, bool sync_to_websocket,
                                              bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetSyslogSettingTables()[syslog_setting_table.GetDeviceId()] = syslog_setting_table;

  return act_status;
}

ACT_STATUS ActCore::HandleTimeSettingResult(ActTimeSettingTable &time_setting_table, bool sync_to_websocket,
                                            bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetTimeSettingTables()[time_setting_table.GetDeviceId()] = time_setting_table;

  return act_status;
}

ACT_STATUS ActCore::HandlePortSettingResult(ActPortSettingTable &port_setting_table, bool sync_to_websocket,
                                            bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetPortSettingTables()[port_setting_table.GetDeviceId()] = port_setting_table;

  return act_status;
}

ACT_STATUS ActCore::HandleStreamPriorityIngressResult(ActStadPortTable &stad_port_table, bool sync_to_websocket,
                                                      bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetStreamPriorityIngressTables()[stad_port_table.GetDeviceId()] = stad_port_table;

  return act_status;
}

ACT_STATUS ActCore::HandleStreamPriorityEgressResult(ActStadConfigTable &stad_config_table, bool sync_to_websocket,
                                                     bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);
  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetStreamPriorityEgressTables()[stad_config_table.GetDeviceId()] = stad_config_table;

  return act_status;
}

ACT_STATUS ActCore::HandleInformationSettingResult(ActInformationSettingTable &info_setting_table,
                                                   bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  // Update Device's DeviceName & Location & Description
  ActDevice device;
  act_status = monitor_project_.GetDeviceById(device, info_setting_table.GetDeviceId());
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Project has no device id" << QString::number(info_setting_table.GetDeviceId());
    return act_status;
  }

  device.SetDeviceName(info_setting_table.GetDeviceName());
  device.GetDeviceInfo().SetLocation(info_setting_table.GetLocation());
  device.GetDeviceProperty().SetDescription(info_setting_table.GetDescription());
  act_status = act::core::g_core.UpdateDevice(monitor_project_, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update device:" << device.ToString().toStdString().c_str();
    return act_status;
  }

  // Update DeviceConfig
  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetInformationSettingTables()[info_setting_table.GetDeviceId()] = info_setting_table;

  return act_status;
}

ACT_STATUS ActCore::HandleManagementInterfaceResult(ActManagementInterfaceTable &mgmt_interface_table,
                                                    bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetManagementInterfaceTables()[mgmt_interface_table.GetDeviceId()] = mgmt_interface_table;

  return act_status;
}

ACT_STATUS ActCore::HandleUnicastStaticResult(ActStaticForwardTable &static_forward_table, bool sync_to_websocket,
                                              bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetUnicastStaticForwardTables()[static_forward_table.GetDeviceId()] = static_forward_table;

  return act_status;
}

ACT_STATUS ActCore::HandleMulticastStaticResult(ActStaticForwardTable &static_forward_table, bool sync_to_websocket,
                                                bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetMulticastStaticForwardTables()[static_forward_table.GetDeviceId()] = static_forward_table;

  return act_status;
}

ACT_STATUS ActCore::HandleTimeAwareShaperResult(ActGclTable &gcl_table, bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetGCLTables()[gcl_table.GetDeviceId()] = gcl_table;

  return act_status;
}

ACT_STATUS ActCore::HandleLoopProtectionResult(ActLoopProtectionTable &loop_protection_table, bool sync_to_websocket,
                                               bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetLoopProtectionTables()[loop_protection_table.GetDeviceId()] = loop_protection_table;

  return act_status;
}

ACT_STATUS ActCore::HandleTimeSyncSettingResult(ActTimeSyncTable &time_sync_table, bool sync_to_websocket,
                                                bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetTimeSyncTables()[time_sync_table.GetDeviceId()] = time_sync_table;

  return act_status;
}

ACT_STATUS ActCore::HandleRstpSettingResult(ActRstpTable &rstp_table, bool sync_to_websocket, bool send_tmp) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->monitor_mutex_);

  ActDeviceConfig &device_config = monitor_project_.GetDeviceConfig();
  device_config.GetRstpTables()[rstp_table.GetDeviceId()] = rstp_table;

  return act_status;
}

void UpdateSwiftStatus(ActProject &monitor_project_, const ActLink &link, const QMap<qint64, qint16> &tier_map,
                       bool is_alive) {
  // Update swift link status
  // if the link is down, the lower device should be false in the swift status
  // if the link is up, should update the device status to true in the swift status

  ACT_STATUS_INIT();
  qint64 src_device_id = link.GetSourceDeviceId();
  qint64 dst_device_id = link.GetDestinationDeviceId();

  // Check if the source or destination device is in the swift group
  if (tier_map.contains(src_device_id) && tier_map.contains(dst_device_id)) {
    qint16 src_device_tier = tier_map[src_device_id];
    qint16 dst_device_tier = tier_map[dst_device_id];

    ActDevice device;
    if (src_device_tier < dst_device_tier) {
      act_status = monitor_project_.GetDeviceById(device, dst_device_id);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "GetDeviceById() failed.";
        return;
      }
    } else if (src_device_tier > dst_device_tier) {
      act_status = monitor_project_.GetDeviceById(device, dst_device_id);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "GetDeviceById() failed.";
        return;
      }
    }

    ActMonitorSwiftStatus swift_status(dst_device_id, device.GetIpv4().GetIpAddress(), true, is_alive);
    g_swift_status_set.insert(swift_status);
  }
}

void ActCore::MonitorProcessThread(const qint64 monitor_project_id, const qint64 ws_listener_id) {
  ACT_STATUS_INIT();

  ActMonitorData data;
  int sleep_time_ms = 100;             // Initial sleep time
  const int max_sleep_time_ms = 1000;  // Maximum sleep time
  qint64 last_sfp_update_timestamp = QDateTime::currentSecsSinceEpoch();
  qint64 last_device_status_update_timestamp = QDateTime::currentSecsSinceEpoch();

  bool sync_to_websocket = true;
  bool send_tmp = false;

  g_last_fake_record_timestamp = 0;
  g_monitor_device_status.clear();
  g_device_status_ws_data_set.clear();
  g_monitor_link_status.clear();
  g_link_status_ws_data_set.clear();
  g_device_system_status_ws_data_set.clear();
  g_monitor_device_traffic.clear();
  g_monitor_link_traffic.clear();
  g_monitor_time_status.clear();
  g_monitor_basic_status.clear();
  g_monitor_rstp_status.clear();

  while (this->GetSystemStatus() == ActSystemStatusEnum::kMonitoring) {
    bool data_processed = false;
    bool device_handled = false;

    qint64 last_report_timestamp = QDateTime::currentSecsSinceEpoch();

    // act_status = this->GetProject(monitor_project_id, monitor_project_);
    // if (!IsActStatusSuccess(act_status)) {
    //   qCritical() << __func__ << "Get project failed with project id:" << monitor_project_id;
    //   std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
    //       ActWSResponseErrorTransfer(ActWSCommandEnum::kStartMonitor, *act_status);
    //   this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    //   continue;
    // }

    while (!monitor_process_queue_.isEmpty()) {
      {
        QMutexLocker locker(monitor_process_queue_mutex_.get());
        data = monitor_process_queue_.dequeue();
      }
      // qDebug() << "[] dequeue monitor data:" << data.ToString().toStdString().c_str();
      data_processed = true;

      QString handle_device_ip = data.GetDeviceIp();

      switch (data.GetType()) {
        case ActMonitorDataTypeEnum::kTrap: {
          if (data.GetData().canConvert<ActMqttMessage>()) {
            ActMqttMessage message = data.GetData().value<ActMqttMessage>();

            act_status = this->HandleTrapMessage(message, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleTrapMessage() failed.";
              device_handled = true;
            }

          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - Trap");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kPing: {
          if (data.GetData().canConvert<ActPingDevice>()) {
            ActPingDevice ping_device = data.GetData().value<ActPingDevice>();

            act_status = this->HandlePingResult(ping_device, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              // qCritical() << __func__ << "HandlePingResult() failed.";
              device_handled = true;
            }

            act_status = this->HandleSwiftStatus(ping_device, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleSwiftStatus() failed.";
              device_handled = true;
              break;
            }

          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - Ping");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceModuleConfigAndInterfaces: {
          if (data.GetData().canConvert<ActDevice>()) {
            ActDevice device_data = data.GetData().value<ActDevice>();

            act_status = this->HandleModuleConfigAndInterfaces(device_data, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleModuleConfigAndInterfaces() failed.";
              device_handled = true;
              break;
            }

          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceModuleConfigAndInterfaces");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kScanBasicStatus: {
          if (data.GetData().canConvert<ActMonitorBasicStatus>()) {
            ActMonitorBasicStatus device_basic_status = data.GetData().value<ActMonitorBasicStatus>();

            act_status = this->HandleBasicStatusResult(device_basic_status, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleBasicStatusResult() failed.";
              device_handled = true;
              break;
            }

          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kScanBasicStatus");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kScanTrafficData: {
          if (data.GetData().canConvert<ActDeviceMonitorTraffic>()) {
            ActDeviceMonitorTraffic traffic = data.GetData().value<ActDeviceMonitorTraffic>();

            act_status = this->HandleTrafficResult(traffic, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleTrafficResult() failed.";
              device_handled = true;
              break;
            }

          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kScanTrafficData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kScanTimeStatus: {
          if (data.GetData().canConvert<ActMonitorTimeStatus>()) {
            ActMonitorTimeStatus time_synchronization_data = data.GetData().value<ActMonitorTimeStatus>();

            act_status = this->HandleTimeStatusResult(time_synchronization_data, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleTimeStatusResult() failed.";
              device_handled = true;
              break;
            }

          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kScanTimeStatus");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceConnectionStatusData: {
          if (data.GetData().canConvert<ActDevice>()) {
            ActDevice device_data = data.GetData().value<ActDevice>();

            act_status = this->HandleDeviceConnectionStatus(device_data, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleDeviceConnectionStatus() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceConnectionStatusData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kAssignScanLinkData: {
          if (data.GetData().canConvert<ActDevice>()) {
            ActDevice device = data.GetData().value<ActDevice>();

            act_status = this->HandleAssignScanLinkData(device, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleAssignScanLinkData() failed.";
              device_handled = true;
              break;
            }

          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kAssignScanLinkData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kIdentifyDeviceData: {
          if (data.GetData().canConvert<ActDevice>()) {
            ActDevice device_data = data.GetData().value<ActDevice>();

            act_status = this->HandleIdentifyDevice(device_data, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              if (IsActStatusSkip(act_status)) {
                device_handled = true;
                break;
              }

              qCritical() << __func__ << "HandleIdentifyDevice() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kIdentifyDeviceData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kManagementEndpoint: {
          if (data.GetData().canConvert<ActSourceDevice>()) {
            ActSourceDevice src_device;

            if (this->fake_monitor_mode_) {
              qint64 current_time = QDateTime::currentSecsSinceEpoch();
              if (current_time - g_last_fake_record_timestamp <=
                  monitor_project_.GetProjectSetting().GetMonitorConfiguration().GetPollingInterval()) {
                break;
              }

              if (monitor_project_.GetDevices().isEmpty()) {
                break;
              }

              g_last_fake_record_timestamp = current_time;

              // Random select a device
              QList<ActDevice> device_list = monitor_project_.GetDevices().values();
              int max_retry = 10;
              ActDevice selected_device;
              while (1) {
                int device_idx = QRandomGenerator::global()->bounded(monitor_project_.GetDevices().size());
                selected_device = device_list.at(device_idx);
                if (selected_device.GetDeviceType() == ActDeviceTypeEnum::kSwitch ||
                    selected_device.GetDeviceType() == ActDeviceTypeEnum::kTSNSwitch) {
                  break;
                }
                max_retry--;

                if (max_retry <= 0) {
                  break;
                }
              }

              if (selected_device.GetId() == -1) {
                break;
              }

              // Random select a interface
              int intf_idx = QRandomGenerator::global()->bounded(selected_device.GetInterfaces().size());

              src_device.SetDeviceId(selected_device.GetId());
              src_device.SetInterfaceId(intf_idx);
              src_device.SetDeviceIp(selected_device.GetIpv4().GetIpAddress());
            } else {
              src_device = data.GetData().value<ActSourceDevice>();
            }

            act_status = this->HandleManagementEndpoint(src_device, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleManagementEndpoint() failed.";
              device_handled = true;
              break;
            }

          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kManagementEndpoint");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kScanLinksResult: {
          if (data.GetData().canConvert<ActScanLinksResult>()) {
            ActScanLinksResult scan_links_result = data.GetData().value<ActScanLinksResult>();

            act_status = this->HandleDeviceScanLinks(scan_links_result, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleDeviceScanLinks() failed.";
              device_handled = true;
              break;
            }

            // The final step of the monitor device
            device_handled = true;

          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kScanLinksResult");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceRSTPData: {
          if (data.GetData().canConvert<ActMonitorRstpStatus>()) {
            ActMonitorRstpStatus rstp_status = data.GetData().value<ActMonitorRstpStatus>();

            act_status = this->HandleRSTPResult(rstp_status, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleTrafficResult() failed.";
              device_handled = true;
              break;
            }

          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceRSTPData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceVLANData: {
          if (data.GetData().canConvert<ActVlanTable>()) {
            ActVlanTable vlan_table = data.GetData().value<ActVlanTable>();

            act_status = this->HandleVLANResult(vlan_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleVLANResult() failed.";
              device_handled = true;
              break;
            }

          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceVLANData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDevicePortDefaultPCPData: {
          if (data.GetData().canConvert<ActDefaultPriorityTable>()) {
            ActDefaultPriorityTable pcp = data.GetData().value<ActDefaultPriorityTable>();

            act_status = this->HandlePortDefaultPCPResult(pcp, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandlePortDefaultPCPResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDevicePortDefaultPCPData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceNetworkSettingData: {
          if (data.GetData().canConvert<ActNetworkSettingTable>()) {
            ActNetworkSettingTable network_setting_table = data.GetData().value<ActNetworkSettingTable>();

            act_status = this->HandleNetworkSettingResult(network_setting_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleNetworkSettingResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceNetworkSettingData");
            break;
          }
        } break;

        case ActMonitorDataTypeEnum::kDeviceUserAccountData: {
          if (data.GetData().canConvert<ActUserAccountTable>()) {
            ActUserAccountTable user_account_table = data.GetData().value<ActUserAccountTable>();

            act_status = this->HandleUserAccountResult(user_account_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleUserAccountResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceUserAccountData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceLoginPolicyData: {
          if (data.GetData().canConvert<ActLoginPolicyTable>()) {
            ActLoginPolicyTable login_policy_table = data.GetData().value<ActLoginPolicyTable>();

            act_status = this->HandleLoginPolicyResult(login_policy_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleLoginPolicyResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceLoginPolicyData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceSnmpTrapSettingData: {
          if (data.GetData().canConvert<ActSnmpTrapSettingTable>()) {
            ActSnmpTrapSettingTable snmp_trap_table = data.GetData().value<ActSnmpTrapSettingTable>();

            act_status = this->HandleSnmpTrapSettingResult(snmp_trap_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleSnmpTrapSettingResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceSnmpTrapSettingData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceSyslogSettingData: {
          if (data.GetData().canConvert<ActSyslogSettingTable>()) {
            ActSyslogSettingTable syslog_table = data.GetData().value<ActSyslogSettingTable>();

            act_status = this->HandleSyslogSettingResult(syslog_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleSyslogSettingResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceSyslogSettingData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceTimeSettingData: {
          if (data.GetData().canConvert<ActTimeSettingTable>()) {
            ActTimeSettingTable time_table = data.GetData().value<ActTimeSettingTable>();

            act_status = this->HandleTimeSettingResult(time_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleTimeSettingResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceTimeSettingData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDevicePortSettingData: {
          if (data.GetData().canConvert<ActPortSettingTable>()) {
            ActPortSettingTable port_table = data.GetData().value<ActPortSettingTable>();

            act_status = this->HandlePortSettingResult(port_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandlePortSettingResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDevicePortSettingData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceStreamPriorityIngressData: {
          if (data.GetData().canConvert<ActStadPortTable>()) {
            ActStadPortTable stad_port_table = data.GetData().value<ActStadPortTable>();

            act_status = this->HandleStreamPriorityIngressResult(stad_port_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleStreamPriorityIngressResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceStreamPriorityIngressData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceStreamPriorityEgressData: {
          if (data.GetData().canConvert<ActStadConfigTable>()) {
            ActStadConfigTable stad_config_table = data.GetData().value<ActStadConfigTable>();

            act_status = this->HandleStreamPriorityEgressResult(stad_config_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleStreamPriorityEgressResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceStreamPriorityEgressData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceInformationSettingData: {
          if (data.GetData().canConvert<ActInformationSettingTable>()) {
            ActInformationSettingTable info_setting_table = data.GetData().value<ActInformationSettingTable>();

            act_status = this->HandleInformationSettingResult(info_setting_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleInformationSettingResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceInformationSettingData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceManagementInterfaceData: {
          if (data.GetData().canConvert<ActManagementInterfaceTable>()) {
            ActManagementInterfaceTable mgmt_interface_table = data.GetData().value<ActManagementInterfaceTable>();

            act_status = this->HandleManagementInterfaceResult(mgmt_interface_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleManagementInterfaceResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceManagementInterfaceData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceUnicastStaticData: {
          if (data.GetData().canConvert<ActStaticForwardTable>()) {
            ActStaticForwardTable unicast_static_table = data.GetData().value<ActStaticForwardTable>();
            act_status = this->HandleUnicastStaticResult(unicast_static_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleUnicastStaticResult() failed.";
              device_handled = true;
              break;
            }

          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceUnicastStaticData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceMulticastStaticData: {
          if (data.GetData().canConvert<ActStaticForwardTable>()) {
            ActStaticForwardTable multicast_static_table = data.GetData().value<ActStaticForwardTable>();

            act_status = this->HandleMulticastStaticResult(multicast_static_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleMulticastStaticResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceMulticastStaticData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceTimeAwareShaperData: {
          if (data.GetData().canConvert<ActGclTable>()) {
            ActGclTable gcl_table = data.GetData().value<ActGclTable>();

            act_status = this->HandleTimeAwareShaperResult(gcl_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleTimeAwareShaperResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceTimeAwareShaperData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceLoopProtectionData: {
          if (data.GetData().canConvert<ActLoopProtectionTable>()) {
            ActLoopProtectionTable loop_protection_table = data.GetData().value<ActLoopProtectionTable>();

            act_status = this->HandleLoopProtectionResult(loop_protection_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleLoopProtectionResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceLoopProtectionData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceTimeSyncSettingData: {
          if (data.GetData().canConvert<ActTimeSyncTable>()) {
            ActTimeSyncTable time_sync_table = data.GetData().value<ActTimeSyncTable>();

            act_status = this->HandleTimeSyncSettingResult(time_sync_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleTimeSyncSettingResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceTimeSyncSettingData");
            break;
          }
        } break;
        case ActMonitorDataTypeEnum::kDeviceRSTPSettingData: {
          if (data.GetData().canConvert<ActRstpTable>()) {
            ActRstpTable rstp_table = data.GetData().value<ActRstpTable>();

            act_status = this->HandleRstpSettingResult(rstp_table, sync_to_websocket, send_tmp);
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "HandleRstpSettingResult() failed.";
              device_handled = true;
              break;
            }
          } else {
            // TODO: How to handle wrong type?
            qCritical("The data type is wrong with data type - kDeviceRSTPSettingData");
            break;
          }
        } break;
        default:
          break;
      }

      if (device_handled && !handle_device_ip.isEmpty()) {
        QMutexLocker locker(&g_busy_device_set_mutex);
        g_busy_device_set.remove(handle_device_ip);
        // qDebug() << "Remove busy device:" << handle_device_ip;
      }

      // Aggregate the notifications
      // If the loop is busy over 2 seconds, break the loop
      qint64 current_time = QDateTime::currentSecsSinceEpoch();
      if (current_time - last_report_timestamp >= 2) {
        last_report_timestamp = current_time;
        break;
      }
    }

    {  // protect monitor_mutex_

      // Only save project when the data is processed
      // if (data_processed) {
      // QMutexLocker lock(&act::core::g_core.mutex_);

      // act_status = act::core::g_core.UpdateProject(monitor_project_);
      // if (!IsActStatusSuccess(act_status)) {
      //   qCritical() << __func__ << "UpdateProject() failed.";
      // }
      // }

      {
        QMutexLocker lock(&this->monitor_mutex_);

        // Update SFP status per polling interval
        qint64 current_time = QDateTime::currentSecsSinceEpoch();
        if (current_time - last_sfp_update_timestamp >=
            monitor_project_.GetProjectSetting().GetMonitorConfiguration().GetPollingInterval()) {
          last_sfp_update_timestamp = current_time;

          if (!g_device_status_ws_data_set.isEmpty()) {
            ActMonitorDeviceMsg msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(),
                                    g_device_status_ws_data_set, sync_to_websocket);

            // qDebug() << "Send ActMonitorDeviceMsg:" << msg.ToString().toStdString().c_str();

            this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

            g_device_status_ws_data_set.clear();

            SLEEP_MS(50);
          }

          if (!g_device_system_status_ws_data_set.isEmpty()) {
            // Handle RSTP status
            qint64 root_device_id = -1;
            for (ActMonitorRstpStatus rstp_status : g_monitor_rstp_status) {
              // [bugfix:3478] Monitor - The device has not enabled RSTP but shows itself as the Root
              if (rstp_status.GetRootCost() == 0 &&
                  rstp_status.GetDesignatedRoot() != ACT_DEFAULT_RSTP_DESIGNATED_ROOT) {
                root_device_id = rstp_status.GetDeviceId();
                break;
              }
            }

            QSet<ActMonitorDeviceSystemStatus> device_system_status_ws_data_set;
            for (ActMonitorDeviceSystemStatus device_system_status : g_device_system_status_ws_data_set) {
              ActMonitorRstpStatus rstp_status = g_monitor_rstp_status[device_system_status.GetDeviceId()];
              if (device_system_status.GetDeviceId() == root_device_id &&
                  (rstp_status.GetDesignatedRoot() != ACT_DEFAULT_RSTP_DESIGNATED_ROOT)) {
                device_system_status.SetRole(ActRstpPortRoleEnum::kRoot);
              } else {
                device_system_status.SetRole(ActRstpPortRoleEnum::kNone);
              }
              device_system_status_ws_data_set.insert(device_system_status);
            }

            // Notify the user that the device is not alive
            ActMonitorDeviceSystemStatusMsg msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(),
                                                device_system_status_ws_data_set, sync_to_websocket);

            // qDebug() << "Send ActMonitorDeviceSystemStatusMsg:" << msg.ToString().toStdString().c_str();

            this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

            g_device_system_status_ws_data_set.clear();

            SLEEP_MS(50);
          }

          // Notify the link traffic utilization to the user
          QSet<ActLink> link_set = monitor_project_.GetLinks();
          QSet<ActLinkMonitorTraffic> link_traffic_set;
          ActSwift swift = monitor_project_.GetTopologySetting().GetRedundantGroup().GetSwift();
          QMap<qint64, qint16> tier_map = swift.GetDeviceTierMap();
          // qDebug() << "swift:" << swift.ToString().toStdString().c_str();

          for (ActLink link : link_set) {
            // Fetch source device & destination device status information
            ActMonitorBasicStatus src_dev_status = g_monitor_basic_status[link.GetSourceDeviceId()];
            ActMonitorBasicStatus dst_dev_status = g_monitor_basic_status[link.GetDestinationDeviceId()];

            QMap<qint64, ActMonitorPortStatusEntry> src_port_status_map = src_dev_status.GetPortStatus();
            QMap<qint64, ActMonitorPortStatusEntry> dst_port_status_map = dst_dev_status.GetPortStatus();

            ActMonitorPortStatusEntry src_port_status_entry = src_port_status_map[link.GetSourceInterfaceId()];
            ActMonitorPortStatusEntry dst_port_status_entry = dst_port_status_map[link.GetDestinationInterfaceId()];

            (src_port_status_entry.GetLinkStatus() == ActLinkStatusTypeEnum::kUp) &&
                    (dst_port_status_entry.GetLinkStatus() == ActLinkStatusTypeEnum::kUp)
                ? link.SetAlive(true)
                : link.SetAlive(false);
            g_link_status_ws_data_set.insert(ActMonitorLinkStatusData(link));

            g_monitor_link_status[link.GetId()] = link.GetAlive();

            // Update swift link status
            // if the link is down, the lower device should be false in the swift status
            // if the link is up, should update the device status to true in the swift status
            if (link.GetAlive()) {
              UpdateSwiftStatus(monitor_project_, link, tier_map, true);
            } else {
              UpdateSwiftStatus(monitor_project_, link, tier_map, false);
            }

            // Delete the link if the link is online dynamic link and the port status is down
            if (!g_monitor_link_status[link.GetId()] && !g_baseline_link_id_list.contains(link.GetId())) {
              // Delete the related link from the project
              act_status = DeleteLink(monitor_project_, link.GetId());
              if (!IsActStatusSuccess(act_status)) {
                qCritical() << __func__ << "Delete link failed. Link ID:" << link.GetId();
              }

              // Notify the user that the device be created
              ActLinkPatchUpdateMsg msg(ActPatchUpdateActionEnum::kDelete, monitor_project_.GetId(), link, true);
              this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

              g_monitor_link_status.remove(link.GetId());

              // Remove the list status notification from g_link_status_ws_data_set
              for (auto it = g_link_status_ws_data_set.begin(); it != g_link_status_ws_data_set.end(); ++it) {
                if (it->GetId() == link.GetId()) {
                  g_link_status_ws_data_set.erase(it);
                  break;
                }
              }
            }

            if (!g_monitor_link_status[link.GetId()]) {
              continue;
            }

            ActLinkMonitorTraffic link_traffic = g_monitor_link_traffic[link.GetId()];
            if (link_traffic.GetLinkId() == -1) {
              // The new created link doesn't has the traffic data
              continue;
            }
            link_traffic_set.insert(link_traffic);
          }

          if (!g_link_status_ws_data_set.isEmpty()) {
            // Handle redundancy link status
            QSet<ActLink> link_set = monitor_project_.GetLinks();
            for (ActLink link : link_set) {
              // For each link status, check both of the source and destination port rstp status to determine the
              // redundancy link status
              qint64 src_device_id = link.GetSourceDeviceId();
              qint64 src_interface_id = link.GetSourceInterfaceId();
              qint64 dst_device_id = link.GetDestinationDeviceId();
              qint64 dst_interface_id = link.GetDestinationInterfaceId();

              ActMonitorRstpStatus src_rstp_status = g_monitor_rstp_status[src_device_id];
              ActMonitorRstpStatus dst_rstp_status = g_monitor_rstp_status[dst_device_id];

              ActMonitorRstpPortStatusEntry src_port_status = src_rstp_status.GetPortStatus()[src_interface_id];
              ActMonitorRstpPortStatusEntry dst_port_status = dst_rstp_status.GetPortStatus()[dst_interface_id];

              // Check RSTP enable or not
              if (src_rstp_status.GetDesignatedRoot() == ACT_DEFAULT_RSTP_DESIGNATED_ROOT ||
                  dst_rstp_status.GetDesignatedRoot() == ACT_DEFAULT_RSTP_DESIGNATED_ROOT) {  // disable
                UpdateRedundantLinkStatus(link.GetId(), false);
              } else {
                // if ((src_port_status.GetPortRole() == ActRstpPortRoleEnum::kAlternate &&
                //      dst_port_status.GetPortRole() == ActRstpPortRoleEnum::kDesignated) ||
                //     (src_port_status.GetPortRole() == ActRstpPortRoleEnum::kDesignated &&
                //      dst_port_status.GetPortRole() == ActRstpPortRoleEnum::kAlternate) ||
                //     (src_port_status.GetPortRole() == ActRstpPortRoleEnum::kBackup &&
                //      dst_port_status.GetPortRole() == ActRstpPortRoleEnum::kDesignated) ||
                //     (src_port_status.GetPortRole() == ActRstpPortRoleEnum::kDesignated &&
                //      dst_port_status.GetPortRole() == ActRstpPortRoleEnum::kBackup) ||
                //     (src_port_status.GetPortState() == ActRstpPortStateEnum::kBlocking ||
                //      dst_port_status.GetPortState() == ActRstpPortStateEnum::kBlocking)) {
                if (src_port_status.GetPortRole() == ActRstpPortRoleEnum::kAlternate ||
                    src_port_status.GetPortRole() == ActRstpPortRoleEnum::kBackup ||
                    dst_port_status.GetPortRole() == ActRstpPortRoleEnum::kAlternate ||
                    dst_port_status.GetPortRole() == ActRstpPortRoleEnum::kBackup) {
                  UpdateRedundantLinkStatus(link.GetId(), true);
                } else {
                  UpdateRedundantLinkStatus(link.GetId(), false);
                }
              }
            }

            ActMonitorLinkMsg link_msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(),
                                       g_link_status_ws_data_set, sync_to_websocket);

            // qDebug() << "Send ActMonitorLinkMsg:" << link_msg.ToString().toStdString().c_str();

            this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, link_msg, monitor_project_.GetId());

            g_link_status_ws_data_set.clear();

            SLEEP_MS(50);
          }

          if (!g_swift_status_set.isEmpty()) {
            ActMonitorSwiftStatusMsg msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(),
                                         g_swift_status_set, sync_to_websocket);
            this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

            g_swift_status_set.clear();

            SLEEP_MS(50);
          }

          if (!link_traffic_set.isEmpty()) {
            ActLinkMonitorTrafficMsg msg(ActPatchUpdateActionEnum::kUpdate, monitor_project_.GetId(), link_traffic_set,
                                         sync_to_websocket);

            // qDebug() << "Send ActLinkMonitorTrafficMsg:" << msg.ToString().toStdString().c_str();

            this->SendMessageToListener(ActWSTypeEnum::kProject, send_tmp, msg, monitor_project_.GetId());

            SLEEP_MS(50);
          }

          act_status = monitor_project_.UpdateSFPList(g_monitor_link_status);
          if (!IsActStatusSuccess(act_status)) {
            qCritical() << __func__ << "GenerateSFPStatus() failed.";
          }

          this->InitNotificationTmp();

        }  // if (current_time - last_sfp_update_timestamp >=
      }  // protect monitor_mutex_

      if (data_processed) {
        sleep_time_ms = 100;  // Reset sleep time if data was processed
      } else {
        sleep_time_ms = std::min(sleep_time_ms + 100, max_sleep_time_ms);  // Increment sleep time up to max
        SLEEP_MS(sleep_time_ms);
      }
    }  // protect monitor_mutex_
  }

  qDebug() << "monitor thread finish...";
}

ACT_STATUS ActCore::StartMonitorProcessEngine(const qint64 project_id, const qint64 ws_listener_id) {
  ACT_STATUS_INIT();
  qDebug() << "Start monitor process engine";

  this->SetSystemStatus(ActSystemStatusEnum::kMonitoring);

  // Default constructor initializes the mutex
  monitor_process_queue_mutex_ = std::make_unique<QMutex>();

  // Create monitor process thread
  monitor_process_thread_ =
      std::make_unique<std::thread>(&ActCore::MonitorProcessThread, this, project_id, ws_listener_id);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"MonitorProcessThread";
  HRESULT hr = SetThreadDescription(this->monitor_process_thread_->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  return act_status;
}

void ActCore::StopMonitorProcessEngine() {
  this->SetSystemStatus(ActSystemStatusEnum::kIdle);
  monitor_process_queue_mutex_.reset();

  if (monitor_process_thread_ != nullptr && monitor_process_thread_->joinable()) {
    monitor_process_thread_->join();
  }
  qDebug() << "Stop monitor process engine";
}

}  // namespace core
}  // namespace act