/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_STATUS_DTO_HPP
#define ACT_STATUS_DTO_HPP

#include "act_status.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

/**
 * @brief Convert the act_status to ActBadRequest type
 *
 */
#define CONVERT_TO_BAD_REQUEST(act_status, status_dto)                    \
  ActBadRequest bad_request = dynamic_cast<ActBadRequest &>(*act_status); \
  auto status_dto = ActBadRequestDto::createShared();                     \
  status_dto->message = bad_request.GetMessage().toStdString();           \
  status_dto->status = StatusDtoEnum::kBadRequest;                        \
  status_dto->severity = SeverityDtoEnum::kCritical;

/**
 * @brief Dump DTO object
 *
 */
#define DUMP_DTO(code, dto)                                                                         \
  auto serializeConfig = oatpp::parser::json::mapping::Serializer::Config::createShared();          \
  auto deserializeConfig = oatpp::parser::json::mapping::Deserializer::Config::createShared();      \
  serializeConfig->useBeautifier = true;                                                            \
  auto jsonObjectMapper =                                                                           \
      oatpp::parser::json::mapping::ObjectMapper::createShared(serializeConfig, deserializeConfig); \
  oatpp::String json = jsonObjectMapper->writeToString(dto);                                        \
  qDebug() << "Response:" << code << ":" << json->c_str();

/**
 * @brief The transfer function between two data structures
 *
 * https://oatpp.io/api/latest/oatpp/web/protocol/http/Http/
 *
 * @param status
 * @return oatpp::web::protocol::http::Status
 */
inline oatpp::web::protocol::http::Status TransferActStatusToOatppStatus(const ActStatusType &status) {
  switch (status) {
    case ActStatusType::kSuccess:
      return oatpp::web::protocol::http::Status::CODE_200;
    case ActStatusType::kCreated:
      return oatpp::web::protocol::http::Status::CODE_201;
    case ActStatusType::kNoContent:
      return oatpp::web::protocol::http::Status::CODE_204;
    case ActStatusType::kBadRequest:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kUnauthorized:
      return oatpp::web::protocol::http::Status::CODE_401;
    case ActStatusType::kForbidden:
      return oatpp::web::protocol::http::Status::CODE_403;
    case ActStatusType::kNotFound:
      return oatpp::web::protocol::http::Status::CODE_404;
    case ActStatusType::kConflict:
      return oatpp::web::protocol::http::Status::CODE_409;
    case ActStatusType::kUnProcessable:
      return oatpp::web::protocol::http::Status::CODE_422;
    case ActStatusType::kInternalError:
      return oatpp::web::protocol::http::Status::CODE_500;

    case ActStatusType::kSkip:
      return oatpp::web::protocol::http::Status::CODE_200;
    case ActStatusType::kRunning:
      return oatpp::web::protocol::http::Status::CODE_200;
    case ActStatusType::kStop:
      return oatpp::web::protocol::http::Status::CODE_200;
    case ActStatusType::kFinished:
      return oatpp::web::protocol::http::Status::CODE_200;
    case ActStatusType::kFailed:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kDuplicated:
      return oatpp::web::protocol::http::Status::CODE_409;
    case ActStatusType::kFull:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kLicenseContentFailed:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kLicenseSizeFailed:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kLicenseNotActive:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kRoutingDestinationUnreachable:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kPcpInsufficientForTimeSlot:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kTimeSyncPcpNotConsistentWithDevice:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kSchedulingFailed:
      return oatpp::web::protocol::http::Status::CODE_200;
    case ActStatusType::kFeasibilityCheckFailed:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kCalculateTimeout:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kSetConfigFailed:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kGetDeviceDataFailed:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kCompareFailed:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kCompareTopologyFailed:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kSouthboundFailed:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kAutoScanFailed:
      return oatpp::web::protocol::http::Status::CODE_400;
    case ActStatusType::kServicePlatformUnauthorized:
      return oatpp::web::protocol::http::Status::CODE_400;

    default:
      qCritical() << "undefined http code:" << GetStringFromEnum<ActStatusType>(status, kActStatusTypeMap);
      return oatpp::web::protocol::http::Status::CODE_500;
  }
}

#include OATPP_CODEGEN_BEGIN(DTO)
ENUM(StatusDtoEnum, v_int32, VALUE(kSuccess, 1, "Success"), VALUE(kCreated, 2, "Created"),
     VALUE(kNoContent, 3, "NoContent"), VALUE(kBadRequest, 4, "BadRequest"), VALUE(kUnauthorized, 5, "Unauthorized"),
     VALUE(kForbidden, 6, "Forbidden"), VALUE(kNotFound, 7, "NotFound"), VALUE(kConflict, 8, "Conflict"),
     VALUE(kUnProcessable, 9, "UnProcessable"), VALUE(kInternalError, 10, "InternalError"), VALUE(kSkip, 11, "Skip"),
     VALUE(kRunning, 12, "Running"), VALUE(kStop, 13, "Stop"), VALUE(kFinished, 14, "Finished"),
     VALUE(kFailed, 15, "Failed"), VALUE(kDuplicated, 16, "Duplicated"), VALUE(kFull, 17, "Full"),
     VALUE(kLicenseContentFailed, 18, "LicenseContentFailed"), VALUE(kLicenseSizeFailed, 19, "LicenseSizeFailed"),
     VALUE(kLicenseNotActive, 20, "LicenseNotActive"), VALUE(kLicenseNotSupport, 21, "LicenseNotSupport"),
     VALUE(kDeviceProfileIsUsedFailed, 22, "DeviceProfileIsUsedFailed"),
     VALUE(kRoutingDestinationUnreachable, 23, "RoutingDestinationUnreachable"),
     VALUE(kSchedulingFailed, 24, "SchedulingFailed"),
     VALUE(kPcpInsufficientForTimeSlot, 25, "PcpInsufficientForTimeSlot"),
     VALUE(kRoutingDeviceTypeIncapable, 26, "RoutingDeviceTypeIncapable"),
     VALUE(kTimeSyncPcpNotConsistentWithDevice, 27, "TimeSyncPcpNotConsistentWithDevice"),
     VALUE(kFeasibilityCheckFailed, 28, "FeasibilityCheckFailed"), VALUE(kCalculateTimeout, 29, "CalculateTimeout"),
     VALUE(kSetConfigFailed, 30, "SetConfigFailed"), VALUE(kGetDeviceDataFailed, 31, "GetDeviceDataFailed"),
     VALUE(kCompareFailed, 32, "CompareFailed"), VALUE(kCompareTopologyFailed, 33, "CompareTopologyFailed"),
     VALUE(kDeployFailed, 34, "DeployFailed"), VALUE(kSyncFailed, 35, "SyncFailed"),
     VALUE(kCheckFeatureFailed, 36, "CheckFeatureFailed"), VALUE(kSouthboundFailed, 37, "SouthboundFailed"),
     VALUE(kAutoScanFailed, 38, "AutoScanFailed"),
     VALUE(kUpdateProjectTopologyFailed, 39, "UpdateProjectTopologyFailed"),
     VALUE(kServicePlatformUnauthorized, 40, "ServicePlatformUnauthorized"),
     VALUE(kOpcUaErrorCodeStart, 41, "OpcUaErrorCodeStart"), VALUE(kOpcUaErrorCodeEnd, 42, "OpcUaErrorCodeEnd"))

ENUM(SeverityDtoEnum, v_int32, VALUE(kFatal, 1, "Fatal"), VALUE(kCritical, 2, "Critical"), VALUE(Warning, 3, "Warning"),
     VALUE(kInformation, 4, "Information"), VALUE(kDebug, 5, "Debug"))

ENUM(HttpCodeDtoEnum, v_int32, VALUE(kSuccess, 200, "Success"), VALUE(kCreated, 201, "Created"),
     VALUE(kNoContent, 204, "NoContent"), VALUE(kBadRequest, 400, "BadRequest"),
     VALUE(kUnauthorized, 401, "Unauthorized"), VALUE(kForbidden, 403, "Forbidden"), VALUE(kNotFound, 404, "NotFound"),
     VALUE(kConflict, 409, "Conflict"), VALUE(kUnprocessable, 422, "Unprocessable"),
     VALUE(kInternalError, 500, "InternalError"))

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStatusDto : public oatpp::DTO {
  DTO_INIT(ActStatusDto, DTO)

  DTO_FIELD(Enum<StatusDtoEnum>, status, "Status");
  DTO_FIELD(Enum<SeverityDtoEnum>, severity, "Severity");
  DTO_FIELD(Enum<HttpCodeDtoEnum>, http_code, "HttpCode");

  ActStatusDto() {
    this->status = StatusDtoEnum::kSuccess;
    this->severity = SeverityDtoEnum::kInformation;
    this->http_code = HttpCodeDtoEnum::kSuccess;
  }

  ActStatusDto(StatusDtoEnum status_, SeverityDtoEnum severity_) : ActStatusDto() {
    this->status = status_;
    this->severity = severity_;
  }

  // static std::shared_ptr<ActStatusDto> createShared(
  //     StatusDtoEnum status, SeverityDtoEnum severity)  // Inject objectMapper component here as default parameter
  // {
  //   return std::make_shared<ActStatusDto>(status, severity);
  // }
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActBadRequestDto : public ActStatusDto {
  DTO_INIT(ActBadRequestDto, DTO)

  DTO_FIELD(String, message, "Message");

  ActBadRequestDto() {
    this->status = StatusDtoEnum::kBadRequest;
    this->severity = SeverityDtoEnum::kCritical;
    this->http_code = HttpCodeDtoEnum::kBadRequest;
  }

  ActBadRequestDto(String msg) : ActBadRequestDto() { this->message = msg; }
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActProgressStatusDto : public ActStatusDto {
  DTO_INIT(ActProgressStatusDto, DTO)

  DTO_FIELD(UInt8, progress, "Progress");

  ActProgressStatusDto() {}

  ActProgressStatusDto(UInt8 progress_) {
    this->status = StatusDtoEnum::kBadRequest;
    this->severity = SeverityDtoEnum::kCritical;
    this->http_code = HttpCodeDtoEnum::kBadRequest;
    this->progress = progress_;
  }
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActLicenseSizeFailedRequestDto : public ActStatusDto {
  DTO_INIT(ActLicenseSizeFailedRequestDto, DTO)

  DTO_FIELD(UInt16, size, "Size");
  DTO_FIELD(String, item, "Item");

  ActLicenseSizeFailedRequestDto() {}

  ActLicenseSizeFailedRequestDto(String item, UInt16 size) : item(item), size(size) {
    this->status = StatusDtoEnum::kLicenseSizeFailed;
    this->severity = SeverityDtoEnum::kCritical;
    this->http_code = HttpCodeDtoEnum::kUnprocessable;
  }
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActLicenseNotActiveDto : public ActStatusDto {
  DTO_INIT(ActLicenseNotActiveDto, DTO)

  DTO_FIELD(String, item, "Item");

  ActLicenseNotActiveDto() {}

  ActLicenseNotActiveDto(String item) : item(item) {
    this->status = StatusDtoEnum::kLicenseNotActive;
    this->severity = SeverityDtoEnum::kCritical;
    this->http_code = HttpCodeDtoEnum::kUnprocessable;
  }
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDuplicatedDto : public ActStatusDto {
  DTO_INIT(ActDuplicatedDto, DTO)

  DTO_FIELD(String, item, "Item");

  ActDuplicatedDto() {}

  ActDuplicatedDto(String msg) : item(msg) {
    this->status = StatusDtoEnum::kDuplicated;
    this->severity = SeverityDtoEnum::kCritical;
    this->http_code = HttpCodeDtoEnum::kConflict;
  }
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActBatchCreatedTopologyResponseDto : public ActStatusDto {
  DTO_INIT(ActBatchCreatedTopologyResponseDto, DTO)

  DTO_FIELD(List<Int64>, device_ids, "DeviceIds");
  DTO_FIELD(List<Int64>, link_ids, "LinkIds");

  ActBatchCreatedTopologyResponseDto() {
    this->status = StatusDtoEnum::kSuccess;
    this->severity = SeverityDtoEnum::kInformation;
    this->http_code = HttpCodeDtoEnum::kSuccess;
  }
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_STATUS_DTO_HPP */
