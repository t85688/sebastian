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
#include "deploy_entry/act_deploy_table.hpp"
#include "dto/act_device_config_dto.hpp"
#include "dto/act_status_dto.hpp"
#include "dto/act_vlan_config_dto.hpp"
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
class ActVlanConfigController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActVlanConfigController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActVlanConfigController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {
    // setDefaultAuthorizationHandler(
    //     std::make_shared<oatpp::web::server::handler::BearerAuthorizationHandler>("my-realm"));  // JWT
  }

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActVlanConfigController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                               objectMapper)) {
    return std::make_shared<ActVlanConfigController>(objectMapper);
  }

 public:
  // ENDPOINT_INFO(GetIntelligentVlanGroup) {
  //   info->summary = "Get all vlan-groups";
  //   info->addSecurityRequirement("my-realm");
  //   info->addTag("VLAN Config");
  //   info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
  //   // info->addResponse<Object<ActVlanConfigsDto>>(Status::CODE_200, "application/json");
  //   info->addResponse<Fields<Object<ActIntelligentVlanDto>>>(Status::CODE_200, "application/json");
  //   info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
  //       .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
  //       .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
  //       .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
  //       .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));

  //   info->pathParams["projectId"].description = "The identifier of the project";
  // }

  // ENDPOINT("GET", QString("%1/project/{projectId}/vlan-config/vlan-groups").arg(ACT_API_PATH_PREFIX).toStdString(),
  //          GetIntelligentVlanGroup, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
  //          AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
  // if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
  //   ActUnauthorized unauthorized;
  //   qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
  //            << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
  //   return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
  //                         unauthorized.ToString(unauthorized.key_order_).toStdString());
  // }

  //   auto routes = request->getStartingLine().path.std_str();
  //   qDebug() << "GET URL:" << routes.c_str();

  //   // Handle request
  //   ACT_STATUS_INIT();
  //   std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

  //   ActProject project;
  //   qint64 project_id = *projectId;
  //
  //   act::core::g_core.Lock();
  //
  //   act_status = act::core::g_core.GetProject(project_id, project);
  //
  //
  //
  //   if (!IsActStatusSuccess(act_status)) {
  //     qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
  //     return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
  //                           act_status->ToString(act_status->key_order_).toStdString());
  //   }

  //   // qDebug() << "Response: Success(200)";
  //   return createResponse(Status::CODE_200,
  //   project.GetTopologySetting().ToString("IntelligentVlanGroup").toStdString());
  // }

  // ENDPOINT_INFO(CreateVlanGroup) {
  //   info->summary = "Create the vlan group";
  //   info->addSecurityRequirement("my-realm");
  //   info->addTag("VLAN Config");
  //   info->description = "This RESTful API only permit for [Admin, Supervisor]";
  //   info->addConsumes<Object<ActIntelligentVlanDto>>("application/json");
  //   info->addResponse<Object<ActIntelligentVlanDto>>(Status::CODE_201, "application/json");
  //   info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
  //       .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
  //       .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
  //       .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActDuplicatedDto>>(Status::CODE_409, "application/json", "The vlan already exist");
  //   info->addResponse<Object<ActLicenseNotActiveDto>>(Status::CODE_422, "application/json")
  //       .addExample("Unprocessable Entity", ActLicenseNotActiveDto::createShared("System VLAN Setting"));

  //   info->pathParams["projectId"].description = "The identifier of the project";
  //   info->pathParams["vlanId"].description = "The identifier of the vlan";
  // }
  // ENDPOINT("POST", QString("%1/project/{projectId}/vlan-config/vlan-group").arg(ACT_API_PATH_PREFIX).toStdString(),
  //          CreateVlanGroup, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
  //          AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
  // if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
  //   ActUnauthorized unauthorized;
  //   qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
  //            << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
  //   return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
  //                         unauthorized.ToString(unauthorized.key_order_).toStdString());
  // }

  //   auto routes = request->getStartingLine().path.std_str();
  //   qDebug() << "POST URL:" << routes.c_str();

  //   ACT_STATUS_INIT();

  //   if (authorizationBearer->role == ActRoleEnum::kUser) {
  //     qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
  //     act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
  //     return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
  //   }

  //   // Dto -> ACT Class
  //   QString str = request->readBodyToString().getPtr()->c_str();
  //   ActIntelligentVlan intelligent_vlan;
  //   try {
  //     intelligent_vlan.FromString(str);
  //   } catch (std::exception &e) {
  //     ActBadRequest bad_request(e.what());
  //     qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
  //              << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
  //     return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
  //                           bad_request.ToString(bad_request.key_order_).toStdString());
  //   }

  //   // GetVlanConfig
  //   // Handle request
  //   std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

  //   qint64 project_id = *projectId;
  //
  //   act::core::g_core.Lock();
  //
  //   act::core::g_core.StartTransaction(project_id);
  //   act_status = act::core::g_core.CreateVlanGroup(project_id, intelligent_vlan);
  //   if (IsActStatusSuccess(act_status)) {
  //     act::core::g_core.CommitTransaction(project_id);
  //   } else {
  //     act::core::g_core.StopTransaction(project_id);
  //   }
  //
  //
  //   if (!IsActStatusSuccess(act_status)) {
  //     qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
  //     return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
  //                           act_status->ToString(act_status->key_order_).toStdString());
  //   }

  //   // qDebug() << "Response: Created(201)" << intelligent_vlan.ToString().toStdString().c_str();
  //   return createResponse(Status::CODE_201, intelligent_vlan.ToString().toStdString());
  // }

  // ENDPOINT_INFO(UpdateVlanGroup) {
  //   info->summary = "Update the vlan group";
  //   info->addSecurityRequirement("my-realm");
  //   info->addTag("VLAN Config");
  //   info->description = "This RESTful API only permit for [Admin, Supervisor]";
  //   info->addConsumes<Object<ActIntelligentVlanDto>>("application/json");
  //   info->addResponse<Object<ActIntelligentVlanDto>>(Status::CODE_201, "application/json");
  //   info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
  //       .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
  //       .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
  //       .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActLicenseNotActiveDto>>(Status::CODE_422, "application/json")
  //       .addExample("Unprocessable Entity", ActLicenseNotActiveDto::createShared("System VLAN Setting"));

  //   info->pathParams["projectId"].description = "The identifier of the project";
  //   info->pathParams["deviceId"].description = "The identifier of the device";
  // }

  // ENDPOINT("PUT", QString("%1/project/{projectId}/vlan-config/vlan-group").arg(ACT_API_PATH_PREFIX).toStdString(),
  //          UpdateVlanGroup, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
  //          AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
  // if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
  //   ActUnauthorized unauthorized;
  //   qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
  //            << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
  //   return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
  //                         unauthorized.ToString(unauthorized.key_order_).toStdString());
  // }

  //   auto routes = request->getStartingLine().path.std_str();
  //   qDebug() << "PUT URL:" << routes.c_str();

  //   ACT_STATUS_INIT();

  //   if (authorizationBearer->role == ActRoleEnum::kUser) {
  //     qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
  //     act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
  //     return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
  //   }

  //   // Dto -> ACT Class
  //   QString str = request->readBodyToString().getPtr()->c_str();
  //   ActIntelligentVlan intelligent_vlan;
  //   try {
  //     intelligent_vlan.FromString(str);
  //   } catch (std::exception &e) {
  //     ActBadRequest bad_request(e.what());
  //     qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
  //              << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
  //     return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
  //                           bad_request.ToString(bad_request.key_order_).toStdString());
  //   }

  //   // GetVlanConfig
  //   // Handle request
  //   std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

  //   qint64 project_id = *projectId;
  //
  //   act::core::g_core.Lock();
  //
  //   act::core::g_core.StartTransaction(project_id);
  //   act_status = act::core::g_core.UpdateVlanGroup(project_id, intelligent_vlan);
  //   if (IsActStatusSuccess(act_status)) {
  //     act::core::g_core.CommitTransaction(project_id);
  //   } else {
  //     act::core::g_core.StopTransaction(project_id);
  //   }
  //
  //
  //   if (!IsActStatusSuccess(act_status)) {
  //     qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
  //     return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
  //                           act_status->ToString(act_status->key_order_).toStdString());
  //   }

  //   // qDebug() << "Response: Created(201)";

  //   return createResponse(Status::CODE_201, intelligent_vlan.ToString().toStdString());
  // }

  // ENDPOINT_INFO(DeleteVlanGroup) {
  //   info->summary = "Delete a vlan group";
  //   info->addSecurityRequirement("my-realm");
  //   info->addTag("VLAN Config");
  //   info->description = "This RESTful API only permit for [Admin, Supervisor]";
  //   info->addResponse<Object<ActStatusDto>>(
  //       Status::CODE_204, "application/json",
  //       "If you DELETE something that doesn't exist, you should just return a 204 (even if the resource never "
  //       "existed). The client wanted the resource gone and it is gone.");
  //   info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
  //       .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
  //       .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
  //       .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActLicenseNotActiveDto>>(Status::CODE_422, "application/json")
  //       .addExample("Unprocessable Entity", ActLicenseNotActiveDto::createShared("System VLAN Setting"));

  //   info->pathParams["projectId"].description = "The identifier of the project";
  //   info->pathParams["vlanId"].description = "The identifier of the device";
  // }

  // ENDPOINT("DELETE",
  //          QString("%1/project/{projectId}/vlan-config/vlan-group/{vlanId}").arg(ACT_API_PATH_PREFIX).toStdString(),
  //          DeleteVlanGroup, PATH(UInt64, projectId), PATH(UInt16, vlanId),
  //          REQUEST(std::shared_ptr<IncomingRequest>, request),
  //          AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
  // if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
  //   ActUnauthorized unauthorized;
  //   qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
  //            << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
  //   return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
  //                         unauthorized.ToString(unauthorized.key_order_).toStdString());
  // }

  //   auto routes = request->getStartingLine().path.std_str();
  //   qDebug() << "DELETE URL:" << routes.c_str();

  //   ACT_STATUS_INIT();

  //   if (authorizationBearer->role == ActRoleEnum::kUser) {
  //     qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
  //     act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
  //     return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
  //   }

  //   // Handle request
  //   std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

  //   qint64 project_id = *projectId;
  //   quint16 vlan_id = *vlanId;
  //
  // act::core::g_core.Lock();
  //
  //   act::core::g_core.StartTransaction(project_id);
  //   act_status = act::core::g_core.DeleteVlanGroup(project_id, vlan_id);
  //   if (IsActStatusSuccess(act_status)) {
  //     act::core::g_core.CommitTransaction(project_id);
  //   } else {
  //     act::core::g_core.StopTransaction(project_id);
  //   }
  //
  //
  //   if (!IsActStatusSuccess(act_status)) {
  //     qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
  //     return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
  //                           act_status->ToString(act_status->key_order_).toStdString());
  //   }

  //   act_status->SetStatus(ActStatusType::kNoContent);
  //   // qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
  //   return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
  //                         act_status->ToString(act_status->key_order_).toStdString());
  // }

  ENDPOINT_INFO(UpdateVlanTable) {
    info->summary = "Update the vlan config";
    info->addSecurityRequirement("my-realm");
    info->addTag("VLAN Config");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActVlanTableDto>>("application/json");
    info->addResponse<Object<ActVlanTableDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->pathParams["projectId"].description = "The identifier of the project";
    info->pathParams["deviceId"].description = "The identifier of the device";
  }

  ENDPOINT("PUT", QString("%1/project/{projectId}/vlan-config/device").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateVlanTable, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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
    ActVlanTable vlan_config;
    try {
      vlan_config.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // GetVlanConfig
    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateVlanConfig(project_id, vlan_config);
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

    return createResponse(Status::CODE_201, vlan_config.ToString().toStdString());
  }

  ENDPOINT_INFO(GetVlanTables) {
    info->summary = "Get all vlan vlan-configs";
    info->addSecurityRequirement("my-realm");
    info->addTag("VLAN Config");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    // info->addResponse<Object<ActVlanConfigsDto>>(Status::CODE_200, "application/json");
    info->addResponse<Fields<Object<ActVlanTableDto>>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));

    info->pathParams["projectId"].description = "The identifier of the project";
  }

  ENDPOINT("GET", QString("%1/project/{projectId}/vlan-config/devices").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetVlanTables, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    ActProject project;
    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GetProject(project_id, project);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, project.GetDeviceConfig().ToString("VlanTables").toStdString());
  }

  ENDPOINT_INFO(GetVlanTable) {
    info->summary = "Get a vlan vlan-config";
    info->addSecurityRequirement("my-realm");
    info->addTag("VLAN Config");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActVlanTableDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));

    info->pathParams["projectId"].description = "The identifier of the project";
    info->pathParams["deviceId"].description = "The identifier of the device";
  }

  ENDPOINT("GET",
           QString("%1/project/{projectId}/vlan-config/device/{deviceId}").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetVlanTable, PATH(UInt64, projectId), PATH(UInt64, deviceId),
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

    // Handle request
    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    ActVlanTable vlan_config;
    qint64 project_id = *projectId;
    qint64 device_id = *deviceId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GetVlanConfig(project_id, device_id, vlan_config);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, vlan_config.ToString().toStdString());
  }

  ENDPOINT_INFO(DeleteVlanTable) {
    info->summary = "Delete a vlan config";
    info->addSecurityRequirement("my-realm");
    info->addTag("VLAN Config");
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

    info->pathParams["projectId"].description = "The identifier of the project";
    info->pathParams["deviceId"].description = "The identifier of the device";
  }
  ENDPOINT("DELETE",
           QString("%1/project/{projectId}/vlan-config/device/{deviceId}").arg(ACT_API_PATH_PREFIX).toStdString(),
           DeleteVlanTable, PATH(UInt64, projectId), PATH(UInt64, deviceId),
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
    qDebug() << "DELETE URL:" << routes.c_str();

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 project_id = *projectId;
    qint64 device_id = *deviceId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.DeleteVlanConfig(project_id, device_id);
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

  ENDPOINT_INFO(UpdateVlanConfig) {
    info->summary = "Update the vlan config";
    info->addSecurityRequirement("my-realm");
    info->addTag("VLAN Config");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActVlanConfigDto>>("application/json");
    info->addResponse<Object<ActVlanConfigDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->pathParams["projectId"].description = "The identifier of the project";
  }

  ENDPOINT("POST", QString("%1/project/{projectId}/vlan-config/vlan").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateVlanConfig, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActVlanConfig vlan_config;
    try {
      vlan_config.FromString(str);
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
    act_status = act::core::g_core.UpdateVlanConfig(project_id, vlan_config);
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

    return createResponse(Status::CODE_201, vlan_config.ToString().toStdString());
  }

  ENDPOINT_INFO(DeleteVlanConfig) {
    info->summary = "Delete a vlan config";
    info->addSecurityRequirement("my-realm");
    info->addTag("VLAN Config");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActVlanConfigDeleteDto>>("application/json");
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

    info->pathParams["projectId"].description = "The identifier of the project";
  }

  ENDPOINT("DELETE", QString("%1/project/{projectId}/vlan-config/vlan").arg(ACT_API_PATH_PREFIX).toStdString(),
           DeleteVlanConfig, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActVlanConfig vlan_config;
    try {
      vlan_config.FromString(str);
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
    act_status = act::core::g_core.DeleteVlanConfig(project_id, vlan_config);
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
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
