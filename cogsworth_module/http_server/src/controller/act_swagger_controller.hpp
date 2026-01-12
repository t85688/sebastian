/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include "../http_utils.h"
#include "oatpp/core/Types.hpp"
// #include "oatpp/core/concurrency/SpinLock.hpp"
#include "oatpp/core/data/stream/FileStream.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/protocol/http/outgoing/StreamingBody.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include OATPP_CODEGEN_BEGIN(ApiController)  //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class ActSwaggerController : public oatpp::web::server::api::ApiController {
 public:
  typedef ActSwaggerController __ControllerType;

 private:
  // OATPP_COMPONENT(std::shared_ptr<SwaggerStaticFilesManager>, swaggerStaticFileManager);

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  // ActSwaggerController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
  //     : oatpp::web::server::api::ApiController(objectMapper) {
  //   setDefaultAuthorizationHandler(
  //       std::make_shared<oatpp::web::server::handler::BearerAuthorizationHandler>("my-realm"));  // JWT
  // }
  ActSwaggerController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {}

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActSwaggerController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                            objectMapper)) {
    return std::make_shared<ActSwaggerController>(objectMapper);
  }

 public:
  // TODO Insert Your endpoints here !!!

  /**
   * @brief The URL of the Swagger-UI
   *
   */
  ENDPOINT("GET", "/swagger", SwaggerUI) {
    const char *html =
        "<html lang='en'>"
        "  <head>"
        "    <meta charset=utf-8/>"
        "  </head>"
        "  <body>"
        "    <p>Chamberlain</p>"
        "    <a href='/swagger/ui'>Checkout Swagger-UI page</a>"
        "  </body>"
        "</html>";
    auto response = createResponse(Status::CODE_200, html);
    response->putHeader(Header::CONTENT_TYPE, "text/html");
    return response;
  }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
