#ifndef __CLASSBASED_EXPORTSYSLOG_H__
#define __CLASSBASED_EXPORTSYSLOG_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus startExportSyslogMethod(const UaNodeId& nodeId, const UaString& filePath);

UaStatus stopExportSyslogMethod(const UaNodeId& nodeId);

void runExportSyslogMethod(const UaNodeId& nodeId);
}  // namespace ClassBased

#endif