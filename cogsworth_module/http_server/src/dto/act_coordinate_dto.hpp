/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_COORDINATE_DTO_HPP
#define ACT_COORDINATE_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCoordinateDto : public oatpp::DTO {
  DTO_INIT(ActCoordinateDto, DTO)

  DTO_FIELD(Int64, x, "X");
  DTO_FIELD(Int64, y, "Y");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_COORDINATE_DTO_HPP */
