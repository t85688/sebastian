#ifndef __CLASSBASED_SYNCDEVICECONFIG_H__
#define __CLASSBASED_SYNCDEVICECONFIG_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus startSyncDeviceConfigMethod(const UaNodeId& nodeId);

UaStatus stopSyncDeviceConfigMethod(const UaNodeId& nodeId);

void runSyncDeviceConfigMethod(const UaNodeId& nodeId);
}  // namespace ClassBased

#endif