/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class ActServicePlatformLoginRequestDto : public oatpp::DTO {
  DTO_INIT(ActServicePlatformLoginRequestDto, DTO)

  DTO_FIELD(String, username);
  DTO_FIELD(String, password);
};

#include OATPP_CODEGEN_END(DTO)
