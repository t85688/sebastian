/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <iostream>
#include <string>

#include "../http_utils.h"
#include "act_core.hpp"
#include "act_status.hpp"
#include "dto/act_status_dto.hpp"
#include "dto/act_vlan_view_dto.hpp"
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
class ActVlanViewController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActVlanViewController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActVlanViewController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {
    // setDefaultAuthorizationHandler(
    //     std::make_shared<oatpp::web::server::handler::BearerAuthorizationHandler>("my-realm"));  // JWT
  }

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActVlanViewController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                             objectMapper)) {
    return std::make_shared<ActVlanViewController>(objectMapper);
  }

 public:
  ENDPOINT_INFO(GetVlanViews) {
    info->summary = "Get all vlan vlan-views";
    info->addSecurityRequirement("my-realm");
    info->addTag("VLAN View");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActVlanViewsDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/vlan-view/vlans").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetVlanViews, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    ActVlanViewTable vlan_view_table;
    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GetVlanViews(project_id, vlan_view_table, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, vlan_view_table.ToString("VlanViews").toStdString());
  }

  ENDPOINT_INFO(GetVlanView) {
    info->summary = "Get a vlan vlan-view";
    info->addSecurityRequirement("my-realm");
    info->addTag("VLAN View");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActVlanViewDto>>(Status::CODE_200, "application/json");
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
    info->pathParams["vlanId"].description = "The identifier of the vlan";
  }

  ENDPOINT("GET", QString("%1/project/{projectId}/vlan-view/vlan/{vlanId}").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetVlanView, PATH(UInt64, projectId), PATH(Int32, vlanId),
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

    ActVlanView vlan_view;
    qint64 project_id = *projectId;
    qint32 vlan_id = *vlanId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GetVlanView(project_id, vlan_id, vlan_view, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, vlan_view.ToString().toStdString());
  }

  ENDPOINT_INFO(GetVlanViewIds) {
    info->summary = "Get a vlan-view vlan-id list";
    info->addSecurityRequirement("my-realm");
    info->addTag("VLAN View");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActVlanViewIdsDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/vlan-view/vlan-ids").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetVlanViewIds, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    ActVlanViewIds vlan_view_ids;
    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GetVlanViewIds(project_id, vlan_view_ids, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, vlan_view_ids.ToString().toStdString());
  }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
