/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_DEVICE_VIEW_RESULT_DTO_HPP
#define ACT_DEVICE_VIEW_RESULT_DTO_HPP

#include "act_gate_control_dto.hpp"
#include "act_stream_view_result_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceInterfaceStreamResultDto : public oatpp::DTO {
  DTO_INIT(ActDeviceInterfaceStreamResultDto, DTO);

  // DTO_FIELD(Int64, interface_id, "InterfaceId");

  DTO_FIELD(Int64, stream_id, "StreamId");
  DTO_FIELD(String, stream_name, "StreamName");

  DTO_FIELD(UInt16, vlan_id, "VlanId");
  DTO_FIELD(UInt8, priority_code_point, "PriorityCodePoint");
  DTO_FIELD(Enum<ActQueueIdDtoEnum>, queue_id, "QueueId");
  DTO_FIELD(List<UInt16>, frame_offset_list, "FrameOffsetList");
  DTO_FIELD(UInt32, start_time, "StartTime");
  DTO_FIELD(UInt32, stop_time, "StopTime");
  DTO_FIELD(Enum<StreamTypeDtoEnum>, stream_type, "StreamType");

  DTO_FIELD(String, talker, "Talker");
  DTO_FIELD(List<String>, listeners, "Listeners");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceInterfaceResultDto : public oatpp::DTO {
  DTO_INIT(ActDeviceInterfaceResultDto, DTO);

  DTO_FIELD(Int64, interface_id, "InterfaceId");
  DTO_FIELD(Int64, interface_name, "InterfaceName");
  DTO_FIELD(UnorderedSet<Object<ActDeviceInterfaceStreamResultDto>>, stream_results, "StreamResults");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceViewResultDto : public oatpp::DTO {
  DTO_INIT(ActDeviceViewResultDto, DTO);

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(UnorderedSet<Object<ActDeviceInterfaceResultDto>>, interface_results, "InterfaceResults");

  DTO_FIELD(String, device_ip, "DeviceIp");
};

#endif  // ACT_DEVICE_VIEW_RESULT_DTO_HPP