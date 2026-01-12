/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_FIRMWARE_DTO_HPP
#define ACT_FIRMWARE_DTO_HPP

#include "./act_device_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActFirmwareDto : public oatpp::DTO {
  DTO_INIT(ActFirmwareDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(UInt64, data_version, "DataVersion");
  DTO_FIELD(String, firmware_name, "FirmwareName");
  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(Enum<DeviceTypeDtoEnum>, device_type, "DeviceType");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActFirmwaresDto : public oatpp::DTO {
  DTO_INIT(ActFirmwaresDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActFirmwareDto>>, firmware_set, "FirmwareSet");
};
#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_FIRMWARE_DTO_HPP */
