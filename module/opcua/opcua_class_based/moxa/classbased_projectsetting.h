#ifndef __CLASSBASED_PROJECTSETTING_H__
#define __CLASSBASED_PROJECTSETTING_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateProjectSettingNode(const ActProject& project, OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus setProjectSettingMethod(const UaNodeId& projectNodeId,
                                 const MoxaClassBased::ProjectSettingDataType& configuration, OpcUa_UInt32& errorCode,
                                 UaString& errorMessage);
}  // namespace ClassBased

#endif