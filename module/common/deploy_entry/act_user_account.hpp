/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_system.hpp"
#include "simplecrypt.h"

enum class ActUserAccountRoleEnum { kAdmin = 1, kSupervisor = 2, kUser = 3 };

static const QMap<QString, ActUserAccountRoleEnum> kActUserAccountRoleEnumMap = {
    {"admin", ActUserAccountRoleEnum::kAdmin},
    {"supervisor", ActUserAccountRoleEnum::kSupervisor},
    {"user", ActUserAccountRoleEnum::kUser}};

class ActUserAccount : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, active, Active);
  ACT_JSON_FIELD(QString, user_name, Username);
  ACT_JSON_FIELD(QString, password, Password);
  ACT_JSON_ENUM(ActUserAccountRoleEnum, role, Role);
  ACT_JSON_FIELD(QString, email, Email);

 private:
  SimpleCrypt crypto_;

 public:
  /**
   * @brief Construct a new Act User Account object
   *
   */
  ActUserAccount() {
    this->active_ = true;
    this->user_name_ = ACT_DEFAULT_DEVICE_ACCOUNT_USERNAME;
    this->password_ = ACT_DEFAULT_DEVICE_ACCOUNT_PASSWORD;
    this->role_ = ActUserAccountRoleEnum::kAdmin;

    // Set crypto key
    bool convert_ok;
    quint64 encrypted_key = 0;
    QString secret = ACT_TOKEN_SECRET;
    encrypted_key = secret.toULongLong(&convert_ok, 16);
    this->crypto_.setKey(encrypted_key);
  }

  /**
   * @brief Hide the password
   *
   * @return ACT_STATUS
   */
  ACT_STATUS HidePassword() {
    ACT_STATUS_INIT();
    this->password_ = "";

    return act_status;
  }

  /**
   * @brief Encrypt the password field
   *
   * @return QString
   */
  ACT_STATUS EncryptPassword() {
    ACT_STATUS_INIT();

    QString password;
    password = crypto_.encryptToString(this->password_);
    this->password_ = password;
    return act_status;
  }

  /**
   * @brief Decrypt the password field
   *
   * @return QString
   */
  ACT_STATUS DecryptPassword() {
    ACT_STATUS_INIT();

    QString password;
    password = crypto_.decryptToString(this->password_);
    this->password_ = password;
    return act_status;
  }
};
