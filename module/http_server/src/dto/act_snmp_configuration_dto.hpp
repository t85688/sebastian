/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_SNMP_CONFIGURATION_DTO_HPP
#define ACT_SNMP_CONFIGURATION_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)
ENUM(SnmpVersionDtoEnum, v_int32, VALUE(kv1, 1, "v1"), VALUE(kv2c, 2, "v2c"), VALUE(kv3, 3, "v3"))
ENUM(SnmpAuthenticationTypeDtoEnum, v_int32, VALUE(kNone, 1, "None"), VALUE(kMD5, 2, "MD5"), VALUE(kSHA1, 3, "SHA1"))
ENUM(SnmpDataEncryptionTypeDtoEnum, v_int32, VALUE(kNone, 1, "None"), VALUE(kDES, 2, "DES"), VALUE(kAES, 3, "AES"))

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSnmpConfigurationDto : public oatpp::DTO {
  DTO_INIT(ActSnmpConfigurationDto, DTO)
  DTO_FIELD(Boolean, default_setting, "DefaultSetting") = true;
  DTO_FIELD(Enum<SnmpVersionDtoEnum>, version, "Version") = SnmpVersionDtoEnum::kv2c;
  DTO_FIELD(UInt16, port, "Port") = 161;

  // SNMPv1, SNMPv2c
  DTO_FIELD(String, read_community, "ReadCommunity") = "public";
  DTO_FIELD(String, write_community, "WriteCommunity") = "private";

  // SNMPv3
  DTO_FIELD(String, user_name, "Username");
  DTO_FIELD(Enum<SnmpAuthenticationTypeDtoEnum>, authentication_type,
            "AuthenticationType") = SnmpAuthenticationTypeDtoEnum::kNone;
  DTO_FIELD(String, authentication_password, "AuthenticationPassword");
  DTO_FIELD(Enum<SnmpDataEncryptionTypeDtoEnum>, data_encryption_type,
            "DataEncryptionType") = SnmpDataEncryptionTypeDtoEnum::kNone;
  DTO_FIELD(String, data_encryption_key, "DataEncryptionKey");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSnmpTrapConfigurationDto : public oatpp::DTO {
  DTO_INIT(ActSnmpTrapConfigurationDto, DTO)

  DTO_FIELD(Enum<SnmpVersionDtoEnum>, version, "Version") = SnmpVersionDtoEnum::kv2c;
  DTO_FIELD(UInt16, port, "Port") = 162;

  // SNMPv1, SNMPv2c
  DTO_FIELD(String, trap_community, "TrapCommunity") = "public";
  // SNMPv3
  DTO_FIELD(String, user_name, "Username");
  DTO_FIELD(Enum<SnmpAuthenticationTypeDtoEnum>, authentication_type,
            "AuthenticationType") = SnmpAuthenticationTypeDtoEnum::kNone;
  DTO_FIELD(String, authentication_password, "AuthenticationPassword");
  DTO_FIELD(Enum<SnmpDataEncryptionTypeDtoEnum>, data_encryption_type,
            "DataEncryptionType") = SnmpDataEncryptionTypeDtoEnum::kNone;
  DTO_FIELD(String, data_encryption_key, "DataEncryptionKey");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_SNMP_CONFIGURATION_DTO_HPP */
