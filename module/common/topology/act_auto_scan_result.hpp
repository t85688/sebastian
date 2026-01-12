/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_device.hpp"
#include "act_json.hpp"
#include "act_link.hpp"

class ActScanLinksResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, device_ip, DeviceIp);                       ///< The device ip
  ACT_JSON_QT_SET_OBJECTS(ActDevice, update_devices, UpdateDevices);  ///< The need to update device set
  ACT_JSON_QT_SET_OBJECTS(ActLink, scan_links, ScanLinks);            ///< The scan link set

 public:
  ActScanLinksResult() {}
};

class ActAutoScanResultItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, ip, Ip);
  ACT_JSON_FIELD(QString, mac_address, MacAddress);
  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(QString, firmware_version, FirmwareVersion);

 public:
  QList<QString> key_order_;

  ActAutoScanResultItem() {
    ip_ = "";
    mac_address_ = "";
    model_name_ = "";
    firmware_version_ = "";
  }

  ActAutoScanResultItem(const ActDevice &device) {
    ip_ = device.GetIpv4().GetIpAddress();
    mac_address_ = device.GetMacAddress();
    model_name_ = device.GetDeviceProperty().GetModelName();
    firmware_version_ = device.GetFirmwareVersion();
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActAutoScanResultItem &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
    return 0;
  }
};