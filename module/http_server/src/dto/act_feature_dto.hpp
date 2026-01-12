/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_FEATURE_DTO_HPP
#define ACT_FEATURE_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActOperationFeatureDto : public oatpp::DTO {
  DTO_INIT(ActOperationFeatureDto, DTO)

  DTO_FIELD(Boolean, reboot, "Reboot");
  DTO_FIELD(Boolean, factory_default, "FactoryDefault");
  DTO_FIELD(Boolean, firmware_upgrade, "FirmwareUpgrade");
  DTO_FIELD(Boolean, import_export, "ImportExport");
  DTO_FIELD(Boolean, enable_snmp_service, "EnableSNMPService");
  DTO_FIELD(Boolean, locator, "Locator");
  DTO_FIELD(Boolean, event_log, "EventLog");
  DTO_FIELD(Boolean, cli, "CLI");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTimeSettingItemDto : public oatpp::DTO {
  DTO_INIT(ActTimeSettingItemDto, DTO)

  DTO_FIELD(Boolean, system_time, "SystemTime");
  DTO_FIELD(Boolean, ptp, "PTP");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActVlanSettingItemDto : public oatpp::DTO {
  DTO_INIT(ActVlanSettingItemDto, DTO)

  DTO_FIELD(Boolean, access_trunk_mode, "AccessTrunkMode");
  DTO_FIELD(Boolean, hybrid_mode, "HybridMode");
  DTO_FIELD(Boolean, management_vlan, "ManagementVLAN");
  DTO_FIELD(Boolean, te_mstid, "TEMSTID");
  DTO_FIELD(Boolean, default_pvid, "DefaultPVID");
  DTO_FIELD(Boolean, default_pcp, "DefaultPCP");
  DTO_FIELD(Boolean, per_stream_priority, "PerStreamPriority");
  DTO_FIELD(Boolean, per_stream_priority_v2, "PerStreamPriorityV2");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActStaticForwardSettingItemDto : public oatpp::DTO {
  DTO_INIT(ActStaticForwardSettingItemDto, DTO)

  DTO_FIELD(Boolean, unicast, "Unicast");
  DTO_FIELD(Boolean, multicast, "Multicast");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActSTPRSTPItemDto : public oatpp::DTO {
  DTO_INIT(ActSTPRSTPItemDto, DTO)

  DTO_FIELD(Boolean, rstp, "RSTP");
  DTO_FIELD(Boolean, error_recovery_time, "ErrorRecoveryTime");
  DTO_FIELD(Boolean, swift, "Swift");

  DTO_FIELD(Boolean, port_rstp_enable, "PortRSTPEnable");
  DTO_FIELD(Boolean, link_type, "LinkType");
  DTO_FIELD(Boolean, bpdu_guard, "BPDUGuard");
  DTO_FIELD(Boolean, root_guard, "RootGuard");
  DTO_FIELD(Boolean, loop_guard, "LoopGuard");
  DTO_FIELD(Boolean, bpdu_filter, "BPDUFilter");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPortSettingItemDto : public oatpp::DTO {
  DTO_INIT(ActPortSettingItemDto, DTO)

  DTO_FIELD(Boolean, admin_status, "AdminStatus");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTimeSyncSettingItemDto : public oatpp::DTO {
  DTO_INIT(ActTimeSyncSettingItemDto, DTO)

  DTO_FIELD(Boolean, ieee1588_2008, "IEEE1588_2008");
  DTO_FIELD(Boolean, ieee802dot1as_2011, "IEEE802Dot1AS_2011");
  DTO_FIELD(Boolean, iec61850_2016, "IEC61850_2016");
  DTO_FIELD(Boolean, ieeec37dot238_2017, "IEEEC37Dot238_2017");
  DTO_FIELD(Boolean, ieee1588_2008_clock_type, "IEEE1588_2008_ClockType");
  DTO_FIELD(Boolean, ieee1588_2008_clock_mode, "IEEE1588_2008_ClockMode");
  DTO_FIELD(Boolean, ieee1588_2008_maximum_steps_removed, "IEEE1588_2008_MaximumStepsRemoved");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTSNItemDto : public oatpp::DTO {
  DTO_INIT(ActTSNItemDto, DTO)

  DTO_FIELD(Boolean, ieee802dot1cb, "IEEE802Dot1CB");
  DTO_FIELD(Boolean, ieee802dot1qbv, "IEEE802Dot1Qbv");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActConfigurationFeatureDto : public oatpp::DTO {
  DTO_INIT(ActConfigurationFeatureDto, DTO)

  DTO_FIELD(Boolean, network_setting, "NetworkSetting");
  DTO_FIELD(Boolean, user_account, "UserAccount");
  DTO_FIELD(Boolean, information_setting, "InformationSetting");
  DTO_FIELD(Boolean, management_interface, "ManagementInterface");
  DTO_FIELD(Boolean, login_policy, "LoginPolicy");
  DTO_FIELD(Boolean, snmp_trap_setting, "SNMPTrapSetting");
  DTO_FIELD(Boolean, syslog_setting, "SyslogSetting");
  DTO_FIELD(Boolean, loop_protection, "LoopProtection");
  DTO_FIELD(Boolean, linkup_delay, "LinkupDelay");
  DTO_FIELD(Boolean, prp_hsr, "PRPHSR");
  DTO_FIELD(Boolean, supervision_frame, "SupervisionFrame");
  DTO_FIELD(Object<ActTimeSettingItemDto>, time_setting, "TimeSetting");
  DTO_FIELD(Object<ActVlanSettingItemDto>, vlan_setting, "VLANSetting");
  DTO_FIELD(Object<ActStaticForwardSettingItemDto>, static_forward_setting, "StaticForwardSetting");
  DTO_FIELD(Object<ActSTPRSTPItemDto>, stp_rstp, "STPRSTP");
  DTO_FIELD(Object<ActPortSettingItemDto>, port_setting, "PortSetting");
  DTO_FIELD(Object<ActTimeSyncSettingItemDto>, time_sync_setting, "TimeSyncSetting");
  DTO_FIELD(Object<ActTSNItemDto>, tsn, "TSN");
  DTO_FIELD(Boolean, check_config_synchronization, "CheckConfigSynchronization");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActIdentifyItemDto : public oatpp::DTO {
  DTO_INIT(ActIdentifyItemDto, DTO)

  DTO_FIELD(Boolean, model_name, "ModelName");
  DTO_FIELD(Boolean, vendor_id, "VendorID");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceInformationItemDto : public oatpp::DTO {
  DTO_INIT(ActDeviceInformationItemDto, DTO)

  DTO_FIELD(Boolean, device_name, "DeviceName");
  DTO_FIELD(Boolean, ip_configuration, "IPConfiguration");
  DTO_FIELD(Boolean, location, "Location");
  DTO_FIELD(Boolean, product_revision, "ProductRevision");
  DTO_FIELD(Boolean, system_uptime, "SystemUptime");
  DTO_FIELD(Boolean, redundant_protocol, "RedundantProtocol");
  DTO_FIELD(Boolean, serial_number, "SerialNumber");
  DTO_FIELD(Boolean, modular_info, "ModularInfo");
  DTO_FIELD(Boolean, port_info, "PortInfo");
  DTO_FIELD(Boolean, mac_table, "MACTable");
  DTO_FIELD(Boolean, interface_name, "InterfaceName");
  DTO_FIELD(Boolean, interface_mac, "InterfaceMAC");
  DTO_FIELD(Boolean, port_speed, "PortSpeed");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActAutoScanFeatureDto : public oatpp::DTO {
  DTO_INIT(ActAutoScanFeatureDto, DTO)

  DTO_FIELD(Boolean, broadcast_search, "BroadcastSearch");
  DTO_FIELD(Object<ActIdentifyItemDto>, identify, "Identify");
  DTO_FIELD(Boolean, lldp, "LLDP");
  DTO_FIELD(Object<ActDeviceInformationItemDto>, device_information, "DeviceInformation");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActBasicStatusItemDto : public oatpp::DTO {
  DTO_INIT(ActBasicStatusItemDto, DTO)

  DTO_FIELD(Boolean, system_utilization, "SystemUtilization");
  DTO_FIELD(Boolean, port_status, "PortStatus");
  DTO_FIELD(Boolean, fiber_check, "FiberCheck");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTrafficItemDto : public oatpp::DTO {
  DTO_INIT(ActTrafficItemDto, DTO)

  DTO_FIELD(Boolean, tx_total_octets, "TxTotalOctets");
  DTO_FIELD(Boolean, tx_total_packets, "TxTotalPackets");
  DTO_FIELD(Boolean, traffic_utilization, "TrafficUtilization");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActRedundancyItemDto : public oatpp::DTO {
  DTO_INIT(ActRedundancyItemDto, DTO)

  DTO_FIELD(Boolean, rstp, "RSTP");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTimeSynchronizationItemDto : public oatpp::DTO {
  DTO_INIT(ActTimeSynchronizationItemDto, DTO)

  DTO_FIELD(Boolean, ieee15882008, "IEEE1588_2008");
  DTO_FIELD(Boolean, ieee802dot1as2011, "IEEE802Dot1AS_2011");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActMonitorFeatureDto : public oatpp::DTO {
  DTO_INIT(ActMonitorFeatureDto, DTO)

  DTO_FIELD(Object<ActBasicStatusItemDto>, basic_status, "BasicStatus");
  DTO_FIELD(Object<ActTrafficItemDto>, traffic, "Traffic");
  DTO_FIELD(Object<ActRedundancyItemDto>, redundancy, "Redundancy");
  DTO_FIELD(Object<ActTimeSynchronizationItemDto>, time_synchronization, "TimeSynchronization");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActFeatureGroupDto : public oatpp::DTO {
  DTO_INIT(ActFeatureGroupDto, DTO)

  DTO_FIELD(Object<ActAutoScanFeatureDto>, auto_scan, "AutoScan");
  DTO_FIELD(Object<ActOperationFeatureDto>, operation, "Operation");
  DTO_FIELD(Object<ActConfigurationFeatureDto>, configuration, "Configuration");
  DTO_FIELD(Object<ActMonitorFeatureDto>, monitor, "Monitor");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_FEATURE_DTO_HPP */
