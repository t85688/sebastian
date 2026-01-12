/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_status.hpp"
#include "act_system.hpp"
#include "simplecrypt.h"

// [feat:3183] UI - Modify device connection setting
class ActDeviceAccount : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, default_setting, DefaultSetting);
  ACT_JSON_FIELD(QString, username, Username);
  ACT_JSON_FIELD(QString, password, Password);

 private:
  SimpleCrypt crypto_;

 public:
  QList<QString> key_order_;
  ActDeviceAccount() {
    default_setting_ = true;

    this->username_ = ACT_DEFAULT_DEVICE_ACCOUNT_USERNAME;
    this->password_ = ACT_DEFAULT_DEVICE_ACCOUNT_PASSWORD;
    this->key_order_.append(QList<QString>({QString("Username"), QString("Password")}));

    // Set crypto key
    bool convert_ok;
    quint64 encrypted_key = 0;
    QString secret = ACT_TOKEN_SECRET;
    encrypted_key = secret.toULongLong(&convert_ok, 16);
    this->crypto_.setKey(encrypted_key);
  }

  ActDeviceAccount(const QString &username, const QString &password) : ActDeviceAccount() {
    this->username_ = username;
    this->password_ = password;
  }

  bool IsSameUsernameAndPassword(const QString &username, const QString &password) {
    if (this->username_ != username) {
      return false;
    }
    if (this->password_ != password) {
      return false;
    }
    return true;
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

  static bool CompareConnectConsistent(const ActDeviceAccount &item1, const ActDeviceAccount &item2) {
    if (item1.GetUsername() != item2.GetUsername()) {
      return false;
    }
    if (item1.GetPassword() != item2.GetPassword()) {
      return false;
    }

    return true;
  }
};

/**
 * @brief The SNMP version class enum class
 *
 */
enum class ActSnmpVersionEnum { kV1, kV2c, kV3 };

/**
 * @brief The QMap SNMP version class enum mapping
 *
 */
static const QMap<QString, ActSnmpVersionEnum> kActSnmpVersionEnumMap = {
    {"v1", ActSnmpVersionEnum::kV1}, {"v2c", ActSnmpVersionEnum::kV2c}, {"v3", ActSnmpVersionEnum::kV3}};

/**
 * @brief The SNMP AuthenticationType class enum class
 *
 */
enum class ActSnmpAuthenticationTypeEnum { kNone = 1, kMD5 = 2, kSHA1 = 3 };

/**
 * @brief The QMap SNMP AuthenticationType class enum mapping
 *
 */
static const QMap<QString, ActSnmpAuthenticationTypeEnum> kActSnmpAuthenticationTypeEnumMap = {
    {"None", ActSnmpAuthenticationTypeEnum::kNone},
    {"MD5", ActSnmpAuthenticationTypeEnum::kMD5},
    {"SHA1", ActSnmpAuthenticationTypeEnum::kSHA1}};

/**
 * @brief The SNMP DataEncryptionTypeEnum class enum class
 *
 */
enum class ActSnmpDataEncryptionTypeEnum { kNone = 1, kDES = 2, kAES = 3 };

/**
 * @brief The QMap SNMP DataEncryptionMethod class enum mapping
 *
 */
static const QMap<QString, ActSnmpDataEncryptionTypeEnum> kActSnmpDataEncryptionTypeEnumMap = {
    {"None", ActSnmpDataEncryptionTypeEnum::kNone},
    {"DES", ActSnmpDataEncryptionTypeEnum::kDES},
    {"AES", ActSnmpDataEncryptionTypeEnum::kAES}};

/**
 * @brief The ActDevice's SnmpConfiguration class
 *
 */
class ActSnmpConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, default_setting, DefaultSetting);
  ACT_JSON_ENUM(ActSnmpVersionEnum, version, Version);
  ACT_JSON_FIELD(quint16, port, Port);

  // SNMPv1, SNMPv2c
  ACT_JSON_FIELD(QString, read_community, ReadCommunity);
  ACT_JSON_FIELD(QString, write_community, WriteCommunity);

  // SNMPv3
  ACT_JSON_FIELD(QString, user_name, Username);
  ACT_JSON_ENUM(ActSnmpAuthenticationTypeEnum, authentication_type, AuthenticationType);
  ACT_JSON_FIELD(QString, authentication_password, AuthenticationPassword);
  ACT_JSON_ENUM(ActSnmpDataEncryptionTypeEnum, data_encryption_type, DataEncryptionType);
  ACT_JSON_FIELD(QString, data_encryption_key, DataEncryptionKey);

 private:
  SimpleCrypt crypto_;

 public:
  ActSnmpConfiguration() {
    default_setting_ = true;

    version_ = ACT_DEFAULT_SNMP_VERSION;
    port_ = ACT_DEFAULT_SNMP_PORT;

    // SNMPv1, SNMPv2c
    read_community_ = ACT_DEFAULT_SNMP_READ_COMMUNITY;
    write_community_ = ACT_DEFAULT_SNMP_WRITE_COMMUNITY;

    // SNMPv3
    user_name_ = ACT_DEFAULT_SNMP_USERNAME;
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

  ActSnmpConfiguration(const QString &read_community, const QString &write_community) : ActSnmpConfiguration() {
    default_setting_ = false;

    version_ = ActSnmpVersionEnum::kV2c;
    read_community_ = read_community;
    write_community_ = write_community;
  }

  /**
   * @brief Hide the password
   *
   * @return ACT_STATUS
   */
  ACT_STATUS HidePassword() {
    ACT_STATUS_INIT();

    this->read_community_ = "";
    this->write_community_ = "";

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

    password = crypto_.encryptToString(this->read_community_);
    this->read_community_ = password;

    password = crypto_.encryptToString(this->write_community_);
    this->write_community_ = password;

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
    password = crypto_.decryptToString(this->read_community_);
    this->read_community_ = password;

    password = crypto_.decryptToString(this->write_community_);
    this->write_community_ = password;

    password = crypto_.decryptToString(this->authentication_password_);
    this->authentication_password_ = password;

    password = crypto_.decryptToString(this->data_encryption_key_);
    this->data_encryption_key_ = password;

    return act_status;
  }

  static bool CompareConnectConsistent(const ActSnmpConfiguration &item1, const ActSnmpConfiguration &item2) {
    if (item1.GetVersion() != item2.GetVersion()) {
      return false;
    }
    if (item1.GetPort() != item2.GetPort()) {
      return false;
    }

    // SNMPv1, SNMPv2c
    if (item1.GetVersion() == ActSnmpVersionEnum::kV1 || item1.GetVersion() == ActSnmpVersionEnum::kV2c) {
      if (item1.GetReadCommunity() != item2.GetReadCommunity()) {
        return false;
      }
      if (item1.GetWriteCommunity() != item2.GetWriteCommunity()) {
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

/**
 * @brief The RESTful protocol class enum class
 *
 */
enum class ActRestfulProtocolEnum { kHTTP, kHTTPS };

/**
 * @brief The QMap RESTful protocol class enum mapping
 *
 */
static const QMap<QString, ActRestfulProtocolEnum> kActRestfulProtocolEnumMap = {
    {"HTTP", ActRestfulProtocolEnum::kHTTP}, {"HTTPS", ActRestfulProtocolEnum::kHTTPS}};

/**
 * @brief The ActDevice's RestfulConfiguration class
 *
 */
class ActRestfulConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActRestfulProtocolEnum, protocol, Protocol);
  ACT_JSON_FIELD(quint16, port, Port);
  ACT_JSON_FIELD(bool, default_setting, DefaultSetting);

 private:
  SimpleCrypt crypto_;

 public:
  ActRestfulConfiguration() {
    protocol_ = ACT_DEFAULT_RESTFUL_PROTOCOL;
    port_ = ACT_DEFAULT_RESTFUL_PORT;
    default_setting_ = true;
  }

  ActRestfulConfiguration(const ActRestfulProtocolEnum &protocol, const quint16 &port) : ActRestfulConfiguration() {
    protocol_ = protocol;
    port_ = port;
    default_setting_ = false;
  }

  static bool CompareConnectConsistent(const ActRestfulConfiguration &item1, const ActRestfulConfiguration &item2) {
    if (item1.GetProtocol() != item2.GetProtocol()) {
      return false;
    }
    if (item1.GetPort() != item2.GetPort()) {
      return false;
    }
    return true;
  }
};

/**
 * @brief Netconf sessions over SSH class
 *
 */
class ActNetconfOverSSH : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, ssh_port, SSHPort);

 public:
  ActNetconfOverSSH() { ssh_port_ = ACT_DEFAULT_NETCONF_SSH_PORT; }
};

/**
 * @brief Netconf sessions over TLS class
 *
 */
class ActNetconfOverTLS : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, tls_port, TLSPort);
  ACT_JSON_FIELD(QString, key_file, KeyFile);
  ACT_JSON_FIELD(QString, cert_file, CertFile);
  ACT_JSON_FIELD(QString, ca_certs, CaCerts);

 public:
  ActNetconfOverTLS() : tls_port_(ACT_DEFAULT_NETCONF_TLS_PORT) {}
};

/**
 * @brief The ActDevice's NetconfConfiguration class
 *
 */
class ActNetconfConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, tls, TLS);
  ACT_JSON_OBJECT(ActNetconfOverSSH, netconf_over_ssh, NetconfOverSSH);
  ACT_JSON_OBJECT(ActNetconfOverTLS, netconf_over_tls, NetconfOverTLS);
  ACT_JSON_FIELD(bool, default_setting, DefaultSetting);

 public:
  /**
   * @brief Construct a new Netconf Configuration object
   *
   */
  ActNetconfConfiguration() {
    tls_ = false;
    default_setting_ = true;
  }

  /**
   * @brief Construct a new Netconf Configuration object
   *
   * @param tls
   * @param netconf_over_ssh
   */
  ActNetconfConfiguration(const bool &tls, const ActNetconfOverSSH &netconf_over_ssh)
      : tls_(tls), netconf_over_ssh_(netconf_over_ssh), default_setting_(false) {}

  /**
   * @brief Construct a new Netconf Configuration object
   *
   * @param tls
   * @param netconf_over_tls
   */
  ActNetconfConfiguration(const bool &tls, const ActNetconfOverTLS &netconf_over_tls)
      : tls_(tls), netconf_over_tls_(netconf_over_tls), default_setting_(false) {}

  static bool CompareConnectConsistent(const ActNetconfConfiguration &item1, const ActNetconfConfiguration &item2) {
    if (item1.GetTLS() != item2.GetTLS()) {
      return false;
    }

    if (item1.GetTLS()) {
      // TLS

      if (item1.GetNetconfOverTLS().GetTLSPort() != item2.GetNetconfOverTLS().GetTLSPort()) {
        return false;
      }
      if (item1.GetNetconfOverTLS().GetKeyFile() != item2.GetNetconfOverTLS().GetKeyFile()) {
        return false;
      }
      if (item1.GetNetconfOverTLS().GetCertFile() != item2.GetNetconfOverTLS().GetCertFile()) {
        return false;
      }
      if (item1.GetNetconfOverTLS().GetCaCerts() != item2.GetNetconfOverTLS().GetCaCerts()) {
        return false;
      }

    } else {
      // SSH

      if (item1.GetNetconfOverSSH().GetSSHPort() != item2.GetNetconfOverSSH().GetSSHPort()) {
        return false;
      }
    }

    return true;
  }
};

/**
 * @brief The Device connection configuration class
 *
 */
class ActDeviceConnectConfig : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDeviceAccount, account, Account);
  ACT_JSON_OBJECT(ActNetconfConfiguration, netconf_configuration, NetconfConfiguration);
  ACT_JSON_OBJECT(ActSnmpConfiguration, snmp_configuration, SnmpConfiguration);
  ACT_JSON_OBJECT(ActRestfulConfiguration, restful_configuration, RestfulConfiguration);

 public:
  /**
   * @brief Construct a new Act Device Connect Config object
   *
   */
  ActDeviceConnectConfig() {}

  /**
   * @brief Construct a new Act Device Connect Config object
   *
   * @param account
   * @param netconf_configuration
   * @param snmp_configuration
   * @param restful_configuration
   */
  ActDeviceConnectConfig(const ActDeviceAccount &account, const ActNetconfConfiguration &netconf_configuration,
                         const ActSnmpConfiguration &snmp_configuration,
                         const ActRestfulConfiguration &restful_configuration) {
    account_ = account;
    netconf_configuration_ = netconf_configuration;
    snmp_configuration_ = snmp_configuration;
    restful_configuration_ = restful_configuration;
  }
};
