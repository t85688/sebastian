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
#include "act_system.hpp"
#include "dto/act_status_dto.hpp"
#include "dto/act_system_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/protocol/http/outgoing/StreamingBody.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include OATPP_CODEGEN_BEGIN(ApiController)  //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class ActSystemController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActSystemController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActSystemController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {
    // setDefaultAuthorizationHandler(
    //     std::make_shared<oatpp::web::server::handler::BearerAuthorizationHandler>("my-realm"));  // JWT
  }

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActSystemController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                           objectMapper)) {
    return std::make_shared<ActSystemController>(objectMapper);
  }

 public:
  // TODO Insert Your endpoints here !!!
  ENDPOINT_INFO(GetHeartBeat) {
    info->summary = "Get heartbeat to test the connection";
    info->addSecurityRequirement("my-realm");
    info->addTag("System");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActStatusDto>>(Status::CODE_200, "application/json")
        .addExample("OK", ActStatusDto::createShared(StatusDtoEnum::kSuccess, SeverityDtoEnum::kDebug));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/heartbeat").arg(ACT_API_PATH_PREFIX).toStdString(), GetHeartBeat,
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
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    auto status_dto = ActStatusDto::createShared();
    return createDtoResponse(Status::CODE_200, status_dto);
  }

  ENDPOINT_INFO(GetSystem) {
    info->summary = "Get system configuration";
    info->addSecurityRequirement("my-realm");
    info->addTag("System");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    ActSystem sys;
    info->addResponse<Object<ActSystemDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/system").arg(ACT_API_PATH_PREFIX).toStdString(), GetSystem,
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
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActSystem sys = act::core::g_core.GetSystemConfig();

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, sys.ToString().toStdString().c_str());
  }

  ENDPOINT_INFO(GetVersion) {
    info->summary = "Get version information";
    info->addTag("System");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    ActSystem sys;
    info->addResponse<Object<ActVersionDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/version").arg(ACT_API_PATH_PREFIX).toStdString(), GetVersion,
           REQUEST(std::shared_ptr<IncomingRequest>, request)) {
    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "GET URL:" << routes.c_str();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActSystem sys = act::core::g_core.GetSystemConfig();

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, sys.ToString("ActVersion").toStdString().c_str());
  }

  ENDPOINT_INFO(UpdateSystem) {
    info->summary = "Update a system configuration";
    info->addSecurityRequirement("my-realm");
    info->addTag("System");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActSystemDto>>("application/json");
    info->addResponse(Status::CODE_204, "The project was successfully deleted (or did not exist)");

    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("PUT", QString("%1/system").arg(ACT_API_PATH_PREFIX).toStdString(), UpdateSystem,
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
    qDebug() << "PUT URL:" << routes.c_str();

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActSystem system;
    try {
      system.FromString(str);
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

    act_status = act::core::g_core.UpdateSystem(system);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    act_status->SetStatus(ActStatusType::kNoContent);
    // qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
    return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                          act_status->ToString(act_status->key_order_).toStdString());
  }

  ENDPOINT_INFO(PatchSystem) {
    info->summary = "Patch update system configuration";
    info->addSecurityRequirement("my-realm");
    info->addTag("System");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActSystemDto>>("application/json");
    info->addResponse<Object<ActSystemDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("PATCH", QString("%1/system").arg(ACT_API_PATH_PREFIX).toStdString(), PatchSystem,
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
    qDebug() << "PATCH URL:" << routes.c_str();

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActSystem system = act::core::g_core.GetSystemConfig();
    try {
      system.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    act_status = act::core::g_core.UpdateSystem(system);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, system.ToString(system.key_order_).toStdString().c_str());
  }

  ENDPOINT_INFO(OpenBrowser) {
    info->summary = "Open the system browser";
    info->addTag("System");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActStatusDto>>(Status::CODE_200, "application/json");
  }
  ENDPOINT("GET", QString("%1/open-browser").arg(ACT_API_PATH_PREFIX).toStdString(), OpenBrowser,
           REQUEST(std::shared_ptr<IncomingRequest>, request)) {
    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "GET URL:" << routes.c_str();

    ACT_STATUS_INIT();
    act_status = act::core::g_core.OpenBrowser();

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, act_status->ToString(act_status->key_order_).toStdString());
  }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
