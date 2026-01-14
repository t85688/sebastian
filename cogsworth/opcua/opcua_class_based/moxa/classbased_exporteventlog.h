#ifndef __CLASSBASED_EXPORTEVENTLOG_H__
#define __CLASSBASED_EXPORTEVENTLOG_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus startExportEventLogMethod(const UaNodeId& nodeId, const UaString& filePath);

UaStatus stopExportEventLogMethod(const UaNodeId& nodeId);

void runExportEventLogMethod(const UaNodeId& nodeId);
}  // namespace ClassBased

#endif