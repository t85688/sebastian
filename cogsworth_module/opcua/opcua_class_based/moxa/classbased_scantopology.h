#ifndef __CLASSBASED_SCANTOPOLOGY_H__
#define __CLASSBASED_SCANTOPOLOGY_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus startScanTopologyMethod(const UaNodeId& nodeId, const MoxaClassBased::AutoScanDataTypes& NetworkIpRange);

UaStatus stopScanTopologyMethod(const UaNodeId& nodeId);

void runScanTopologyMethod(const UaNodeId& nodeId);
}  // namespace ClassBased

#endif