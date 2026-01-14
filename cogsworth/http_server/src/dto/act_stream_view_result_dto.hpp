/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_STREAM_VIEW_RESULT_DTO_HPP
#define ACT_STREAM_VIEW_RESULT_DTO_HPP

#include "./act_device_dto.hpp"
#include "./act_interval_dto.hpp"
#include "./act_link_dto.hpp"
#include "act_gate_control_dto.hpp"
#include "act_stream_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(StreamTypeDtoEnum, v_int32, VALUE(kUserDefinedVlan, 1, "UserDefinedVlan"),
     VALUE(kPerStreamPriority, 2, "PerStreamPriority"), VALUE(kSystemAssignedVlanTag, 3, "SystemAssignedVlanTag"))

ENUM(ActQueueIdDtoEnum, v_int32, VALUE(k0, 0, "0"), VALUE(k1, 1, "1"), VALUE(k2, 2, "2"), VALUE(k3, 3, "3"),
     VALUE(k4, 4, "4"), VALUE(k5, 5, "5"), VALUE(k6, 6, "6"), VALUE(k7, 7, "7"), VALUE(kTalker, 8, "Talker"),
     VALUE(kListener, 9, "Listener"))

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStreamDeviceInterfaceResultDto : public oatpp::DTO {
  DTO_INIT(ActStreamDeviceInterfaceResultDto, DTO);

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp");
  DTO_FIELD(UInt16, ingress_interface_id, "IngressInterfaceId");
  DTO_FIELD(UInt16, egress_interface_id, "EgressInterfaceId");
  DTO_FIELD(UInt8, priority_code_point, "PriorityCodePoint");
  DTO_FIELD(UInt32, start_time, "StartTime");
  DTO_FIELD(UInt32, stop_time, "StopTime");

  // properties
  DTO_FIELD(String, device_name, "DeviceName");
  DTO_FIELD(String, ingress_interface_name, "IngressInterfaceName");
  DTO_FIELD(String, egress_interface_name, "EgressInterfaceName");
  DTO_FIELD(UInt32, accumulated_latency, "AccumulatedLatency");
  DTO_FIELD(UInt32, stream_duration, "StreamDuration");

  DTO_FIELD(Enum<ActQueueIdDtoEnum>, queue_id, "QueueId");
  DTO_FIELD(UInt16, vlan_id, "VlanId");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStreamPathResultDto : public oatpp::DTO {
  DTO_INIT(ActStreamPathResultDto, DTO);

  DTO_FIELD(String, talker, "Talker");
  DTO_FIELD(String, listener, "Listener");
  DTO_FIELD(List<Object<ActStreamDeviceInterfaceResultDto>>, device_interface_results, "DeviceInterfaceResults");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStreamViewResultDto : public oatpp::DTO {
  DTO_INIT(ActStreamViewResultDto, DTO);

  DTO_FIELD(Int64, stream_id, "StreamId");

  DTO_FIELD(Enum<StreamTypeDtoEnum>, stream_type, "StreamType");

  DTO_FIELD(Enum<FrameTypeDtoEnum>, frame_type, "FrameType");
  DTO_FIELD(Enum<StreamTrafficTypeDtoEnum>, stream_traffic_type, "StreamTrafficType");

  DTO_FIELD(List<Object<ActStreamPathResultDto>>, stream_path_results, "StreamPathResults");

  DTO_FIELD(String, stream_name, "StreamName");
  DTO_FIELD(Object<ActIntervalDto>, interval, "Interval");
  DTO_FIELD(UInt8, time_slot_index, "TimeSlotIndex");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStreamViewResultsDto : public oatpp::DTO {
  DTO_INIT(ActStreamViewResultsDto, DTO);

  DTO_FIELD(List<Object<ActStreamViewResultDto>>, stream_view_results, "StreamViewResults");
  DTO_FIELD(UnorderedSet<Object<ActDeviceDto>>, devices, "Devices");
  DTO_FIELD(UnorderedSet<Object<ActLinkDto>>, links, "Links");
  DTO_FIELD(UInt32, cycle_time, "CycleTime");
};

#endif  // ACT_STREAM_VIEW_RESULT_DTO_HPP