/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActUndoRedoStatusDto : public oatpp::DTO {
  DTO_INIT(ActUndoRedoStatusDto, DTO)

  DTO_FIELD(Boolean, status, "Status");

  ActUndoRedoStatusDto() {}

  ActUndoRedoStatusDto(Boolean status_) { this->status = status_; }
};

#include OATPP_CODEGEN_END(DTO)
