/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "./act_class_based_dto.hpp"
#include "./act_compute_dto.hpp"
#include "./act_device_config_dto.hpp"
#include "./act_device_dto.hpp"
#include "./act_device_profile_dto.hpp"
#include "./act_gcl_result_dto.hpp"
#include "./act_link_dto.hpp"
#include "./act_manufacture_dto.hpp"
#include "./act_network_baseline_dto.hpp"
#include "./act_redundant_swift_dto.hpp"
#include "./act_service_profile_dto.hpp"
#include "./act_stream_dto.hpp"
#include "./act_system_dto.hpp"
#include "./act_traffic_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(ActProjectModeDtoEnum, v_int32, VALUE(kDesign, 0, "Design"), VALUE(kOperation, 1, "Operation"),
     VALUE(kManufacture, 2, "Manufacture"))

ENUM(ActQuestionnaireModeDtoEnum, v_int32, VALUE(kNone, 0, ""), VALUE(kVerified, 1, "Verified"),
     VALUE(kUnVerified, 2, "UnVerified"))

ENUM(ActIpConfiguringSequenceTypeDtoEnum, v_int32, VALUE(kFromFarToNear, 1, "FromFarToNear"),
     VALUE(kFromNearToFar, 2, "FromNearToFar"))
ENUM(ActIpAssigningSequenceTypeDtoEnum, v_int32, VALUE(kByMacDescending, 1, "ByMacDescending"),
     VALUE(kByMacAscending, 2, "ByMacAscending"), VALUE(kFromFarToNear, 3, "FromFarToNear"),
     VALUE(kFromNearToFar, 4, "FromNearToFar"))
ENUM(ActDefineDeviceToBeSetTypeDtoEnum, v_int32, VALUE(kAllDevicesUponSearch, 1, "AllDevicesUponSearch"),
     VALUE(kDevicesOfSpecificIPs, 2, "DevicesOfSpecificIPs"),
     VALUE(kDevicesExcludingSpecificIPs, 3, "DevicesExcludingSpecificIPs"))
ENUM(ActDefineIpAssignmentTypeDtoEnum, v_int32, VALUE(kDefineIpAssigningRule, 1, "DefineIpAssigningRule"),
     VALUE(kDefineIpAssignmentRange, 2, "DefineIpAssignmentRange"))

ENUM(ActProjectStatusDtoEnum, v_int32, VALUE(kIdle, 0, "Idle"), VALUE(kComputing, 1, "Computing"),
     VALUE(kComparing, 2, "Comparing"), VALUE(kDeploying, 3, "Deploying"), VALUE(kSyncing, 4, "Syncing"),
     VALUE(kScanning, 5, "Scanning"), VALUE(kMonitoring, 6, "Monitoring"),
     VALUE(kTopologyMapping, 7, "TopologyMapping"), VALUE(kManufacturing, 8, "Manufacturing"))

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActProjectNameDto : public oatpp::DTO {
  DTO_INIT(ActProjectNameDto, DTO);

  DTO_FIELD(String, project_name, "ProjectName");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActProjectStartIpDto : public oatpp::DTO {
  DTO_INIT(ActProjectStartIpDto, DTO);

  DTO_FIELD(Object<ActIpv4Dto>, project_start_ip, "ProjectStartIp");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActAlgorithmConfigurationDto : public oatpp::DTO {
  DTO_INIT(ActAlgorithmConfigurationDto, DTO);

  DTO_FIELD(UInt32, media_specific_overhead_bytes, "MediaSpecificOverheadBytes") = 43;
  // DTO_FIELD(Boolean, qbv_after_cb_merge, "QbvAfterCbMerge") = false;
  DTO_FIELD(UInt32, time_sync_delay, "TimeSyncDelay") = 200;
  DTO_FIELD(UInt32, timeout, "Timeout") = 9999;
  DTO_FIELD(UInt32, best_effort_bandwidth, "BestEffortBandwidth") = 35;
  DTO_FIELD(UInt32, time_sync_bandwidth, "TimeSyncBandwidth") = 35;
  DTO_FIELD(Boolean, keep_previous_result, "KeepPreviousResult") = true;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActIpRangeDto : public oatpp::DTO {
  DTO_INIT(ActIpRangeDto, DTO);

  DTO_FIELD(String, start_ip, "StartIp") = "192.168.127.1";
  DTO_FIELD(String, end_ip, "EndIp") = "192.168.127.254";
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActScanIpRangeDto : public oatpp::DTO {
  DTO_INIT(ActScanIpRangeDto, DTO);

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, start_ip, "StartIp") = "192.168.127.1";
  DTO_FIELD(String, end_ip, "EndIp") = "192.168.127.254";
  DTO_FIELD(Object<ActDeviceAccountDto>, account, "Account");
  DTO_FIELD(Object<ActNetconfConfigurationDto>, netconf_configuration, "NetconfConfiguration");
  DTO_FIELD(Object<ActSnmpConfigurationDto>, snmp_configuration, "SnmpConfiguration");
  DTO_FIELD(Object<ActRestfulConfigurationDto>, restful_configuration, "RestfulConfiguration");
  DTO_FIELD(Boolean, enable_snmp_setting, "EnableSnmpSetting");
  DTO_FIELD(Boolean, auto_probe, "AutoProbe");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActVlanRangeDto : public oatpp::DTO {
  DTO_INIT(ActVlanRangeDto, DTO);

  DTO_FIELD(UInt16, min, "Min") = 2;
  DTO_FIELD(UInt16, max, "Max") = 4094;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTrafficTypeToPriorityCodePointMappingDto : public oatpp::DTO {
  DTO_INIT(ActTrafficTypeToPriorityCodePointMappingDto, DTO);
  DTO_FIELD(UnorderedSet<UInt8>, na, "NA");
  DTO_FIELD(UnorderedSet<UInt8>, best_effort, "BestEffort");
  DTO_FIELD(UnorderedSet<UInt8>, cyclic, "Cyclic");
  DTO_FIELD(UnorderedSet<UInt8>, timeSync, "TimeSync");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPriorityCodePointToQueueMappingDto : public oatpp::DTO {
  DTO_INIT(ActPriorityCodePointToQueueMappingDto, DTO);

  DTO_FIELD(Fields<UnorderedSet<UInt8>>, priority_code_point_to_queue_mapping, "PriorityCodePointToQueueMapping");
  //   ACT_JSON_QT_DICT_SET(quint8, QSet<quint8>, priority_code_point_to_queue_mapping,PriorityCodePointToQueueMapping);
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCreateProjectDto : public oatpp::DTO {
  DTO_INIT(ActCreateProjectDto, DTO);

  DTO_FIELD(String, project_name, "ProjectName");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(Enum<ActProjectModeDtoEnum>, project_mode, "ProjectMode") = ActProjectModeDtoEnum::kDesign;
  DTO_FIELD(UInt64, user_id, "UserId");
  DTO_FIELD(Enum<ActServiceProfileForLicenseDtoEnum>, profile,
            "Profile") = ActServiceProfileForLicenseDtoEnum::kSelfPlanning;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCopyProjectDto : public oatpp::DTO {
  DTO_INIT(ActCopyProjectDto, DTO);

  DTO_FIELD(String, project_name, "ProjectName");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDefineDeviceTypeDto : public oatpp::DTO {
  DTO_INIT(ActDefineDeviceTypeDto, DTO);

  DTO_FIELD(Boolean, skip, "Skip");
  DTO_FIELD(Boolean, moxa_industrial_ethernet_product, "MoxaIndustrialEthernetProduct");
  DTO_FIELD(Boolean, moxa_industrial_wireless_product, "MoxaIndustrialWirelessProduct") = false;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDefineNetworkInterfaceDto : public oatpp::DTO {
  DTO_INIT(ActDefineNetworkInterfaceDto, DTO);

  DTO_FIELD(Boolean, skip, "Skip");
  DTO_FIELD(UnorderedSet<UInt64>, define_network_interfaces, "DefineNetworkInterfaces");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDefineDeviceToBeSetDto : public oatpp::DTO {
  DTO_INIT(ActDefineDeviceToBeSetDto, DTO);

  DTO_FIELD(Boolean, skip, "Skip");
  DTO_FIELD(Enum<ActDefineDeviceToBeSetTypeDtoEnum>, define_device_to_be_set_type,
            "DefineDeviceToBeSetType") = ActDefineDeviceToBeSetTypeDtoEnum::kAllDevicesUponSearch;
  DTO_FIELD(UnorderedSet<String>, specific_ips, "SpecificIps");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDefineIpAssigningRuleDto : public oatpp::DTO {
  DTO_INIT(ActDefineIpAssigningRuleDto, DTO);

  DTO_FIELD(String, start_ip, "StartIp") = "192.168.127.1";  ///< The start IP of the ip assignment
  DTO_FIELD(UInt8, incremental_gap, "IncrementalGap") = 1;   ///< The increment gap of the ip assignment
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDefineIpAssignmentDto : public oatpp::DTO {
  DTO_INIT(ActDefineIpAssignmentDto, DTO);

  DTO_FIELD(Boolean, skip, "Skip");
  DTO_FIELD(Enum<ActDefineIpAssignmentTypeDtoEnum>, define_ip_assignment,
            "DefineIpAssignment") = ActDefineIpAssignmentTypeDtoEnum::kDefineIpAssigningRule;

  DTO_FIELD(Object<ActDefineIpAssigningRuleDto>, define_ip_assigning_rule, "ActDefineIpAssigningRule");
  DTO_FIELD(Object<ActIpRangeDto>, define_ip_assigning_range, "ActDefineIpAssigningRange");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDefineIpAssigningSequenceDto : public oatpp::DTO {
  DTO_INIT(ActDefineIpAssigningSequenceDto, DTO);

  DTO_FIELD(Boolean, skip, "Skip");
  DTO_FIELD(Enum<ActIpAssigningSequenceTypeDtoEnum>, ip_assigning_sequence_type,
            "IpAssigningSequenceType") = ActIpAssigningSequenceTypeDtoEnum::kFromFarToNear;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDefineIpConfiguringSequenceDto : public oatpp::DTO {
  DTO_INIT(ActDefineIpConfiguringSequenceDto, DTO);

  DTO_FIELD(Boolean, skip, "Skip");
  DTO_FIELD(Enum<ActIpConfiguringSequenceTypeDtoEnum>, ip_configuring_sequence_type,
            "IpConfiguringSequenceType") = ActIpConfiguringSequenceTypeDtoEnum::kFromFarToNear;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDefineTopologyScanDto : public oatpp::DTO {
  DTO_INIT(ActDefineTopologyScanDto, DTO);

  DTO_FIELD(Boolean, skip, "Skip");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActUnlockDevicesDto : public oatpp::DTO {
  DTO_INIT(ActUnlockDevicesDto, DTO);

  DTO_FIELD(Boolean, skip, "Skip");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCfgWizardSettingDto : public oatpp::DTO {
  DTO_INIT(ActCfgWizardSettingDto, DTO);

  DTO_FIELD(Object<ActDefineDeviceTypeDto>, define_device_type, "DefineDeviceType");
  DTO_FIELD(Object<ActDefineNetworkInterfaceDto>, define_network_interface, "DefineNetworkInterface");
  DTO_FIELD(Object<ActUnlockDevicesDto>, unlock_devices, "UnlockDevices");
  DTO_FIELD(Object<ActDefineDeviceToBeSetDto>, define_device_to_be_set, "DefineDeviceToBeSet");
  DTO_FIELD(Object<ActDefineIpAssignmentDto>, define_ip_assignment, "DefineIpAssignment");
  DTO_FIELD(Object<ActDefineIpAssigningSequenceDto>, define_ip_assigning_sequence, "DefineIpAssigningSequence");
  DTO_FIELD(Object<ActDefineIpConfiguringSequenceDto>, define_ip_configuring_sequence, "DefineIpConfiguringSequence");
  DTO_FIELD(Object<ActDefineTopologyScanDto>, define_topology_scan, "DefineTopologyScan");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActManagementInterfaceDto : public oatpp::DTO {
  DTO_INIT(ActManagementInterfaceDto, DTO);

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(UnorderedSet<Int64>, interfaces, "Interfaces");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActManagementInterfacesDto : public oatpp::DTO {
  DTO_INIT(ActManagementInterfacesDto, DTO);

  DTO_FIELD(List<Object<ActManagementInterfaceDto>>, management_interfaces, "ManagementInterfaces");
};

class ActRSTPDto : public oatpp::DTO {
  DTO_INIT(ActRSTPDto, DTO);

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(Int64, hello_time, "HelloTime") = 2;
  DTO_FIELD(Int64, root_device, "RootDevice") = -1;
  DTO_FIELD(Int64, backup_root_device, "BackupRootDevice") = -1;
  DTO_FIELD(UnorderedSet<Int64>, devices, "Devices");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActRedundantGroupDto : public oatpp::DTO {
  DTO_INIT(ActRedundantGroupDto, DTO);

  DTO_FIELD(Object<ActRSTPDto>, rstp, "Rstp");
  DTO_FIELD(Object<ActSwiftDto>, swift, "Swift");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTopologySettingDto : public oatpp::DTO {
  DTO_INIT(ActTopologySettingDto, DTO);

  DTO_FIELD(UnorderedSet<Object<ActAlgorithmConfigurationDto>>, vlan_groups, "IntelligentVlanGroup");

  DTO_FIELD(Object<ActRedundantGroupDto>, redundant_group, "RedundantGroup");

  DTO_FIELD(List<Object<ActManagementInterfaceDto>>, management_interfaces, "ManagementInterfaces");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActMonitorConfigurationDto : public oatpp::DTO {
  DTO_INIT(ActMonitorConfigurationDto, DTO);

  DTO_FIELD(UInt16, polling_interval, "PollingInterval");          ///< The ping interval
  DTO_FIELD(Boolean, from_offline_project, "FromOfflineProject");  ///< The ping interval
  DTO_FIELD(Boolean, from_ip_scan_list, "FromIpScanList");         ///< The ping interval
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActProjectSettingDto : public oatpp::DTO {
  DTO_INIT(ActProjectSettingDto, DTO);

  DTO_FIELD(String, project_name, "ProjectName");
  DTO_FIELD(Object<ActAlgorithmConfigurationDto>, algorithm_configuration, "AlgorithmConfiguration");
  DTO_FIELD(Object<ActVlanRangeDto>, vlan_range, "VlanRange");
  DTO_FIELD(Object<ActCfgWizardSettingDto>, cfg_wizard_setting, "CfgWizardSetting");
  DTO_FIELD(Object<ActDeviceAccountDto>, account, "Account");
  DTO_FIELD(Object<ActNetconfConfigurationDto>, netconf_configuration, "NetconfConfiguration");
  DTO_FIELD(Object<ActSnmpConfigurationDto>, snmp_configuration, "SnmpConfiguration");
  DTO_FIELD(Object<ActRestfulConfigurationDto>, restful_configuration, "RestfulConfiguration");
  DTO_FIELD(Object<ActTrafficTypeToPriorityCodePointMappingDto>, traffic_type_to_priority_code_point_mapping,
            "TrafficTypeToPriorityCodePointMapping");
  DTO_FIELD(UnorderedSet<Object<ActPriorityCodePointToQueueMappingDto>>, priority_code_point_to_queue_mapping,
            "PriorityCodePointToQueueMapping");

  DTO_FIELD(List<Object<ActScanIpRangeDto>>, scan_ip_ranges, "ScanIpRanges");
  DTO_FIELD(Object<ActIpv4Dto>, project_start_ip, "ProjectStartIp");

  DTO_FIELD(Object<ActSnmpTrapConfigurationDto>, snmp_trap_configuration, "SnmpTrapConfiguration");
  DTO_FIELD(Object<ActMonitorConfigurationDto>, monitor_configuration, "MonitorConfiguration");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActScanIpRangesDto : public oatpp::DTO {
  DTO_INIT(ActScanIpRangesDto, DTO);

  DTO_FIELD(List<Object<ActScanIpRangeDto>>, scan_ip_ranges, "ScanIpRanges");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActProjectDto : public oatpp::DTO {
  DTO_INIT(ActProjectDto, DTO);

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, act_version, "ActVersion");
  DTO_FIELD(UInt64, data_version, "DataVersion");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(Enum<ActProjectModeDtoEnum>, project_mode, "ProjectMode") = ActProjectModeDtoEnum::kDesign;
  DTO_FIELD(Enum<ActQuestionnaireModeDtoEnum>, questionnaire_mode,
            "QuestionnaireMode") = ActQuestionnaireModeDtoEnum::kNone;
  DTO_FIELD(Enum<ActServiceProfileForLicenseDtoEnum>, profile,
            "Profile") = ActServiceProfileForLicenseDtoEnum::kSelfPlanning;

  DTO_FIELD(String, uuid, "UUID");
  DTO_FIELD(UInt64, user_id, "UserId");
  DTO_FIELD(String, organization_id, "OrganizationId");
  DTO_FIELD(UInt64, created_time, "CreatedTime");
  DTO_FIELD(UInt64, last_modified_time, "LastModifiedTime");

  DTO_FIELD(UnorderedSet<Object<ActDeviceDto>>, devices, "Devices");
  DTO_FIELD(UnorderedSet<Object<ActLinkDto>>, links, "Links");
  DTO_FIELD(UnorderedSet<Object<ActStreamDto>>, streams, "Streams");
  DTO_FIELD(Object<ActCycleSettingDto>, cycle_setting, "CycleSetting");
  DTO_FIELD(Object<ActComputedResultDto>, computed_result, "ComputedResult");

  DTO_FIELD(Object<ActProjectSettingDto>, project_setting, "ProjectSetting");
  DTO_FIELD(Object<ActTopologySettingDto>, topology_setting, "TopologySetting");
  DTO_FIELD(Object<ActDeviceConfigDto>, device_config, "DeviceConfig");

  DTO_FIELD(Object<ActTrafficDesignDto>, traffic_design, "TrafficDesign");
  DTO_FIELD(Object<ActManufactureResultDto>, manufacture_result, "ManufactureResult");
  DTO_FIELD(Fields<UInt32>, sfp_counts, "SFPCounts");
  DTO_FIELD(Fields<Object<ActSkuQuantityDto>>, sku_quantities_map, "SkuQuantitiesMap");
  DTO_FIELD(Int64, platform_project_id, "PlatformProjectId");
  DTO_FIELD(Int64, activate_baseline_id, "ActivateBaselineId");
  DTO_FIELD(UnorderedSet<Int64>, design_baseline_ids, "DesignBaselineIds");
  DTO_FIELD(UnorderedSet<Int64>, operation_baseline_ids, "OperationBaselineIds");
};

// /**
//  *  Data Transfer Object. Object containing fields only.
//  *  Used in API for serialization/deserialization and validation
//  */
// class ActProjectsDto : public oatpp::DTO {
//   DTO_INIT(ActProjectsDto, DTO)

//   DTO_FIELD(UnorderedSet<Object<ActProjectDto>>, ProjectSet);
// };

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSimpleProjectDto : public oatpp::DTO {
  DTO_INIT(ActSimpleProjectDto, DTO);

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, project_name, "ProjectName");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(UInt64, created_time, "CreatedTime");
  DTO_FIELD(UInt64, last_modified_time, "LastModifiedTime");
  DTO_FIELD(Enum<ActProjectModeDtoEnum>, project_mode, "ProjectMode") = ActProjectModeDtoEnum::kDesign;
  DTO_FIELD(Enum<ActQuestionnaireModeDtoEnum>, questionnaire_mode,
            "QuestionnaireMode") = ActQuestionnaireModeDtoEnum::kNone;
  DTO_FIELD(Enum<ActServiceProfileForLicenseDtoEnum>, profile,
            "Profile") = ActServiceProfileForLicenseDtoEnum::kSelfPlanning;
  DTO_FIELD(String, uuid, "UUID");
  DTO_FIELD(UInt64, user_id, "UserId");
  DTO_FIELD(String, organization_id, "OrganizationId");

  DTO_FIELD(Enum<ActProjectStatusDtoEnum>, project_status, "ProjectStatus") = ActProjectStatusDtoEnum::kIdle;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSimpleProjectsDto : public oatpp::DTO {
  DTO_INIT(ActSimpleProjectsDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActSimpleProjectDto>>, simple_project_set, "SimpleProjectSet");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActImportProjectDto : public oatpp::DTO {
  DTO_INIT(ActImportProjectDto, DTO);

  DTO_FIELD(String, project_name, "ProjectName");
  DTO_FIELD(Object<ActProjectDto>, project, "Project");
  DTO_FIELD(Boolean, overwrite, "Overwrite");
  DTO_FIELD(UnorderedSet<Object<ActDeviceProfileDto>>, device_profiles, "DeviceProfiles");
  DTO_FIELD(UnorderedSet<Object<ActExportDeviceProfileInfoDto>>, device_profile_infos, "DeviceProfileInfos");
  DTO_FIELD(List<Object<ActNetworkBaselineDto>>, design_baselines, "DesignBaselines");
  DTO_FIELD(List<Object<ActNetworkBaselineDto>>, operation_baselines, "OperationBaselines");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActExportProjectDto : public oatpp::DTO {
  DTO_INIT(ActExportProjectDto, DTO);

  DTO_FIELD(String, project_name, "ProjectName");
  DTO_FIELD(Object<ActProjectDto>, project, "Project");
  DTO_FIELD(UnorderedSet<Object<ActDeviceProfileDto>>, device_profiles, "DeviceProfiles");
  DTO_FIELD(UnorderedSet<Object<ActExportDeviceProfileInfoDto>>, device_profile_infos, "DeviceProfileInfos");
  DTO_FIELD(List<Object<ActNetworkBaselineDto>>, design_baselines, "DesignBaselines");
  DTO_FIELD(List<Object<ActNetworkBaselineDto>>, operation_baselines, "OperationBaselines");
};

#include OATPP_CODEGEN_END(DTO)
