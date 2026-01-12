/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_VLAN_VIEW_DTO_HPP
#define ACT_VLAN_VIEW_DTO_HPP

#include "./act_device_config_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActVlanViewDevicePortDto : public oatpp::DTO {
  DTO_INIT(ActVlanViewDevicePortDto, DTO)

  DTO_FIELD(Int64, port_id, "PortId");
  DTO_FIELD(Enum<VlanPortTypeDtoEnum>, device_type, "VlanPortType");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActVlanViewDeviceDto : public oatpp::DTO {
  DTO_INIT(ActVlanViewDeviceDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(UnorderedSet<Object<ActVlanViewDevicePortDto>>, ports, "Ports");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActVlanViewDto : public oatpp::DTO {
  DTO_INIT(ActVlanViewDto, DTO)

  DTO_FIELD(Int32, vlan_id, "VlanId");
  DTO_FIELD(UnorderedSet<Object<ActVlanViewDeviceDto>>, devices, "Devices");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActVlanViewsDto : public oatpp::DTO {
  DTO_INIT(ActVlanViewsDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActVlanViewDto>>, vlan_view_set, "VlanViewSet");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActVlanViewIdsDto : public oatpp::DTO {
  DTO_INIT(ActVlanViewIdsDto, DTO)

  DTO_FIELD(List<Int32>, vlan_id_list, "VlanIdList");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_VLAN_VIEW_DTO_HPP */
