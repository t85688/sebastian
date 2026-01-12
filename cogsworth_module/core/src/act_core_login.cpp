#include <QSet>

#include "act_core.hpp"
#include "act_user.hpp"
#include "jwt-cpp/jwt.h"

namespace act {
namespace core {

auto prev_iss_time = std::chrono::system_clock::now();

QString __GenerateToken(quint64 hard_timeout) {
  const auto iss_time = std::chrono::system_clock::now();
  const auto exp_time = iss_time + std::chrono::minutes{hard_timeout};

  auto duration = std::chrono::duration_cast<std::chrono::seconds>(iss_time - prev_iss_time);

  if (duration.count() < 1) {
    qDebug() << "Wait a minute... to prevent too many logins at the same time";
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  prev_iss_time = iss_time;

  // Generate token via JWT
  const char *actAppName = GetActAppName().toStdString().c_str();
  QString new_token = jwt::create()
                          .set_issuer(actAppName)
                          .set_issued_at(iss_time)
                          .set_expires_at(exp_time)
                          .set_type("JWS")
                          //  .set_payload_claim("sample", jwt::claim(std::string("test")))
                          .sign(jwt::algorithm::hs256{ACT_TOKEN_SECRET})
                          .c_str();

  return new_token;
}

ACT_STATUS ActCore::VerifyToken(QString token, ActRoleEnum &role) {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->mutex_);

  // Iterator of the set
  typename QMap<QString, qint64>::const_iterator iterator;
  iterator = this->tokens_.find(token);

  bool found = false;
  qint64 user_id;
  // Check find result
  if (iterator != this->tokens_.end()) {
    found = true;
    user_id = iterator.value();

    ActUser user;
    act_status = this->GetUser(user_id, user);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "User not found:" << user_id;
      return std::make_shared<ActStatusNotFound>("User");
    }

    role = user.GetRole();
  }

  if (!found) {
    // qDebug() << "token not found:" << token;
    // qDebug() << "tokens:";
    // QMap<QString, qint64>::const_iterator i = this->tokens_.constBegin();
    // while (i != this->tokens_.constEnd()) {
    //   qDebug() << i.key() << ":" << i.value();
    //   ++i;
    // }
    return std::make_shared<ActStatusNotFound>("Token");
  }

  return act_status;
}

ACT_STATUS ActCore::Login(ActUser &user, QString &token) {
  ACT_STATUS_INIT();

  QSet<ActUser> user_set = this->GetUserSet();

  // Check the login account & password
  bool found = false;
  qint64 user_id = -1;
  for (auto u : user_set) {
    if (u.GetUsername() == user.GetUsername() && u.GetPassword() == user.GetPassword()) {
      user.SetRole(u.GetRole());
      found = true;
      user_id = u.GetId();
      break;
    }
  }

  if (!found) {
    QString error_msg = QString("User not found");
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActUnauthorized>();
  }

  // Check the maximum size of concurrency login users
  // if (this->tokens_.size() > this->GetSystemConfig().GetMaxTokenSize()) {
  //   QString error_msg = QString("The current login session is full");
  //   qCritical() << error_msg.toStdString().c_str();
  //   return std::make_shared<ActBadRequest>(error_msg);
  // }

  token = __GenerateToken(this->GetSystemConfig().GetHardTimeout());

  this->tokens_[token] = user_id;
  // qDebug() << "Allocate token:" << token;
  return act_status;
}

ACT_STATUS ActCore::CLILogin(const QString &username, const QString &password) {
  ACT_STATUS_INIT();
  ActUser user(username, password);
  QString token;
  act_status = Login(user, token);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << act_status->ToString().toStdString().c_str();
    return act_status;
  }

  this->cli_token = token;

  return act_status;
}

ACT_STATUS ActCore::RenewToken(const QString &orig_token, QString &new_token, ActUser &user) {
  ACT_STATUS_INIT();
  QMutexLocker lock(&this->mutex_);

  typename QMap<QString, qint64>::const_iterator iterator;
  iterator = this->tokens_.find(orig_token);

  bool found = false;
  // Check find result
  if (iterator != this->tokens_.end()) {
    found = true;
    qint64 user_id = iterator.value();

    act_status = this->GetUser(user_id, user);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "User not found:" << user_id;
      return std::make_shared<ActStatusNotFound>("User");
    }
  }

  // ! It should be passed because the VerifyToken() API is called before
  if (!found) {
    qDebug() << "token not found:" << orig_token;
    qDebug() << "tokens:";
    QMap<QString, qint64>::const_iterator i = this->tokens_.constBegin();
    while (i != this->tokens_.constEnd()) {
      qDebug() << i.key() << ":" << i.value();
      ++i;
    }

    return std::make_shared<ActStatusNotFound>("Token");
  }

  new_token = __GenerateToken(this->GetSystemConfig().GetHardTimeout());
  // const auto iss_time = std::chrono::system_clock::now();
  // const auto exp_time = iss_time + std::chrono::seconds{this->GetSystemConfig().GetHardTimeout()};

  // Generate token via JWT
  // new_token = jwt::create()
  //                 .set_issuer(GetActAppName().toStdString().c_str())
  //                 .set_issued_at(iss_time)
  //                 .set_expires_at(exp_time)
  //                 .set_type("JWS")
  //                 //  .set_payload_claim("sample", jwt::claim(std::string("test")))
  //                 .sign(jwt::algorithm::hs256{ACT_TOKEN_SECRET})
  //                 .c_str();

  this->tokens_.remove(orig_token);
  this->tokens_[new_token] = user.GetId();
  // qDebug() << "orig token:" << orig_token;
  // qDebug() << "new token:" << new_token;
  return act_status;
}

ACT_STATUS ActCore::Logout(const QString &token) {
  ACT_STATUS_INIT();
  QMutexLocker lock(&this->mutex_);
  this->tokens_.remove(token);
  return act_status;
}

ACT_STATUS ActCore::CheckTokenHardTimeout() {
  ACT_STATUS_INIT();

  QMutexLocker lock(&this->mutex_);

  QMap<QString, qint64> &token_map = this->tokens_;

  const char *actAppName = GetActAppName().toStdString().c_str();

  // Iterator of the set
  for (QMap<QString, qint64>::iterator token_iterator = token_map.begin(); token_iterator != token_map.end();) {
    QString token = token_iterator.key();
    qint64 user_id = token_iterator.value();

    auto decoded = jwt::decode(token.toStdString());
    auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{ACT_TOKEN_SECRET}).with_issuer(actAppName);

    bool expired = false;

    try {
      verifier.verify(decoded);
    } catch (const std::exception &exc) {
      // Remove this token here
      qDebug() << "Token expired:" << token << exc.what();

      token_map.erase(token_iterator++);
      expired = true;
    }

    // [feat:1248] The login user should be logout after password change
    if (this->user_password_changed_.contains(user_id)) {
      qDebug() << "User id:" << QString::number(user_id) << ": Token expired due to password change:" << token;
      token_map.erase(token_iterator++);
      this->user_password_changed_.remove(user_id);
      expired = true;
    }

    if (!expired) {
      token_iterator++;
    }
  }

  this->tokens_ = token_map;
  return act_status;
}

ACT_STATUS ActCore::GetUserIdByToken(const QString &token, qint64 &user_id) {
  ACT_STATUS_INIT();

  typename QMap<QString, qint64>::const_iterator iterator;
  iterator = this->tokens_.find(token);

  bool found = false;
  // Check find result
  if (iterator != this->tokens_.end()) {
    found = true;
    user_id = iterator.value();
  }

  if (!found) {
    qDebug() << "token not found:" << token;
    qDebug() << "tokens:";
    QMap<QString, qint64>::const_iterator i = this->tokens_.constBegin();
    while (i != this->tokens_.constEnd()) {
      qDebug() << i.key() << ":" << i.value();
      ++i;
    }

    return std::make_shared<ActStatusNotFound>("Token");
  }

  return act_status;
}

ACT_STATUS ActCore::CheckCLITokenExist(ActUser &user, QString &token) {
  ACT_STATUS_INIT();

  if (this->cli_token.isEmpty()) {
    return std::make_shared<ActStatusNotFound>("Token");
  }

  if (!this->tokens_.contains(this->cli_token)) {
    return std::make_shared<ActStatusNotFound>("Token");
  }

  qint64 user_id = this->tokens_[this->cli_token];
  act_status = act::core::g_core.GetUser(user_id, user);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  token = this->cli_token;

  return act_status;
}

}  // namespace core
}  // namespace act
