#ifndef __CLASSBASED_PERSTREAMPRIORITY_H__
#define __CLASSBASED_PERSTREAMPRIORITY_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updatePerStreamPriorityNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                     UaString& errorMessage);

UaStatus addPerStreamPriorityMethod(const UaNodeId& perStreamPriorityFolderNodeId,
                                    const MoxaClassBased::PerStreamPriorityDataType& configuration, UaNodeId& nodeId,
                                    OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus setPerStreamPriorityMethod(const UaNodeId& perStreamPriorityFolderNodeId,
                                    const MoxaClassBased::PerStreamPriorityDataType& configuration,
                                    OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus removePerStreamPriorityMethod(const UaNodeId& perStreamPriorityFolderNodeId, const UaNodeId& nodeId,
                                       OpcUa_UInt32& errorCode, UaString& errorMessage);
}  // namespace ClassBased

#endif