/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_COMPUTE_DTO_HPP
#define ACT_COMPUTE_DTO_HPP

#include "act_device_view_result_dto.hpp"
#include "act_gcl_result_dto.hpp"
#include "act_routing_result_dto.hpp"
#include "act_stream_view_result_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActComputedResultDto : public oatpp::DTO {
  DTO_INIT(ActComputedResultDto, DTO);

  DTO_FIELD(UnorderedSet<Object<ActRoutingResultDto>>, routing_results, "RoutingResults");
  DTO_FIELD(UnorderedSet<Object<ActGclResultDto>>, gcl_results, "GclResults");
  DTO_FIELD(UnorderedSet<Object<ActStreamViewResultDto>>, stream_view_results, "StreamViewResults");
  DTO_FIELD(UnorderedSet<Object<ActDeviceViewResultDto>>, device_view_results, "DeviceViewResults");

  DTO_FIELD(UnorderedSet<Object<ActDeviceDto>>, devices, "Devices");
  DTO_FIELD(UnorderedSet<Object<ActLinkDto>>, links, "Links");
  DTO_FIELD(UnorderedSet<Object<ActStreamDto>>, streams, "Streams");
  DTO_FIELD(UInt32, cycle_time, "CycleTime");
};

#endif  // ACT_COMPUTE_DTO_HPP