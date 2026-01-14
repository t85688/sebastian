#ifndef __CLASSBASED_SNMPTRAPSERVER_H__
#define __CLASSBASED_SNMPTRAPSERVER_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateSNMPTrapServerNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                  UaString& errorMessage);

UaStatus addSNMPTrapHostMethod(const UaNodeId& SNMPTrapServerNodeId,
                               const MoxaClassBased::SNMPTrapHostDataType& configuration, UaNodeId& SNMPTrapHostNodeId,
                               OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus setSNMPTrapHostMethod(const UaNodeId& SNMPTrapHostNodeId,
                               const MoxaClassBased::SNMPTrapHostDataType& configuration, OpcUa_UInt32& errorCode,
                               UaString& errorMessage);

UaStatus removeSNMPTrapHostMethod(const UaNodeId& SNMPTrapServerNodeId, const UaNodeId& SNMPTrapHostNodeId,
                                  OpcUa_UInt32& errorCode, UaString& errorMessage);

}  // namespace ClassBased

#endif