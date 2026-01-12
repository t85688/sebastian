/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActManufactureOrderDto : public oatpp::DTO {
  DTO_INIT(ActManufactureOrderDto, DTO);

  DTO_FIELD(String, order, "Order");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActManufactureResultDeviceDto : public oatpp::DTO {
  DTO_INIT(ActManufactureResultDeviceDto, DTO)

  DTO_FIELD(Int64, index, "Index");
  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, ip_address, "IpAddress");
  DTO_FIELD(String, device_name, "DeviceName");
  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(String, serial_number, "SerialNumber");
  DTO_FIELD(Int64, time_stamp, "TimeStamp");
};

ENUM(ActManufactureStatusDtoEnum, v_int32, VALUE(kSuccess, 1, "Success"), VALUE(kFailed, 2, "Failed"),
     VALUE(kReady, 3, "Ready"), VALUE(kRemain, 4, "Remain"))

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActManufactureResultItemDto : public oatpp::DTO {
  DTO_INIT(ActManufactureResultItemDto, DTO)

  DTO_FIELD(Int64, batch, "Batch");
  DTO_FIELD(List<Object<ActManufactureResultDeviceDto>>, devices, "Devices");

  DTO_FIELD(Enum<ActManufactureStatusDtoEnum>, status, "Status") = ActManufactureStatusDtoEnum::kReady;

  DTO_FIELD(Int64, time_stamp, "TimeStamp");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStageManufactureResultDto : public oatpp::DTO {
  DTO_INIT(ActStageManufactureResultDto, DTO)

  DTO_FIELD(List<Object<ActManufactureResultItemDto>>, manufacture_report, "ManufactureReport");
  DTO_FIELD(Object<ActManufactureResultItemDto>, remain, "Remain");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActManufactureResultDto : public oatpp::DTO {
  DTO_INIT(ActManufactureResultDto, DTO)

  DTO_FIELD(String, order, "Order");
  DTO_FIELD(List<Object<ActManufactureResultItemDto>>, manufacture_report, "ManufactureReport");
  DTO_FIELD(Object<ActManufactureResultItemDto>, ready, "Ready");
  DTO_FIELD(Object<ActManufactureResultItemDto>, remain, "Remain");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTotalManufactureResultDto : public oatpp::DTO {
  DTO_INIT(ActTotalManufactureResultDto, DTO)

  DTO_FIELD(List<Object<ActManufactureResultDeviceDto>>, total_report, "TotalReport");
};

#include OATPP_CODEGEN_END(DTO)
