#ifndef __CLASSBASED_TIMESYNC_H__
#define __CLASSBASED_TIMESYNC_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateTimeSyncNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                            UaString& errorMessage);

UaStatus setTimeSyncMethod(const UaNodeId& nodeId, const MoxaClassBased::TimeSyncDataType& configuration,
                           OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus setIeeeDot1AS2011PortMethod(const UaNodeId& nodeId,
                                     const MoxaClassBased::IeeeDot1AS2011PortDataType& configuration,
                                     OpcUa_UInt32& errorCode, UaString& errorMessage);
}  // namespace ClassBased

#endif