/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_USER_DTO_HPP
#define ACT_USER_DTO_HPP

// #include "act_user.hpp"
#include "./act_system_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)
ENUM(RoleDtoEnum, v_int32, VALUE(ADMIN, 1, "Admin"),  //<- you may use qualifiers here
     VALUE(SUPERVISOR, 2, "Supervisor"), VALUE(USER, 3, "User"))

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCreateUserDto : public oatpp::DTO {
  DTO_INIT(ActCreateUserDto, DTO)

  DTO_FIELD(String, username, "Username") = "admin";
  DTO_FIELD(String, password, "Password") = "moxa";
  DTO_FIELD(String, organization, "Organization") = "Moxa";
  DTO_FIELD(Enum<RoleDtoEnum>, role, "Role") = RoleDtoEnum::ADMIN;
  DTO_FIELD(List<Enum<ActServiceProfileForLicenseDtoEnum>>, profiles, "Profiles");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActUserDto : public oatpp::DTO {
  DTO_INIT(ActUserDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(Int64, organization_id, "OrganizationId");
  DTO_FIELD(String, username, "Username") = "admin";
  DTO_FIELD(String, password, "Password") = "moxa";
  DTO_FIELD(Enum<RoleDtoEnum>, role, "Role") = RoleDtoEnum::ADMIN;
  DTO_FIELD(List<Enum<ActServiceProfileForLicenseDtoEnum>>, profiles, "Profiles");
  DTO_FIELD(UInt64, data_version, "DataVersion");
};

class ActUsersDto : public oatpp::DTO {
  DTO_INIT(ActUsersDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActUserDto>>, user_set, "UserSet");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSimpleUserDto : public oatpp::DTO {
  DTO_INIT(ActSimpleUserDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(Int64, organization_id, "OrganizationId");
  DTO_FIELD(String, username, "Username") = "admin";
  DTO_FIELD(Enum<RoleDtoEnum>, role, "Role") = RoleDtoEnum::ADMIN;
  DTO_FIELD(List<Enum<ActServiceProfileForLicenseDtoEnum>>, profiles, "Profiles");
};

class ActSimpleUsersDto : public oatpp::DTO {
  DTO_INIT(ActSimpleUsersDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActSimpleUserDto>>, user_set, "UserSet");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_USER_DTO_HPP */
