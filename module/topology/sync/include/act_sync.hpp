/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_SYNC_HPP
#define ACT_SYNC_HPP

#include <QMutex>
#include <QQueue>
#include <QString>
#include <thread>

#include "act_json.hpp"
#include "act_project.hpp"
#include "act_southbound.hpp"
#include "act_status.hpp"
namespace act {
namespace topology {

class ActSync {
  Q_GADGET

  // for thread
  QMutex mutex_;
  std::unique_ptr<std::thread> sync_thread_;   ///< SyncMac thread item
  ACT_STATUS sync_act_status_;                 ///< SyncMac thread status
  ACT_JSON_FIELD(bool, stop_flag, StopFlag);   ///< StopFlag item
  ACT_JSON_FIELD(quint8, progress, Progress);  ///< Progress item

  // member
  ACT_JSON_OBJECT(ActProfiles, profiles, Profiles);  ///< Profiles item
  ACT_JSON_QT_DICT(QMap, qint64, QString, device_error_map, DeviceErrorMap);

  ActSouthbound southbound_;

 public:
  QQueue<ActDeviceConfigureResult> result_queue_;
  QSet<qint64> failed_device_id_set_;

 private:
  /**
   * @brief Triggered Sync for thread
   *
   * @param project
   * @param dev_id_list
   */
  void TriggeredSyncForThread(ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Sync error handler
   *
   * @param called_func
   * @param error_reason
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS SyncErrorHandler(QString called_func, const QString &error_reason, const ActDevice &device);

  /**
   * @brief Sync the Firmware Version
   *
   * @param dev
   * @return ACT_STATUS
   */
  ACT_STATUS SyncFirmwareVersion(ActDevice &dev);

  /**
   * @brief Sync devices
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS SyncDevices(ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Sync the MAC address
   *
   * @param project
   * @return ACT_STATUS
   */
  ACT_STATUS Sync(ActProject &project);

  /**
   * @brief  Update progress
   *
   * @param progress
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProgress(quint8 progress);

  // /**
  //  * @brief Sync Devices MAC address
  //  *
  //  * @param project
  //  * @param sync_mac_mapping_map
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS SyncDevicesMac(ActProject &project, QMap<QString, QString> &sync_mac_mapping_map);

  // /**
  //  * @brief Update Compute Result
  //  *
  //  * @param project
  //  * @param sync_mac_mapping_map
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS UpdateComputeResult(ActProject &project, QMap<QString, QString> &sync_mac_mapping_map);

  // /**
  //  * @brief Update Streams
  //  *
  //  * @param project
  //  * @param sync_mac_mapping_map
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS UpdateStreams(ActProject &project, QMap<QString, QString> &sync_mac_mapping_map);

  // /**
  //  * @brief Update Device Config
  //  *
  //  * @param project
  //  * @param sync_mac_mapping_map
  //  * @return ACT_STATUS
  //  */
  // ACT_STATUS UpdateDeviceConfig(ActProject &project, QMap<QString, QString> &sync_mac_mapping_map);

 public:
  /**
   * @brief Construct a new Act Sync Mac object
   *
   * @param profiles
   */
  ActSync(const ActProfiles &profiles)
      : profiles_(profiles),
        progress_(0),
        stop_flag_(false),
        sync_act_status_(std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug)),
        sync_thread_(nullptr) {
    southbound_.SetProfiles(profiles);
  }

  /**
   * @brief Destroy the SyncMac object
   *
   */
  ~ActSync();

  /**
   * @brief Start the SyncDevices by new thread
   *
   * @param project
   * @param dev_id_list
   * @return ACT_STATUS
   */
  ACT_STATUS Start(ActProject &project, const QList<qint64> &dev_id_list);

  /**
   * @brief Stop the SyncMac thread
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Stop();

  /**
   * @brief Get the status of the SyncMac thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetStatus();
};

}  // namespace topology
}  // namespace act

#endif /* ACT_SYNC_HPP */