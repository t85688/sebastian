/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef OPC_UA_CLASS_BASED_SERVER_H
#define OPC_UA_CLASS_BASED_SERVER_H

#include "act_core.hpp"
#include "act_ws_client.hpp"

namespace MoxaOpcUaClassBased {

void opcua_class_based_server_start(WsClient *ws_client);
void opcua_class_based_server_stop();

ACT_STATUS updateOpcUaProject(const ActProject &project);
ACT_STATUS removeOpcUaProject(const ActProject &project);

ACT_STATUS uploadOpcUaDeviceProfile(const ActDeviceProfile &deviceProfile);
ACT_STATUS removeOpcUaDeviceProfile(const ActDeviceProfile &deviceProfile);

}  // namespace MoxaOpcUaClassBased
#endif
