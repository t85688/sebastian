/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"

class ActMgmtEncryptedMoxaService : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, Enable);

 public:
  /**
   * @brief Construct a new Act Encrypted Moxa Service object
   *
   */
  ActMgmtEncryptedMoxaService() { this->enable_ = true; }

  ActMgmtEncryptedMoxaService(const bool &enable) { this->enable_ = enable; }
};

class ActMgmtHttpService : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, Enable);
  ACT_JSON_FIELD(quint16, port, Port);

 public:
  /**
   * @brief Construct a new Act Management Http Service object
   *
   */
  ActMgmtHttpService() {
    this->enable_ = true;
    this->port_ = 80;
  }

  ActMgmtHttpService(const bool &enable, const quint16 &port) {
    this->enable_ = enable;
    this->port_ = port;
  }
};

class ActMgmtHttpsService : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, Enable);
  ACT_JSON_FIELD(quint16, port, Port);

 public:
  /**
   * @brief Construct a new Act Management Http Service object
   *
   */
  ActMgmtHttpsService() {
    this->enable_ = true;
    this->port_ = 443;
  }

  ActMgmtHttpsService(const bool &enable, const quint16 &port) {
    this->enable_ = enable;
    this->port_ = port;
  }
};

enum class ActMgmtSnmpServiceModeEnum { kEnabled = 1, kDisabled = 2, kReadOnly = 3 };
static const QMap<QString, ActMgmtSnmpServiceModeEnum> kActMgmtSnmpServiceModeEnumMap = {
    {"Enabled", ActMgmtSnmpServiceModeEnum::kEnabled},
    {"Disabled", ActMgmtSnmpServiceModeEnum::kDisabled},
    {"ReadOnly", ActMgmtSnmpServiceModeEnum::kReadOnly}};

enum class ActMgmtSnmpServiceTransLayerProtoEnum {
  kUDP = 1,
  kTCP = 2,
};
static const QMap<QString, ActMgmtSnmpServiceTransLayerProtoEnum> kActMgmtSnmpServiceTransLayerProtoEnumMap = {
    {"UDP", ActMgmtSnmpServiceTransLayerProtoEnum::kUDP}, {"TCP", ActMgmtSnmpServiceTransLayerProtoEnum::kTCP}};

class ActMgmtSnmpService : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActMgmtSnmpServiceModeEnum, mode, Mode);
  ACT_JSON_FIELD(quint16, port, Port);
  ACT_JSON_ENUM(ActMgmtSnmpServiceTransLayerProtoEnum, transport_layer_protocol, TransportLayerProtocol);

 public:
  /**
   * @brief Construct a new Act Management Snmp Service object
   *
   */
  ActMgmtSnmpService() {
    this->mode_ = ActMgmtSnmpServiceModeEnum::kEnabled;
    this->port_ = 161;
    this->transport_layer_protocol_ = ActMgmtSnmpServiceTransLayerProtoEnum::kUDP;
  }

  ActMgmtSnmpService(const ActMgmtSnmpServiceModeEnum &mode, const quint16 &port,
                     const ActMgmtSnmpServiceTransLayerProtoEnum &transport_layer_protocol) {
    this->mode_ = mode;
    this->port_ = port;
    this->transport_layer_protocol_ = ActMgmtSnmpServiceTransLayerProtoEnum::kUDP;
  }
};

class ActMgmtSshService : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, Enable);
  ACT_JSON_FIELD(quint16, port, Port);

 public:
  /**
   * @brief Construct a new Act Management Ssh Service object
   *
   */
  ActMgmtSshService() {
    this->enable_ = true;
    this->port_ = 22;
  }

  ActMgmtSshService(const bool &enable, const quint16 &port) {
    this->enable_ = enable;
    this->port_ = port;
  }
};

class ActMgmtTelnetService : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, Enable);
  ACT_JSON_FIELD(quint16, port, Port);

 public:
  /**
   * @brief Construct a new Act Management Telnet Service object
   *
   */
  ActMgmtTelnetService() {
    this->enable_ = true;
    this->port_ = 23;
  }

  ActMgmtTelnetService(const bool &enable, const quint16 &port) {
    this->enable_ = enable;
    this->port_ = port;
  }
};