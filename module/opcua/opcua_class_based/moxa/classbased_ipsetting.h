#ifndef __CLASSBASED_IPSETTING_H__
#define __CLASSBASED_IPSETTING_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateIpSettingNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                             UaString& errorMessage);

UaStatus setIpSettingMethod(const UaNodeId& ipSettingNodeId, const MoxaClassBased::IpSettingDataType& configuration,
                            OpcUa_UInt32& errorCode, UaString& errorMessage);

}  // namespace ClassBased

#endif