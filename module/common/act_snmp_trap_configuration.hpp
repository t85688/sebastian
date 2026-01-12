/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"
#include "act_status.hpp"
#include "act_system.hpp"
#include "simplecrypt.h"

/**
 * @brief The SNMP trap server configuration class
 *
 */
class ActSnmpTrapConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActSnmpVersionEnum, version, Version);  ///< The SNMP Version enum
  ACT_JSON_FIELD(quint16, port, Port);                  ///< The SNMP Port

  // SNMPv1, SNMPv2c
  ACT_JSON_FIELD(QString, trap_community, TrapCommunity);  ///< The SNMP TrapCommunity

  // SNMPv3
  ACT_JSON_FIELD(QString, user_name, Username);                                           ///< The Username
  ACT_JSON_ENUM(ActSnmpAuthenticationTypeEnum, authentication_type, AuthenticationType);  ///< The AuthenticationType
  ACT_JSON_FIELD(QString, authentication_password, AuthenticationPassword);  ///< The AuthenticationPassword
  ACT_JSON_ENUM(ActSnmpDataEncryptionTypeEnum, data_encryption_type, DataEncryptionType);  ///< The DataEncryptionType
  ACT_JSON_FIELD(QString, data_encryption_key, DataEncryptionKey);                         ///< The DataEncryptionKey

 private:
  SimpleCrypt crypto_;

 public:
  ActSnmpTrapConfiguration() {
    version_ = ACT_DEFAULT_SNMP_TRAP_VERSION;
    port_ = ACT_DEFAULT_SNMP_TRAP_PORT;

    // SNMPv1, SNMPv2c
    trap_community_ = ACT_DEFAULT_SNMP_TRAP_COMMUNITY;

    // SNMPv3
    user_name_ = "";
    authentication_type_ = ActSnmpAuthenticationTypeEnum::kNone;
    authentication_password_ = "";
    data_encryption_type_ = ActSnmpDataEncryptionTypeEnum::kNone;
    data_encryption_key_ = "";

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
    this->trap_community_ = "";

    this->authentication_password_ = "";
    this->data_encryption_key_ = "";

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

    password = crypto_.encryptToString(this->trap_community_);
    this->trap_community_ = password;

    password = crypto_.encryptToString(this->authentication_password_);
    this->authentication_password_ = password;

    password = crypto_.encryptToString(this->data_encryption_key_);
    this->data_encryption_key_ = password;

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
    password = crypto_.decryptToString(this->trap_community_);
    this->trap_community_ = password;

    password = crypto_.decryptToString(this->authentication_password_);
    this->authentication_password_ = password;

    password = crypto_.decryptToString(this->data_encryption_key_);
    this->data_encryption_key_ = password;

    return act_status;
  }

  static bool CompareConnectConsistent(const ActSnmpTrapConfiguration &item1, const ActSnmpTrapConfiguration &item2) {
    if (item1.GetVersion() != item2.GetVersion()) {
      return false;
    }
    if (item1.GetPort() != item2.GetPort()) {
      return false;
    }

    // SNMPv1, SNMPv2c
    if (item1.GetVersion() == ActSnmpVersionEnum::kV1 || item1.GetVersion() == ActSnmpVersionEnum::kV2c) {
      if (item1.GetTrapCommunity() != item2.GetTrapCommunity()) {
        return false;
      }
    }

    // SNMPv3
    if (item1.GetVersion() == ActSnmpVersionEnum::kV3) {
      if (item1.GetUsername() != item2.GetUsername()) {
        return false;
      }
      if (item1.GetAuthenticationType() != item2.GetAuthenticationType()) {
        return false;
      }
      if (item1.GetAuthenticationPassword() != item2.GetAuthenticationPassword()) {
        return false;
      }
      if (item1.GetDataEncryptionType() != item2.GetDataEncryptionType()) {
        return false;
      }
      if (item1.GetDataEncryptionKey() != item2.GetDataEncryptionKey()) {
        return false;
      }
    }

    return true;
  }
};
