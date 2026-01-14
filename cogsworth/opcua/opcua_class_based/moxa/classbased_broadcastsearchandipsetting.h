#ifndef __CLASSBASED_BROADCASTSEARCHANDIPSETTING_H__
#define __CLASSBASED_BROADCASTSEARCHANDIPSETTING_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus startBroadcastSearchAndIPSettingMethod(
    UaNode* pUaNode,
    MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::BroadcastSearchAndIPSettingState state);

void runBroadcastSearchAndIPSettingMethod(
    UaNode* pUaNode,
    MoxaClassBased::BroadcastSearchAndIPSettingStateMachineType::BroadcastSearchAndIPSettingState state);

UaStatus getDiscoveredDevicesMethod(UaNode* pUaNode, MoxaClassBased::DiscoveredDeviceDataTypes& discoveredDevices);
}  // namespace ClassBased

#endif