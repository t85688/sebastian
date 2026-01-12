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
#include "act_link.hpp"
#include "act_status.hpp"
#include "dto/act_link_dto.hpp"
#include "dto/act_status_dto.hpp"
#include "dto/act_topology_dto.hpp"
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
class ActLinkController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActLinkController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActLinkController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {
    setDefaultAuthorizationHandler(
        std::make_shared<oatpp::web::server::handler::BearerAuthorizationHandler>("my-realm"));  // JWT
  }

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActLinkController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
    return std::make_shared<ActLinkController>(objectMapper);
  }

 public:
  // TODO Insert Your endpoints here !!!
  ENDPOINT_INFO(GetLinks) {
    info->summary = "Get all link configurations";
    info->addSecurityRequirement("my-realm");
    info->addTag("Link");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActLinksDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
  }
  ENDPOINT("GET", QString("%1/project/{projectId}/links").arg(ACT_API_PATH_PREFIX).toStdString(), GetLinks,
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
    qDebug() << "GET URL:" << routes.c_str();

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    // Handle request
    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    ActProject project;
    qint64 id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GetProject(id, project, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, project.ToString("Links").toStdString());
  }

  ENDPOINT_INFO(CreateLink) {
    info->summary = "Create a new link";
    info->addSecurityRequirement("my-realm");
    info->addTag("Link");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActCreateLinkDto>>("application/json");
    // .addExample("example_1", ActLinkDto::createShared(0, "aaa", "bbb", "Supervisor"));
    info->addResponse<Object<ActLinkDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActDuplicatedDto>>(Status::CODE_409, "application/json", "The link already exist");

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
  }
  ENDPOINT("POST", QString("%1/project/{projectId}/link").arg(ACT_API_PATH_PREFIX).toStdString(), CreateLink,
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
    qDebug() << "POST URL:" << routes.c_str();

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActLink link;
    try {
      link.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.CreateLink(project_id, link, is_operation);
    if (IsActStatusSuccess(act_status)) {
      act::core::g_core.CommitTransaction(project_id);
    } else {
      act::core::g_core.StopTransaction(project_id);
    }

    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Created(201)";
    return createResponse(Status::CODE_201, link.ToString().toStdString());
  }

  ENDPOINT_INFO(CreateLinks) {
    info->summary = "Create multiple new links";
    info->addSecurityRequirement("my-realm");
    info->addTag("Link");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActCreateLinksDto>>("application/json");
    // .addExample("example_1", ActLinkDto::createShared(0, "aaa", "bbb", "Supervisor"));
    info->addResponse<Object<ActLinkIdsDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActDuplicatedDto>>(Status::CODE_409, "application/json", "The link already exist");

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
  }
  ENDPOINT("POST", QString("%1/project/{projectId}/links").arg(ACT_API_PATH_PREFIX).toStdString(), CreateLinks,
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
    qDebug() << "POST URL:" << routes.c_str();

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActLinkList link_set;
    try {
      link_set.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 project_id = *projectId;
    QList<qint64> created_link_ids;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.CreateLinks(project_id, link_set.GetLinks(), created_link_ids, is_operation);
    if (IsActStatusSuccess(act_status)) {
      act::core::g_core.CommitTransaction(project_id);
    } else {
      act::core::g_core.StopTransaction(project_id);
    }

    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    act_status = std::make_shared<ActBatchCreateLinkResponse>(created_link_ids);
    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, act_status->ToString(act_status->key_order_).toStdString());
  }

  ENDPOINT_INFO(GetLink) {
    info->summary = "Get an existing link configuration";
    info->addSecurityRequirement("my-realm");
    info->addTag("Link");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActLinkDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
    info->pathParams["linkId"].description = "The identifier of the link";
  }
  ENDPOINT("GET", QString("%1/project/{projectId}/link/{linkId}").arg(ACT_API_PATH_PREFIX).toStdString(), GetLink,
           PATH(UInt64, projectId), PATH(UInt64, linkId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    // Handle request
    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    ActLink link;
    qint64 project_id = *projectId;
    qint64 link_id = *linkId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GetLink(project_id, link_id, link, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, link.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateLink) {
    info->summary = "Update a link configuration";
    info->addSecurityRequirement("my-realm");
    info->addTag("Link");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActLinkDto>>("application/json");
    info->addResponse(Status::CODE_204, "The project was successfully deleted (or did not exist)");

    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
  }
  ENDPOINT("PUT", QString("%1/project/{projectId}/link").arg(ACT_API_PATH_PREFIX).toStdString(), UpdateLink,
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

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActLink link;
    try {
      link.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateLink(project_id, link, is_operation);
    if (IsActStatusSuccess(act_status)) {
      act::core::g_core.CommitTransaction(project_id);
    } else {
      act::core::g_core.StopTransaction(project_id);
    }

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

  ENDPOINT_INFO(PatchLink) {
    info->summary = "(Deprecated) Patch update a link configuration";
    info->addSecurityRequirement("my-realm");
    info->addTag("Link");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActLinkDto>>("application/json");
    info->addResponse<Object<ActLinkDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
  }
  ENDPOINT("PATCH", QString("%1/project/{projectId}/link").arg(ACT_API_PATH_PREFIX).toStdString(), PatchLink,
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
    qDebug() << "PATCH URL:" << routes.c_str();

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActLink tmp_link;
    try {
      tmp_link.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    ActLink link;
    qint64 project_id = *projectId;
    qint64 link_id = tmp_link.GetId();

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GetLink(project_id, link_id, link, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // Do patch parser
    link.FromString(str);

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateLink(project_id, link, is_operation);
    if (IsActStatusSuccess(act_status)) {
      act::core::g_core.CommitTransaction(project_id);
    } else {
      act::core::g_core.StopTransaction(project_id);
    }

    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, link.ToString().toStdString());
  }

  ENDPOINT_INFO(PatchLinks) {
    info->summary = "Patch update links configurations";
    info->addSecurityRequirement("my-realm");
    info->addTag("Link");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPatchLinkMapDto>>("application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
  }
  ENDPOINT("PATCH", QString("%1/project/{projectId}/links").arg(ACT_API_PATH_PREFIX).toStdString(), PatchLinks,
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
    qDebug() << "PATCH URL:" << routes.c_str();

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActPatchLinkMap request_link_map;

    try {
      request_link_map.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }
    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    // Store links getting from project and updated links
    QList<ActLink> link_list;
    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    // Start Transaction
    act_status = act::core::g_core.StartTransaction(project_id);

    // Get each item in link map
    const QMap<qint64, QString> patch_link_map = request_link_map.GetPatchLinkMap();
    for (auto link_id : patch_link_map.keys()) {
      QString patch_content = patch_link_map[link_id];
      ActLink project_link;

      // Use the link id to find the link in project
      act_status = act::core::g_core.GetLink(project_id, link_id, project_link, is_operation);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << "Response: " << act_status->ToString(act_status->key_order_).toStdString().c_str();
        return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                              act_status->ToString(act_status->key_order_).toStdString());
      }

      // 'id' -> "id"
      patch_content.replace(QString("\'"), QString("\""));

      // The link with full original data update the patch content
      project_link.FromString(patch_content);

      // Insert link into link_list
      link_list.append(project_link);
    }

    // Update links
    act_status = act::core::g_core.UpdateLinks(project_id, link_list, is_operation);
    if (IsActStatusSuccess(act_status)) {
      act::core::g_core.CommitTransaction(project_id);
    } else {
      act::core::g_core.StopTransaction(project_id);
    }

    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    return createResponse(Status::CODE_200, act_status->ToString(act_status->key_order_).toStdString());
  }

  ENDPOINT_INFO(DeleteLink) {
    info->summary = "(Deprecated) Delete a link";
    info->addSecurityRequirement("my-realm");
    info->addTag("Link");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addResponse<Object<ActStatusDto>>(
        Status::CODE_204, "application/json",
        "If you DELETE something that doesn't exist, you should just return a 204 (even if the resource never "
        "existed). The client wanted the resource gone and it is gone.");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
    info->pathParams["linkId"].description = "The identifier of the link";
  }
  ENDPOINT("DELETE", QString("%1/project/{projectId}/link/{linkId}").arg(ACT_API_PATH_PREFIX).toStdString(), DeleteLink,
           PATH(UInt64, projectId), PATH(UInt64, linkId), REQUEST(std::shared_ptr<IncomingRequest>, request),
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "DELETE URL:" << routes.c_str();

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 project_id = *projectId;
    qint64 link_id = *linkId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.DeleteLink(project_id, link_id, is_operation);
    if (IsActStatusSuccess(act_status)) {
      act::core::g_core.CommitTransaction(project_id);
    } else {
      act::core::g_core.StopTransaction(project_id);
    }

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

  ENDPOINT_INFO(DeleteLinks) {
    info->summary = "Delete links at once";
    info->addSecurityRequirement("my-realm");
    info->addTag("Link");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActLinkIdsDto>>("application/json");
    info->addResponse<Object<ActStatusDto>>(
        Status::CODE_204, "application/json",
        "If you DELETE something that doesn't exist, you should just return a 204 (even if the resource never "
        "existed). The client wanted the resource gone and it is gone.");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
  }
  ENDPOINT("DELETE", QString("%1/project/{projectId}/links").arg(ACT_API_PATH_PREFIX).toStdString(), DeleteLinks,
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
    qDebug() << "DELETE URL:" << routes.c_str();

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    LinkIds link_ids;

    try {
      link_ids.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.DeleteLinks(project_id, link_ids.GetLinkIds(), is_operation);
    if (IsActStatusSuccess(act_status)) {
      act::core::g_core.CommitTransaction(project_id);
    } else {
      act::core::g_core.StopTransaction(project_id);
    }

    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    act_status->SetStatus(ActStatusType::kNoContent);
    return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                          act_status->ToString(act_status->key_order_).toStdString());
  }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
