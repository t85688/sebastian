#ifndef __CLASSBASED_FIRMWAREUPGRADE_H__
#define __CLASSBASED_FIRMWAREUPGRADE_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus startFirmwareUpgradeMethod(const UaNodeId& nodeId, const UaString& filePath);

UaStatus stopFirmwareUpgradeMethod(const UaNodeId& nodeId);

void runFirmwareUpgradeMethod(const UaNodeId& nodeId);
}  // namespace ClassBased

#endif