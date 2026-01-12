/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_TRAFFIC_DTO_HPP
#define ACT_TRAFFIC_DTO_HPP

// #include "act_traffic.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(TrafficQosTypeDtoEnum, v_int32, VALUE(kBandwidth, 0, "Bandwidth"), VALUE(kBoundedLatency, 1, "Bounded Latency"),
     VALUE(kDeadline, 2, "Deadline"))

ENUM(TrafficUntaggedModeDtoEnum, v_int32, VALUE(kPVID, 0, "PVID"), VALUE(kPerStreamPriority, 1, "PerStreamPriority"))

class ActTrafficTypeConfigurationDto : public oatpp::DTO {
  DTO_INIT(ActTrafficTypeConfigurationDto, DTO)

  DTO_FIELD(UInt8, traffic_class, "TrafficClass");
  DTO_FIELD(String, traffic_type, "TrafficType");
  DTO_FIELD(UnorderedSet<UInt8>, priority_code_point_set, "PriorityCodePointSet");
  DTO_FIELD(UInt32, reserved_time, "ReservedTime");
};

class ActTrafficSettingDto : public oatpp::DTO {
  DTO_INIT(ActTrafficSettingDto, DTO)

  DTO_FIELD(UInt8, traffic_type_class, "TrafficTypeClass");
  DTO_FIELD(Enum<TrafficQosTypeDtoEnum>, qos_type, "QosType");
  DTO_FIELD(UInt32, min_receive_offset, "MinReceiveOffset");
  DTO_FIELD(UInt32, max_receive_offset, "MaxReceiveOffset");
};

class ActTrafficTSNDto : public oatpp::DTO {
  DTO_INIT(ActTrafficTSNDto, DTO)

  DTO_FIELD(Boolean, active, "Active");
  DTO_FIELD(Boolean, frer, "FRER");
};

class ActTrafficVlanSettingDto : public oatpp::DTO {
  DTO_INIT(ActTrafficVlanSettingDto, DTO)

  DTO_FIELD(Boolean, tagged, "Tagged");
  DTO_FIELD(Enum<TrafficUntaggedModeDtoEnum>, untagged_mode, "UntaggedMode");
  DTO_FIELD(Boolean, user_defined_vlan, "UserDefinedVlan");
  DTO_FIELD(UInt16, vlan_id, "VlanId");
  DTO_FIELD(UInt8, priority_code_point, "PriorityCodePoint");
  DTO_FIELD(UInt32, ether_type, "EtherType");
  DTO_FIELD(UInt16, sub_type, "SubType");
  DTO_FIELD(Boolean, enable_sub_type, "EnableSubType");
  DTO_FIELD(UnorderedSet<Enum<StreamPriorityTypeDtoEnum>>, type, "Type");
  DTO_FIELD(Int32, udp_port, "UdpPort");
  DTO_FIELD(Int32, tcp_port, "TcpPort");
};

class ActTrafficStreamParameterDto : public oatpp::DTO {
  DTO_INIT(ActTrafficStreamParameterDto, DTO)

  DTO_FIELD(UInt32, interval, "Interval");
  DTO_FIELD(UInt32, max_frame_size, "MaxFrameSize");
  DTO_FIELD(UInt32, max_frames_per_interval, "MaxFramesPerInterval");
  DTO_FIELD(UInt32, max_bytes_per_interval, "MaxBytesPerInterval");
  DTO_FIELD(UInt32, earliest_transmit_offset, "EarliestTransmitOffset");
  DTO_FIELD(UInt32, latest_transmit_offset, "LatestTransmitOffset");
  DTO_FIELD(UInt32, jitter, "Jitter");
};

class ActTrafficApplicationDto : public oatpp::DTO {
  DTO_INIT(ActTrafficApplicationDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, application_name, "ApplicationName");
  DTO_FIELD(Object<ActTrafficSettingDto>, traffic_setting, "TrafficSetting");
  DTO_FIELD(Object<ActTrafficTSNDto>, tsn, "TSN");
  DTO_FIELD(Object<ActTrafficVlanSettingDto>, vlan_setting, "VlanSetting");
  DTO_FIELD(Object<ActTrafficStreamParameterDto>, stream_parameter, "StreamParameter");
};

class ActTrafficStreamInterfaceDto : public oatpp::DTO {
  DTO_INIT(ActTrafficStreamInterfaceDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(Int64, interface_id, "InterfaceId");
};

class ActTrafficStreamDto : public oatpp::DTO {
  DTO_INIT(ActTrafficStreamDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(Boolean, active, "Active");
  DTO_FIELD(String, stream_name, "StreamName");
  DTO_FIELD(Int64, application_id, "ApplicationId");
  DTO_FIELD(Boolean, multicast, "Multicast");
  DTO_FIELD(String, destination_mac, "DestinationMac");
  DTO_FIELD(Object<ActTrafficStreamInterfaceDto>, talker, "Talker");
  DTO_FIELD(UnorderedSet<Object<ActTrafficStreamInterfaceDto>>, listeners, "Listeners");
};

class ActTrafficTypeConfigurationSettingDto : public oatpp::DTO {
  DTO_INIT(ActTrafficTypeConfigurationSettingDto, DTO)

  DTO_FIELD(List<Object<ActTrafficTypeConfigurationDto>>, traffic_type_configuration_setting,
            "TrafficTypeConfigurationSetting");
};

class ActTrafficApplicationSettingDto : public oatpp::DTO {
  DTO_INIT(ActTrafficApplicationSettingDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActTrafficApplicationDto>>, application_setting, "ApplicationSetting");
};

class ActTrafficStreamSettingDto : public oatpp::DTO {
  DTO_INIT(ActTrafficStreamSettingDto, DTO)

  DTO_FIELD(UnorderedSet<Object<ActTrafficStreamDto>>, stream_setting, "StreamSetting");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTrafficDesignDto : public oatpp::DTO {
  DTO_INIT(ActTrafficDesignDto, DTO)

  DTO_FIELD(List<Object<ActTrafficTypeConfigurationDto>>, traffic_type_configuration_setting,
            "TrafficTypeConfigurationSetting");
  DTO_FIELD(UnorderedSet<Object<ActTrafficApplicationDto>>, application_setting, "ApplicationSetting");
  DTO_FIELD(UnorderedSet<Object<ActTrafficStreamDto>>, stream_setting, "StreamSetting");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_USER_DTO_HPP */
