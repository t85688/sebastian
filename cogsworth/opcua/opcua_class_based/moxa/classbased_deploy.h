#ifndef __CLASSBASED_DEPLOY_H__
#define __CLASSBASED_DEPLOY_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus startDeployMethod(const UaNodeId& nodeId);

UaStatus stopDeployMethod(const UaNodeId& nodeId);

void runDeployMethod(const UaNodeId& nodeId);
}  // namespace ClassBased

#endif