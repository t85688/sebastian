#ifndef __CLASSBASED_MULTICASTSTATICFORWARD_H__
#define __CLASSBASED_MULTICASTSTATICFORWARD_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateMulticastStaticForwardNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                          UaString& errorMessage);

UaStatus addMulticastStaticForwardMethod(const UaNodeId& multicastStaticForwardFolderNodeId,
                                         const MoxaClassBased::MulticastStaticForwardDataType& configuration,
                                         UaNodeId& multicastStaticForwardNodeId, OpcUa_UInt32& errorCode,
                                         UaString& errorMessage);

UaStatus setMulticastStaticForwardMethod(const UaNodeId& multicastStaticForwardFolderNodeId,
                                         const MoxaClassBased::MulticastStaticForwardDataType& configuration,
                                         OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus removeMulticastStaticForwardMethod(const UaNodeId& multicastStaticForwardFolderNodeId,
                                            const UaNodeId& multicastStaticForwardNodeId, OpcUa_UInt32& errorCode,
                                            UaString& errorMessage);

}  // namespace ClassBased

#endif