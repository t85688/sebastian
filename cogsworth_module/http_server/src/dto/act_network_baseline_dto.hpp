/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "./act_service_profile_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(ActBaselineModeDtoEnum, v_int32, VALUE(kDesign, 1, "Design"), VALUE(kOperation, 2, "Operation"))

class ActNetworkBaselineInfoDto : public oatpp::DTO {
  DTO_INIT(ActNetworkBaselineInfoDto, DTO);

  DTO_FIELD(String, name, "Name");
  DTO_FIELD(String, description, "Description");
};

class ActNetworkBaselineDeviceDto : public oatpp::DTO {
  DTO_INIT(ActNetworkBaselineDeviceDto, DTO);

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, ip_address, "IpAddress");
  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(String, firmware_version, "FirmwareVersion");
  DTO_FIELD(String, configuration, "Configuration");
};

class ActNetworkBaselineDto : public oatpp::DTO {
  DTO_INIT(ActNetworkBaselineDto, DTO);

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(UInt64, data_version, "DataVersion");
  DTO_FIELD(String, name, "Name");
  DTO_FIELD(UInt64, date, "Date");
  DTO_FIELD(String, created_user, "CreatedUser");
  DTO_FIELD(String, project_id, "ProjectId");
  DTO_FIELD(UnorderedSet<Object<ActNetworkBaselineDeviceDto>>, devices, "Devices");
  DTO_FIELD(Boolean, activate, "Activate");
  DTO_FIELD(String, activated_user, "ActivatedUser");
  DTO_FIELD(UInt64, activated_date, "ActivatedDate");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(Enum<ActBaselineModeDtoEnum>, mode, "Mode") = ActBaselineModeDtoEnum::kDesign;
};

// class ActBaselineBOMDetailEntryDto : public oatpp::DTO {
//   DTO_INIT(ActBaselineBOMDetailEntryDto, DTO);

//   DTO_FIELD(String, device_model, "DeviceModel");
//   DTO_FIELD(UInt32, quantity, "Quantity");
//   DTO_FIELD(String, unit_price, "UnitPrice");
//   DTO_FIELD(String, total_price, "TotalPrice");
// };

class ActBaselineBOMDetailDto : public oatpp::DTO {
  DTO_INIT(ActBaselineBOMDetailDto, DTO);

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, name, "Name");
  DTO_FIELD(UInt64, date, "Date");
  DTO_FIELD(String, created_user, "CreatedUser");
  DTO_FIELD(String, project_id, "ProjectId");
  DTO_FIELD(String, total_price, "TotalPrice");
  DTO_FIELD(Fields<Object<ActSkuQuantityDto>>, sku_quantities, "SkuQuantities");
};

class ActSimpleNetworkBaselineDto : public oatpp::DTO {
  DTO_INIT(ActSimpleNetworkBaselineDto, DTO);

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, name, "Name");
  DTO_FIELD(UInt64, date, "Date");
  DTO_FIELD(String, project_id, "ProjectId");
  DTO_FIELD(String, created_user, "CreatedUser");
  DTO_FIELD(Boolean, activate, "Activate");
  DTO_FIELD(String, activated_user, "ActivatedUser");
  DTO_FIELD(UInt64, activated_date, "ActivatedDate");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(Enum<ActBaselineModeDtoEnum>, mode, "Mode") = ActBaselineModeDtoEnum::kDesign;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActNetworkBaselineListDto : public oatpp::DTO {
  DTO_INIT(ActNetworkBaselineListDto, DTO)

  DTO_FIELD(List<Object<ActSimpleNetworkBaselineDto>>, network_baseline_list, "NetworkBaselineList");
};

class ActBaselineProjectDiffDetailDto : public oatpp::DTO {
  DTO_INIT(ActBaselineProjectDiffDetailDto, DTO);

  DTO_FIELD(Boolean, bom, "BOM");
  DTO_FIELD(Boolean, device_config, "DeviceConfig");
  DTO_FIELD(Boolean, topology_device, "TopologyDevice");
  DTO_FIELD(Boolean, topology_link, "TopologyLink");
  DTO_FIELD(Boolean, project_setting, "ProjectSetting");
};

class ActBaselineProjectDiffReportDto : public oatpp::DTO {
  DTO_INIT(ActBaselineProjectDiffReportDto, DTO);

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(Int64, project_id, "ProjectId");
  DTO_FIELD(Boolean, has_diff, "HasDiff");
  DTO_FIELD(Object<ActBaselineProjectDiffDetailDto>, diff_detail, "DiffDetail");
};

#include OATPP_CODEGEN_END(DTO)
