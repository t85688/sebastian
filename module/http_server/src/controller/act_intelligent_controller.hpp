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
#include "act_intelligent_request.hpp"
#include "act_status.hpp"
#include "dto/act_intelligent_dto.hpp"
#include "dto/act_status_dto.hpp"
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
class ActIntelligentController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  OATPP_COMPONENT(std::shared_ptr<StaticQuestionnaireTemplateFilesManager>, staticQuestionnaireTemplateFilesManager);

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActIntelligentController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActIntelligentController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {}

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActIntelligentController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                                objectMapper)) {
    return std::make_shared<ActIntelligentController>(objectMapper);
  }

 public:
  // TODO Insert Your endpoints here !!!

  ENDPOINT_INFO(IntelligentReport) {
    info->summary = "Report user feedback of intelligent response";
    info->addSecurityRequirement("my-realm");
    info->addTag("Intelligent");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActIntelligentReportDto>>("application/json");
    info->addResponse<Object<ActIntelligentResponseDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActIntelligentResponseDto>>(Status::CODE_400, "application/json");

    info->pathParams["projectId"].description = "The identifier of the project";
  }
  ENDPOINT("POST", QString("%1/project/{projectId}/intelligent/report").arg(ACT_API_PATH_PREFIX).toStdString(),
           IntelligentReport, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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
    qDebug() << "projectId:" << *projectId;

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActIntelligentReport report;
    try {
      report.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    ActIntelligentResponse response;

    act_status = act::core::g_core.IntelligentReport(report, response);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << "Report intelligent response feedback failed, Response:"
                 << response.ToString().toStdString().c_str();
    }

    return createResponse(TransferActStatusToOatppStatus(static_cast<ActStatusType>(response.Getstatus().Getcode())),
                          response.ToString().toStdString());
  }

  ENDPOINT_INFO(GetIntelligentHistory) {
    info->summary = "Load user chat history";
    info->addSecurityRequirement("my-realm");
    info->addTag("Intelligent");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActIntelligentHistoryDto>>("application/json");
    info->addResponse<Object<ActIntelligentHistoryResponseDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActIntelligentHistoryResponseDto>>(Status::CODE_400, "application/json");

    info->pathParams["projectId"].description = "The identifier of the project";
  }
  ENDPOINT("POST", QString("%1/project/{projectId}/intelligent/history").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetIntelligentHistory, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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
    qDebug() << "projectId:" << *projectId;

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActIntelligentHistory history;
    try {
      history.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    ActIntelligentHistoryResponse response;
    history.SetProjectId(QString::number(project_id));

    act_status = act::core::g_core.GetIntelligentHistory(history, response);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << "Report intelligent response feedback failed, Response:"
                 << response.ToString().toStdString().c_str();
    }

    return createResponse(TransferActStatusToOatppStatus(static_cast<ActStatusType>(response.Getstatus().Getcode())),
                          response.ToString().toStdString());
  }

  ENDPOINT_INFO(IntelligentUploadFile) {
    info->summary = "Intelligent user upload file";
    info->addSecurityRequirement("my-realm");
    info->addTag("Intelligent");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActIntelligentUploadFileDto>>("application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_204, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("PUT", QString("%1/project/intelligent/upload-file").arg(ACT_API_PATH_PREFIX).toStdString(),
           IntelligentUploadFile, REQUEST(std::shared_ptr<IncomingRequest>, request),
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
    ActIntelligentUploadFile upload_file;
    try {
      upload_file.FromString(str);
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

    act_status = act::core::g_core.IntelligentUploadFile(upload_file);
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

  ENDPOINT_INFO(GetQuestionnaireTemplate) {
    info->summary = "Get questionnaire template";
    info->addSecurityRequirement("my-realm");
    info->addTag("Intelligent");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addResponse<oatpp::swagger::Binary>(Status::CODE_200,
                                              "application/octet-stream");  // tell swagger that it's a file
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->pathParams["projectId"].description = "The identifier of the project";
    info->pathParams["filename"].description = "The template name of the downloaded file";
  }
  ADD_CORS(GetQuestionnaireTemplate, "*", "GET")
  ENDPOINT("GET",
           QString("%1/project/{projectId}/intelligent/download-questionnaire-template/{filename}")
               .arg(ACT_API_PATH_PREFIX)
               .toStdString(),
           GetQuestionnaireTemplate, PATH(UInt64, projectId), PATH(String, filename),
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
    qDebug() << "projectId:" << *projectId;
    qDebug() << "filename:" << QString::fromStdString(*filename);

    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    ACT_STATUS_INIT();

    qint64 project_id = *projectId;
    QString file_name = QString::fromStdString(*filename);

    // Get project by project id
    // ActProject project;
    //
    // QMutexLocker core_lock(&act::core::g_core.mutex_);
    // act_status = act::core::g_core.GetProject(project_id, project);
    //

    // if (!IsActStatusSuccess(act_status)) {
    //   qCritical() << "project id" << QString::number(project_id) << "not found";
    //   qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
    //   return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
    //                         act_status->ToString(act_status->key_order_).toStdString());
    // }

    // // Fetch the device icon name in the device profile
    // QString file_name = QString("%1_questionnaire_template.xlsx").arg(project.GetProjectName());

    // Get the icon from configuration folder
    auto file = staticQuestionnaireTemplateFilesManager->GetFile(file_name.toStdString().c_str());
    // qDebug() << "Response Success(200)";
    auto response = createResponse(Status::CODE_200, file);

    return response;
  }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
