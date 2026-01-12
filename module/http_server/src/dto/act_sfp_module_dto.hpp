/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "./act_interface_dto.hpp"
#include "./act_system_dto.hpp"
#include "./act_temperature_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(ActSFPmodeDtoEnum, v_int32, VALUE(kSingle, 1, "Single"), VALUE(kMulti, 2, "Multi"))
ENUM(ActSFPConnectorTypeDtoEnum, v_int32, VALUE(kLC, 1, "LC"), VALUE(kRJ45, 2, "RJ45"))

class WavelengthNMDto : public oatpp::DTO {
  DTO_INIT(WavelengthNMDto, DTO)

  DTO_FIELD(Int32, tx, "TX");
  DTO_FIELD(Int32, rx, "RX");
};

class ActSFPModuleDto : public oatpp::DTO {
  DTO_INIT(ActSFPModuleDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, icon_name, "IconName");
  DTO_FIELD(String, data_version, "DataVersion");
  DTO_FIELD(String, module_name, "ModuleName");
  DTO_FIELD(Boolean, purchasable, "Purchasable");
  DTO_FIELD(String, module_type, "ModuleType");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(Enum<ActServiceProfileForDeviceProfileDtoEnum>, profiles,
            "Profiles") = ActServiceProfileForDeviceProfileDtoEnum::kSelfPlanning;
  DTO_FIELD(Int32, speed, "Speed");
  DTO_FIELD(Boolean, wdm, "WDM");
  DTO_FIELD(Enum<ActSFPmodeDtoEnum>, mode, "Mode") = ActSFPmodeDtoEnum::kSingle;
  DTO_FIELD(Enum<ActSFPConnectorTypeDtoEnum>, connector_type, "ConnectorType") = ActSFPConnectorTypeDtoEnum::kLC;
  DTO_FIELD(Int32, transmission_distance_km, "TransmissionDistanceKM");
  DTO_FIELD(Object<OperatingTemperatureCDto>, operating_temperature_c, "OperatingTemperatureC");
  DTO_FIELD(Object<WavelengthNMDto>, wavelength_nm, "WavelengthNM");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSFPModulesDto : public oatpp::DTO {
  DTO_INIT(ActSFPModulesDto, DTO)

  DTO_FIELD(Fields<Object<ActSFPModuleDto>>, sfp_module_map, "SFPModuleMap");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSFPCountsDto : public oatpp::DTO {
  DTO_INIT(ActSFPCountsDto, DTO)
  DTO_FIELD(Fields<UInt32>, sfp_counts, "SFPCounts");
};

#include OATPP_CODEGEN_END(DTO)
