/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_REDUNDANT_SWIFT_DTO_HPP
#define ACT_REDUNDANT_SWIFT_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSwiftCandidateDto : public oatpp::DTO {
  DTO_INIT(ActSwiftCandidateDto, DTO);

  DTO_FIELD(Int64, root_device, "RootDevice");
  DTO_FIELD(Int64, backup_root_device, "BackupRootDevice");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSwiftCandidatesDto : public oatpp::DTO {
  DTO_INIT(ActSwiftCandidatesDto, DTO);

  DTO_FIELD(UnorderedSet<Object<ActSwiftCandidateDto>>, candidates, "Candidates");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSwiftDto : public oatpp::DTO {
  DTO_INIT(ActSwiftDto, DTO);

  DTO_FIELD(Boolean, active, "Active");
  DTO_FIELD(Int64, root_device, "RootDevice");
  DTO_FIELD(Int64, backup_root_device, "BackupRootDevice");
  DTO_FIELD(Fields<Int16>, device_tier_map, "DeviceTierMap");
  DTO_FIELD(UnorderedSet<Int64>, links, "Links");
};

#endif  // ACT_REDUNDANT_SWIFT_DTO_HPP