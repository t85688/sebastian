/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_FIRMWARE_FEATURE_PROFILE_DTO_HPP
#define ACT_FIRMWARE_FEATURE_PROFILE_DTO_HPP

#include "./act_feature_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSimpleFirmwareFeatureProfileDto : public oatpp::DTO {
  DTO_INIT(ActSimpleFirmwareFeatureProfileDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(UInt64, data_version, "DataVersion");
  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(UnorderedSet<String>, firmware_versions, "FirmwareVersions");
  DTO_FIELD(Object<ActFeatureGroupDto>, feature_group, "FeatureGroup");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSimpleFirmwareFeatureProfilesDto : public oatpp::DTO {
  DTO_INIT(ActSimpleFirmwareFeatureProfilesDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActSimpleFirmwareFeatureProfileDto>>, firmware_feature_profile_set,
            "FirmwareFeatureProfileSet");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_FIRMWARE_FEATURE_PROFILE_DTO_HPP */
