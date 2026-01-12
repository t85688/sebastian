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
#include "act_device.hpp"
#include "act_project.hpp"
#include "act_status.hpp"
#include "dto/act_device_dto.hpp"
#include "dto/act_device_profile_dto.hpp"
#include "dto/act_firmware_feature_profile_dto.hpp"
#include "dto/act_power_device_profile_dto.hpp"
#include "dto/act_software_license_profile_dto.hpp"
#include "dto/act_status_dto.hpp"
#include "dto/act_topology_dto.hpp"
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
class ActDeviceProfileController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  OATPP_COMPONENT(std::shared_ptr<StaticDeviceIconFilesManager>, staticDeviceIconFilesManager);

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActDeviceProfileController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActDeviceProfileController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {
    // setDefaultAuthorizationHandler(
    //     std::make_shared<oatpp::web::server::handler::BearerAuthorizationHandler>("my-realm"));  // JWT
  }

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActDeviceProfileController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                                  objectMapper)) {
    return std::make_shared<ActDeviceProfileController>(objectMapper);
  }

 public:
  // TODO Insert Your endpoints here !!!

  /********************
   *                  *
   *  Device Profile  *
   *                  *
   * ******************/
  ENDPOINT_INFO(GetDeviceProfiles) {
    info->summary = "Get all device profiles";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Profile");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActDeviceProfilesDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/device-profiles").arg(ACT_API_PATH_PREFIX).toStdString(), GetDeviceProfiles,
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

    return createResponse(Status::CODE_200, act::core::g_core.ToString("DeviceProfileSet").toStdString());
  }

  ENDPOINT_INFO(GetSimpleDeviceProfiles) {
    info->summary = "Get all device profiles in simple format";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Profile");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActSimpleDeviceProfilesDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/simple-device-profiles").arg(ACT_API_PATH_PREFIX).toStdString(), GetSimpleDeviceProfiles,
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

    // Turn device profile into simple device profile
    QSet<ActDeviceProfile> &profiles = act::core::g_core.GetDeviceProfileSet();
    QSet<ActSimpleDeviceProfile> simple_profiles;

    for (auto profile : profiles) {
      ActSimpleDeviceProfile simple_profile(profile);
      simple_profiles.insert(simple_profile);
    }

    // Response
    ActSimpleDeviceProfileSet reply;
    reply.SetSimpleDeviceProfileSet(simple_profiles);

    return createResponse(Status::CODE_200, reply.ToString().toStdString());
  }

  ENDPOINT_INFO(GetDeviceProfilesWithDefaultDeviceConfig) {
    info->summary = "Get all device profiles with the DefaultDeviceConfig";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Profile");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActDeviceProfilesWithDefaultDeviceConfigDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/device-profiles-with-default-device-config").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDeviceProfilesWithDefaultDeviceConfig, REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Turn device profile into simple device profile
    QSet<ActDeviceProfile> &profiles = act::core::g_core.GetDeviceProfileSet();
    QSet<ActDeviceProfileWithDefaultDeviceConfig> profile_with_default_device_config_set;

    for (auto profile : profiles) {
      ActDeviceProfileWithDefaultDeviceConfig profile_with_default_device_config(profile);
      profile_with_default_device_config_set.insert(profile_with_default_device_config);
    }

    // Response
    ActDevicesProfileWithDefaultDeviceConfigSet reply;
    reply.SetDeviceProfileWithDefaultDeviceConfigSet(profile_with_default_device_config_set);

    return createResponse(Status::CODE_200, reply.ToString().toStdString());
  }

  ENDPOINT_INFO(UploadDeviceProfile) {
    info->summary = "Upload a new device profile";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Profile");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActCreateDeviceProfileDto>>("application/json");
    info->addResponse<Object<ActDeviceProfileDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_409, "application/json", "The device profile already exist")
        .addExample("Already Exist", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("POST", QString("%1/device-profile/upload").arg(ACT_API_PATH_PREFIX).toStdString(), UploadDeviceProfile,
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDeviceProfile device_profile;
    try {
      device_profile.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // Upload device profile to configuration folder
    act_status = act::core::g_core.UploadDeviceProfile(device_profile);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    ActSimpleDeviceProfile simple_profile(device_profile);

    // qDebug() << "Response: Created(201)";
    return createResponse(Status::CODE_201, simple_profile.ToString(simple_profile.key_order_).toStdString());
  }

  ENDPOINT_INFO(CreateDeviceProfile) {
    info->summary = "Create a new device profile";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Profile");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActCreateDeviceProfileDto>>("application/json");
    info->addResponse<Object<ActDeviceProfileDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_409, "application/json", "The device profile already exist")
        .addExample("Already Exist", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("POST", QString("%1/device-profile/create").arg(ACT_API_PATH_PREFIX).toStdString(), CreateDeviceProfile,
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

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActDeviceProfile device_profile;
    try {
      device_profile.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    device_profile.SetBuiltIn(false);

    // Upload device profile to configuration folder
    act_status = act::core::g_core.UploadDeviceProfile(device_profile);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    ActSimpleDeviceProfile simple_profile(device_profile);

    // qDebug() << "Response: Created(201)";
    return createResponse(Status::CODE_201, simple_profile.ToString(simple_profile.key_order_).toStdString());
  }

  ENDPOINT_INFO(DeleteDeviceProfile) {
    info->summary = "Delete a device profile, the device icon will be deleted too";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Profile");
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

    info->pathParams["deviceProfileId"].description = "The identifier of the device profile ";
  }
  ENDPOINT("DELETE", QString("%1/device-profile/{deviceProfileId}").arg(ACT_API_PATH_PREFIX).toStdString(),
           DeleteDeviceProfile, PATH(UInt64, deviceProfileId), REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    QMutexLocker core_lock(&act::core::g_core.mutex_);
    // Delete device profile & icon from configuration folder
    qint64 id = *deviceProfileId;
    act_status = act::core::g_core.DeleteDeviceProfile(id);
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

  /************************
   *                      *
   * Power Device Profile *
   *                      *
   ************************/

  ENDPOINT_INFO(GetPowerDevices) {
    info->summary = "Get all power device configurations";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Profile");
    info->description = "RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActPowerDevicesDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/power-devices").arg(ACT_API_PATH_PREFIX).toStdString(), GetPowerDevices,
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

    // Dump all power device profiles
    return createResponse(Status::CODE_200, act::core::g_core.ToString("PowerDeviceProfileSet").toStdString());
  }

  /****************************
   *                          *
   * Software License Profile *
   *                          *
   ****************************/

  ENDPOINT_INFO(GetSoftwareLicenses) {
    info->summary = "Get all software license configurations";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Profile");
    info->description = "RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActSoftwareLicensesDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json");
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/software-licenses").arg(ACT_API_PATH_PREFIX).toStdString(), GetSoftwareLicenses,
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

    // Dump all software license profiles
    return createResponse(Status::CODE_200, act::core::g_core.ToString("SoftwareLicenseProfileSet").toStdString());
  }

  /********************
   *                  *
   *  Device Icon     *
   *                  *
   * ******************/
  ENDPOINT_INFO(UploadDeviceIcon) {
    info->summary = "Upload a new device icon for the specific device profile ";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Icon");
    info->addConsumes<String>("text/plain");
    info->description =
        "Uploads the icon. **Important:** The body must be a **Base64 encoded Data URI string** (e.g., "
        "`data:image/png;base64,iVBORw...`). This RESTful API only permit for [Admin, Supervisor]";

    info->addResponse<Object<ActStatusDto>>(Status::CODE_204, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_409, "application/json", "The device icon already exist")
        .addExample("Already Exist", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->pathParams["deviceIconId"].description = "The identifier of the device profile ";
  }
  ADD_CORS(UploadDeviceIcon, "*", "GET, POST, DELETE, PUT")
  ENDPOINT("POST", QString("%1/device-icon/{deviceProfileId}/{iconName}").arg(ACT_API_PATH_PREFIX).toStdString(),
           UploadDeviceIcon, PATH(UInt64, deviceProfileId), PATH(String, iconName),
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

    // qDebug() << "iconName:" << iconName->c_str();

    ACT_STATUS_INIT();

    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Upload icon to configuration folder
    qint64 id = *deviceProfileId;
    QString device_icon_name(iconName->c_str());
    // kene+
    /*
    QString filename(ACT_DEVICE_ICON_FOLDER);
    */
    QString filename(GetDeviceIconPath());
    // kene-
    filename.append("/");
    filename.append(device_icon_name);
    // filename.append(".png");

    QFile file(filename);
    // Remove the check duplication logic, the user may want to overwrite the previous icon
    // if (file.exists()) {
    //   qDebug() << "File exists:" << filename.toStdString().c_str();

    //   ActStatusInternalError internal_error("Device Icon");
    //   qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(internal_error.GetStatus(), kActStatusTypeMap)
    //           << internal_error.ToString().toStdString().c_str();
    //   return createResponse(TransferActStatusToOatppStatus(internal_error.GetStatus()),
    //                         internal_error.ToString().toStdString());
    // }

    // You need to remove the data:image/png;base64, prefix from the start of the string first.
    // example:
    //   "data:image/png;base64,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";
    QString url_base64_data = request->readBodyToString()->c_str();
    // qDebug() << "url_base64_data:" << url_base64_data.toStdString().c_str();
    QStringList list = url_base64_data.split(",");
    if (list.size() < 2) {
      ActBadRequest bad_request("Invalid Data URI format");
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    QByteArray base64Data = list[1].toStdString().c_str();
    QImage image;
    image.loadFromData(QByteArray::fromBase64(base64Data), "PNG");
    bool success = image.save(filename, "PNG");
    if (success) {
      // [feat:2382] Update device icon

      act_status = act::core::g_core.UpdateDeviceProfileIcon(id, device_icon_name);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "Upload device icon failed:" << device_icon_name.toStdString().c_str();
        return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                              act_status->ToString(act_status->key_order_).toStdString());
      }

      act_status->SetStatus(ActStatusType::kNoContent);
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    } else {
      ActBadRequest bad_request("Create icon file failed, please check the icon format");
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }
  }

  ENDPOINT_INFO(GetDeviceIcon) {
    info->summary = "Get an existing device icon, the \"deviceIconId\" is equal to \"deviceProfileId\"";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Icon");
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
  ADD_CORS(GetDeviceIcon, "*", "GET, POST, DELETE, PUT")
  ENDPOINT("GET", QString("%1/device-icon/{deviceProfileId}").arg(ACT_API_PATH_PREFIX).toStdString(), GetDeviceIcon,
           PATH(UInt64, deviceProfileId), REQUEST(std::shared_ptr<IncomingRequest>, request),
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

    auto routes = request->getStartingLine().path.std_str();
    // qDebug() << "GET URL:" << routes.c_str();

    ACT_STATUS_INIT();

    // std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    // QMutexLocker core_lock(&act::core::g_core.mutex_);

    qint64 device_profile_id = *deviceProfileId;

    // Get the device profile via the device profile id
    ActDeviceProfile device_profile;

    const QSet<ActDeviceProfile> device_profiles = act::core::g_core.GetDeviceProfileSet();

    act_status = ActGetItemById(device_profiles, device_profile_id, device_profile);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "device profile id" << QString::number(device_profile_id) << "not found";
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // Fetch the device icon name in the device profile
    QString icon_name = device_profile.GetIconName();

    // Get the icon from configuration folder
    auto file = staticDeviceIconFilesManager->GetFile(icon_name.toStdString().c_str());
    // qDebug() << "Response Success(200)";
    auto response = createResponse(Status::CODE_200, file);

    return response;
  }

  ENDPOINT_INFO(GetDeviceIconByName) {
    info->summary = "Get the device icon by name";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Icon");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<oatpp::swagger::Binary>(Status::CODE_200,
                                              "application/octet-stream");  // tell swagger that it's a file
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->pathParams["iconName"].description = "The device icon filename";
  }
  ADD_CORS(GetDeviceIconByName, "*", "GET, POST, DELETE, PUT")
  ENDPOINT("GET", QString("%1/device-icon/name/{iconName}").arg(ACT_API_PATH_PREFIX).toStdString(), GetDeviceIconByName,
           PATH(String, iconName), REQUEST(std::shared_ptr<IncomingRequest>, request),
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

    auto routes = request->getStartingLine().path.std_str();
    // qDebug() << "GET URL:" << routes.c_str();

    ACT_STATUS_INIT();

    // std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QString icon_name = QString::fromStdString(iconName->c_str());

    // Get the icon from configuration folder
    auto file = staticDeviceIconFilesManager->GetFile(icon_name.toStdString().c_str());
    // qDebug() << "Response Success(200)";
    auto response = createResponse(Status::CODE_200, file);

    return response;
  }

  ENDPOINT_INFO(GetDefaultDeviceIcon) {
    info->summary = "Get the default device icon";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Icon");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<oatpp::swagger::Binary>(Status::CODE_200,
                                              "application/octet-stream");  // tell swagger that it's a file
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/default-device-icon").arg(ACT_API_PATH_PREFIX).toStdString(), GetDefaultDeviceIcon,
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

    // std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QString filename(ACT_DEFAULT_DEVICE_ICON_NAME);
    // qDebug() << "Response Success(200)";
    auto file = staticDeviceIconFilesManager->GetFile(filename.toStdString().c_str());
    auto response = createResponse(Status::CODE_200, file);
    return response;
  }

  ENDPOINT_INFO(GetDefaultICMPDeviceIcon) {
    info->summary = "Get the default icon of TSN ICMP";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Icon");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<oatpp::swagger::Binary>(Status::CODE_200,
                                              "application/octet-stream");  // tell swagger that it's a file
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }

  ENDPOINT("GET", QString("%1/default-icmp-icon").arg(ACT_API_PATH_PREFIX).toStdString(), GetDefaultICMPDeviceIcon,
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

    // std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QString filename(ACT_DEFAULT_ICMP_ICON_NAME);
    // qDebug() << "Response Success(200)";
    auto file = staticDeviceIconFilesManager->GetFile(filename.toStdString().c_str());
    auto response = createResponse(Status::CODE_200, file);
    return response;
  }

  ENDPOINT_INFO(GetDefaultSwitchDeviceIcon) {
    info->summary = "Get the default icon of TSN Switch";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Icon");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<oatpp::swagger::Binary>(Status::CODE_200,
                                              "application/octet-stream");  // tell swagger that it's a file
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/default-switch-icon").arg(ACT_API_PATH_PREFIX).toStdString(), GetDefaultSwitchDeviceIcon,
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

    // std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QString filename(ACT_DEFAULT_SWITCH_ICON_NAME);
    // qDebug() << "Response Success(200)";
    auto file = staticDeviceIconFilesManager->GetFile(filename.toStdString().c_str());
    auto response = createResponse(Status::CODE_200, file);
    return response;
  }

  ENDPOINT_INFO(GetDefaultEndStationDeviceIcon) {
    info->summary = "Get the default icon of End Station";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Icon");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<oatpp::swagger::Binary>(Status::CODE_200,
                                              "application/octet-stream");  // tell swagger that it's a file
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/default-end-station-icon").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDefaultEndStationDeviceIcon, REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QString filename(ACT_DEFAULT_END_STATION_ICON_NAME);
    // qDebug() << "Response Success(200)";
    auto file = staticDeviceIconFilesManager->GetFile(filename.toStdString().c_str());
    auto response = createResponse(Status::CODE_200, file);
    return response;
  }

  ENDPOINT_INFO(GetDefaultBridgedEndStationDeviceIcon) {
    info->summary = "Get the default icon of Bridged End Station";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Icon");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<oatpp::swagger::Binary>(Status::CODE_200,
                                              "application/octet-stream");  // tell swagger that it's a file
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/default-bridged-end-station-icon").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetDefaultBridgedEndStationDeviceIcon, REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    QString filename(ACT_DEFAULT_BRIDGED_END_STATION_ICON_NAME);
    // qDebug() << "Response Success(200)";
    auto file = staticDeviceIconFilesManager->GetFile(filename.toStdString().c_str());
    auto response = createResponse(Status::CODE_200, file);
    return response;
  }

  /******************************
   *                            *
   *  Firmware feature profile  *
   *                            *
   * ****************************/

  ENDPOINT_INFO(GetFirmwareFeatureProfiles) {
    info->summary = "Get all firmware feature profiles";
    info->addSecurityRequirement("my-realm");
    info->addTag("Device Profile");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActSimpleFirmwareFeatureProfilesDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));
  }
  ENDPOINT("GET", QString("%1/firmware-feature-profiles").arg(ACT_API_PATH_PREFIX).toStdString(),
           GetFirmwareFeatureProfiles, REQUEST(std::shared_ptr<IncomingRequest>, request),
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

    // Turn profile into simple profile
    QSet<ActFirmwareFeatureProfile> &profiles = act::core::g_core.GetFirmwareFeatureProfileSet();
    QSet<ActSimpleFirmwareFeatureProfile> simple_profiles;
    for (auto profile : profiles) {
      ActSimpleFirmwareFeatureProfile simple_profile(profile);
      simple_profiles.insert(simple_profile);
    }

    // Response
    ActSimpleFirmwareFeatureProfileSet reply;
    reply.SetSimpleFirmwareFeatureProfileSet(simple_profiles);

    return createResponse(Status::CODE_200, reply.ToString().toStdString());
  }
};

#include OATPP_CODEGEN_END(ApiController)  //<-- End Codegen
