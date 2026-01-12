/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <QList>
#include <iostream>
#include <string>
#include <unordered_map>

#include "../http_utils.h"
#include "act_core.hpp"
#include "act_status.hpp"
#include "dto/act_status_dto.hpp"
#include "oatpp-websocket/Handshaker.hpp"
#include "oatpp/core/Types.hpp"

// #include "oatpp/core/concurrency/SpinLock.hpp"
#include "oatpp/core/data/stream/FileStream.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/protocol/http/outgoing/StreamingBody.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class ActWSController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 private:
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler, "websocket");

 public:
  typedef ActWSController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActWSController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {
    // setDefaultAuthorizationHandler(
    //     std::make_shared<oatpp::web::server::handler::BearerAuthorizationHandler>("my-realm"));  // JWT
  }

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActWSController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
    return std::make_shared<ActWSController>(objectMapper);
  }

 public:
  // TODO Insert Your endpoints here !!!
  ENDPOINT_INFO(ConnectWebSocket) {
    info->summary = "Start web socket";
    info->addTag("Web Socket");
  }
  ENDPOINT("GET", QString("%1/ws").arg(ACT_API_PATH_PREFIX).toStdString(), ConnectWebSocket,
           REQUEST(std::shared_ptr<IncomingRequest>, request)) {
    ACT_STATUS_INIT();

    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "WebSocket URL:" << routes.c_str();

    qint64 project_id = -1;

    QMutexLocker lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.CheckWSConnect(project_id);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    auto response =
        oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), websocketConnectionHandler);

    auto parameters = std::make_shared<oatpp::network::ConnectionHandler::ParameterMap>();
    (*parameters)["projectId"] = String("-1");  // set project id -1 as system connect
    // Set connection params
    response->setConnectionUpgradeParameters(parameters);
    return response;
  };

  ENDPOINT_INFO(ProjectConnectWebSocket) {
    info->summary = "Start web socket for Project";
    info->addTag("WebSocket");
  }
  ENDPOINT("GET", QString("%1/ws/project/{projectId}").arg(ACT_API_PATH_PREFIX).toStdString(), ProjectConnectWebSocket,
           PATH(String, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request)) {
    ACT_STATUS_INIT();

    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "WebSocket URL:" << routes.c_str();

    qint64 project_id_qint64 = QString(projectId->c_str()).toInt();

    QMutexLocker lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.CheckWSConnect(project_id_qint64);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    auto response =
        oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), websocketConnectionHandler);

    auto parameters = std::make_shared<oatpp::network::ConnectionHandler::ParameterMap>();
    (*parameters)["projectId"] = projectId;
    // Set connection params
    response->setConnectionUpgradeParameters(parameters);
    return response;
  };
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
