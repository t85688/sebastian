/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_AUTO_SCAN_HPP
#define ACT_AUTO_SCAN_HPP

#include <QMutex>
#include <QQueue>
#include <QString>
#include <thread>

#include "act_arp_table.hpp"
#include "act_auto_scan_result.hpp"
#include "act_device_profile.hpp"
#include "act_json.hpp"
#include "act_scan_ip_range.hpp"
#include "act_southbound.hpp"
#include "act_status.hpp"
namespace act {
namespace topology {

class ActAutoScanResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActDevice, devices, Devices);
  ACT_JSON_QT_SET_OBJECTS(ActLink, links, Links);
  ACT_JSON_OBJECT(ActDeviceConfig, device_config, DeviceConfig);

  ACT_JSON_OBJECT(ActManagementInterface, management_endpoint, ManagementEndpoint);

 public:
  /**
   * @brief [feat:1662] Remove the password related fields in the project
   *
   * @return ACT_STATUS
   */
  ACT_STATUS HidePassword() {
    ACT_STATUS_INIT();

    // Hide password in each device
    QSet<ActDevice> device_set;
    for (auto dev : this->GetDevices()) {
      dev.HidePassword();
      device_set.insert(dev);
    }
    this->SetDevices(device_set);
    return act_status;
  }
};

class ActAutoScan {
  Q_GADGET

  // for thread
  QMutex mutex_;
  std::unique_ptr<std::thread> scan_topology_thread_;
  ACT_STATUS scan_topology_act_status_;
  ACT_JSON_FIELD(bool, stop_flag, StopFlag);
  ACT_JSON_FIELD(quint8, progress, Progress);
  ACT_JSON_FIELD(qint64, latest_new_dev_profile_id, LatestNewDevProfileId);
  ACT_JSON_OBJECT(ActProfiles, profiles, Profiles);

  ActSouthbound southbound_;
  QSet<ActDevice> icmp_response_devices_;
  QSet<ActDevice> alive_devices_;
  QSet<ActLink> alive_links_;
  ActDeviceConfig alive_device_config_;

  QMap<QString, QString> ip_mac_table_;
  QSet<ActDeviceProfile> new_device_profiles_;

 private:
  /**
   * @brief Triggered ScanTopology for thread
   *
   * @param scan_ip_ranges
   * @param auto_probe_license
   * @param auto_scan_result
   */
  void TriggeredScanTopologyForThread(QList<ActScanIpRangeEntry> scan_ip_ranges, const bool &auto_probe_license,
                                      ActAutoScanResult &auto_scan_result);

  /**
   * @brief Scan Topology
   *
   * @param scan_ip_ranges
   * @param auto_probe_license
   * @param auto_scan_result
   * @return ACT_STATUS
   */
  ACT_STATUS ScanTopology(QList<ActScanIpRangeEntry> &scan_ip_ranges, const bool &auto_probe_license,
                          ActAutoScanResult &auto_scan_result);

  /**
   * @brief Identify Devices
   *
   * @return ACT_STATUS
   */
  ACT_STATUS IdentifyDevices();

  /**
   * @brief Probe Devices
   *
   * @return ACT_STATUS
   */
  ACT_STATUS ProbeDevices();

  /**
   * @brief Assign Devices Configuration
   *
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDevicesConfiguration();

  // /**
  //  * @brief Update device connect status by AutoScan FeatureCapability
  //  *
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS UpdateDeviceConnectByAutoScanFeature(ActDevice &device);

  /**
   * @brief Scan Links
   *
   * @param alive_devices
   * @param result_alive_links
   * @return ACT_STATUS
   */
  ACT_STATUS ScanLinks(QSet<ActDevice> &alive_devices, QSet<ActLink> &result_alive_links);

  /**
   * @brief Assign device informations
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceInformations(ActDevice &device);

  /**
   * @brief Update interfaces of the unknown interface devices
   *
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateUnknownInterfacesDeviceInterface();

  /**
   * @brief Find the management endpoint
   *
   * @param device_remote_port_mac_map
   * @param result_management_endpoint
   * @return ACT_STATUS
   */
  ACT_STATUS act::topology::ActAutoScan::FindManagementEndpoint(
      const QMap<qint64, QMap<qint64, QString>> &device_remote_port_mac_map,
      ActManagementInterface &result_management_endpoint);

  /**
   * @brief  Update progress
   *
   * @param progress
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProgress(quint8 progress);

 public:
  QQueue<ActDeviceProfile> probe_device_profiles_queue_;

  /**
   * @brief Construct a new Act Auto Scan object
   *
   * @param profiles
   */
  ActAutoScan(const ActProfiles &profiles)
      : profiles_(profiles),
        latest_new_dev_profile_id_(10000),
        progress_(0),
        stop_flag_(false),
        scan_topology_act_status_(std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug)),
        scan_topology_thread_(nullptr) {
    southbound_.SetProfiles(profiles);
  }

  /**
   * @brief Destroy the Act Auto Scan object
   *
   */
  ~ActAutoScan();

  /**
   * @brief Start the ScanTopology by new thread
   *
   * @param scan_ip_ranges
   * @param auto_probe_license
   * @param auto_scan_result
   * @return ACT_STATUS
   */
  ACT_STATUS Start(QList<ActScanIpRangeEntry> scan_ip_ranges, const bool &auto_probe_license,
                   ActAutoScanResult &auto_scan_result);

  /**
   * @brief Stop the ScanTopology thread
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Stop();

  /**
   * @brief Get the status of the ScanTopology thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetStatus();
};

}  // namespace topology
}  // namespace act

#endif /* ACT_AUTO_SCAN_HPP */