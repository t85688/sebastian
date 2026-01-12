#ifndef __CLASSBASED_EXPORTTRAPEVENT_H__
#define __CLASSBASED_EXPORTTRAPEVENT_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus startExportTrapEventMethod(const UaNodeId& nodeId, const UaString& filePath);

UaStatus stopExportTrapEventMethod(const UaNodeId& nodeId);

void runExportTrapEventMethod(const UaNodeId& nodeId);
}  // namespace ClassBased

#endif