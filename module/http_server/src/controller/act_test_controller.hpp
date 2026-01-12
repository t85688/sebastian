/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/utils/ConversionUtils.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include OATPP_CODEGEN_BEGIN(ApiController)  //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class ActTestController : public oatpp::web::server::api::ApiController {
 public:
  typedef ActTestController __ControllerType;

  //  private:
  //   static constexpr const char* TAG = "test::web::app::ActTestController";

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActTestController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {}

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActTestController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
    return std::make_shared<ActTestController>(objectMapper);
  }

 public:
  // TODO Insert Your endpoints here !!!

  ENDPOINT_INFO(testtoken) {
    info->summary = "Test";
    info->addSecurityRequirement("my-realm");
    info->addTag("TEST");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    // info->addConsumes<Object<UserDto>>("application/json");
    // info->addResponse<Object<UserDto>>(Status::CODE_200, "application/json");
  }
  // ENDPOINT("GET", "/testtoken", testtoken) {
  //   qDebug() << "GET: /testtoken";
  //   return createResponse(Status::CODE_200, "OK");
  // }
  ENDPOINT("GET", "/testtoken", testtoken,
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }
    qDebug() << "GET: /testtoken";
    return createResponse(Status::CODE_200, "OK");
  }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
