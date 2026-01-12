/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "topology/act_device.hpp"

class ActProbeDeviceProfileConfig : public ActDeviceConnectConfig {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, ip_address, IpAddress);  ///< IpAddress item

 public:
  /**
   * @brief Construct a new Act Ipv 4 object
   *
   */
  ActProbeDeviceProfileConfig() { ip_address_ = ""; }
};
