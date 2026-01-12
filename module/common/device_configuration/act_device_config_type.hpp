/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include "act_json.hpp"

/**
 * @brief The deploy action enum class
 *
 */
enum class ActDeviceConfigTypeEnum {
  // kReboot = 1,
  // kFactoryDefault = 2,
  // kFirmwareUpgrade = 3,
  // kLocator = 4,
  kNetworkSetting = 5,
  kInformationSetting = 6,
  kLoginPolicy = 7,
  kSNMPTrapSetting = 8,
  kSyslogSetting = 9,
  kTimeSetting = 10,
  kLoopProtection = 11,
  kVLANSetting = 12,
  kPortSetting = 13,
  kSTPRSTP = 14,
  kManagementInterface = 15,
};

/**
 * @brief The QMap for deploy action enum mapping
 *
 */
static const QMap<QString, ActDeviceConfigTypeEnum> kActDeviceConfigTypeEnumMap = {
    {"NetworkSetting", ActDeviceConfigTypeEnum::kNetworkSetting},
    {"InformationSetting", ActDeviceConfigTypeEnum::kInformationSetting},
    {"LoginPolicy", ActDeviceConfigTypeEnum::kLoginPolicy},
    {"SNMPTrapSetting", ActDeviceConfigTypeEnum::kSNMPTrapSetting},
    {"SyslogSetting", ActDeviceConfigTypeEnum::kSyslogSetting},
    {"TimeSetting", ActDeviceConfigTypeEnum::kTimeSetting},
    {"LoopProtection", ActDeviceConfigTypeEnum::kLoopProtection},
    {"VLANSetting", ActDeviceConfigTypeEnum::kVLANSetting},
    {"PortSetting", ActDeviceConfigTypeEnum::kPortSetting},
    {"STPRSTP", ActDeviceConfigTypeEnum::kSTPRSTP},
    {"ManagementInterface", ActDeviceConfigTypeEnum::kManagementInterface}};