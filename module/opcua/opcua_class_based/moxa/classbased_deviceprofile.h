#ifndef __CLASSBASED_DEVICEPROFILE_H__
#define __CLASSBASED_DEVICEPROFILE_H__

#include "classbased_nodemanagermoxans.h"

namespace ClassBased {

UaStatus createDeviceProfileNode(const ActDeviceProfile& deviceProfile, OpcUa_UInt32& errorCode,
                                 UaString& errorMessage);

UaStatus removeDeviceProfileNode(const ActDeviceProfile& deviceProfile, OpcUa_UInt32& errorCode,
                                 UaString& errorMessage);

UaStatus checkDeviceProfileMethod(const UaString& deviceProfileName, OpcUa_Boolean& registered, OpcUa_UInt32& errorCode,
                                  UaString& errorMessage);

UaStatus importDeviceProfileMethod(const UaString& profileData, UaNodeId& deviceProfileNodeId, OpcUa_UInt32& errorCode,
                                   UaString& errorMessage);

UaStatus removeDeviceProfileMethod(const UaNodeId& deviceProfileNodeId, OpcUa_UInt32& errorCode,
                                   UaString& errorMessage);
}  // namespace ClassBased

#endif