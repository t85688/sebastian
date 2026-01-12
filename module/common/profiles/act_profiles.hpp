/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_device_profile.hpp"
#include "act_feature_profile.hpp"
#include "act_firmware_feature_profile.hpp"
#include "act_json.hpp"
#include "act_status.hpp"

/**
 * @brief The Profile class
 *
 */
class ActProfiles : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActFeatureProfile, feature_profiles, FeatureProfiles);  ///< FeatureProfile set
  ACT_JSON_QT_SET_OBJECTS(ActFirmwareFeatureProfile, firmware_feature_profiles,
                          FirmwareFeatureProfiles);                            ///< FirmwareFeatureProfiles set
  ACT_JSON_QT_SET_OBJECTS(ActDeviceProfile, device_profiles, DeviceProfiles);  ///< DeviceProfiles set
  ACT_JSON_QT_SET_OBJECTS(ActDeviceProfile, default_device_profiles,
                          DefaultDeviceProfiles);  ///< DefaultDeviceProfiles set

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act Feature Profile object
   *
   */
  ActProfiles() {
    this->key_order_.append(QList<QString>({QString("FeatureProfiles"), QString("FirmwareFeatureProfiles"),
                                            QString("DeviceProfiles"), QString("DefaultDeviceProfiles")}));
  }

  /**
   * @brief Construct a new Act Feature Profile object
   *
   * @param feature
   */
  ActProfiles(const QSet<ActFeatureProfile> &feature_profiles,
              const QSet<ActFirmwareFeatureProfile> &firmware_feature_profiles,
              const QSet<ActDeviceProfile> &device_profiles, const QSet<ActDeviceProfile> &default_device_profiles)
      : ActProfiles() {
    this->feature_profiles_ = feature_profiles;
    this->firmware_feature_profiles_ = firmware_feature_profiles;
    this->device_profiles_ = device_profiles;
    this->default_device_profiles_ = default_device_profiles;
  }
};
