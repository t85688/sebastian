#ifndef __CLASSBASED_COMPUTE_H__
#define __CLASSBASED_COMPUTE_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus startComputeMethod(const UaNodeId& nodeId);

UaStatus stopComputeMethod(const UaNodeId& nodeId);

void runComputeMethod(const UaNodeId& nodeId);
}  // namespace ClassBased

#endif