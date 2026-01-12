#ifndef __CLASSBASED_BRIDGE_H__
#define __CLASSBASED_BRIDGE_H__

#include "classbased_device.h"
#include "classbased_deviceaccount.h"
#include "classbased_managementinterface.h"
#include "classbased_multicaststaticforward.h"
#include "classbased_nodemanagermoxans.h"
#include "classbased_perstreampriority.h"
#include "classbased_snmptrapserver.h"
#include "classbased_spanningtree.h"
#include "classbased_syslogserver.h"
#include "classbased_timeawareshaper.h"
#include "classbased_timesync.h"
#include "classbased_unicaststaticforward.h"
#include "classbased_vlan.h"

namespace ClassBased {

UaStatus updateBridgeNode(const ActProject& project, const ActDevice& device, OpcUa_UInt32& errorCode,
                          UaString& errorMessage);

UaStatus addBridgeMethod(const UaNodeId& folderNodeId, const MoxaClassBased::BridgeDataType& configuration,
                         UaNodeId& bridgeNodeId, OpcUa_UInt32& errorCode, UaString& errorMessage);

UaStatus setBridgeSettingMethod(const UaNodeId& bridgeNodeId, const MoxaClassBased::BridgeDataType& configuration,
                                OpcUa_UInt32& errorCode, UaString& errorMessage);
}  // namespace ClassBased

#endif