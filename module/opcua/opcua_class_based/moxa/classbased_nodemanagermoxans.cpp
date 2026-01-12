#include "classbased_nodemanagermoxans.h"

#include <iostream>

#include "classbased_deviceprofile.h"
#include "classbased_project.h"
namespace ClassBased {

MoxaNodeManager* pMoxaNodeManager = NULL;
MoxaClassBased::NodeManagerMoxaClassBasedNS* pClassBased = NULL;
OpcUaClassBnm::NodeManagerOpcUaClassBnmNS* pOpcUaClassBnm = NULL;

// public method
MoxaNodeManager::MoxaNodeManager() : NodeManagerBase("http://www.moxa.com/UA/") {}

MoxaNodeManager::~MoxaNodeManager() {}

UaStatus MoxaNodeManager::afterStartUp() {
  UaStatus ret;

  // ACT core
  OpcUa_UInt32 errorCode = M_UA_NO_ERROR;
  UaString errorMessage;

  for (ActDeviceProfile deviceProfile : act::core::g_core.GetDeviceProfileSet()) {
    UaString modelName(deviceProfile.GetModelName().toStdString().c_str());
    ret = ClassBased::createDeviceProfileNode(deviceProfile, errorCode, errorMessage);
    if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
      qDebug() << UaString("Invalid: Add device profile \"%1\" from ACT failed").arg(modelName).toUtf8();
    }
  }

  for (ActProject project : act::core::g_core.GetProjectSet()) {
    UaString projectName(project.GetProjectName().toStdString().c_str());
    ret = ClassBased::updateProjectNode(project, errorCode, errorMessage);
    if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
      qDebug() << UaString("Invalid: Add project \"%1\" from ACT failed").arg(projectName).toUtf8();
    }
  }

  return ret;
}

UaStatus MoxaNodeManager::beforeShutDown() {
  UaStatus ret;
  return ret;
}

void MoxaNodeManager::getNodeReference(UaNodeId startingNode, OpcUa_Boolean isInverse,
                                       UaReferenceDescriptions& references) {
  UaStatus ret;

  // Starting node of browse
  ServerManager* pServerManager = NodeManagerRoot::CreateRootNodeManager()->pServerManager();
  ContinuationPointWrapper continuationPoint;
  UaNodeId referenceFilter(OpcUaId_HierarchicalReferences);
  // UaReferenceDescriptions references;
  // Call internal browse, there are other overloads
  ret = pServerManager->browse(startingNode, isInverse, referenceFilter, 0, continuationPoint, references);
  UA_ASSERT(ret.isGood());
}

MoxaClassBased::SNMPVersion MoxaNodeManager::getSnmpVersion(const ActSnmpVersionEnum& actSnmpVersion) {
  static_cast<ActSnmpVersionEnum>(actSnmpVersion);
  return MoxaClassBased::SNMPVersion_v2c;
}

const ActSnmpVersionEnum MoxaNodeManager::setSnmpVersion(const MoxaClassBased::SNMPVersion& SnmpVersion) {
  static_cast<MoxaClassBased::SNMPVersion>(SnmpVersion);
  return ActSnmpVersionEnum::kV2c;
}

MoxaClassBased::LinkCableType MoxaNodeManager::getCableType(const ActCableTypeEnum& actCableType) {
  if (actCableType == ActCableTypeEnum::kCopper) {
    return MoxaClassBased::LinkCableType_Copper;
  } else {
    return MoxaClassBased::LinkCableType_Fiber;
  }
}

const ActCableTypeEnum MoxaNodeManager::setCableType(const MoxaClassBased::LinkCableType linkCableType) {
  if (linkCableType == MoxaClassBased::LinkCableType_Copper) {
    return ActCableTypeEnum::kCopper;
  } else {
    return ActCableTypeEnum::kFiber;
  }
}

bool MoxaNodeManager::checkMacAddress(const QString macAddress) {
  // XX-XX-XX-XX-XX-XX or XX:XX:XX:XX:XX:XX
  QRegularExpression reg("^([0-9A-Fa-f]{2}([:-]|$)){6}$");

  // Check MAC address format
  QRegularExpressionMatch match = reg.match(macAddress);
  return match.hasMatch();
}

bool MoxaNodeManager::checkIpAddress(const QString ipAddress, const uint8_t type) {
  // Regex expression for validating IPv4
  QRegularExpression ipv4("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\\.|$)){4}$");
  // Regex expression for validating IPv6
  QRegularExpression ipv6("^((([0-9a-fA-F]){1,4})\\:|$){8}$");

  // Check IP address format
  QRegularExpressionMatch match;
  if (type == 4) {
    match = ipv4.match(ipAddress);
  } else if (type == 6) {
    match = ipv6.match(ipAddress);
  }
  return match.hasMatch();
}

bool MoxaNodeManager::isDeviceProfileExist(const UaString& deviceProfileName) {
  return this->deviceProfileIdMap.contains(deviceProfileName);
}
void MoxaNodeManager::setDeviceProfileName(const UaString& deviceProfileName, const UaNodeId& nodeId,
                                           const qint64& device_profile_id, const ActDeviceTypeEnum& device_type) {
  this->deviceProfileIdMap[deviceProfileName] = QPair(nodeId, QPair(device_profile_id, device_type));
}
void MoxaNodeManager::removeDeviceProfileName(const UaString& deviceProfileName) {
  this->deviceProfileIdMap.remove(deviceProfileName);
}
qint64 MoxaNodeManager::getDeviceProfileId(const UaString& deviceProfileName) {
  return this->deviceProfileIdMap[deviceProfileName].second.first;
}
ActDeviceTypeEnum MoxaNodeManager::getDeviceProfileType(const UaString& deviceProfileName) {
  return this->deviceProfileIdMap[deviceProfileName].second.second;
}

qint64 MoxaNodeManager::getProjectId(const UaNodeId& nodeId) {
  QString nodeIdStr(nodeId.toString().toUtf8());
  QStringList str_list = nodeIdStr.split(".");
  return str_list[1].toUInt();
}

qint64 MoxaNodeManager::getDeviceId(const UaNodeId& nodeId) {
  QString nodeIdStr(nodeId.toString().toUtf8());
  QStringList str_list = nodeIdStr.split(".");
  return str_list[3].toUInt();
}

qint64 MoxaNodeManager::getInterfaceId(const UaNodeId& nodeId) {
  QString nodeIdStr(nodeId.toString().toUtf8());
  QStringList str_list = nodeIdStr.split(".");
  return str_list[4].toUInt();
}

qint64 MoxaNodeManager::getLinkId(const UaNodeId& nodeId) {
  QString nodeIdStr(nodeId.toString().toUtf8());
  QStringList str_list = nodeIdStr.split(".");
  return str_list[3].toUInt();
}

qint64 MoxaNodeManager::getTsnStreamApplicationId(const UaNodeId& nodeId) {
  QString nodeIdStr(nodeId.toString().toUtf8());
  QStringList str_list = nodeIdStr.split(".");
  return str_list[4].toUInt();
}

qint64 MoxaNodeManager::getTsnStreamId(const UaNodeId& nodeId) {
  QString nodeIdStr(nodeId.toString().toUtf8());
  QStringList str_list = nodeIdStr.split(".");
  return str_list[4].toUInt();
}

UaNodeId MoxaNodeManager::getDeviceProfileNodeId(const qint64& device_profile_id) {
  return UaNodeId(UaString("DeviceProfiles.%1").arg(unsigned int(device_profile_id)), this->getNameSpaceIndex());
}

UaNodeId MoxaNodeManager::getProjectNodeId(const qint64& project_id) {
  return UaNodeId(UaString("Projects.%1").arg(unsigned int(project_id)), this->getNameSpaceIndex());
}

UaNodeId MoxaNodeManager::getDeviceNodeId(const qint64& project_id, const qint64& device_id) {
  return UaNodeId(UaString("Projects.%1.Devices.%2").arg(unsigned int(project_id)).arg(unsigned int(device_id)),
                  this->getNameSpaceIndex());
}

UaNodeId MoxaNodeManager::getInterfaceNodeId(const qint64& project_id, const qint64& device_id,
                                             const qint64& interface_id) {
  return UaNodeId(UaString("Projects.%1.Devices.%2.%3")
                      .arg(unsigned int(project_id))
                      .arg(unsigned int(device_id))
                      .arg(unsigned int(interface_id)),
                  this->getNameSpaceIndex());
}

UaNodeId MoxaNodeManager::getLinkNodeId(const qint64& project_id, const qint64& link_id) {
  return UaNodeId(UaString("Projects.%1.Links.%2").arg(unsigned int(project_id)).arg(unsigned int(link_id)),
                  this->getNameSpaceIndex());
}

UaNodeId MoxaNodeManager::getTsnStreamFolderNodeId(const qint64& project_id) {
  return UaNodeId(UaString("Projects.%1.TrafficDesign.TsnStream").arg(unsigned int(project_id)),
                  this->getNameSpaceIndex());
}

UaNodeId MoxaNodeManager::getTsnStreamNodeId(const qint64& project_id, const qint64& stream_id) {
  return UaNodeId(
      UaString("Projects.%1.TrafficDesign.TsnStream.%2").arg(unsigned int(project_id)).arg(unsigned int(stream_id)),
      this->getNameSpaceIndex());
}

UaNodeId MoxaNodeManager::getTsnStreamTalkerNodeId(const qint64& project_id, const qint64& stream_id,
                                                   const QString& talker_interface) {
  return UaNodeId(UaString("Projects.%1.TrafficDesign.TsnStream.%2.TalkerInterface.%3")
                      .arg(unsigned int(project_id))
                      .arg(unsigned int(stream_id))
                      .arg(talker_interface.toStdString().c_str()),
                  this->getNameSpaceIndex());
}

UaNodeId MoxaNodeManager::getTsnStreamListenerNodeId(const qint64& project_id, const qint64& stream_id,
                                                     const QString& listener_interface) {
  return UaNodeId(UaString("Projects.%1.TrafficDesign.TsnStream.%2.ListenerInterfaces.%3")
                      .arg(unsigned int(project_id))
                      .arg(unsigned int(stream_id))
                      .arg(listener_interface.toStdString().c_str()),
                  this->getNameSpaceIndex());
}

UaNodeId MoxaNodeManager::getTsnStreamApplicationFolderNodeId(const qint64& project_id) {
  return UaNodeId(UaString("Projects.%1.TrafficDesign.TsnStreamApplication").arg(unsigned int(project_id)),
                  this->getNameSpaceIndex());
}

UaNodeId MoxaNodeManager::getTsnStreamApplicationNodeId(const qint64& project_id, const qint64& application_id) {
  return UaNodeId(UaString("Projects.%1.TrafficDesign.TsnStreamApplication.%2")
                      .arg(unsigned int(project_id))
                      .arg(unsigned int(application_id)),
                  this->getNameSpaceIndex());
}
void MoxaNodeManager::sendWsFromUaThread(QString payload) {
  // 把呼叫排隊回 WsClient 所在的 thread
  QMetaObject::invokeMethod(ws_, "sendRequest", Qt::QueuedConnection, Q_ARG(QString, payload));
}

}  // namespace ClassBased
