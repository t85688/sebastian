/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "./act_interface_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActEthernetModuleDto : public oatpp::DTO {
  DTO_INIT(ActEthernetModuleDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, icon_name, "IconName");
  DTO_FIELD(String, data_version, "DataVersion");
  DTO_FIELD(String, module_name, "ModuleName");
  DTO_FIELD(Boolean, purchasable, "Purchasable");
  DTO_FIELD(String, module_type, "ModuleType");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(Enum<ActServiceProfileForDeviceProfileDtoEnum>, profiles,
            "Profiles") = ActServiceProfileForDeviceProfileDtoEnum::kSelfPlanning;
  DTO_FIELD(String, number_of_interfaces, "NumberOfInterfaces");
  DTO_FIELD(List<Object<ActInterfacePropertyDto>>, interfaces, "Interfaces");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActEthernetModulesDto : public oatpp::DTO {
  DTO_INIT(ActEthernetModulesDto, DTO)

  DTO_FIELD(Fields<Object<ActEthernetModuleDto>>, ethernet_module_map, "EthernetModuleMap");
};

#include OATPP_CODEGEN_END(DTO)
