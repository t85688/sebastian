/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "./act_device_dto.hpp"
#include "./act_link_dto.hpp"
#include "./act_stream_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCreateTopologyDto : public oatpp::DTO {
  DTO_INIT(ActCreateTopologyDto, DTO);

  DTO_FIELD(String, topology_name, "TopologyName");
  DTO_FIELD(Int64, project_id, "ProjectId");
  DTO_FIELD(UnorderedSet<Int64>, devices, "Devices");
  DTO_FIELD(UnorderedSet<Int64>, links, "Links");
  // DTO_FIELD(UnorderedSet<Int64>, streams, "Streams");
  DTO_FIELD(String, image, "Image");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTopologyDto : public oatpp::DTO {
  DTO_INIT(ActTopologyDto, DTO);

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(UInt64, data_version, "DataVersion");
  DTO_FIELD(String, topology_name, "TopologyName");
  DTO_FIELD(UnorderedSet<Object<ActDeviceDto>>, devices, "Devices");
  DTO_FIELD(UnorderedSet<Object<ActLinkDto>>, links, "Links");
  // DTO_FIELD(UnorderedSet<Object<ActStreamDto>>, streams, "Streams");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActAppendDeviceDto : public oatpp::DTO {
  DTO_INIT(ActAppendDeviceDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(Object<ActIpv4Dto>, ipv4, "Ipv4");
  DTO_FIELD(Object<ActCoordinateDto>, coordinate, "Coordinate");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
// class ActAppendStreamDto : public oatpp::DTO {
//   DTO_INIT(ActAppendStreamDto, DTO)

//   DTO_FIELD(Int64, id, "Id");
//   DTO_FIELD(String, stream_name, "StreamName");
// };

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTopologyAppendDto : public oatpp::DTO {
  DTO_INIT(ActTopologyAppendDto, DTO);

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(UnorderedSet<Object<ActAppendDeviceDto>>, devices, "Devices");
  // DTO_FIELD(UnorderedSet<Object<ActAppendStreamDto>>, streams, "Streams");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSimpleTopologyDto : public oatpp::DTO {
  DTO_INIT(ActSimpleTopologyDto, DTO);

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, topology_name, "TopologyName");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSimpleTopologiesDto : public oatpp::DTO {
  DTO_INIT(ActSimpleTopologiesDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActSimpleTopologyDto>>, simple_topology_set, "SimpleTopologySet");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPatchContentListDto : public oatpp::DTO {
  DTO_INIT(ActPatchContentListDto, DTO)

  DTO_FIELD(List<String>, patch_content_list, "PatchContentList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTopologyStatusDto : public oatpp::DTO {
  DTO_INIT(ActTopologyStatusDto, DTO)

  DTO_FIELD(Boolean, loop, "Loop") = false;

  ActTopologyStatusDto() {}

  ActTopologyStatusDto(Boolean loop_) { this->loop = loop_; }
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
ENUM(TopologyTemplateTypeDtoEnum, v_int32, VALUE(kLine, 1, "Line"), VALUE(kRing, 2, "Ring"), VALUE(kStar, 3, "Star"),
     VALUE(kNone, 4, "None"))

class ActTopologyTemplateDto : public oatpp::DTO {
  DTO_INIT(ActTopologyTemplateDto, DTO);

  DTO_FIELD(Enum<TopologyTemplateTypeDtoEnum>, type, "Type") = TopologyTemplateTypeDtoEnum::kNone;
  DTO_FIELD(Object<ActCoordinateDto>, coordinate, "Coordinate");
  DTO_FIELD(Int64, device_profile_id, "DeviceProfileId");
  DTO_FIELD(Int64, device_count, "DeviceCount");
  DTO_FIELD(UnorderedSet<Int64>, ip_list, "IPList");
};

#include OATPP_CODEGEN_END(DTO)
