/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_SYSTEM_DTO_HPP
#define ACT_SYSTEM_DTO_HPP

// #include "act_user.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(ActServiceProfileForLicenseDtoEnum, v_int32, VALUE(kSelfPlanning, 0, "SelfPlanning"),
     VALUE(kGeneral, 2, "General"), VALUE(kFoxboro, 3, "Foxboro"))

ENUM(ActServiceProfileForDeviceProfileDtoEnum, v_int32, VALUE(kSelfPlanning, 0, "SelfPlanning"),
     VALUE(kUnVerifiedGeneral, 2, "UnVerifiedGeneral"), VALUE(kVerifiedGeneral, 3, "VerifiedGeneral"),
     VALUE(kUnVerifiedFoxboro, 4, "UnVerifiedFoxboro"), VALUE(kVerifiedFoxboro, 5, "VerifiedFoxboro"))

ENUM(ActDeploymentTypeDtoEnum, v_int32, VALUE(kLocal, 0, "Local"), VALUE(kServer, 1, "Server"))

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActVersionDto : public oatpp::DTO {
  DTO_INIT(ActVersionDto, DTO)

  DTO_FIELD(String, ActVersion) = "Testing_Version";
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSystemDto : public oatpp::DTO {
  DTO_INIT(ActSystemDto, DTO)

  DTO_FIELD(String, ActVersion) = "Testing_Version";
  DTO_FIELD(Boolean, web, "Web");
  DTO_FIELD(Boolean, opcua, "Opcua");
  DTO_FIELD(UInt64, data_version, "DataVersion");
  DTO_FIELD(Boolean, auto_save, "AutoSave");
  DTO_FIELD(UInt64, idle_timeout, "IdleTimeout");
  DTO_FIELD(UInt64, hard_timeout, "HardTimeout");
  DTO_FIELD(UInt16, max_token_size, "MaxTokenSize");
  DTO_FIELD(String, serial_number, "SerialNumber");
  DTO_FIELD(String, IntelligentEndpoint) = ACT_DEFAULT_INTELLIGENT_ENDPOINT;
  DTO_FIELD(String, IntelligentLocalEndpoint) = ACT_DEFAULT_INTELLIGENT_LOCAL_ENDPOINT;
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_SYSTEM_DTO_HPP */
