#ifndef __CLASSBASED_TSNSTREAMAPPLICATION_H__
#define __CLASSBASED_TSNSTREAMAPPLICATION_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateTsnStreamApplicationNodes(const ActProject& project, const ActTrafficDesign& traffic_design,
                                         OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus addTsnStreamApplicationMethod(const UaNodeId& tsnStreamApplicationFolderNodeId,
                                       const MoxaClassBased::TsnStreamApplicationDataType& configuration,
                                       UaNodeId& tsnStreamApplicationNodeId, OpcUa_UInt32& errorCode,
                                       UaString& errorMessage);

UaStatus setTsnStreamApplicationMethod(const UaNodeId& tsnStreamApplicationFolderNodeId,
                                       const MoxaClassBased::TsnStreamApplicationDataType& configuration,
                                       OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus removeTsnStreamApplicationMethod(const UaNodeId& tsnStreamApplicationFolderNodeId,
                                          const UaNodeId& tsnStreamApplicationNodeId, OpcUa_UInt32& errorCode,
                                          UaString& errorMessage);

}  // namespace ClassBased

#endif