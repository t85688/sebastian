/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"
// #include "act_system.hpp"
#include "deploy_entry/act_vlan_static_entry.hpp"

/**
 * @brief The ACT vlan-view device class
 *
 */
class ActVlanViewDevice : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);  ///< The id of the device
  ACT_JSON_QT_SET_OBJECTS(ActVlanPortTypeEntry, ports, Ports);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act vlan-view device object
   *
   */
  ActVlanViewDevice() {
    this->key_order_.append(QList<QString>({QString("DeviceId"), QString("Ports")}));
    this->device_id_ = -1;
  }

  /**
   * @brief Construct a new Act Vlan View Device object
   *
   * @param device_id
   */
  ActVlanViewDevice(const qint64 &device_id) : ActVlanViewDevice() { this->device_id_ = device_id; }

  /**
   * @brief Construct a new Act vlan-view device object
   *
   * @param device_id
   * @param ports
   */
  ActVlanViewDevice(const qint64 &device_id, const QSet<ActVlanPortTypeEntry> &ports) : ActVlanViewDevice() {
    this->device_id_ = device_id;
    this->ports_ = ports;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActVlanViewDevice &obj) {
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
  friend bool operator==(const ActVlanViewDevice &x, const ActVlanViewDevice &y) {
    return x.device_id_ == y.device_id_;
  }
};

/**
 * @brief The ACT vlan-view class
 *
 */
class ActVlanView : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(qint32, vlan_id, VlanId);  ///< The unique id of the vlan id
  ACT_JSON_QT_SET_OBJECTS(ActVlanViewDevice, devices, Devices);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act vlan-view object
   *
   */
  ActVlanView() {
    this->key_order_.append(QList<QString>({QString("VlanId"), QString("Devices")}));
    this->vlan_id_ = -1;
  }

  /**
   * @brief Construct a new Act vlan-view object
   *
   * @param vlan_id
   */
  ActVlanView(const qint32 &vlan_id) : ActVlanView() { this->vlan_id_ = vlan_id; }

  /**
   * @brief Construct a new Act vlan-view object
   *
   * @param vlan_id
   * @param devices
   */
  ActVlanView(const qint32 &vlan_id, const QSet<ActVlanViewDevice> &devices) : ActVlanView() {
    this->vlan_id_ = vlan_id;
    this->devices_ = devices;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActVlanView &obj) {
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
  friend bool operator==(const ActVlanView &x, const ActVlanView &y) { return x.vlan_id_ == y.vlan_id_; }
};

/**
 * @brief The ACT vlan-view device class
 *
 */
class ActVlanViewTable : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActVlanView, vlan_views,
                          VlanViews);  ///< Vlan-view set

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new Act vlan-view device object
   *
   */
  ActVlanViewTable() { this->key_order_.append(QList<QString>({QString("VlanViews")})); }

  /**
   * @brief Construct a new Act vlan-view device object
   *
   * @param device_id
   * @param ports
   */
  ActVlanViewTable(const QSet<ActVlanView> &vlan_views) : ActVlanViewTable() { this->vlan_views_ = vlan_views; }
};

/**
 * @brief The ACT vlan-view device class
 *
 */
class ActVlanViewIds : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint32, vlan_id_list, VlanIdList);  ///< The vlan id list

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new Act vlan-view device object
   *
   */
  ActVlanViewIds() { this->key_order_.append(QList<QString>({QString("VlanIdList")})); }

  /**
   * @brief Construct a new Act vlan-view device object
   *
   * @param device_id
   * @param ports
   */
  ActVlanViewIds(const QList<qint32> &vlan_id_list) : ActVlanViewIds() { this->vlan_id_list_ = vlan_id_list; }
};

enum class ActIntelligentVlanStreamTypeEnum { kUnknown = 0, kTagged = 1, kUntagged = 2 };

static const QMap<QString, ActIntelligentVlanStreamTypeEnum> kActIntelligentVlanStreamTypeEnumMap = {
    {"Unknown", ActIntelligentVlanStreamTypeEnum::kUnknown},
    {"Tagged", ActIntelligentVlanStreamTypeEnum::kTagged},
    {"Untagged", ActIntelligentVlanStreamTypeEnum::kUntagged}};

class ActIntelligentVlan : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, vlan_id, VlanId);                                  ///< The unique id of the vlan id
  ACT_JSON_ENUM(ActIntelligentVlanStreamTypeEnum, stream_type, StreamType);  ///< The stream type
  ACT_JSON_QT_SET(qint64, end_device_ids, EndStationList);                   ///< The vlan id list

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new Act vlan-group object
   *
   */
  ActIntelligentVlan() {
    this->key_order_.append(QList<QString>({QString("VlanId"), QString("StreamType"), QString("EndStationList")}));
    this->vlan_id_ = -1;
  }

  ActIntelligentVlan(quint16 &vlan_id) {
    this->key_order_.append(QList<QString>({QString("VlanId"), QString("StreamType"), QString("EndStationList")}));
    this->vlan_id_ = vlan_id;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActIntelligentVlan &obj) { return qHash(obj.vlan_id_, 0x0); }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActIntelligentVlan &x, const ActIntelligentVlan &y) { return x.vlan_id_ == y.vlan_id_; }
};