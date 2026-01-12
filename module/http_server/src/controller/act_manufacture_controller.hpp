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
#include "dto/act_manufacture_dto.hpp"
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
class ActManufactureController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActManufactureController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActManufactureController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {}

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActManufactureController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                                objectMapper)) {
    return std::make_shared<ActManufactureController>(objectMapper);
  }

 public:
  ENDPOINT_INFO(DownloadDeviceConfigZipFile) {
    info->summary = "Download the device config zip file";
    info->addSecurityRequirement("my-realm");
    info->addTag("Manufacture");
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

    info->pathParams["projectId"].description = "The identifier of the project";
  }

  ENDPOINT("GET", QString("%1/project/{projectId}/download-device-config-zip").arg(ACT_API_PATH_PREFIX).toStdString(),
           DownloadDeviceConfigZipFile, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    ActDeviceBackupFile device_backup_file;
    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GenerateDeviceIniConfigZipFile(project_id, device_backup_file);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, device_backup_file.ToString().toStdString());
  }

  ENDPOINT_INFO(UploadDeviceConfigZipFile) {
    info->summary = "Upload the device config zip file";
    info->addSecurityRequirement("my-realm");
    info->addTag("Manufacture");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";

    // Specify multipart/form-data for file uploads
    info->addConsumes<Object<ActZipFileMultipartDto>>("multipart/form-data");
    info->addResponse(Status::CODE_204, "The device config zip file was successfully uploaded");

    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->pathParams["projectId"].description = "The identifier of the project";
  }

  ENDPOINT("PUT", QString("%1/upload-device-config-zip").arg(ACT_API_PATH_PREFIX).toStdString(),
           UploadDeviceConfigZipFile, REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Multipart
    auto multipart = std::make_shared<oatpp::web::mime::multipart::PartList>(request->getHeaders());
    oatpp::web::mime::multipart::Reader multipartReader(multipart.get());

    // Configure to stream part with name "part1" to file */
    multipartReader.setPartReader("file", oatpp::web::mime::multipart::createFilePartReader(ACT_FIRMWARE_TMP_FILE));
    multipartReader.setPartReader(
        "request", oatpp::web::mime::multipart::createInMemoryPartReader(16 * 1024 /* max-data-size(Bytes) */));
    request->transferBody(&multipartReader);

    auto file_part = multipart->getNamedPart("file");
    if (file_part == nullptr) {  // return null
      act_status = std::make_shared<ActStatusNotFound>("file");
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    // Clear DeviceConfig tmp folder
    // kene+
    /*
    act_status = act::core::g_core.ClearFolder(ACT_DEVICE_CONFIG_FILE_FOLDER);
    */
    QString deviceConfigFilePath = GetDeviceConfigFilePath();
    act_status = act::core::g_core.ClearFolder(deviceConfigFilePath);
    // kene-
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // Save Zip file
    auto inputStream = file_part->getPayload()->openInputStream();
    QString file_part_file_name = QString(file_part->getFilename()->c_str());
    // kene+
    /*
    QString save_file_path = QString("%1/%2").arg(ACT_DEVICE_CONFIG_FILE_FOLDER).arg(file_part_file_name);
    */
    QString save_file_path = QString("%1/%2").arg(deviceConfigFilePath).arg(file_part_file_name);
    // kene-
    qInfo() << __func__ << QString("Upload File: %1").arg(file_part_file_name).toStdString().c_str();

    // Directly replace file
    oatpp::String buffer(1024);
    oatpp::data::stream::FileOutputStream outputStream(save_file_path.toStdString().c_str(), "wb");
    oatpp::data::stream::transfer(inputStream, &outputStream, 0, buffer->data(), buffer->size());
    outputStream.close();

    // Unzip file
    // kene+
    /*
    act_status = act::core::g_core.UnZipFile(save_file_path, ACT_DEVICE_CONFIG_FILE_FOLDER);
    */
    act_status = act::core::g_core.UnZipFile(save_file_path, deviceConfigFilePath);
    // kene-
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // Remove Zip file
    if (!QFile::remove(save_file_path)) {
      qWarning() << "Failed to remove file:" << save_file_path;
      act_status = std::make_shared<ActStatusInternalError>("RemoveZipFileFailed");
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    act_status->SetStatus(ActStatusType::kNoContent);
    qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
    return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                          act_status->ToString(act_status->key_order_).toStdString());
  }

  ENDPOINT_INFO(UpdateManufactureOrder) {
    info->summary = "Update the Manufacture Order";
    info->addSecurityRequirement("my-realm");
    info->addTag("Manufacture");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addConsumes<Object<ActManufactureOrderDto>>("application/json");
    info->addResponse<Object<ActManufactureOrderDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("PUT", QString("%1/project/{projectId}/manufacture-order").arg(ACT_API_PATH_PREFIX).toStdString(),
           UpdateManufactureOrder, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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
    qDebug() << "projectId:" << *projectId;

    ACT_STATUS_INIT();
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActManufactureResult manufacture_result;
    try {
      manufacture_result.FromString(str);
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

    act_status =
        act::core::g_core.UpdateManufactureResultOrder(project_id, manufacture_result.GetOrder(), manufacture_result);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    return createResponse(Status::CODE_200, manufacture_result.ToString("Order").toStdString());
  }

  ENDPOINT_INFO(GetManufactureOrder) {
    info->summary = "Get the Manufacture Order";
    info->addSecurityRequirement("my-realm");
    info->addTag("Manufacture");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActManufactureOrderDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/manufacture-order").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetManufactureOrder, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    ActManufactureResult manufacture_result;
    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GetManufactureResult(project_id, manufacture_result);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, manufacture_result.ToString("Order").toStdString());
  }

  ENDPOINT_INFO(ClearManufactureResult) {
    info->summary = "Clear the Manufacture Result";
    info->addSecurityRequirement("my-realm");
    info->addTag("Manufacture");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActStageManufactureResultDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("PUT", QString("%1/project/{projectId}/clear-manufacture-result").arg(ACT_API_PATH_PREFIX).toStdString(),
           ClearManufactureResult, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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
    qDebug() << "projectId:" << *projectId;

    // Handle request
    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    ActManufactureResult manufacture_result;
    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.InitManufactureResult(project_id, manufacture_result);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, manufacture_result.ToString({"ManufactureReport", "Remain"}).toStdString());
  }

  ENDPOINT_INFO(GetStageManufactureResult) {
    info->summary = "Get the Stage Manufacture Result";
    info->addSecurityRequirement("my-realm");
    info->addTag("Manufacture");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActStageManufactureResultDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("GET", QString("%1/project/{projectId}/stage-manufacture-result").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetStageManufactureResult, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    ActManufactureResult manufacture_result;
    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GetManufactureResult(project_id, manufacture_result);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, manufacture_result.ToString({"ManufactureReport", "Remain"}).toStdString());
  }

  // ENDPOINT_INFO(GetTotalManufactureResult) {
  //   info->summary = "Get the Total Manufacture Result";
  //   info->addSecurityRequirement("my-realm");
  //   info->addTag("Manufacture");
  //   info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
  //   info->addResponse<Object<ActTotalManufactureResultDto>>(Status::CODE_200, "application/json");
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

  // ENDPOINT("GET", QString("%1/project/{projectId}/total_manufacture_result").arg(ACT_API_PATH_PREFIX).toStdString(),
  //          GetTotalManufactureResult, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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
  //   qDebug() << "projectId:" << *projectId;

  //   // Handle request
  //   ACT_STATUS_INIT();
  //   std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

  //   ActTotalManufactureResult total_manufacture_result;
  //   qint64 project_id = *projectId;
  // QMutexLocker core_lock(&act::core::g_core.mutex_);
  //   act_status = act::core::g_core.GetTotalManufactureResult(project_id, total_manufacture_result);
  //   if (!IsActStatusSuccess(act_status)) {
  //     qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
  //     return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
  //                           act_status->ToString(act_status->key_order_).toStdString());
  //   }

  //   // qDebug() << "Response: Success(200)";
  //   return createResponse(Status::CODE_200, total_manufacture_result.ToString().toStdString());
  // }

  ENDPOINT_INFO(ReManufactureDevices) {
    info->summary = "Re-manufacture the Devices";
    info->addSecurityRequirement("my-realm");
    info->addTag("Manufacture");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addConsumes<Object<ActDeviceIdsDto>>("application/json");
    info->addResponse<Object<ActStageManufactureResultDto>>(Status::CODE_200, "application/json");
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

  ENDPOINT("PUT", QString("%1/project/{projectId}/re-manufacture-devices").arg(ACT_API_PATH_PREFIX).toStdString(),
           ReManufactureDevices, PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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
    qDebug() << "projectId:" << *projectId;

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
    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    ActManufactureResult manufacture_result;
    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.UpdateReManufactureDevices(project_id, dev_ids.GetDeviceIds(), manufacture_result);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, manufacture_result.ToString({"ManufactureReport", "Remain"}).toStdString());
  }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
