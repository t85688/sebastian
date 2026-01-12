#ifndef __CLASSBASED_SYSLOGSERVER_H__
#define __CLASSBASED_SYSLOGSERVER_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateSyslogServerNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                                UaString& errorMessage);

UaStatus setSyslogServerMethod(const UaNodeId& nodeId, const MoxaClassBased::SyslogServerDataType& configuration,
                               OpcUa_UInt32& errorCode, UaString& errorMessage);

}  // namespace ClassBased

#endif