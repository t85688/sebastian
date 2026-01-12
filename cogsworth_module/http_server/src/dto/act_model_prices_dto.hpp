#pragma once

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class ActModelListDto : public oatpp::DTO {
  DTO_INIT(ActModelListDto, DTO)

  DTO_FIELD(List<String>, model_list, "ModelList");
};

class ActModelPricesDto : public oatpp::DTO {
  DTO_INIT(ActModelPricesDto, DTO)

  DTO_FIELD(Fields<String>, data, "Data");
};

#include OATPP_CODEGEN_END(DTO)
