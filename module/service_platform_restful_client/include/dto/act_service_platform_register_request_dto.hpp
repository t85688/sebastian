/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class ActServicePlatformRegisterRequestBaselineModelDto : public oatpp::DTO {
  DTO_INIT(ActServicePlatformRegisterRequestBaselineModelDto, DTO)

  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(UInt64, qty, "Qty");
  DTO_FIELD(String, price, "Price");
  DTO_FIELD(String, total_price, "TotalPrice");
};

class ActServicePlatformRegisterRequestBaselineDto : public oatpp::DTO {
  DTO_INIT(ActServicePlatformRegisterRequestBaselineDto, DTO)

  DTO_FIELD(UInt64, id, "Id");
  DTO_FIELD(String, name, "Name");
  DTO_FIELD(UInt64, created_time, "CreatedTime");
  DTO_FIELD(String, total_price, "TotalPrice");
  DTO_FIELD(List<Object<ActServicePlatformRegisterRequestBaselineModelDto>>, model_list, "ModelList");
};

class ActServicePlatformRegisterRequestDto : public oatpp::DTO {
  DTO_INIT(ActServicePlatformRegisterRequestDto, DTO)

  DTO_FIELD(String, project_name, "ProjectName");
  DTO_FIELD(List<Object<ActServicePlatformRegisterRequestBaselineDto>>, baseline_list, "BaselineList");
};

class ActServicePlatformUpdateProjectRequestDto : public oatpp::DTO {
  DTO_INIT(ActServicePlatformUpdateProjectRequestDto, DTO)

  DTO_FIELD(String, project_name, "ProjectName");
  DTO_FIELD(String, status, "Status");
  DTO_FIELD(List<Object<ActServicePlatformRegisterRequestBaselineDto>>, baseline_list, "BaselineList");
};

#include OATPP_CODEGEN_END(DTO)
