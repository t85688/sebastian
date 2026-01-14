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
#include "act_project.hpp"
#include "act_status.hpp"
#include "dto/act_device_setting_dto.hpp"
#include "dto/act_project_dto.hpp"
#include "dto/act_status_dto.hpp"
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
class ActDeviceSettingController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActDeviceSettingController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActDeviceSettingController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {}

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActDeviceSettingController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                                  objectMapper)) {
    return std::make_shared<ActDeviceSettingController>(objectMapper);
  }

 public:
  // TODO Insert Your endpoints here !!!

  ENDPOINT_INFO(GetDeviceInformations) {
    info->summary = "Get all device information";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetDeviceInformationsDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/device-informations").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDeviceInformations, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDeviceInformationList device_information_list;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetDeviceInformations(project_id, device_information_list, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, device_information_list.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateDeviceInformation) {
    info->summary = "Update the device information";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceInformationsDto>>("application/json");
    info->addResponse(Status::CODE_204, "The device information was successfully updated");

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

  ENDPOINT("PUT", QString("%1/project/{projectId}/device-informations").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateDeviceInformation, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDeviceInformationList device_information_list;
    try {
      device_information_list.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceInformations(project_id, device_information_list, is_operation);
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

  ENDPOINT_INFO(PatchDeviceInformation) {
    info->summary = "Patch the device information";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceInformationsDto>>("application/json");
    info->addResponse(Status::CODE_204, "The device information was successfully updated");

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

  ENDPOINT("PATCH", QString("%1/project/{projectId}/device-informations").arg(ACT_API_PATH_PREFIX).toStdString(),
           PatchDeviceInformation, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    qint64 project_id = *projectId;

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    QJsonObject req_obj = QJsonDocument::fromJson(str.toUtf8()).object();

    ActDeviceInformationList device_information_list;
    for (const QJsonValue &value : req_obj["DeviceInformationList"].toArray()) {
      QJsonObject value_obj = value.toObject();
      qint64 device_id = value_obj["DeviceId"].toInt();

      ActDeviceInformation device_information;
      act_status = act::core::g_core.GetDeviceInformation(project_id, device_id, device_information, is_operation);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
        return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                              act_status->ToString(act_status->key_order_).toStdString());
      }

      if (value_obj.contains("DeviceName")) {
        device_information.SetDeviceName(value_obj["DeviceName"].toString());
      }
      if (value_obj.contains("Location")) {
        device_information.SetLocation(value_obj["Location"].toString());
      }
      if (value_obj.contains("Description")) {
        device_information.SetDescription(value_obj["Description"].toString());
      }
      if (value_obj.contains("ContactInformation")) {
        device_information.SetContactInformation(value_obj["ContactInformation"].toString());
      }

      device_information_list.GetDeviceInformationList().append(device_information);
    }

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceInformations(project_id, device_information_list, is_operation);
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

  ENDPOINT_INFO(GetDeviceLoginPolicies) {
    info->summary = "Get all device login policy";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetDeviceLoginPolicyListDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/login-policies").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDeviceLoginPolicies, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDeviceLoginPolicyList login_policy_list;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetDeviceLoginPolicies(project_id, login_policy_list, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, login_policy_list.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateDeviceLoginPolicies) {
    info->summary = "Update the device login policy";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceLoginPolicyListDto>>("application/json");
    info->addResponse(Status::CODE_204, "The device login policies were successfully updated");

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

  ENDPOINT("PUT", QString("%1/project/{projectId}/login-policies").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateDeviceLoginPolicies, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDeviceLoginPolicyList login_policy_list;
    try {
      login_policy_list.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceLoginPolicies(project_id, login_policy_list, is_operation);
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

  ENDPOINT_INFO(PatchDeviceLoginPolicies) {
    info->summary = "Patch the device login policy";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceLoginPolicyListDto>>("application/json");
    info->addResponse(Status::CODE_204, "The device login policies were successfully updated");

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
  ENDPOINT("PATCH", QString("%1/project/{projectId}/login-policies").arg(ACT_API_PATH_PREFIX).toStdString(),
           PatchDeviceLoginPolicies, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    qint64 project_id = *projectId;

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    QJsonObject req_obj = QJsonDocument::fromJson(str.toUtf8()).object();

    ActDeviceLoginPolicyList login_policy_list;
    for (const QJsonValue &value : req_obj["DeviceLoginPolicyList"].toArray()) {
      QJsonObject value_obj = value.toObject();
      qint64 device_id = value_obj["DeviceId"].toInt();

      ActDeviceLoginPolicy login_policy;
      act_status = act::core::g_core.GetDeviceLoginPolicy(project_id, device_id, login_policy, is_operation);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
        return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                              act_status->ToString(act_status->key_order_).toStdString());
      }

      if (value_obj.contains("LoginMessage")) {
        login_policy.SetLoginMessage(value_obj["LoginMessage"].toString());
      }
      if (value_obj.contains("LoginAuthenticationFailureMessage")) {
        login_policy.SetLoginAuthenticationFailureMessage(value_obj["LoginAuthenticationFailureMessage"].toString());
      }
      if (value_obj.contains("AccountLoginFailureLockout")) {
        login_policy.SetAccountLoginFailureLockout(value_obj["AccountLoginFailureLockout"].toBool());
      }
      if (value_obj.contains("RetryFailureThreshold")) {
        login_policy.SetRetryFailureThreshold(value_obj["RetryFailureThreshold"].toInt());
      }
      if (value_obj.contains("LockoutDuration")) {
        login_policy.SetLockoutDuration(value_obj["LockoutDuration"].toInt());
      }
      if (value_obj.contains("AutoLogoutAfter")) {
        login_policy.SetAutoLogoutAfter(value_obj["AutoLogoutAfter"].toInt());
      }

      login_policy_list.GetDeviceLoginPolicyList().append(login_policy);
    }

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceLoginPolicies(project_id, login_policy_list, is_operation);
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

  ENDPOINT_INFO(GetDeviceLoopProtections) {
    info->summary = "Get all device loop protection";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetDeviceLoopProtectionsDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/loop-protections").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDeviceLoopProtections, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDeviceLoopProtectionList loop_protection_list;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetDeviceLoopProtections(project_id, loop_protection_list, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, loop_protection_list.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateDeviceLoopProtections) {
    info->summary = "Update the device loop protection";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceLoopProtectionsDto>>("application/json");
    info->addResponse(Status::CODE_204, "The device loop protections were successfully updated");

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

  ENDPOINT("PUT", QString("%1/project/{projectId}/loop-protections").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateDeviceLoopProtections, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDeviceLoopProtectionList loop_protection_list;
    try {
      loop_protection_list.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceLoopProtections(project_id, loop_protection_list, is_operation);
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

  ENDPOINT_INFO(PatchDeviceLoopProtections) {
    info->summary = "Patch the device loop protection";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceLoopProtectionsDto>>("application/json");
    info->addResponse(Status::CODE_204, "The device loop protections were successfully updated");

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

  ENDPOINT("PATCH", QString("%1/project/{projectId}/loop-protections").arg(ACT_API_PATH_PREFIX).toStdString(),
           PatchDeviceLoopProtections, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    qint64 project_id = *projectId;

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    QJsonObject req_obj = QJsonDocument::fromJson(str.toUtf8()).object();

    ActDeviceLoopProtectionList loop_protection_list;
    for (const QJsonValue &value : req_obj["DeviceLoopProtectionList"].toArray()) {
      QJsonObject value_obj = value.toObject();
      qint64 device_id = value_obj["DeviceId"].toInt();

      ActDeviceLoopProtection loop_protection;
      act_status = act::core::g_core.GetDeviceLoopProtection(project_id, device_id, loop_protection, is_operation);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
        return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                              act_status->ToString(act_status->key_order_).toStdString());
      }

      if (value_obj.contains("NetworkLoopProtection")) {
        loop_protection.SetNetworkLoopProtection(value_obj["NetworkLoopProtection"].toBool());
      }
      if (value_obj.contains("DetectInterval")) {
        loop_protection.SetDetectInterval(value_obj["DetectInterval"].toInt());
      }

      loop_protection_list.GetDeviceLoopProtectionList().append(loop_protection);
    }

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceLoopProtections(project_id, loop_protection_list, is_operation);
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

  ENDPOINT_INFO(GetDeviceSyslogSettings) {
    info->summary = "Get all device syslog setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetDeviceSyslogSettingsDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/syslog-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDeviceSyslogSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDeviceSyslogSettingList syslog_setting_list;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetDeviceSyslogSettings(project_id, syslog_setting_list, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, syslog_setting_list.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateDeviceSyslogSettings) {
    info->summary = "Update the device syslog setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceSyslogSettingsDto>>("application/json");
    info->addResponse(Status::CODE_204, "The device syslog settings were successfully updated");

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

  ENDPOINT("PUT", QString("%1/project/{projectId}/syslog-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateDeviceSyslogSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDeviceSyslogSettingList syslog_setting_list;
    try {
      syslog_setting_list.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceSyslogSettings(project_id, syslog_setting_list, is_operation);
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

  ENDPOINT_INFO(PatchDeviceSyslogSettings) {
    info->summary = "Patch the device syslog setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceSyslogSettingsDto>>("application/json");
    info->addResponse(Status::CODE_204, "The device syslog settings were successfully updated");

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
  ENDPOINT("PATCH", QString("%1/project/{projectId}/syslog-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           PatchDeviceSyslogSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    qint64 project_id = *projectId;

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    QJsonObject req_obj = QJsonDocument::fromJson(str.toUtf8()).object();

    ActDeviceSyslogSettingList syslog_setting_list;
    for (const QJsonValue &value : req_obj["DeviceSyslogSettingList"].toArray()) {
      QJsonObject value_obj = value.toObject();
      qint64 device_id = value_obj["DeviceId"].toInt();

      ActDeviceSyslogSetting syslog_setting;
      act_status = act::core::g_core.GetDeviceSyslogSetting(project_id, device_id, syslog_setting, is_operation);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
        return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                              act_status->ToString(act_status->key_order_).toStdString());
      }

      if (value_obj.contains("Enabled")) {
        syslog_setting.SetEnabled(value_obj["Enabled"].toBool());
      }
      if (value_obj.contains("SyslogServer1")) {
        syslog_setting.SetSyslogServer1(value_obj["SyslogServer1"].toBool());
      }
      if (value_obj.contains("Address1")) {
        syslog_setting.SetAddress1(value_obj["Address1"].toString());
      }
      if (value_obj.contains("UDPPort1")) {
        syslog_setting.SetUDPPort1(value_obj["UDPPort1"].toInt());
      }
      if (value_obj.contains("SyslogServer2")) {
        syslog_setting.SetSyslogServer2(value_obj["SyslogServer2"].toBool());
      }
      if (value_obj.contains("Address2")) {
        syslog_setting.SetAddress2(value_obj["Address2"].toString());
      }
      if (value_obj.contains("UDPPort2")) {
        syslog_setting.SetUDPPort2(value_obj["UDPPort2"].toInt());
      }
      if (value_obj.contains("SyslogServer3")) {
        syslog_setting.SetSyslogServer3(value_obj["SyslogServer3"].toBool());
      }
      if (value_obj.contains("Address3")) {
        syslog_setting.SetAddress3(value_obj["Address3"].toString());
      }
      if (value_obj.contains("UDPPort3")) {
        syslog_setting.SetUDPPort3(value_obj["UDPPort3"].toInt());
      }

      syslog_setting_list.GetDeviceSyslogSettingList().append(syslog_setting);
    }

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceSyslogSettings(project_id, syslog_setting_list, is_operation);
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

  ENDPOINT_INFO(GetDeviceSnmpTrapSettings) {
    info->summary = "Get all device snmp trap setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetDeviceSnmpTrapSettingsDto>>(Status::CODE_200, "application/json");
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
  ENDPOINT("GET", QString("%1/project/{projectId}/snmp-trap-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDeviceSnmpTrapSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDeviceSnmpTrapSettingList snmp_trap_setting_list;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetDeviceSnmpTrapSettings(project_id, snmp_trap_setting_list, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, snmp_trap_setting_list.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateDeviceSnmpTrapSettings) {
    info->summary = "Update the device snmp trap setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceSnmpTrapSettingsDto>>("application/json");
    info->addResponse(Status::CODE_204, "The device SNMP trap settings were successfully updated");

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
  ENDPOINT("PUT", QString("%1/project/{projectId}/snmp-trap-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateDeviceSnmpTrapSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDeviceSnmpTrapSettingList snmp_trap_setting_list;
    try {
      snmp_trap_setting_list.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceSnmpTrapSettings(project_id, snmp_trap_setting_list, is_operation);
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

  ENDPOINT_INFO(PatchDeviceSnmpTrapSettings) {
    info->summary = "Patch the device snmp trap setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceSnmpTrapSettingsDto>>("application/json");
    info->addResponse(Status::CODE_204, "The device SNMP trap settings were successfully updated");

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

  ENDPOINT("PATCH", QString("%1/project/{projectId}/snmp-trap-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           PatchDeviceSnmpTrapSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    qint64 project_id = *projectId;

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    QJsonObject req_obj = QJsonDocument::fromJson(str.toUtf8()).object();

    ActDeviceSnmpTrapSettingList snmp_trap_setting_list;
    for (const QJsonValue &value : req_obj["DeviceSnmpTrapSettingList"].toArray()) {
      QJsonObject value_obj = value.toObject();
      qint64 device_id = value_obj["DeviceId"].toInt();

      ActDeviceSnmpTrapSetting snmp_trap_setting;
      act_status = act::core::g_core.GetDeviceSnmpTrapSetting(project_id, device_id, snmp_trap_setting, is_operation);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
        return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                              act_status->ToString(act_status->key_order_).toStdString());
      }

      QList<ActSnmpTrapHostEntry> &host_list = snmp_trap_setting.GetHostList();
      if (value_obj.contains("HostList")) {
        for (const QJsonValue &host_value : value_obj["HostList"].toArray()) {
          QJsonObject host_obj = host_value.toObject();
          ActSnmpTrapHostEntry host_entry;
          host_entry.SetHostName(host_obj["HostName"].toString());
          if (host_list.contains(host_entry)) {
            host_entry = host_list[host_list.indexOf(host_entry)];
            host_list.removeAll(host_entry);
          }
          if (host_obj.contains("Mode")) {
            host_entry.SetMode(kActSnmpTrapModeEnumMap[host_obj["Mode"].toString()]);
          }
          if (host_obj.contains("TrapCommunity")) {
            host_entry.SetTrapCommunity(host_obj["TrapCommunity"].toString());
          }
          host_list.append(host_entry);
        }
      }
      snmp_trap_setting_list.GetDeviceSnmpTrapSettingList().append(snmp_trap_setting);
    }

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceSnmpTrapSettings(project_id, snmp_trap_setting_list, is_operation);
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

  ENDPOINT_INFO(GetDeviceBackups) {
    info->summary = "Get all backup device config informations";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetDeviceBackupsDto>>(Status::CODE_200, "application/json");
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
  ENDPOINT("GET", QString("%1/project/{projectId}/device-backups").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDeviceBackups, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDeviceBackupSettingList device_backup_setting_list;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetDeviceBackups(project_id, device_backup_setting_list, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, device_backup_setting_list.ToString().toStdString());
  }

  ENDPOINT_INFO(ExportDeviceBackupFile) {
    info->summary = "Get backup device config file";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActDeviceBackupFileDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/device-backups/export").arg(ACT_API_PATH_PREFIX).toStdString(),
           ExportDeviceBackupFile, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDeviceBackupFile device_backup_file;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.ExportDeviceBackupFile(project_id, device_backup_file, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, device_backup_file.ToString().toStdString());
  }

  ENDPOINT_INFO(ImportDeviceBackupFile) {
    info->summary = "Update the backup device config setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActDeviceBackupFileDto>>("application/json");
    info->addResponse(Status::CODE_204, "The backup device config setting was successfully imported");

    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
    info->pathParams["deviceId"].description = "The identifier of the device";
  }
  ENDPOINT(
      "PUT",
      QString("%1/project/{projectId}/device-backups/import/device/{deviceId}").arg(ACT_API_PATH_PREFIX).toStdString(),
      ImportDeviceBackupFile, PATH(UInt64, projectId), PATH(UInt64, deviceId),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDeviceBackupFile device_backup_file;
    try {
      device_backup_file.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    qint64 device_id = *deviceId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.ImportDeviceBackupFile(project_id, device_id, device_backup_file, is_operation);
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

  ENDPOINT_INFO(GetDeviceVlanSettings) {
    info->summary = "Get all device vlan setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetDeviceVlanSettingsDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/vlan-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDeviceVlanSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDeviceVlanSettingList vlan_setting_list;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetDeviceVlanSettings(project_id, vlan_setting_list, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, vlan_setting_list.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateDeviceVlanSettings) {
    info->summary = "Update the device vlan setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceVlanSettingsDto>>("application/json");
    info->addResponse(Status::CODE_204, "The device VLAN settings were successfully updated");

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

  ENDPOINT("PUT", QString("%1/project/{projectId}/vlan-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateDeviceVlanSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDeviceVlanSettingList vlan_setting_list;
    try {
      vlan_setting_list.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceVlanSettings(project_id, vlan_setting_list, is_operation);
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

  ENDPOINT_INFO(PatchDeviceVlanSettings) {
    info->summary = "Patch the device vlan setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceVlanSettingsDto>>("application/json");
    info->addResponse(Status::CODE_204, "The device VLAN settings were successfully updated");

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
  ENDPOINT("PATCH", QString("%1/project/{projectId}/vlan-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           PatchDeviceVlanSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    qint64 project_id = *projectId;

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    QJsonObject req_obj = QJsonDocument::fromJson(str.toUtf8()).object();

    ActDeviceVlanSettingList device_vlan_setting_list;
    for (const QJsonValue &value : req_obj["DeviceVlanSettingList"].toArray()) {
      QJsonObject value_obj = value.toObject();
      qint64 device_id = value_obj["DeviceId"].toInt();

      ActDeviceVlanSetting device_vlan_setting;
      act_status = act::core::g_core.GetDeviceVlanSetting(project_id, device_id, device_vlan_setting, is_operation);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
        return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                              act_status->ToString(act_status->key_order_).toStdString());
      }

      if (value_obj.contains("ManagementVlan")) {
        device_vlan_setting.SetManagementVlan(value_obj["ManagementVlan"].toInt());
      }

      QList<ActDeviceVlan> &vlan_list = device_vlan_setting.GetVlanList();
      if (value_obj.contains("VlanList")) {
        for (const QJsonValue &vlan_value : value_obj["VlanList"].toArray()) {
          QJsonObject vlan_obj = vlan_value.toObject();
          qint64 vlan_id = vlan_obj["VlanId"].toInt();
          ActDeviceVlan device_vlan(vlan_id);
          if (vlan_list.contains(device_vlan)) {
            device_vlan = vlan_list[vlan_list.indexOf(device_vlan)];
            vlan_list.removeAll(device_vlan);
          }
          if (vlan_obj.contains("VlanName")) {
            device_vlan.SetVlanName(vlan_obj["VlanName"].toString());
          }
          if (vlan_obj.contains("TeMstid")) {
            device_vlan.SetTeMstid(vlan_obj["TeMstid"].toBool());
          }
          if (vlan_obj.contains("MemberPort")) {
            QSet<qint64> member_port;
            for (const QJsonValue &port_value : vlan_obj["MemberPort"].toArray()) {
              member_port.insert(port_value.toInt());
            }
            device_vlan.SetMemberPort(member_port);
          }
          vlan_list.append(device_vlan);
        }
      }

      QList<ActDevicePort> &port_list = device_vlan_setting.GetPortList();
      if (value_obj.contains("PortList")) {
        for (const QJsonValue &port_value : value_obj["PortList"].toArray()) {
          QJsonObject port_obj = port_value.toObject();
          qint64 port_id = port_obj["PortId"].toInt();
          ActDevicePort device_port(port_id);
          if (port_list.contains(device_port)) {
            device_port = port_list[port_list.indexOf(device_port)];
            port_list.removeAll(device_port);
          }
          if (port_obj.contains("PortName")) {
            device_port.SetPortName(port_obj["PortName"].toString());
          }
          if (port_obj.contains("Active")) {
            device_port.SetActive(port_obj["Active"].toBool());
          }
          if (port_obj.contains("PortType")) {
            device_port.SetPortType(kActVlanPortTypeEnumMap[port_obj["PortType"].toString()]);
          }
          if (port_obj.contains("Pvid")) {
            device_port.SetPvid(port_obj["Pvid"].toInt());
          }
          if (port_obj.contains("DefaultPCP")) {
            device_port.SetDefaultPCP(port_obj["DefaultPCP"].toInt());
          }
          if (port_obj.contains("UntaggedVlan")) {
            QSet<qint32> untagged_vlan;
            for (const QJsonValue &untagged_vlan_value : port_obj["UntaggedVlan"].toArray()) {
              untagged_vlan.insert(untagged_vlan_value.toInt());
            }
            device_port.SetUntaggedVlan(untagged_vlan);
          }
          if (port_obj.contains("TaggedVlan")) {
            QSet<qint32> tagged_vlan;
            for (const QJsonValue &tagged_vlan_value : port_obj["TaggedVlan"].toArray()) {
              tagged_vlan.insert(tagged_vlan_value.toInt());
            }
            device_port.SetTaggedVlan(tagged_vlan);
          }
          port_list.append(device_port);
        }
      }

      device_vlan_setting_list.GetDeviceVlanSettingList().append(device_vlan_setting);
    }

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceVlanSettings(project_id, device_vlan_setting_list, is_operation);
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

  ENDPOINT_INFO(GetDeviceTimeSettings) {
    info->summary = "Get all device time setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetDeviceTimeSettingListDto>>(Status::CODE_200, "application/json");
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
  ENDPOINT("GET", QString("%1/project/{projectId}/time-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDeviceTimeSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDeviceTimeSettingList time_setting_list;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetDeviceTimeSettings(project_id, time_setting_list, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, time_setting_list.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateDeviceTimeSettings) {
    info->summary = "Update the device time setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceTimeSettingListDto>>("application/json");
    info->addResponse(Status::CODE_204, "The device time settings were successfully updated");

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
  ENDPOINT("PUT", QString("%1/project/{projectId}/time-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateDeviceTimeSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDeviceTimeSettingList time_setting_list;
    try {
      time_setting_list.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceTimeSettings(project_id, time_setting_list, is_operation);
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

  ENDPOINT_INFO(PatchDeviceTimeSettings) {
    info->summary = "Patch the device time setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceTimeSettingListDto>>("application/json");
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
  ENDPOINT("PATCH", QString("%1/project/{projectId}/time-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           PatchDeviceTimeSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    qint64 project_id = *projectId;

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    QJsonObject req_obj = QJsonDocument::fromJson(str.toUtf8()).object();

    ActDeviceTimeSettingList time_setting_list;
    for (const QJsonValue &value : req_obj["DeviceTimeSettingList"].toArray()) {
      QJsonObject value_obj = value.toObject();
      qint64 device_id = value_obj["DeviceId"].toInt();

      ActDeviceTimeSetting time_setting;
      act_status = act::core::g_core.GetDeviceTimeSetting(project_id, device_id, time_setting, is_operation);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
        return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                              act_status->ToString(act_status->key_order_).toStdString());
      }

      if (value_obj.contains("ClockSource")) {
        time_setting.SetClockSource(kActClockSourceEnumMap[value_obj["ClockSource"].toString()]);
      }
      if (value_obj.contains("NTPTimeServer1")) {
        time_setting.SetNTPTimeServer1(value_obj["NTPTimeServer1"].toString());
      }
      if (value_obj.contains("NTPTimeServer2")) {
        time_setting.SetNTPTimeServer2(value_obj["NTPTimeServer2"].toString());
      }
      if (value_obj.contains("SNTPTimeServer1")) {
        time_setting.SetSNTPTimeServer1(value_obj["SNTPTimeServer1"].toString());
      }
      if (value_obj.contains("SNTPTimeServer2")) {
        time_setting.SetSNTPTimeServer2(value_obj["SNTPTimeServer2"].toString());
      }
      if (value_obj.contains("TimeZone")) {
        time_setting.SetTimeZone(kActTimeZoneEnumMap[value_obj["TimeZone"].toString()]);
      }
      if (value_obj.contains("DaylightSavingTime")) {
        time_setting.SetDaylightSavingTime(value_obj["DaylightSavingTime"].toBool());
      }
      if (value_obj.contains("Offset")) {
        time_setting.SetOffset(value_obj["Offset"].toString());
      }
      if (value_obj.contains("Start")) {
        time_setting.GetStart().fromJson(value_obj["Start"].toObject());
      }
      if (value_obj.contains("End")) {
        time_setting.GetEnd().fromJson(value_obj["End"].toObject());
      }

      time_setting_list.GetDeviceTimeSettingList().append(time_setting);
    }

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceTimeSettings(project_id, time_setting_list, is_operation);
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

  ENDPOINT_INFO(GetDevicePortSettings) {
    info->summary = "Get all device port setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetDevicePortSettingListDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/port-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDevicePortSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDevicePortSettingList port_setting_list;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetDevicePortSettings(project_id, port_setting_list, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, port_setting_list.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateDevicePortSettings) {
    info->summary = "Update the device port setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDevicePortSettingListDto>>("application/json");
    info->addResponse<Object<ActStatusDto>>(
        Status::CODE_204, "application/json",
        "The successful result of a PUT or a DELETE is often not a 200 OK but a 204 No Content (or a 201 Created when "
        "the resource is uploaded for the first port).");
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

  ENDPOINT("PUT", QString("%1/project/{projectId}/port-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateDevicePortSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDevicePortSettingList port_setting_list;
    try {
      port_setting_list.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDevicePortSettings(project_id, port_setting_list, is_operation);
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

  ENDPOINT_INFO(PatchDevicePortSettings) {
    info->summary = "Patch the device port setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDevicePortSettingListDto>>("application/json");
    info->addResponse<Object<ActStatusDto>>(
        Status::CODE_204, "application/json",
        "The successful result of a PUT or a DELETE is often not a 200 OK but a 204 No Content (or a 201 Created when "
        "the resource is uploaded for the first port).");
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

  ENDPOINT("PATCH", QString("%1/project/{projectId}/port-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           PatchDevicePortSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    qint64 project_id = *projectId;

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    QJsonObject req_obj = QJsonDocument::fromJson(str.toUtf8()).object();

    ActDevicePortSettingList port_setting_list;
    for (const QJsonValue &value : req_obj["DevicePortSettingList"].toArray()) {
      QJsonObject value_obj = value.toObject();
      qint64 device_id = value_obj["DeviceId"].toInt();

      ActDevicePortSetting port_setting;
      act_status = act::core::g_core.GetDevicePortSetting(project_id, device_id, port_setting, is_operation);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
        return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                              act_status->ToString(act_status->key_order_).toStdString());
      }

      QList<ActDevicePortStatus> &port_list = port_setting.GetPortList();
      if (value_obj.contains("PortList")) {
        for (const QJsonValue &port_value : value_obj["PortList"].toArray()) {
          QJsonObject port_obj = port_value.toObject();
          qint64 port_id = port_obj["PortId"].toInt();
          ActDevicePortStatus port_status(port_id);
          if (port_list.contains(port_status)) {
            port_status = port_list[port_list.indexOf(port_status)];
            port_list.removeAll(port_status);
          }
          if (port_obj.contains("PortName")) {
            port_status.SetPortName(port_obj["PortName"].toString());
          }
          if (port_obj.contains("Active")) {
            port_status.SetActive(port_obj["Active"].toBool());
          }
          if (port_obj.contains("AdminStatus")) {
            port_status.SetAdminStatus(port_obj["AdminStatus"].toBool());
          }
          port_list.append(port_status);
        }
      }
      port_setting_list.GetDevicePortSettingList().append(port_setting);
    }

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDevicePortSettings(project_id, port_setting_list, is_operation);
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

  ENDPOINT_INFO(GetDeviceIpSettings) {
    info->summary = "Get all device ip setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetDeviceIpSettingListDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/ip-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDeviceIpSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDeviceIpSettingList ip_setting_list;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetDeviceIpSettings(project_id, ip_setting_list, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, ip_setting_list.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateDeviceIpSettings) {
    info->summary = "Update the device ip setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceIpSettingListDto>>("application/json");
    info->addResponse<Object<ActStatusDto>>(
        Status::CODE_204, "application/json",
        "The successful result of a PUT or a DELETE is often not a 200 OK but a 204 No Content (or a 201 Created when "
        "the resource is uploaded for the first ip).");
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
  ENDPOINT("PUT", QString("%1/project/{projectId}/ip-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateDeviceIpSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDeviceIpSettingList ip_setting_list;
    try {
      ip_setting_list.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceIpSettings(project_id, ip_setting_list, is_operation);
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

  ENDPOINT_INFO(PatchDeviceIpSettings) {
    info->summary = "Patch the device ip setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceIpSettingListDto>>("application/json");
    info->addResponse<Object<ActStatusDto>>(
        Status::CODE_204, "application/json",
        "The successful result of a PUT or a DELETE is often not a 200 OK but a 204 No Content (or a 201 Created when "
        "the resource is uploaded for the first ip).");
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

  ENDPOINT("PATCH", QString("%1/project/{projectId}/ip-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           PatchDeviceIpSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    qint64 project_id = *projectId;

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    QJsonObject req_obj = QJsonDocument::fromJson(str.toUtf8()).object();

    ActDeviceIpSettingList ip_setting_list;
    for (const QJsonValue &value : req_obj["DeviceIpSettingList"].toArray()) {
      QJsonObject value_obj = value.toObject();
      qint64 device_id = value_obj["DeviceId"].toInt();

      ActDeviceIpSetting ip_setting;
      act_status = act::core::g_core.GetDeviceIpSetting(project_id, device_id, ip_setting, is_operation);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
        return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                              act_status->ToString(act_status->key_order_).toStdString());
      }

      if (value_obj.contains("DeviceIp")) {
        ip_setting.SetDeviceIp(value_obj["DeviceIp"].toString());
      }
      if (value_obj.contains("SubnetMask")) {
        ip_setting.SetSubnetMask(value_obj["SubnetMask"].toString());
      }
      if (value_obj.contains("Gateway")) {
        ip_setting.SetGateway(value_obj["Gateway"].toString());
      }
      if (value_obj.contains("DNS1")) {
        ip_setting.SetDNS1(value_obj["DNS1"].toString());
      }
      if (value_obj.contains("DNS2")) {
        ip_setting.SetDNS2(value_obj["DNS2"].toString());
      }

      ip_setting_list.GetDeviceIpSettingList().append(ip_setting);
    }

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceIpSettings(project_id, ip_setting_list, is_operation);
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

  ENDPOINT_INFO(GetDeviceRstpSettings) {
    info->summary = "Get all device rstp setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetDeviceRstpSettingListDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/rstp-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDeviceRstpSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDeviceRstpSettingList rstp_setting_list;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetDeviceRstpSettings(project_id, rstp_setting_list, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, rstp_setting_list.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateDeviceRstpSettings) {
    info->summary = "Update the device rstp setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceRstpSettingListDto>>("application/json");
    info->addResponse<Object<ActStatusDto>>(
        Status::CODE_204, "application/json",
        "The successful result of a PUT or a DELETE is often not a 200 OK but a 204 No Content (or a 201 Created when "
        "the resource is uploaded for the first rstp).");
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

  ENDPOINT("PUT", QString("%1/project/{projectId}/rstp-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateDeviceRstpSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDeviceRstpSettingList rstp_setting_list;
    try {
      rstp_setting_list.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceRstpSettings(project_id, rstp_setting_list, is_operation);
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

  ENDPOINT_INFO(PatchDeviceRstpSettings) {
    info->summary = "Patch the device rstp setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceRstpSettingListDto>>("application/json");
    info->addResponse<Object<ActStatusDto>>(
        Status::CODE_204, "application/json",
        "The successful result of a PUT or a DELETE is often not a 200 OK but a 204 No Content (or a 201 Created when "
        "the resource is uploaded for the first rstp).");
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

  ENDPOINT("PATCH", QString("%1/project/{projectId}/rstp-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           PatchDeviceRstpSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    qint64 project_id = *projectId;

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    QJsonObject req_obj = QJsonDocument::fromJson(str.toUtf8()).object();

    ActDeviceRstpSettingList rstp_setting_list;
    for (const QJsonValue &value : req_obj["DeviceRstpSettingList"].toArray()) {
      QJsonObject value_obj = value.toObject();
      qint64 device_id = value_obj["DeviceId"].toInt();

      ActDeviceRstpSetting rstp_setting;
      act_status = act::core::g_core.GetDeviceRstpSetting(project_id, device_id, rstp_setting, is_operation);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
        return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                              act_status->ToString(act_status->key_order_).toStdString());
      }

      if (value_obj.contains("StpRstp")) {
        rstp_setting.SetStpRstp(value_obj["StpRstp"].toBool());
      }
      if (value_obj.contains("Priority")) {
        rstp_setting.SetPriority(value_obj["Priority"].toInt());
      }
      if (value_obj.contains("ForwardDelay")) {
        rstp_setting.SetForwardDelay(value_obj["ForwardDelay"].toInt());
      }
      if (value_obj.contains("HelloTime")) {
        rstp_setting.SetHelloTime(value_obj["HelloTime"].toInt());
      }
      if (value_obj.contains("MaxAge")) {
        rstp_setting.SetMaxAge(value_obj["MaxAge"].toInt());
      }
      if (value_obj.contains("RstpErrorRecoveryTime")) {
        rstp_setting.SetRstpErrorRecoveryTime(value_obj["RstpErrorRecoveryTime"].toInt());
      }
      if (value_obj.contains("RstpConfigSwift")) {
        rstp_setting.SetRstpConfigSwift(value_obj["RstpConfigSwift"].toBool());
      }
      if (value_obj.contains("RstpConfigRevert")) {
        rstp_setting.SetRstpConfigRevert(value_obj["RstpConfigRevert"].toBool());
      }

      QSet<ActDeviceRstpPortEntry> &rstp_port_entries = rstp_setting.GetRstpPortEntries();
      if (value_obj.contains("RstpPortEntries")) {
        for (const QJsonValue &port_value : value_obj["RstpPortEntries"].toArray()) {
          QJsonObject port_obj = port_value.toObject();
          ActDeviceRstpPortEntry rstp_port_entry(port_obj["PortId"].toInt());
          QSet<ActDeviceRstpPortEntry>::iterator rstp_port_entry_iter = rstp_port_entries.find(rstp_port_entry);
          if (rstp_port_entry_iter != rstp_port_entries.end()) {
            rstp_port_entry = *rstp_port_entry_iter;
            rstp_port_entries.erase(rstp_port_entry_iter);
          }
          if (port_obj.contains("RstpEnable")) {
            rstp_port_entry.SetRstpEnable(port_obj["RstpEnable"].toBool());
          }
          if (port_obj.contains("Edge")) {
            rstp_port_entry.SetEdge(kActRstpEdgeEnumMap[port_obj["Edge"].toString()]);
          }
          if (port_obj.contains("PortPriority")) {
            rstp_port_entry.SetPortPriority(port_obj["PortPriority"].toInt());
          }
          if (port_obj.contains("PathCost")) {
            rstp_port_entry.SetPathCost(port_obj["PathCost"].toInt());
          }
          if (port_obj.contains("LinkType")) {
            rstp_port_entry.SetLinkType(kActRstpLinkTypeEnumMap[port_obj["LinkType"].toString()]);
          }
          if (port_obj.contains("BpduGuard")) {
            rstp_port_entry.SetBpduGuard(port_obj["BpduGuard"].toBool());
          }
          if (port_obj.contains("RootGuard")) {
            rstp_port_entry.SetRootGuard(port_obj["RootGuard"].toBool());
          }
          if (port_obj.contains("LoopGuard")) {
            rstp_port_entry.SetLoopGuard(port_obj["LoopGuard"].toBool());
          }
          if (port_obj.contains("BpduFilter")) {
            rstp_port_entry.SetBpduFilter(port_obj["BpduFilter"].toBool());
          }
          rstp_port_entries.insert(rstp_port_entry);
        }
      }

      rstp_setting_list.GetDeviceRstpSettingList().append(rstp_setting);
    }

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceRstpSettings(project_id, rstp_setting_list, is_operation);
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

  ENDPOINT_INFO(GetDevicePerStreamPrioritySettings) {
    info->summary = "Get all device per-stream priority setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetDevicePerStreamPrioritySettingListDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/per-stream-priority-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDevicePerStreamPrioritySettings, PATH(UInt64, projectId),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDevicePerStreamPrioritySettingList setting_list;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetDevicePerStreamPrioritySettings(project_id, setting_list, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, setting_list.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateDevicePerStreamPrioritySettings) {
    info->summary = "Update the device per-stream priority setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDevicePerStreamPrioritySettingListDto>>("application/json");
    info->addResponse<Object<ActStatusDto>>(
        Status::CODE_204, "application/json",
        "The successful result of a PUT or a DELETE is often not a 200 OK but a 204 No Content (or a 201 Created when "
        "the resource is uploaded for the first per-stream priority setting).");
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

  ENDPOINT("PUT", QString("%1/project/{projectId}/per-stream-priority-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateDevicePerStreamPrioritySettings, PATH(UInt64, projectId),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDevicePerStreamPrioritySettingList setting_list;
    try {
      setting_list.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDevicePerStreamPrioritySettings(project_id, setting_list, is_operation);
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

  // ENDPOINT_INFO(PatchDevicePerStreamPrioritySettings) {
  //   info->summary = "Patch the device per-stream priority setting";
  //   info->addSecurityRequirement("my-realm");
  //   info->addTag("Device Setting");
  //   info->description = "This RESTful API only permit for [Admin, Supervisor]";
  //   info->addConsumes<Object<ActPutDevicePerStreamPrioritySettingListDto>>("application/json");
  //   info->addResponse<Object<ActStatusDto>>(
  //       Status::CODE_204, "application/json",
  //       "The successful result of a PUT or a DELETE is often not a 200 OK but a 204 No Content (or a 201 Created when
  //       " "the resource is uploaded for the first rstp).");
  //   info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
  //       .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
  //       .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
  //       .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

  //   info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
  //   info->queryParams["mode"].required = false;

  //   info->pathParams["projectId"].description = "The identifier of the project";
  // }

  // ENDPOINT("PATCH",
  //          QString("%1/project/{projectId}/per-stream-priority-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
  //          PatchDevicePerStreamPrioritySettings, PATH(UInt64, projectId),
  //          REQUEST(std::shared_ptr<IncomingRequest>, request),
  //          AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
  //   if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
  //     ActUnauthorized unauthorized;
  //     qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
  //              << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
  //     return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
  //                           unauthorized.ToString(unauthorized.key_order_).toStdString());
  //   }

  //   auto routes = request->getStartingLine().path.std_str();
  //   qDebug() << "PATCH URL:" << routes.c_str();

  //   // 解析 mode（僅接受 design / operation，其餘視為 design）
  //   oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
  //   std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
  //   std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
  //   bool is_operation = (mode_str == "operation");
  //   if (mode_str != "design" && mode_str != "operation") {
  //     qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
  //   }

  //   ACT_STATUS_INIT();
  //   // Handle request
  //   std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

  //   QMutexLocker core_lock(&act::core::g_core.mutex_);

  //   if (authorizationBearer->role == ActRoleEnum::kUser) {
  //     qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
  //     act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
  //     return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
  //   }

  //   qint64 project_id = *projectId;

  //   // Dto -> ACT Class
  //   QString str = request->readBodyToString().getPtr()->c_str();
  //   QJsonObject req_obj = QJsonDocument::fromJson(str.toUtf8()).object();

  //   ActDevicePerStreamPrioritySettingList setting_list;
  //   for (const QJsonValue &value : req_obj["DevicePerStreamPrioritySettingList"].toArray()) {
  //     QJsonObject value_obj = value.toObject();
  //     qint64 device_id = value_obj["DeviceId"].toInt();

  //     ActDevicePerStreamPrioritySetting device_setting;
  //     act_status =
  //         act::core::g_core.GetDevicePerStreamPrioritySetting(project_id, device_id, device_setting, is_operation);
  //     if (!IsActStatusSuccess(act_status)) {
  //       qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
  //       return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
  //                             act_status->ToString(act_status->key_order_).toStdString());
  //     }

  //     QList<ActDevicePerStreamPriorityEntry> &entries = device_setting.GetPerStreamPrioritySetting();
  //     if (value_obj.contains("PerStreamPrioritySetting")) {
  //       for (const QJsonValue &entry_value : value_obj["PerStreamPrioritySetting"].toArray()) {
  //         QJsonObject entry_obj = entry_value.toObject();
  //         ActDevicePerStreamPriorityEntry entry(entry_obj["PerStreamPriorityId"].toInt());
  //         QList<ActDevicePerStreamPriorityEntry>::iterator entry_iter =
  //             std::find(entries.begin(), entries.end(), entry);
  //         if (entry_iter != entries.end()) {
  //           entry = *entry_iter;
  //           entries.erase(entry_iter);
  //         }
  //         if (entry_obj.contains("Type")) {
  //           entry.SetType(kActStreamPriorityTypeEnumMap[entry_obj["Type"].toString()]);
  //         }
  //         if (entry_obj.contains("EtherType")) {
  //           entry.SetEtherType(entry_obj["EtherType"].toInt());
  //         }
  //         if (entry_obj.contains("EnableSubType")) {
  //           entry.SetEnableSubType(entry_obj["EnableSubType"].toBool());
  //         }
  //         if (entry_obj.contains("SubType")) {
  //           entry.SetSubType(entry_obj["SubType"].toInt());
  //         }
  //         if (entry_obj.contains("UdpPort")) {
  //           entry.SetUdpPort(entry_obj["UdpPort"].toInt());
  //         }
  //         if (entry_obj.contains("TcpPort")) {
  //           entry.SetTcpPort(entry_obj["TcpPort"].toInt());
  //         }
  //         if (entry_obj.contains("VlanId")) {
  //           entry.SetVlanId(entry_obj["VlanId"].toInt());
  //         }
  //         if (entry_obj.contains("PriorityCodePoint")) {
  //           entry.SetPriorityCodePoint(entry_obj["PriorityCodePoint"].toInt());
  //         }
  //         entries.append(entry);
  //       }
  //     }
  //     setting_list.GetDevicePerStreamPrioritySettingList().append(device_setting);
  //   }

  //   act::core::g_core.StartTransaction(project_id);
  //   act_status = act::core::g_core.UpdateDevicePerStreamPrioritySettings(project_id, setting_list, is_operation);
  //   if (IsActStatusSuccess(act_status)) {
  //     act::core::g_core.CommitTransaction(project_id);
  //   } else {
  //     act::core::g_core.StopTransaction(project_id);
  //   }

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

  ENDPOINT_INFO(GetDeviceTimeSlotSettings) {
    info->summary = "Get all device time slot setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGetDeviceTimeSlotSettingListDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/time-slot-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDeviceTimeSlotSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    ActDeviceTimeSlotSettingList setting_list;
    qint64 project_id = *projectId;
    act_status = act::core::g_core.GetDeviceTimeSlotSettings(project_id, setting_list, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, setting_list.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateDeviceTimeSlotSettings) {
    info->summary = "Update the device time slot setting";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Setting");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActPutDeviceTimeSlotSettingListDto>>("application/json");
    info->addResponse<Object<ActStatusDto>>(
        Status::CODE_204, "application/json",
        "The successful result of a PUT or a DELETE is often not a 200 OK but a 204 No Content (or a 201 Created when "
        "the resource is uploaded for the first per-stream priority setting).");
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

  ENDPOINT("PUT", QString("%1/project/{projectId}/time-slot-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateDeviceTimeSlotSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDeviceTimeSlotSettingList setting_list;
    try {
      setting_list.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    qint64 project_id = *projectId;
    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateDeviceTimeSlotSettings(project_id, setting_list, is_operation);
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

  // ENDPOINT_INFO(PatchDeviceTimeSlotSettings) {
  //   info->summary = "Patch the device time slot setting";
  //   info->addSecurityRequirement("my-realm");
  //   info->addTag("Device Setting");
  //   info->description = "This RESTful API only permit for [Admin, Supervisor]";
  //   info->addConsumes<Object<ActGetDeviceTimeSlotSettingListDto>>("application/json");
  //   info->addResponse<Object<ActStatusDto>>(
  //       Status::CODE_204, "application/json",
  //       "The successful result of a PUT or a DELETE is often not a 200 OK but a 204 No Content (or a 201 Created when
  //       " "the resource is uploaded for the first rstp).");
  //   info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
  //       .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
  //       .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  //   info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
  //       .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

  //   info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
  //   info->queryParams["mode"].required = false;

  //   info->pathParams["projectId"].description = "The identifier of the project";
  // }

  // ENDPOINT("PATCH", QString("%1/project/{projectId}/time-slot-settings").arg(ACT_API_PATH_PREFIX).toStdString(),
  //          PatchDeviceTimeSlotSettings, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
  //          AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
  //   if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
  //     ActUnauthorized unauthorized;
  //     qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
  //              << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
  //     return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
  //                           unauthorized.ToString(unauthorized.key_order_).toStdString());
  //   }

  //   auto routes = request->getStartingLine().path.std_str();
  //   qDebug() << "PATCH URL:" << routes.c_str();

  //   // 解析 mode（僅接受 design / operation，其餘視為 design）
  //   oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
  //   std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
  //   std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
  //   bool is_operation = (mode_str == "operation");
  //   if (mode_str != "design" && mode_str != "operation") {
  //     qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
  //   }

  //   ACT_STATUS_INIT();
  //   // Handle request
  //   std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

  //   QMutexLocker core_lock(&act::core::g_core.mutex_);

  //   if (authorizationBearer->role == ActRoleEnum::kUser) {
  //     qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
  //     act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
  //     return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
  //   }

  //   qint64 project_id = *projectId;

  //   // Dto -> ACT Class
  //   QString str = request->readBodyToString().getPtr()->c_str();
  //   QJsonObject req_obj = QJsonDocument::fromJson(str.toUtf8()).object();

  //   ActDeviceTimeSlotSettingList setting_list;
  //   for (const QJsonValue &value : req_obj["DeviceTimeSlotSettingList"].toArray()) {
  //     QJsonObject value_obj = value.toObject();
  //     qint64 device_id = value_obj["DeviceId"].toInt();

  //     ActDeviceTimeSlotSetting device_setting;
  //     act_status = act::core::g_core.GetDeviceTimeSlotSetting(project_id, device_id, device_setting, is_operation);
  //     if (!IsActStatusSuccess(act_status)) {
  //       qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
  //       return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
  //                             act_status->ToString(act_status->key_order_).toStdString());
  //     }

  //     QList<ActDeviceTimeSlotPortEntry> &port_list = device_setting.GetPortList();
  //     if (value_obj.contains("PortList")) {
  //       for (const QJsonValue &port_value : value_obj["PortList"].toArray()) {
  //         QJsonObject port_obj = port_value.toObject();
  //         ActDeviceTimeSlotPortEntry port_entry(port_obj["PortId"].toInt());
  //         QList<ActDeviceTimeSlotPortEntry>::iterator port_entry_iter =
  //             std::find(port_list.begin(), port_list.end(), port_entry);
  //         if (port_entry_iter != port_list.end()) {
  //           port_entry = *port_entry_iter;
  //           port_list.erase(port_entry_iter);
  //         }
  //         if (port_obj.contains("CycleTime")) {
  //           port_entry.SetCycleTime(port_obj["CycleTime"].toInt());
  //         }

  //         if (port_obj.contains("GateControlList")) {
  //           QList<ActDeviceTimeSlot> &gcl_list = port_entry.GetGateControlList();

  //           for (const QJsonValue &gcl_value : port_obj["GateControlList"].toArray()) {
  //             QJsonObject gcl_obj = gcl_value.toObject();
  //             ActDeviceTimeSlot time_slot(gcl_obj["SlotId"].toInt());
  //             QList<ActDeviceTimeSlot>::iterator time_slot_iter =
  //                 std::find(gcl_list.begin(), gcl_list.end(), time_slot);

  //             if (time_slot_iter != gcl_list.end()) {
  //               time_slot = *time_slot_iter;
  //               gcl_list.erase(time_slot_iter);
  //             }
  //             if (gcl_obj.contains("Interval")) {
  //               time_slot.SetInterval(gcl_obj["Interval"].toInt());
  //             }
  //             if (gcl_obj.contains("QueueSet")) {
  //               QList<quint8> queue_set;
  //               for (const QJsonValue &queue_value : gcl_obj["QueueSet"].toArray()) {
  //                 queue_set.append(queue_value.toInt());
  //               }
  //               time_slot.SetQueueSet(queue_set);
  //             }
  //             gcl_list.append(time_slot);
  //           }
  //         }
  //         port_list.append(port_entry);
  //       }
  //     }
  //     setting_list.GetDeviceTimeSlotSettingList().append(device_setting);
  //   }

  //   act::core::g_core.StartTransaction(project_id);
  //   act_status = act::core::g_core.UpdateDeviceTimeSlotSettings(project_id, setting_list, is_operation);
  //   if (IsActStatusSuccess(act_status)) {
  //     act::core::g_core.CommitTransaction(project_id);
  //   } else {
  //     act::core::g_core.StopTransaction(project_id);
  //   }

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
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
