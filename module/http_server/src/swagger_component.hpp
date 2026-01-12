/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "http_utils.h"
#include "oatpp-swagger/ControllerPaths.hpp"
#include "oatpp-swagger/Generator.hpp"
#include "oatpp-swagger/Model.hpp"
#include "oatpp-swagger/Resources.hpp"
#include "oatpp/core/macro/component.hpp"

// #include "websocket/act_ws_listener.hpp"

/**
 *  Swagger ui is served at
 *  http://host:port/swagger/ui
 */
class SwaggerComponent {
 public:
  /**
   *  General API docs info
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::DocumentInfo>, swaggerDocumentInfo)
  ([] {
    oatpp::String httpsUrl = GetMainHttpsUrlForSwagger();
    oatpp::swagger::DocumentInfo::Builder builder;

    builder.setTitle("User entity service")
        .setDescription("Chamberlain API with swagger docs\nPlease call Login API to get the token")
        .setVersion("3.0")
        .setLicenseName("Apache License, Version 2.0")
        .setLicenseUrl("http://www.apache.org/licenses/LICENSE-2.0")
        // When you are using the AUTHENTICATION() Endpoint-Macro you must add an SecurityScheme object
        // (https://swagger.io/specification/#securitySchemeObject) For basic-authentication you can use the default
        // Basic-Authorization-Security-Scheme like this For more complex authentication schemes you can use the
        // oatpp::swagger::DocumentInfo::SecuritySchemeBuilder builder Don't forget to add
        // info->addSecurityRequirement("basic_auth") to your ENDPOINT_INFO() Macro!
        .addSecurityScheme(
            "my-realm", oatpp::swagger::DocumentInfo::SecuritySchemeBuilder::DefaultBearerAuthorizationSecurityScheme())
        .addServer(httpsUrl, "server on localhost");

    return builder.build();
  }());

  /**
   *  Swagger-Ui Resources (<oatpp-examples>/lib/oatpp-swagger/res)
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::Resources>, swaggerResources)
  ([] {
    // Make sure to specify correct full path to oatpp-swagger/res folder !!!
    return oatpp::swagger::Resources::loadResources(OATPP_SWAGGER_RES_PATH);
  }());

  // Generator::Config
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::Generator::Config>, generatorConfig)
  ([] { return std::make_shared<oatpp::swagger::Generator::Config>(); }());

  /**
   *  Swagger Controller Paths
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::ControllerPaths>, controllerPaths)
  ([] {
    auto paths = std::make_shared<oatpp::swagger::ControllerPaths>();
    paths->apiJson = "api-docs/oas-3.0.0.json";  // default is "api-docs/oas-3.0.0.json"
    paths->ui = "swagger/ui";                    // default is "swagger/ui"
    paths->uiResources = "swagger/{filename}";   // default is "swagger/{filename}"
    return paths;
  }());
};
