#ifndef __CLASSBASED_REBOOT_H__
#define __CLASSBASED_REBOOT_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus startRebootMethod(const UaNodeId& nodeId);

UaStatus stopRebootMethod(const UaNodeId& nodeId);

void runRebootMethod(const UaNodeId& nodeId);
}  // namespace ClassBased

#endif