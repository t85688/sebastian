/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_ROUTING_RESULT_DTO_HPP
#define ACT_ROUTING_RESULT_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActRedundantPathDto : public oatpp::DTO {
  DTO_INIT(ActRedundantPathDto, DTO);

  DTO_FIELD(List<Int64>, link_ids, "LinkIds");
  DTO_FIELD(List<Int64>, device_ids, "DeviceIds");
  DTO_FIELD(UInt16, vlan_id, "VlanId");
  // DTO_FIELD(UInt16, zones, "Zones");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActRoutingPathDto : public oatpp::DTO {
  DTO_INIT(ActRoutingPathDto, DTO);

  DTO_FIELD(List<Object<ActRedundantPathDto>>, redundantpaths, "RedundantPaths");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActRoutingResultDto : public oatpp::DTO {
  DTO_INIT(ActRoutingResultDto, DTO);

  DTO_FIELD(String, stream_name, "StreamName");
  DTO_FIELD(Int64, stream_id, "StreamId");
  DTO_FIELD(UInt16, vlan_id, "VlanId");
  DTO_FIELD(UInt16, priority_code_point, "PriorityCodePoint");
  DTO_FIELD(Boolean, cb, "CB");
  DTO_FIELD(Boolean, multicast, "Multicast");
  DTO_FIELD(List<Object<ActRoutingPathDto>>, routing_paths, "RoutingPaths");
};

#endif  // ACT_ROUTING_RESULT_DTO_HPP