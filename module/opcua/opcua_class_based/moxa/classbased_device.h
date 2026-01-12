#ifndef __CLASSBASED_DEVICE_H__
#define __CLASSBASED_DEVICE_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {
UaStatus updateDeviceNodes(const ActProject& project, OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus updateDeviceNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                          UaString& errorMessage);

UaStatus removeDeviceMethod(const UaNodeId& deviceNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage);

}  // namespace ClassBased
#endif