#pragma once

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class ActServicePlatformGetPriceRequestDto : public oatpp::DTO {
  DTO_INIT(ActServicePlatformGetPriceRequestDto, DTO)

  DTO_FIELD(List<String>, model_list, "ModelList");
};

class ActServicePlatformGetPriceResponseDto : public oatpp::DTO {
  DTO_INIT(ActServicePlatformGetPriceResponseDto, DTO)

  DTO_FIELD(Fields<String>, data, "data");
};

#include OATPP_CODEGEN_END(DTO)
