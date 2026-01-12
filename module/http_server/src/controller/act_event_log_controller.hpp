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
#include "act_event_log.hpp"
#include "act_status.hpp"
#include "dto/act_event_log_dto.hpp"
#include "dto/act_status_dto.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include OATPP_CODEGEN_BEGIN(ApiController)  //<-- Begin Codegen

/**
 * Event Log Api Controller.
 */
class ActEventLogController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActEventLogController __ControllerType;

 public:
  /**
   * @brief Construct a new Event Log Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActEventLogController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {}

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActEventLogController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                             objectMapper)) {
    return std::make_shared<ActEventLogController>(objectMapper);
  }

 public:
  ENDPOINT_INFO(GetEventLogs) {
    info->summary = "Get event logs";
    info->addSecurityRequirement("my-realm");
    info->addTag("EventLog");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActEventLogsDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));

    info->queryParams["limit"].description = "Limit the number of result.";
    info->queryParams["offset"].description = "Log starting from the offset.";
  }
  ENDPOINT("GET", QString("%1/event-logs").arg(ACT_API_PATH_PREFIX).toStdString(), GetEventLogs, QUERY(Int32, limit),
           QUERY(Int32, offset), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    qint32 limit_ = *limit;
    qint32 offset_ = *offset;
    ActEventLogsResponse response;
    act_status = act::core::g_core.GetEventLogs(limit_, offset_, response);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << "Get event logs from MAF failed, Response:" << response.ToString().toStdString().c_str();
    }

    return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()), response.ToString().toStdString());
  }

  ENDPOINT_INFO(GetSyslogs) {
    info->summary = "Get syslogs";
    info->addSecurityRequirement("my-realm");
    info->addTag("EventLog");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActSyslogsDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));

    info->queryParams["limit"].description = "Limit the number of result.";
    info->queryParams["offset"].description = "Log starting from the offset.";
    info->queryParams["severities"].description =
        std::string("Filter logs by one or multiple severity levels (comma-separated). ") +
        std::string("A value of \"-1\" indicates that filtering for severity will not be applied.");
    info->queryParams["facilities"].description =
        std::string("Filter logs by one or multiple facility codes (comma-separated). ") +
        std::string("A value of \"-1\" indicates that filtering for facility will not be applied.");
    info->queryParams["starttime"].description =
        "Filter logs by start time. \"-1\" means filtering for starttime will be skipped.";
    info->queryParams["endtime"].description =
        "Filter logs by end time. \"-1\" means filtering for endtime will be skipped.";
    info->queryParams["ipaddress"].description =
        std::string("Filter logs by one or multiple IP addresses (comma-separated). ") +
        std::string("\"-1\" means filtering for ipaddress will be skipped.");
  }
  ENDPOINT("GET", QString("%1/syslogs").arg(ACT_API_PATH_PREFIX).toStdString(), GetSyslogs, QUERY(Int32, limit),
           QUERY(Int32, offset), QUERY(String, severities), QUERY(String, facilities), QUERY(String, starttime),
           QUERY(String, endtime), QUERY(String, ipaddress), REQUEST(std::shared_ptr<IncomingRequest>, request),
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
    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActSyslogQueryData query_data(*limit, *offset);
    QString severities_str = QString::fromStdString(*severities);
    query_data.SetSeverities(severities_str);
    QString facilities_str = QString::fromStdString(*facilities);
    query_data.SetFacilities(facilities_str);
    QString starttime_str = QString::fromStdString(*starttime);
    query_data.SetStarttime(starttime_str);
    QString endtime_str = QString::fromStdString(*endtime);
    query_data.SetEndtime(endtime_str);
    QString ipaddress_str = QString::fromStdString(*ipaddress);
    query_data.SetIpaddress(ipaddress_str);

    ActSyslogsResponse response;

    act_status = act::core::g_core.GetSyslogs(query_data, response);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << "Get syslogs from MAF failed, Response:" << response.ToString().toStdString().c_str();
    }

    return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()), response.ToString().toStdString());
  }

  ENDPOINT_INFO(SaveSyslogs) {
    info->summary = "Save syslogs as csv file";
    info->addSecurityRequirement("my-realm");
    info->addTag("EventLog");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addConsumes<Object<ActCsvPathDto>>("application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));

    info->queryParams["limit"].description = "Limit the number of result.";
    info->queryParams["offset"].description = "Log starting from the offset.";
    info->queryParams["severities"].description =
        std::string("Filter logs by one or multiple severity levels (comma-separated). ") +
        std::string("A value of -1 indicates that filtering for severity will not be applied.");
    info->queryParams["facilities"].description =
        std::string("Filter logs by one or multiple facility codes (comma-separated). ") +
        std::string("A value of -1 indicates that filtering for facility will not be applied.");
    info->queryParams["starttime"].description =
        "Filter logs by start time. \"-1\" means filtering for starttime will be skipped.";
    info->queryParams["endtime"].description =
        "Filter logs by end time. \"-1\" means filtering for endtime will be skipped.";
    info->queryParams["ipaddress"].description =
        std::string("Filter logs by one or multiple IP addresses (comma-separated). ") +
        std::string("\"-1\" means filtering for ipaddress will be skipped.");
  }
  ENDPOINT("POST", QString("%1/save-syslogs").arg(ACT_API_PATH_PREFIX).toStdString(), SaveSyslogs, QUERY(Int32, limit),
           QUERY(Int32, offset), QUERY(String, severities), QUERY(String, facilities), QUERY(String, starttime),
           QUERY(String, endtime), QUERY(String, ipaddress), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActCsvFilepath filepath;
    try {
      filepath.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    ActSyslogQueryData query_data(*limit, *offset);
    QString severities_str = QString::fromStdString(*severities);
    query_data.SetSeverities(severities_str);
    QString facilities_str = QString::fromStdString(*facilities);
    query_data.SetFacilities(facilities_str);
    QString starttime_str = QString::fromStdString(*starttime);
    query_data.SetStarttime(starttime_str);
    QString endtime_str = QString::fromStdString(*endtime);
    query_data.SetEndtime(endtime_str);
    QString ipaddress_str = QString::fromStdString(*ipaddress);
    query_data.SetIpaddress(ipaddress_str);

    act_status = act::core::g_core.SaveSyslogsAsCsv(query_data, filepath.Getfilepath());
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << "Save syslogs as csv file failed, filepath is:" << filepath.ToString().toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    return createResponse(Status::CODE_200, "generate csv file successfully.");
  }

  ENDPOINT_INFO(DeleteSyslogs) {
    info->summary = "Delete all syslogs";
    info->addSecurityRequirement("my-realm");
    info->addTag("EventLog");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActStatusDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("DELETE", QString("%1/syslogs").arg(ACT_API_PATH_PREFIX).toStdString(), DeleteSyslogs,
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    ActDeleteSyslogsResponse response;
    act_status = act::core::g_core.DeleteSyslogs(response);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << "Delete syslogs from MAF failed, Response:" << response.ToString().toStdString().c_str();
    }

    return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()), response.ToString().toStdString());
  }

  ENDPOINT_INFO(GetSyslogConfig) {
    info->summary = "Get syslog server configuration";
    info->addSecurityRequirement("my-realm");
    info->addTag("EventLog");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetSyslogConfigurationDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/syslog-configuration").arg(ACT_API_PATH_PREFIX).toStdString(), GetSyslogConfig,
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    ActSyslogGetConfiguration response;
    act_status = act::core::g_core.GetSyslogConfiguration(response);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << "Get syslog configuration from MAF failed, Response:" << response.ToString().toStdString().c_str();
    }

    return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()), response.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateSyslogConfig) {
    info->summary = "Update syslog server configuration";
    info->addSecurityRequirement("my-realm");
    info->addTag("EventLog");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addConsumes<Object<ActUpdateSyslogConfigurationDto>>("application/json");
    info->addResponse<Object<ActUpdateSyslogConfigurationDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("PUT", QString("%1/syslog-configuration").arg(ACT_API_PATH_PREFIX).toStdString(), UpdateSyslogConfig,
           REQUEST(std::shared_ptr<IncomingRequest>, request),
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

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
    ActSyslogPutConfiguration syslog_config;
    try {
      syslog_config.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    ActSyslogPutConfiguration response;
    act_status = act::core::g_core.SetSyslogConfiguration(syslog_config, response);
    if (!IsActStatusSuccess(act_status)) {
      qWarning() << "Get syslog configuration from MAF failed, Response:" << response.ToString().toStdString().c_str();
    }

    return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()), response.ToString().toStdString());
  }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
