/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include <QDebug>

#include "act_feature_profile.hpp"
#include "act_json.hpp"
#include "act_status.hpp"
#include "act_temperature.hpp"
#include "device_configuration/act_device_config.hpp"
#include "device_modules/act_power_module.hpp"
#include "json_utils.hpp"
#include "topology/act_device.hpp"

/**
 * @brief The ActDeviceProfile class
 *
 */
class ActDeviceProfile : public ActDeviceProperty {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, icon_name, IconName);
  ACT_JSON_FIELD(QString, data_version, DataVersion);
  ACT_JSON_FIELD(bool, purchasable, Purchasable);
  ACT_JSON_COLLECTION_ENUM(QList, ActServiceProfileForDeviceProfileEnum, profiles, Profiles);
  ACT_JSON_FIELD(bool, built_in_power, BuiltInPower);
  ACT_JSON_QT_DICT(QMap, qint64, QString, support_power_modules,
                   SupportPowerModules);  ///< The power module map <ID, EthernetModule>
  ACT_JSON_FIELD(quint8, support_power_slots, SupportPowerSlots);

  ACT_JSON_QT_DICT(QMap, qint64, QString, support_ethernet_modules,
                   SupportEthernetModules);  ///< The ethernet module map <ID, EthernetModule>
  ACT_JSON_FIELD(quint8, support_ethernet_slots, SupportEthernetSlots);
  ACT_JSON_FIELD(quint8, default_ethernet_slot_occupied_intfs, DefaultEthernetSlotOccupiedIntfs);

  ACT_JSON_FIELD(QString, latest_firmware_version, LatestFirmwareVersion);

  ACT_JSON_ENUM(ActDeviceTypeEnum, device_type, DeviceType);

  ACT_JSON_FIELD(QString, device_name, DeviceName);
  ACT_JSON_FIELD(bool, built_in, BuiltIn);
  ACT_JSON_FIELD(bool, hide, Hide);

  ACT_JSON_QT_SET(QString, support_firmware_versions, SupportFirmwareVersions);

  ACT_JSON_QT_SET(QString, standards_and_certifications, StandardsAndCertifications);
  ACT_JSON_QT_SET(QString, supported_interfaces, SupportedInterfaces);
  ACT_JSON_FIELD(qint64, max_port_speed, MaxPortSpeed);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActInterfaceProperty, interfaces, Interfaces);
  ACT_JSON_QT_SET_OBJECTS(ActFeatureProfile, feature_capability, FeatureCapability);

  ACT_JSON_OBJECT(ActDeviceConfig, default_device_config, DefaultDeviceConfig);

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new Device Profile object
   *
   */
  ActDeviceProfile() {
    this->id_ = -1;
    this->data_version_ = ACT_DEVICE_PROFILE_DATA_VERSION;
    this->purchasable_ = false;
    this->icon_name_ = "default.png";
    this->built_in_ = false;
    this->hide_ = false;
    this->device_type_ = ActDeviceTypeEnum::kEndStation;
    this->support_ethernet_slots_ = 0;
    this->default_ethernet_slot_occupied_intfs_ = 0;
    this->support_power_slots_ = 0;
    this->built_in_power_ = true;
    this->profiles_.append(ActServiceProfileForDeviceProfileEnum::kSelfPlanning);
    this->max_port_speed_ = 0;

    // From ActDeviceProperty
    this->SetL2Family("");
    this->SetL3Series("");
    this->SetL4Series("");
    this->SetModelName("");
    this->SetPhysicalModelName("");
    this->SetLatestFirmwareVersion("");
    this->SetDescription("");
    this->SetCertificate(false);
    this->SetVendor("");
    this->SetGCLOffsetMinDuration(0);
    this->SetGCLOffsetMaxDuration(0);
    this->SetMaxVlanCfgSize(0);
    this->SetPerQueueSize(0);
    this->SetPtpQueueId(0);
    this->SetGateControlListLength(0);
    this->SetNumberOfQueue(0);
    this->SetTickGranularity(0);
    this->SetStreamPriorityConfigIngressIndexMax(0);

    this->key_order_ = QList<QString>({QString("Id"),
                                       QString("IconName"),
                                       QString("DataVersion"),
                                       QString("Purchasable"),
                                       QString("Profiles"),
                                       QString("L2Family"),
                                       QString("L3Series"),
                                       QString("L4Series"),
                                       QString("MountType"),
                                       QString("ModelName"),
                                       QString("PhysicalModelName"),
                                       QString("LatestFirmwareVersion"),
                                       QString("SupportFirmwareVersions"),
                                       QString("Description"),
                                       QString("OperatingTemperatureC"),
                                       QString("BuiltInPower"),
                                       QString("SupportPowerModules"),
                                       QString("SupportPowerSlots"),
                                       QString("DeviceName"),
                                       QString("BuiltIn"),
                                       QString("Hide"),
                                       QString("Certificate"),
                                       QString("Vendor"),
                                       QString("DeviceType"),
                                       QString("GCLOffsetMinDuration"),
                                       QString("GCLOffsetMaxDuration"),
                                       QString("MaxVlanCfgSize"),
                                       QString("PerQueueSize"),
                                       QString("PtpQueueId"),
                                       QString("GateControlListLength"),
                                       QString("NumberOfQueue"),
                                       QString("TickGranularity"),
                                       QString("StreamPriorityConfigIngressIndexMax"),
                                       QString("ProcessingDelayMap"),
                                       QString("Ieee802Dot1cb"),
                                       QString("SupportEthernetModules"),
                                       QString("SupportEthernetSlots"),
                                       QString("DefaultEthernetSlotOccupiedIntfs"),
                                       QString("StandardsAndCertifications"),
                                       QString("SupportedInterfaces"),
                                       QString("MaxPortSpeed"),
                                       QString("Interfaces"),
                                       QString("TrafficSpecification"),
                                       QString("Ieee802VlanTag"),
                                       QString("FeatureGroup"),
                                       QString("FeatureCapability"),
                                       QString("DefaultDeviceConfig")});
  }

  /**
   * @brief Construct a new Act Device Profile object
   *
   * @param id
   */
  ActDeviceProfile(const qint64 &id) : ActDeviceProfile() { this->id_ = id; }

  /**
   * @brief Construct a new Device Profile object
   *
   * @param model_name
   */
  ActDeviceProfile(const QString &model_name) : ActDeviceProfile() { this->SetModelName(model_name); }

  ACT_STATUS ResetKeyOrderByDeviceType() {
    ACT_STATUS_INIT();
    if (this->device_type_ == ActDeviceTypeEnum::kTSNSwitch) {
      this->key_order_ = QList<QString>({QString("Id"),
                                         QString("IconName"),
                                         QString("DataVersion"),
                                         QString("Purchasable"),
                                         QString("Profiles"),
                                         QString("L2Family"),
                                         QString("L3Series"),
                                         QString("L4Series"),
                                         QString("MountType"),
                                         QString("ModelName"),
                                         QString("PhysicalModelName"),
                                         QString("LatestFirmwareVersion"),
                                         QString("SupportFirmwareVersions"),
                                         QString("Description"),
                                         QString("OperatingTemperatureC"),
                                         QString("BuiltInPower"),
                                         QString("SupportPowerModules"),
                                         QString("SupportPowerSlots"),
                                         QString("DeviceName"),
                                         QString("BuiltIn"),
                                         QString("Hide"),
                                         QString("Certificate"),
                                         QString("Vendor"),
                                         QString("DeviceType"),
                                         QString("GCLOffsetMinDuration"),
                                         QString("GCLOffsetMaxDuration"),
                                         QString("MaxVlanCfgSize"),
                                         QString("PerQueueSize"),
                                         QString("PtpQueueId"),
                                         QString("GateControlListLength"),
                                         QString("NumberOfQueue"),
                                         QString("TickGranularity"),
                                         QString("StreamPriorityConfigIngressIndexMax"),
                                         QString("ProcessingDelayMap"),
                                         QString("Ieee802Dot1cb"),
                                         QString("SupportEthernetModules"),
                                         QString("SupportEthernetSlots"),
                                         QString("DefaultEthernetSlotOccupiedIntfs"),
                                         QString("StandardsAndCertifications"),
                                         QString("SupportedInterfaces"),
                                         QString("MaxPortSpeed"),
                                         QString("Interfaces"),
                                         QString("FeatureGroup"),
                                         QString("FeatureCapability"),
                                         QString("DefaultDeviceConfig")});
    } else if (this->device_type_ == ActDeviceTypeEnum::kSwitch) {
      this->key_order_ = QList<QString>({QString("Id"),
                                         QString("IconName"),
                                         QString("DataVersion"),
                                         QString("Purchasable"),
                                         QString("Profiles"),
                                         QString("L2Family"),
                                         QString("L3Series"),
                                         QString("L4Series"),
                                         QString("MountType"),
                                         QString("ModelName"),
                                         QString("PhysicalModelName"),
                                         QString("LatestFirmwareVersion"),
                                         QString("SupportFirmwareVersions"),
                                         QString("Description"),
                                         QString("OperatingTemperatureC"),
                                         QString("BuiltInPower"),
                                         QString("SupportPowerModules"),
                                         QString("SupportPowerSlots"),
                                         QString("DeviceName"),
                                         QString("BuiltIn"),
                                         QString("Hide"),
                                         QString("Certificate"),
                                         QString("Vendor"),
                                         QString("DeviceType"),
                                         QString("MaxVlanCfgSize"),
                                         QString("PerQueueSize"),
                                         QString("PtpQueueId"),
                                         QString("NumberOfQueue"),
                                         QString("TickGranularity"),
                                         QString("ProcessingDelayMap"),
                                         QString("SupportEthernetModules"),
                                         QString("SupportEthernetSlots"),
                                         QString("DefaultEthernetSlotOccupiedIntfs"),
                                         QString("StandardsAndCertifications"),
                                         QString("SupportedInterfaces"),
                                         QString("MaxPortSpeed"),
                                         QString("Interfaces"),
                                         QString("FeatureGroup"),
                                         QString("FeatureCapability"),
                                         QString("DefaultDeviceConfig")});
    } else if (this->device_type_ == ActDeviceTypeEnum::kEndStation) {
      this->key_order_ = QList<QString>({QString("Id"),
                                         QString("IconName"),
                                         QString("DataVersion"),
                                         QString("Purchasable"),
                                         QString("Profiles"),
                                         QString("MountType"),
                                         QString("ModelName"),
                                         QString("PhysicalModelName"),
                                         QString("LatestFirmwareVersion"),
                                         QString("SupportFirmwareVersions"),
                                         QString("DeviceName"),
                                         QString("BuiltIn"),
                                         QString("Hide"),
                                         QString("Certificate"),
                                         QString("Vendor"),
                                         QString("DeviceType"),
                                         QString("Ieee802VlanTag"),
                                         QString("TrafficSpecification"),
                                         QString("Interfaces"),
                                         QString("FeatureGroup"),
                                         QString("FeatureCapability"),
                                         QString("DefaultDeviceConfig")});
    } else if (this->device_type_ == ActDeviceTypeEnum::kUnknown || this->device_type_ == ActDeviceTypeEnum::kMoxa) {
      this->key_order_ =
          QList<QString>({QString("Id"), QString("IconName"), QString("DataVersion"), QString("Purchasable"),
                          QString("Profiles"), QString("MountType"), QString("ModelName"), QString("PhysicalModelName"),
                          QString("DeviceName"), QString("BuiltIn"), QString("Hide"), QString("Certificate"),
                          QString("Vendor"), QString("DeviceType"), QString("Interfaces"), QString("FeatureGroup"),
                          QString("FeatureCapability"), QString("DefaultDeviceConfig")});
    } else if (this->device_type_ == ActDeviceTypeEnum::kICMP) {
      this->key_order_ = QList<QString>(
          {QString("Id"), QString("IconName"), QString("DataVersion"), QString("Purchasable"), QString("Profiles"),
           QString("MountType"), QString("ModelName"), QString("DeviceName"), QString("BuiltIn"), QString("Hide"),
           QString("Certificate"), QString("Vendor"), QString("DeviceType")});
    } else {  // All & kBridgedEndStation
      this->key_order_ = QList<QString>({QString("Id"),
                                         QString("IconName"),
                                         QString("DataVersion"),
                                         QString("Purchasable"),
                                         QString("Profiles"),
                                         QString("L2Family"),
                                         QString("L3Series"),
                                         QString("L4Series"),
                                         QString("MountType"),
                                         QString("ModelName"),
                                         QString("PhysicalModelName"),
                                         QString("LatestFirmwareVersion"),
                                         QString("SupportFirmwareVersions"),
                                         QString("Description"),
                                         QString("OperatingTemperatureC"),
                                         QString("BuiltInPower"),
                                         QString("SupportPowerModules"),
                                         QString("SupportPowerSlots"),
                                         QString("DeviceName"),
                                         QString("BuiltIn"),
                                         QString("Hide"),
                                         QString("Certificate"),
                                         QString("Vendor"),
                                         QString("DeviceType"),
                                         QString("GCLOffsetMinDuration"),
                                         QString("GCLOffsetMaxDuration"),
                                         QString("MaxVlanCfgSize"),
                                         QString("PerQueueSize"),
                                         QString("PtpQueueId"),
                                         QString("GateControlListLength"),
                                         QString("NumberOfQueue"),
                                         QString("TickGranularity"),
                                         QString("StreamPriorityConfigIngressIndexMax"),
                                         QString("ProcessingDelayMap"),
                                         QString("Ieee802Dot1cb"),
                                         QString("Ieee802VlanTag"),
                                         QString("TrafficSpecification"),
                                         QString("SupportEthernetModules"),
                                         QString("SupportEthernetSlots"),
                                         QString("DefaultEthernetSlotOccupiedIntfs"),
                                         QString("StandardsAndCertifications"),
                                         QString("SupportedInterfaces"),
                                         QString("MaxPortSpeed"),
                                         QString("Interfaces"),
                                         QString("FeatureGroup"),
                                         QString("FeatureCapability"),
                                         QString("DefaultDeviceConfig")});
    }
    return act_status;
  }

  /**
   * @brief Get the Interfaces Id Name Map object
   *
   * @param interfaces_name_map
   * @return ACT_STATUS
   */
  ACT_STATUS
  GetInterfacesIdNameMap(QMap<qint64, QString> &interfaces_name_map) const {
    ACT_STATUS_INIT();

    interfaces_name_map.clear();
    for (int i = 0; i < interfaces_.size(); i++) {
      qint64 inteface_id = interfaces_[i].GetInterfaceId();
      QString inteface_name = interfaces_[i].GetInterfaceName();
      interfaces_name_map.insert(inteface_id, inteface_name);
    }

    return act_status;
  }

  /**
   * @brief Get the Class Keys
   *
   * @param class_keys
   * @return ACT_STATUS
   */
  ACT_STATUS GetClassKeys(QString &class_keys) const {
    ACT_STATUS_INIT();

    QString key = this->GetModelName();

    class_keys = key;

    return act_status;
  }

  /**
   * @brief Find the device profile by model name
   *
   * @param dev_profiles
   * @param model_name
   * @param found_dev_profile
   * @return ACT_STATUS
   */
  static ACT_STATUS FindDeviceProfileByModelName(const QSet<ActDeviceProfile> dev_profiles, const QString &model_name,
                                                 ActDeviceProfile &found_dev_profile) {
    ACT_STATUS_INIT();
    for (auto dev_profile : dev_profiles) {
      if (dev_profile.GetModelName() == model_name) {
        found_dev_profile = dev_profile;
        return act_status;
      }
    }
    return std::make_shared<ActStatusNotFound>("Device Profile");
  }

  /**
   * @brief Find the device profile by model name
   *
   * @param dev_profiles
   * @param physical_model_name
   * @param found_dev_profile
   * @return ACT_STATUS
   */
  static ACT_STATUS FindDeviceProfileByPhysicalModelName(const QSet<ActDeviceProfile> dev_profiles,
                                                         const QString &physical_model_name,
                                                         ActDeviceProfile &found_dev_profile) {
    ACT_STATUS_INIT();
    for (auto dev_profile : dev_profiles) {
      if (dev_profile.GetPhysicalModelName() == physical_model_name) {
        found_dev_profile = dev_profile;
        return act_status;
      }
    }
    return std::make_shared<ActStatusNotFound>("Device Profile");
  }

  /**
   * @brief Find the device profile by PhysicalModelName & BuildInPower
   *
   * @param dev_profiles
   * @param physical_model_name
   * @param build_in_power
   * @param found_dev_profile
   * @return ACT_STATUS
   */
  static ACT_STATUS FindDeviceProfileByPhysicalModelNameAndBuildInPower(const QSet<ActDeviceProfile> dev_profiles,
                                                                        const QString &physical_model_name,
                                                                        const QString &build_in_power,
                                                                        ActDeviceProfile &found_dev_profile) {
    ACT_STATUS_INIT();
    for (auto dev_profile : dev_profiles) {
      if (dev_profile.GetPhysicalModelName() == physical_model_name) {
        if (dev_profile.GetSupportPowerModules().values().contains(build_in_power)) {
          found_dev_profile = dev_profile;
          return act_status;
        }
      }
    }
    return std::make_shared<ActStatusNotFound>("Device Profile");
  }

  static ACT_STATUS GetInitDefaultAccount(const ActDevice &dev, const QSet<ActDeviceProfile> dev_profiles,
                                          ActDeviceAccount &default_account) {
    ACT_STATUS_INIT();

    ActDeviceAccount init_default_account;
    default_account = init_default_account;

    ActDeviceProfile dev_profile;
    act_status = ActGetItemById<ActDeviceProfile>(dev_profiles, dev.GetDeviceProfileId(), dev_profile);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
    if (!dev_profile.GetDefaultDeviceConfig().GetUserAccountTables().isEmpty()) {
      auto default_account_table = dev_profile.GetDefaultDeviceConfig().GetUserAccountTables().first();
      // find admin account

      for (auto key : default_account_table.GetAccounts().keys()) {
        if (default_account_table.GetAccounts()[key].GetRole() == ActUserAccountRoleEnum::kAdmin) {
          default_account.SetUsername(default_account_table.GetAccounts()[key].GetUsername());
          default_account.SetPassword(default_account_table.GetAccounts()[key].GetPassword());
          return act_status;
        }
      }
      return std::make_shared<ActStatusNotFound>("Administrator's account");
    }
    return act_status;
  }

  /**
   * @brief Get the All Model Name By Device Type object
   *
   * @param dev_profiles
   * @param device_type
   * @param result_model_names
   * @return ACT_STATUS
   */
  static ACT_STATUS GetAllModelNameSetByDeviceType(const QSet<ActDeviceProfile> dev_profiles,
                                                   const ActDeviceTypeEnum &device_type,
                                                   QSet<QString> &result_model_names) {
    ACT_STATUS_INIT();
    result_model_names.clear();
    for (auto dev_profile : dev_profiles) {
      if (dev_profile.GetDeviceType() == device_type) {
        auto mode_name = dev_profile.GetModelName();
        result_model_names.insert(mode_name);
      }
    }
    return act_status;
  }

  /**
   * @brief Get the Method By Std Find object
   *
   * @param target_action
   * @return ACT_STATUS
   */
  ACT_STATUS GetFeatureProfile(ActFeatureProfile &feature_profile) {
    ACT_STATUS_INIT();

    // Get device profile first
    QSet<ActFeatureProfile>::const_iterator iterator = this->feature_capability_.find(feature_profile);
    if (iterator == this->feature_capability_.end()) {
      QString log_msg = QString("FeatureProfile(%1) not found in DeviceProfile(%2)")
                            .arg(kActFeatureEnumMap.key(feature_profile.GetFeature()))
                            .arg(this->id_);
      qCritical() << log_msg;
      QString error_msg = QString("FeatureProfile(%1) at the DeviceProfile(%2)")
                              .arg(kActFeatureEnumMap.key(feature_profile.GetFeature()))
                              .arg(this->id_);
      return std::make_shared<ActStatusNotFound>(error_msg);
    }

    feature_profile = ActFeatureProfile(*iterator);
    return act_status;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActDeviceProfile &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
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
  friend bool operator==(const ActDeviceProfile &x, const ActDeviceProfile &y) {
    return (x.id_ == y.id_) || (x.GetModelName() == y.GetModelName());
  }
};

class ActSimpleDeviceProfile : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, icon_name, IconName);
  ACT_JSON_FIELD(bool, purchasable, Purchasable);
  ACT_JSON_COLLECTION_ENUM(QList, ActServiceProfileForDeviceProfileEnum, profiles,
                           Profiles);  ///< The supported service profiles of the user

  // Identify
  ACT_JSON_FIELD(QString, l2_family, L2Family);
  ACT_JSON_FIELD(QString, l3_series, L3Series);
  ACT_JSON_FIELD(QString, l4_series, L4Series);
  ACT_JSON_COLLECTION_ENUM(QList, ActMountTypeEnum, mount_type, MountType);
  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(QString, physical_model_name, PhysicalModelName);
  ACT_JSON_FIELD(QString, latest_firmware_version, LatestFirmwareVersion);
  ACT_JSON_QT_SET(QString, support_firmware_versions, SupportFirmwareVersions);

  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_OBJECT(ActOperatingTemperatureC, operating_temperature_c, OperatingTemperatureC);
  ACT_JSON_ENUM(ActDeviceClusterEnum, device_cluster, DeviceCluster);

  ACT_JSON_FIELD(bool, built_in_power, BuiltInPower);
  ACT_JSON_QT_DICT(QMap, qint64, QString, support_power_modules,
                   SupportPowerModules);  ///< The power module map <ID, EthernetModule>
  ACT_JSON_FIELD(quint8, support_power_slots, SupportPowerSlots);

  ACT_JSON_QT_DICT(QMap, qint64, QString, support_ethernet_modules,
                   SupportEthernetModules);  ///< The ethernet module map <ID, EthernetModule>
  ACT_JSON_FIELD(quint8, support_ethernet_slots, SupportEthernetSlots);
  ACT_JSON_FIELD(quint8, default_ethernet_slot_occupied_intfs, DefaultEthernetSlotOccupiedIntfs);

  ACT_JSON_ENUM(ActDeviceTypeEnum, device_type, DeviceType);

  ACT_JSON_QT_SET(QString, standards_and_certifications, StandardsAndCertifications);
  ACT_JSON_QT_SET(QString, supported_interfaces, SupportedInterfaces);
  ACT_JSON_FIELD(qint64, max_port_speed, MaxPortSpeed);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActInterfaceProperty, interfaces, Interfaces);
  ACT_JSON_FIELD(QString, vendor, Vendor);
  ACT_JSON_FIELD(QString, device_name, DeviceName);

  // Others Infos
  ACT_JSON_FIELD(bool, built_in, BuiltIn);
  ACT_JSON_FIELD(bool, hide, Hide);
  ACT_JSON_FIELD(bool, certificate, Certificate);
  ACT_JSON_QT_SET(qint32, reserved_vlan, ReservedVlan);

  // FeatureGroup
  ACT_JSON_OBJECT(ActFeatureGroup, feature_group, FeatureGroup);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Simple Device Profile object
   *
   */
  ActSimpleDeviceProfile() {
    this->key_order_ = QList<QString>({QString("Id"),
                                       QString("IconName"),
                                       QString("Purchasable"),
                                       QString("Profiles"),
                                       QString("L2Family"),
                                       QString("L3Series"),
                                       QString("L4Series"),
                                       QString("MountType"),
                                       QString("ModelName"),
                                       QString("PhysicalModelName"),
                                       QString("LatestFirmwareVersion"),
                                       QString("SupportFirmwareVersions"),
                                       QString("Description"),
                                       QString("OperatingTemperatureC"),
                                       QString("DeviceCluster"),
                                       QString("BuiltInPower"),
                                       QString("SupportPowerModules"),
                                       QString("SupportPowerSlots"),
                                       QString("DeviceType"),
                                       QString("SupportEthernetModules"),
                                       QString("SupportEthernetSlots"),
                                       QString("DefaultEthernetSlotOccupiedIntfs"),
                                       QString("StandardsAndCertifications"),
                                       QString("SupportedInterfaces"),
                                       QString("MaxPortSpeed"),
                                       QString("Interfaces"),
                                       QString("Vendor"),
                                       QString("DeviceName"),
                                       QString("BuiltIn"),
                                       QString("Hide"),
                                       QString("Certificate"),
                                       QString("ReservedVlan"),
                                       QString("FeatureGroup")});
  }

  /**
   * @brief Construct a new Act Simple Device Profile object
   *
   * @param device_profile
   */
  ActSimpleDeviceProfile(ActDeviceProfile &device_profile) : ActSimpleDeviceProfile() {
    this->id_ = device_profile.GetId();
    this->icon_name_ = device_profile.GetIconName();
    this->purchasable_ = device_profile.GetPurchasable();
    this->profiles_ = device_profile.GetProfiles();
    this->l2_family_ = device_profile.GetL2Family();
    this->l3_series_ = device_profile.GetL3Series();
    this->l4_series_ = device_profile.GetL4Series();
    this->mount_type_ = device_profile.GetMountType();
    this->model_name_ = device_profile.GetModelName();
    this->physical_model_name_ = device_profile.GetPhysicalModelName();
    this->latest_firmware_version_ = device_profile.GetLatestFirmwareVersion();
    this->support_firmware_versions_ = device_profile.GetSupportFirmwareVersions();
    this->description_ = device_profile.GetDescription();
    this->operating_temperature_c_ = device_profile.GetOperatingTemperatureC();
    this->device_cluster_ = device_profile.GetDeviceCluster();

    this->built_in_power_ = device_profile.GetBuiltInPower();
    this->support_power_modules_ = device_profile.GetSupportPowerModules();
    this->support_power_slots_ = device_profile.GetSupportPowerSlots();

    this->support_ethernet_modules_ = device_profile.GetSupportEthernetModules();
    this->support_ethernet_slots_ = device_profile.GetSupportEthernetSlots();

    this->device_type_ = device_profile.GetDeviceType();

    this->standards_and_certifications_ = device_profile.GetStandardsAndCertifications();
    this->supported_interfaces_ = device_profile.GetSupportedInterfaces();
    this->max_port_speed_ = device_profile.GetMaxPortSpeed();
    this->interfaces_ = device_profile.GetInterfaces();
    this->vendor_ = device_profile.GetVendor();
    this->device_name_ = device_profile.GetDeviceName();

    this->built_in_ = device_profile.GetBuiltIn();
    this->hide_ = device_profile.GetHide();
    this->certificate_ = device_profile.GetCertificate();
    this->reserved_vlan_ = device_profile.GetReservedVlan();
    this->feature_group_ = device_profile.GetFeatureGroup();
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActSimpleDeviceProfile &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
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
  friend bool operator==(const ActSimpleDeviceProfile &x, const ActSimpleDeviceProfile &y) {
    return (x.id_ == y.id_) || (x.GetModelName() == y.GetModelName());
  }
};

class ActSimpleDeviceProfileSet : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActSimpleDeviceProfile, simple_device_profile_set, SimpleDeviceProfileSet);
};

class ActExportDeviceProfileInfo : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);     ///< The origin device profile id of the exported system
  ACT_JSON_FIELD(QString, key, Key);  ///< The ModelName

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActExportDeviceProfileInfo &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActExportDeviceProfileInfo &x, const ActExportDeviceProfileInfo &y) {
    return x.id_ == y.id_;
  }
};

class ActDeviceProfileWithDefaultDeviceConfig : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_OBJECT(ActDeviceConfig, default_device_config, DefaultDeviceConfig);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Simple Device Profile object
   *
   */
  ActDeviceProfileWithDefaultDeviceConfig() {
    this->key_order_ = QList<QString>({QString("Id"), QString("DefaultDeviceConfig")});
  }

  /**
   * @brief Construct a new Act Simple Device Profile object
   *
   * @param device_profile
   */
  ActDeviceProfileWithDefaultDeviceConfig(ActDeviceProfile &device_profile) {
    this->id_ = device_profile.GetId();
    this->default_device_config_ = device_profile.GetDefaultDeviceConfig();
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActDeviceProfileWithDefaultDeviceConfig &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
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
  friend bool operator==(const ActDeviceProfileWithDefaultDeviceConfig &x,
                         const ActDeviceProfileWithDefaultDeviceConfig &y) {
    return (x.id_ == y.id_);
  }
};

class ActDevicesProfileWithDefaultDeviceConfigSet : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActDeviceProfileWithDefaultDeviceConfig, device_profile_with_default_device_config_set,
                          DeviceProfileWithDefaultDeviceConfigSet);
};
