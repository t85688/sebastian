/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_device.hpp"
#include "act_device_profile.hpp"
// #include "act_feature_profile.hpp"
#include "act_firmware_feature_profile.hpp"
#include "act_status.hpp"
#include "act_utilities.hpp"

inline ACT_STATUS GetFeaturesUsedConnectProtocol(const ActFeatureProfile &feature_profile,
                                                 ActDeviceConnectStatusControl &connect_status_control) {
  ACT_STATUS_INIT();
  const QString restful_str = kActConnectProtocolTypeEnumMap.key(ActConnectProtocolTypeEnum::kRESTful);
  const QString snmp_str = kActConnectProtocolTypeEnumMap.key(ActConnectProtocolTypeEnum::kSNMP);
  const QString netconf_str = kActConnectProtocolTypeEnumMap.key(ActConnectProtocolTypeEnum::kNETCONF);
  const QString moxa_command_str = kActConnectProtocolTypeEnumMap.key(ActConnectProtocolTypeEnum::kMOXAcommand);

  ActDeviceConnectStatusControl new_control(false, false, false, false);

  // iterator each Items
  for (auto item_key : feature_profile.GetItems().keys()) {
    // iterator each Sub-item
    for (auto sub_item_key : feature_profile.GetItems()[item_key].GetSubItems().keys()) {
      // iterator each Method
      for (auto method_key : feature_profile.GetItems()[item_key].GetSubItems()[sub_item_key].GetMethods().keys()) {
        auto method = feature_profile.GetItems()[item_key].GetSubItems()[sub_item_key].GetMethods()[method_key];
        // SNMP
        if ((!new_control.GetSNMP()) && method.GetProtocols().contains(snmp_str)) {
          new_control.SetSNMP(true);
        }

        // RESTful
        if ((!new_control.GetRESTful()) && method.GetProtocols().contains(restful_str)) {
          new_control.SetRESTful(true);
        }

        // NETCONF
        if ((!new_control.GetNETCONF()) && method.GetProtocols().contains(netconf_str)) {
          new_control.SetNETCONF(true);
        }

        // MOXAcommand
        if ((!new_control.GetNewMOXACommand()) && method.GetProtocols().contains(moxa_command_str)) {
          new_control.SetNewMOXACommand(true);
        }

        // All use would return
        if (new_control.GetSNMP() && new_control.GetNETCONF() && new_control.GetRESTful() &&
            new_control.GetNewMOXACommand()) {
          connect_status_control = new_control;
          return act_status;
        }
      }
    }
  }

  connect_status_control = new_control;
  return act_status;
}

inline ACT_STATUS GetDeviceProfileFeatureSubItem(const qint64 &device_profile_id,
                                                 const QSet<ActDeviceProfile> &device_profiles,
                                                 const ActFeatureEnum &feature_enum, const QString &item_key,
                                                 const QString &sub_item_key, ActFeatureSubItem &result_feat_sub_item) {
  ACT_STATUS_INIT();
  auto feat_str = kActFeatureEnumMap.key(feature_enum);

  // DeviceProfile > Feature > Item > SubItem

  // Get device's DeviceProfile
  ActDeviceProfile device_profile;
  act_status = ActGetItemById<ActDeviceProfile>(device_profiles, device_profile_id, device_profile);
  if (!IsActStatusSuccess(act_status)) {
    QString not_found_elem = QString("DeviceProfile(%1)").arg(device_profile_id);

    qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
    qCritical() << __func__ << QString("The device_profiles size: %1").arg(device_profiles.size());
    return std::make_shared<ActStatusNotFound>(not_found_elem);
  }

  // Get Feature element
  ActFeatureProfile feature_profile(feature_enum);
  act_status = device_profile.GetFeatureProfile(feature_profile);
  if (IsActStatusNotFound(act_status)) {
    QString not_found_elem = QString("Feature(%1) of the DeviceProfile(%2)").arg(feat_str).arg(device_profile_id);

    qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
    return std::make_shared<ActStatusNotFound>(not_found_elem);
  }

  // Get Sub-Item element
  act_status = feature_profile.GetFeatureSubItem(item_key, sub_item_key, result_feat_sub_item);
  if (IsActStatusNotFound(act_status)) {
    qCritical() << __func__
                << QString("Item not found. Item(%1) > SubItem(%2) at FeatureProfile(%3)")
                       .arg(item_key)
                       .arg(sub_item_key)
                       .arg(feat_str);

    return act_status;
  }

  return act_status;
}

inline ACT_STATUS GetDeviceFeature(const ActDevice &device,
                                   const QSet<ActFirmwareFeatureProfile> &firmware_feature_profiles,
                                   const QSet<ActDeviceProfile> &device_profiles, const ActFeatureEnum &feature_enum,
                                   ActFeatureProfile &result_feature_profile) {
  ACT_STATUS_INIT();
  auto feat_str = kActFeatureEnumMap.key(feature_enum);
  ActFeatureProfile feature_profile(feature_enum);

  // FirmwareFeatureProfile > Feature
  if (device.GetFirmwareFeatureProfileId() != -1) {
    // Get device's FirmwareFeatureProfile
    ActFirmwareFeatureProfile firmware_feature_profile;
    act_status = ActGetItemById<ActFirmwareFeatureProfile>(
        firmware_feature_profiles, device.GetFirmwareFeatureProfileId(), firmware_feature_profile);
    if (!IsActStatusSuccess(act_status)) {
      QString not_found_elem = QString("FirmwareFeatureProfile(%1)").arg(device.GetFirmwareFeatureProfileId());

      qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
      qCritical() << __func__
                  << QString("The firmware_feature_profiles size: %1").arg(firmware_feature_profiles.size());
      return std::make_shared<ActStatusNotFound>(not_found_elem);
    }

    // Get Feature element
    act_status = firmware_feature_profile.GetFeatureProfile(feature_profile);
    if (IsActStatusNotFound(act_status)) {
      QString not_found_elem = QString("Feature(%1) of the FirmwareFeatureProfile(%2)")
                                   .arg(feat_str)
                                   .arg(device.GetFirmwareFeatureProfileId());

      qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
      return std::make_shared<ActStatusNotFound>(not_found_elem);
    }
  } else {
    // DeviceProfile > Feature
    // Get device's DeviceProfile
    ActDeviceProfile device_profile;
    act_status = ActGetItemById<ActDeviceProfile>(device_profiles, device.GetDeviceProfileId(), device_profile);
    if (!IsActStatusSuccess(act_status)) {
      QString not_found_elem = QString("DeviceProfile(%1)").arg(device.GetDeviceProfileId());

      qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
      qCritical() << __func__ << QString("The device_profiles size: %1").arg(device_profiles.size());
      return std::make_shared<ActStatusNotFound>(not_found_elem);
    }

    // Get Feature element
    act_status = device_profile.GetFeatureProfile(feature_profile);
    if (IsActStatusNotFound(act_status)) {
      QString not_found_elem =
          QString("Feature(%1) of the DeviceProfile(%2)").arg(feat_str).arg(device.GetDeviceProfileId());

      qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
      return std::make_shared<ActStatusNotFound>(not_found_elem);
    }
  }

  result_feature_profile = feature_profile;

  return act_status;
}

inline ACT_STATUS GetDeviceFeatureSubItem(const ActDevice &device,
                                          const QSet<ActFirmwareFeatureProfile> &firmware_feature_profiles,
                                          const QSet<ActDeviceProfile> &device_profiles,
                                          const ActFeatureEnum &feature_enum, const QString &item_key,
                                          const QString &sub_item_key, ActFeatureSubItem &result_feat_sub_item) {
  ACT_STATUS_INIT();
  auto feat_str = kActFeatureEnumMap.key(feature_enum);
  ActFeatureProfile feature_profile(feature_enum);

  act_status = GetDeviceFeature(device, firmware_feature_profiles, device_profiles, feature_enum, feature_profile);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetDeviceFeature() failed";
    return act_status;
  }

  // Get Sub-Item element
  act_status = feature_profile.GetFeatureSubItem(item_key, sub_item_key, result_feat_sub_item);
  if (IsActStatusNotFound(act_status)) {
    qCritical() << __func__
                << QString("Item not found. Item(%1) > SubItem(%2) at FeatureProfile(%3). Device: %4(%5)")
                       .arg(item_key)
                       .arg(sub_item_key)
                       .arg(feat_str)
                       .arg(device.GetIpv4().GetIpAddress())
                       .arg(device.GetId());

    return act_status;
  }

  return act_status;
}
inline ACT_STATUS GetFeatureProfileFeatureSubItem(const QSet<ActFeatureProfile> &feature_profiles,
                                                  const ActFeatureEnum &feature_enum, const QString &item_key,
                                                  const QString &sub_item_key,
                                                  ActFeatureSubItem &result_feat_sub_item) {
  ACT_STATUS_INIT();
  auto feat_str = kActFeatureEnumMap.key(feature_enum);

  // FeatureProfile > Item > SubItem

  // Get Feature profile
  auto feature_profile_it = feature_profiles.find(ActFeatureProfile(feature_enum));
  if (feature_profile_it == feature_profiles.end()) {  // not found
    QString not_found_elem = QString("FeatureProfile(%1) not found").arg(feat_str);

    qCritical() << __func__ << not_found_elem;
    return std::make_shared<ActStatusNotFound>(not_found_elem);
  }
  ActFeatureProfile feature_profile(*feature_profile_it);

  // Get Sub-Item element
  act_status = feature_profile.GetFeatureSubItem(item_key, sub_item_key, result_feat_sub_item);
  if (IsActStatusNotFound(act_status)) {
    qCritical() << __func__
                << QString("Item not found. Item(%1) > SubItem(%2) at FeatureProfile(%3)")
                       .arg(item_key)
                       .arg(sub_item_key)
                       .arg(feat_str);
    return act_status;
  }

  return act_status;
}
