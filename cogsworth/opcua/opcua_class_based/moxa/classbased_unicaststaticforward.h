#ifndef __CLASSBASED_UNICASTSTATICFORWARD_H__
#define __CLASSBASED_UNICASTSTATICFORWARD_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateUnicastStaticForwardNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                        UaString& errorMessage);

UaStatus addUnicastStaticForwardMethod(const UaNodeId& unicastStaticForwardFolderNodeId,
                                       const MoxaClassBased::UnicastStaticForwardDataType& configuration,
                                       UaNodeId& unicastStaticForwardNodeId, OpcUa_UInt32& errorCode,
                                       UaString& errorMessage);

UaStatus setUnicastStaticForwardMethod(const UaNodeId& unicastStaticForwardFolderNodeId,
                                       const MoxaClassBased::UnicastStaticForwardDataType& configuration,
                                       OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus removeUnicastStaticForwardMethod(const UaNodeId& unicastStaticForwardFolderNodeId,
                                          const UaNodeId& unicastStaticForwardNodeId, OpcUa_UInt32& errorCode,
                                          UaString& errorMessage);

}  // namespace ClassBased

#endif