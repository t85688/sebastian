#ifndef __CLASSBASED_VLAN_H__
#define __CLASSBASED_VLAN_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus updateVlanNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                        UaString& errorMessage);

UaStatus addVlanMethod(const UaNodeId& vlanTableFolderNodeId, const MoxaClassBased::VlanDataType& configuration,
                       UaNodeId& vlanNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus setVlanMethod(const UaNodeId& vlanTableFolderNodeId, const MoxaClassBased::VlanDataType& configuration,
                       OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus removeVlanMethod(const UaNodeId& vlanTableFolderNodeId, const UaNodeId& vlanNodeId, OpcUa_UInt32& errorCode,
                          UaString& errorMessage);

UaStatus setVlanPortMethod(const UaNodeId& vlanPortNodeId, const MoxaClassBased::VlanPortDataType& configuration,
                           OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus setVlanSettingMethod(const UaNodeId& vlanSettingNodeId,
                              const MoxaClassBased::VlanSettingDataType& configuration, OpcUa_UInt32& errorCode,
                              UaString& errorMessage);
}  // namespace ClassBased

#endif