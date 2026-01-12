/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_INTERVAL_DTO_HPP
#define ACT_INTERVAL_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActIntervalDto : public oatpp::DTO {
  DTO_INIT(ActIntervalDto, DTO)

  DTO_FIELD(UInt32, denominator, "Denominator") = 1;
  DTO_FIELD(UInt32, numerator, "Numerator");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_INTERVAL_DTO_HPP */
