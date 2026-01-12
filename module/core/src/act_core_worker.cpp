
#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QDebug>
#include <QElapsedTimer>
#include <QQueue>
#include <QThread>
#include <QVector>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>  // for std::max

#include "act_core.hpp"
#include "act_monitor.hpp"

namespace act {
namespace core {  // namespace core

extern QMap<qint64, ActDeviceMonitorTraffic> g_monitor_device_traffic;  // <device_id, device_traffic>
extern QSet<QString> g_busy_device_set;  // Used to prevent multiple jobs for the same device
extern QMutex g_busy_device_set_mutex;

void ActCore::StartWorkerThread(qint8 worker_id) {
  ACT_STATUS_INIT();
  qDebug() << QString("Worker[%1]: Start worker thread").arg(worker_id);

  // Trigger monitor procedure
  // Southbound module need the feature profile information
  ActProfiles profiles(this->GetFeatureProfileSet(), this->GetFirmwareFeatureProfileSet(), this->GetDeviceProfileSet(),
                       this->GetDefaultDeviceProfileSet());

  // TODO: Allen - the worker doesn't focus on the monitor module
  // it also support deploy mode in the future
  ActMonitor monitor(profiles);

  const int short_sleep_interval = 100;  // Short sleep interval (milliseconds)
  // const int long_sleep_interval = 1000;  // Long sleep interval (milliseconds)
  // const int max_empty_checks = 10;       // Maximum empty check count

  // int empty_check_count = 0;
  while (g_act_process_status == ActProcessStatus::Running) {
    act_status = ACT_STATUS_SUCCESS;
    monitor.SetFakeMode(fake_monitor_mode_);

    ActJob job;
    {
      QMutexLocker locker(&job_queue_mutex_);

      // Check job queue exist job
      if (job_queue_.isEmpty()) {
        SLEEP_MS(short_sleep_interval);  // Short sleep
        // empty_check_count++;
        // if (empty_check_count >= max_empty_checks) {
        //   SLEEP_MS(long_sleep_interval);  // Long sleep
        //   empty_check_count = 0;          // Reset empty check count
        // } else {
        //   SLEEP_MS(short_sleep_interval);  // Short sleep
        // }
        continue;
      }

      // empty_check_count = 0;  // Reset empty check count

      job = job_queue_.dequeue();
    }

    QString handle_device_ip;
    bool device_handled = false;

    switch (job.GetType()) {
      case ActJobTypeEnum::kMultiplePing: {
        if (job.GetData().canConvert<QList<ActPingJob>>()) {
          QList<ActPingJob> job_list = job.GetData().value<QList<ActPingJob>>();

          if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
            // Skip the job if the project is not under monitoring
            qDebug() << QString("Worker[%1]: Skip the job cause the project is not under monitoring.")
                            .arg(worker_id)
                            .toStdString()
                            .c_str();
            device_handled = true;
            break;
          }

          QVector<QFuture<void>> futures;
          QVector<ActPingDevice> devices(job_list.size());

          // qint64 start_time = QDateTime::currentMSecsSinceEpoch();

          for (int i = 0; i < job_list.size(); ++i) {
            const ActPingJob &ping_job = job_list[i];
            ActPingDevice &ping_device = devices[i];
            ping_device.SetId(ping_job.GetDeviceId());
            ping_device.SetIpAddress(ping_job.GetIp());
            ping_device.SetAccount(ping_job.GetAccount());
            ping_device.SetSnmpConfiguration(ping_job.GetSnmpConfiguration());
            ping_device.SetNetconfConfiguration(ping_job.GetNetconfConfiguration());
            ping_device.SetRestfulConfiguration(ping_job.GetRestfulConfiguration());
            ping_device.SetEnableSnmpSetting(ping_job.GetEnableSnmpSetting());

            // Use QtConcurrent::run to run monitor.PingDevice function concurrently
            futures.append(
                QtConcurrent::run(&monitor, &ActMonitor::MutiPingDeviceTask, ping_job, std::ref(ping_device)));
          }

          for (auto &future : futures) {
            future.waitForFinished();
          }

          // qint64 end_time = QDateTime::currentMSecsSinceEpoch();

          // Process ping results
          for (int i = 0; i < devices.size(); ++i) {
            ActPingDevice &ping_device = devices[i];
            if (ping_device.GetAlive()) {
              // qDebug() << QString("Worker[%1]: Ping done: %2. (%3 ms)")
              //                 .arg(worker_id)
              //                 .arg(ping_device.GetIpAddress())
              //                 .arg(end_time - start_time)
              //                 .toStdString()
              //                 .c_str();
            }

            // Notify the user that the device is alive
            // the following action decided by the monitor module
            // such as create a new job to scan the device details or other actions
            ActMonitorData data;
            data.AssignData<ActPingDevice>(ping_device.GetIpAddress(), ActMonitorDataTypeEnum::kPing, ping_device);
            DistributeMonitorData(data);
          }

        } else {
          // TODO: How to handle wrong type?
          qCritical("The data type is wrong with job type - Ping");
          break;
        }

      } break;
      case ActJobTypeEnum::kMultipleHeartbeat: {
        if (job.GetData().canConvert<QList<ActHeartbeatJob>>()) {
          QList<ActHeartbeatJob> job_list = job.GetData().value<QList<ActHeartbeatJob>>();

          if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
            // Skip the job if the project is not under monitoring
            qDebug() << QString("Worker[%1]: Skip the job cause the project is not under monitoring.")
                            .arg(worker_id)
                            .toStdString()
                            .c_str();
            break;
          }

          QVector<QFuture<void>> futures;
          QVector<ActPingDevice> devices(job_list.size());

          // qint64 start_time = QDateTime::currentMSecsSinceEpoch();

          for (int i = 0; i < job_list.size(); ++i) {
            const ActHeartbeatJob &heartbeat_job = job_list[i];
            ActDevice device = heartbeat_job.GetDevice();

            // Use QtConcurrent::run to run monitor.PingDevice function concurrently
            futures.append(QtConcurrent::run(&monitor, &ActMonitor::MultiKeepRestfulConnectionTask, device));
          }

          for (auto &future : futures) {
            future.waitForFinished();
          }

        } else {
          // TODO: How to handle wrong type?
          qCritical("The data type is wrong with job type - Ping");
          break;
        }

      } break;
      case ActJobTypeEnum::kIdentify: {
        if (job.GetData().canConvert<ActIdentifyJob>()) {
          ActIdentifyJob identify_job = job.GetData().value<ActIdentifyJob>();
          handle_device_ip = identify_job.GetDevice().GetIpv4().GetIpAddress();

          // Create monitor data
          ActDevice device = identify_job.GetDevice();

          if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
            // Skip the job if the project is not under monitoring
            qDebug() << QString("Worker[%1]: Skip the job cause the project is not under monitoring. Device IP: %2")
                            .arg(worker_id)
                            .arg(device.GetIpv4().GetIpAddress())
                            .toStdString()
                            .c_str();
            device_handled = true;
            break;
          }

          // Basic status
          // qint64 start_time = QDateTime::currentMSecsSinceEpoch();
          act_status = monitor.IdentifyNewDevice(device);
          // qint64 end_time = QDateTime::currentMSecsSinceEpoch();

          // qDebug() << QString("Worker[%1]: Identify done: %2(%3). (%4 ms)")
          //                 .arg(worker_id)
          //                 .arg(device.GetIpv4().GetIpAddress())
          //                 .arg(device.GetId())
          //                 .arg(end_time - start_time)
          //                 .toStdString()
          //                 .c_str();
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "IdentifyNewDevice() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActDevice>(device.GetIpv4().GetIpAddress(), ActMonitorDataTypeEnum::kIdentifyDeviceData,
                                       device);
            DistributeMonitorData(data);
            // qDebug() << __func__
            //          << QString("DistributeMonitorData Device(%1) done")
            //                 .arg(device.GetIpv4().GetIpAddress())
            //                 .toStdString()
            //                 .c_str();
          }

        } else {
          // TODO: How to handle wrong type?
          qCritical("The data type is wrong with job type - Scan Basic Status");
          break;
        }
      } break;
      case ActJobTypeEnum::kScan: {
        if (job.GetData().canConvert<ActScanJob>()) {
          ActScanJob scan_job = job.GetData().value<ActScanJob>();
          handle_device_ip = scan_job.GetDevice().GetIpv4().GetIpAddress();

          // qint64 start_time;
          // qint64 end_time;

          ActDevice device = scan_job.GetDevice();

          if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
            // Skip the job if the project is not under monitoring
            qDebug() << QString("Worker[%1]: Skip the job cause the project is not under monitoring. Device IP: %2")
                            .arg(worker_id)
                            .arg(device.GetIpv4().GetIpAddress())
                            .toStdString()
                            .c_str();
            device_handled = true;
            break;
          }

          if (!scan_job.GetKeepConnectStatus()) {
            // start_time = QDateTime::currentMSecsSinceEpoch();
            act_status = monitor.UpdateDeviceConnectByMonitorFeature(device);
            // end_time = QDateTime::currentMSecsSinceEpoch();

            // qDebug() << QString("Worker[%1]: Update device connection done: %2(%3). (%4 ms)")
            //                 .arg(worker_id)
            //                 .arg(device.GetIpv4().GetIpAddress())
            //                 .arg(device.GetId())
            //                 .arg(end_time - start_time)
            //                 .toStdString()
            //                 .c_str();
            if (!IsActStatusSuccess(act_status)) {
              qCritical() << __func__ << "UpdateDeviceConnectByMonitorFeature() failed. Device IP:"
                          << device.GetIpv4().GetIpAddress();
              device_handled = true;
              break;
            } else {
              ActMonitorData data;
              data.AssignData<ActDevice>(device.GetIpv4().GetIpAddress(),
                                         ActMonitorDataTypeEnum::kDeviceConnectionStatusData, device);
              DistributeMonitorData(data);
            }
          }

          if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
            // Skip the job if the project is not under monitoring
            qDebug() << QString("Worker[%1]: Skip the job cause the project is not under monitoring. Device IP: %2")
                            .arg(worker_id)
                            .arg(device.GetIpv4().GetIpAddress())
                            .toStdString()
                            .c_str();
            device_handled = true;
            break;
          }

          // Assign Module & Interfaces
          act_status = monitor.AssignDeviceModuleAndInterfaces(device);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__
                        << "AssignDeviceModuleAndInterfaces() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActDevice>(device.GetIpv4().GetIpAddress(),
                                       ActMonitorDataTypeEnum::kDeviceModuleConfigAndInterfaces, device);
            DistributeMonitorData(data);
          }

          // Basic status
          ActMonitorBasicStatus basic_status_data;
          // start_time = QDateTime::currentMSecsSinceEpoch();
          act_status = monitor.GetBasicStatus(device, basic_status_data);
          // end_time = QDateTime::currentMSecsSinceEpoch();

          // qDebug() << QString("Worker[%1]: Get device usage done: %2(%3). (%4 ms)")
          //                 .arg(worker_id)
          //                 .arg(device.GetIpv4().GetIpAddress())
          //                 .arg(device.GetId())
          //                 .arg(end_time - start_time)
          //                 .toStdString()
          //                 .c_str();

          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "GetBasicStatus() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActMonitorBasicStatus>(device.GetIpv4().GetIpAddress(),
                                                   ActMonitorDataTypeEnum::kScanBasicStatus, basic_status_data);
            DistributeMonitorData(data);
          }

          if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
            // Skip the job if the project is not under monitoring
            qDebug() << QString("Worker[%1]: Skip the job cause the project is not under monitoring. Device IP: %2")
                            .arg(worker_id)
                            .arg(device.GetIpv4().GetIpAddress())
                            .toStdString()
                            .c_str();
            device_handled = true;
            break;
          }

          // Traffic
          ActDeviceMonitorTraffic traffic;
          if (this->fake_monitor_mode_) {
            traffic.SetDeviceId(device.GetId());
            traffic.SetDeviceIp(device.GetIpv4().GetIpAddress());

            QMap<qint64, ActDeviceMonitorTrafficEntry> &result_traffic_view_map = traffic.GetTrafficMap();

            ActDeviceMonitorTraffic prev_traffic;
            if (g_monitor_device_traffic.contains(device.GetId())) {
              prev_traffic = g_monitor_device_traffic[device.GetId()];
            } else {
              prev_traffic = traffic;
            }

            QMap<qint64, ActDeviceMonitorTrafficEntry> prev_result_traffic_view_map = prev_traffic.GetTrafficMap();

            qint64 previous_time = prev_traffic.GetTimestamp();
            traffic.SetTimestamp(previous_time + 15000);

            // According the input port number to generate the random port status
            quint16 intf_num = device.GetInterfaces().size();
            for (qint64 port_id = 1; port_id <= intf_num; port_id++) {
              ActDeviceMonitorTrafficEntry prev_traffic_entry = prev_result_traffic_view_map[port_id];

              ActDeviceMonitorTrafficEntry traffic_entry;
              traffic_entry.SetDeviceId(device.GetId());
              traffic_entry.SetPortId(port_id);

              const quint32 link_speed_bps = 1000000000;                                // 1Gbps
              const quint32 target_bandwidth_bps = link_speed_bps * 0.7;                // 70% of 1Gbps
              const quint32 target_bandwidth_bytes_per_sec = target_bandwidth_bps / 8;  // Convert to bytes per second
              const quint32 polling_interval_sec = 15;                                  // Polling interval in seconds
              const quint32 target_bytes_per_interval =
                  target_bandwidth_bytes_per_sec * polling_interval_sec;  // Total bytes per polling interval

              qint64 new_tx_total_octets = prev_traffic_entry.GetTxTotalOctets() + target_bytes_per_interval;
              qint64 new_tx_total_packets = prev_traffic_entry.GetTxTotalPackets() +
                                            (target_bytes_per_interval / 1024);  // Assuming average packet size is 1K

              traffic_entry.SetTxTotalOctets(new_tx_total_octets);
              traffic_entry.SetTxTotalPackets(new_tx_total_packets);

              traffic_entry.SetPortSpeed(1000);  // 1Gbps

              result_traffic_view_map[port_id] = traffic_entry;
            }

          } else {
            // start_time = QDateTime::currentMSecsSinceEpoch();
            act_status = monitor.GetTraffic(device, traffic);
            // end_time = QDateTime::currentMSecsSinceEpoch();

            // qDebug() << QString("Worker[%1]: Get device traffic done: %2(%3). (%4 ms)")
            //                 .arg(worker_id)
            //                 .arg(device.GetIpv4().GetIpAddress())
            //                 .arg(device.GetId())
            //                 .arg(end_time - start_time)
            //                 .toStdString()
            //                 .c_str();
          }
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "GetTraffic() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActDeviceMonitorTraffic>(device.GetIpv4().GetIpAddress(),
                                                     ActMonitorDataTypeEnum::kScanTrafficData, traffic);
            DistributeMonitorData(data);
          }

          if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
            // Skip the job if the project is not under monitoring
            qDebug() << QString("Worker[%1]: Skip the job cause the project is not under monitoring. Device IP: %2")
                            .arg(worker_id)
                            .arg(device.GetIpv4().GetIpAddress())
                            .toStdString()
                            .c_str();
            device_handled = true;
            break;
          }

          // [feat:3372] Monitor support the RSTP view
          ActMonitorRstpStatus rstp_status;
          act_status = monitor.GetRSTPStatus(device, rstp_status);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "GetRSTPStatus() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActMonitorRstpStatus>(device.GetIpv4().GetIpAddress(),
                                                  ActMonitorDataTypeEnum::kDeviceRSTPData, rstp_status);
            DistributeMonitorData(data);
          }

          // [feat:3399] Monitor support the VLAN view
          ActVlanTable vlan_table(device.GetId());
          act_status = monitor.GetVlan(device, vlan_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "GetVlan() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActVlanTable>(device.GetIpv4().GetIpAddress(), ActMonitorDataTypeEnum::kDeviceVLANData,
                                          vlan_table);
            DistributeMonitorData(data);
          }

          // Get Port Default PCP
          ActDefaultPriorityTable pcp_table(device.GetId());
          act_status = monitor.GetPortDefaultPCP(device, pcp_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "GetPortDefaultPCP() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActDefaultPriorityTable>(device.GetIpv4().GetIpAddress(),
                                                     ActMonitorDataTypeEnum::kDevicePortDefaultPCPData, pcp_table);
            DistributeMonitorData(data);
          }

          // Get Network setting
          ActNetworkSettingTable network_setting_table(device.GetId());
          act_status = monitor.GetDeviceIPConfiguration(device, network_setting_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__
                        << "GetDeviceIPConfiguration() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActNetworkSettingTable>(device.GetIpv4().GetIpAddress(),
                                                    ActMonitorDataTypeEnum::kDeviceNetworkSettingData,
                                                    network_setting_table);
            DistributeMonitorData(data);
          }

          // // Get User Account
          // ActUserAccountTable user_account_table(device.GetId());
          // act_status = monitor.GetDeviceUserAccount(device, user_account_table);
          // if (!IsActStatusSuccess(act_status)) {
          //   // TODO: Allen - How to handle the error?
          //   qCritical() << __func__ << "GetDeviceUserAccount() failed. Device IP:" <<
          //   device.GetIpv4().GetIpAddress(); device_handled = true; break;
          // } else {
          //   ActMonitorData data;
          //   data.AssignData<ActUserAccountTable>(device.GetIpv4().GetIpAddress(),
          //                                        ActMonitorDataTypeEnum::kDeviceUserAccountData, user_account_table);
          //   DistributeMonitorData(data);
          // }

          // Get Login Policy
          ActLoginPolicyTable login_policy_table(device.GetId());
          act_status = monitor.GetDeviceLoginPolicy(device, login_policy_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "GetDeviceLoginPolicy() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActLoginPolicyTable>(device.GetIpv4().GetIpAddress(),
                                                 ActMonitorDataTypeEnum::kDeviceLoginPolicyData, login_policy_table);
            DistributeMonitorData(data);
          }

          // Get Information setting
          ActInformationSettingTable info_setting_table(device.GetId());
          act_status = monitor.GetDeviceInformationSetting(device, info_setting_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__
                        << "GetDeviceInformationSetting() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActInformationSettingTable>(device.GetIpv4().GetIpAddress(),
                                                        ActMonitorDataTypeEnum::kDeviceInformationSettingData,
                                                        info_setting_table);
            DistributeMonitorData(data);
          }

          // Get Management Interface
          ActManagementInterfaceTable mgmt_interface_table(device.GetId());
          act_status = monitor.GetDeviceManagementInterface(device, mgmt_interface_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__
                        << "GetDeviceManagementInterface() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActManagementInterfaceTable>(device.GetIpv4().GetIpAddress(),
                                                         ActMonitorDataTypeEnum::kDeviceManagementInterfaceData,
                                                         mgmt_interface_table);
            DistributeMonitorData(data);
          }

          // Get Loop Protection
          ActLoopProtectionTable loop_protection_table(device.GetId());
          act_status = monitor.GetDeviceLoopProtection(device, loop_protection_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__
                        << "GetDeviceLoopProtection() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActLoopProtectionTable>(device.GetIpv4().GetIpAddress(),
                                                    ActMonitorDataTypeEnum::kDeviceLoopProtectionData,
                                                    loop_protection_table);
            DistributeMonitorData(data);
          }

          // Get SNMP Trap setting
          ActSnmpTrapSettingTable snmp_trap_setting_table(device.GetId());
          act_status = monitor.GetDeviceSnmpTrapSetting(device, snmp_trap_setting_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__
                        << "GetDeviceSnmpTrapSetting() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActSnmpTrapSettingTable>(device.GetIpv4().GetIpAddress(),
                                                     ActMonitorDataTypeEnum::kDeviceSnmpTrapSettingData,
                                                     snmp_trap_setting_table);
            DistributeMonitorData(data);
          }

          // Get Syslog setting
          ActSyslogSettingTable syslog_setting_table(device.GetId());
          act_status = monitor.GetDeviceSyslogSetting(device, syslog_setting_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "GetDeviceSyslogSetting() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActSyslogSettingTable>(device.GetIpv4().GetIpAddress(),
                                                   ActMonitorDataTypeEnum::kDeviceSyslogSettingData,
                                                   syslog_setting_table);
            DistributeMonitorData(data);
          }

          // Get Time setting
          ActTimeSettingTable time_setting_table(device.GetId());
          act_status = monitor.GetDeviceTimeSetting(device, time_setting_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "GetDeviceTimeSetting() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActTimeSettingTable>(device.GetIpv4().GetIpAddress(),
                                                 ActMonitorDataTypeEnum::kDeviceTimeSettingData, time_setting_table);
            DistributeMonitorData(data);
          }

          // Get Port setting
          ActPortSettingTable port_setting_table(device.GetId());
          act_status = monitor.GetDevicePortSetting(device, port_setting_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "GetDevicePortSetting() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActPortSettingTable>(device.GetIpv4().GetIpAddress(),
                                                 ActMonitorDataTypeEnum::kDevicePortSettingData, port_setting_table);
            DistributeMonitorData(data);
          }

          // Get Stream Priority Ingress
          ActStadPortTable stad_port_table(device.GetId());
          act_status = monitor.GetStreamPriorityIngress(device, stad_port_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__
                        << "GetStreamPriorityIngress() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActStadPortTable>(device.GetIpv4().GetIpAddress(),
                                              ActMonitorDataTypeEnum::kDeviceStreamPriorityIngressData,
                                              stad_port_table);
            DistributeMonitorData(data);
          }

          // Get Stream Priority Egress
          ActStadConfigTable stad_config_table(device.GetId());
          act_status = monitor.GetStreamPriorityEgress(device, stad_config_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__
                        << "GetStreamPriorityEgress() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActStadConfigTable>(device.GetIpv4().GetIpAddress(),
                                                ActMonitorDataTypeEnum::kDeviceStreamPriorityEgressData,
                                                stad_config_table);
            DistributeMonitorData(data);
          }

          // Get Unicast StaticForward
          ActStaticForwardTable unicast_static_table(device.GetId());
          act_status = monitor.GetDeviceUnicastStatic(device, unicast_static_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "GetDeviceUnicastStatic() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActStaticForwardTable>(device.GetIpv4().GetIpAddress(),
                                                   ActMonitorDataTypeEnum::kDeviceUnicastStaticData,
                                                   unicast_static_table);
            DistributeMonitorData(data);
          }

          // Get Multicast StaticForward
          ActStaticForwardTable multicast_static_table(device.GetId());
          act_status = monitor.GetDeviceMulticastStatic(device, multicast_static_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__
                        << "GetDeviceMulticastStatic() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActStaticForwardTable>(device.GetIpv4().GetIpAddress(),
                                                   ActMonitorDataTypeEnum::kDeviceMulticastStaticData,
                                                   multicast_static_table);
            DistributeMonitorData(data);
          }

          // Get TimeSync setting
          ActTimeSyncTable time_sync_table(device.GetId());
          act_status = monitor.GetTimeSyncSetting(device, time_sync_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "GetTimeSyncSetting() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActTimeSyncTable>(device.GetIpv4().GetIpAddress(),
                                              ActMonitorDataTypeEnum::kDeviceTimeSyncSettingData, time_sync_table);
            DistributeMonitorData(data);
          }

          // Get RSTP setting
          ActRstpTable rstp_table(device.GetId());
          act_status = monitor.GetDeviceRstpSetting(device, rstp_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "GetDeviceRstpSetting() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActRstpTable>(device.GetIpv4().GetIpAddress(),
                                          ActMonitorDataTypeEnum::kDeviceRSTPSettingData, rstp_table);
            DistributeMonitorData(data);
          }

          // Get TSN TimeAwareShaper(802.1Qbv)
          ActGclTable gcl_table(device.GetId());
          act_status = monitor.GetDeviceTimeAwareShaper(device, gcl_table);
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__
                        << "GetDeviceTimeAwareShaper() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActGclTable>(device.GetIpv4().GetIpAddress(),
                                         ActMonitorDataTypeEnum::kDeviceTimeAwareShaperData, gcl_table);
            DistributeMonitorData(data);
          }

          // Time Status
          // TODO: UI not ready
          // ActMonitorTimeStatus time_synchronization_data;
          // act_status = monitor.GetTimeSynchronization(device, time_synchronization_data);
          // if (!IsActStatusSuccess(act_status)) {
          //   qCritical() << __func__ << "GetTimeSynchronization() failed.";
          //   device_handled_ = true;
          //   break;
          // } else {  // update data
          //   // Notify the user that the device is alive
          //   // the following action decided by the monitor module
          //   // such as create a new job to scan the device details or other actions
          //   ActMonitorData data;
          //   data.AssignData<ActMonitorTimeStatus>(ActMonitorDataTypeEnum::kScanTimeStatus,
          //   time_synchronization_data); DistributeMonitorData(data);
          // }

          if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
            // Skip the job if the project is not under monitoring
            qDebug() << QString("Worker[%1]: Skip the job cause the project is not under monitoring. Device IP: %2")
                            .arg(worker_id)
                            .arg(device.GetIpv4().GetIpAddress())
                            .toStdString()
                            .c_str();
            device_handled = true;
            break;
          }

          // Assign Device ScanData (LLDP data & MAC table)
          // start_time = QDateTime::currentMSecsSinceEpoch();
          act_status = monitor.AssignDeviceScanLinkData(device);
          // end_time = QDateTime::currentMSecsSinceEpoch();

          // qDebug() << QString("Worker[%1]: Assign device link done: %2(%3). (%4 ms)")
          //                 .arg(worker_id)
          //                 .arg(device.GetIpv4().GetIpAddress())
          //                 .arg(device.GetId())
          //                 .arg(end_time - start_time)
          //                 .toStdString()
          //                 .c_str();
          if (!IsActStatusSuccess(act_status)) {
            qCritical() << __func__
                        << "AssignDeviceScanLinkData() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActDevice>(device.GetIpv4().GetIpAddress(), ActMonitorDataTypeEnum::kAssignScanLinkData,
                                       device);
            DistributeMonitorData(data);
          }

          if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
            // Skip the job if the project is not under monitoring
            qDebug() << QString("Worker[%1]: Skip the job cause the project is not under monitoring. Device IP: %2")
                            .arg(worker_id)
                            .arg(device.GetIpv4().GetIpAddress())
                            .toStdString()
                            .c_str();
            device_handled = true;
            break;
          }

          // Check device as Management Endpoint
          ActSourceDevice src_device;

          // start_time = QDateTime::currentMSecsSinceEpoch();
          act_status = monitor.CheckDeviceAsManagementEndpoint(device, src_device);
          // end_time = QDateTime::currentMSecsSinceEpoch();

          // qDebug() << QString("Worker[%1]: Check endpoint done: %2(%3). (%4 ms)")
          //                 .arg(worker_id)
          //                 .arg(device.GetIpv4().GetIpAddress())
          //                 .arg(device.GetId())
          //                 .arg(end_time - start_time)
          //                 .toStdString()
          //                 .c_str();

          if (!IsActStatusSuccess(act_status)) {
            if (IsActStatusNotFound(act_status)) {
              break;
            }

            qCritical() << __func__
                        << "CheckDeviceAsManagementEndpoint() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActSourceDevice>(device.GetIpv4().GetIpAddress(),
                                             ActMonitorDataTypeEnum::kManagementEndpoint, src_device);
            DistributeMonitorData(data);
          }

        } else {
          // TODO: How to handle wrong type?
          qCritical("The data type is wrong with job type - Scan Basic Status");
          break;
        }
      } break;
      case ActJobTypeEnum::kScanLink: {
        if (job.GetData().canConvert<ActScanLinkJob>()) {
          ActScanLinkJob scan_link_job = job.GetData().value<ActScanLinkJob>();
          handle_device_ip = scan_link_job.GetDevice().GetIpv4().GetIpAddress();
          // qDebug() << QString("Worker[%1]: ScanLink job: %2(%3)")
          //                 .arg(worker_id)
          //                 .arg(scan_link_job.GetDevice().GetIpv4().GetIpAddress())
          //                 .arg(scan_link_job.GetDevice().GetId())
          //                 .toStdString()
          //                 .c_str();

          // Create monitor data
          auto device = scan_link_job.GetDevice();
          auto project_devices = scan_link_job.GetProjectDevices();

          if (this->GetSystemStatus() != ActSystemStatusEnum::kMonitoring) {
            // Skip the job if the project is not under monitoring
            qDebug() << QString("Worker[%1]: Skip the job cause the project is not under monitoring. Device IP: %2")
                            .arg(worker_id)
                            .arg(device.GetIpv4().GetIpAddress())
                            .toStdString()
                            .c_str();
            device_handled = true;
            break;
          }

          // Generate device links
          ActScanLinksResult scan_link_result;
          scan_link_result.SetDeviceIp(device.GetIpv4().GetIpAddress());

          // qint64 start_time = QDateTime::currentMSecsSinceEpoch();
          act_status = monitor.GenerateDeviceLinks(device, project_devices, scan_link_result);
          // qint64 end_time = QDateTime::currentMSecsSinceEpoch();

          // qDebug() << QString("Worker[%1]: Scan link done: %2(%3). (%4 ms)")
          //                 .arg(worker_id)
          //                 .arg(device.GetIpv4().GetIpAddress())
          //                 .arg(device.GetId())
          //                 .arg(end_time - start_time)
          //                 .toStdString()
          //                 .c_str();
          if (!IsActStatusSuccess(act_status)) {
            // TODO: Allen - How to handle the error?
            qCritical() << __func__ << "GenerateDeviceLinks() failed. Device IP:" << device.GetIpv4().GetIpAddress();
            device_handled = true;
            break;
          } else {
            ActMonitorData data;
            data.AssignData<ActScanLinksResult>(device.GetIpv4().GetIpAddress(),
                                                ActMonitorDataTypeEnum::kScanLinksResult, scan_link_result);
            DistributeMonitorData(data);
          }

        } else {
          // TODO: How to handle wrong type?
          qCritical("The data type is wrong with job type - Scan Basic Status");
          break;
        }
      } break;
      default:
        qCritical() << "Not supported job type:" << static_cast<qint8>(job.GetType());
    }

    if (device_handled && !handle_device_ip.isEmpty()) {
      QMutexLocker locker(&g_busy_device_set_mutex);
      g_busy_device_set.remove(handle_device_ip);
      // qDebug() << "Remove busy device:" << handle_device_ip;
    }
  }

  qDebug() << "Worker[" << worker_id << "]:" << "Stop worker thread";
}

ACT_STATUS ActCore::StartWorkers() {
  ACT_STATUS_INIT();

  // Fetch the maximum number of threads supported by the hardware
  qint8 max_threads = static_cast<qint8>(QThread::idealThreadCount());
  // if (max_threads == 0) {
  //   qDebug() << "Unable to determine the number of hardware threads. Defaulting to some reasonable value.";
  //   // Assign available value
  //   max_threads = ACT_MIN_NUM_WORKER_THREAD;
  // }

  qDebug() << "Maximum number of threads supported by the system:" << max_threads;

  qint8 max_workers = std::max<qint8>(static_cast<qint8>(1), static_cast<qint8>(max_threads / 2));

  qDebug() << "Maximum number of workers:" << max_workers;

  for (qint8 worker_id = 0; worker_id < max_workers; worker_id++) {
    // Create worker job queues
    worker_job_queue_.emplace_back(QQueue<ActJob>());

    // Default constructor initializes the mutex
    worker_queue_mutex_.push_back(std::make_unique<QMutex>());
  }

  for (qint8 worker_id = 0; worker_id < max_workers; worker_id++) {
    // Create worker threads
    std::thread worker_thread(&ActCore::StartWorkerThread, this, worker_id);

#ifdef _WIN32
    // Set thread name
    std::string thread_name = "Worker_" + std::to_string(worker_id);
    HRESULT hr = SetThreadDescription(worker_thread.native_handle(),
                                      std::wstring(thread_name.begin(), thread_name.end()).c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    workers_.emplace_back(std::move(worker_thread));
  }

  return act_status;
}

void ActCore::StopWorkers() {
  qint8 num_workers = static_cast<qint8>(workers_.size());

  // Create worker threads
  for (int i = 0; i < num_workers; i++) {
    if (workers_[i].joinable()) {
      workers_[i].join();
    }
  }

  return;
}

void ActCore::DistributeWorkerJobs(const QList<ActJob> &jobs) {
  // qDebug() << "Distribute jobs:" << jobs.size();

  for (const ActJob &job : jobs) {
    if (g_act_process_status != ActProcessStatus::Running) {
      return;
    }

    // Loop to wait until the queue is under the limit
    quint64 times = 0;
    auto job_queue_size = 0;
    {
      QMutexLocker locker(&job_queue_mutex_);
      job_queue_size = job_queue_.size();
    }

    while (job_queue_size >= 100) {
      if (g_act_process_status != ActProcessStatus::Running) {
        return;
      }
      times++;
      SLEEP_MS(100);

      if (times > 100) {
        qWarning() << "The job queue is full, please check the worker status.";
      }

      {
        QMutexLocker locker(&job_queue_mutex_);
        job_queue_size = job_queue_.size();
      }
    }

    // Add the user job to the target worker's job queue
    QMutexLocker locker(&job_queue_mutex_);
    job_queue_.enqueue(job);
  }
}

}  // namespace core
}  // namespace act