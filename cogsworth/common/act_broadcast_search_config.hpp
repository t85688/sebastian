/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_project.hpp"
#include "topology/act_device.hpp"

/**
 * @brief The ActDeviceDiscoveryConfig class
 *
 */
class ActDeviceDiscoveryConfig : public ActDeviceConnectConfig {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDefineDeviceType, define_device_type,
                  DefineDeviceType);  ///< The DefineDeviceType of the broadcast search module

  ACT_JSON_OBJECT(ActDefineNetworkInterface, define_network_interface,
                  DefineNetworkInterface);  ///< The DefineNetworkInterface of the broadcast search module

  ACT_JSON_FIELD(bool, enable_snmp_setting, EnableSnmpSetting);  ///< The DefaultSetting
 public:
  /**
   * @brief Construct a new Act Device Discovery Config object
   *
   */
  ActDeviceDiscoveryConfig() { enable_snmp_setting_ = true; }
};

/**
 * @brief The ActDeviceDiscoveryConfig class
 *
 */
class ActRetryConnectConfig : public ActDeviceConnectConfig {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint64, id, Id);                    ///< The device id list
  ACT_JSON_FIELD(bool, enable_snmp_setting, EnableSnmpSetting);  ///< The DefaultSetting

 public:
  /**
   * @brief Construct a new Act Retry Connect Config object
   *
   */
  ActRetryConnectConfig() { enable_snmp_setting_ = true; }
};
