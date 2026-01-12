/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_device.hpp"
#include "act_json.hpp"
#include "act_link.hpp"
#include "act_project.hpp"
#include "act_status.hpp"
#include "act_stream.hpp"

/**
 * @brief The ACT Topology class
 *
 */
class ActTopology : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);  ///< The id of the topology
  ACT_JSON_FIELD(QString, data_version, DataVersion);
  ACT_JSON_FIELD(QString, topology_name, TopologyName);  ///< The name of the topology
  ACT_JSON_QT_SET_OBJECTS(ActDevice, devices, Devices);  ///< The device set in the topology
  ACT_JSON_QT_SET_OBJECTS(ActLink, links, Links);        ///< The link set in the topology
  ACT_JSON_QT_SET_OBJECTS(ActStream, streams, Streams);  ///< The stream set in the topology

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act Topology object
   *
   */
  ActTopology() {
    this->id_ = -1;
    this->data_version_ = ACT_TOPOLOGY_DATA_VERSION;
    this->key_order_.append(QList<QString>({QString("Id"), QString("DataVersion"), QString("TopologyName"),
                                            QString("Devices"), QString("Links"), QString("Streams")}));
  }

  /**
   * @brief Construct a new Act Topology object
   *
   * @param id
   */
  ActTopology(const qint64 &id) : id_(id) {}

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActTopology &obj) {
    return qHash(obj.id_, 0);  // arbitrary value is 0
  }

  /**
   * @brief The equal operator
   *
   * @param source The copied source
   */
  friend bool operator==(const ActTopology &x, const ActTopology &y) { return x.id_ == y.id_; }

  /**
   * @brief Hide the password field
   *
   * @return ACT_STATUS
   */
  ACT_STATUS HidePassword() {
    ACT_STATUS_INIT();

    // Hide password in each device
    QSet<ActDevice> device_set;
    for (auto dev : this->GetDevices()) {
      dev.HidePassword();
      device_set.insert(dev);
    }
    this->SetDevices(device_set);

    return act_status;
  }

  /**
   * @brief Encrypt the password field
   *
   * @return QString
   */
  ACT_STATUS EncryptPassword() {
    ACT_STATUS_INIT();

    // Decrypt password in each device
    QSet<ActDevice> device_set;
    for (auto dev : this->GetDevices()) {
      dev.EncryptPassword();
      device_set.insert(dev);
    }
    this->SetDevices(device_set);

    return act_status;
  }

  /**
   * @brief Decrypt the encrypted password
   *
   * @return QString
   */
  ACT_STATUS DecryptPassword() {
    ACT_STATUS_INIT();

    // Decrypt password in each device
    QSet<ActDevice> device_set;
    for (auto dev : this->GetDevices()) {
      dev.DecryptPassword();
      device_set.insert(dev);
    }
    this->SetDevices(device_set);

    return act_status;
  }
};

class ActSimpleTopology : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(qint64, id, Id);                        ///< The id of the topology
  ACT_JSON_FIELD(QString, topology_name, TopologyName);  ///< The name of the topology

 public:
  /**
   * @brief Construct a new Act Simple Topology object
   *
   */
  ActSimpleTopology() {}

  /**
   * @brief Construct a new Act Simple Topology object
   *
   * @param project
   */
  ActSimpleTopology(ActTopology &topology) {
    this->id_ = topology.GetId();
    this->topology_name_ = topology.GetTopologyName();
  }

  /**
   * @brief Construct a new Act Simple Topology object
   *
   * @param id
   */
  ActSimpleTopology(const qint64 &id) : id_(id) {}

  /**
   * @brief The equal operator
   *
   * @param source The copied source
   */
  friend bool operator==(const ActSimpleTopology &x, const ActSimpleTopology &y) { return x.id_ == y.id_; }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActSimpleTopology &obj) {
    return qHash(obj.id_, 0);  // arbitrary value is 0
  }
};

class ActSimpleTopologySet : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActSimpleTopology, simple_topology_set, SimpleTopologySet);
};

class ActTopologyCreateParam : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, topology_name, TopologyName);  ///< The name of the topology
  ACT_JSON_FIELD(qint64, project_id, ProjectId);         ///< The ID of the project
  ACT_JSON_QT_SET(qint64, devices, Devices);             ///< The device set in the project
  ACT_JSON_QT_SET(qint64, links, Links);                 ///< The link set in the project
  ACT_JSON_QT_SET(qint64, streams, Streams);             ///< The stream set in the project
  ACT_JSON_FIELD(QString, image, Image);                 ///< The image of the topology in base64 format
};

/**
 * @brief [feat:2701] A list class to store patch content of devices/streams/links
 *
 */
class ActPatchContentList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, QString, patch_content_list, PatchContentList);
};

/**
 * @brief The undo/redo status class
 *
 */
class ActTopologyStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, loop, Loop);  ///< The status of the loop

 public:
  ActTopologyStatus() : loop_(false) {}
};

/**
 * @brief The topology template type class enum class
 *
 */
enum class ActTopologyTemplateTypeEnum { kLine, kRing, kStar, kNone };

/**
 * @brief The QMap for Cable type class enum mapping
 *
 */
static const QMap<QString, ActTopologyTemplateTypeEnum> kActTopologyTemplateTypeEnumMap = {
    {"Line", ActTopologyTemplateTypeEnum::kLine},
    {"Ring", ActTopologyTemplateTypeEnum::kRing},
    {"Star", ActTopologyTemplateTypeEnum::kStar},
    {"None", ActTopologyTemplateTypeEnum::kNone}};

/**
 * @brief The topology template class
 *
 */
class ActTopologyTemplate : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActTopologyTemplateTypeEnum, type, Type);      ///< The template type enum
  ACT_JSON_OBJECT(ActCoordinate, coordinate, Coordinate);      ///< The coordinate of the topology template
  ACT_JSON_FIELD(qint64, device_profile_id, DeviceProfileId);  ///< The device profile id
  ACT_JSON_FIELD(qint64, device_count, DeviceCount);           ///< The device count
  ACT_JSON_COLLECTION(QList, QString, ip_list, IPList);        ///< The device IP list

 public:
  ActTopologyTemplate() {
    this->type_ = ActTopologyTemplateTypeEnum::kNone;
    this->device_profile_id_ = 0;
    this->device_count_ = 0;
  }
};