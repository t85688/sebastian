#include "opcua_class_based_server.h"

#include "classbased_bridge.h"
#include "classbased_device.h"
#include "classbased_deviceprofile.h"
#include "classbased_link.h"
#include "classbased_project.h"
#include "classbased_projectsetting.h"

namespace MoxaOpcUaClassBased {

ACT_STATUS updateOpcUaProject(const ActProject &project) {
  UaStatus ret;
  ACT_STATUS_INIT();

  OpcUa_UInt32 errorCode = M_UA_NO_ERROR;
  UaString errorMessage;
  ret = ClassBased::updateProjectNode(project, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return std::make_shared<ActBadRequest>(QString(errorMessage.toUtf8()));
  }
  return act_status;
}

ACT_STATUS removeOpcUaProject(const ActProject &project) {
  UaStatus ret;
  ACT_STATUS_INIT();

  OpcUa_UInt32 errorCode = M_UA_NO_ERROR;
  UaString errorMessage;
  ret = ClassBased::removeProjectNode(project, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return std::make_shared<ActBadRequest>(QString(errorMessage.toUtf8()));
  }
  return act_status;
}

ACT_STATUS uploadOpcUaDeviceProfile(const ActDeviceProfile &deviceProfile) {
  UaStatus ret;
  ACT_STATUS_INIT();

  OpcUa_UInt32 errorCode = M_UA_NO_ERROR;
  UaString errorMessage;
  ret = ClassBased::createDeviceProfileNode(deviceProfile, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return std::make_shared<ActBadRequest>(QString(errorMessage.toUtf8()));
  }
  return act_status;
}

ACT_STATUS removeOpcUaDeviceProfile(const ActDeviceProfile &deviceProfile) {
  UaStatus ret;
  ACT_STATUS_INIT();

  OpcUa_UInt32 errorCode = M_UA_NO_ERROR;
  UaString errorMessage;
  ret = ClassBased::removeDeviceProfileNode(deviceProfile, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return std::make_shared<ActBadRequest>(QString(errorMessage.toUtf8()));
  }
  return act_status;
}
}  // namespace MoxaOpcUaClassBased