/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_MOXA_IEI_DTO_HPP
#define ACT_MOXA_IEI_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 *  For MOXA IEI devices
 */
class ActLoginRequestDto : public oatpp::DTO {
  DTO_INIT(ActLoginRequestDto, DTO)

  DTO_FIELD(String, username, "username");
  DTO_FIELD(String, password, "password");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 *  For MOXA IEI devices
 */
class ActSnmpServiceDto : public oatpp::DTO {
  DTO_INIT(ActSnmpServiceDto, DTO)

  DTO_FIELD(UInt8, mode, "mode");  // 1(Enabled), 2(Disabled), 3(ReadOnly)
  DTO_FIELD(UInt16, port, "port");
  DTO_FIELD(UInt16, transport_layer_protocol, "transportLayerProtocol");
};

// /**
//  *  Data Transfer Object. Object containing fields only.
//  *  Used in API for serialization/deserialization and validation
//  *  For MOXA IEI devices
//  */
// class ActMxRstpSwiftDto : public oatpp::DTO {
//   DTO_INIT(ActMxRstpSwiftDto, DTO)

//   DTO_FIELD(Boolean, rstp_config_swift, "rstpConfigSwift");
//   DTO_FIELD(Boolean, rstp_config_revert, "rstpConfigRevert");
// };

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 *  For MOXA IEI devices
 */
class ActManagementIpIpv4Dto : public oatpp::DTO {
  DTO_INIT(ActManagementIpIpv4Dto, DTO);

  DTO_FIELD(String, network_setting_mode, "networkSettingMode") = "Manual";
  DTO_FIELD(String, ip_address, "ipAddress");
  DTO_FIELD(String, netmask, "netmask");
  DTO_FIELD(String, gateway, "gateway");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 *  For MOXA IEI devices
 */
class ActServiceManagementDto : public oatpp::DTO {
  DTO_INIT(ActServiceManagementDto, DTO)

  DTO_FIELD(Object<ActSnmpServiceDto>, snmp_service, "snmpService");
};

// class ActNullDto : public oatpp::DTO {
//   DTO_INIT(ActNullDto, DTO)
// };
// /**
//  *  Data Transfer Object. Object containing fields only.
//  *  Used in API for serialization/deserialization and validation
//  *  For MOXA IEI devices
//  */
// class ActFirmwareUpgradeDto : public oatpp::DTO {
//   DTO_INIT(ActFirmwareUpgradeDto, DTO)
//   DTO_FIELD(Object<ActNullDto>, file_parameter, "file_parameter");
// };

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_MOXA_IEI_DTO_HPP */
