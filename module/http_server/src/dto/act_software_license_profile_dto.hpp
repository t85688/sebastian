/* Copyright (C) MOXA Inc. All rights reserved.
   This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
   See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include "./act_device_profile_dto.hpp"
#include "./act_system_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class ActSoftwareLicenseDto : public oatpp::DTO {
  DTO_INIT(ActSoftwareLicenseDto, DTO)

  DTO_FIELD(Int32, id, "Id");
  DTO_FIELD(String, icon_name, "IconName");
  DTO_FIELD(String, data_version, "DataVersion");
  DTO_FIELD(Boolean, purchasable, "Purchasable");
  DTO_FIELD(Enum<ActServiceProfileForDeviceProfileDtoEnum>, profiles,
            "Profiles") = ActServiceProfileForDeviceProfileDtoEnum::kSelfPlanning;
  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(Boolean, built_in, "BuiltIn");
  DTO_FIELD(Boolean, hide, "Hide");
  DTO_FIELD(Boolean, certificate, "Certificate");
  DTO_FIELD(String, vendor, "Vendor");
  DTO_FIELD(Enum<DeviceTypeDtoEnum>, device_type, "DeviceType");
};

class ActSoftwareLicensesDto : public oatpp::DTO {
  DTO_INIT(ActSoftwareLicensesDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActSoftwareLicenseDto>>, software_license_set, "SoftwareLicenseSet");
};

#include OATPP_CODEGEN_END(DTO)