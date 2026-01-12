#ifndef __CLASSBASED_TRAFFICCONFIGURATION_H__
#define __CLASSBASED_TRAFFICCONFIGURATION_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateTrafficConfigurationNodes(const ActProject& project, const ActTrafficDesign& traffic_design,
                                         OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus setTrafficConfigurationEntryMethod(const UaNodeId& nodeId,
                                            const MoxaClassBased::TrafficConfigurationEntryDataType configuration,
                                            OpcUa_UInt32& errorCode, UaString& errorMessage);
}  // namespace ClassBased

#endif