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
#include "act_db.hpp"
#include "act_status.hpp"
#include "act_topology.hpp"
#include "dto/act_status_dto.hpp"
#include "dto/act_topology_dto.hpp"
#include "oatpp/core/Types.hpp"

// #include "oatpp/core/concurrency/SpinLock.hpp"
// #include "oatpp/core/data/stream/FileStream.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/protocol/http/outgoing/StreamingBody.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include OATPP_CODEGEN_BEGIN(ApiController)  //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class ActTopologyController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  OATPP_COMPONENT(std::shared_ptr<StaticTopologyIconFilesManager>, staticTopologyIconFilesManager);

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActTopologyController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActTopologyController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {
    // setDefaultAuthorizationHandler(
    //     std::make_shared<oatpp::web::server::handler::BearerAuthorizationHandler>("my-realm"));  // JWT
  }

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActTopologyController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                             objectMapper)) {
    return std::make_shared<ActTopologyController>(objectMapper);
  }

 public:
  // TODO Insert Your endpoints here !!!
  ENDPOINT_INFO(GetSimpleTopologies) {
    info->summary = "Get all topology configurations in simple format";
    info->addSecurityRequirement("my-realm");
    info->addTag("Topology");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActSimpleTopologiesDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/simple-topologies").arg(ACT_API_PATH_PREFIX).toStdString(), GetSimpleTopologies,
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

    // Turn topology into simple topology
    QSet<ActTopology> &topologies = act::core::g_core.GetTopologySet();
    QSet<ActSimpleTopology> simple_topologies;

    for (auto topology : topologies) {
      ActSimpleTopology simple_topology(topology);
      simple_topologies.insert(simple_topology);
    }

    // Response
    ActSimpleTopologySet reply;
    reply.SetSimpleTopologySet(simple_topologies);

    return createResponse(Status::CODE_200, reply.ToString().toStdString());
  }

  ENDPOINT_INFO(GetTopology) {
    info->summary = "Get the topology configuration";
    info->addSecurityRequirement("my-realm");
    info->addTag("Topology");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActTopologyDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));

    info->pathParams["topologyId"].description = "The identifier of the topology";
  }
  ENDPOINT("GET", QString("%1/topology/{topologyId}").arg(ACT_API_PATH_PREFIX).toStdString(), GetTopology,
           PATH(UInt64, topologyId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    ActTopology topology;
    qint64 id = *topologyId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GetTopology(id, topology);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, topology.ToString().toStdString());
  }

  ENDPOINT_INFO(SaveTopology) {
    info->summary = "Save topology configuration";
    info->addSecurityRequirement("my-realm");
    info->addTag("Topology");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActCreateTopologyDto>>("application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActBadRequestDto::createShared(
                                       "\"admin\" is not valid string which should be: [Admin, Supervisor, Guest]"));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("POST", QString("%1/topology").arg(ACT_API_PATH_PREFIX).toStdString(), SaveTopology,
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
    qDebug() << "POST URL:" << routes.c_str();

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActTopologyCreateParam topology_param;
    try {
      topology_param.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.SaveTopology(topology_param);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, act_status->ToString(act_status->key_order_).toStdString());
  }

  ENDPOINT_INFO(AppendTopology) {
    info->summary = "Append topology configuration to specific project";
    info->addSecurityRequirement("my-realm");
    info->addTag("Topology");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActTopologyAppendDto>>("application/json");
    info->addResponse<Object<ActTopologyDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActBadRequestDto::createShared(
                                       "\"admin\" is not valid string which should be: [Admin, Supervisor, Guest]"));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->pathParams["projectId"].description = "Project Identifier";
  }
  ENDPOINT("POST", QString("%1/topology/append/project/{projectId}").arg(ACT_API_PATH_PREFIX).toStdString(),
           AppendTopology, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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
    ActTopology topology_cfg;
    try {
      topology_cfg.FromString(str);
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
    act_status = act::core::g_core.AppendTopology(project_id, topology_cfg);
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
    return createResponse(Status::CODE_200, act_status->ToString(act_status->key_order_).toStdString());
  }

  ENDPOINT_INFO(UpdateTopology) {
    info->summary = "Update the topology configuration";
    info->addSecurityRequirement("my-realm");
    info->addTag("Topology");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActTopologyDto>>("application/json");
    info->addResponse(Status::CODE_204, "The project was successfully deleted (or did not exist)");

    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("PUT", QString("%1/topology").arg(ACT_API_PATH_PREFIX).toStdString(), UpdateTopology,
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
    qDebug() << "PUT URL:" << routes.c_str();

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActTopology topology;
    try {
      topology.FromString(str);
    } catch (std::exception &e) {
      // auto status_dto = ActBadRequestDto::createShared();
      // status_dto->message = e.what();
      // status_dto->status = StatusDtoEnum::kBadRequest;
      // status_dto->severity = SeverityDtoEnum::kCritical;
      // DUMP_DTO(400, status_dto);
      // return createDtoResponse(Status::CODE_400, status_dto);
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.UpdateTopology(topology);
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

  ENDPOINT_INFO(PatchTopology) {
    info->summary = "Patch update the topology configuration";
    info->addSecurityRequirement("my-realm");
    info->addTag("Topology");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActTopologyDto>>("application/json");
    info->addResponse(Status::CODE_204, "The project was successfully deleted (or did not exist)");

    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("PATCH", QString("%1/topology").arg(ACT_API_PATH_PREFIX).toStdString(), PatchTopology,
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
    qDebug() << "PATCH URL:" << routes.c_str();

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActTopology tmp_topology;
    try {
      tmp_topology.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    ActTopology topology;
    qint64 id = tmp_topology.GetId();

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GetTopology(id, topology);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // Do patch parser
    topology.FromString(str);

    act_status = act::core::g_core.UpdateTopology(topology);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, topology.ToString().toStdString());
  }

  ENDPOINT_INFO(DeleteTopology) {
    info->summary = "Delete the topology from topology manager";
    info->addSecurityRequirement("my-realm");
    info->addTag("Topology");
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

    info->pathParams["topologyId"].description = "The identifier of the topology";
  }
  ENDPOINT("DELETE", QString("%1/topology/{topologyId}").arg(ACT_API_PATH_PREFIX).toStdString(), DeleteTopology,
           PATH(UInt64, topologyId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Delete device profile & icon from configuration folder
    qint64 id = *topologyId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.DeleteTopology(id);
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

  ENDPOINT_INFO(GetTopologyIcon) {
    info->summary = "Get an existing topology icon, the \"topologyIconId\" is equal to \"topologyId\"";
    info->addSecurityRequirement("my-realm");
    info->addTag("Topology");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<oatpp::swagger::Binary>(Status::CODE_200,
                                              "application/octet-stream");  // tell swagger that it's a file
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->pathParams["deviceProfileId"].description = "The identifier of the device icon";
  }
  ADD_CORS(GetTopologyIcon, "*", "GET, POST, DELETE, PUT")
  ENDPOINT("GET", QString("%1/topology-icon/{topologyId}").arg(ACT_API_PATH_PREFIX).toStdString(), GetTopologyIcon,
           PATH(UInt64, topologyId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    ACT_STATUS_INIT();

    qint64 topology_id = *topologyId;
    // QString filename(ACT_TOPOLOGY_ICON_DB_FOLDER);
    // filename.append("/");
    // filename.append(QString::number(topology_id));
    // filename.append(".PNG");
    QString filename(QString::number(topology_id));
    filename.append(".PNG");

    // Get the icon from configuration folder
    auto file = staticTopologyIconFilesManager->GetFile(filename.toStdString().c_str());
    // qDebug() << "Response Success(200)";
    auto response = createResponse(Status::CODE_200, file);

    return response;
  }

  ENDPOINT_INFO(CopyTopology) {
    info->summary = "Copy select devices & links";
    info->addSecurityRequirement("my-realm");
    info->addTag("Topology");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActDeviceIdsDto>>("application/json");
    info->addResponse<Object<ActBatchCreatedTopologyResponseDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActLicenseSizeFailedRequestDto>>(Status::CODE_422, "application/json")
        .addExample("Unprocessable Entity",
                    ActLicenseSizeFailedRequestDto::createShared("Device Size", ACT_EVALUATION_DEVICE_QTY));

    info->pathParams["projectId"].description = "The identifier of the project";
  }

  ENDPOINT("POST", QString("%1/topology/copy/project/{projectId}/").arg(ACT_API_PATH_PREFIX).toStdString(),
           CopyTopology, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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
    DevIds dev_ids;
    try {
      dev_ids.FromString(str);
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

    QList<qint64> copied_dev_ids;
    QList<qint64> copied_link_ids;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.CopyTopology(project_id, dev_ids.GetDeviceIds(), copied_dev_ids, copied_link_ids);
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

    act_status = std::make_shared<ActCopyTopologyResponse>(copied_dev_ids, copied_link_ids);
    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, act_status->ToString(act_status->key_order_).toStdString());
  }

  ENDPOINT_INFO(CheckTopologyLoop) {
    info->summary = "Check if topology is loop";
    info->addSecurityRequirement("my-realm");
    info->addTag("Topology");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addResponse<Object<ActTopologyStatusDto>>(Status::CODE_200, "application/json")
        .addExample("OK", ActTopologyStatusDto::createShared(true));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->pathParams["projectId"].description = "Project Identifier";
  }

  ENDPOINT("GET", QString("%1/topology/project/{projectId}/loop").arg(ACT_API_PATH_PREFIX).toStdString(),
           CheckTopologyLoop, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActTopologyStatus topology_status;
    act_status = act::core::g_core.CheckTopologyLoop(project_id, topology_status);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, topology_status.ToString().toStdString());
  }

  ENDPOINT_INFO(CreateTopologyTemplate) {
    info->summary = "Create a line/ring/star pattern in topology";
    info->addSecurityRequirement("my-realm");
    info->addTag("Topology");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActTopologyTemplateDto>>("application/json");
    info->addResponse<Object<ActBatchCreatedTopologyResponseDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActDuplicatedDto>>(Status::CODE_409, "application/json", "The link already exist");

    info->pathParams["projectId"].description = "The identifier of the project";
  }
  ENDPOINT("POST", QString("%1/project/{projectId}/topology-template").arg(ACT_API_PATH_PREFIX).toStdString(),
           CreateTopologyTemplate, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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
    ActTopologyTemplate topology_template;
    try {
      topology_template.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QList<qint64> copied_dev_ids;
    QList<qint64> copied_link_ids;
    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act::core::g_core.StartTransaction(project_id);
    act_status =
        act::core::g_core.CreateTopologyTemplate(project_id, topology_template, copied_dev_ids, copied_link_ids);
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

    act_status = std::make_shared<ActCopyTopologyResponse>(copied_dev_ids, copied_link_ids);
    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, act_status->ToString(act_status->key_order_).toStdString());
  }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
