#pragma once

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class ActCreateGroupDto : public oatpp::DTO {
  DTO_INIT(ActCreateGroupDto, DTO);

  DTO_FIELD(String, Name);
  DTO_FIELD(Int64, ParentId);
  DTO_FIELD(String, BackgroundColor);
  DTO_FIELD(Int32, BackgroundColorAlpha);
};

class ActGroupDto : public oatpp::DTO {
  DTO_INIT(ActGroupDto, DTO);

  DTO_FIELD(Int64, Id);
  DTO_FIELD(String, Name);
  DTO_FIELD(Int64, ParentId);
  DTO_FIELD(String, BackgroundColor);
  DTO_FIELD(Int32, BackgroundColorAlpha);
  DTO_FIELD(List<Int64>, DeviceIds);
};

class ActGroupsDto : public oatpp::DTO {
  DTO_INIT(ActGroupsDto, DTO);

  DTO_FIELD(Fields<Object<ActGroupDto>>, Groups);
};

#include OATPP_CODEGEN_END(DTO)
