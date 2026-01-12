#ifndef __CLASSBASED_TSNSTREAM_H__
#define __CLASSBASED_TSNSTREAM_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateTsnStreamNodes(const ActProject& project, const ActTrafficDesign& traffic_design,
                              OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus addTsnStreamMethod(const UaNodeId& tsnStreamFolderNodeId,
                            const MoxaClassBased::TsnStreamDataType& configuration, UaNodeId& tsnStreamNodeId,
                            OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus setTsnStreamMethod(const UaNodeId& tsnStreamNodeId, const MoxaClassBased::TsnStreamDataType& configuration,
                            OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus removeTsnStreamMethod(const UaNodeId& tsnStreamFolderNodeId, const UaNodeId& tsnStreamNodeId,
                               OpcUa_UInt32& errorCode, UaString& errorMessage);

}  // namespace ClassBased

#endif