/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_LINK_DTO_HPP
#define ACT_LINK_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)
ENUM(CableTypeDtoEnum, v_int32, VALUE(kCopper, 1, "Copper"), VALUE(kFiber, 2, "Fiber"))

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActLinkIdsDto : public oatpp::DTO {
  DTO_INIT(ActLinkIdsDto, DTO)

  DTO_FIELD(List<Int64>, link_ids, "LinkIds");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCreateLinkDto : public oatpp::DTO {
  DTO_INIT(ActCreateLinkDto, DTO)

  // DTO_FIELD(UInt64, bandwidth_value, "BandwidthValue");
  DTO_FIELD(UInt16, speed, "Speed") = 100;
  DTO_FIELD(UInt32, cable_length, "CableLength") = 1;
  DTO_FIELD(Enum<CableTypeDtoEnum>, cable_type, "CableType") = CableTypeDtoEnum::kCopper;
  DTO_FIELD(UInt32, propagation_delay, "PropagationDelay") = 5;

  DTO_FIELD(Int64, destination_device_id, "DestinationDeviceId");
  DTO_FIELD(UInt16, destination_interface_id, "DestinationInterfaceId");
  DTO_FIELD(Int64, source_device_id, "SourceDeviceId");
  DTO_FIELD(UInt16, source_interface_id, "SourceInterfaceId");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCreateLinksDto : public oatpp::DTO {
  DTO_INIT(ActCreateLinksDto, DTO)

  DTO_FIELD(List<Object<ActCreateLinkDto>>, links, "Links");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActLinkDto : public oatpp::DTO {
  DTO_INIT(ActLinkDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  // DTO_FIELD(UInt64, bandwidth_value, "BandwidthValue");
  DTO_FIELD(UInt16, speed, "Speed") = 100;
  DTO_FIELD(UInt32, cable_length, "CableLength") = 1;
  DTO_FIELD(Enum<CableTypeDtoEnum>, cable_type, "CableType") = CableTypeDtoEnum::kCopper;
  DTO_FIELD(UInt32, propagation_delay, "PropagationDelay") = 5;

  DTO_FIELD(Int64, destination_device_id, "DestinationDeviceId");
  DTO_FIELD(String, destination_device_ip, "DestinationDeviceIp");
  DTO_FIELD(Int64, destination_interface_id, "DestinationInterfaceId");
  // DTO_FIELD(String, destination_interface_name, "DestinationInterfaceName");
  DTO_FIELD(UInt16, source_device_id, "SourceDeviceId");
  DTO_FIELD(String, source_device_ip, "SourceDeviceIp");
  DTO_FIELD(UInt16, source_interface_id, "SourceInterfaceId");
  // DTO_FIELD(String, source_interface_name, "SourceInterfaceName");
  DTO_FIELD(Boolean, redundant, "Redundant") = false;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActLinksDto : public oatpp::DTO {
  DTO_INIT(ActLinksDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActLinkDto>>, link_set, "LinkSet");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPatchLinkMapDto : public oatpp::DTO {
  DTO_INIT(ActPatchLinkMapDto, DTO)

  DTO_FIELD(Fields<String>, patch_link_map, "PatchLinkMap");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_LINK_DTO_HPP */
