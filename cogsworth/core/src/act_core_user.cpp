#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::CheckUser(const ActUser &user, const bool allow_password_empty) {
  ACT_STATUS_INIT();

  // Check username name is valid
  // Check if contains special characters (\ / : * ? “ < > |) that can not use in windows
  const QString username = user.GetUsername();
  const QString forbidden_char = "\\/:*?\"<>|";
  for (const QChar &c : forbidden_char) {
    if (username.contains(c)) {
      QString error_msg = QString("Username contains forbidden characters, the name is %1").arg(username);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Check if trailing space and multiple consecutive spaces
  if (username.endsWith(" ") || username.contains(QRegExp("\\s{2,}"))) {
    QString error_msg =
        QString("Username contains multiple consecutive spaces or end with space, the name is %1").arg(username);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // The length should be 4 ~ 32 characters.
  if (user.GetUsername().length() < ACT_USERNAME_LENGTH_MIN || user.GetUsername().length() > ACT_STRING_LENGTH_MAX) {
    QString error_msg = QString("The length of user name %1 should be %2 ~ %3 characters")
                            .arg(user.GetUsername())
                            .arg(ACT_USERNAME_LENGTH_MIN)
                            .arg(ACT_USERNAME_LENGTH_MAX);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // The length should be 4 ~ 63 characters.
  if (allow_password_empty) {  // update user, the password field might be empty
    if (user.GetPassword().length() > 0) {
      if (user.GetPassword().length() < ACT_PASSWORD_LENGTH_MIN ||
          user.GetPassword().length() > ACT_PASSWORD_LENGTH_MAX) {
        QString error_msg = QString("The length of user password should be %1 ~ %2 characters, not %3")
                                .arg(ACT_PASSWORD_LENGTH_MIN)
                                .arg(ACT_PASSWORD_LENGTH_MAX)
                                .arg(user.GetPassword().length());
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }
  } else {  // create user, the password field is necessary
    if (user.GetPassword().length() < ACT_PASSWORD_LENGTH_MIN ||
        user.GetPassword().length() > ACT_PASSWORD_LENGTH_MAX) {
      QString error_msg = QString("The length of user password should be %1 ~ %2 characters, not %3")
                              .arg(ACT_PASSWORD_LENGTH_MIN)
                              .arg(ACT_PASSWORD_LENGTH_MAX)
                              .arg(user.GetPassword().length());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  return act_status;
}

ACT_STATUS ActCore::CreateUser(ActUser &user) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QSet<ActUser> user_set = this->GetUserSet();

  // Check the user account does not exist
  for (auto u : user_set) {
    if (u.GetUsername() == user.GetUsername()) {
      qCritical() << "The user name" << user.GetUsername() << "is duplicated";
      return std::make_shared<ActDuplicatedError>(user.GetUsername());
    }
  }

  // Add license control
  if (user_set.size() >= ACT_EVALUATION_USER_SIZE) {
    QString error_msg = QString("The user size exceeds the limit: %1").arg(ACT_EVALUATION_USER_SIZE);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActLicenseSizeFailedRequest>("User Size", ACT_EVALUATION_USER_SIZE);
  }

  act_status = this->CheckUser(user);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check User failed with user id:" << user.GetId();
    return act_status;
  }

  // Generate a new unique id
  qint64 id;
  act_status = this->GenerateUniqueIdFromTimeStamp<ActUser>(user_set, this->last_assigned_user_id_, id);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot get an available unique id";
    return act_status;
  }
  user.SetId(id);

  user.SetDataVersion(ACT_USER_DATA_VERSION);

  // Insert the user to core set
  user_set.insert(user);
  this->SetUserSet(user_set);

  // Write to db
  act_status = act::database::user::WriteData(user);

  // Send update msg
  ActUserPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kCreate, user, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_msg);

  return act_status;
}

ACT_STATUS ActCore::GetUser(qint64 &id, ActUser &user) {
  ACT_STATUS_INIT();

  act_status = ActGetItemById<ActUser>(this->GetUserSet(), id, user);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "User id:" << id << "not found";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateUser(ActUser &user) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  act_status = this->CheckUser(user, true);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Check User failed with user id:" << user.GetId();
    return act_status;
  }

  QString password;
  bool password_change = false;
  // If the password is empty, which means this update is for other fields
  if (user.GetPassword().length() != 0) {
    password_change = true;
  }

  QSet<ActUser> user_set = this->GetUserSet();

  // Check the item does exist by id
  typename QSet<ActUser>::const_iterator iterator;
  iterator = user_set.find(user);
  if (iterator != user_set.end()) {
    // Keep the password first
    if (!password_change) {
      password = (*iterator).GetPassword();
    }

    // If yes, delete it
    user_set.erase(iterator);
  }

  // Check the user account does not exist
  for (auto u : user_set) {
    if (u.GetUsername() == user.GetUsername()) {
      qCritical() << "The user name" << user.GetUsername() << "is duplicated";
      return std::make_shared<ActDuplicatedError>(user.GetUsername());
    }
  }

  if (user.GetId() == -1) {
    // Generate a new unique id
    qint64 id;
    act_status = this->GenerateUniqueIdFromTimeStamp<ActUser>(user_set, this->last_assigned_user_id_, id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Cannot get an available unique id";
      return act_status;
    }
    user.SetId(id);
  }

  user.SetDataVersion(ACT_USER_DATA_VERSION);

  // Write back the password item
  if (password_change) {
    // [feat:1248] After changing the password or deleting the account, the login user should be logged out
    this->user_password_changed_[user.GetId()] = true;
  } else {
    user.SetPassword(password);
  }

  // Insert the user to core set
  user_set.insert(user);
  this->SetUserSet(user_set);

  // Write to db
  act_status = act::database::user::WriteData(user);

  // Send Websocket update msg
  ActUserPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, user, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_msg);

  return act_status;
}

ACT_STATUS ActCore::DeleteUser(qint64 &id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QSet<ActUser> user_set = this->GetUserSet();
  // If the user size is 1, it should not be deleted
  if (user_set.size() == 1) {
    qDebug() << "There should be at least one user account in the system";
    return std::make_shared<ActBadRequest>("There should be at least one user account in the system");
  }

  // Check the item does exist by id
  typename QSet<ActUser>::const_iterator iterator;
  iterator = user_set.find(ActUser(id));
  if (iterator == user_set.end()) {
    QString error_msg = QString("Delete user failed, cannot found user id %1").arg(id);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  ActUser user = (*iterator);
  QString username = user.GetUsername();

  // If yes, delete it
  user_set.erase(iterator);

  this->SetUserSet(user_set);

  // Write to db
  act_status = act::database::user::DeleteUserFile(id, username);

  // [feat:1248] After changing the password or deleting the account, the login user should be logged out
  this->user_password_changed_[id] = true;

  // Send update msg
  ActUserPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kDelete, user, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_msg);

  return act_status;
}

}  // namespace core
}  // namespace act
