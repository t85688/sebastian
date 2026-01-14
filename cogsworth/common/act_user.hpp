/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"
#include "act_system.hpp"
#include "simplecrypt.h"

enum class ActRoleEnum { kUnauthorized, kAdmin, kSupervisor, kUser };

static const QMap<QString, ActRoleEnum> kActRoleEnumMap = {{"Unauthorized", ActRoleEnum::kUnauthorized},
                                                           {"Admin", ActRoleEnum::kAdmin},
                                                           {"Supervisor", ActRoleEnum::kSupervisor},
                                                           {"User", ActRoleEnum::kUser}};

/**
 * @brief The ACT user class
 *
 */
class ActUser : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(qint64, organization_id,
                 OrganizationId);  ///< The organization id is the same as user id in the current version
  ACT_JSON_FIELD(QString, user_name, Username);
  ACT_JSON_FIELD(QString, password, Password);
  ACT_JSON_ENUM(ActRoleEnum, role, Role);
  ACT_JSON_COLLECTION_ENUM(QList, ActServiceProfileForLicenseEnum, profiles,
                           Profiles);  ///< The supported service profiles of the user
  ACT_JSON_FIELD(QString, data_version, DataVersion);

 private:
  SimpleCrypt crypto_;

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new Act User object
   *
   */
  ActUser() {
    this->id_ = -1;
    this->role_ = ActRoleEnum::kUser;
    this->data_version_ = ACT_USER_DATA_VERSION;
    this->key_order_.append(QList<QString>({QString("Id"), QString("Username"), QString("Password"), QString("Role"),
                                            QString("Profiles"), QString("DataVersion")}));

    bool convert_ok;
    quint64 encrypted_key = 0;
    QString secret = ACT_TOKEN_SECRET;
    encrypted_key = secret.toULongLong(&convert_ok, 16);

    this->profiles_.append(ActServiceProfileForLicenseEnum::kSelfPlanning);
    this->crypto_.setKey(encrypted_key);
  }

  /**
   * @brief Construct a new Act User object
   *
   * @param id
   */
  ActUser(const qint64 &id) : ActUser() { this->id_ = id; }

  /**
   * @brief Construct a new Act User object
   *
   * @param id
   */
  ActUser(const QString username, const QString password) : ActUser() {
    this->user_name_ = username;
    this->password_ = password;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActUser &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActUser &x, const ActUser &y) { return x.id_ == y.id_ || x.user_name_ == y.user_name_; }

  /**
   * @brief [bugfix:1961] Remove the password related fields
   *
   * @return ACT_STATUS
   */
  ACT_STATUS HidePassword() {
    ACT_STATUS_INIT();

    this->SetPassword("");

    return act_status;
  }

  /**
   * @brief Encrypt the password
   *
   * @return ACT_STATUS
   */
  ACT_STATUS EncryptPassword() {
    ACT_STATUS_INIT();

    this->password_ = crypto_.encryptToString(this->password_);

    return act_status;
  }

  /**
   * @brief Decrypt the encrypted password
   *
   * @return QString
   */
  ACT_STATUS DecryptPassword() {
    ACT_STATUS_INIT();

    this->password_ = crypto_.decryptToString(this->password_);

    return act_status;
  }

  // /**
  //  * @brief Get encrypted password
  //  *
  //  * @return ACT_STATUS
  //  */
  // QString GetEncryptedPassword(const ActUser& user) {
  //   // Convert const to non-const
  //   QString password = user.GetPassword();
  //   QString result = crypto_.encryptToString(password);
  //   return result;
  // }
  // /**
  //  * @brief Get decrypted password
  //  *
  //  * @return ACT_STATUS
  //  */
  // QString GetDecryptedPassword(const ActUser& user) {
  //   // Convert const to non-const
  //   QString password = user.GetPassword();
  //   QString result = crypto_.decryptToString(password);
  //   return result;
  // }
};

/**
 * @brief The simple ACT user class
 *
 */
class ActSimpleUser : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(qint64, id, Id);                ///< The unique id of the user
  ACT_JSON_FIELD(QString, user_name, Username);  ///< The name of the user
  ACT_JSON_ENUM(ActRoleEnum, role, Role);        ///< The role of the user
  ACT_JSON_COLLECTION_ENUM(QList, ActServiceProfileForLicenseEnum, profiles,
                           Profiles);  ///< The supported service profiles of the user

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new Act User object
   *
   */
  ActSimpleUser() {
    this->id_ = -1;
    this->role_ = ActRoleEnum::kUser;
    this->key_order_.append(QList<QString>({QString("Id"), QString("Username"), QString("Role"), QString("Profiles")}));
  }

  /**
   * @brief Construct a new Act User object
   *
   * @param id
   */
  ActSimpleUser(const qint64 &id) : ActSimpleUser() { this->id_ = id; }

  ActSimpleUser(const ActUser &user) {
    this->id_ = user.GetId();
    this->user_name_ = user.GetUsername();
    this->role_ = user.GetRole();
    this->profiles_ = user.GetProfiles();
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActSimpleUser &obj) {
    // no effect on the program's behavior
    static_cast<void>(obj);
    return 0;
  }

  /**
   * @brief The equal operator
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActSimpleUser &x, const ActSimpleUser &y) {
    return x.id_ == y.id_ || x.user_name_ == y.user_name_;
  }
};
class ActSimpleUserSet : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET_OBJECTS(ActSimpleUser, simple_user_set, SimpleUserSet);
};

/**
 * @brief The class of token
 *
 */
// class ActToken : public QSerializer {
//   Q_GADGET
//   QS_SERIALIZABLE

//   // Create data members to be serialized - you can use this members in code
//   ACT_JSON_FIELD(QString, token, Token);               ///< The token item
//   ACT_JSON_FIELD(quint64, expired_time, ExpiredTime);  ///< The expired time of the token
//   ACT_JSON_FIELD(qint64, user_id, UserId);             ///< The user id of the token
// };
