#ifndef __CLASSBASED_DEVICEACCOUNT_H__
#define __CLASSBASED_DEVICEACCOUNT_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateDeviceAccountNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                 UaString& errorMessage);

UaStatus addDeviceAccountMethod(const UaNodeId& deviceAccountFolderNodeId,
                                const MoxaClassBased::DeviceAccountDataType& configuration,
                                UaNodeId& deviceAccountNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus setDeviceAccountMethod(const UaNodeId& deviceAccountFolderNodeId,
                                const MoxaClassBased::DeviceAccountDataType& configuration, OpcUa_UInt32& errorCode,
                                UaString& errorMessage);

UaStatus removeDeviceAccountMethod(const UaNodeId& deviceAccountFolderNodeId, const UaNodeId& deviceAccountNodeId,
                                   OpcUa_UInt32& errorCode, UaString& errorMessage);

}  // namespace ClassBased

#endif