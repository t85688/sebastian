/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <iostream>
#include <string>

#include "../http_utils.h"
#include "act_core.hpp"
#include "act_model.hpp"
#include "act_service_platform_request.hpp"
#include "act_status.hpp"
#include "dto/act_model_prices_dto.hpp"
#include "dto/act_service_platform_dto.hpp"
#include "dto/act_status_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/protocol/http/outgoing/StreamingBody.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include OATPP_CODEGEN_BEGIN(ApiController)  //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class ActServicePlatformController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActServicePlatformController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActServicePlatformController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {
    // setDefaultAuthorizationHandler(
    //     std::make_shared<oatpp::web::server::handler::BearerAuthorizationHandler>("my-realm"));  // JWT
  }

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActServicePlatformController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                                    objectMapper)) {
    return std::make_shared<ActServicePlatformController>(objectMapper);
  }

  ENDPOINT_INFO(Login) {
    info->summary = "Login to the service platform";
    info->addSecurityRequirement("my-realm");
    info->addTag("Service Platform");
    info->description = "This RESTful API only permits for [Admin, Supervisor]";
    info->addConsumes<Object<ActServicePlatformLoginDto>>("application/json");
    info->addResponse<Object<ActServicePlatformLoginResponseDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("POST", QString("%1/platform/login/").arg(ACT_API_PATH_PREFIX).toStdString(), Login,
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
    qDebug() << "POST URL:" << routes.c_str();

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActServicePlatformLoginRequest login_req;
    try {
      login_req.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qDebug() << "Platform Login user:" << login_req.GetUsername();

    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    QString token_str = QString(authorizationBearer->token->c_str());
    qint64 user_id;
    act_status = act::core::g_core.GetUserIdByToken(token_str, user_id);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    ActServicePlatformLoginResponse login_res;
    act_status = act::core::g_core.ServicePlatformLogin(user_id, login_req, login_res);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    return createResponse(Status::CODE_200, login_res.ToString().toStdString());
  }

  ENDPOINT_INFO(GetModelPrices) {
    info->summary = "Get prices of specified models";
    info->addSecurityRequirement("my-realm");
    info->addTag("Service Platform");
    info->description = "This RESTful API only permits for [Admin, Supervisor]";
    info->addConsumes<Object<ActModelListDto>>("application/json");
    info->addResponse<Object<ActModelPricesDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("POST", QString("%1/platform/model-prices").arg(ACT_API_PATH_PREFIX).toStdString(), GetModelPrices,
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
    qDebug() << "POST URL:" << routes.c_str();

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActModelList model_list;
    try {
      model_list.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    QString token_str = QString(authorizationBearer->token->c_str());
    qint64 user_id;
    act_status = act::core::g_core.GetUserIdByToken(token_str, user_id);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    ActModelPrices model_prices;
    QMap<QString, QString> model_prices_map;
    // create the mock response for the model prices
    for (const auto &model : model_list.GetModelList()) {
      // If the model is the last one, return undefined
      if (model == model_list.GetModelList().last()) {
        continue;
      }

      // Random dollars in string format
      model_prices_map[model] = QString::number(QRandomGenerator::global()->bounded(1, 1001));
    }

    model_prices.SetData(model_prices_map);

    return createResponse(Status::CODE_200, model_prices.ToString().toStdString());
  }

  ENDPOINT_INFO(RegisterProject) {
    info->summary = "Register project to the platform";
    info->addSecurityRequirement("my-realm");
    info->addTag("Service Platform");
    info->description = "This RESTful API only permits for [Admin, Supervisor]";
    info->addResponse<Object<ActStatusDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));

    info->pathParams["projectId"].description = "Project Identifier";
  }
  ENDPOINT("PUT", QString("%1/platform/register/{projectId}").arg(ACT_API_PATH_PREFIX).toStdString(), RegisterProject,
           PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "PUT URL:" << routes.c_str();

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    QString token_str = QString(authorizationBearer->token->c_str());
    qint64 user_id;
    act_status = act::core::g_core.GetUserIdByToken(token_str, user_id);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    act_status = act::core::g_core.ServicePlatformRegister(user_id, project_id);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    act_status->SetActStatus(ActStatusType::kNoContent, ActSeverity::kInformation);
    return createResponse(Status::CODE_204, act_status->ToString(act_status->key_order_).toStdString());
  }

  ENDPOINT_INFO(CheckLoginStatus) {
    info->summary = "Get login status";
    info->addSecurityRequirement("my-realm");
    info->addTag("Service Platform");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActServicePlatformLoginCheckDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/platform/login/status").arg(ACT_API_PATH_PREFIX).toStdString(), CheckLoginStatus,
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    QString token_str = QString(authorizationBearer->token->c_str());
    qint64 user_id;
    act_status = act::core::g_core.GetUserIdByToken(token_str, user_id);
    if (!IsActStatusSuccess(act_status)) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

    ActServicePlatformLoginCheck status;
    act_status = act::core::g_core.CheckServicePlatformToken(user_id, status);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    return createResponse(Status::CODE_200, status.ToString().toStdString());
  }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
