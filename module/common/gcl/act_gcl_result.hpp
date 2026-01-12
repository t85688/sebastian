/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"
#include "gcl/act_gate_control.hpp"

/**
 * @brief The GCL's ActGclResult class
 *
 */
class ActGclResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);   ///< ID of this device
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);  ///< Ip Address of this device
  ACT_JSON_COLLECTION_OBJECTS(QList, ActInterfaceGateControls, interface_gate_controls,
                              InterfaceGateControls);  ///< InterfaceGateControls

 public:
  /**
   * @brief Construct a new Act GCL Result object
   *
   */
  ActGclResult() : device_id_(-1) {}

  /**
   * @brief Construct a new Act Gcl Result object
   *
   * @param device_id
   */
  ActGclResult(const qint64 &device_id) : device_id_(device_id) {}

  /**
   * @brief Construct a new Act Gcl Result object
   *
   * @param device_id
   */
  ActGclResult(const qint64 &device_id, const QString &device_ip) : device_id_(device_id), device_ip_(device_ip) {}

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActGclResult &x) {
    return qHash(x.device_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActGclResult &x, const ActGclResult &y) { return x.device_id_ == y.device_id_; }
};
