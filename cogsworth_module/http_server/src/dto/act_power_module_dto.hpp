/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "./act_interface_dto.hpp"
#include "./act_system_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(ActModuleSupportedSeriesDtoEnum, v_int32, VALUE(kTSN, 1, "TSN"), VALUE(kSDS, 2, "SDS"), VALUE(kTWS, 3, "TWS"),
     VALUE(kPT_7528, 4, "PT-7528"), VALUE(kPT_G500, 5, "PT-G500"), VALUE(kRKS, 6, "RKS"), VALUE(kEDS, 7, "EDS-(G)4000"),
     VALUE(kMDS, 8, "MDS"), VALUE(kEDS_2000, 9, "EDS-2000"), VALUE(kEDS_200A, 10, "EDS-200A"),
     VALUE(kEDS_500E, 11, "EDS-500E"), VALUE(kEDS_G2000, 12, "EDS-G2000"), VALUE(kEDS_G205, 13, "EDS-G205"),
     VALUE(kEDS_G200A, 14, "EDS-G200A"), VALUE(kEDS_G300, 15, "EDS-G300"), VALUE(kEDS_G500E, 16, "EDS-G500E"),
     VALUE(kIKS_6700A, 17, "IKS-6700A"), VALUE(kSDS_G3000, 18, "SDS-(G)3000"))

class ActOperatingVoltageDto : public oatpp::DTO {
  DTO_INIT(ActOperatingVoltageDto, DTO)

  DTO_FIELD(Float32, min, "Min");
  DTO_FIELD(Float32, max, "Max");
};

class ActVdcDto : public oatpp::DTO {
  DTO_INIT(ActVdcDto, DTO)

  DTO_FIELD(Boolean, supported, "supported");
  DTO_FIELD(List<Int32>, input, "Input");
  DTO_FIELD(Object<ActOperatingVoltageDto>, operating_voltage, "OperatingVoltage");
};

class ActVacDto : public oatpp::DTO {
  DTO_INIT(ActVacDto, DTO)

  DTO_FIELD(Boolean, supported, "supported");
  DTO_FIELD(List<Int32>, input, "Input");
  DTO_FIELD(Object<ActOperatingVoltageDto>, operating_voltage, "OperatingVoltage");
};

class ActPowerModuleDto : public oatpp::DTO {
  DTO_INIT(ActPowerModuleDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, icon_name, "IconName");
  DTO_FIELD(String, data_version, "DataVersion");
  DTO_FIELD(String, module_name, "ModuleName");
  DTO_FIELD(Boolean, purchasable, "Purchasable");
  DTO_FIELD(String, module_type, "ModuleType");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(Enum<ActServiceProfileForDeviceProfileDtoEnum>, profiles,
            "Profiles") = ActServiceProfileForDeviceProfileDtoEnum::kSelfPlanning;
  DTO_FIELD(Enum<ActModuleSupportedSeriesDtoEnum>, series, "Series") = ActModuleSupportedSeriesDtoEnum::kRKS;
  DTO_FIELD(Boolean, redundant_input, "RedundantInput");
  DTO_FIELD(Boolean, poe, "PoE");
  DTO_FIELD(Boolean, dying_gasp, "DyingGasp");
  DTO_FIELD(Object<ActVdcDto>, vdc, "VDC");
  DTO_FIELD(Object<ActVacDto>, vac, "VAC");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPowerModulesDto : public oatpp::DTO {
  DTO_INIT(ActPowerModulesDto, DTO)

  DTO_FIELD(Fields<Object<ActPowerModuleDto>>, power_module_map, "PowerModuleMap");
};

#include OATPP_CODEGEN_END(DTO)
