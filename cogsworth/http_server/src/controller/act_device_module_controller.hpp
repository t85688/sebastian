/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <QImage>
#include <iostream>
#include <string>
#include <unordered_map>

#include "../http_utils.h"
#include "act_core.hpp"
#include "act_ethernet_module.hpp"
#include "act_status.hpp"
#include "dto/act_ethernet_module_dto.hpp"
#include "dto/act_power_module_dto.hpp"
#include "dto/act_sfp_module_dto.hpp"
#include "dto/act_status_dto.hpp"
#include "oatpp-swagger/Types.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/data/stream/FileStream.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/encoding/Base64.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/protocol/http/outgoing/StreamingBody.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class ActDeviceModuleController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActDeviceModuleController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActDeviceModuleController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {
    // setDefaultAuthorizationHandler(
    //     std::make_shared<oatpp::web::server::handler::BearerAuthorizationHandler>("my-realm"));  // JWT
  }

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActDeviceModuleController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                                 objectMapper)) {
    return std::make_shared<ActDeviceModuleController>(objectMapper);
  }

 public:
  // TODO Insert Your endpoints here !!!

  /*********************
   *  Power Module  *
   * *******************/
  ENDPOINT_INFO(GetPowerModules) {
    info->summary = "Get all power module configurations";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Module");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActPowerModulesDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/power-modules").arg(ACT_API_PATH_PREFIX).toStdString(), GetPowerModules,
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

    // Dump all power modules
    return createResponse(Status::CODE_200, act::core::g_core.ToString("PowerModuleMap").toStdString());
  }

  /*********************
   *  Ethernet Module  *
   * *******************/
  ENDPOINT_INFO(GetEthernetModules) {
    info->summary = "Get all ethernet module configurations";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Module");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActEthernetModulesDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/ethernet-modules").arg(ACT_API_PATH_PREFIX).toStdString(), GetEthernetModules,
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

    // Dump all ethernet modules
    return createResponse(Status::CODE_200, act::core::g_core.ToString("EthernetModuleMap").toStdString());
  }

  /*********************
   *  SFP Module  *
   * *******************/
  ENDPOINT_INFO(GetSFPModules) {
    info->summary = "Get all SFP module configurations";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Module");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActSFPModulesDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/sfp-modules").arg(ACT_API_PATH_PREFIX).toStdString(), GetSFPModules,
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

    // Dump all sfp modules
    return createResponse(Status::CODE_200, act::core::g_core.ToString("SFPModuleMap").toStdString());
  }

  ENDPOINT_INFO(GetSFPCounts) {
    info->summary = "Get project SFP counts";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Module");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActSFPCountsDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/project/{projectId}/sfp-counts").arg(ACT_API_PATH_PREFIX).toStdString(), GetSFPCounts,
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

    ACT_STATUS_INIT();

    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActProject project;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetProject(project_id, project);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }
    return createResponse(Status::CODE_200, project.ToString("SFPCounts").toStdString());
  }

  ENDPOINT_INFO(UpdateSFPCounts) {
    info->summary = "Update all SFP counts";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Module");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActSFPCountsDto>>("application/json");
    info->addResponse(Status::CODE_204, "The SFP counts were successfully updated");

    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->pathParams["projectId"].description = "The identifier of the project";
  }

  ENDPOINT("PUT", QString("%1/project/{projectId}/sfp-counts").arg(ACT_API_PATH_PREFIX).toStdString(), UpdateSFPCounts,
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActSFPCounts act_sfp_counts;
    try {
      act_sfp_counts.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateSFPCounts(project_id, act_sfp_counts.GetSFPCounts());
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

  ENDPOINT_INFO(PatchSFPCounts) {
    info->summary = "Update the SFP counts";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Module");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActSFPCountsDto>>("application/json");
    info->addResponse(Status::CODE_204, "The SFP counts were successfully updated");

    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->pathParams["projectId"].description = "The identifier of the project";
  }

  ENDPOINT("PATCH", QString("%1/project/{projectId}/sfp-counts").arg(ACT_API_PATH_PREFIX).toStdString(), PatchSFPCounts,
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
    ActSFPCounts act_sfp_counts;
    try {
      act_sfp_counts.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.PatchSFPCounts(project_id, act_sfp_counts.GetSFPCounts());

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

  ENDPOINT_INFO(ClearSFPCounts) {
    info->summary = "Clear the SFP counts";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Module");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addResponse(Status::CODE_204, "The SFP counts were successfully cleared");

    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->pathParams["projectId"].description = "The identifier of the project";
  }

  ENDPOINT("PUT", QString("%1/project/{projectId}/clear-sfp-counts").arg(ACT_API_PATH_PREFIX).toStdString(),
           ClearSFPCounts, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.ClearSFPCounts(project_id);
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
