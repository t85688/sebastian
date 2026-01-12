/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_DEVICE_SETTING_DTO_HPP
#define ACT_DEVICE_SETTING_DTO_HPP

#include "act_device_config_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceInformationDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceInformationDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp") = "0.0.0.0";
  DTO_FIELD(String, device_alias, "DeviceAlias");
  DTO_FIELD(Boolean, support, "Support") = false;
  DTO_FIELD(String, device_name, "DeviceName") = "moxa";
  DTO_FIELD(String, location, "Location");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(String, contact_information, "ContactInformation");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceInformationsDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceInformationsDto, DTO)

  DTO_FIELD(List<Object<ActGetDeviceInformationDto>>, device_informations, "DeviceInformationList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceInformationDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceInformationDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_name, "DeviceName") = "moxa";
  DTO_FIELD(String, location, "Location");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(String, contact_information, "ContactInformation");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceInformationsDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceInformationsDto, DTO)

  DTO_FIELD(List<Object<ActPutDeviceInformationDto>>, device_informations, "DeviceInformationList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceLoginPolicyDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceLoginPolicyDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp") = "0.0.0.0";
  DTO_FIELD(String, device_alias, "DeviceAlias");
  DTO_FIELD(Boolean, support, "Support") = false;
  DTO_FIELD(String, login_message, "LoginMessage");
  DTO_FIELD(String, login_authentication_failure_message, "LoginAuthenticationFailureMessage");
  DTO_FIELD(Boolean, account_login_failure_lockout, "AccountLoginFailureLockout") = false;
  DTO_FIELD(Int32, retry_failure_threshold, "RetryFailureThreshold") = 5;
  DTO_FIELD(Int32, lockout_duration, "LockoutDuration") = 5;
  DTO_FIELD(Int32, auto_logout_after, "AutoLogoutAfter") = 5;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceLoginPolicyListDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceLoginPolicyListDto, DTO)

  DTO_FIELD(List<Object<ActGetDeviceLoginPolicyDto>>, device_login_policies, "DeviceLoginPolicyList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceLoginPolicyDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceLoginPolicyDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, login_message, "LoginMessage");
  DTO_FIELD(String, login_authentication_failure_message, "LoginAuthenticationFailureMessage");
  DTO_FIELD(Boolean, account_login_failure_lockout, "AccountLoginFailureLockout") = false;
  DTO_FIELD(Int32, retry_failure_threshold, "RetryFailureThreshold") = 5;
  DTO_FIELD(Int32, lockout_duration, "LockoutDuration") = 5;
  DTO_FIELD(Int32, auto_logout_after, "AutoLogoutAfter") = 5;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceLoginPolicyListDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceLoginPolicyListDto, DTO)

  DTO_FIELD(List<Object<ActPutDeviceLoginPolicyDto>>, device_login_policies, "DeviceLoginPolicyList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceLoopProtectionDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceLoopProtectionDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp") = "0.0.0.0";
  DTO_FIELD(String, device_alias, "DeviceAlias");
  DTO_FIELD(Boolean, support, "Support") = false;
  DTO_FIELD(Boolean, network_loop_protection, "NetworkLoopProtection") = false;
  DTO_FIELD(Int32, detect_interval, "DetectInterval") = 10;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceLoopProtectionsDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceLoopProtectionsDto, DTO)

  DTO_FIELD(List<Object<ActGetDeviceLoopProtectionDto>>, device_loop_protection_list, "DeviceLoopProtectionList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceLoopProtectionDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceLoopProtectionDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(Boolean, network_loop_protection, "NetworkLoopProtection") = false;
  DTO_FIELD(Int32, device_interval, "DeviceInterval") = 10;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceLoopProtectionsDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceLoopProtectionsDto, DTO)

  DTO_FIELD(List<Object<ActPutDeviceLoopProtectionDto>>, device_loop_protection_list, "DeviceLoopProtectionList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceSyslogSettingDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceSyslogSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp") = "0.0.0.0";
  DTO_FIELD(String, device_alias, "DeviceAlias");
  DTO_FIELD(Boolean, support, "Support") = false;
  DTO_FIELD(Boolean, enabled, "Enabled") = false;
  DTO_FIELD(Boolean, syslog_server_1, "SyslogServer1") = false;
  DTO_FIELD(String, address_1, "Address1") = "0.0.0.0";
  DTO_FIELD(Int32, udp_port_1, "UDPPort1") = 514;
  DTO_FIELD(Boolean, syslog_server_2, "SyslogServer2") = false;
  DTO_FIELD(String, address_2, "Address2") = "0.0.0.0";
  DTO_FIELD(Int32, udp_port_2, "UDPPort2") = 514;
  DTO_FIELD(Boolean, syslog_server_3, "SyslogServer3") = false;
  DTO_FIELD(String, address_3, "Address3") = "0.0.0.0";
  DTO_FIELD(Int32, udp_port_3, "UDPPort3") = 514;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceSyslogSettingsDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceSyslogSettingsDto, DTO)

  DTO_FIELD(List<Object<ActGetDeviceSyslogSettingDto>>, device_syslog_setting_list, "DeviceSyslogSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceSyslogSettingDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceSyslogSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(Boolean, enabled, "Enabled") = false;
  DTO_FIELD(Boolean, syslog_server_1, "SyslogServer1") = false;
  DTO_FIELD(String, address_1, "Address1") = "0.0.0.0";
  DTO_FIELD(Int32, udp_port_1, "UDPPort1") = 514;
  DTO_FIELD(Boolean, syslog_server_2, "SyslogServer2") = false;
  DTO_FIELD(String, address_2, "Address2") = "0.0.0.0";
  DTO_FIELD(Int32, udp_port_2, "UDPPort2") = 514;
  DTO_FIELD(Boolean, syslog_server_3, "SyslogServer3") = false;
  DTO_FIELD(String, address_3, "Address3") = "0.0.0.0";
  DTO_FIELD(Int32, udp_port_3, "UDPPort3") = 514;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceSyslogSettingsDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceSyslogSettingsDto, DTO)

  DTO_FIELD(List<Object<ActPutDeviceSyslogSettingDto>>, device_syslog_setting_list, "DeviceSyslogSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
ENUM(ActSnmpTrapModeDtoEnum, v_int32, VALUE(kTrapV1, 1, "TrapV1"), VALUE(kTrapV2c, 2, "TrapV2c"),
     VALUE(kInformV2c, 3, "InformV2c"), VALUE(kNotSupport, 4, "NotSupport"))

class ActSnmpTrapHostEntryDto : public oatpp::DTO {
  DTO_INIT(ActSnmpTrapHostEntryDto, DTO)

  DTO_FIELD(String, host_name, "HostName");
  DTO_FIELD(Enum<ActSnmpTrapModeDtoEnum>, mode, "Mode");
  DTO_FIELD(String, trap_community, "TrapCommunity");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceSnmpTrapSettingDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceSnmpTrapSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp") = "0.0.0.0";
  DTO_FIELD(String, device_alias, "DeviceAlias");
  DTO_FIELD(Boolean, support, "Support") = false;
  DTO_FIELD(List<Object<ActSnmpTrapHostEntryDto>>, host_list, "HostList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceSnmpTrapSettingsDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceSnmpTrapSettingsDto, DTO)

  DTO_FIELD(List<Object<ActGetDeviceSnmpTrapSettingDto>>, device_syslog_setting_list, "DeviceSnmpTrapSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceSnmpTrapSettingDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceSnmpTrapSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(List<Object<ActSnmpTrapHostEntryDto>>, host_list, "HostList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceSnmpTrapSettingsDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceSnmpTrapSettingsDto, DTO)

  DTO_FIELD(List<Object<ActPutDeviceSnmpTrapSettingDto>>, device_syslog_setting_list, "DeviceSnmpTrapSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceBackupSettingDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceBackupSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp") = "0.0.0.0";
  DTO_FIELD(String, device_alias, "DeviceAlias");
  DTO_FIELD(Boolean, support, "Support") = false;
  DTO_FIELD(String, model_name, "ModelName");
  DTO_FIELD(String, firmware_version, "FirmwareVersion");
  DTO_FIELD(String, mac_address, "MacAddress");
  DTO_FIELD(String, serial_number, "SerialNumber");
  DTO_FIELD(String, location, "Location");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceBackupsDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceBackupsDto, DTO)

  DTO_FIELD(List<Object<ActGetDeviceBackupSettingDto>>, device_backup_setting_list, "DeviceBackupSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceBackupFileDto : public oatpp::DTO {
  DTO_INIT(ActDeviceBackupFileDto, DTO)

  DTO_FIELD(String, file_name, "FileName");
  DTO_FIELD(String, backup_file, "BackupFile");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActZipFileMultipartDto : public oatpp::DTO {
  DTO_INIT(ActZipFileMultipartDto, DTO)

  DTO_FIELD(oatpp::swagger::Binary, file);
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDevicePortDto : public oatpp::DTO {
  DTO_INIT(ActDevicePortDto, DTO)

  DTO_FIELD(Int64, port_id, "PortId");
  DTO_FIELD(String, port_name, "PortName");
  DTO_FIELD(Boolean, active, "Active") = false;
  DTO_FIELD(Enum<VlanPortTypeDtoEnum>, port_type, "PortType") = VlanPortTypeDtoEnum::kAccess;
  DTO_FIELD(Int32, pvid, "Pvid") = 1;
  DTO_FIELD(UInt8, default_pcp, "DefaultPCP");
  DTO_FIELD(UnorderedSet<Int32>, untagged_vlan, "UntaggedVlan");
  DTO_FIELD(UnorderedSet<Int32>, tagged_vlan, "TaggedVlan");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDeviceVlanDto : public oatpp::DTO {
  DTO_INIT(ActDeviceVlanDto, DTO)

  DTO_FIELD(Int32, vlan_id, "VlanId");
  DTO_FIELD(String, vlan_name, "VlanName");
  DTO_FIELD(Boolean, te_mstid, "TeMstid");
  DTO_FIELD(UnorderedSet<Int64>, member_port, "MemberPort");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceVlanSettingDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceVlanSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp") = "0.0.0.0";
  DTO_FIELD(String, device_alias, "DeviceAlias");
  DTO_FIELD(Boolean, support, "Support") = false;
  DTO_FIELD(Int32, management_vlan, "ManagementVlan");
  DTO_FIELD(List<Object<ActDeviceVlanDto>>, vlan_list, "VlanList");
  DTO_FIELD(List<Object<ActDevicePortDto>>, port_list, "PortList");
  DTO_FIELD(UnorderedSet<Int32>, reserved_vlan, "ReservedVlan");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceVlanSettingsDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceVlanSettingsDto, DTO)

  DTO_FIELD(List<Object<ActGetDeviceVlanSettingDto>>, device_vlan_setting_list, "DeviceVlanSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceVlanSettingDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceVlanSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(Int32, management_vlan, "ManagementVlan");
  DTO_FIELD(List<Object<ActDeviceVlanDto>>, vlan_list, "VlanList");
  DTO_FIELD(List<Object<ActDevicePortDto>>, port_list, "PortList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceVlanSettingsDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceVlanSettingsDto, DTO)

  DTO_FIELD(List<Object<ActPutDeviceVlanSettingDto>>, device_vlan_setting_list, "DeviceVlanSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */

ENUM(ActClockSourceDtoEnum, v_int32, VALUE(kLocal, 1, "Local"), VALUE(kSNTP, 2, "SNTP"), VALUE(kNTP, 3, "NTP"),
     VALUE(kPTP, 4, "PTP"))

ENUM(ActTimeZoneDtoEnum, v_int32, VALUE(kIDLW, 1, "UTC-12:00"), VALUE(kSST, 2, "UTC-11:00"),
     VALUE(kHST, 3, "UTC-10:00"), VALUE(kMIT, 4, "UTC-09:30"), VALUE(kAKST, 5, "UTC-09:00"),
     VALUE(kPST, 6, "UTC-08:00"), VALUE(kMST, 7, "UTC-07:00"), VALUE(kCST, 8, "UTC-06:00"), VALUE(kEST, 9, "UTC-05:00"),
     VALUE(kAST, 10, "UTC-04:00"), VALUE(kNST, 11, "UTC-03:30"), VALUE(kBRT, 12, "UTC-03:00"),
     VALUE(kFNT, 13, "UTC-02:00"), VALUE(kCVT, 14, "UTC-01:00"), VALUE(kGMT, 15, "UTC+00:00"),
     VALUE(kCET, 16, "UTC+01:00"), VALUE(kEET, 17, "UTC+02:00"), VALUE(kMSK, 18, "UTC+03:00"),
     VALUE(kIRST, 19, "UTC+03:30"), VALUE(kGST, 20, "UTC+04:00"), VALUE(kAFT, 21, "UTC+04:30"),
     VALUE(kPKT, 22, "UTC+05:00"), VALUE(kIST, 23, "UTC+05:30"), VALUE(kNPT, 24, "UTC+05:45"),
     VALUE(kBHT, 25, "UTC+06:00"), VALUE(kMMT, 26, "UTC+06:30"), VALUE(kICT, 27, "UTC+07:00"),
     VALUE(kBJT, 28, "UTC+08:00"), VALUE(kJST, 29, "UTC+09:00"), VALUE(kACST, 30, "UTC+09:30"),
     VALUE(kAEST, 31, "UTC+10:00"), VALUE(kLHST, 32, "UTC+10:30"), VALUE(kVUT, 33, "UTC+11:00"),
     VALUE(kNZST, 34, "UTC+12:00"), VALUE(kCHAST, 35, "UTC+12:45"), VALUE(kPHOT, 36, "UTC+13:00"),
     VALUE(kLINT, 37, "UTC+14:00"))

class ActTimeDayDto : public oatpp::DTO {
  DTO_INIT(ActTimeDayDto, DTO)

  DTO_FIELD(UInt16, month, "Month");
  DTO_FIELD(UInt16, week, "Week");
  DTO_FIELD(UInt16, day, "Day");
  DTO_FIELD(UInt16, hour, "Hour");
  DTO_FIELD(UInt16, minute, "Minute");
};

class ActGetDeviceTimeSettingDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceTimeSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp") = "0.0.0.0";
  DTO_FIELD(String, device_alias, "DeviceAlias");
  DTO_FIELD(Boolean, support, "Support") = false;

  // Time
  DTO_FIELD(Enum<ActClockSourceDtoEnum>, clock_source, "ClockSource") = ActClockSourceDtoEnum::kPTP;
  DTO_FIELD(String, ntp_time_server_1, "NTPTimeServer1");
  DTO_FIELD(String, ntp_time_server_2, "NTPTimeServer2");
  DTO_FIELD(String, sntp_time_server_1, "SNTPTimeServer1");
  DTO_FIELD(String, sntp_time_server_2, "SNTPTimeServer2");

  // TimeZone
  DTO_FIELD(Enum<ActTimeZoneDtoEnum>, time_zone, "TimeZone") = ActTimeZoneDtoEnum::kGMT;
  DTO_FIELD(Boolean, daylight_saving_time, "DaylightSavingTime");
  DTO_FIELD(String, offset, "Offset");  // HH:MM
  DTO_FIELD(Object<ActTimeDayDto>, start, "Start");
  DTO_FIELD(Object<ActTimeDayDto>, end, "End");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceTimeSettingListDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceTimeSettingListDto, DTO)

  DTO_FIELD(List<Object<ActGetDeviceTimeSettingDto>>, device_Time_setting_list, "DeviceTimeSettingList");
};

class ActPutDeviceTimeSettingDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceTimeSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");

  // Time
  DTO_FIELD(Enum<ActClockSourceDtoEnum>, clock_source, "ClockSource") = ActClockSourceDtoEnum::kPTP;
  DTO_FIELD(String, ntp_time_server_1, "NTPTimeServer1");
  DTO_FIELD(String, ntp_time_server_2, "NTPTimeServer2");
  DTO_FIELD(String, sntp_time_server_1, "SNTPTimeServer1");
  DTO_FIELD(String, sntp_time_server_2, "SNTPTimeServer2");

  // TimeZone
  DTO_FIELD(Enum<ActTimeZoneDtoEnum>, time_zone, "TimeZone") = ActTimeZoneDtoEnum::kGMT;
  DTO_FIELD(Boolean, daylight_saving_time, "DaylightSavingTime");
  DTO_FIELD(String, offset, "Offset");  // HH:MM
  DTO_FIELD(Object<ActTimeDayDto>, start, "Start");
  DTO_FIELD(Object<ActTimeDayDto>, end, "End");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceTimeSettingListDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceTimeSettingListDto, DTO)

  DTO_FIELD(List<Object<ActPutDeviceTimeSettingDto>>, device_Time_setting_list, "DeviceTimeSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActDevicePortStatusDto : public oatpp::DTO {
  DTO_INIT(ActDevicePortStatusDto, DTO)

  DTO_FIELD(Int32, port_id, "PortId");
  DTO_FIELD(String, port_name, "PortName");
  DTO_FIELD(Boolean, active, "Active") = false;
  DTO_FIELD(Boolean, admin_status, "AdminStatus");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDevicePortSettingDto : public oatpp::DTO {
  DTO_INIT(ActGetDevicePortSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp") = "0.0.0.0";
  DTO_FIELD(String, device_alias, "DeviceAlias");
  DTO_FIELD(Boolean, support, "Support") = false;
  DTO_FIELD(List<Object<ActDevicePortStatusDto>>, port_list, "PortList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDevicePortSettingListDto : public oatpp::DTO {
  DTO_INIT(ActGetDevicePortSettingListDto, DTO)

  DTO_FIELD(List<Object<ActGetDevicePortSettingDto>>, device_port_setting_list, "DevicePortSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDevicePortSettingDto : public oatpp::DTO {
  DTO_INIT(ActPutDevicePortSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(List<Object<ActDevicePortStatusDto>>, port_list, "PortList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDevicePortSettingListDto : public oatpp::DTO {
  DTO_INIT(ActPutDevicePortSettingListDto, DTO)

  DTO_FIELD(List<Object<ActPutDevicePortSettingDto>>, device_port_setting_list, "DevicePortSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceIpSettingDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceIpSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp") = "0.0.0.0";
  DTO_FIELD(String, device_alias, "DeviceAlias");
  DTO_FIELD(Boolean, support, "Support") = false;
  DTO_FIELD(String, subnet_mask, "SubnetMask");
  DTO_FIELD(String, gateway, "Gateway");
  DTO_FIELD(String, dns1, "DNS1");
  DTO_FIELD(String, dns2, "DNS2");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActGetDeviceIpSettingListDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceIpSettingListDto, DTO)

  DTO_FIELD(List<Object<ActGetDeviceIpSettingDto>>, device_ip_setting_list, "DeviceIpSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceIpSettingDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceIpSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp") = "0.0.0.0";
  DTO_FIELD(String, subnet_mask, "SubnetMask");
  DTO_FIELD(String, gateway, "Gateway");
  DTO_FIELD(String, dns1, "DNS1");
  DTO_FIELD(String, dns2, "DNS2");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceIpSettingListDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceIpSettingListDto, DTO)

  DTO_FIELD(List<Object<ActPutDeviceIpSettingDto>>, device_ip_setting_list, "DeviceIpSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
ENUM(ActRstpEdgeDtoEnum, v_int32, VALUE(kAuto, 1, "Auto"), VALUE(kYes, 2, "Yes"), VALUE(kNo, 3, "No"))

ENUM(ActRstpLinkTypeDtoEnum, v_int32, VALUE(kPointToPoint, 1, "Point-to-point"), VALUE(kShared, 2, "Shared"),
     VALUE(kAuto, 3, "Auto"))

class ActGetRstpPortEntryDto : public oatpp::DTO {
  DTO_INIT(ActGetRstpPortEntryDto, DTO)

  DTO_FIELD(Int32, port_id, "PortId");
  DTO_FIELD(String, port_name, "PortName");
  DTO_FIELD(Boolean, active, "Active") = false;
  // std1w1ap
  DTO_FIELD(Boolean, rstp_enable, "RstpEnable") = false;
  DTO_FIELD(Enum<ActRstpEdgeDtoEnum>, edge, "Edge") = ActRstpEdgeDtoEnum::kAuto;

  DTO_FIELD(Int64, port_priority, "PortPriority") = 128;
  DTO_FIELD(Int64, path_cost, "PathCost");

  // std1d1ap
  DTO_FIELD(Boolean, link_type_support, "LinkTypeSupport") = false;
  DTO_FIELD(Enum<ActRstpLinkTypeDtoEnum>, link_type, "LinkType") = ActRstpLinkTypeDtoEnum::kAuto;

  // mxrstp
  DTO_FIELD(Boolean, bpdu_guard_support, "BpduGuardSupport") = false;
  DTO_FIELD(Boolean, root_guard_support, "RootGuardSupport") = false;
  DTO_FIELD(Boolean, loop_guard_support, "LoopGuardSupport") = false;
  DTO_FIELD(Boolean, bpdu_filter_support, "BpduFilterSupport") = false;

  DTO_FIELD(Boolean, bpdu_guard, "BpduGuard") = false;
  DTO_FIELD(Boolean, root_guard, "RootGuard") = false;
  DTO_FIELD(Boolean, loop_guard, "LoopGuard") = false;
  DTO_FIELD(Boolean, bpdu_filter, "BpduFilter") = false;
};

class ActPutRstpPortEntryDto : public oatpp::DTO {
  DTO_INIT(ActPutRstpPortEntryDto, DTO)

  DTO_FIELD(Int32, port_id, "PortId");
  // std1w1ap
  DTO_FIELD(Boolean, rstp_enable, "RstpEnable") = false;
  DTO_FIELD(Enum<ActRstpEdgeDtoEnum>, edge, "Edge") = ActRstpEdgeDtoEnum::kAuto;

  DTO_FIELD(Int64, port_priority, "PortPriority") = 128;
  DTO_FIELD(Int64, path_cost, "PathCost");

  // std1d1ap
  DTO_FIELD(Enum<ActRstpLinkTypeDtoEnum>, link_type, "LinkType") = ActRstpLinkTypeDtoEnum::kAuto;

  // mxrstp
  DTO_FIELD(Boolean, bpdu_guard, "BpduGuard") = false;
  DTO_FIELD(Boolean, root_guard, "RootGuard") = false;
  DTO_FIELD(Boolean, loop_guard, "LoopGuard") = false;
  DTO_FIELD(Boolean, bpdu_filter, "BpduFilter") = false;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
ENUM(ActCompatibilityDtoEnum, v_int32, VALUE(kSTP, 0, "STP"), VALUE(kRSTP, 2, "RSTP"),
     VALUE(kNotSupport, 3, "NotSupport"))

class ActGetDeviceRstpSettingDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceRstpSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(String, device_ip, "DeviceIp") = "0.0.0.0";
  DTO_FIELD(String, device_alias, "DeviceAlias");
  DTO_FIELD(Boolean, support, "Support") = false;

  DTO_FIELD(Boolean, stp_rstp, "StpRstp") = true;

  // std1w1ap
  DTO_FIELD(Enum<ActCompatibilityDtoEnum>, compatibility, "Compatibility") = ActCompatibilityDtoEnum::kRSTP;
  DTO_FIELD(Int64, priority, "Priority") = 36728;
  DTO_FIELD(Int64, forward_delay, "ForwardDelay") = 15;
  DTO_FIELD(Int64, hello_time, "HelloTime") = 2;
  DTO_FIELD(Int64, max_age, "MaxAge") = 20;
  // mxrstp
  DTO_FIELD(Int64, rstp_error_recovery_time, "RstpErrorRecoveryTime") = 300;
  // mxrstp > swift
  DTO_FIELD(Boolean, rstp_config_swift_support, "RstpConfigSwiftSupport") = false;
  DTO_FIELD(Boolean, rstp_config_swift, "RstpConfigSwift") = false;
  DTO_FIELD(Boolean, rstp_config_revert, "RstpConfigRevert") = false;

  DTO_FIELD(Boolean, rstp_port_entry_support, "RstpPortEntrySupport") = false;
  DTO_FIELD(UnorderedSet<Object<ActGetRstpPortEntryDto>>, rstp_port_entries, "RstpPortEntries");
};

class ActGetDeviceRstpSettingListDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceRstpSettingListDto, DTO)

  DTO_FIELD(List<Object<ActGetDeviceRstpSettingDto>>, device_rstp_setting_list, "DeviceRstpSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceRstpSettingDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceRstpSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");
  DTO_FIELD(Boolean, stp_rstp, "StpRstp") = true;
  // std1w1ap
  DTO_FIELD(Int64, priority, "Priority") = 36728;
  DTO_FIELD(Int64, forward_delay, "ForwardDelay") = 15;
  DTO_FIELD(Int64, hello_time, "HelloTime") = 2;
  DTO_FIELD(Int64, max_age, "MaxAge") = 20;
  // mxrstp
  DTO_FIELD(Int64, rstp_error_recovery_time, "RstpErrorRecoveryTime") = 300;
  // mxrstp > swift
  DTO_FIELD(Boolean, rstp_config_swift, "RstpConfigSwift") = false;
  DTO_FIELD(Boolean, rstp_config_revert, "RstpConfigRevert") = false;

  DTO_FIELD(UnorderedSet<Object<ActPutRstpPortEntryDto>>, rstp_port_entries, "RstpPortEntries");
};

class ActPutDeviceRstpSettingListDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceRstpSettingListDto, DTO)

  DTO_FIELD(List<Object<ActPutDeviceRstpSettingDto>>, device_rstp_setting_list, "DeviceRstpSettingList");
};

// Per-stream priority

ENUM(StreamPriorityTypeDtoEnum, v_int32, VALUE(kEthertype, 0, "etherType"), VALUE(kTcp, 1, "tcpPort"),
     VALUE(kUdp, 2, "udpPort"))

class ActPerStreamPriorityEntryDto : public oatpp::DTO {
  DTO_INIT(ActPerStreamPriorityEntryDto, DTO)

  DTO_FIELD(Int32, port_id, "PortId");
  DTO_FIELD(String, port_name, "PortName");

  DTO_FIELD(Enum<StreamPriorityTypeDtoEnum>, type, "Type");

  // L2
  DTO_FIELD(Int32, ether_type, "EtherType") = 0;
  DTO_FIELD(Boolean, enable_sub_type, "EnableSubType") = false;
  DTO_FIELD(Int32, sub_type, "SubType") = 0;

  // L3
  DTO_FIELD(Int32, udp_port, "UdpPort") = 1;
  DTO_FIELD(Int32, tcp_port, "TcpPort") = 1;

  DTO_FIELD(Int32, vlan_id, "VlanId") = 1;
  DTO_FIELD(UInt8, priority_code_point, "PriorityCodePoint") = 1;
};

class ActDevicePortListEntryDto : public oatpp::DTO {
  DTO_INIT(ActDevicePortListEntryDto, DTO)

  DTO_FIELD(Int64, port_id, "PortId") = 1;
  DTO_FIELD(String, port_name, "PortName") = "1";
  DTO_FIELD(Boolean, active, "Active") = true;
  DTO_FIELD(List<Int32>, available_vlans, "AvailableVlans");
};

class ActGetDevicePerStreamPrioritySettingDto : public oatpp::DTO {
  DTO_INIT(ActGetDevicePerStreamPrioritySettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId") = 1;
  DTO_FIELD(String, device_ip, "DeviceIp") = "100.100.100.100";
  DTO_FIELD(String, device_alias, "DeviceAlias") = "alias";
  DTO_FIELD(Boolean, support, "Support") = true;

  DTO_FIELD(Boolean, udp_tcp_support, "UdpTcpSupport") = false;

  DTO_FIELD(List<Object<ActDevicePortListEntryDto>>, port_list, "PortList");

  DTO_FIELD(List<Object<ActPerStreamPriorityEntryDto>>, per_stream_priority_entry, "PerStreamPrioritySetting");
};

class ActGetDevicePerStreamPrioritySettingListDto : public oatpp::DTO {
  DTO_INIT(ActGetDevicePerStreamPrioritySettingListDto, DTO)

  DTO_FIELD(List<Object<ActGetDevicePerStreamPrioritySettingDto>>, device_per_stream_priority_setting_list,
            "DevicePerStreamPrioritySettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDevicePerStreamPrioritySettingDto : public oatpp::DTO {
  DTO_INIT(ActPutDevicePerStreamPrioritySettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");

  DTO_FIELD(List<Object<ActPerStreamPriorityEntryDto>>, per_stream_priority_entry, "PerStreamPrioritySetting");
};

class ActPutDevicePerStreamPrioritySettingListDto : public oatpp::DTO {
  DTO_INIT(ActPutDevicePerStreamPrioritySettingListDto, DTO)

  DTO_FIELD(List<Object<ActPutDevicePerStreamPrioritySettingDto>>, device_per_stream_priority_setting_list,
            "DevicePerStreamPrioritySettingList");
};

// GCL (802.1Qbv)

class ActDeviceTimeSlotDto : public oatpp::DTO {
  DTO_INIT(ActDeviceTimeSlotDto, DTO)

  DTO_FIELD(Int64, slot_id, "SlotId");
  DTO_FIELD(Float64, interval, "Interval");
  DTO_FIELD(List<Int8>, queue_set, "QueueSet");
};

class ActDeviceTimeSlotPortSettingDto : public oatpp::DTO {
  DTO_INIT(ActDeviceTimeSlotPortSettingDto, DTO)

  DTO_FIELD(Int32, port_id, "PortId") = 1;
  DTO_FIELD(String, port_name, "PortName") = "1";
  DTO_FIELD(Boolean, active, "Active") = true;

  DTO_FIELD(Float64, cycle_time, "CycleTime") = 100.0;

  DTO_FIELD(List<Object<ActDeviceTimeSlotDto>>, gate_control_list, "GateControlList");
};

class ActGetDeviceTimeSlotSettingDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceTimeSlotSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId") = 1;
  DTO_FIELD(String, device_ip, "DeviceIp") = "100.100.100.100";
  DTO_FIELD(String, device_alias, "DeviceAlias") = "alias";
  DTO_FIELD(Boolean, support, "Support") = true;

  DTO_FIELD(List<Object<ActDeviceTimeSlotPortSettingDto>>, port_list, "PortList");
};

class ActGetDeviceTimeSlotSettingListDto : public oatpp::DTO {
  DTO_INIT(ActGetDeviceTimeSlotSettingListDto, DTO)

  DTO_FIELD(List<Object<ActGetDeviceTimeSlotSettingDto>>, device_time_slot_setting_list, "DeviceTimeSlotSettingList");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActPutDeviceTimeSlotSettingDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceTimeSlotSettingDto, DTO)

  DTO_FIELD(Int64, device_id, "DeviceId");

  DTO_FIELD(List<Object<ActDeviceTimeSlotPortSettingDto>>, port_list, "PortList");
};

class ActPutDeviceTimeSlotSettingListDto : public oatpp::DTO {
  DTO_INIT(ActPutDeviceTimeSlotSettingListDto, DTO)

  DTO_FIELD(List<Object<ActPutDeviceTimeSlotSettingDto>>, device_time_slot_setting_list, "DeviceTimeSlotSettingList");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_DEVICE_SETTING_DTO_HPP */
