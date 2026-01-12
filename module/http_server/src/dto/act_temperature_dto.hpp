/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class OperatingTemperatureCDto : public oatpp::DTO {
  DTO_INIT(OperatingTemperatureCDto, DTO)

  DTO_FIELD(Int32, min, "Min");
  DTO_FIELD(Int32, max, "Max");
};

#include OATPP_CODEGEN_END(DTO)
