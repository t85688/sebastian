/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_TOPOLOGY_MAPPING_HPP
#define ACT_TOPOLOGY_MAPPING_HPP

#include <QString>
#include <thread>

#include "act_device.hpp"
#include "act_json.hpp"
#include "act_link.hpp"
#include "act_project.hpp"
#include "act_southbound.hpp"
#include "act_status.hpp"
#include "act_sync.hpp"
#include "act_topology_mapping_result.hpp"

namespace act {
namespace topology {

enum class ActScanMappingLevel { kNone = 0, kRemain = 1, kModelName = 2, kPowerModule = 3, kLineModule = 4 };

class ActTopologyMappingResultCandidate : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint32, offline_source_ip_number, OfflineSourceIpNumber);  ///< OfflineSource IP number item
  ACT_JSON_OBJECT(ActTopologyMappingResult, topology_mapping_result,
                  TopologyMappingResult);                              ///< TopologyMappingResult item
  ACT_JSON_OBJECT(ActMapTopology, offline_topology, OfflineTopology);  ///< OfflineTopology item
  ACT_JSON_FIELD(quint64, warning_item_num, WarningItemNum);           ///< Number of the Warning item
  ACT_JSON_FIELD(quint64, failed_item_num, FailedItemNum);             ///< Number of the Failed item

 public:
  ActTopologyMappingResultCandidate() {
    this->offline_source_ip_number_ = 0;
    this->warning_item_num_ = 0;
    this->failed_item_num_ = 0;
  }

  ActTopologyMappingResultCandidate(const ActTopologyMappingResult &topology_mapping_result,
                                    const ActMapTopology &offline_topology)
      : ActTopologyMappingResultCandidate() {
    // Get Offline source device IP num
    auto device_index = offline_topology.GetDevices().indexOf(ActDevice(offline_topology.GetSourceDeviceId()));
    if (device_index != -1) {
      auto offline_device = offline_topology.GetDevices().at(device_index);
      offline_device.GetIpv4().GetIpAddressNumber(this->offline_source_ip_number_);
    }

    this->warning_item_num_ = 0;
    this->failed_item_num_ = 0;

    this->topology_mapping_result_ = topology_mapping_result;
    this->offline_topology_ = offline_topology;

    for (auto item : topology_mapping_result.GetMappingReport()) {
      if (item.GetStatus() == ActDeviceMapStatus::kWarning) {
        this->warning_item_num_ += 1;
        continue;
      }
      if (item.GetStatus() == ActDeviceMapStatus::kFailed || item.GetStatus() == ActDeviceMapStatus::kNotFound) {
        this->failed_item_num_ += 1;
        continue;
      }
    }
  }
};

class ActSourceDeviceCandidate : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(quint32, ip_number, IpNumber);  ///< IP number item
  ACT_JSON_COLLECTION(QList, qint64, up_link_interfaces_id,
                      UpLinkInterfacesId);  ///< Up link Interface ID QList<InterfaceID>

 public:
  ActSourceDeviceCandidate() {
    this->id_ = -1;
    this->ip_number_ = 0;
  }

  ActSourceDeviceCandidate(const qint64 &id, const quint32 &ip_number) : ActSourceDeviceCandidate() {
    this->id_ = id;
    this->ip_number_ = ip_number;
  }

  ActSourceDeviceCandidate(const qint64 &id, const quint32 &ip_number, const QList<qint64> &up_link_interfaces_id)
      : ActSourceDeviceCandidate() {
    this->id_ = id;
    this->ip_number_ = ip_number;
    this->up_link_interfaces_id_ = up_link_interfaces_id;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActSourceDeviceCandidate &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
    // Because the operator== contains id and other fields, so we need to put all entries at the same index
    return 0;
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActSourceDeviceCandidate &x, const ActSourceDeviceCandidate &y) {
    return (x.id_ == y.id_);
  }
};

class ActTopologyMapping {
  Q_GADGET

  // for thread
  std::unique_ptr<std::thread> topology_mapping_thread_;  ///< TopologyMapping thread item
  ACT_STATUS topology_mapping_act_status_;                ///< TopologyMapping thread status
  ACT_JSON_FIELD(bool, stop_flag, StopFlag);              ///< StopFlag item
  ACT_JSON_FIELD(quint8, progress, Progress);             ///< Progress item

  // member
  ACT_JSON_OBJECT(ActProfiles, profiles, Profiles);  ///< Profiles item
  ACT_JSON_QT_DICT(QMap, QString, QString, mac_host_map, MACHostMap);
  ACT_JSON_QT_DICT(QMap, qint64, QString, sync_device_error_map, SyncDeviceErrorMap);

  ActSouthbound southbound_;

 private:
  /**
   * @brief  Update progress
   *
   * @param progress
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProgress(quint8 progress);

  /**
   * @brief Triggered Mapper for thread
   *
   * @param project
   * @param mapping_result
   */
  void TriggerMapperForThread(ActProject &project, ActTopologyMappingResult &mapping_result);

  /**
   * @brief Triggered Scan Mapper for thread
   *
   * @param project
   * @param mapping_result
   */
  void TriggerScanMapperForThread(ActProject &project, ActTopologyMappingResult &mapping_result);

  /**
   * @brief Error log handler for device
   *
   * @param error_fun
   * @param error_reason
   * @param device
   */
  void DeviceErrorLogHandler(const QString &error_fun, const QString &error_reason, const ActDevice &device);

  // /**
  //  * @brief Execute the Sync module
  //  *
  //  * @param project
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS ExecuteSyncModule(ActProject &project);

  /**
  Scan the physical topology
   *
   * @param project
   * @param result_physical_map_topology
   * @return ACT_STATUS
   */
  ACT_STATUS ScanPhysicalTopology(const ActProject &project, ActMapTopology &result_physical_map_topology);

  /**
   * @brief Assign Device configuration object
   *
   * @param project
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AssignDeviceConfiguration(const ActProject &project, ActDevice &device);

  /**
   * @brief Create a Device Link object
   *
   * @param device
   * @param alive_device_list
   * @param result_link_list
   * @return ACT_STATUS
   */
  ACT_STATUS CreateDeviceLink(const ActDevice &device, const QList<ActDevice> &alive_device_list,
                              QList<ActLink> &result_link_list);

  /**
   * @brief Find offline source device candidates
   *
   * @param offline_topology
   * @param online_topology
   * @param result_source_device_candidates
   * @return ACT_STATUS
   */
  ACT_STATUS FindOfflineSourceDeviceCandidates(const ActMapTopology &offline_topology,
                                               const ActMapTopology &online_topology,
                                               QList<qint64> &result_source_device_candidates);

  /**
   * @brief Trigger the Mapping topology by multiple offline source devices
   *
   * @param offline_topology
   * @param online_topology
   * @param offline_source_devices
   * @param mapping_result
   * @return ACT_STATUS
   */
  ACT_STATUS MappingTopologyByOfflineSources(const ActMapTopology &offline_topology,
                                             const ActMapTopology &online_topology,
                                             const QList<qint64> &offline_source_devices,
                                             ActTopologyMappingResult &mapping_result);

  /**
   * @brief Generate the DeviceLinksMap<DeviceID, QSet<LinkID>> by ActMapTopology
   *
   * @param map_topology
   * @param result_device_links_map
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateDeviceLinksMap(const ActMapTopology &map_topology,
                                    QMap<qint64, QSet<ActLink>> &result_device_links_map);

  /**
   * @brief Generate the LeaveInterfaceOppositeDeviceMap<LeaveInterfaceID, OppositeDeviceID> by Device's link
   *
   * @param check_moxa_vendor
   * @param device_id
   * @param device_links
   * @param moxa_vendor_devices_id
   * @param result_map
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateLeaveInterfaceOppositeDeviceMap(const bool &check_moxa_vendor, const qint64 device_id,
                                                     const QSet<ActLink> &device_links,
                                                     const QSet<qint64> moxa_vendor_devices_id,
                                                     QMap<qint64, qint64> &result_map);

  /**
   * @brief Generate the MOXA vendor Device set
   *
   * @param map_topology
   * @param result_device_id_set
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateOfflineMappingDeviceSet(const ActMapTopology &map_topology, QSet<qint64> &result_device_set);

  /**
   * @brief Generate the TopologyMappingResult
   *
   * @param offline_devices_result
   * @param found_online_devices
   * @param offline_end_station_devices
   * @param online_topology
   * @param result
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateTopologyMappingResult(const QMap<qint64, ActMapDeviceResultItem> &offline_devices_result,
                                           const QSet<qint64> &found_online_devices,
                                           const QSet<qint64> &offline_end_station_devices,
                                           const ActMapTopology &online_topology, ActTopologyMappingResult &result);

  /**
   * @brief Update the IP Setting map by MappingResult
   *
   * @param project
   * @param mapping_result
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateIpSettingMapByMappingResult(ActProject &project, const ActTopologyMappingResult &mapping_result);

  /**
   * @brief Update the Device's Configuration
   *
   * @param project
   * @param online_devices
   * @param mapping_result
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProjectDevicesConfiguration(ActProject &project, const QList<ActDevice> &online_devices,
                                               const ActTopologyMappingResult &mapping_result);

  /**
   * @brief Update the Manufacture's Ready devices
   *
   * @param project
   * @param mapping_result
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateManufactureReadyDevices(ActProject &project, const ActTopologyMappingResult &mapping_result);

  /**
   * @brief Mapping Offline & Online topology
   *
   * @param offline_topology
   * @param online_topology
   * @param result
   * @return ACT_STATUS
   */
  ACT_STATUS MappingTopology(const ActMapTopology &offline_topology, const ActMapTopology &online_topology,
                             ActTopologyMappingResult &result);

  /**
   * @brief Sort MapTopology's devices
   *
   * @param map_topology
   * @return ACT_STATUS
   */
  ACT_STATUS SortMapTopologyDevices(ActMapTopology &map_topology);

  /**
   * @brief Check TopologyMapping offline & online device
   *
   * @param offline_device
   * @param online_device
   * @param built_in_power
   * @param offline_devices_result
   * @param offline_device_check_failed_id_set
   * @return ACT_STATUS
   */
  ACT_STATUS CheckDevice(const ActDevice &offline_device, const ActDevice &online_device, const bool &built_in_power,
                         QMap<qint64, ActMapDeviceResultItem> &offline_devices_result,
                         QSet<qint64> &offline_device_check_failed_id_set);

 public:
  /**
   * @brief Construct a new Act TopologyMapping object
   *
   */
  ActTopologyMapping(const ActProfiles &profiles)
      : profiles_(profiles),
        progress_(0),
        stop_flag_(false),
        topology_mapping_act_status_(std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug)),
        topology_mapping_thread_(nullptr) {
    southbound_.SetProfiles(profiles);
  }

  /**
   * @brief Destroy the ActTopologyMapping object
   *
   */
  ~ActTopologyMapping();

  /**
   * @brief Start the TopologyMapping by new thread
   *
   * @param project
   * @param mapping_result
   * @return ACT_STATUS
   */
  ACT_STATUS Start(ActProject &project, ActTopologyMappingResult &mapping_result);

  /**
   * @brief Stop the TopologyMapping thread
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Stop();

  /**
   * @brief Get the status of the TopologyMapping thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetStatus();

  /**
   * @brief Mapper object
   *
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS Mapper(ActProject &project, ActTopologyMappingResult &mapping_result);

  /**
   * @brief Start the ScanMapper by new thread
   *
   * @param project
   * @param mapping_result
   * @return ACT_STATUS
   */
  ACT_STATUS StartScanMapper(ActProject &project, ActTopologyMappingResult &mapping_result);

  /**
   * @brief Scan Mapper object
   *
   * @param project
   * @param mapping_result
   * @return ACT_STATUS
   */
  ACT_STATUS ScanMapper(ActProject &project, ActTopologyMappingResult &mapping_result);
};

}  // namespace topology
}  // namespace act

#endif /* ACT_TOPOLOGY_MAPPING_HPP */