#ifndef __CLASSBASED_FACTORYDEFAULT_H__
#define __CLASSBASED_FACTORYDEFAULT_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus startFactoryDefaultMethod(const UaNodeId& nodeId);

UaStatus stopFactoryDefaultMethod(const UaNodeId& nodeId);

void runFactoryDefaultMethod(const UaNodeId& nodeId);
}  // namespace ClassBased

#endif