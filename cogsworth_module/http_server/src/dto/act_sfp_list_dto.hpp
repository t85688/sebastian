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
class ActMonitorFiberCheckEntryDto : public oatpp::DTO {
  DTO_INIT(ActMonitorFiberCheckEntryDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId") = 1;
  DTO_FIELD(String, device_ip, "DeviceIp");
  DTO_FIELD(UInt16, interface_id, "InterfaceId") = 1;
  DTO_FIELD(String, interface_name, "InterfaceName");

  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(String, serial_number, "SerialNumber");
  DTO_FIELD(String, wavelength, "Wavelength");
  DTO_FIELD(String, temperature_c, "TemperatureC");
  DTO_FIELD(String, temperature_f, "TemperatureF");
  DTO_FIELD(String, voltage, "Voltage");
  DTO_FIELD(String, tx_power, "TxPower");
  DTO_FIELD(String, rx_power, "RxPower");
  DTO_FIELD(String, temperatureLimit_c, "TemperatureLimitC");
  DTO_FIELD(String, temperatureLimit_f, "TemperatureLimitF");

  DTO_FIELD(List<String>, tx_power_limit, "TxPowerLimit");
  DTO_FIELD(List<String>, rx_power_limit, "RxPowerLimit");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSFPPairDto : public oatpp::DTO {
  DTO_INIT(ActSFPPairDto, DTO)

  DTO_FIELD(Object<ActMonitorFiberCheckEntryDto>, source, "Source");
  DTO_FIELD(Object<ActMonitorFiberCheckEntryDto>, target, "Target");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSFPListDto : public oatpp::DTO {
  DTO_INIT(ActSFPListDto, DTO)

  DTO_FIELD(List<Object<ActSFPPairDto>>, sfp_list, "SFPList");
};

#include OATPP_CODEGEN_END(DTO)
