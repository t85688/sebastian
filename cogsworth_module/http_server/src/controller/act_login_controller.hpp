/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include "../http_utils.h"
#include "act_core.hpp"
#include "act_status.hpp"
#include "act_user.hpp"
#include "dto/act_login_dto.hpp"
#include "dto/act_status_dto.hpp"
#include "dto/act_user_dto.hpp"
#include "oatpp/core/Types.hpp"

// #include "oatpp/core/concurrency/SpinLock.hpp"
// #include "oatpp/core/data/stream/FileStream.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/protocol/http/outgoing/StreamingBody.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include OATPP_CODEGEN_BEGIN(ApiController)  //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class ActLoginController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActLoginController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActLoginController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {}

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActLoginController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                          objectMapper)) {
    return std::make_shared<ActLoginController>(objectMapper);
  }

 public:
  // TODO Insert Your endpoints here !!!
  ENDPOINT_INFO(Login) {
    info->summary = "Login to the system";
    info->addTag("Login");
    info->addConsumes<Object<ActLoginDto>>("application/json");

    info->addResponse<Object<ActTokenDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("POST", QString("%1/login").arg(ACT_API_PATH_PREFIX).toStdString(), Login,
           REQUEST(std::shared_ptr<IncomingRequest>, request)) {
    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "POST URL:" << routes.c_str();

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActUser user;
    try {
      user.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qDebug() << "Login user:" << user.GetUsername();

    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QString token;
    act_status = act::core::g_core.Login(user, token);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // Generate the response
    auto dto = ActTokenDto::createShared();
    dto->token = token.toStdString().c_str();
    switch (user.GetRole()) {
      case ActRoleEnum::kAdmin:
        dto->role = RoleDtoEnum::ADMIN;
        break;
      case ActRoleEnum::kSupervisor:
        dto->role = RoleDtoEnum::SUPERVISOR;
        break;
      case ActRoleEnum::kUser:
      default:
        dto->role = RoleDtoEnum::USER;
        break;
    }
    return createDtoResponse(Status::CODE_200, dto);
  }

  ENDPOINT_INFO(CheckCLITokenExist) {
    info->summary = "Check the CLI token exist";
    info->addTag("Login");
    info->addResponse<Object<ActTokenDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/login/check").arg(ACT_API_PATH_PREFIX).toStdString(), CheckCLITokenExist,
           REQUEST(std::shared_ptr<IncomingRequest>, request)) {
    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "GET URL:" << routes.c_str();

    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    ActUser user;
    QString token;
    act_status = act::core::g_core.CheckCLITokenExist(user, token);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // Generate the response
    auto dto = ActTokenDto::createShared();
    dto->token = token.toStdString().c_str();
    switch (user.GetRole()) {
      case ActRoleEnum::kAdmin:
        dto->role = RoleDtoEnum::ADMIN;
        break;
      case ActRoleEnum::kSupervisor:
        dto->role = RoleDtoEnum::SUPERVISOR;
        break;
      case ActRoleEnum::kUser:
      default:
        dto->role = RoleDtoEnum::USER;
        break;
    }
    return createDtoResponse(Status::CODE_200, dto);
  }

  ENDPOINT_INFO(RenewToken) {
    info->summary = "Renew the token";
    info->addSecurityRequirement("my-realm");
    info->addTag("Login");
    info->addResponse<Object<ActTokenDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/renew").arg(ACT_API_PATH_PREFIX).toStdString(), RenewToken,
           REQUEST(std::shared_ptr<IncomingRequest>, request),
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "GET URL:" << routes.c_str();

    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QString orig_token = authorizationBearer->token->c_str();
    QString new_token;
    ActUser user;
    act_status = act::core::g_core.RenewToken(orig_token, new_token, user);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // Generate the response
    auto dto = ActTokenDto::createShared();
    dto->token = new_token.toStdString().c_str();
    switch (user.GetRole()) {
      case ActRoleEnum::kAdmin:
        dto->role = RoleDtoEnum::ADMIN;
        break;
      case ActRoleEnum::kSupervisor:
        dto->role = RoleDtoEnum::SUPERVISOR;
        break;
      case ActRoleEnum::kUser:
      default:
        dto->role = RoleDtoEnum::USER;
        break;
    }
    return createDtoResponse(Status::CODE_200, dto);
  }

  ENDPOINT_INFO(Logout) {
    info->summary = "Logout from the system";
    info->addSecurityRequirement("my-realm");
    info->addTag("Login");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_200, "application/json")
        .addExample("OK", ActStatusDto::createShared(StatusDtoEnum::kSuccess, SeverityDtoEnum::kDebug));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("POST", QString("%1/logout").arg(ACT_API_PATH_PREFIX).toStdString(), Logout,
           REQUEST(std::shared_ptr<IncomingRequest>, request),
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "POST URL:" << routes.c_str();

    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QString token = authorizationBearer->token->c_str();
    act_status = act::core::g_core.Logout(token);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    auto status_dto = ActStatusDto::createShared();
    status_dto->status = StatusDtoEnum::kSuccess;
    status_dto->severity = SeverityDtoEnum::kDebug;
    return createDtoResponse(Status::CODE_200, status_dto);
  }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
