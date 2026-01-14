/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_DEVICE_DTO_HPP
#define ACT_DEVICE_DTO_HPP

#include "./act_coordinate_dto.hpp"
#include "./act_device_profile_dto.hpp"
#include "./act_interface_dto.hpp"
#include "./act_snmp_configuration_dto.hpp"
#include "./act_stream_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceAccountDto : public oatpp::DTO {
  DTO_INIT(ActDeviceAccountDto, DTO)

  DTO_FIELD(String, username, "Username") = "admin";
  DTO_FIELD(String, password, "Password") = "moxa";
  DTO_FIELD(Boolean, default_setting, "DefaultSetting") = true;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActNetconfOverTLSDto : public oatpp::DTO {
  DTO_INIT(ActNetconfOverTLSDto, DTO)

  DTO_FIELD(UInt16, tls_port, "TLSPort") = 6513;
  DTO_FIELD(String, key_file, "KeyFile");
  DTO_FIELD(String, cert_file, "CertFile");
  DTO_FIELD(String, ca_certs, "CaCerts");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActNetconfOverSSHDto : public oatpp::DTO {
  DTO_INIT(ActNetconfOverSSHDto, DTO)

  DTO_FIELD(UInt16, ssh_port, "SSHPort") = 830;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActNetconfConfigurationDto : public oatpp::DTO {
  DTO_INIT(ActNetconfConfigurationDto, DTO)

  DTO_FIELD(Boolean, tls, "TLS") = false;
  DTO_FIELD(Object<ActNetconfOverSSHDto>, netconf_over_ssh, "NetconfOverSSH");
  DTO_FIELD(Object<ActNetconfOverTLSDto>, netconf_over_tls, "NetconfOverTLS");
  DTO_FIELD(Boolean, default_setting, "DefaultSetting") = true;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActRestfulConfigurationDto : public oatpp::DTO {
  DTO_INIT(ActRestfulConfigurationDto, DTO)

  DTO_FIELD(Enum<RestfulProtocolDtoEnum>, protocol, "Protocol") = RestfulProtocolDtoEnum::kHTTPS;
  DTO_FIELD(UInt16, port, "Port") = 443;
  DTO_FIELD(Boolean, default_setting, "DefaultSetting") = true;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActIpv4Dto : public oatpp::DTO {
  DTO_INIT(ActIpv4Dto, DTO)

  DTO_FIELD(String, ip_address, "IpAddress") = "10.0.0.1";
  DTO_FIELD(String, subnet_mask, "SubnetMask") = "255.255.255.0";
  DTO_FIELD(String, gateway, "Gateway") = "10.0.0.254";
  DTO_FIELD(String, dns1, "DNS1") = "";
  DTO_FIELD(String, dns2, "DNS2") = "";
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDevicePropertyDto : public oatpp::DTO {
  DTO_INIT(ActDevicePropertyDto, DTO)

  DTO_FIELD(String, model_name, "ModelName") = "TSN-G5008-2GTXSFP";
  DTO_FIELD(Enum<DeviceClusterDtoEnum>, device_cluster, "DeviceCluster") = DeviceClusterDtoEnum::kDefault;
  DTO_FIELD(Enum<MountTypeDtoEnum>, mount_type, "MountType") = MountTypeDtoEnum::kDinRail;
  DTO_FIELD(UInt16, gate_control_list_length, "GateControlListLength") = 1024;
  DTO_FIELD(UInt16, number_of_queue, "NumberOfQueue") = 8;
  DTO_FIELD(UInt16, tick_granularity, "TickGranularity") = 8;
  DTO_FIELD(UInt16, stream_priority_config_ingress_index_max, "StreamPriorityConfigIngressIndexMax") = 10;
  DTO_FIELD(UInt64, gcl_offset_min_duration, "GCLOffsetMinDuration");
  DTO_FIELD(UInt64, gcl_offset_max_duration, "GCLOffsetMaxDuration") = 999999999;
  DTO_FIELD(UInt16, max_vlan_cfg_size, "MaxVlanCfgSize") = 256;
  DTO_FIELD(UInt16, per_queue_size, "PerQueueSize") = 8000;
  DTO_FIELD(UInt8, ptp_queue_id, "PtpQueueId") = 6;

  DTO_FIELD(Object<ActTrafficSpecificationDto>, traffic_specification, "TrafficSpecification");
  DTO_FIELD(Object<ActIeee802VlanTagDto>, ieee802_vlan_tag, "Ieee802VlanTag");
  DTO_FIELD(Object<ActFeatureGroupDto>, feature_group, "FeatureGroup");
  DTO_FIELD(UnorderedSet<Int32>, reserved_vlan, "ReservedVlan");

  // Newly added fields from ActDeviceProperty
  DTO_FIELD(String, l2_family, "L2Family");
  DTO_FIELD(String, l3_series, "L3Series");
  DTO_FIELD(String, l4_series, "L4Series");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(String, vendor, "Vendor");
  DTO_FIELD(Boolean, certificate, "Certificate");
  DTO_FIELD(Object<ActIeee802Dot1cbDto>, ieee802_1cb, "Ieee802Dot1cb");
  DTO_FIELD(Fields<Object<ActProcessingDelayDto>>, processing_delay_map, "ProcessingDelayMap");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceInfoDto : public oatpp::DTO {
  DTO_INIT(ActDeviceInfoDto, DTO)

  DTO_FIELD(String, location, "Location");
  DTO_FIELD(String, serial_number, "SerialNumber");
  DTO_FIELD(String, product_revision, "ProductRevision");
  DTO_FIELD(String, system_uptime, "SystemUptime");
  DTO_FIELD(String, redundant_protocol, "RedundantProtocol");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceEthernetModuleDto : public oatpp::DTO {
  DTO_INIT(ActDeviceEthernetModuleDto, DTO)

  DTO_FIELD(String, module_name, "ModuleName");
  DTO_FIELD(String, serial_number, "SerialNumber");
  DTO_FIELD(String, product_revision, "ProductRevision");
  DTO_FIELD(String, status, "Status");
  DTO_FIELD(Int64, module_id, "ModuleId");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDevicePowerModuleDto : public oatpp::DTO {
  DTO_INIT(ActDevicePowerModuleDto, DTO)

  DTO_FIELD(String, module_name, "ModuleName");
  DTO_FIELD(String, serial_number, "SerialNumber");
  DTO_FIELD(String, product_revision, "ProductRevision");
  DTO_FIELD(String, status, "Status");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActModularInfoDto : public oatpp::DTO {
  DTO_INIT(ActModularInfoDto, DTO)

  DTO_FIELD(Fields<Object<ActDeviceEthernetModuleDto>>, ethernet, "Ethernet");
  DTO_FIELD(Fields<Object<ActDevicePowerModuleDto>>, power, "Power");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceStatusDto : public oatpp::DTO {
  DTO_INIT(ActDeviceStatusDto, DTO)

  DTO_FIELD(Boolean, icmp_status, "ICMPStatus");
  DTO_FIELD(Boolean, restful_status, "RESTfulStatus");
  DTO_FIELD(Boolean, snmp_status, "SNMPStatus");
  DTO_FIELD(Boolean, netconf_status, "NETCONFStatus");
  DTO_FIELD(Boolean, new_moxa_command_status, "NewMOXACommandStatus");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActUpdateDeviceCoordinateDto : public oatpp::DTO {
  DTO_INIT(ActUpdateDeviceCoordinateDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(Object<ActCoordinateDto>, coordinate, "Coordinate");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActUpdateDeviceCoordinatesDto : public oatpp::DTO {
  DTO_INIT(ActUpdateDeviceCoordinatesDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActUpdateDeviceCoordinateDto>>, devices, "Devices");
  DTO_FIELD(Boolean, sync_to_websocket, "NotifyUIUpdate") = false;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceIdsDto : public oatpp::DTO {
  DTO_INIT(ActDeviceIdsDto, DTO)

  DTO_FIELD(List<Int64>, device_ids, "DeviceIds");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSkipDeviceIdsDto : public oatpp::DTO {
  DTO_INIT(ActSkipDeviceIdsDto, DTO)

  DTO_FIELD(List<Int64>, skip_device_ids, "SkipDeviceIds");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCreateDeviceDto : public oatpp::DTO {
  DTO_INIT(ActCreateDeviceDto, DTO)

  DTO_FIELD(String, device_name, "DeviceName") = "moxa";
  DTO_FIELD(String, device_alias, "DeviceAlias") = "Device Alias";
  DTO_FIELD(String, firmware_version, "FirmwareVersion") = "";
  DTO_FIELD(Int64, device_profile_id, "DeviceProfileId");
  DTO_FIELD(Object<ActIpv4Dto>, ipv4, "Ipv4");

  DTO_FIELD(Int16, tier, "Tier");
  DTO_FIELD(Object<ActCoordinateDto>, coordinate, "Coordinate");
  DTO_FIELD(Int64, group_id, "GroupId");

  DTO_FIELD(List<Object<ActInterfaceDto>>, interfaces, "Interfaces");
  DTO_FIELD(Object<ActDeviceAccountDto>, account, "Account");
  DTO_FIELD(Object<ActNetconfConfigurationDto>, netconf_configuration, "NetconfConfiguration");
  DTO_FIELD(Object<ActSnmpConfigurationDto>, snmp_configuration, "SnmpConfiguration");
  DTO_FIELD(Object<ActRestfulConfigurationDto>, restful_configuration, "RestfulConfiguration");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCreateDeviceRequestDto : public oatpp::DTO {
  DTO_INIT(ActCreateDeviceRequestDto, DTO)

  DTO_FIELD(Object<ActCreateDeviceDto>, device, "Device");
  DTO_FIELD(Boolean, from_bag, "FromBag") = false;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCreateDevicesRequestDto : public oatpp::DTO {
  DTO_INIT(ActCreateDevicesRequestDto, DTO)

  DTO_FIELD(List<Object<ActCreateDeviceDto>>, device_list, "DeviceList");
  DTO_FIELD(Boolean, from_bag, "FromBag") = false;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeleteDevicesRequestDto : public oatpp::DTO {
  DTO_INIT(ActDeleteDevicesRequestDto, DTO)

  DTO_FIELD(List<Int64>, device_ids, "DeviceIds");
  DTO_FIELD(Boolean, to_bag, "ToBag") = false;
};

class ActDeviceModularConfigurationDto : public oatpp::DTO {
  DTO_INIT(ActDeviceModularConfigurationDto, DTO)

  // 將下列 QMap<qint64, qint64> 轉換成 oatpp::DTO
  DTO_FIELD(Fields<Int64>, ethernet, "Ethernet");
  DTO_FIELD(Fields<Int64>, power, "Power");
  // ACT_JSON_QT_DICT(QMap, qint64, qint64, power, Power);  ///< The Ethernet module map <SlotID, PowerModule ID>
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceDto : public oatpp::DTO {
  DTO_INIT(ActDeviceDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, device_name, "DeviceName") = "moxa";
  DTO_FIELD(String, device_alias, "DeviceAlias") = "Device Alias";
  DTO_FIELD(String, firmware_version, "FirmwareVersion") = "";
  DTO_FIELD(Int64, device_profile_id, "DeviceProfileId");
  DTO_FIELD(Int64, firmware_feature_profile_id, "FirmwareFeatureProfileId") = -1;
  DTO_FIELD(Enum<DeviceTypeDtoEnum>, device_type, "DeviceType") = DeviceTypeDtoEnum::kTSNSwitch;

  DTO_FIELD(Object<ActIpv4Dto>, ipv4, "Ipv4");
  DTO_FIELD(String, mac_address, "MacAddress");

  DTO_FIELD(Object<ActDeviceInfoDto>, device_info, "DeviceInfo");
  DTO_FIELD(Object<ActDeviceModularConfigurationDto>, modular_configuration, "ModularConfiguration");

  DTO_FIELD(UInt32, distance, "Distance");
  DTO_FIELD(Object<ActDeviceStatusDto>, device_status, "DeviceStatus");

  DTO_FIELD(Int16, tier, "Tier");
  DTO_FIELD(Object<ActCoordinateDto>, coordinate, "Coordinate");
  DTO_FIELD(Int64, group_id, "GroupId");

  DTO_FIELD(Object<ActDevicePropertyDto>, device_property, "DeviceProperty");
  DTO_FIELD(List<Object<ActInterfaceDto>>, interfaces, "Interfaces");

  DTO_FIELD(Object<ActDeviceAccountDto>, account, "Account");
  DTO_FIELD(Object<ActNetconfConfigurationDto>, netconf_configuration, "NetconfConfiguration");
  DTO_FIELD(Object<ActSnmpConfigurationDto>, snmp_configuration, "SnmpConfiguration");
  DTO_FIELD(Object<ActRestfulConfigurationDto>, restful_configuration, "RestfulConfiguration");

  DTO_FIELD(Boolean, enable_snmp_setting, "EnableSnmpSetting");

  DTO_FIELD(Enum<DeviceRoleDtoEnum>, device_role, "DeviceRole") = DeviceRoleDtoEnum::kUnknown;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDevicesDto : public oatpp::DTO {
  DTO_INIT(ActDevicesDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActDeviceDto>>, device_set, "DeviceSet");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSimpleDeviceDto : public oatpp::DTO {
  DTO_INIT(ActSimpleDeviceDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, device_name, "DeviceName") = "moxa";
  DTO_FIELD(String, device_alias, "DeviceAlias") = "Device Alias";
  DTO_FIELD(String, firmware_version, "FirmwareVersion") = "";
  DTO_FIELD(Int64, device_profile_id, "DeviceProfileId");
  DTO_FIELD(Int64, firmware_feature_profile_id, "FirmwareFeatureProfileId") = -1;
  DTO_FIELD(Enum<DeviceTypeDtoEnum>, device_type, "DeviceType") = DeviceTypeDtoEnum::kTSNSwitch;

  DTO_FIELD(Object<ActIpv4Dto>, ipv4, "Ipv4");
  DTO_FIELD(String, mac_address, "MacAddress");

  DTO_FIELD(Object<ActDeviceInfoDto>, device_info, "DeviceInfo");
  DTO_FIELD(Object<ActDeviceModularConfigurationDto>, modular_configuration, "ModularConfiguration");

  DTO_FIELD(Int16, tier, "Tier");
  DTO_FIELD(Object<ActCoordinateDto>, coordinate, "Coordinate");
  DTO_FIELD(Int64, group_id, "GroupId");

  DTO_FIELD(List<Object<ActInterfaceDto>>, interfaces, "Interfaces");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSimpleDevicesDto : public oatpp::DTO {
  DTO_INIT(ActSimpleDevicesDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActSimpleDeviceDto>>, device_set, "DeviceSet");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPatchDeviceMapDto : public oatpp::DTO {
  DTO_INIT(ActPatchDeviceMapDto, DTO)

  DTO_FIELD(Fields<String>, patch_device_map, "PatchDeviceMap");
};

class ActDeployDeviceDto : public oatpp::DTO {
  DTO_INIT(ActDeployDeviceDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp") = "0.0.0.0";
  DTO_FIELD(String, device_alias, "DeviceAlias");
  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(String, firmware_version, "FirmwareVersion");
  DTO_FIELD(String, mac_address, "MacAddress");
  DTO_FIELD(String, serial_number, "SerialNumber");
  DTO_FIELD(String, location, "Location");
  DTO_FIELD(Boolean, support, "Support");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeployDeviceListDto : public oatpp::DTO {
  DTO_INIT(ActDeployDeviceListDto, DTO)

  DTO_FIELD(List<Object<ActDeployDeviceDto>>, device_list, "DeviceList");
};

class ActDeviceOfflineConfigFileMapDto : public oatpp::DTO {
  DTO_INIT(ActDeviceOfflineConfigFileMapDto, DTO)

  DTO_FIELD(Fields<String>, device_offline_config_file_map, "DeviceOfflineConfigFileMap");  // <DeviceID, ConfigFileId>
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceBagItemDto : public oatpp::DTO {
  DTO_INIT(ActDeviceBagItemDto, DTO)

  DTO_FIELD(Boolean, can_add_to_topology, "CanAddToTopology") = true;
  DTO_FIELD(UInt64, remaining_quantity, "RemainQuantity") = 3;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceBagDto : public oatpp::DTO {
  DTO_INIT(ActDeviceBagDto, DTO)

  DTO_FIELD(Fields<Object<ActDeviceBagItemDto>>, device_bag, "DeviceBagMap");  // <ModelName, DeviceBagItem>
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_DEVICE_DTO_HPP */
