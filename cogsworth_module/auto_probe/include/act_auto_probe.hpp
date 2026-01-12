/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_AUTO_PROBE_HPP
#define ACT_AUTO_PROBE_HPP

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QMutex>
#include <QQueue>
#include <QString>
#include <thread>

#include "act_auto_probe_warning.hpp"
#include "act_device.hpp"
#include "act_feature_profile.hpp"
#include "act_json.hpp"
#include "act_scan_ip_range.hpp"
#include "act_southbound.hpp"
#include "act_status.hpp"

namespace act {
namespace auto_probe {

class ActAutoProbeResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // ACT_JSON_OBJECT(ActDevice, device, Device);                        ///< Device item

  ACT_JSON_FIELD(QString, ip_address, IpAddress);                    ///< IpAddress item
  ACT_JSON_OBJECT(ActDeviceProfile, device_profile, DeviceProfile);  ///< DeviceProfile item
  ACT_JSON_OBJECT(ActAutoProbeWarning, probe_report, ProbeReport);   ///< ProbeReport item
  ACT_JSON_ENUM(ActStatusType, status, Status);                      ///< Probe result status

 public:
  ActAutoProbeResult() {
    ip_address_ = "";
    status_ = ActStatusType::kFailed;
  }

  ActAutoProbeResult(const QString &ip_address, const ActDeviceProfile &device_profile,
                     const ActAutoProbeWarning &probe_report, const ActStatusType &status) {
    ip_address_ = ip_address;
    device_profile_ = device_profile;
    probe_report_ = probe_report;
    status_ = status;
  }
};

class ActAutoProbe {
  Q_GADGET

  // for thread
  std::unique_ptr<std::thread> auto_probe_thread_;   ///< AutoProbe thread item
  ACT_JSON_FIELD(bool, stop_flag, StopFlag);         ///< StopFlag item
  ACT_JSON_FIELD(quint8, progress, Progress);        ///< Progress item
  ACT_STATUS auto_probe_act_status_;                 ///< AutoProbe thread status
  ACT_JSON_OBJECT(ActProfiles, profiles, Profiles);  ///< Profiles item

  // member
 private:
  ActSouthbound southbound_;
  QSet<ActDevice> icmp_response_devices_;
  bool reidentify_;
  QMutex mutex_;  // for thread

  /**
   * @brief Trigger AutoProbe for thread
   *
   * @param device
   * @param result_probe_warning
   * @param result_dev_profile
   */
  void TriggerAutoProbeForThread(ActDevice &device, ActAutoProbeWarning &result_probe_warning,
                                 ActDeviceProfile &result_dev_profile);

  /**
   * @brief Trigger AutoProbe by scan_ip_ranges for thread
   *
   * @param scan_ip_ranges
   */
  void TriggerAutoProbeByScanIpRangeForThread(QList<ActScanIpRangeEntry> &scan_ip_ranges);

  /**
   * @brief  AutoProbe devices by scan_ip_ranges
   *
   * @param scan_ip_ranges
   * @return ACT_STATUS
   */
  ACT_STATUS AutoProbeByScanIpRange(QList<ActScanIpRangeEntry> &scan_ip_ranges);

  /**
   * @brief  Update progress
   *
   * @param progress
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProgress(quint8 progress);

  /**
   * @brief AutoProbe error handler
   *
   * @param error_fun
   * @param error_reason
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS AutoProbeErrorHandler(const QString &error_fun, const QString &error_reason, const ActDevice &device);

  /**
   * @brief Disconnect error return handler
   *
   * @param device
   * @param feature
   * @param features_warning
   * @param result_probe_warning
   * @return ACT_STATUS
   */
  ACT_STATUS DisconnectReturnHandler(const ActDevice &device, const ActFeatureEnum &feature,
                                     QList<ActFeatureWarning> &features_warning,
                                     ActAutoProbeWarning &result_probe_warning);

  /**
   * @brief Not found Method error handler
   *
   * @param called_func
   * @param method
   * @return ACT_STATUS
   */
  ACT_STATUS NotFoundMethodErrorHandler(QString called_func, ActActionMethod method);

  /**
   * @brief Probe Features object
   *
   * @param device
   * @param device_profile
   * @param result_probe_warning

   * @return ACT_STATUS
   */
  ACT_STATUS ProbeFeatures(const ActDevice &device, ActDeviceProfile &device_profile,
                           ActAutoProbeWarning &result_probe_warning);

  /**
   * @brief Get the DeviceProfile infos object
   *
   * @param device
   * @param device_profile
   * @return ACT_STATUS
   */
  ACT_STATUS GetDeviceProfileInfos(const ActDevice &device, ActDeviceProfile &device_profile);

  /**
   * @brief Generate the FeatureGroup on DeviceProfile
   *
   * @param device_profile
   * @return ACT_STATUS
   */
  ACT_STATUS GenerateFeatureGroup(ActDeviceProfile &device_profile);

  /**
   * @brief Append the southbound's feature_sub_item
   *
   * @param feature_enum
   * @param item_key
   * @param sub_item_key
   * @param device
   * @param result_feature_profile
   * @param result_feature_warning
   * @param action_func
   * @return ACT_STATUS
   */
  ACT_STATUS AppendAction(const ActFeatureEnum &feature_enum, const QString &item_key, const QString &sub_item_key,
                          const ActDevice &device, ActFeatureProfile &result_feature_profile,
                          ActFeatureWarning &result_feature_warning,
                          ACT_STATUS (ActSouthbound::*action_func)(const ActDevice &, const ActFeatureSubItem &,
                                                                   ActFeatureSubItem &, ActFeatureSubItemWarning &));

  /**
   * @brief Base feature detect
   *
   * @param device
   * @param result_feature_profile
   * @param result_feature_warning
   * @return ACT_STATUS
   */
  ACT_STATUS BaseDetect(const ActDevice &device, ActFeatureProfile &result_feature_profile,
                        ActFeatureWarning &result_feature_warning);

  /**
   * @brief AutoScan feature detect
   *
   * @param device
   * @param result_feature_profile
   * @param result_feature_warning
   * @return ACT_STATUS
   */
  ACT_STATUS AutoScanDetect(const ActDevice &device, ActFeatureProfile &result_feature_profile,
                            ActFeatureWarning &result_feature_warning);

  /**
   * @brief Operation feature detect
   *
   * @param device
   * @param result_feature_profile
   * @param result_feature_warning
   * @return ACT_STATUS
   */
  ACT_STATUS OperationDetect(const ActDevice &device, ActFeatureProfile &result_feature_profile,
                             ActFeatureWarning &result_feature_warning);

  /**
   * @brief Configuration feature detect
   *
   * @param device
   * @param result_feature_profile
   * @param result_feature_warning
   * @return ACT_STATUS
   */
  ACT_STATUS ConfigurationDetect(const ActDevice &device, ActFeatureProfile &result_feature_profile,
                                 ActFeatureWarning &result_feature_warning);

 public:
  QQueue<ActAutoProbeResult> result_queue_;

  /**
   * @brief Construct a new Act Auto Probe object
   *
   * @param feature_profiles
   */
  ActAutoProbe(const ActProfiles &profiles)
      : profiles_(profiles),
        progress_(0),
        stop_flag_(false),
        reidentify_(false),
        auto_probe_act_status_(std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug)),
        auto_probe_thread_(nullptr) {
    southbound_.SetProfiles(profiles);
  }

  /**
   * @brief Destroy the Act AutoProbe object
   *
   */
  ~ActAutoProbe();

  /**
   * @brief Stop thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Stop();

  /**
   * @brief Get the status of the AutoProbe thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetStatus();

  /**
   * @brief Start AutoProbe thread object
   *
   * @param device
   * @param result_probe_warning
   * @param result_dev_profile
   * @return ACT_STATUS
   */
  ACT_STATUS StartAutoProbe(ActDevice &device, ActAutoProbeWarning &result_probe_warning,
                            ActDeviceProfile &result_dev_profile);

  /**
   * @brief AutoProbe object
   *
   * @param device
   * @param result_probe_warning
   * @param result_dev_profile
   * @return ACT_STATUS
   */
  ACT_STATUS AutoProbe(ActDevice &device, ActAutoProbeWarning &result_probe_warning,
                       ActDeviceProfile &result_dev_profile);

  /**
   * @brief Start AutoProbe by scan ip ranges object
   *
   * @param scan_ip_ranges
   * @return ACT_STATUS
   */
  ACT_STATUS StartAutoProbeByScanIpRange(QList<ActScanIpRangeEntry> &scan_ip_ranges);
};
}  // namespace auto_probe
}  // namespace act

#endif /* ACT_AUTO_PROBE_HPP */