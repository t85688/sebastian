#include "act_monitor.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QDebug>

namespace act {
namespace monitor {
ACT_STATUS ActMonitor::Start(const ActProject &project) {
  // Checking has the thread is running
  if (IsActStatusRunning(feature_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>(GetFeatureName());
  }

  // Init member variables
  SetProgress(0);
  SetStopFlag(false);
  feature_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to start
  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((feature_thread_ != nullptr) && (feature_thread_->joinable())) {
      feature_thread_->join();
    }
    feature_act_status_->SetStatus(ActStatusType::kRunning);
    feature_thread_ = std::make_unique<std::thread>(&ActMonitor::TriggerMonitorForThread, this, project);

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"FeatureThread";
    HRESULT hr = SetThreadDescription(feature_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    qDebug() << __func__ << QString("Start %1 thread.").arg(GetFeatureName()).toStdString().c_str();

  } catch (std::exception &e) {
    qCritical()
        << __func__
        << QString("New std::thread(%1) failed. Error: %2.").arg(GetFeatureName()).arg(e.what()).toStdString().c_str();

    feature_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>(GetFeatureName());
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*feature_act_status_), GetProgress());
}

void ActMonitor::TriggerMonitorForThread(const ActProject &project) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the thread and wait for the return, and update feature_thread_.
  try {
    feature_act_status_ = this->Monitor(project);
  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(interrupt) failed. Error:" << e.what();
    feature_act_status_ = std::make_shared<ActStatusInternalError>("Monitor");
  }
}

ACT_STATUS act::monitor::ActMonitor::Monitor(const ActProject &project) {
  ACT_STATUS_INIT();

  //   Transfer Device Set to Device List
  QList<ActDevice> device_list = project.GetDevices().values();
  for (auto device : device_list) {
    // Update Device Connect status
    act_status = UpdateDeviceConnectByMonitorFeature(device);
    if (!IsActStatusSuccess(act_status)) {
      continue;
    }

    // Create monitor data
    ActMonitorData monitor_data(device.GetId());

    // Basic status
    ActMonitorBasicStatus basic_status_data;
    act_status = GetBasicStatus(device, basic_status_data);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetBasicStatus() failed.";
    } else {  // update data
      monitor_data.SetBasicStatus(basic_status_data);
    }

    // Traffic View
    QMap<qint64, ActDeviceMonitorTrafficEntry> traffic_map_data;
    act_status = GetTraffic(device, traffic_map_data);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetBasicStatus() failed.";
    } else {  // update data
      monitor_data.SetTraffic(traffic_map_data);
    }

    // Time Synchronization
    ActMonitorTimeStatus time_synchronization_data;
    act_status = GetTimeSynchronization(device, time_synchronization_data);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetTimeSynchronization() failed.";
    } else {  // update data
      monitor_data.SetTimeSynchronization(time_synchronization_data);
    }

    // Print the Monitor Data
    qDebug() << __func__
             << QString("Device: %1, Monitor: %2")
                    .arg(device.GetIpv4().GetIpAddress())
                    .arg(monitor_data.ToString())
                    .toStdString()
                    .c_str();

    // Reset the act_status
    act_status = ACT_STATUS_SUCCESS;
  }

  SLEEP_MS(5000);
  qDebug() << "Monitoring";
}

SLEEP_MS(1000);
UpdateProgress(100);
act_status = ACT_STATUS_SUCCESS;
return act_status;
}

ACT_STATUS ActMonitor::Stop() {
  // qDebug() << __func__ << compare_topology_act_status_->ToString().toStdString().c_str();
  // Checking has the thread is running
  if (IsActStatusRunning(feature_act_status_)) {
    qDebug() << "Stop Monitor thread.";

    southbound_.SetStopFlag(true);

    // Send the stop signal to the Monitor and wait for the thread to finish.
    SetStopFlag(true);
    if ((feature_thread_ != nullptr) && (feature_thread_->joinable())) {
      feature_thread_->join();  // wait thread finished
    }
  } else {
    qDebug() << __func__ << "The Monitor thread not running.";
  }
  return std::make_shared<ActProgressStatus>(ActStatusBase(*feature_act_status_), GetProgress());
}

ACT_STATUS act::monitor::ActMonitor::UpdateDeviceConnectByMonitorFeature(ActDevice &device) {
  ACT_STATUS_INIT();

  ActFeatureProfile feature_profile;
  act_status = GetDeviceFeature(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kMonitor, feature_profile);
  if (IsActStatusSuccess(act_status)) {
    // Find Feature used connect protocols
    ActDeviceConnectStatusControl connect_status_control;
    GetFeaturesUsedConnectProtocol(feature_profile, connect_status_control);
    qDebug() << __func__
             << QString("Device(%1) Monitor features used Connect: %2")
                    .arg(device.GetIpv4().GetIpAddress())
                    .arg(connect_status_control.ToString())
                    .toStdString()
                    .c_str();

    // Update Device connect
    act_status = southbound_.FeatureAssignDeviceStatus(true, device, connect_status_control);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << __func__
               << QString("Device(%1) AssignDeviceStatus failed.")
                      .arg(device.GetIpv4().GetIpAddress())
                      .toStdString()
                      .c_str();
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS act::monitor::ActMonitor::GetBasicStatus(const ActDevice &device,
                                                    ActMonitorBasicStatus &result_basic_status) {
  ACT_STATUS_INIT();

  ActFeatureSubItem feature_sub_item;

  // System Utilization
  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetBasicStatus().GetSystemUtilization()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kMonitor, "BasicStatus", "SystemUtilization", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActMonitorSystemUtilization system_utilization;
      act_status = southbound_.ActionGetSystemUtilization(device, feature_sub_item, system_utilization);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetSystemUtilization() failed.";
      } else {  // update result
        result_basic_status.SetSystemUtilization(system_utilization);
      }
    }
  }

  // Port Status
  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetBasicStatus().GetPortStatus()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kMonitor, "BasicStatus", "PortStatus", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QMap<qint64, ActMonitorPortStatusEntry> port_status_map;
      act_status = southbound_.ActionGetPortStatus(device, feature_sub_item, port_status_map);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetPortStatus() failed.";
      } else {  // update result
        result_basic_status.SetPortStatus(port_status_map);
      }
    }
  }

  // Fiber Check
  qDebug() << __func__ << "GetFiberCheck():"
           << device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetBasicStatus().GetFiberCheck();

  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetBasicStatus().GetFiberCheck()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kMonitor, "BasicStatus", "FiberCheck", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QMap<qint64, ActMonitorFiberCheckEntry> port_fiber_map;
      act_status = southbound_.ActionGetFiberCheck(device, feature_sub_item, port_fiber_map);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetFiberCheck() failed.";
      } else {  // update result
        result_basic_status.SetFiberCheck(port_fiber_map);
      }
    }
  }

  return act_status;
}

ACT_STATUS act::monitor::ActMonitor::GetTraffic(const ActDevice &device,
                                                QMap<qint64, ActDeviceMonitorTrafficEntry> &result_traffic_map) {
  ACT_STATUS_INIT();

  result_traffic_map.clear();

  ActFeatureSubItem feature_sub_item;

  // Tx Total Octets
  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetTraffic().GetTxTotalOctets()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kMonitor, "Traffic", "TxTotalOctets", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QMap<qint64, quint64> port_tx_total_octets_map;
      act_status = southbound_.ActionGetTxTotalOctets(device, feature_sub_item, port_tx_total_octets_map);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetTxTotalOctets() failed.";
      } else {  // update result
        for (auto port_id : port_tx_total_octets_map.keys()) {
          result_traffic_map[port_id].SetTxTotalOctets(port_tx_total_octets_map[port_id]);
          result_traffic_map[port_id].SetPortId(port_id);
          result_traffic_map[port_id].SetDeviceId(device.GetId());
        }
      }
    }
  }

  // Port Status
  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetTraffic().GetTxTotalPackets()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kMonitor, "Traffic", "TxTotalPackets", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QMap<qint64, quint64> port_tx_total_packets_map;
      act_status = southbound_.ActionGetTxTotalPackets(device, feature_sub_item, port_tx_total_packets_map);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetTxTotalPackets() failed.";
      } else {  // update result
        for (auto port_id : port_tx_total_packets_map.keys()) {
          result_traffic_map[port_id].SetTxTotalPackets(port_tx_total_packets_map[port_id]);
          result_traffic_map[port_id].SetPortId(port_id);
          result_traffic_map[port_id].SetDeviceId(device.GetId());
        }
      }
    }
  }

  // Port Speed
  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetPortSpeed()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "DeviceInformation", "PortSpeed", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QMap<qint64, qint64> port_speed_map;
      act_status = southbound_.ActionGetPortSpeed(device, feature_sub_item, port_speed_map);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetPortSpeed() failed.";
      } else {  // update result
        for (auto port_id : port_speed_map.keys()) {
          result_traffic_map[port_id].SetPortSpeed(port_speed_map[port_id]);
          result_traffic_map[port_id].SetPortId(port_id);
          result_traffic_map[port_id].SetDeviceId(device.GetId());
        }
      }
    }
  }

  return act_status;
}

ACT_STATUS act::monitor::ActMonitor::GetTimeSynchronization(const ActDevice &device,
                                                            ActMonitorTimeStatus &result_time_synchronization) {
  ACT_STATUS_INIT();

  ActFeatureSubItem feature_sub_item;

  // System Utilization
  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetTimeSynchronization().GetIEEE1588_2008()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kMonitor, "TimeSynchronization", "IEEE1588_2008", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActMonitorTimeSyncStatus time_sync_status;
      act_status = southbound_.ActionGet1588TimeSyncStatus(device, feature_sub_item, time_sync_status);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGet1588TimeSyncStatus() failed.";
      } else {  // update result
        result_time_synchronization.SetIEEE1588_2008(time_sync_status);
      }
    }
  }

  // Fiber Check
  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetTimeSynchronization().GetIEEE802Dot1AS_2011()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kMonitor, "TimeSynchronization", "IEEE802Dot1AS_2011",
                                         feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActMonitorTimeSyncStatus time_sync_status;
      act_status = southbound_.ActionGetDot1ASTimeSyncStatus(device, feature_sub_item, time_sync_status);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetDot1ASTimeSyncStatus() failed.";
      } else {  // update result
        result_time_synchronization.SetIEEE802Dot1AS_2011(time_sync_status);
      }
    }
  }

  return act_status;
}

// ACT_STATUS act::monitor::ActMonitor::GetPortUtilization(const ActDevice &device,
//                                                         const ActFeatureCapability &monitor_capability,
//                                                         QMap<qint64, quint64> &port_utilization_map) {
//   ACT_STATUS_INIT();

//   ActFeatureCapability feature_capability = monitor_capability;

//   // // Get ProtSpeed
//   // QMap<qint64, qint64> port_speed_map;
//   // act_status = southbound_.ActionGetPortSpeed(device, port_speed_action_capability.GetAction(),
//   port_speed_methods,
//   //                                             port_speed_map);
//   // if (!IsActStatusSuccess(act_status)) {
//   //   return act_status;
//   // }

//   // // Get ByteCount
//   // QMap<qint64, quint64> byte_count_map;
//   // act_status = southbound_.ActionGetByteCount(device, byte_count_action_capability.GetAction(),
//   byte_count_methods,
//   //                                             byte_count_map);
//   // if (!IsActStatusSuccess(act_status)) {
//   //   return act_status;
//   // }

//   // for (auto key : port_speed_map.keys()) {
//   //   qDebug() << __func__ << QString("Port Speed: %1:
//   //   %2").arg(key).arg(port_speed_map.value(key)).toStdString().c_str();
//   // }

//   // for (auto key : byte_count_map.keys()) {
//   //   qDebug() << __func__ << QString("Byte Count: %1:
//   //   %2").arg(key).arg(byte_count_map.value(key)).toStdString().c_str();
//   // }

//   // // TODO: Check Port number(map size)

//   // // Insert Port Utilization map
//   // for (auto key : port_speed_map.keys()) {
//   //   // key is port id

//   //   // TODO: insert correct value by Calculate utilization
//   //   port_utilization_map.insert(key, port_speed_map.value(key));
//   // }

//   return act_status;
// }
}  // namespace monitor
}  // namespace act