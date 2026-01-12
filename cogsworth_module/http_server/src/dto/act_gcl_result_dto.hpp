/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_GCL_RESULT_DTO_HPP
#define ACT_GCL_RESULT_DTO_HPP

#include "act_gate_control_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGclResultDto : public oatpp::DTO {
  DTO_INIT(ActGclResultDto, DTO);

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp");
  DTO_FIELD(List<Object<ActInterfaceGateControlsDto>>, interface_gate_controls, "InterfaceGateControls");
};

#endif  // ACT_GCL_RESULT_DTO_HPP