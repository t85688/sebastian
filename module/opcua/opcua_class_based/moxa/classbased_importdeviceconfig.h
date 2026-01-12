#ifndef __CLASSBASED_IMPORTDEVICECONFIG_H__
#define __CLASSBASED_IMPORTDEVICECONFIG_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus startImportDeviceConfigMethod(const UaNodeId& nodeId, const UaString& filePath);

UaStatus stopImportDeviceConfigMethod(const UaNodeId& nodeId);

void runImportDeviceConfigMethod(const UaNodeId& nodeId);
}  // namespace ClassBased

#endif