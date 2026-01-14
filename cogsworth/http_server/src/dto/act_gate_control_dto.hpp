/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_GATE_CONTROL_DTO_HPP
#define ACT_GATE_CONTROL_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGateControlDto : public oatpp::DTO {
  DTO_INIT(ActGateControlDto, DTO);

  DTO_FIELD(UInt64, start_time, "StartTime");
  DTO_FIELD(UInt64, stop_time, "StopTime");
  DTO_FIELD(UInt8, gate_states_value, "GateStatesValue");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActInterfaceGateControlsDto : public oatpp::DTO {
  DTO_INIT(ActInterfaceGateControlsDto, DTO);

  DTO_FIELD(Int64, interface_id, "InterfaceId");
  DTO_FIELD(String, interface_name, "InterfaceName");
  DTO_FIELD(List<Object<ActGateControlDto>>, gate_controls, "GateControls");
};

#endif  // ACT_GATE_CONTROL_DTO_HPP