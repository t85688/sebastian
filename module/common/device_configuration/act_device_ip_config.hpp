/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include "act_json.hpp"
#include "topology/act_device.hpp"

class ActDeviceIpConfiguration : public ActDeviceConnectConfig {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, mac_address, MacAddress);
  ACT_JSON_FIELD(QString, origin_ip, OriginIp);
  ACT_JSON_FIELD(QString, new_ip, NewIp);
  ACT_JSON_FIELD(QString, subnet_mask, SubnetMask);
  ACT_JSON_FIELD(QString, gateway, Gateway);
  ACT_JSON_FIELD(QString, dns1, DNS1);
  ACT_JSON_FIELD(QString, dns2, DNS2);

  ACT_JSON_FIELD(bool, enable_snmp_setting, EnableSnmpSetting);

 public:
  /**
   * @brief Construct a new Act Ipv 4 object
   *
   */
  ActDeviceIpConfiguration() {
    id_ = -1;
    mac_address_ = "";
    origin_ip_ = "";
    new_ip_ = "";
    subnet_mask_ = "";
    gateway_ = "";
    dns1_ = "";
    dns2_ = "";
    enable_snmp_setting_ = true;
  }
};
