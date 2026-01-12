/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "device_modules/act_ethernet_module.hpp"
#include "device_modules/act_power_module.hpp"

class ActSFPInfo : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, interface_id, InterfaceId);
  ACT_JSON_FIELD(QString, interface_name, InterfaceName);

  ACT_JSON_FIELD(bool, exist, Exist);
  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(QString, serial_number, SerialNumber);
  // ACT_JSON_FIELD(QString, wavelength, Wavelength);
  // ACT_JSON_FIELD(QString, temperature_c, TemperatureC);
  // ACT_JSON_FIELD(QString, temperature_f, TemperatureF);
  // ACT_JSON_FIELD(QString, voltage, Voltage);
  // ACT_JSON_FIELD(QString, tx_power, TxPower);
  // ACT_JSON_FIELD(QString, rx_power, RxPower);
  // ACT_JSON_FIELD(QString, temperatureLimit_c, TemperatureLimitC);
  // ACT_JSON_FIELD(QString, temperatureLimit_f, TemperatureLimitF);
  // ACT_JSON_COLLECTION(QList, QString, tx_power_limit, TxPowerLimit);
  // ACT_JSON_COLLECTION(QList, QString, rx_power_limit, RxPowerLimit);

 public:
  /**
   * @brief Construct a new Act SFP Info object
   *
   */
  ActSFPInfo() {
    this->interface_id_ = -1;
    this->interface_name_ = "";
    this->exist_ = false;

    this->model_name_ = "";
    this->serial_number_ = "";
    // this->wavelength_ = "";
    // this->temperature_c_ = "";
    // this->temperature_f_ = "";
    // this->voltage_ = "";
    // this->tx_power_ = "";
    // this->rx_power_ = "";
    // this->temperatureLimit_c_ = "";
    // this->temperatureLimit_f_ = "";
  }

  ActSFPInfo(const qint64 &interface_id) : ActSFPInfo() { this->interface_id_ = interface_id; }
};

class ActDeviceEthernetModule : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, exist, Exist);

  ACT_JSON_FIELD(QString, module_name, ModuleName);
  ACT_JSON_FIELD(QString, serial_number, SerialNumber);
  ACT_JSON_FIELD(QString, product_revision, ProductRevision);
  ACT_JSON_FIELD(QString, status, Status);
  ACT_JSON_FIELD(qint64, module_id, ModuleId);

  // ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActSFPInfo, sfp, SFP);  // <PortID, SFP>

 public:
  ActDeviceEthernetModule() {
    this->exist_ = false;
    this->module_name_ = "";
    this->serial_number_ = "";
    this->product_revision_ = "";
    this->status_ = "";
    this->module_id_ = -1;
  }
};

class ActDeviceSimpleEthernetModule : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, module_name, ModuleName);
  ACT_JSON_FIELD(QString, serial_number, SerialNumber);

 public:
  ActDeviceSimpleEthernetModule() {
    this->module_name_ = "";
    this->serial_number_ = "";
  }

  ActDeviceSimpleEthernetModule(const ActDeviceEthernetModule &ethernet_module) : ActDeviceSimpleEthernetModule() {
    module_name_ = ethernet_module.GetModuleName();
    serial_number_ = ethernet_module.GetSerialNumber();
  }
};

class ActDevicePowerModule : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, exist, Exist);

  ACT_JSON_FIELD(QString, module_name, ModuleName);            ///< ModuleName item
  ACT_JSON_FIELD(QString, serial_number, SerialNumber);        ///< SerialNumber item
  ACT_JSON_FIELD(QString, product_revision, ProductRevision);  ///< ProductRevision item
  ACT_JSON_FIELD(QString, status, Status);                     ///< Status item

 public:
  ActDevicePowerModule() {
    this->exist_ = false;
    this->module_name_ = "";
    this->serial_number_ = "";
    this->product_revision_ = "";
    this->status_ = "";
  }
};

class ActDeviceSimplePowerModule : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, module_name, ModuleName);      ///< ModuleName item
  ACT_JSON_FIELD(QString, serial_number, SerialNumber);  ///< SerialNumber item

 public:
  ActDeviceSimplePowerModule() {
    this->module_name_ = "";
    this->serial_number_ = "";
  }
  ActDeviceSimplePowerModule(const ActDevicePowerModule &power_module) : ActDeviceSimplePowerModule() {
    module_name_ = power_module.GetModuleName();
    serial_number_ = power_module.GetSerialNumber();
  }
};

/**
 * @brief The ACT Device modular info class
 *
 */
class ActDeviceModularInfo : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActDeviceEthernetModule, ethernet,
                           Ethernet);  ///< The Ethernet module map <SlotID, EthernetModule>

  ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActDevicePowerModule, power,
                           Power);  ///< The Ethernet module map <SlotID, PowerModule>

  // ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActSFPInfo, sfp, SFP);  // Doesn't belong to Ethernet <PortID, SFP>
};

class ActDevicePortInfoEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, interface_id, InterfaceId);
  ACT_JSON_FIELD(qint64, module_slot, ModuleSlot);
  ACT_JSON_FIELD(qint64, module_port, ModulePort);
  ACT_JSON_FIELD(bool, exist, Exist);
  ACT_JSON_FIELD(bool, sfp_inserted, SFPInserted);

 public:
  /**
   * @brief Construct a new Act Monitor Port Info Entry object
   *
   */
  ActDevicePortInfoEntry() {
    this->interface_id_ = -1;
    this->module_slot_ = -1;
    this->module_port_ = -1;
    this->exist_ = false;
    this->sfp_inserted_ = false;
  }

  ActDevicePortInfoEntry(const qint64 &interface_id) : ActDevicePortInfoEntry() { this->interface_id_ = interface_id; }
};

class ActDeviceModularConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT(QMap, qint64, qint64, ethernet,
                   Ethernet);  ///< The Ethernet module map <SlotID, EthernetModule ID>

  ACT_JSON_QT_DICT(QMap, qint64, qint64, power, Power);  ///< The Ethernet module map <SlotID, PowerModule ID>

  // ACT_JSON_QT_DICT_OBJECTS(QMap, qint64, ActSFPInfo, sfp, SFP);  // Doesn't belong to Ethernet <PortID, SFP>

 public:
  friend bool operator==(const ActDeviceModularConfiguration &x, const ActDeviceModularConfiguration &y) {
    bool ethernet_is_same = (x.GetEthernet() == y.GetEthernet());
    bool power_is_same = (x.GetPower() == y.GetPower());
    return (ethernet_is_same && power_is_same);
  }
};