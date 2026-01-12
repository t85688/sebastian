#ifndef __CLASSBASED_TRAFFICDESIGN_H__
#define __CLASSBASED_TRAFFICDESIGN_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {
UaStatus updateTrafficDesignNodes(const ActProject& project, OpcUa_UInt32& errorCode, UaString& errorMessage);


UaStatus getAllStreamComputedResultMethod(const UaNodeId& nodeId,
                                          MoxaClassBased::StreamComputedResultDataTypes& StreamResult,
                                          OpcUa_UInt32& ErrorCode, UaString& ErrorMessage);

UaStatus getStreamComputedResultMethod(const UaNodeId& nodeId, const UaString& StreamName,
                                       MoxaClassBased::StreamComputedResultDataType& StreamResult,
                                       OpcUa_UInt32& ErrorCode, UaString& ErrorMessage);
}  // namespace ClassBased
#endif