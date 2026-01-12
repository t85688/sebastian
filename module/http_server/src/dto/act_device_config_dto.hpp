/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

// #include "./act_device_dto.hpp"
#include "./act_class_based_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(NetworkSettingModeDtoEnum, v_int32, VALUE(kStatic, 0, "Static"), VALUE(kDHCP, 1, "DHCP"),
     VALUE(kBootp, 2, "Bootp"))

ENUM(VlanPortTypeDtoEnum, v_int32, VALUE(kAccess, 1, "Access"), VALUE(kTrunk, 2, "Trunk"), VALUE(kHybrid, 3, "Hybrid"))

ENUM(IdentificationTypeDtoEnum, v_int32,
     VALUE(kNull_stream_identification, 1, "dot1cb-stream-identification-types:null-stream-identification"),
     VALUE(kSmac_vlan_stream_identification, 2, "dot1cb-stream-identification-types:smac-vlan-stream-identification"),
     VALUE(kDmac_vlan_stream_identification, 3, "dot1cb-stream-identification-types:dmac-vlan-stream-identification"),
     VALUE(kIp_stream_identification, 4, "dot1cb-stream-identification-types:ip-stream-identification"))

ENUM(NextProtocolDtoEnum, v_int32, VALUE(kNone, 0, "none"), VALUE(kUdp, 1, "udp"), VALUE(kTcp, 2, "tcp"),
     VALUE(kSctp, 3, "sctp"))

ENUM(VlanTagIdentificationTypeDtoEnum, v_int32, VALUE(kTagged, 1, "tagged"), VALUE(kPriority, 2, "priority"),
     VALUE(kAll, 3, "all"))
ENUM(SequenceEncodeDecodeTypesDtoEnum, v_int32, VALUE(kRTag, 1, "r-tag"), VALUE(kHSRSequenceTag, 2, "hsr-sequence-tag"),
     VALUE(kPRPSequenceTrailer, 3, "prp-sequence-trailer"))

ENUM(VlanPriorityDtoEnum, v_int32, VALUE(kNonTSN, 1, "NonTSN"), VALUE(kTSNUser, 2, "TSNUser"),
     VALUE(kTSNSystem, 3, "TSNSystem"))

ENUM(SequenceRecoveryAlgorithmDtoEnum, v_int32, VALUE(kVector, 1, "vector"), VALUE(kMatch, 2, "match"))

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActNetworkSettingTableDto : public oatpp::DTO {
  DTO_INIT(ActNetworkSettingTableDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(Enum<NetworkSettingModeDtoEnum>, network_setting_mode, "NetworkSettingMode");
  DTO_FIELD(String, ip_address, "IpAddress");
  DTO_FIELD(String, subnet_mask, "SubnetMask");
  DTO_FIELD(String, gateway, "Gateway");
  DTO_FIELD(String, dns1, "DNS1");
  DTO_FIELD(String, dns2, "DNS2");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActVlanStaticEntryDto : public oatpp::DTO {
  DTO_INIT(ActVlanStaticEntryDto, DTO)

  DTO_FIELD(Enum<VlanPriorityDtoEnum>, vlan_priority, "VlanPriority");
  DTO_FIELD(Int32, vlan_id, "VlanId");
  DTO_FIELD(String, name, "Name");
  DTO_FIELD(UnorderedSet<Int64>, untagged_ports, "UntaggedPorts");
  DTO_FIELD(UnorderedSet<Int64>, egress_ports, "EgressPorts");
  DTO_FIELD(UnorderedSet<Int64>, forbidden_egress_ports, "ForbiddenEgressPorts");
  DTO_FIELD(Int32, row_status, "RowStatus");
  DTO_FIELD(Boolean, te_mstid, "TeMstid") = false;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPortVlanEntryDto : public oatpp::DTO {
  DTO_INIT(ActPortVlanEntryDto, DTO)

  DTO_FIELD(Int64, port_id, "PortId");
  DTO_FIELD(UInt16, pvid, "PVID");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActVlanPortTypeEntryDto : public oatpp::DTO {
  DTO_INIT(ActVlanPortTypeEntryDto, DTO)

  DTO_FIELD(Int64, port_id, "PortId");
  DTO_FIELD(Enum<VlanPortTypeDtoEnum>, vlan_port_type, "VlanPortType");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActVlanTableDto : public oatpp::DTO {
  DTO_INIT(ActVlanTableDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(UnorderedSet<Object<ActVlanStaticEntryDto>>, vlan_static_entries, "VlanStaticEntries");
  DTO_FIELD(UnorderedSet<Object<ActPortVlanEntryDto>>, port_vlan_entries, "PortVlanEntries");
  DTO_FIELD(UnorderedSet<Object<ActVlanPortTypeEntryDto>>, vlan_port_type_entries, "VlanPortTypeEntries");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActIntelligentVlanDto : public oatpp::DTO {
  DTO_INIT(ActIntelligentVlanDto, DTO)

  DTO_FIELD(Int32, vlan_id, "VlanId");
  DTO_FIELD(UnorderedSet<Int64>, end_device_ids, "EndDeviceIds");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStaticForwardEntryDto : public oatpp::DTO {
  DTO_INIT(ActStaticForwardEntryDto, DTO)

  DTO_FIELD(Int32, vlan_id, "VlanId");
  DTO_FIELD(String, mac, "MAC");
  DTO_FIELD(Int64, egress_ports, "EgressPorts");
  DTO_FIELD(Int32, dot1q_status, "Dot1qStatus");

  // DTO_FIELD(Int64, ingress_port, "IngressPort");
  // DTO_FIELD(Int64, forbidden_egress_ports, "ForbiddenEgressPorts");
  // DTO_FIELD(Int32, storage_type, "StorageType");
  // DTO_FIELD(Int32, row_status, "RowStatus");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStaticForwardTableDto : public oatpp::DTO {
  DTO_INIT(ActStaticForwardTableDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(UnorderedSet<Object<ActStaticForwardEntryDto>>, static_forward_entries, "StaticForwardEntries");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDefaultPriorityEntryDto : public oatpp::DTO {
  DTO_INIT(ActDefaultPriorityEntryDto, DTO)

  DTO_FIELD(Int32, port_id, "PortId");
  DTO_FIELD(UInt8, default_pcp, "DefaultPCP");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDefaultPriorityTableDto : public oatpp::DTO {
  DTO_INIT(ActDefaultPriorityTableDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(UnorderedSet<Object<ActDefaultPriorityEntryDto>>, default_priority_entries, "DefaultPriorityEntries");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStadPortEntryDto : public oatpp::DTO {
  DTO_INIT(ActStadPortEntryDto, DTO)

  DTO_FIELD(Int64, stream_id, "StreamId");
  DTO_FIELD(Int64, port_id, "PortId");
  DTO_FIELD(Int32, vlan_id, "VlanId");
  DTO_FIELD(Int32, vlan_pcp, "VlanPcp");
  DTO_FIELD(Int32, index_enable, "IndexEnable");
  DTO_FIELD(Int32, ingress_index, "IngressIndex");
  DTO_FIELD(Int32, subtype_enable, "SubtypeEnable");
  DTO_FIELD(Int32, subtype_value, "SubtypeValue");
  DTO_FIELD(Int32, ethertype_value, "EthertypeValue");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActInterfaceStadPortEntryDto : public oatpp::DTO {
  DTO_INIT(ActInterfaceStadPortEntryDto, DTO)

  DTO_FIELD(Int64, interface_id, "InterfaceId");
  DTO_FIELD(UnorderedSet<Object<ActStadPortEntryDto>>, stad_port_entries, "StadPortEntries");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStadPortTableDto : public oatpp::DTO {
  DTO_INIT(ActStadPortTableDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(UnorderedSet<Object<ActInterfaceStadPortEntryDto>>, interface_stad_port_entries,
            "InterfaceStadPortEntries");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStadConfigEntryDto : public oatpp::DTO {
  DTO_INIT(ActStadConfigEntryDto, DTO)

  DTO_FIELD(Int64, port_id, "PortId");
  DTO_FIELD(Int32, egress_untag, "EgressUntag");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStadConfigTableDto : public oatpp::DTO {
  DTO_INIT(ActStadConfigTableDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(UnorderedSet<Object<ActStadConfigEntryDto>>, stad_config_entries, "StadConfigEntries");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSGSParamsDto : public oatpp::DTO {
  DTO_INIT(ActSGSParamsDto, DTO)

  DTO_FIELD(UInt8, gate_states_value, "GateStatesValue");
  DTO_FIELD(UInt32, time_interval_value, "TimeIntervalValue");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActAdminControlDto : public oatpp::DTO {
  DTO_INIT(ActAdminControlDto, DTO)

  DTO_FIELD(UInt32, index, "Index");
  DTO_FIELD(String, operation_name, "OperationName");
  DTO_FIELD(Object<ActSGSParamsDto>, sgs_params, "SgsParams");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGateParametersDto : public oatpp::DTO {
  DTO_INIT(ActGateParametersDto, DTO)

  DTO_FIELD(Boolean, gate_enabled, "GateEnabled");
  DTO_FIELD(Boolean, config_change, "ConfigChange");
  DTO_FIELD(UInt8, admin_gate_states, "AdminGateStates");
  DTO_FIELD(Object<ActAdminBaseTimeDto>, admin_base_time, "AdminBaseTime");
  DTO_FIELD(Object<ActAdminCycleTimeDto>, admin_cycle_time, "AdminCycleTime");
  DTO_FIELD(UInt32, admin_cycle_time_extension, "AdminCycleTimeExtension");
  DTO_FIELD(UInt32, admin_control_list_length, "AdminControlListLength");
  DTO_FIELD(List<Object<ActAdminControlDto>>, admin_control_list, "AdminControlList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActInterfaceGateParametersDto : public oatpp::DTO {
  DTO_INIT(ActInterfaceGateParametersDto, DTO)

  DTO_FIELD(Int64, interface_id, "InterfaceId");
  DTO_FIELD(Object<ActGateParametersDto>, gate_parameters, "GateParameters");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGclTableDto : public oatpp::DTO {
  DTO_INIT(ActGclTableDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(UnorderedSet<Object<ActInterfaceGateParametersDto>>, interfaces_gate_parameters,
            "InterfacesGateParameters");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActInFacingDto : public oatpp::DTO {
  DTO_INIT(ActInFacingDto, DTO)

  DTO_FIELD(UnorderedSet<String>, input_port_list, "InputPortList");
  DTO_FIELD(UnorderedSet<String>, output_port_list, "OutputPortList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActOutFacingDto : public oatpp::DTO {
  DTO_INIT(ActOutFacingDto, DTO)

  DTO_FIELD(UnorderedSet<String>, input_port_list, "InputPortList");
  DTO_FIELD(UnorderedSet<String>, output_port_list, "OutputPortList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActNullStreamIdentificationGroupDto : public oatpp::DTO {
  DTO_INIT(ActNullStreamIdentificationGroupDto, DTO)

  DTO_FIELD(String, destination_mac, "DestinationMac");
  DTO_FIELD(Enum<VlanTagIdentificationTypeDtoEnum>, tagged, "Tagged");

  DTO_FIELD(UInt16, vlan, "Vlan");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSmacVlanStreamIdentificationGroupDto : public oatpp::DTO {
  DTO_INIT(ActSmacVlanStreamIdentificationGroupDto, DTO)

  DTO_FIELD(String, source_mac, "SourceMac");
  DTO_FIELD(Enum<VlanTagIdentificationTypeDtoEnum>, tagged, "Tagged");

  DTO_FIELD(UInt16, vlan, "Vlan");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDownDto : public oatpp::DTO {
  DTO_INIT(ActDownDto, DTO)

  DTO_FIELD(String, destination_mac, "DestinationMac");
  DTO_FIELD(Enum<VlanTagIdentificationTypeDtoEnum>, tagged, "Tagged");
  DTO_FIELD(UInt16, vlan, "Vlan");
  DTO_FIELD(UInt8, priority, "Priority");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActUpDto : public oatpp::DTO {
  DTO_INIT(ActUpDto, DTO)

  DTO_FIELD(String, destination_mac, "DestinationMac");
  DTO_FIELD(Enum<VlanTagIdentificationTypeDtoEnum>, tagged, "Tagged");
  DTO_FIELD(UInt16, vlan, "Vlan");
  DTO_FIELD(UInt8, priority, "Priority");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDmacVlanStreamIdentificationGroupDto : public oatpp::DTO {
  DTO_INIT(ActDmacVlanStreamIdentificationGroupDto, DTO)

  DTO_FIELD(Object<ActDownDto>, down, "Down");
  DTO_FIELD(Object<ActUpDto>, up, "Up");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActIpStreamIdentificationGroupDto : public oatpp::DTO {
  DTO_INIT(ActIpStreamIdentificationGroupDto, DTO)

  DTO_FIELD(String, destination_mac, "DestinationMac");
  DTO_FIELD(Enum<VlanTagIdentificationTypeDtoEnum>, tagged, "Tagged");
  DTO_FIELD(UInt16, vlan, "Vlan");
  DTO_FIELD(String, ip_source, "IpSource");
  DTO_FIELD(String, ip_destination, "IpDestination");
  DTO_FIELD(UInt8, dscp, "Dscp");

  DTO_FIELD(Enum<NextProtocolDtoEnum>, next_protocol, "NextProtocol");
  DTO_FIELD(UInt16, source_port, "SourcePort");
  DTO_FIELD(UInt16, destination_port, "DestinationPort");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTsnStreamIdEntryGroupDto : public oatpp::DTO {
  DTO_INIT(ActTsnStreamIdEntryGroupDto, DTO)

  DTO_FIELD(UInt32, handle, "Handle");
  DTO_FIELD(Object<ActInFacingDto>, in_facing, "InFacing");
  DTO_FIELD(Object<ActOutFacingDto>, out_facing, "OutFacing");

  DTO_FIELD(Enum<IdentificationTypeDtoEnum>, identification_type, "IdentificationType");

  DTO_FIELD(Object<ActNullStreamIdentificationGroupDto>, null_stream_identification, "NullStreamIdentification");
  DTO_FIELD(Object<ActSmacVlanStreamIdentificationGroupDto>, smac_vlan_stream_identification,
            "SmacVlanStreamIdentification");
  DTO_FIELD(Object<ActDmacVlanStreamIdentificationGroupDto>, dmac_vlan_stream_identification,
            "DmacVlanStreamIdentification");
  DTO_FIELD(Object<ActIpStreamIdentificationGroupDto>, ip_stream_identification, "IpStreamIdentification");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStreamIdentityEntryDto : public oatpp::DTO {
  DTO_INIT(ActStreamIdentityEntryDto, DTO)

  DTO_FIELD(UInt32, index, "Index");
  DTO_FIELD(Object<ActTsnStreamIdEntryGroupDto>, tsn_stream_id_entry_group, "TsnStreamIdEntryGroup");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSequenceIdentificationEntryDto : public oatpp::DTO {
  DTO_INIT(ActSequenceIdentificationEntryDto, DTO)

  DTO_FIELD(UnorderedSet<UInt32>, stream_list, "StreamList");
  DTO_FIELD(String, port, "Port");
  DTO_FIELD(Boolean, direction, "Direction");
  DTO_FIELD(Boolean, active, "Active");
  DTO_FIELD(Enum<SequenceEncodeDecodeTypesDtoEnum>, encapsulation, "Encapsulation");
  DTO_FIELD(UInt8, path_id_lan_id, "PathIdLanId");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSequenceIdentificationListDto : public oatpp::DTO {
  DTO_INIT(ActSequenceIdentificationListDto, DTO)

  DTO_FIELD(Object<ActSequenceIdentificationEntryDto>, sequence_identification_entry, "SequenceIdentificationEntry");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSequenceGenerationEntryDto : public oatpp::DTO {
  DTO_INIT(ActSequenceGenerationEntryDto, DTO)

  DTO_FIELD(UInt32, stream_list, "StreamList");
  DTO_FIELD(Boolean, direction, "Direction");
  DTO_FIELD(Boolean, reset, "Reset");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSequenceGenerationListDto : public oatpp::DTO {
  DTO_INIT(ActSequenceGenerationListDto, DTO)

  DTO_FIELD(UInt32, index, "Index");
  DTO_FIELD(Object<ActSequenceGenerationEntryDto>, sequence_generation_entry, "SequenceGenerationEntry");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActLatentErrorDetectionParametersDto : public oatpp::DTO {
  DTO_INIT(ActLatentErrorDetectionParametersDto, DTO)

  DTO_FIELD(UInt32, difference, "Difference");
  DTO_FIELD(UInt32, period, "Period");
  DTO_FIELD(UInt16, paths, "Paths");
  DTO_FIELD(UInt32, reset_period, "ResetPeriod");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSequenceRecoveryEntryDto : public oatpp::DTO {
  DTO_INIT(ActSequenceRecoveryEntryDto, DTO)

  DTO_FIELD(UnorderedSet<UInt32>, stream_list, "StreamList");
  DTO_FIELD(UnorderedSet<String>, port_list, "PortList");
  DTO_FIELD(Boolean, direction, "Direction");
  DTO_FIELD(Boolean, reset, "Reset");
  DTO_FIELD(Enum<SequenceRecoveryAlgorithmDtoEnum>, algorithm, "Algorithm");
  DTO_FIELD(UInt32, history_length, "HistoryLength");
  DTO_FIELD(UInt32, reset_timeout, "ResetTimeout");
  DTO_FIELD(UInt32, invalid_sequence_value, "InvalidSequenceValue");
  DTO_FIELD(Boolean, take_no_sequence, "TakeNoSequence");
  DTO_FIELD(Boolean, individual_recovery, "IndividualRecovery");
  DTO_FIELD(Boolean, latent_error_detection, "LatentErrorDetection");
  DTO_FIELD(Object<ActLatentErrorDetectionParametersDto>, latent_error_detection_parameters,
            "LatentErrorDetectionParameters");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSequenceRecoveryListDto : public oatpp::DTO {
  DTO_INIT(ActSequenceRecoveryListDto, DTO)

  DTO_FIELD(UInt32, index, "Index");
  DTO_FIELD(Object<ActSequenceRecoveryEntryDto>, sequence_recovery_entry, "SequenceRecoveryEntry");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActFREREntryDto : public oatpp::DTO {
  DTO_INIT(ActFREREntryDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActSequenceIdentificationListDto>>, sequence_identification_lists,
            "SequenceIdentificationLists");
  DTO_FIELD(List<Object<ActSequenceGenerationListDto>>, sequence_generation_lists, "SequenceGenerationLists");
  DTO_FIELD(List<Object<ActSequenceRecoveryListDto>>, sequence_recovery_lists, "SequenceRecoveryLists");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCbTableDto : public oatpp::DTO {
  DTO_INIT(ActCbTableDto, DTO)

  DTO_FIELD(List<Object<ActStreamIdentityEntryDto>>, stream_identity_list, "StreamIdentityList");
  DTO_FIELD(Object<ActFREREntryDto>, frer, "FRER");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActRstpTableDto : public oatpp::DTO {
  DTO_INIT(ActRstpTableDto, DTO)

  DTO_FIELD(Boolean, active, "Active");
  DTO_FIELD(UInt16, priority, "Priority");
  DTO_FIELD(Int64, hello_time, "HelloTime");
  DTO_FIELD(Boolean, swift, "Swift");
  DTO_FIELD(UnorderedSet<Int64>, root_guards, "RootGuards");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceConfigDto : public oatpp::DTO {
  DTO_INIT(ActDeviceConfigDto, DTO)

  DTO_FIELD(Fields<Object<ActNetworkSettingTableDto>>, network_setting_tables, "NetworkSettingTables");
  DTO_FIELD(Fields<Object<ActVlanTableDto>>, vlan_tables, "VlanTables");
  DTO_FIELD(Fields<Object<ActStaticForwardTableDto>>, unicast_static_forward_tables, "UnicastStaticForwardTables");
  DTO_FIELD(Fields<Object<ActStaticForwardTableDto>>, multicast_static_forward_tables, "MulticastStaticForwardTables");
  DTO_FIELD(Fields<Object<ActDefaultPriorityTableDto>>, port_default_pcp_tables, "PortDefaultPCPTables");
  DTO_FIELD(Fields<Object<ActStadPortTableDto>>, stream_priority_ingress_tables, "StreamPriorityIngressTables");
  DTO_FIELD(Fields<Object<ActStadConfigTableDto>>, stream_priority_egress_tables, "StreamPriorityEgressTables");
  DTO_FIELD(Fields<Object<ActGclTableDto>>, gcl_tables, "GCLTables");
  DTO_FIELD(Fields<Object<ActCbTableDto>>, cb_tables, "CbTables");
  DTO_FIELD(Fields<Object<ActRstpTableDto>>, rstp_tables, "RstpTables");
};

#include OATPP_CODEGEN_END(DTO)
