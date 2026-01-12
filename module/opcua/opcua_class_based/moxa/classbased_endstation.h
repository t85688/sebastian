#ifndef __CLASSBASED_ENDSTATION_H__
#define __CLASSBASED_ENDSTATION_H__

#include "classbased_device.h"
#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateEndStationNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                              UaString& errorMessage);

UaStatus addEndStationMethod(const UaNodeId& folderNodeId, const MoxaClassBased::EndStationDataType& configuration,
                             UaNodeId& endstationNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus setEndStationSettingMethod(const UaNodeId& endstationNodeId,
                                    const MoxaClassBased::EndStationDataType& configuration, OpcUa_UInt32& errorCode,
                                    UaString& errorMessage);

}  // namespace ClassBased

#endif