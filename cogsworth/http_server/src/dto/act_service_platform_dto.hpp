/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class ActServicePlatformLoginCheckDto : public oatpp::DTO {
  DTO_INIT(ActServicePlatformLoginCheckDto, DTO)

  DTO_FIELD(Boolean, status, "Status");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActServicePlatformLoginDto : public oatpp::DTO {
  DTO_INIT(ActServicePlatformLoginDto, DTO)

  DTO_FIELD(String, username, "Username") = "admin";
  DTO_FIELD(String, password, "Password") = "moxa";
};

class ActServicePlatformUserDto : public oatpp::DTO {
  DTO_INIT(ActServicePlatformUserDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, username, "Username");
  DTO_FIELD(String, role, "Role");
  DTO_FIELD(Vector<String>::ObjectWrapper, profiles, "Profiles");
  DTO_FIELD(Int64, data_version, "DataVersion");
};

class ActServicePlatformLoginResponseDto : public oatpp::DTO {
  DTO_INIT(ActServicePlatformLoginResponseDto, DTO)

  DTO_FIELD(String, token, "token");
  DTO_FIELD(Object<ActServicePlatformUserDto>, user, "user");
};

#include OATPP_CODEGEN_END(DTO)
