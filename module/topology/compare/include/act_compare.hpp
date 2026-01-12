/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_COMPARE_HPP
#define ACT_COMPARE_HPP

#include <QMutex>
#include <QString>
#include <thread>

#include "act_device.hpp"
#include "act_json.hpp"
#include "act_link.hpp"
#include "act_project.hpp"
#include "act_southbound.hpp"
#include "act_status.hpp"
#define ACT_INTERFACE_STATUS_UP (1)  ///< The interface status is up

namespace act {
namespace topology {

class ActCompareControl : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, device_config, DeviceConfig);  ///< DeviceConfig item
  ACT_JSON_FIELD(bool, vlan_hybrid_capable_consistent,
                 VlanHybridCapableConsistent);                    ///< VlanHybridCapableConsistent item
  ACT_JSON_FIELD(bool, alive, Alive);                             ///< Alive item
  ACT_JSON_FIELD(bool, topology_consistent, TopologyConsistent);  ///< TopologyConsistent item
  ACT_JSON_FIELD(bool, model_name, ModelName);                    ///< ModelName item
  ACT_JSON_FIELD(bool, yang_revision, YangRevision);              ///< YangRevision item
  ACT_JSON_FIELD(bool, interface_status, InterfaceStatus);        ///< InterfaceStatus item
  ACT_JSON_FIELD(bool, speed, Speed);                             ///< Speed item
  ACT_JSON_FIELD(bool, propagation_delay, PropagationDelay);      ///< PropagationDelay item

 public:
  /**
   * @brief Construct a new Deploy Control object
   *
   */
  ActCompareControl()
      : device_config_(true),
        vlan_hybrid_capable_consistent_(true),
        alive_(true),
        topology_consistent_(true),
        model_name_(true),
        yang_revision_(true),
        interface_status_(true),
        speed_(true),
        propagation_delay_(true) {}
};

class ActCompare {
  Q_GADGET

  // for thread
  std::unique_ptr<std::thread> compare_topology_thread_;  ///< Compare thread item
  ACT_JSON_FIELD(bool, stop_flag, StopFlag);              ///< StopFlag item
  ACT_JSON_FIELD(quint8, progress, Progress);             ///< Progress item
  ACT_STATUS compare_topology_act_status_;                ///< Compare thread status

  // member
  ACT_JSON_OBJECT(ActProject, project, Project);
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActDevice, compare_devices,
                           CompareDevices);  ///< The CompareDevices map
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActLink, compare_links,
                           CompareLinks);  ///< The CompareLinks map

  // ACT_JSON_QT_SET_OBJECTS(ActDevice, compare_devices, CompareDevices);            ///< Switches item set
  // ACT_JSON_QT_SET_OBJECTS(ActLink, compare_links, CompareLinks);                  ///< Switches item set
  ACT_JSON_OBJECT(ActProfiles, profiles, Profiles);  ///< Profiles item

  ActSouthbound southbound_;

 private:
  /**
   * @brief Triggered CompareTopology for thread
   *
   * @param project
   * @param dev_id_list
   * @param compare_control
   */
  void TriggeredCompareTopologyForThread(const ActProject &project, const QList<qint64> &dev_id_list,
                                         const ActCompareControl &compare_control);

  /**
   * @brief Device error log handler
   *
   * @param called_func
   * @param error_reason
   * @param device
   */
  void DeviceErrorLogHandler(QString called_func, const QString &error_reason, const ActDevice &device);

  /**
   * @brief Generate CompareDevices and CompareLinks
   *
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateCompareDevicesAndLinks(const QList<qint64> &dev_id_list);

  /**
   * @brief Create a Link String object
   *
   * @param link
   * @param link_str
   * @return ACT_STATUS
   */
  ACT_STATUS CreateLinkString(const ActLink link, QString &link_str);

  /**
   * @brief Check Vlan Hybrid capable consistent
   *
   * @return ACT_STATUS
   */
  ACT_STATUS CheckVlanHybridCapableConsistent();

  /**
   * @brief Check Device Config
   *
   * @return ACT_STATUS
   */
  ACT_STATUS CheckDeviceConfig();

  /**
   * @brief Check Alive
   *
   * @return ACT_STATUS
   */
  ACT_STATUS CheckAlive();

  /**
   * @brief Update devices ConnectStatus & Enable SNMP service
   *
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateDevicesConnectStatusAndEnableSnmp();

  /**
   * @brief Check TopologyConsistent
   *
   * @return ACT_STATUS
   */
  ACT_STATUS CheckTopologyConsistent();

  /**
   * @brief Check ModelName
   *
   * @param devices
   * @return ACT_STATUS
   */
  ACT_STATUS CheckModelName();

  // /**
  //  * @brief Check Interface Status
  //  *
  //  * @param devices
  //  * @param links
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS CheckInterfaceStatus();

  /**
   * @brief Check Link Speed
   *
   * @param devices
   * @param links
   * @return ACT_STATUS
   */
  ACT_STATUS CheckLinkSpeed();

  // /**
  //  * @brief Check Propagation Delay
  //  *
  //  * @param devices
  //  * @param links
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS CheckPropagationDelay();

  /**
   * @brief Check Link's devices are compared device
   *
   * @param link
   * @return ACT_STATUS
   */
  bool CheckLinkDevicesAreComparedDevice(const ActLink &link);

  // /**
  //  * @brief Check Link's devices are TsnSwitch
  //  *
  //  * @param link
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS CheckLinkDevicesAreTsnSwitch(const ActLink &link);

  /**
   * @brief Get the Device Feature Capability Yang Revision object
   *
   * @param device
   * @param result_yang_revisions
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceFeatureYangRevision(const ActDevice &device, QSet<QString> &result_yang_revisions);

  /**
   * @brief  Update progress
   *
   * @param progress
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProgress(quint8 progress);

 public:
  void SetSouthboundProfiles(const ActProfiles &profiles);
  /**
   * @brief Construct a new Act Compare object
   *
   */
  ActCompare()
      : progress_(0),
        stop_flag_(false),
        compare_topology_act_status_(std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug)),
        compare_topology_thread_(nullptr) {}

  /**
   * @brief Construct a new Act Compare object
   *
   */
  ActCompare(const ActProfiles &profiles)
      : profiles_(profiles),
        progress_(0),
        stop_flag_(false),
        compare_topology_act_status_(std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug)),
        compare_topology_thread_(nullptr) {
    southbound_.SetProfiles(profiles);
  }

  /**
   * @brief Destroy the Act Compare object
   *
   */
  ~ActCompare();

  /**
   * @brief Start the Compare thread
   *
   * @param project
   * @param dev_id_list
   * @param compare_control
   * @return ACT_STATUS
   */
  ACT_STATUS Start(const ActProject &project, const QList<qint64> &dev_id_list,
                   const ActCompareControl &compare_control);

  /**
   * @brief Compare topology
   *
   * @param project
   * @param dev_id_list
   * @param compare_control
   * @return ACT_STATUS
   */
  ACT_STATUS CompareTopology(const ActProject &project, const QList<qint64> &dev_id_list,
                             const ActCompareControl &compare_control);

  /**
   * @brief Stop the Compare thread
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Stop();

  /**
   * @brief Get the status of the Compare thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetStatus();
};

}  // namespace topology
}  // namespace act

#endif /* ACT_COMPARE_HPP */