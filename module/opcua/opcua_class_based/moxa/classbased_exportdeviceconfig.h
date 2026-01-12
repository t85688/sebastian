#ifndef __CLASSBASED_EXPORTDEVICECONFIG_H__
#define __CLASSBASED_EXPORTDEVICECONFIG_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus startExportDeviceConfigMethod(const UaNodeId& nodeId, const UaString& filePath);

UaStatus stopExportDeviceConfigMethod(const UaNodeId& nodeId);

void runExportDeviceConfigMethod(const UaNodeId& nodeId);
}  // namespace ClassBased

#endif