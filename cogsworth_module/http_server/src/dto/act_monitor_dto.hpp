/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_sfp_list_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(ActLinkStatusTypeDtoEnum, v_int32, VALUE(kUp, 1, "Up"), VALUE(kDown, 2, "Down"))

class ActMonitorDeviceBasicInfoDto : public oatpp::DTO {
  DTO_INIT(ActMonitorDeviceBasicInfoDto, DTO)

  DTO_FIELD(String, mac_address, "MacAddress");
  DTO_FIELD(String, firmware_version, "FirmwareVersion");
  DTO_FIELD(String, serial_number, "SerialNumber");
  DTO_FIELD(String, device_name, "DeviceName");
  DTO_FIELD(String, location, "Location");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(String, contact_information, "ContactInformation");
  DTO_FIELD(String, system_uptime, "SystemUptime");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActMonitorSFPStatusEntryDto : public oatpp::DTO {
  DTO_INIT(ActMonitorSFPStatusEntryDto, DTO)

  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(String, wavelength, "Wavelength");
  DTO_FIELD(String, temperature_c, "TemperatureC");
  DTO_FIELD(String, temperature_f, "TemperatureF");
  DTO_FIELD(String, voltage, "Voltage");
  DTO_FIELD(String, tx_power, "TxPower");
  DTO_FIELD(String, rx_power, "RxPower");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActMonitorDeviceSFPStatusDto : public oatpp::DTO {
  DTO_INIT(ActMonitorDeviceSFPStatusDto, DTO)

  DTO_FIELD(Fields<Object<ActMonitorSFPStatusEntryDto>>, fiber_check, "FiberCheck");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActMonitorDevicePortDetailStatusDto : public oatpp::DTO {
  DTO_INIT(ActMonitorDevicePortDetailStatusDto, DTO)

  DTO_FIELD(Enum<ActLinkStatusTypeDtoEnum>, link_status, "LinkStatus");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActMonitorDevicePortStatusDto : public oatpp::DTO {
  DTO_INIT(ActMonitorDevicePortStatusDto, DTO)

  DTO_FIELD(Fields<Object<ActMonitorDevicePortDetailStatusDto>>, port_status, "PortStatus");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceMonitorTrafficEntryDto : public oatpp::DTO {
  DTO_INIT(ActDeviceMonitorTrafficEntryDto, DTO)

  DTO_FIELD(Int64, tx_total_octets, "TxTotalOctets");
  DTO_FIELD(Int64, tx_total_packets, "TxTotalPackets");
  DTO_FIELD(Float32, traffic_utilization, "TrafficUtilization");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActMonitorDeviceTrafficStatusDto : public oatpp::DTO {
  DTO_INIT(ActMonitorDeviceTrafficStatusDto, DTO)

  DTO_FIELD(Fields<Object<ActDeviceMonitorTrafficEntryDto>>, traffic_map, "TrafficMap");
};

#include OATPP_CODEGEN_END(DTO)
