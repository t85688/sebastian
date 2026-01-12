#ifndef __CLASSBASED_SPANNINGTREE_H__
#define __CLASSBASED_SPANNINGTREE_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateSpanningTreeNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                UaString& errorMessage);

UaStatus setSpanningTreeMethod(const UaNodeId& nodeId, const MoxaClassBased::SpanningTreeDataType& configuration,
                               OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus setSpanningTreePortMethod(const UaNodeId& nodeId,
                                   const MoxaClassBased::SpanningTreePortDataType& configuration,
                                   OpcUa_UInt32& errorCode, UaString& errorMessage);

}  // namespace ClassBased

#endif