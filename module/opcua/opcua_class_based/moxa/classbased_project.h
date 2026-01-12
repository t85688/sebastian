#ifndef __CLASSBASED_PROJECT_H__
#define __CLASSBASED_PROJECT_H__

#include "classbased_nodemanagermoxans.h"
namespace ClassBased {
UaStatus createProjectNode(const ActProject& project, OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus updateProjectNode(const ActProject& actProject, OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus removeProjectNode(const ActProject& project, OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus addProjectMethod(const UaString& projectName, UaNodeId& projectNodeId, OpcUa_UInt32& errorCode,
                          UaString& errorMessage);
                          
UaStatus removeProjectMethod(const UaString& projectName, OpcUa_UInt32& errorCode, UaString& errorMessage);

}  // namespace ClassBased
#endif