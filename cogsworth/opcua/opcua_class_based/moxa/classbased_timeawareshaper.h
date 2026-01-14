#ifndef __CLASSBASED_TIMEAWARESHAPER_H__
#define __CLASSBASED_TIMEAWARESHAPER_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateTimeAwareShaperNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                   UaString& errorMessage);

UaStatus insertTimeAwareShaperMethod(const UaNodeId& timeAwareShaperNodeId,
                                     const MoxaClassBased::GateControlDataType& configuration, UaNodeId& nodeId,
                                     OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus setTimeAwareShaperMethod(const UaNodeId& timeAwareShaperNodeId,
                                  const MoxaClassBased::GateControlDataType& configuration, OpcUa_UInt32& errorCode,
                                  UaString& errorMessage);

UaStatus removeTimeAwareShaperMethod(const UaNodeId& timeAwareShaperNodeId, const OpcUa_Int64& index,
                                     OpcUa_UInt32& errorCode, UaString& errorMessage);

}  // namespace ClassBased

#endif