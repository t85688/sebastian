/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include "../http_utils.h"
#include "act_status.hpp"
#include "dto/act_firmware_dto.hpp"
#include "dto/act_status_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/data/stream/FileStream.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/mime/multipart/FileProvider.hpp"
#include "oatpp/web/mime/multipart/InMemoryDataProvider.hpp"
#include "oatpp/web/mime/multipart/Part.hpp"
#include "oatpp/web/mime/multipart/PartList.hpp"
#include "oatpp/web/mime/multipart/Reader.hpp"
// #include "oatpp/parser/json/mapping/ObjectMapper.hpp"
// #include "oatpp/web/mime/multipart/TemporaryFileProvider.hpp"
#include "oatpp/web/protocol/http/outgoing/StreamingBody.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#define ACT_FIRMWARE_TMP_FILE "fimware_tmp"

#include OATPP_CODEGEN_BEGIN(ApiController)  //<-- Begin Codegen

/**
 * Sample Api Controller.
 */
class ActFirmwareController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  OATPP_COMPONENT(std::shared_ptr<StaticFilesManager>, staticFilesManager);

  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActFirmwareController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActFirmwareController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {
    // setDefaultAuthorizationHandler(
    //     std::make_shared<oatpp::web::server::handler::BearerAuthorizationHandler>("my-realm"));  // JWT
  }

  ~ActFirmwareController() {
    if (!QFile::remove(QString(ACT_FIRMWARE_TMP_FILE))) {
      qDebug() << __func__ << "Cannot remove Firmware tmp file.";
    } else {
      qDebug() << __func__ << "Success remove Firmware tmp file.";
    }
  }

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActFirmwareController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                             objectMapper)) {
    return std::make_shared<ActFirmwareController>(objectMapper);
  }

 public:
  ENDPOINT_INFO(GetFirmwares) {
    info->summary = "Get all firmwares";
    info->addSecurityRequirement("my-realm");
    info->addTag("Firmware");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActFirmwaresDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/firmwares").arg(ACT_API_PATH_PREFIX).toStdString(), GetFirmwares,
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

    std::string str = act::core::g_core.ToString("FirmwareSet").toStdString();

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, str);
  }

  ENDPOINT_INFO(UploadFirmware) {
    info->summary = "Upload a firmware file";
    info->addSecurityRequirement("my-realm");
    info->addTag("Firmware");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addConsumes<oatpp::swagger::Binary>("application/octet-stream");  // tell swagger that it's a file

    info->addResponse<Object<ActStatusDto>>(Status::CODE_204, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_409, "application/json", "The firmware already exist")
        .addExample("Already Exist", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }

  ENDPOINT("POST", QString("%1/firmware").arg(ACT_API_PATH_PREFIX).toStdString(), UploadFirmware,
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
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    // -- -- -- -- -- -- -- --Multipart
    auto multipart = std::make_shared<oatpp::web::mime::multipart::PartList>(request->getHeaders());
    // Create multipart reader
    oatpp::web::mime::multipart::Reader multipartReader(multipart.get());

    // Configure to stream part with name "part1" to file */
    multipartReader.setPartReader("file", oatpp::web::mime::multipart::createFilePartReader(ACT_FIRMWARE_TMP_FILE));
    multipartReader.setPartReader(
        "request", oatpp::web::mime::multipart::createInMemoryPartReader(16 * 1024 /* max-data-size(Bytes) */));

    // Read multipart body
    request->transferBody(&multipartReader);

    // Print value of "file_part"
    auto file_part = multipart->getNamedPart("file");
    auto request_part = multipart->getNamedPart("request");

    // Return file_part is null
    if (file_part == nullptr) {
      act_status = std::make_shared<ActStatusNotFound>("file");
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // Get part data input stream
    auto inputStream = file_part->getPayload()->openInputStream();

    // Check firmware file folder exists
    QDir fw_file_dir(ACT_FIRMWARE_FILE_FOLDER);
    if (!fw_file_dir.exists()) {
      if (!QDir().mkpath(ACT_FIRMWARE_FILE_FOLDER)) {
        qDebug() << __func__ << "mkpath() failed:" << ACT_FIRMWARE_FILE_FOLDER;
        act_status = std::make_shared<ActStatusInternalError>("CreateFirmwareFileFolder");
        qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
        return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                              act_status->ToString(act_status->key_order_).toStdString());
      }
    }

    // Save Firmware file(inputStream to file)
    QString file_part_file_name = QString(file_part->getFilename()->c_str());
    // qDebug() << __func__ << QString("file_part->Filename:%1").arg(file_part_file_name).toStdString().c_str();
    QString save_file_path = QString("%1/%2").arg(ACT_FIRMWARE_FILE_FOLDER).arg(file_part_file_name);

    // Directly replace file
    oatpp::String buffer(1024);
    oatpp::data::stream::FileOutputStream outputStream(save_file_path.toStdString().c_str(), "wb");
    oatpp::data::stream::transfer(inputStream, &outputStream, 0, buffer->data(), buffer->size());
    outputStream.close();
    // Get Firmware info(request part)
    ActFirmware act_firmware;

    if (request_part != nullptr) {
      auto request_part_content = request_part->getPayload()->getInMemoryData()->c_str();
      qDebug() << __func__ << QString("Received request_part: %1").arg(request_part_content).toStdString().c_str();
      act_firmware.FromString(request_part_content);
    }
    act_firmware.SetFirmwareName(file_part_file_name);  // set firmware file name
    qDebug() << __func__ << QString("Upload Firmware: %1").arg(act_firmware.ToString()).toStdString().c_str();

    // Upload Firmware to database

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.UploadFirmware(act_firmware);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // Response
    act_status->SetStatus(ActStatusType::kNoContent);
    // qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
    return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                          act_status->ToString(act_status->key_order_).toStdString());
  }

  ENDPOINT_INFO(DeleteFirmware) {
    info->summary = "Delete a firmware file";
    info->addSecurityRequirement("my-realm");
    info->addTag("Firmware");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
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

    info->pathParams["firmwareName"].description = "The identifier of the firmware ";
  }
  ENDPOINT("DELETE", QString("%1/firmware/{firmwareId}").arg(ACT_API_PATH_PREFIX).toStdString(), DeleteFirmware,
           PATH(UInt64, firmwareId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Handle request
    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 id = *firmwareId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.DeleteFirmware(id);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // // Delete firmware from configuration folder
    // QString file_path = QString("%1/%2").arg(ACT_FIRMWARE_FILE_FOLDER).arg(firmwareId->c_str());
    // qDebug() << __func__ << QString("File path:%3").arg(file_path).toStdString().c_str();

    // if (!QFile::remove(file_path)) {
    //   // qCritical() << __func__ << "Cannot remove firmware from configuration folder:" << ACT_FIRMWARE_FILE_FOLDER;

    //   QString message = QString("Cannot remove firmware");
    //   act_status = std::make_shared<ActBadRequest>(message);

    //   qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
    //   return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
    //                         act_status->ToString(act_status->key_order_).toStdString());
    // }

    act_status->SetStatus(ActStatusType::kNoContent);
    // qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
    return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                          act_status->ToString(act_status->key_order_).toStdString());
  }

  // ENDPOINT("POST",QString("%1/firmware/{firmwareName}").arg(ACT_API_PATH_PREFIX).toStdString(), UploadFirmware,
  // PATH(String, firmwareName),
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
  //   qDebug() << "POST URL:" << routes.c_str();
  //   qDebug() << "firmwareName:" << firmwareName->c_str();

  //   ACT_STATUS_INIT();
  //   std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

  //   // Upload firmwareName to configuration folder
  //   QString file_path = QString("%1/%2").arg(ACT_FIRMWARE_FILE_FOLDER).arg(firmwareName->c_str());
  //   qDebug() << __func__ << QString("File path:%3").arg(file_path).toStdString().c_str();

  //   QFile file(file_path);
  //   if (file.exists()) {
  //     qDebug() << "File exists:" << file_path.toStdString().c_str();

  //     ActStatusInternalError internal_error("Firmware");
  //     qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(internal_error.GetStatus(), kActStatusTypeMap)
  //             << internal_error.ToString().toStdString().c_str();
  //     return createResponse(TransferActStatusToOatppStatus(internal_error.GetStatus()),
  //                           internal_error.ToString().toStdString());
  //   }

  //   // ---------------- Binary
  //   oatpp::data::stream::FileOutputStream fileOutputStream(file_path.toStdString().c_str(), "wb");
  //   request->transferBodyToStream(&fileOutputStream);  // transfer body chunk by chunk
  //
  //   // Response
  //   act_status->SetStatus(ActStatusType::kNoContent);
  //   qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
  //   return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
  //                         act_status->ToString(act_status->key_order_).toStdString());
  // }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
