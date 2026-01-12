/* Copyright (C) MOXA Inc. All rights reserved.
   This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
   See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include "./act_device_profile_dto.hpp"
#include "./act_system_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class ActInterfaceItemDto : public oatpp::DTO {
  DTO_INIT(ActInterfaceItemDto, DTO)

  DTO_FIELD(List<String>, cable_types, "CableTypes");
  DTO_FIELD(Int32, interface_id, "InterfaceId");
  DTO_FIELD(String, interface_name, "InterfaceName");
  DTO_FIELD(List<Int32>, support_speeds, "SupportSpeeds");
};

class ActPowerDeviceDto : public oatpp::DTO {
  DTO_INIT(ActPowerDeviceDto, DTO)

  DTO_FIELD(Int32, id, "Id");
  DTO_FIELD(String, icon_name, "IconName");
  DTO_FIELD(String, data_version, "DataVersion");
  DTO_FIELD(Boolean, purchasable, "Purchasable");
  DTO_FIELD(Enum<ActServiceProfileForDeviceProfileDtoEnum>, profiles,
            "Profiles") = ActServiceProfileForDeviceProfileDtoEnum::kSelfPlanning;
  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(List<String>, mount_type, "MountType");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(Object<OperatingTemperatureCDto>, operating_temperature_c, "OperatingTemperatureC");
  DTO_FIELD(Boolean, built_in, "BuiltIn");
  DTO_FIELD(Boolean, hide, "Hide");
  DTO_FIELD(Boolean, certificate, "Certificate");
  DTO_FIELD(String, vendor, "Vendor");
  DTO_FIELD(Enum<DeviceTypeDtoEnum>, device_type, "DeviceType");
  DTO_FIELD(String, max_port_speed, "MaxPortSpeed");
  DTO_FIELD(List<Object<ActInterfaceItemDto>>, interfaces, "Interfaces");
};

class ActPowerDevicesDto : public oatpp::DTO {
  DTO_INIT(ActPowerDevicesDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActPowerDeviceDto>>, power_device_set, "PowerDeviceSet");
};

#include OATPP_CODEGEN_END(DTO)