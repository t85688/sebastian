#ifndef __CLASSBASED_LINK_H__
#define __CLASSBASED_LINK_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateLinkNodes(const ActProject& project, OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus addLinkMethod(const UaNodeId& folderNodeId, const MoxaClassBased::LinkDataType& configuration,
                       UaNodeId& linkNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus removeLinkMethod(const UaNodeId& linkNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage);

}  // namespace ClassBased
#endif