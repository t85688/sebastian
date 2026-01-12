/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_STREAM_DTO_HPP
#define ACT_STREAM_DTO_HPP

// #include "act_user.hpp"
#include "./act_interface_dto.hpp"
#include "./act_interval_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(StreamUntaggedModeDtoEnum, v_int32, VALUE(kPVID, 1, "PVID"), VALUE(kPerStreamPriority, 2, "PerStreamPriority"))

ENUM(StreamTrafficTypeDtoEnum, v_int32, VALUE(kBestEffort, 1, "BestEffort"), VALUE(kCyclic, 2, "Cyclic"),
     VALUE(kTimeSync, 3, "TimeSync"))
ENUM(QosTypeDtoEnum, v_int32, VALUE(kBandwidth, 1, "Bandwidth"), VALUE(kBoundLatency, 2, "BoundLatency"),
     VALUE(kDeadline, 3, "Deadline"))
ENUM(FrameTypeDtoEnum, v_int32, VALUE(kNA, 1, "NA"), VALUE(kCCLinkIeTsn, 2, "CCLinkIeTsn"),
     VALUE(kEtherCAT, 3, "EtherCAT"), VALUE(kProfinet, 4, "Profinet"), VALUE(kEthernetIp, 5, "EthernetIp"),
     VALUE(kUserDefined, 6, "UserDefined"))
// ENUM(FieldTypeDtoEnum, v_int32, VALUE(kIeee802MacAddresses, 1, "Ieee802MacAddresses"),
//      VALUE(kIeee802VlanTag, 2, "Ieee802VlanTag"), VALUE(kIpv4Tuple, 3, "Ipv4Tuple"), VALUE(kIpv6Tuple, 4,
//      "Ipv6Tuple"))
ENUM(FieldTypeDtoEnum, v_int32, VALUE(kIeee802MacAddresses, 1, "Ieee802MacAddresses"),
     VALUE(kIeee802VlanTag, 2, "Ieee802VlanTag"))

ENUM(StreamStatusDtoEnum, v_int32, VALUE(kPlanned, 1, "Planned"), VALUE(kScheduled, 2, "Scheduled"),
     VALUE(kModified, 3, "Modified"))

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStreamUniqueIdsDto : public oatpp::DTO {
  DTO_INIT(ActStreamUniqueIdsDto, DTO)

  DTO_FIELD(List<Int64>, stream_unique_ids, "StreamUniqueIds");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStreamIdDto : public oatpp::DTO {
  DTO_INIT(ActStreamIdDto, DTO)

  DTO_FIELD(Int64, unique_id, "UniqueId");
  DTO_FIELD(String, mac_address, "MacAddress");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCBDto : public oatpp::DTO {
  DTO_INIT(ActCBDto, DTO)

  DTO_FIELD(Boolean, active, "Active");
  DTO_FIELD(UInt16, vlan_id, "VlanId");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCCLinkIeTsnDto : public oatpp::DTO {
  DTO_INIT(ActCCLinkIeTsnDto, DTO)

  DTO_FIELD(UInt32, ether_type, "EtherType");
  DTO_FIELD(Boolean, enable_sub_type, "EnableSubType");
  DTO_FIELD(UInt16, sub_type, "SubType");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStreamPriorityDto : public oatpp::DTO {
  DTO_INIT(ActStreamPriorityDto, DTO)

  DTO_FIELD(Enum<FrameTypeDtoEnum>, frame_type, "FrameType") = FrameTypeDtoEnum::kCCLinkIeTsn;
  DTO_FIELD(Object<ActCCLinkIeTsnDto>, cclink_ie_tsn, "CCLinkIeTSN");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActIeee802MacAddressesDto : public oatpp::DTO {
  DTO_INIT(ActIeee802MacAddressesDto, DTO)

  DTO_FIELD(String, source_mac_address, "SourceMacAddress");
  DTO_FIELD(String, destination_mac_address, "DestinationMacAddress");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActIeee802VlanTagDto : public oatpp::DTO {
  DTO_INIT(ActIeee802VlanTagDto, DTO)

  DTO_FIELD(UInt16, vlan_id, "VlanId");
  DTO_FIELD(UInt8, priority_code_point, "PriorityCodePoint");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActIpv4TupleDto : public oatpp::DTO {
  DTO_INIT(ActIpv4TupleDto, DTO)

  DTO_FIELD(String, source_ip_address, "SourceIpAddress");
  DTO_FIELD(String, destination_ip_address, "DestinationIpAddress");
  DTO_FIELD(UInt8, dscp, "Dscp");
  DTO_FIELD(UInt16, protocol, "Protocol");
  DTO_FIELD(UInt16, source_port, "SourcePort");
  DTO_FIELD(UInt16, destination_port, "DestinationPort");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActIpv6TupleDto : public oatpp::DTO {
  DTO_INIT(ActIpv6TupleDto, DTO)

  DTO_FIELD(String, source_ip_address, "SourceIpAddress");
  DTO_FIELD(String, destination_ip_address, "DestinationIpAddress");
  DTO_FIELD(UInt8, dscp, "Dscp");
  DTO_FIELD(UInt16, protocol, "Protocol");
  DTO_FIELD(UInt16, source_port, "SourcePort");
  DTO_FIELD(UInt16, destination_port, "DestinationPort");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDataFrameSpecificationDto : public oatpp::DTO {
  DTO_INIT(ActDataFrameSpecificationDto, DTO)

  DTO_FIELD(Enum<FieldTypeDtoEnum>, type, "Type");
  DTO_FIELD(Object<ActIeee802MacAddressesDto>, ieee802_mac_addresses, "Ieee802MacAddresses");
  DTO_FIELD(Object<ActIeee802VlanTagDto>, ieee802_vlan_tag, "Ieee802VlanTag");
  // DTO_FIELD(Object<ActIpv4TupleDto>, ipv4_tuple, "Ipv4Tuple");
  // DTO_FIELD(Object<ActIpv6TupleDto>, ipv6_tuple, "Ipv6Tuple");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTimeAwareDto : public oatpp::DTO {
  DTO_INIT(ActTimeAwareDto, DTO)

  DTO_FIELD(UInt32, earliest_transmit_offset, "EarliestTransmitOffset");
  DTO_FIELD(UInt32, latest_transmit_offset, "LatestTransmitOffset");
  DTO_FIELD(UInt32, jitter, "Jitter");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTrafficSpecificationDto : public oatpp::DTO {
  DTO_INIT(ActTrafficSpecificationDto, DTO)

  DTO_FIELD(UInt16, max_frame_size, "MaxFrameSize") = 46;
  DTO_FIELD(UInt16, max_frames_per_interval, "MaxFramesPerInterval") = 1;
  DTO_FIELD(Object<ActIntervalDto>, interval, "Interval");
  // DTO_FIELD(UInt8, transmission_selection, "TransmissionSelection");
  DTO_FIELD(Object<ActTimeAwareDto>, time_aware, "TimeAware");
  DTO_FIELD(Boolean, enable_max_bytes_per_interval, "EnableMaxBytesPerInterval") = false;
  DTO_FIELD(UInt16, max_bytes_per_interval, "MaxBytesPerInterval");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTalkerDto : public oatpp::DTO {
  DTO_INIT(ActTalkerDto, DTO)

  DTO_FIELD(Object<ActInterfaceDto>, end_station_interface, "EndStationInterface");
  DTO_FIELD(List<Object<ActDataFrameSpecificationDto>>, data_frame_specifications, "DataFrameSpecifications");
  DTO_FIELD(Object<ActTrafficSpecificationDto>, traffic_specification, "TrafficSpecification");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActUserToNetworkRequirementDto : public oatpp::DTO {
  DTO_INIT(ActUserToNetworkRequirementDto, DTO)

  DTO_FIELD(UInt32, max_latency, "MaxLatency");
  DTO_FIELD(UInt8, num_seamless_trees, "NumSeamlessTrees");
  DTO_FIELD(UInt32, min_receive_offset, "MinReceiveOffset") = 1000;
  DTO_FIELD(UInt32, max_receive_offset, "MaxReceiveOffset") = 999999999;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActInterfaceCapabilityDto : public oatpp::DTO {
  DTO_INIT(ActInterfaceCapabilityDto, DTO)

  DTO_FIELD(Boolean, cb_capable, "CbCapable");
  // DTO_FIELD(List<Int64>, cb_sequence_type_list, "CbSequenceTypeList");
  // DTO_FIELD(List<Int64>, cb_stream_iden_type_list, "CbStreamIdenTypeList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActListenerDto : public oatpp::DTO {
  DTO_INIT(ActListenerDto, DTO)

  DTO_FIELD(Object<ActInterfaceDto>, end_station_interface, "EndStationInterface");
  DTO_FIELD(Object<ActUserToNetworkRequirementDto>, user_to_network_requirement, "UserToNetworkRequirement");
  DTO_FIELD(Object<ActInterfaceCapabilityDto>, interface_capability, "InterfaceCapability");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCreateStreamDto : public oatpp::DTO {
  DTO_INIT(ActCreateStreamDto, DTO)

  DTO_FIELD(String, stream_name, "StreamName");
  DTO_FIELD(Boolean, tagged, "Tagged");
  DTO_FIELD(Enum<StreamUntaggedModeDtoEnum>, untagged_mode, "UntaggedMode");
  DTO_FIELD(Enum<StreamTrafficTypeDtoEnum>, stream_traffic_type, "StreamTrafficType");
  DTO_FIELD(Enum<QosTypeDtoEnum>, qos_type, "QosType");
  DTO_FIELD(Object<ActCBDto>, cb, "CB");
  DTO_FIELD(Boolean, multicast, "Multicast") = false;
  DTO_FIELD(Boolean, user_defined_vlan, "UserDefinedVlan") = false;
  DTO_FIELD(Object<ActStreamPriorityDto>, stream_priority, "StreamPriority");
  DTO_FIELD(Object<ActTalkerDto>, talker, "Talker");
  DTO_FIELD(List<Object<ActListenerDto>>, listeners, "Listeners");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStreamDto : public oatpp::DTO {
  DTO_INIT(ActStreamDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(Object<ActStreamIdDto>, stream_id, "StreamId");
  DTO_FIELD(String, stream_name, "StreamName");
  DTO_FIELD(Boolean, tagged, "Tagged");
  DTO_FIELD(Enum<StreamUntaggedModeDtoEnum>, untagged_mode, "UntaggedMode");
  DTO_FIELD(Enum<StreamTrafficTypeDtoEnum>, stream_traffic_type, "StreamTrafficType");
  DTO_FIELD(Enum<QosTypeDtoEnum>, qos_type, "QosType");
  DTO_FIELD(UInt16, vlan_id, "VlanId");
  DTO_FIELD(Object<ActCBDto>, cb, "CB");
  DTO_FIELD(Boolean, multicast, "Multicast");
  DTO_FIELD(Boolean, user_defined_vlan, "UserDefinedVlan");
  DTO_FIELD(UInt8, stream_rank, "StreamRank");
  DTO_FIELD(Object<ActStreamPriorityDto>, stream_priority, "StreamPriority");
  DTO_FIELD(UInt8, time_slot_index, "TimeSlotIndex");
  DTO_FIELD(Object<ActTalkerDto>, talker, "Talker");
  DTO_FIELD(List<Object<ActListenerDto>>, listeners, "Listeners");
  DTO_FIELD(Enum<StreamStatusDtoEnum>, stream_status, "StreamStatus");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStreamsDto : public oatpp::DTO {
  DTO_INIT(ActStreamsDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActStreamDto>>, stream_set, "StreamSet");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPatchStreamMapDto : public oatpp::DTO {
  DTO_INIT(ActPatchStreamMapDto, DTO)

  DTO_FIELD(Fields<String>, patch_stream_map, "PatchStreamMap");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStreamListDto : public oatpp::DTO {
  DTO_INIT(ActStreamListDto, DTO)

  DTO_FIELD(List<Object<ActCreateStreamDto>>, stream_list, "StreamList");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_STREAM_DTO_HPP */
