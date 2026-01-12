/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <QHostAddress>

#include "act_device_connection.hpp"
#include "act_device_module.hpp"
#include "act_feature.hpp"
#include "act_feature_profile.hpp"
#include "act_json.hpp"
#include "act_lldp_data.hpp"
#include "act_monitor_data.hpp"
#include "act_status.hpp"
#include "act_system.hpp"
#include "act_temperature.hpp"
#include "device_modules/act_ethernet_module.hpp"
#include "device_modules/act_power_module.hpp"
#include "simplecrypt.h"
#include "stream/act_interface.hpp"
#include "stream/act_traffic_specification.hpp"
#include "stream/field/act_ieee_802_1cb.hpp"
#include "stream/field/act_ieee_802_vlan_tag.hpp"
#include "topology/act_coordinate.hpp"

/**
 * @brief The device mount type enum class
 *
 */
enum class ActMountTypeEnum { kDinRail, kWallMount, kRackMount };

/**
 * @brief The QMap for device mount type enum mapping
 *
 */
static const QMap<QString, ActMountTypeEnum> kActMountTypeEnumMap = {{"DinRail", ActMountTypeEnum::kDinRail},
                                                                     {"WallMount", ActMountTypeEnum::kWallMount},
                                                                     {"RackMount", ActMountTypeEnum::kRackMount}};

/**
 * @brief [feat:2648] The class of copied device ids
 *
 */
class DevIds : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_COLLECTION(QList, qint64, dev_ids, DeviceIds);
};

class SkipDevIds : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_COLLECTION(QList, qint64, skip_device_ids, SkipDeviceIds);
};

class ActDeviceConnectStatusControl : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, restful, RESTful);
  ACT_JSON_FIELD(bool, snmp, SNMP);
  ACT_JSON_FIELD(bool, netconf, NETCONF);
  ACT_JSON_FIELD(bool, new_moxa_command, NewMOXACommand);

 public:
  /**
   * @brief Construct a new Act Feature Assign Device Status Control object
   *
   */
  ActDeviceConnectStatusControl() {
    this->restful_ = true;
    this->snmp_ = true;
    this->netconf_ = true;
    this->new_moxa_command_ = true;
  }

  /**
   * @brief Construct a new Act Feature Assign Device Status Control object
   *
   * @param restful
   * @param snmp
   * @param netconf
   */
  ActDeviceConnectStatusControl(const bool &restful, const bool &snmp, const bool &netconf,
                                const bool &new_moxa_command) {
    this->restful_ = restful;
    this->snmp_ = snmp;
    this->netconf_ = netconf;
    this->new_moxa_command_ = new_moxa_command;
  }
};

class ActIdentifyDeviceInfo : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(quint32, vendor_id, VendorId);
  ACT_JSON_FIELD(QString, firmware_version, FirmwareVersion);

 public:
  /**
   * @brief Construct a new Act IdentifyDeviceInfo object
   *
   */
  ActIdentifyDeviceInfo() {
    model_name_ = "";
    vendor_id_ = 0;
    firmware_version_ = "";
  }
};

/**
 * @brief The Device IpAddress connection configuration class
 *
 */
class ActDeviceIpConnectConfig : public ActDeviceConnectConfig {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, ip_address, IpAddress);
  ACT_JSON_FIELD(bool, enable_snmp_setting, EnableSnmpSetting);
};

/**
 * @brief The device type enum class
 *
 */
enum class ActDeviceTypeEnum {
  kTSNSwitch,
  kEndStation,
  kBridgedEndStation,
  kSwitch,
  kPoeAccessory,
  kNetworkMgmtSoftware,
  kICMP,
  kUnknown,
  kMoxa
};

/**
 * @brief The QMap for device type enum mapping
 *
 */
static const QMap<QString, ActDeviceTypeEnum> kActDeviceTypeEnumMap = {
    {"TSNSwitch", ActDeviceTypeEnum::kTSNSwitch},
    {"EndStation", ActDeviceTypeEnum::kEndStation},
    {"BridgedEndStation", ActDeviceTypeEnum::kBridgedEndStation},
    {"Switch", ActDeviceTypeEnum::kSwitch},
    {"PoeAccessory", ActDeviceTypeEnum::kPoeAccessory},
    {"NetworkMgmtSoftware", ActDeviceTypeEnum::kNetworkMgmtSoftware},
    {"ICMP", ActDeviceTypeEnum::kICMP},
    {"Unknown", ActDeviceTypeEnum::kUnknown},
    {"Moxa", ActDeviceTypeEnum::kMoxa}};

inline uint qHash(ActDeviceTypeEnum key) { return ::qHash(static_cast<int>(key), 0); }

inline bool operator==(ActDeviceTypeEnum &x, ActDeviceTypeEnum &y) {
  return static_cast<int>(x) == static_cast<int>(y);
}

/**
 * @brief The device role enum class for redundant view
 *
 */
enum class ActDeviceRoleEnum {
  kRSTPRoot,
  kRSTPBackupRoot,
  kRSTP,
  kSwiftRoot,
  kSwiftBackupRoot,
  kSwift,
  kRingMaster,
  kTurboRing,
  kTurboChain,
  kUnknown
};

/**
 * @brief The QMap for device role enum mapping
 *
 */
static const QMap<QString, ActDeviceRoleEnum> kActDeviceRoleEnumMap = {
    {"RSTPRoot", ActDeviceRoleEnum::kRSTPRoot},
    {"RSTPBackupRoot", ActDeviceRoleEnum::kRSTPBackupRoot},
    {"RSTP", ActDeviceRoleEnum::kRSTP},
    {"SwiftRoot", ActDeviceRoleEnum::kSwiftRoot},
    {"SwiftBackupRoot", ActDeviceRoleEnum::kSwiftBackupRoot},
    {"Swift", ActDeviceRoleEnum::kSwift},
    {"RingMaster", ActDeviceRoleEnum::kRingMaster},
    {"TurboRing", ActDeviceRoleEnum::kTurboRing},
    {"TurboChain", ActDeviceRoleEnum::kTurboChain},
    {"Unknown", ActDeviceRoleEnum::kUnknown}};

/**
 * @brief The ACT IPv4 class
 *
 */
class ActIpv4 : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, ip_address, IpAddress);
  ACT_JSON_FIELD(QString, subnet_mask, SubnetMask);
  ACT_JSON_FIELD(QString, gateway, Gateway);
  ACT_JSON_FIELD(QString, dns1, DNS1);
  ACT_JSON_FIELD(QString, dns2, DNS2);

 public:
  /**
   * @brief Construct a new Act Ipv 4 object
   *
   */
  ActIpv4() {
    ip_address_ = "";
    subnet_mask_ = "";
    gateway_ = "";
    dns1_ = "";
    dns2_ = "";
  }

  /**
   * @brief Construct a new Act Ipv 4 object
   *
   * @param ip_address
   */
  ActIpv4(const QString &ip_address) : ActIpv4() { ip_address_ = ip_address; }

  /**
   * @brief Check IpAddress is valid
   *
   * @param is_valid
   */
  bool IsValidIpAddress() {
    bool is_valid;
    QHostAddress(ip_address_).toIPv4Address(&is_valid);
    return is_valid;
  }

  // TODO: Check SubMask
  // ip.isInSubnet(QHostAddress::parseSubnet("172.16.0.0/12"))
  // Ref:
  // https://cpp.hotexamples.com/examples/-/QHostAddress/isInSubnet/cpp-qhostaddress-isinsubnet-method-examples.html

  /**
   * @brief Get the IP Address Number object
   *
   * @param address_number
   * @return ACT_STATUS
   */
  ACT_STATUS GetIpAddressNumber(quint32 &address_number) { return AddressStrToNumber(ip_address_, address_number); }

  /**
   * @brief Assign IP Address & Subnet Mask & Gateway
   *
   * @param address_str
   * @return ACT_STATUS
   */
  ACT_STATUS AutoAssignIPSubnetMask(const QString &address_str) {
    ACT_STATUS_INIT();

    this->ip_address_ = address_str;
    this->subnet_mask_ = "255.255.255.0";

    // Generate the gateway setting x.x.x.254 with the same subnet as the IP address
    // Ex: IP is 192.168.127.10, then the gateway is 192.168.127.254
    QString gateway_ip = address_str;
    gateway_ip.replace(gateway_ip.lastIndexOf('.'), 3, ".254");
    this->gateway_ = gateway_ip;

    return act_status;
  }

  /**
   * @brief Transfer AddressStr to AddressNumber
   *
   * @param address_str
   * @param address_number
   * @return ACT_STATUS
   */
  static ACT_STATUS AddressStrToNumber(const QString &address_str, quint32 &address_number) {
    ACT_STATUS_INIT();
    // 127.0.0.1 -> 2130706433 (i.e. 0x7f000001)
    // 0.0.0.0 ~ 255.255.255.255 -> 0 ~ 4294967295

    QHostAddress q_host_addr(address_str);
    if (q_host_addr.isNull()) {  // invalid
      qCritical() << __func__ << "The IPv4 (" << address_str << ") is invalid.";
      return std::make_shared<ActStatusBase>(ActStatusType::kFailed, ActSeverity::kCritical);
    }
    address_number = q_host_addr.toIPv4Address();  // str to number
    return act_status;
  }

  /**
   * @brief Transfer AddressNumber to AddressStr
   *
   * @param address_str
   * @param address_number
   * @return ACT_STATUS
   */
  static ACT_STATUS AddressNumberToStr(const quint32 &address_number, QString &address_str) {
    ACT_STATUS_INIT();
    // 2130706433 -> 127.0.0.1
    // 0 ~ 4294967295 -> 0.0.0.0 ~ 255.255.255.255
    // Because using quint32 so address_number is always valid.
    address_str = QHostAddress(address_number).toString();  // number to str
    return act_status;
  }
};

/**
 * @brief The DeviceProfile class
 *
 */
class ActProcessingDelay : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, independent_delay, IndependentDelay);         ///< Independent Delay of each interface in ns
  ACT_JSON_FIELD(quint16, dependent_delay_ratio, DependentDelayRatio);  ///< Dependent Ratio of each interface in ns/bit

 public:
  ActProcessingDelay() {}

  ActProcessingDelay(const quint16 &independent_delay, const quint16 &dependent_delay_ratio)
      : independent_delay_(independent_delay), dependent_delay_ratio_(dependent_delay_ratio) {}
};

/**
 * @brief The FeatureAction Capability class
 *
 */
class ActFeatureActionCapability : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, action, Action);
  ACT_JSON_OBJECT(ActActionMethod, method, Method);

  // ACT_JSON_FIELD(bool, active, Active);

 public:
  ActFeatureActionCapability() {}

  ActFeatureActionCapability(const QString &action) { this->SetAction(action); }

  ActFeatureActionCapability(const QString &action, const ActActionMethod &method) {
    this->SetAction(action);
    this->SetMethod(method);
  }
  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActFeatureActionCapability &x) {
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
  friend bool operator==(const ActFeatureActionCapability &x, const ActFeatureActionCapability &y) {
    return x.action_ == y.action_;
  }
};

/**
 * @brief The Feature Capability class
 *
 */
class ActFeatureCapability : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActFeatureEnum, feature, Feature);
  ACT_JSON_FIELD(bool, active, Active);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActFeatureActionCapability, actions, Actions);

 public:
  ActFeatureCapability() {
    this->active_ = false;
    this->feature_ = ActFeatureEnum::kBase;
  }

  ActFeatureCapability(const ActFeatureEnum &feature) : ActFeatureCapability() { this->SetFeature(feature); }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActFeatureCapability &x) {
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
  friend bool operator==(const ActFeatureCapability &x, const ActFeatureCapability &y) {
    return x.feature_ == y.feature_;
  }
};

/**
 * @brief The device type enum class
 *
 */
enum class ActDeviceClusterEnum { kDefault, kNZ2MHG };

/**
 * @brief The QMap for device type enum mapping
 *
 */
static const QMap<QString, ActDeviceClusterEnum> kActDeviceClusterEnumMap = {
    {"Default", ActDeviceClusterEnum::kDefault}, {"NZ2MHG", ActDeviceClusterEnum::kNZ2MHG}};

/**
 * @brief The ACT Device property class
 *
 */
class ActDeviceProperty : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Identify
  ACT_JSON_FIELD(QString, l2_family, L2Family);
  ACT_JSON_FIELD(QString, l3_series, L3Series);
  ACT_JSON_FIELD(QString, l4_series, L4Series);
  ACT_JSON_COLLECTION_ENUM(QList, ActMountTypeEnum, mount_type, MountType);

  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(QString, physical_model_name, PhysicalModelName);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_OBJECT(ActOperatingTemperatureC, operating_temperature_c, OperatingTemperatureC);
  ACT_JSON_ENUM(ActDeviceClusterEnum, device_cluster, DeviceCluster);

  // Others Infos
  ACT_JSON_FIELD(bool, certificate, Certificate);
  ACT_JSON_FIELD(QString, vendor, Vendor);

  // Algorithm
  ACT_JSON_FIELD(quint64, gcl_offset_min_duration, GCLOffsetMinDuration);
  ACT_JSON_FIELD(quint64, gcl_offset_max_duration, GCLOffsetMaxDuration);
  ACT_JSON_FIELD(quint16, max_vlan_cfg_size, MaxVlanCfgSize);
  ACT_JSON_FIELD(quint16, per_queue_size, PerQueueSize);
  ACT_JSON_FIELD(quint16, ptp_queue_id, PtpQueueId);

  // FeatureGroup
  ACT_JSON_OBJECT(ActFeatureGroup, feature_group, FeatureGroup);

  // Deploy
  ACT_JSON_QT_SET(qint32, reserved_vlan, ReservedVlan);

  ACT_JSON_FIELD(quint16, gate_control_list_length,
                 GateControlListLength);  ///< GateControlListLength item (MaxScheduleEvents)
  ACT_JSON_FIELD(quint16, number_of_queue, NumberOfQueue);
  ACT_JSON_FIELD(quint16, tick_granularity, TickGranularity);
  ACT_JSON_FIELD(quint16, stream_priority_config_ingress_index_max, StreamPriorityConfigIngressIndexMax);
  ACT_JSON_QT_DICT_OBJECTS(QMap, quint16, ActProcessingDelay, processing_delay_map, ProcessingDelayMap);

  // Stream capability
  ACT_JSON_OBJECT(ActTrafficSpecification, traffic_specification, TrafficSpecification);  ///< TrafficSpecification item
                                                                                          // (46.2.3.5)
  ACT_JSON_OBJECT(ActIeee802VlanTag, ieee802_vlan_tag, Ieee802VlanTag);

  // 802.1CB capability
  ACT_JSON_OBJECT(ActIeee802Dot1cb, ieee802_1cb, Ieee802Dot1cb);

 public:
  ActDeviceProperty() {
    l2_family_ = "";
    l3_series_ = "";
    l4_series_ = "";
    description_ = "";
    device_cluster_ = ActDeviceClusterEnum::kDefault;
    vendor_ = "";
    gcl_offset_min_duration_ = 0;
    gcl_offset_max_duration_ = 0;
    max_vlan_cfg_size_ = 0;
    per_queue_size_ = 0;
    ptp_queue_id_ = 0;
    gate_control_list_length_ = 0;
    number_of_queue_ = 0;
    tick_granularity_ = 0;
    stream_priority_config_ingress_index_max_ = 0;
    reserved_vlan_.insert(1);
  };
};

/**
 * @brief The ACT Device info class
 *
 */
class ActDeviceInfo : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, serial_number, SerialNumber);
  ACT_JSON_FIELD(QString, location, Location);
  ACT_JSON_FIELD(QString, product_revision, ProductRevision);
  ACT_JSON_FIELD(QString, system_uptime, SystemUptime);
  ACT_JSON_FIELD(QString, redundant_protocol, RedundantProtocol);

 public:
  ActDeviceInfo() {}

  ActDeviceInfo(const QString &location) : location_(location) {}
};

/**
 * @brief The ACT Device status class
 *
 */
class ActDeviceStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, icmp_status, ICMPStatus);
  ACT_JSON_FIELD(bool, restful_status, RESTfulStatus);
  ACT_JSON_FIELD(bool, snmp_status, SNMPStatus);
  ACT_JSON_FIELD(bool, netconf_status, NETCONFStatus);
  ACT_JSON_FIELD(bool, new_moxa_command_status, NewMOXACommandStatus);

 public:
  ActDeviceStatus() {
    this->icmp_status_ = false;
    this->restful_status_ = false;
    this->snmp_status_ = false;
    this->netconf_status_ = false;
    this->new_moxa_command_status_ = false;
  }

  void SetAllConnectStatus(const bool &status) {
    this->restful_status_ = status;
    this->snmp_status_ = status;
    this->netconf_status_ = status;
    this->new_moxa_command_status_ = status;
  }
};

/**
 * @brief The ACT Source Device class(Management EndPoint)
 *
 */
class ActSourceDevice : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);
  ACT_JSON_FIELD(qint64, interface_id, InterfaceId);
  ACT_JSON_FIELD(quint8, recv_count, RecvCount);

 public:
  /**
   * @brief Construct a new End Station Interface object
   *
   */
  ActSourceDevice() {
    device_id_ = -1;
    interface_id_ = -1;
    recv_count_ = 0;
    device_ip_ = "";
  }

  /**
   * @brief Construct a new End Station Interface object
   *
   * @param device_id
   * @param interface_id
   */
  ActSourceDevice(const qint64 &device_id, const qint64 &interface_id) : ActSourceDevice() {
    device_id_ = device_id;
    interface_id_ = interface_id;
  }
};

/**
 * @brief The ACT Device class
 *
 */
class ActDevice : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, device_name, DeviceName);
  ACT_JSON_FIELD(QString, device_alias, DeviceAlias);  ///< Device alias, only for UI display on ACT
  ACT_JSON_FIELD(QString, firmware_version, FirmwareVersion);
  ACT_JSON_FIELD(qint64, device_profile_id, DeviceProfileId);
  ACT_JSON_FIELD(qint64, firmware_feature_profile_id, FirmwareFeatureProfileId);
  ACT_JSON_ENUM(ActDeviceTypeEnum, device_type, DeviceType);

  ACT_JSON_OBJECT(ActIpv4, ipv4, Ipv4);
  ACT_JSON_FIELD(QString, mac_address, MacAddress);

  ACT_JSON_OBJECT(ActDeviceInfo, device_info, DeviceInfo);
  ACT_JSON_OBJECT(ActDeviceModularConfiguration, modular_configuration, ModularConfiguration);

  // For BroadCast Search
  ACT_JSON_FIELD(quint32, distance, Distance);

  // Connection status
  ACT_JSON_OBJECT(ActDeviceStatus, device_status, DeviceStatus);

  // For UI
  ACT_JSON_FIELD(qint16, tier, Tier);
  ACT_JSON_OBJECT(ActCoordinate, coordinate, Coordinate);
  ACT_JSON_FIELD(qint64, group_id, GroupId);  ///< The group id of the device in the topology architecture

  // From Device Profile
  ACT_JSON_OBJECT(ActDeviceProperty, device_property, DeviceProperty);

  ACT_JSON_COLLECTION_OBJECTS(QList, ActInterface, interfaces, Interfaces);

  // For deploy
  ACT_JSON_OBJECT(ActDeviceAccount, account, Account);
  ACT_JSON_OBJECT(ActNetconfConfiguration, netconf_configuration, NetconfConfiguration);
  ACT_JSON_OBJECT(ActSnmpConfiguration, snmp_configuration, SnmpConfiguration);
  ACT_JSON_OBJECT(ActRestfulConfiguration, restful_configuration, RestfulConfiguration);
  ACT_JSON_FIELD(qint32, ssh_port, SSHPort);

  ACT_JSON_FIELD(bool, enable_snmp_setting, EnableSnmpSetting);

  ACT_JSON_ENUM(ActDeviceRoleEnum, device_role,
                DeviceRole);  // for redundant-view (rstp: Root, turbo ring: Ring Master)

 public:
  QList<QString> key_order_;
  qint64 mac_address_int;
  ActLLDPData lldp_data_;                         //  for auto_scan
  QMap<qint64, QString> single_entry_mac_table_;  //  for auto_scan <PortID, MAC>
  ActDeviceModularInfo modular_info_;             //  for manufacture mapping report

  // QMap<qint64, QString> remote_port_mac_;         //  for find source device <PortID, RemoteDeviceMAC>
  bool auto_probe_;

  /**
   * @brief Construct a new ActDevice object
   *
   */
  ActDevice() {
    this->id_ = -1;
    this->device_type_ = ActDeviceTypeEnum::kUnknown;
    this->device_role_ = ActDeviceRoleEnum::kUnknown;
    this->firmware_version_ = "";
    this->tier_ = 0;
    this->group_id_ = -1;
    this->distance_ = 0;
    this->enable_snmp_setting_ = true;
    this->device_profile_id_ = -1;
    this->firmware_feature_profile_id_ = -1;
    this->mac_address_int = 0;
    this->ssh_port_ = ACT_DEFAULT_SSH_PORT;
    this->key_order_.append(QList<QString>({QString("Id"),
                                            QString("DeviceName"),
                                            QString("DeviceAlias"),
                                            QString("FirmwareVersion"),
                                            QString("DeviceProfileId"),
                                            QString("FirmwareFeatureProfileId"),
                                            QString("DeviceType"),
                                            QString("Ipv4"),
                                            QString("MacAddress"),
                                            QString("Distance"),
                                            QString("DeviceInfo"),
                                            QString("ModularConfiguration"),
                                            QString("DeviceStatus"),
                                            QString("Tier"),
                                            QString("Coordinate"),
                                            QString("GroupId"),
                                            QString("DeviceProperty"),
                                            QString("Interfaces"),
                                            QString("Account"),
                                            QString("NetconfConfiguration"),
                                            QString("SnmpConfiguration"),
                                            QString("RestfulConfiguration"),
                                            QString("EnableSnmpSetting"),
                                            QString("DeviceRole")}));
  }

  /**
   * @brief Construct a new Act Device object
   *
   * @param id
   */
  ActDevice(const qint64 &id) : ActDevice() { this->id_ = id; }

  /**
   * @brief Construct a new ActDevice object
   *
   * @param ip_address The device ip_address
   */
  ActDevice(const QString &ip_address) : ActDevice() {
    // set ipv4
    this->ipv4_.SetIpAddress(ip_address);

    // assign id use IpAddressNumber
    quint32 address_num;
    this->ipv4_.GetIpAddressNumber(address_num);
    this->id_ = address_num;
  }

  /**
   * @brief Construct a new ActDevice object
   *
   * @param ip_address The device IP_address
   * @param model_name The device Model Name
   */
  ActDevice(const QString &ip_address, const QString &model_name) : ActDevice(ip_address) {
    this->device_property_.SetModelName(model_name);
  }

  ActDevice(const ActPingDevice &ping_device) : ActDevice() {
    // set ipv4
    this->ipv4_.SetIpAddress(ping_device.GetIpAddress());

    // assign id use IpAddressNumber
    quint32 address_num;
    this->ipv4_.GetIpAddressNumber(address_num);

    this->account_ = ping_device.GetAccount();
    this->netconf_configuration_ = ping_device.GetNetconfConfiguration();
    this->snmp_configuration_ = ping_device.GetSnmpConfiguration();
    this->restful_configuration_ = ping_device.GetRestfulConfiguration();
    this->enable_snmp_setting_ = ping_device.GetEnableSnmpSetting();
  }

  /**
   * @brief Check device can be deploy by device type
   *
   * @param dev
   * @return true
   * @return false
   */
  static bool CheckDeviceCanBeDeploy(const ActDevice &dev) {
    if (dev.GetDeviceType() == ActDeviceTypeEnum::kTSNSwitch || dev.GetDeviceType() == ActDeviceTypeEnum::kSwitch ||
        dev.GetDeviceType() == ActDeviceTypeEnum::kBridgedEndStation ||
        dev.GetDeviceType() == ActDeviceTypeEnum::kMoxa) {
      return true;
    }

    return false;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActDevice &x) {
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
  friend bool operator==(const ActDevice &x, const ActDevice &y) {
    return (x.id_ == y.id_) || (x.ipv4_.GetIpAddress() == y.ipv4_.GetIpAddress());
  }

  /**
   * @brief The comparison operator for std:set & std:sort
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator<(const ActDevice &x, const ActDevice &y) { return x.distance_ > y.distance_ || x.id_ < y.id_; }

  /**
   * @brief [feat:1662] Remove the password related fields in the device
   *
   * @return ACT_STATUS
   */
  ACT_STATUS HidePassword() {
    ACT_STATUS_INIT();

    this->account_.HidePassword();
    this->snmp_configuration_.HidePassword();
    return act_status;
  }

  /**
   * @brief Encrypt the password field
   *
   * @return QString
   */
  ACT_STATUS EncryptPassword() {
    ACT_STATUS_INIT();

    this->account_.EncryptPassword();
    this->snmp_configuration_.EncryptPassword();

    return act_status;
  }

  /**
   * @brief Decrypt the encrypted password
   *
   * @return QString
   */
  ACT_STATUS DecryptPassword() {
    ACT_STATUS_INIT();

    // Decrypt password in the project setting
    this->account_.DecryptPassword();
    this->snmp_configuration_.DecryptPassword();

    return act_status;
  }

  /**
   * @brief Disable all features at DeviceProperty
   *
   * @return ACT_STATUS
   */
  ACT_STATUS DisableAllFeatures() {
    ACT_STATUS_INIT();

    ActFeatureGroup new_feat_group;  // default all feature are false
    this->GetDeviceProperty().SetFeatureGroup(new_feat_group);
    return act_status;
  }

  /**
   * @brief Get the Interfaces Id Name Map object
   *
   * @param interfaces_name_map
   * @return ACT_STATUS
   */
  ACT_STATUS GetInterfacesIdNameMap(QMap<qint64, QString> &interfaces_name_map) const {
    ACT_STATUS_INIT();

    interfaces_name_map.clear();
    for (int i = 0; i < interfaces_.size(); i++) {
      qint64 inteface_id = interfaces_[i].GetInterfaceId();
      QString inteface_name = interfaces_[i].GetInterfaceName();
      interfaces_name_map.insert(inteface_id, inteface_name);
    }

    return act_status;
  }

  bool IsUnknownInterfacesDeviceType() {
    if (this->device_type_ == ActDeviceTypeEnum::kICMP || this->device_type_ == ActDeviceTypeEnum::kUnknown ||
        this->device_type_ == ActDeviceTypeEnum::kMoxa) {
      return true;
    }
    return false;
  }

  bool CheckCanDeploy() {
    if (this->GetDeviceType() != ActDeviceTypeEnum::kSwitch && this->GetDeviceType() != ActDeviceTypeEnum::kTSNSwitch &&
        this->GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      return false;
    }

    // [bugfix:4096] Design mode with the default switch would let download failed
    if (this->GetDeviceProfileId() == ACT_SWITCH_PROFILE_ID ||
        this->GetDeviceProfileId() == ACT_BRIDGE_END_STATION_PROFILE_ID) {
      return false;
    }

    return (this->GetDeviceProperty().GetFeatureGroup().GetOperation().GetImportExport() &&
            this->GetDeviceProperty().GetFeatureGroup().GetConfiguration().CheckSupportAnyOne());
  }
};
/**
 * @brief The class represent the device name should be updated
 *
 */
class ActBatchDeviceName : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, change, Change);  ///< represent the item should be updated
  ACT_JSON_FIELD(QString, device_name, DeviceName);
};

/**
 * @brief The class represent the SNMP configuration should be updated
 *
 */
class ActBatchSnmpConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, change, Change);  ///< represent the item should be updated
  ACT_JSON_OBJECT(ActSnmpConfiguration, snmp_configuration, SnmpConfiguration);
};

/**
 * @brief The class represent the NETCONF configuration should be updated
 *
 */
class ActBatchNetconfConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, change, Change);  ///< represent the item should be updated
  ACT_JSON_OBJECT(ActNetconfConfiguration, netconf_configuration, NetconfConfiguration);
};

/**
 * @brief The class represent the RESTful configuration should be updated
 *
 */
class ActBatchRestfulConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, change, Change);  ///< represent the item should be updated
  ACT_JSON_OBJECT(ActRestfulConfiguration, restful_configuration, RestfulConfiguration);
};

/**
 * @brief The ACT Device class
 *
 */
class ActBatchDevice : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET(qint64, device_ids, DeviceIds);
  ACT_JSON_FIELD(qint64, device_profile_id, DeviceProfileId);
  ACT_JSON_OBJECT(ActBatchDeviceName, device_name, DeviceName);
  ACT_JSON_OBJECT(ActBatchSnmpConfiguration, snmp_configuration, SnmpConfiguration);
  ACT_JSON_OBJECT(ActBatchNetconfConfiguration, netconf_configuration, NetconfConfiguration);
  ACT_JSON_OBJECT(ActBatchRestfulConfiguration, restful_configuration, RestfulConfiguration);
};

class ActDeviceDistanceEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(qint64, mac_address_int, MacAddressInt);
  ACT_JSON_FIELD(quint32, distance, Distance);

 public:
  /**
   * @brief Construct a new Act Device Distance Entry object
   *
   */
  ActDeviceDistanceEntry() {
    id_ = -1;
    distance_ = 0;
    mac_address_int_ = 0;
  }

  ActDeviceDistanceEntry(const qint64 &id, const quint32 &distance, const qint64 &mac_address_int)
      : id_(id), distance_(distance), mac_address_int_(mac_address_int) {}

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActDeviceDistanceEntry &x) {
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
  friend bool operator==(const ActDeviceDistanceEntry &x, const ActDeviceDistanceEntry &y) { return x.id_ == y.id_; }

  // /**
  //  * @brief The comparison operator for std:set & std:sort
  //  *
  //  * @param x
  //  * @param y
  //  * @return true
  //  * @return false
  //  */
  friend bool operator<(const ActDeviceDistanceEntry &x, const ActDeviceDistanceEntry &y) {
    return y.distance_ < x.distance_ || (x.distance_ == y.distance_ && x.mac_address_int_ < y.mac_address_int_);
  }

  // /**
  //  * @brief The sort function for std:sort use
  //  *
  //  * @param x
  //  * @param y
  //  * @return true
  //  * @return false
  //  */
  // static bool SortByDistance(const ActDeviceDistanceEntry &x, const ActDeviceDistanceEntry &y) {
  //   return x.distance_ < y.distance_;
  // }
};

/**
 * @brief The Status is for scan/search device finished result
 *
 * Cause of the include issue, act_status.hpp cannot include act_device.hpp
 *
 */
class ActScanFinishedStatus : public ActProgressStatus {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActDevice, devices, Devices);

 public:
  /**
   * @brief Construct a new Act JSON Parse Error object
   *
   */
  ActScanFinishedStatus() {
    this->SetStatus(ActStatusType::kFinished);
    this->SetSeverity(ActSeverity::kDebug);
    this->SetProgress(100);
  }

  ActScanFinishedStatus(const ActProgressStatus &progress_status, const QSet<ActDevice> &devices) {
    this->SetStatus(progress_status.GetStatus());
    this->SetSeverity(progress_status.GetSeverity());
    this->SetProgress(progress_status.GetProgress());
    this->SetDevices(devices);
  }
};

class ActDeviceDiscoveryFinishedStatus : public ActProgressStatus {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDevice, devices, Devices);

 public:
  /**
   * @brief Construct a new Act JSON Parse Error object
   *
   */
  ActDeviceDiscoveryFinishedStatus() {
    this->SetStatus(ActStatusType::kFinished);
    this->SetSeverity(ActSeverity::kDebug);
    this->SetProgress(100);
  }
};

class ActLinkSequenceDetectFinishedStatus : public ActProgressStatus {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceDistanceEntry, sorted_result, SortedResult);

 public:
  /**
   * @brief Construct a new Act JSON Parse Error object
   *
   */
  ActLinkSequenceDetectFinishedStatus() {
    this->SetStatus(ActStatusType::kFinished);
    this->SetSeverity(ActSeverity::kDebug);
    this->SetProgress(100);
  }
};

/**
 * @brief [feat:2648] The class of moved device
 *
 */
class ActDeviceCoordinate : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_OBJECT(ActCoordinate, coordinate, Coordinate);

 public:
  ActDeviceCoordinate() { this->id_ = -1; }

  ActDeviceCoordinate(const ActDevice &device) {
    this->id_ = device.GetId();
    this->coordinate_ = device.GetCoordinate();
  }

  ActDeviceCoordinate(const qint64 &x, const qint64 &y) : ActDeviceCoordinate() {
    this->coordinate_.SetX(x);
    this->coordinate_.SetY(y);
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActDeviceCoordinate &x) {
    // no effect on the program's behavior
    // static_cast<void>(x);
    return qHash(x.id_, 0x0);
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActDeviceCoordinate &x, const ActDeviceCoordinate &y) { return (x.id_ == y.id_); }
};

/**
 * @brief [feat:2648] The class of moved device
 *
 */
class ActUpdateDeviceCoordinates : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActDeviceCoordinate, devices, Devices);
  ACT_JSON_FIELD(bool, notify_ui_update, NotifyUIUpdate);

 public:
  /**
   * @brief Construct a new Act Update Device Coordinates object
   *
   */
  ActUpdateDeviceCoordinates() { this->notify_ui_update_ = false; }
};

/**
 * @brief The ACT Device class
 *
 */
class ActSimpleDevice : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, device_name, DeviceName);
  ACT_JSON_FIELD(QString, device_alias, DeviceAlias);  ///< Device alias, only for UI display on ACT
  ACT_JSON_FIELD(QString, firmware_version, FirmwareVersion);
  ACT_JSON_FIELD(qint64, device_profile_id, DeviceProfileId);
  ACT_JSON_FIELD(qint64, firmware_feature_profile_id, FirmwareFeatureProfileId);
  ACT_JSON_ENUM(ActDeviceTypeEnum, device_type, DeviceType);

  ACT_JSON_OBJECT(ActIpv4, ipv4, Ipv4);
  ACT_JSON_FIELD(QString, mac_address, MacAddress);

  ACT_JSON_OBJECT(ActDeviceInfo, device_info, DeviceInfo);
  ACT_JSON_OBJECT(ActDeviceModularConfiguration, modular_configuration, ModularConfiguration);

  ACT_JSON_FIELD(qint16, tier, Tier);  ///< The tier of the device in the topology architecture
  ACT_JSON_OBJECT(ActCoordinate, coordinate, Coordinate);
  ACT_JSON_FIELD(qint64, group_id, GroupId);  ///< The group id of the device in the topology architecture

  ACT_JSON_COLLECTION_OBJECTS(QList, ActInterface, interfaces, Interfaces);

 public:
  /**
   * @brief Construct a new Act User object
   *
   * @param id
   */
  ActSimpleDevice() { this->id_ = -1; }

  ActSimpleDevice(const qint64 &id) : ActSimpleDevice() { this->id_ = id; }

  ActSimpleDevice(const ActDevice &device) {
    this->id_ = device.GetId();
    this->device_name_ = device.GetDeviceName();
    this->device_alias_ = device.GetDeviceAlias();
    this->firmware_version_ = device.GetFirmwareVersion();
    this->device_profile_id_ = device.GetDeviceProfileId();
    this->firmware_feature_profile_id_ = device.GetFirmwareFeatureProfileId();
    this->device_type_ = device.GetDeviceType();
    this->ipv4_ = device.GetIpv4();
    this->mac_address_ = device.GetMacAddress();
    this->device_info_ = device.GetDeviceInfo();
    this->modular_configuration_ = device.GetModularConfiguration();
    this->tier_ = device.GetTier();
    this->coordinate_ = device.GetCoordinate();
    this->group_id_ = device.GetGroupId();
    this->interfaces_ = device.GetInterfaces();
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActSimpleDevice &obj) {
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
  friend bool operator==(const ActSimpleDevice &x, const ActSimpleDevice &y) { return x.id_ == y.id_; }
};

class ActSimpleDeviceSet : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActSimpleDevice, simple_devices, Devices);
};

/**
 * @brief [feat:2651] A map class to store device id (key) and patch content (value)
 *
 */
class ActPatchDeviceMap : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT(QMap, qint64, QString, patch_device_map, PatchDeviceMap);
};

/**
 * @brief request of create device API
 *
 */
class ActCreateDeviceRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDevice, device, Device);
  ACT_JSON_FIELD(bool, from_bag, FromBag);

 public:
  ActCreateDeviceRequest() { this->from_bag_ = false; }
};

/**
 * @brief request of create devices API
 *
 */
class ActCreateDevicesRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActDevice, device_list, DeviceList);
  ACT_JSON_FIELD(bool, from_bag, FromBag);

 public:
  ActCreateDevicesRequest() { this->from_bag_ = false; }
};

/**
 * @brief request of delete devices API
 *
 */
class ActDeleteDevicesRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint64, dev_ids, DeviceIds);
  ACT_JSON_FIELD(bool, to_bag, ToBag);

 public:
  ActDeleteDevicesRequest() { this->to_bag_ = false; }
};

class ActDeviceOfflineConfigFileMap : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT(QMap, qint64, QString, device_offline_config_file_map, DeviceOfflineConfigFileMap);
};

class ActDeviceBagItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, can_add_to_topology, CanAddToTopology);
  ACT_JSON_FIELD(qint64, remaining_quantity, RemainingQuantity);

 public:
  QList<QString> key_order_;

  ActDeviceBagItem() {
    this->can_add_to_topology_ = false;
    this->remaining_quantity_ = 0;

    this->key_order_.append(QList<QString>({QString("CanAddToTopology"), QString("RemainingQuantity")}));
  }
};

class ActDeviceBag : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActDeviceBagItem, device_bag_map, DeviceBagMap);
};