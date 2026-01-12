/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_VLAN_CONFIG_DTO_HPP
#define ACT_VLAN_CONFIG_DTO_HPP

#include "./act_device_config_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActVlanConfigDto : public oatpp::DTO {
  DTO_INIT(ActVlanConfigDto, DTO)

  DTO_FIELD(Int32, vlan_id, "VlanId");
  DTO_FIELD(Enum<VlanPortTypeDtoEnum>, port_type, "PortType");
  DTO_FIELD(UnorderedSet<Int64>, devices, "Devices");
  DTO_FIELD(UnorderedSet<Int64>, ports, "Ports");
  DTO_FIELD(Int16, pvid, "Pvid");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActVlanConfigDeleteDto : public oatpp::DTO {
  DTO_INIT(ActVlanConfigDeleteDto, DTO)

  DTO_FIELD(Int32, vlan_id, "VlanId");
  DTO_FIELD(UnorderedSet<Int64>, devices, "Devices");
  DTO_FIELD(UnorderedSet<Int64>, ports, "Ports");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_VLAN_CONFIG_DTO_HPP */
