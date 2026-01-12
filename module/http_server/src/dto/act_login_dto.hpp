/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_LOGIN_DTO_HPP
#define ACT_LOGIN_DTO_HPP

#include "./act_user_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActLoginDto : public oatpp::DTO {
  DTO_INIT(ActLoginDto, DTO)

  DTO_FIELD(String, username, "Username") = "admin";
  DTO_FIELD(String, password, "Password") = "moxa";
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTokenDto : public oatpp::DTO {
  DTO_INIT(ActTokenDto, DTO)

  DTO_FIELD(String, token, "Token");
  DTO_FIELD(Enum<RoleDtoEnum>, role, "Role");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_LOGIN_DTO_HPP */
