/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "http_utils.h"
#include "oatpp-websocket/ConnectionHandler.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "swagger_component.hpp"
#include "websocket/act_ws_listener.hpp"

/**
 *  Class which creates and holds Application components and registers components in oatpp::base::Environment
 *  Order of components initialization is from top to bottom
 */
class AppComponent {
 public:
  /**
   *  Swagger component
   */
  SwaggerComponent swaggerComponent;

  /**
   *  Create ConnectionProvider component which listens on the port
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)
  ([] {
    return oatpp::network::tcp::server::ConnectionProvider::createShared(
        {"localhost", (v_uint16)GetCogsworthServerPort(), oatpp::network::Address::IP_4});
  }());

  /**
   *  Create Router component
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)
  ([] { return oatpp::web::server::HttpRouter::createShared(); }());

  /**
   *  Create http ConnectionHandler
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, httpConnectionHandler)
  ("https" /* qualifier */, [] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);  // get Router component
    return oatpp::web::server::HttpConnectionHandler::createShared(router);
  }());

  /**
   *  Create ObjectMapper component to serialize/deserialize DTOs in Contoller's API
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)
  ([] { return oatpp::parser::json::mapping::ObjectMapper::createShared(); }());

  /**
   *  Create websocket connection handler
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler)
  ("websocket", [] {
    // OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
    // auto connectionHandler = oatpp::websocket::AsyncConnectionHandler::createShared(executor);
    auto connectionHandler = oatpp::websocket::AsyncConnectionHandler::createShared();
    connectionHandler->setSocketInstanceListener(std::make_shared<WSInstanceListener>());
    return connectionHandler;
  }());

  // OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler)
  // ("websocket" /* qualifier */, [] {
  //   auto connectionHandler = oatpp::websocket::ConnectionHandler::createShared();
  //   connectionHandler->setSocketInstanceListener(std::make_shared<WSInstanceListener>());
  //   return connectionHandler;
  // }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<StaticFilesManager>, staticFilesManager)
  ([] {
    return std::make_shared<StaticFilesManager>(
        "./dist" /* path to '<this-repo>/dist' folder. Put full, absolute path here */);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<StaticDeviceIconFilesManager>, staticDeviceIconFilesManager)
  ([] {
    return std::make_shared<StaticDeviceIconFilesManager>(
        "./configuration/device_icon" /* path to '<this-repo>/configuration/device_icon' folder. Put full, absolute path here */);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<StaticQuestionnaireTemplateFilesManager>,
                         staticQuestionnaireTemplateFilesManager)
  ([] { return std::make_shared<StaticQuestionnaireTemplateFilesManager>("./db/intelligent"); }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<StaticTopologyIconFilesManager>, staticTopologyIconFilesManager)
  ([] {
    return std::make_shared<StaticTopologyIconFilesManager>(
        "./db/topologies/icons" /* path to '<this-repo>/db/topologies/icons' folder. Put full, absolute path here */);
  }());
};
