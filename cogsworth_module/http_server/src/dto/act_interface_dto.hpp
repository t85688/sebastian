/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_INTERFACE_DTO_HPP
#define ACT_INTERFACE_DTO_HPP

#include "./act_link_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActInterfaceDto : public oatpp::DTO {
  DTO_INIT(ActInterfaceDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId") = 1;
  DTO_FIELD(UInt16, interface_id, "InterfaceId") = 1;
  DTO_FIELD(String, interface_name, "InterfaceName") = "Eth1";
  DTO_FIELD(String, mac_address, "MacAddress") = "11-22-33-44-55-66";
  DTO_FIELD(String, ip_address, "IpAddress") = "0.0.0.0";
  DTO_FIELD(Boolean, used, "Used") = false;
  DTO_FIELD(Boolean, management, "Management") = false;
  DTO_FIELD(Boolean, root_guard, "RootGuard") = false;

  DTO_FIELD(List<Int64>, support_speeds, "SupportSpeeds");
  DTO_FIELD(Enum<CableTypeDtoEnum>, cable_type, "CableType");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActInterfacePropertyDto : public oatpp::DTO {
  DTO_INIT(ActInterfacePropertyDto, DTO)

  DTO_FIELD(String, interface_number, "InterfaceNumber");
  DTO_FIELD(String, interface_name, "InterfaceName");
  DTO_FIELD(List<Int64>, support_speeds, "SupportSpeeds");
  DTO_FIELD(Enum<CableTypeDtoEnum>, cable_type, "CableType");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_INTERFACE_DTO_HPP */
