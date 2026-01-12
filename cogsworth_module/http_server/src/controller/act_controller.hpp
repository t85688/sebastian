/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <QDebug>
#include <QFileInfo>
#include <iostream>
#include <string>
#include <unordered_map>

#include "../http_utils.h"
#include "act_core.hpp"
#include "act_status.hpp"

// #include "dto/DTOs.hpp"
#include "oatpp/core/Types.hpp"
// #include "oatpp/core/concurrency/SpinLock.hpp"
#include "oatpp/core/base/Environment.hpp"
#include "oatpp/core/data/stream/FileStream.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/utils/String.hpp"
#include "oatpp/web/protocol/http/outgoing/StreamingBody.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  //<-- Begin Codegen

/**
 * @brief The class of verified data in the authorization logic
 *
 * https://github.com/oatpp/oatpp/issues/475
 * https://github.com/oatpp/oatpp/blob/master/test/oatpp/web/app/BearerAuthorizationController.hpp
 * https://github.com/oatpp/example-jwt
 *
 */
class BearerAuthorizationObject : public oatpp::web::server::handler::AuthorizationObject {
 public:
  // oatpp::String user;
  // oatpp::String password;
  ActRoleEnum role;
  oatpp::String token;
};

/**
 * @brief The class of authorization handler
 *
 */
class MyBearerAuthorizationHandler : public oatpp::web::server::handler::BearerAuthorizationHandler {
 public:
  MyBearerAuthorizationHandler() : oatpp::web::server::handler::BearerAuthorizationHandler("my-realm") {}

  std::shared_ptr<AuthorizationObject> authorize(const oatpp::String &token) override {
    ACT_STATUS_INIT();

    // Verify Token
    ActRoleEnum role;
    QList<QString> builtin_token_list;
    builtin_token_list.append("MoxaTech89191230");                        // For swagger & API testing
    builtin_token_list.append("ONLY-FOR-MOXA-AI-SERVICE-DO-NOT-REMOVE");  // [feat:3295] For AI service
    if (builtin_token_list.contains(token.getPtr()->c_str())) {
      role = ActRoleEnum::kAdmin;
    } else {
      act_status = act::core::g_core.VerifyToken(token.getPtr()->c_str(), role);
      if (!IsActStatusSuccess((act_status))) {
        qWarning() << "Authorization failed: invalid token";
        auto obj = std::make_shared<BearerAuthorizationObject>();
        obj->role = ActRoleEnum::kUnauthorized;
        obj->token = token;
        return obj;
      }
    }

    auto obj = std::make_shared<BearerAuthorizationObject>();
    obj->role = role;
    obj->token = token;
    return obj;
  }
};

/**
 * Sample Api Controller.
 */
class ActController : public oatpp::web::server::api::ApiController {
 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActController __ControllerType;

 private:
  OATPP_COMPONENT(std::shared_ptr<StaticFilesManager>, staticFileManager);

  //  private:
  //   std::shared_ptr<OutgoingResponse> getStaticFileResponse(const oatpp::String& filename,
  //                                                           const oatpp::String& rangeHeader) const;
  //   std::shared_ptr<OutgoingResponse> getFullFileResponse(const oatpp::String& file) const;
  //   std::shared_ptr<OutgoingResponse> getRangeResponse(const oatpp::String& rangeStr, const oatpp::String& file)
  //   const;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {
    // setDefaultAuthorizationHandler(
    //     std::make_shared<oatpp::web::server::handler::BearerAuthorizationHandler>("my-realm"));  // JWT
  }

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
    return std::make_shared<ActController>(objectMapper);
  }

 public:
  // TODO Insert Your endpoints here !!!

  /**
   * @brief The root URL of the http server
   *
   */
  ENDPOINT("GET", "/", Root, REQUEST(std::shared_ptr<IncomingRequest>, request)) {
    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "User-Agent:" << request->getHeader("User-Agent")->c_str();
    qDebug() << "URL:" << routes.c_str();
    // qDebug() << "peer_address_format:" << context.getProperties().get("peer_address_format")->c_str();
    // qDebug() << "peer_address:" << context.getProperties().get("peer_address")->c_str();
    // qDebug() << "peer_port:" << context.getProperties().get("peer_port")->c_str();
    auto &context = request->getConnection()->getInputStreamContext();
    for (const auto &pair : context.getProperties().getAll()) {
      qDebug() << pair.first.std_str().c_str() << ":" << pair.second.std_str().c_str();
    }

    auto file = staticFileManager->GetFile("index.html");
    // return createResponse(Status::CODE_200, file);
    auto response = createResponse(Status::CODE_200, file);
    // response->putHeader(Header::CONNECTION, Header::Value::CONNECTION_CLOSE);
    return response;
  }

  QString getFileExtension(const QString &filePath) {
    QFileInfo info(filePath);
    return info.suffix();
  }

  QString stripQuery(const QString &urlPath) {
    // Find the position of '?'
    int pos = urlPath.indexOf('?');

    // Extract the substring before '?'
    QString res = (pos == -1) ? urlPath : urlPath.left(pos);

    // Trim leading and trailing whitespace or control characters
    return res.trimmed();
  }
  /**
   * @brief The URL to get static files
   *
   * NOTE: Endpoint with "*" mapping should be added as a bottom list endpoint.
   *       In order not to intercept other mappings.
   */
  ENDPOINT("GET", "*", StaticFile, REQUEST(std::shared_ptr<IncomingRequest>, request)) {
    auto routes = request->getStartingLine().path.std_str();
    // qDebug() << "URL:" << routes.c_str();

    // Get the tail
    auto tail = request->getPathTail();

    // Remove the query parameter. ex: "assets/icons-sprite.svg?ts=1757945604178" -> "assets/icons-sprite.svg"
    QString qclean_tail = stripQuery(tail->c_str());
    // qDebug() << QString("URL: %1, Tail: %2, CleanTail: %3 ").arg(routes.c_str()).arg(tail->c_str()).arg(qclean_tail);

    // Get the target file
    oatpp::String oat_path = qclean_tail.toStdString().c_str();
    auto file = staticFileManager->GetFile(oat_path);

    QString extension = getFileExtension(qclean_tail);

    // Check tail not file or like dir, would redirect to index.html
    bool looksLikeDir = qclean_tail.isEmpty() || qclean_tail.endsWith('/') || extension.isEmpty();
    if (!file || looksLikeDir) {
      auto file = staticFileManager->GetFile("index.html");

      OATPP_ASSERT_HTTP(file.get() != nullptr, Status::CODE_404, "File not found");
      auto response = createResponse(Status::CODE_200, file);
      return response;
    }

    // qDebug() << QString("URL Extension: %1").arg(extension);
    // Set the MIME type based on the file extension
    oatpp::String mimeType;
    if (extension == "js") {
      mimeType = "application/javascript";
    } else if (extension == "css") {
      mimeType = "text/css";
    } else if (extension == "html") {
      mimeType = "text/html";
    } else if (extension == "png") {
      mimeType = "image/png";
    } else if (extension == "jpg" || extension == "jpeg") {
      mimeType = "image/jpeg";
    } else if (extension == "gif") {
      mimeType = "image/gif";
    } else if (extension == "ico") {
      mimeType = "image/x-icon";
    } else if (extension == "svg") {
      mimeType = "image/svg+xml";
    } else if (extension == "pdf") {
      mimeType = "application/pdf";
    } else if (extension == "json") {
      mimeType = "application/json";
    } else {
      mimeType = "application/octet-stream";
    }

    // Create the response object
    auto response = createResponse(Status::CODE_200, file);
    response->putHeader(oatpp::web::protocol::http::Header::CONTENT_TYPE, mimeType);
    return response;

    // // return createResponse(Status::CODE_200, file);
    // auto response = createResponse(Status::CODE_200, file);
    // // Set the MIME type in the response headers
    // response->putHeader(oatpp::web::protocol::http::Header::CONTENT_TYPE, "application/javascript");
    // return response;
  }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
