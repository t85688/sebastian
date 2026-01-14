/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include <QRandomGenerator>
#include <QVariant>

#include "act_device_connection.hpp"
#include "act_device_module.hpp"
#include "act_json.hpp"
#define ACT_DEFAULT_RSTP_DESIGNATED_ROOT "0/00:00:00:00:00:00"

enum class ActLinkStatusTypeEnum { kUp = 1, kDown = 2 };
static const QMap<QString, ActLinkStatusTypeEnum> kActLinkStatusTypeEnumMap = {{"Up", ActLinkStatusTypeEnum::kUp},
                                                                               {"Down", ActLinkStatusTypeEnum::kDown}};

// RSTP PortRole(ref:UI team)
enum class ActRstpPortRoleEnum {
  kNone = -1,
  kDisabled = 0,
  kAlternate = 1,
  kBackup = 2,
  kRoot = 3,
  kDesignated = 4,
  kMaster = 5
};
static const QMap<QString, ActRstpPortRoleEnum> kActRstpPortRoleEnumMap = {
    {"None", ActRstpPortRoleEnum::kNone},           {"Disabled", ActRstpPortRoleEnum::kDisabled},
    {"Alternate", ActRstpPortRoleEnum::kAlternate}, {"Backup", ActRstpPortRoleEnum::kBackup},
    {"Root", ActRstpPortRoleEnum::kRoot},           {"Designated", ActRstpPortRoleEnum::kDesignated},
    {"Master", ActRstpPortRoleEnum::kMaster}};

// RSTP PortState(ref:UI team)
enum class ActRstpPortStateEnum {
  kUnknown = 0,
  kDisabled = 1,
  kBlocking = 2,
  kListening = 3,
  kLearning = 4,
  kForwarding = 5,
  kBroken = 6
};
static const QMap<QString, ActRstpPortStateEnum> kActRstpPortStateEnumMap = {
    {"Unknown", ActRstpPortStateEnum::kUnknown},   {"Disabled", ActRstpPortStateEnum::kDisabled},
    {"Blocking", ActRstpPortStateEnum::kBlocking}, {"Listening", ActRstpPortStateEnum::kListening},
    {"Learning", ActRstpPortStateEnum::kLearning}, {"Forwarding", ActRstpPortStateEnum::kForwarding},
    {"Broken", ActRstpPortStateEnum::kBroken}};

class ActMonitorSystemUtilization : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qreal, cpu_usage, CPUUsage);
  ACT_JSON_FIELD(qreal, memory_usage, MemoryUsage);

 public:
  /**
   * @brief Construct a new Act Monitor System Utilization object
   *
   */
  ActMonitorSystemUtilization() {
    this->cpu_usage_ = 0;
    this->memory_usage_ = 0;
  }
};

enum class ActMonitorDataTypeEnum {
  kPing = 1,
  kScanBasicStatus = 2,
  kScanTrafficData = 3,
  kScanTimeStatus = 4,
  kAssignScanLinkData = 5,
  kIdentifyDeviceData = 6,
  kManagementEndpoint = 7,
  kScanLinksResult = 8,
  kDeviceConnectionStatusData = 9,
  kDeviceRSTPData = 10,
  kDeviceVLANData = 11,
  kDeviceNetworkSettingData = 12,
  kDeviceLoginPolicyData = 13,
  kDeviceInformationSettingData = 14,
  kDeviceLoopProtectionData = 15,
  kDeviceSnmpTrapSettingData = 16,
  kDeviceSyslogSettingData = 17,
  kDeviceTimeSettingData = 18,
  kDevicePortSettingData = 19,
  kDeviceRSTPSettingData = 20,
  kDeviceModuleConfigAndInterfaces = 21,
  kDevicePortDefaultPCPData = 22,
  kTrap = 23,
  kDeviceUserAccountData = 24,
  kDeviceManagementInterfaceData = 25,
  kDeviceUnicastStaticData = 26,
  kDeviceMulticastStaticData = 27,
  kDeviceTimeAwareShaperData = 28,
  kDeviceStreamPriorityIngressData = 29,
  kDeviceStreamPriorityEgressData = 30,
  kDeviceTimeSyncSettingData = 31,

};

/**
 * @brief The ActMonitorData class to receive variant data
 *
 */
class ActMonitorData : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

 public:
  ActMonitorData() { id_ = QRandomGenerator::system()->generate(); }

  template <typename T>
  void AssignData(const QString &device_ip, const ActMonitorDataTypeEnum &type, T &data) {
    device_ip_ = device_ip;
    type_ = type;
    data_ = QVariant::fromValue(data);
  }

  qint64 GetId() { return id_; }
  QString GetDeviceIp() { return device_ip_; }
  ActMonitorDataTypeEnum GetType() { return type_; }
  QVariant GetData() { return data_; }

 private:
  qint64 id_;
  QString device_ip_;
  ActMonitorDataTypeEnum type_;
  QVariant data_;  // QVariant to store different types of data
};

/**
 * @brief The ACT Monitor Device class
 *
 */
class ActPingDevice : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);

  // Connection status
  ACT_JSON_FIELD(bool, alive, Alive);

  // From ping job
  // Technical issue, I can't include act_job.hpp here
  ACT_JSON_FIELD(QString, ip_address, IpAddress);
  ACT_JSON_OBJECT(ActDeviceAccount, account, Account);
  ACT_JSON_OBJECT(ActSnmpConfiguration, snmp_configuration, SnmpConfiguration);
  ACT_JSON_OBJECT(ActNetconfConfiguration, netconf_configuration, NetconfConfiguration);
  ACT_JSON_OBJECT(ActRestfulConfiguration, restful_configuration, RestfulConfiguration);

  ACT_JSON_FIELD(bool, enable_snmp_setting, EnableSnmpSetting);

 public:
  QList<QString> key_order_;

  ActPingDevice() {
    this->id_ = -1;
    this->key_order_.append(QList<QString>({QString("Id"), QString("Alive"), QString("IpAddress"), QString("Account"),
                                            QString("SnmpConfiguration"), QString("NetconfConfiguration"),
                                            QString("RestfulConfiguration"), QString("EnableSnmpSetting")}));
  }

  void SetStatus(const bool &alive) { this->alive_ = alive; }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActPingDevice &x) { return x.id_; }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActPingDevice &x, const ActPingDevice &y) { return (x.id_ == y.id_); }

  /**
   * @brief The comparison operator for std:set & std:sort
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator<(const ActPingDevice &x, const ActPingDevice &y) { return x.id_ < y.id_; }
};

class ActMonitorPortStatusEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActLinkStatusTypeEnum, link_status, LinkStatus);

 public:
  /**
   * @brief Construct a new Act Monitor Data object
   *
   */
  ActMonitorPortStatusEntry() { this->link_status_ = ActLinkStatusTypeEnum::kUp; }
};

class ActMonitorFiberCheckEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);
  ACT_JSON_FIELD(qint64, interface_id, InterfaceId);
  ACT_JSON_FIELD(QString, interface_name, InterfaceName);

  ACT_JSON_FIELD(bool, exist, Exist);

  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(QString, serial_number, SerialNumber);
  ACT_JSON_FIELD(QString, wavelength, Wavelength);
  ACT_JSON_FIELD(QString, temperature_c, TemperatureC);
  ACT_JSON_FIELD(QString, temperature_f, TemperatureF);
  ACT_JSON_FIELD(QString, voltage, Voltage);
  ACT_JSON_FIELD(QString, tx_power, TxPower);
  ACT_JSON_FIELD(QString, rx_power, RxPower);
  ACT_JSON_FIELD(QString, temperatureLimit_c, TemperatureLimitC);
  ACT_JSON_FIELD(QString, temperatureLimit_f, TemperatureLimitF);

  ACT_JSON_COLLECTION(QList, QString, tx_power_limit, TxPowerLimit);
  ACT_JSON_COLLECTION(QList, QString, rx_power_limit, RxPowerLimit);

 public:
  /**
   * @brief Construct a new Act Monitor Data object
   *
   */
  ActMonitorFiberCheckEntry() {
    this->device_id_ = -1;
    this->device_ip_ = "";
    this->interface_id_ = -1;
    this->interface_name_ = "";

    this->exist_ = false;

    this->model_name_ = "";
    this->serial_number_ = "";
    this->wavelength_ = "";
    this->temperature_c_ = "";
    this->temperature_f_ = "";
    this->voltage_ = "";
    this->tx_power_ = "";
    this->rx_power_ = "";
    this->temperatureLimit_c_ = "";
    this->temperatureLimit_f_ = "";
  }

  ActMonitorFiberCheckEntry(const qint64 &device_id, const qint64 &interface_id) : ActMonitorFiberCheckEntry() {
    this->device_id_ = device_id;
    this->interface_id_ = interface_id;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActMonitorFiberCheckEntry &x) { return x.device_id_; }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActMonitorFiberCheckEntry &x, const ActMonitorFiberCheckEntry &y) {
    return (x.device_id_ == y.device_id_ && x.interface_id_ == y.interface_id_);
  }

  /**
   * @brief The comparison operator for std:set & std:sort
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator<(const ActMonitorFiberCheckEntry &x, const ActMonitorFiberCheckEntry &y) {
    return x.interface_id_ < y.interface_id_;
  }
};

class ActMonitorSFPStatusEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(QString, wavelength, Wavelength);
  ACT_JSON_FIELD(QString, temperature_c, TemperatureC);
  ACT_JSON_FIELD(QString, temperature_f, TemperatureF);
  ACT_JSON_FIELD(QString, voltage, Voltage);
  ACT_JSON_FIELD(QString, tx_power, TxPower);
  ACT_JSON_FIELD(QString, rx_power, RxPower);

 public:
  ActMonitorSFPStatusEntry() {
    this->model_name_ = "";
    this->wavelength_ = "";
    this->temperature_c_ = "";
    this->temperature_f_ = "";
    this->voltage_ = "";
    this->tx_power_ = "";
    this->rx_power_ = "";
  }

  ActMonitorSFPStatusEntry(const ActMonitorFiberCheckEntry &fiber_check_entry) : ActMonitorSFPStatusEntry() {
    this->model_name_ = fiber_check_entry.GetModelName();
    this->wavelength_ = fiber_check_entry.GetWavelength();
    this->temperature_c_ = fiber_check_entry.GetTemperatureC();
    this->temperature_f_ = fiber_check_entry.GetTemperatureF();
    this->voltage_ = fiber_check_entry.GetVoltage();
    this->tx_power_ = fiber_check_entry.GetTxPower();
    this->rx_power_ = fiber_check_entry.GetRxPower();
  }
};

class ActMonitorTrafficStatisticsEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint64, tx_total_octets, TxTotalOctets);
  ACT_JSON_FIELD(quint64, tx_total_packets, TxTotalPackets);
  ACT_JSON_FIELD(qreal, traffic_utilization, TrafficUtilization);

 public:
  /**
   * @brief Construct a new Act Monitor Data object
   *
   */
  ActMonitorTrafficStatisticsEntry() {
    this->tx_total_octets_ = 0;
    this->tx_total_packets_ = 0;
    this->traffic_utilization_ = 0;
  }
};

class ActMonitorTimeSyncStatus : public QSerializer {
  // For IEEE1588_2008 & 802.1AS
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, grandmaster_identity, GrandmasterIdentity);
  ACT_JSON_FIELD(QString, parent_identity, ParentIdentity);
  ACT_JSON_QT_DICT(QMap, qint64, quint8, port_state, PortState);
  ACT_JSON_FIELD(QString, offset_from_master, OffsetFromMaster);
  ACT_JSON_FIELD(quint64, steps_removed, StepsRemoved);

 public:
  ActMonitorTimeSyncStatus() {
    this->grandmaster_identity_ = "";
    this->parent_identity_ = "";
    this->offset_from_master_ = "";
    this->steps_removed_ = 0;
  };
};

class ActMonitorBasicStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);
  ACT_JSON_OBJECT(ActMonitorSystemUtilization, system_utilization, SystemUtilization);
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActMonitorPortStatusEntry, port_status, PortStatus);
  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActMonitorFiberCheckEntry, fiber_check, FiberCheck);

  ACT_JSON_FIELD(QString, mac_address, MacAddress);
  ACT_JSON_FIELD(QString, firmware_version, FirmwareVersion);
  ACT_JSON_FIELD(QString, serial_number, SerialNumber);
  ACT_JSON_FIELD(QString, device_name, DeviceName);
  ACT_JSON_FIELD(QString, location, Location);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_FIELD(QString, contact_information, ContactInformation);
  ACT_JSON_FIELD(QString, system_uptime, SystemUptime);
  ACT_JSON_FIELD(QString, product_revision, ProductRevision);
  ACT_JSON_FIELD(QString, redundant_protocol, RedundantProtocol);
  ACT_JSON_OBJECT(ActDeviceModularInfo, modular_info, ModularInfo);

 public:
  ActMonitorBasicStatus() { this->device_id_ = -1; };
};

class ActMonitorDeviceBasicInfo : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, mac_address, MacAddress);
  ACT_JSON_FIELD(QString, firmware_version, FirmwareVersion);
  ACT_JSON_FIELD(QString, serial_number, SerialNumber);
  ACT_JSON_FIELD(QString, device_name, DeviceName);
  ACT_JSON_FIELD(QString, location, Location);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_FIELD(QString, contact_information, ContactInformation);
  ACT_JSON_FIELD(QString, system_uptime, SystemUptime);

 public:
  ActMonitorDeviceBasicInfo() {
    this->mac_address_ = "";
    this->firmware_version_ = "";
    this->serial_number_ = "";
    this->device_name_ = "";
    this->location_ = "";
    this->description_ = "";
    this->contact_information_ = "";
    this->system_uptime_ = "";
  }

  ActMonitorDeviceBasicInfo(const ActMonitorBasicStatus &basic_status) : ActMonitorDeviceBasicInfo() {
    this->mac_address_ = basic_status.GetMacAddress();
    this->firmware_version_ = basic_status.GetFirmwareVersion();
    this->serial_number_ = basic_status.GetSerialNumber();
    this->device_name_ = basic_status.GetDeviceName();
    this->location_ = basic_status.GetLocation();
    this->description_ = basic_status.GetDescription();
    this->contact_information_ = basic_status.GetContactInformation();
    this->system_uptime_ = basic_status.GetSystemUptime();
  }
};

class ActMonitorDeviceSFPStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActMonitorSFPStatusEntry, sfp_status, SFPStatus);
};

class ActMonitorDevicePortStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActMonitorPortStatusEntry, port_status, PortStatus);
};

class ActMonitorDeviceTrafficStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActMonitorTrafficStatisticsEntry, traffic_map, TrafficMap);
};

class ActDeviceMonitorTrafficEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(qint64, port_id, PortId);

  ACT_JSON_FIELD(quint64, tx_total_octets, TxTotalOctets);
  ACT_JSON_FIELD(quint64, tx_total_packets, TxTotalPackets);
  ACT_JSON_FIELD(qint64, port_speed, PortSpeed);
  ACT_JSON_FIELD(qreal, traffic_utilization, TrafficUtilization);

 public:
  ActDeviceMonitorTrafficEntry() {
    this->tx_total_octets_ = 0;
    this->tx_total_packets_ = 0;
    this->port_speed_ = 0;
    this->traffic_utilization_ = 0;
  };
};

class ActDeviceMonitorTraffic : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);

  ACT_JSON_FIELD(quint64, timestamp, Timestamp);

  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActDeviceMonitorTrafficEntry, traffic_map, TrafficMap);

 public:
  ActDeviceMonitorTraffic() {
    this->timestamp_ = 0;
    this->traffic_map_.clear();
  };

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActDeviceMonitorTraffic &obj) {
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
  friend bool operator==(const ActDeviceMonitorTraffic &x, const ActDeviceMonitorTraffic &y) {
    return x.device_id_ == y.device_id_;
  }
};

class ActMonitorRstpPortStatusEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, edge, Edge);
  ACT_JSON_ENUM(ActRstpPortRoleEnum, port_role, PortRole);
  ACT_JSON_ENUM(ActRstpPortStateEnum, port_state, PortState);
  ACT_JSON_FIELD(qint64, root_path_cost, RootPathCost);
  ACT_JSON_FIELD(qint64, path_cost, PathCost);

  // ACT_JSON_FIELD(bool, oper_bridge_link_type,
  //                OperBridgeLinkType);
  // ACT_JSON_FIELD(bool, bpdu_inconsistency,
  //                BpduInconsistency);
  // ACT_JSON_FIELD(bool, loop_inconsistency,
  //                LoopInconsistency);
  // ACT_JSON_FIELD(bool, root_inconsistency,
  //                RootInconsistency);

 public:
  /**
   * @brief Construct a new Act Monitor Data object
   *
   */
  ActMonitorRstpPortStatusEntry() {
    this->edge_ = false;
    this->port_role_ = ActRstpPortRoleEnum::kDisabled;
    this->port_state_ = ActRstpPortStateEnum::kBlocking;
    this->root_path_cost_ = 0;
    this->path_cost_ = 0;

    // this->bpduInconsistency_ = false;
    // this->loopInconsistency_ = false;
    // this->operBridgeLinkType_ = false;
    // this->rootInconsistency_ = false;
  }
};

class ActMonitorRstpStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);

  ACT_JSON_FIELD(QString, designated_root, DesignatedRoot);
  ACT_JSON_FIELD(qint64, forward_delay, ForwardDelay);
  ACT_JSON_FIELD(qint64, hello_time, HelloTime);
  ACT_JSON_FIELD(qint64, max_age, MaxAge);
  ACT_JSON_FIELD(qint64, root_cost, RootCost);

  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActMonitorRstpPortStatusEntry, port_status, PortStatus);

 public:
  ActMonitorRstpStatus() {
    this->designated_root_ = ACT_DEFAULT_RSTP_DESIGNATED_ROOT;
    this->forward_delay_ = 0;
    this->hello_time_ = 0;
    this->max_age_ = 0;
    this->root_cost_ = 0;
  };
};

class ActLinkMonitorTraffic : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, link_id, LinkId);
  ACT_JSON_FIELD(quint64, speed, Speed);

  ACT_JSON_FIELD(quint64, timestamp, Timestamp);

  ACT_JSON_FIELD(qint64, destination_device_id, DestinationDeviceId);
  ACT_JSON_FIELD(qint64, destination_interface_id, DestinationInterfaceId);
  ACT_JSON_FIELD(qint64, source_device_id, SourceDeviceId);
  ACT_JSON_FIELD(qint64, source_interface_id, SourceInterfaceId);

  ACT_JSON_FIELD(qreal, source_traffic_utilization, SourceTrafficUtilization);
  ACT_JSON_FIELD(qreal, destination_traffic_utilization, DestinationTrafficUtilization);

 public:
  ActLinkMonitorTraffic() {
    this->link_id_ = -1;
    this->speed_ = 0;
    this->timestamp_ = 0;
    this->destination_device_id_ = -1;
    this->destination_interface_id_ = -1;
    this->source_device_id_ = -1;
    this->source_interface_id_ = -1;
    this->source_traffic_utilization_ = 0;
    this->destination_traffic_utilization_ = 0;
  };

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActLinkMonitorTraffic &obj) {
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
  friend bool operator==(const ActLinkMonitorTraffic &x, const ActLinkMonitorTraffic &y) {
    return x.link_id_ == y.link_id_;
  }
};

class ActMonitorTimeStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);

  ACT_JSON_OBJECT(ActMonitorTimeSyncStatus, ieee1588_2008, IEEE1588_2008);
  ACT_JSON_OBJECT(ActMonitorTimeSyncStatus, ieee802dot1as_2011, IEEE802Dot1AS_2011);

 public:
  ActMonitorTimeStatus() {};

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActMonitorTimeStatus &obj) {
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
  friend bool operator==(const ActMonitorTimeStatus &x, const ActMonitorTimeStatus &y) {
    return x.device_id_ == y.device_id_;
  }
};

/**
 * @brief The ACT Monitor Link Status class
 *
 */
class ActMonitorSwiftStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);

  ACT_JSON_FIELD(bool, offline, Offline);
  ACT_JSON_FIELD(bool, online, Online);

 public:
  QList<QString> key_order_;

  ActMonitorSwiftStatus() {
    this->key_order_.append(
        QList<QString>({QString("DeviceId"), QString("DeviceIp"), QString("Offline"), QString("Online")}));
  }

  ActMonitorSwiftStatus(const qint64 &device_id, const QString &device_ip, const bool &offline, const bool &online)
      : ActMonitorSwiftStatus() {
    this->device_id_ = device_id;
    this->device_ip_ = device_ip;
    this->offline_ = offline;
    this->online_ = online;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActMonitorSwiftStatus &obj) {
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
  friend bool operator==(const ActMonitorSwiftStatus &x, const ActMonitorSwiftStatus &y) {
    return x.device_id_ == y.device_id_;
  }
};

class ActMonitorDeviceStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, alive, Alive);

 public:
  ActMonitorDeviceStatus() { alive_ = false; }
};
