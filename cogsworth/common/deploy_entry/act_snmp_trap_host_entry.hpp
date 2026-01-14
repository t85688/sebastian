/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_system.hpp"

enum class ActSnmpTrapModeEnum { kTrapV1 = 1, kTrapV2c = 2, kInformV2c = 3, kNotSupport = 4 };

static const QMap<QString, ActSnmpTrapModeEnum> kActSnmpTrapModeEnumMap = {
    {"TrapV1", ActSnmpTrapModeEnum::kTrapV1},
    {"TrapV2c", ActSnmpTrapModeEnum::kTrapV2c},
    {"InformV2c", ActSnmpTrapModeEnum::kInformV2c},
    {"TrapV3", ActSnmpTrapModeEnum::kNotSupport}};

class ActSnmpTrapHostEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, host_name, HostName);
  ACT_JSON_ENUM(ActSnmpTrapModeEnum, mode, Mode);
  ACT_JSON_FIELD(QString, trap_community, TrapCommunity);

 public:
  /**
   * @brief Construct a new Act Vlan Port Type Entry object
   *
   */
  ActSnmpTrapHostEntry() {
    this->host_name_ = "";
    this->mode_ = ActSnmpTrapModeEnum::kTrapV1;
    this->trap_community_ = ACT_DEFAULT_SNMP_TRAP_COMMUNITY;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActSnmpTrapHostEntry &x) {
    return qHash(x.host_name_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActSnmpTrapHostEntry &x, const ActSnmpTrapHostEntry &y) {
    return x.host_name_ == y.host_name_;
  }
};
