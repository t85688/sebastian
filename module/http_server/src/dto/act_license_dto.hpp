/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_LICENSE_DTO_HPP
#define ACT_LICENSE_DTO_HPP

// #include "act_user.hpp"
#include "./act_system_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class ActSizeLicenseDto : public oatpp::DTO {
  DTO_INIT(ActSizeLicenseDto, DTO)

  DTO_FIELD(UInt16, project_size, "ProjectSize");
  DTO_FIELD(UInt16, device_qty, "DeviceQty");
};

class ActDeviceConfigLicenseDto : public oatpp::DTO {
  DTO_INIT(ActDeviceConfigLicenseDto, DTO)

  DTO_FIELD(Boolean, enabled, "Enabled");
};

class ActTSNLicenseDto : public oatpp::DTO {
  DTO_INIT(ActTSNLicenseDto, DTO)

  DTO_FIELD(Boolean, enabled, "Enabled");
  DTO_FIELD(Boolean, scheduling, "Scheduling");
  DTO_FIELD(Boolean, frer, "FRER");
};

class ActStageLicenseDto : public oatpp::DTO {
  DTO_INIT(ActStageLicenseDto, DTO)

  DTO_FIELD(Boolean, design, "Design");
  DTO_FIELD(Boolean, operation, "Operation");
  DTO_FIELD(Boolean, manufacture, "Manufacture");
};

class ActFeatureLicenseDto : public oatpp::DTO {
  DTO_INIT(ActFeatureLicenseDto, DTO)

  DTO_FIELD(Boolean, https, "Https");
  DTO_FIELD(Boolean, opcua, "Opcua");
  DTO_FIELD(Boolean, intelligent, "Intelligent");
  DTO_FIELD(Boolean, auto_probe, "AutoProbe");
  DTO_FIELD(Object<ActDeviceConfigLicenseDto>, device_config, "DeviceConfig");
  DTO_FIELD(Object<ActTSNLicenseDto>, tsn, "TSN");
  DTO_FIELD(Object<ActStageLicenseDto>, stage, "Stage");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActLicenseDto : public oatpp::DTO {
  DTO_INIT(ActLicenseDto, DTO)

  DTO_FIELD(String, date, "Date");
  DTO_FIELD(UInt8, major_version, "MajorVersion");
  DTO_FIELD(String, bound_mac_address, "BoundMacAddress");
  DTO_FIELD(Object<ActSizeLicenseDto>, size, "Size");
  DTO_FIELD(List<Enum<ActServiceProfileForLicenseDtoEnum>>, profiles, "Profiles");
  DTO_FIELD(Enum<ActDeploymentTypeDtoEnum>, deployment_type, "DeploymentType");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_LICENSE_DTO_HPP */
