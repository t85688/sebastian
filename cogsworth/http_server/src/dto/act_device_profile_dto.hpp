/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_DEVICE_PROFILE_DTO_HPP
#define ACT_DEVICE_PROFILE_DTO_HPP

// #include "./act_device_dto.hpp"
#include "./act_device_config_dto.hpp"
#include "./act_feature_dto.hpp"
#include "./act_interface_dto.hpp"
#include "./act_link_dto.hpp"
#include "./act_sfp_module_dto.hpp"
#include "./act_stream_dto.hpp"
#include "./act_system_dto.hpp"
#include "./act_temperature_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(DeviceTypeDtoEnum, v_int32, VALUE(kTSNSwitch, 1, "TSNSwitch"), VALUE(kEndStation, 2, "EndStation"),
     VALUE(kBridgedEndStation, 3, "BridgedEndStation"), VALUE(kSwitch, 4, "Switch"),
     VALUE(kPoeAccessory, 5, "PoeAccessory"), VALUE(kNetworkMgmtSoftware, 6, "NetworkMgmtSoftware"),
     VALUE(kICMP, 7, "ICMP"), VALUE(kUnknown, 8, "Unknown"), VALUE(kMoxa, 9, "Moxa"))
ENUM(DeviceClusterDtoEnum, v_int32, VALUE(kDefault, 1, "Default"), VALUE(kNZ2MHG, 2, "NZ2MHG"))
ENUM(RestfulProtocolDtoEnum, v_int32, VALUE(kHTTP, 1, "HTTP"), VALUE(kHTTPS, 2, "HTTPS"))
ENUM(DeviceRoleDtoEnum, v_int32, VALUE(kRSTPRoot, 1, "RSTPRoot"), VALUE(kRSTPBackupRoot, 2, "RSTPBackupRoot"),
     VALUE(kRSTP, 3, "RSTP"), VALUE(kRingMaster, 4, "RingMaster"), VALUE(kTurboRing, 5, "TurboRing"),
     VALUE(kTurboChain, 6, "TurboChain"), VALUE(kUnknown, 7, "Unknown"))
ENUM(MountTypeDtoEnum, v_int32, VALUE(kDinRail, 1, "DinRail"), VALUE(kWallMount, 2, "WallMount"),
     VALUE(kRackMount, 3, "RackMount"))

// ENUM(SouthboundProtocolTypeDtoEnum, v_int32, VALUE(MOXAcommand, 1, "MOXAcommand"), VALUE(SNMP, 2, "SNMP"),
//      VALUE(NETCONF, 3, "NETCONF"), VALUE(RESTful, 4, "RESTful"))

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActProcessingDelayDto : public oatpp::DTO {
  DTO_INIT(ActProcessingDelayDto, DTO)

  DTO_FIELD(UInt64, independent_delay, "IndependentDelay");
  DTO_FIELD(UInt64, dependent_delay_ratio, "DependentDelayRatio");
};

class ActSequenceGenerationDto : public oatpp::DTO {
  DTO_INIT(ActSequenceGenerationDto, DTO)

  DTO_FIELD(UInt16, min, "Min");
  DTO_FIELD(UInt16, max, "Max");
};

class ActSequenceRecoveryDto : public oatpp::DTO {
  DTO_INIT(ActSequenceRecoveryDto, DTO)

  DTO_FIELD(UInt16, min, "Min");
  DTO_FIELD(UInt16, max, "Max");
  DTO_FIELD(UInt16, min_history_length, "MinHistoryLength");
  DTO_FIELD(UInt16, max_history_length, "MaxHistoryLength");
  DTO_FIELD(UInt16, reset_timeout, "ResetTimeout");
};

class ActSequenceIdentificationDto : public oatpp::DTO {
  DTO_INIT(ActSequenceIdentificationDto, DTO)

  DTO_FIELD(UInt16, min, "Min");
  DTO_FIELD(UInt16, max, "Max");
};

class ActStreamIdentityDto : public oatpp::DTO {
  DTO_INIT(ActStreamIdentityDto, DTO)

  DTO_FIELD(UInt16, min, "Min");
  DTO_FIELD(UInt16, max, "Max");
  DTO_FIELD(UInt64, min_handle, "MinHandle");
  DTO_FIELD(UInt64, max_handle, "MaxHandle");
};

class ActIeee802Dot1cbDto : public oatpp::DTO {
  DTO_INIT(ActIeee802Dot1cbDto, DTO)

  DTO_FIELD(Object<ActSequenceGenerationDto>, sequence_generation, "SequenceGeneration");
  DTO_FIELD(Object<ActSequenceRecoveryDto>, sequence_recovery, "SequenceRecovery");
  DTO_FIELD(Object<ActSequenceIdentificationDto>, sequence_identification, "SequenceIdentification");
  DTO_FIELD(Object<ActStreamIdentityDto>, stream_identity, "StreamIdentity");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCreateDeviceProfileDto : public oatpp::DTO {
  DTO_INIT(ActCreateDeviceProfileDto, DTO)

  DTO_FIELD(String, icon_name, "IconName");
  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(String, vendor, "Vendor");
  DTO_FIELD(Enum<DeviceTypeDtoEnum>, device_type, "DeviceType");

  DTO_FIELD(UnorderedSet<Object<ActInterfacePropertyDto>>, interfaces, "Interfaces");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceProfileDto : public oatpp::DTO {
  DTO_INIT(ActDeviceProfileDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, icon_name, "IconName") = "default.png";
  DTO_FIELD(String, data_version, "DataVersion");
  DTO_FIELD(Boolean, purchasable, "Purchasable");

  DTO_FIELD(String, l2_family, "L2Family");
  DTO_FIELD(String, l3_series, "L3Series");
  DTO_FIELD(String, l4_series, "L4Series");
  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(String, physical_model_name, "PhysicalModelName");
  DTO_FIELD(Enum<ActServiceProfileForDeviceProfileDtoEnum>, profiles,
            "Profiles") = ActServiceProfileForDeviceProfileDtoEnum::kSelfPlanning;
  DTO_FIELD(UnorderedSet<Enum<MountTypeDtoEnum>>, mount_type, "MountType") = {
    Enum<MountTypeDtoEnum>(MountTypeDtoEnum::kDinRail),
    Enum<MountTypeDtoEnum>(MountTypeDtoEnum::kRackMount),
    Enum<MountTypeDtoEnum>(MountTypeDtoEnum::kWallMount)
  };
  DTO_FIELD(String, latest_firmware_version, "LatestFirmwareVersion") = "";
  DTO_FIELD(UnorderedSet<String>, support_firmware_versions, "SupportFirmwareVersions");
  DTO_FIELD(String, built_in_power, "BuiltInPower");
  DTO_FIELD(Fields<String>, support_power_modules, "SupportPowerModules");
  DTO_FIELD(UInt8, support_power_slots, "SupportPowerSlots");

  DTO_FIELD(String, description, "Description");
  DTO_FIELD(Object<OperatingTemperatureCDto>, operating_temperature_c, "OperatingTemperatureC");
  DTO_FIELD(Enum<DeviceClusterDtoEnum>, device_cluster, "DeviceCluster") = DeviceClusterDtoEnum::kDefault;
  DTO_FIELD(String, device_name, "DeviceName");
  DTO_FIELD(Boolean, built_in, "BuiltIn");
  DTO_FIELD(Boolean, hide, "Hide");
  DTO_FIELD(String, vendor, "Vendor");
  DTO_FIELD(Enum<DeviceTypeDtoEnum>, device_type, "DeviceType");

  DTO_FIELD(UInt64, gcl_time_interval_value_min, "GCLTimeIntervalValueMin");
  DTO_FIELD(UInt64, gcl_time_interval_value_max, "GCLTimeIntervalValueMax");
  DTO_FIELD(UInt16, max_vlan_cfg_size, "MaxVlanCfgSize");
  DTO_FIELD(UInt16, per_queue_size, "PerQueueSize");
  DTO_FIELD(UInt8, ptp_queue_id, "PtpQueueId");

  DTO_FIELD(UInt16, gate_control_list_length, "GateControlListLength");
  DTO_FIELD(UInt16, number_of_queue, "NumberOfQueue");
  DTO_FIELD(UInt16, tick_granularity, "TickGranularity");
  DTO_FIELD(UInt16, stad_config_ingress_index_max, "StadConfigIngressIndexMax");
  DTO_FIELD(Fields<Object<ActProcessingDelayDto>>, processing_delay_map, "ProcessingDelayMap");

  DTO_FIELD(Fields<String>, support_ethernet_modules, "SupportEthernetModules");
  DTO_FIELD(UInt8, support_ethernet_slots, "SupportEthernetSlots");
  DTO_FIELD(UInt8, default_ethernet_slot_occupied_intfs, "DefaultEthernetSlotOccupiedIntfs");

  DTO_FIELD(Fields<String>, standards_and_certifications, "StandardsAndCertifications");
  DTO_FIELD(Fields<String>, supported_interfaces, "SupportedInterfaces");
  DTO_FIELD(Int64, max_port_speed, "MaxPortSpeed");
  DTO_FIELD(UnorderedSet<Object<ActInterfacePropertyDto>>, interfaces, "Interfaces");

  DTO_FIELD(Object<ActTrafficSpecificationDto>, traffic_specification, "TrafficSpecification");
  DTO_FIELD(Object<ActIeee802VlanTagDto>, ieee802_vlan_tag, "Ieee802VlanTag");
  DTO_FIELD(Object<ActFeatureGroupDto>, feature_group, "FeatureGroup");
  DTO_FIELD(UnorderedSet<Int32>, reserved_vlan, "ReservedVlan");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceProfilesDto : public oatpp::DTO {
  DTO_INIT(ActDeviceProfilesDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActDeviceProfileDto>>, device_profile_set, "DeviceProfileSet");
};

class ActSimpleDeviceProfileDto : public oatpp::DTO {
  DTO_INIT(ActSimpleDeviceProfileDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, icon_name, "IconName") = "default.png";
  DTO_FIELD(Boolean, purchasable, "Purchasable");

  DTO_FIELD(String, l2_family, "L2Family");
  DTO_FIELD(String, l3_series, "L3Series");
  DTO_FIELD(String, l4_series, "L4Series");
  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(String, physical_model_name, "PhysicalModelName");
  DTO_FIELD(UnorderedSet<Enum<MountTypeDtoEnum>>, mount_type, "MountType") = {
    Enum<MountTypeDtoEnum>(MountTypeDtoEnum::kDinRail),
    Enum<MountTypeDtoEnum>(MountTypeDtoEnum::kRackMount),
    Enum<MountTypeDtoEnum>(MountTypeDtoEnum::kWallMount)
  };
  DTO_FIELD(String, latest_firmware_version, "LatestFirmwareVersion") = "";
  DTO_FIELD(UnorderedSet<String>, support_firmware_versions, "SupportFirmwareVersions");

  DTO_FIELD(String, built_in_power, "BuiltInPower");
  DTO_FIELD(Fields<String>, support_power_modules, "SupportPowerModules");
  DTO_FIELD(UInt8, support_power_slots, "SupportPowerSlots");

  DTO_FIELD(String, description, "Description");
  DTO_FIELD(Object<OperatingTemperatureCDto>, operating_temperature_c, "OperatingTemperatureC");
  DTO_FIELD(Enum<DeviceClusterDtoEnum>, device_cluster, "DeviceCluster") = DeviceClusterDtoEnum::kDefault;

  DTO_FIELD(Enum<DeviceTypeDtoEnum>, device_type, "DeviceType");

  DTO_FIELD(Fields<String>, support_ethernet_modules, "SupportEthernetModules");
  DTO_FIELD(UInt8, support_ethernet_slots, "SupportEthernetSlots");
  DTO_FIELD(UInt8, default_ethernet_slot_occupied_intfs, "DefaultEthernetSlotOccupiedIntfs");

  DTO_FIELD(Fields<String>, standards_and_certifications, "StandardsAndCertifications");
  DTO_FIELD(Fields<String>, supported_interfaces, "SupportedInterfaces");
  DTO_FIELD(Int64, max_port_speed, "MaxPortSpeed");
  DTO_FIELD(UnorderedSet<Object<ActInterfacePropertyDto>>, interfaces, "Interfaces");
  DTO_FIELD(String, vendor, "Vendor");
  DTO_FIELD(String, device_name, "DeviceName");

  DTO_FIELD(Boolean, built_in, "BuiltIn");
  DTO_FIELD(Boolean, hide, "Hide");
  DTO_FIELD(Boolean, certificate, "Certificate");

  DTO_FIELD(Object<ActFeatureGroupDto>, feature_group, "FeatureGroup");
  DTO_FIELD(UnorderedSet<Int32>, reserved_vlan, "ReservedVlan");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSimpleDeviceProfilesDto : public oatpp::DTO {
  DTO_INIT(ActSimpleDeviceProfilesDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActSimpleDeviceProfileDto>>, simple_device_profile_set, "SimpleDeviceProfileSet");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActExportDeviceProfileInfoDto : public oatpp::DTO {
  DTO_INIT(ActExportDeviceProfileInfoDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, key, "Key");
};

class ActDeviceProfileWithDefaultDeviceConfigDto : public oatpp::DTO {
  DTO_INIT(ActDeviceProfileWithDefaultDeviceConfigDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(Object<ActDeviceConfigDto>, default_device_config, "DefaultDeviceConfig");
};

class ActDeviceProfilesWithDefaultDeviceConfigDto : public oatpp::DTO {
  DTO_INIT(ActDeviceProfilesWithDefaultDeviceConfigDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActDeviceProfileWithDefaultDeviceConfigDto>>,
            device_profile_with_default_device_config_set, "DeviceProfileWithDefaultDeviceConfigSet");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_DEVICE_PROFILE_DTO_HPP */
