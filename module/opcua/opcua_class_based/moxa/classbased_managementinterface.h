#ifndef __CLASSBASED_MANAGEMENTINTERFACE_H__
#define __CLASSBASED_MANAGEMENTINTERFACE_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateManagementInterfaceNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                       UaString& errorMessage);

UaStatus setManagementInterfaceMethod(const UaNodeId& nodeId,
                                      const MoxaClassBased::ManagementInterfaceDataType& configuration,
                                      OpcUa_UInt32& errorCode, UaString& errorMessage);

}  // namespace ClassBased

#endif