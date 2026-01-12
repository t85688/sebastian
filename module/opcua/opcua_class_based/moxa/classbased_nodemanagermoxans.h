#ifndef __CLASSBASED_NODEMANAGERMOXANS_H__
#define __CLASSBASED_NODEMANAGERMOXANS_H__

#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <thread>

#include "moxa_opcua_errorcode.h"
#include "moxa_opcua_limitation.h"
#include "moxaclassbased_datatypes.h"
#include "moxaclassbased_identifiers.h"
#include "moxaclassbased_nodemanagermoxaclassbasedns.h"
#include "nodemanagerbase.h"
#include "nodemanagerroot.h"
#include "opcuaclassbnm_nodemanageropcuaclassbnmns.h"
// moxaclassbased data type
#include "moxaclassbased_autoscandatatype.h"
#include "moxaclassbased_bridgedatatype.h"
#include "moxaclassbased_connectionaccountdatatype.h"
#include "moxaclassbased_deviceaccountdatatype.h"
#include "moxaclassbased_devicediscoverydatatype.h"
#include "moxaclassbased_discovereddevicedatatype.h"
#include "moxaclassbased_endstationdatatype.h"
#include "moxaclassbased_ethernetinterfacedatatype.h"
#include "moxaclassbased_gatecontroldatatype.h"
#include "moxaclassbased_ieeebasetsntrafficspecificationdatatype.h"
#include "moxaclassbased_ieeedot1as2011datatype.h"
#include "moxaclassbased_ieeedot1as2011portdatatype.h"
#include "moxaclassbased_ieeetsnvlantagdatatype.h"
#include "moxaclassbased_ipconfiguredatatype.h"
#include "moxaclassbased_ipsettingdatatype.h"
#include "moxaclassbased_linkdatatype.h"
#include "moxaclassbased_managementinterfacedatatype.h"
#include "moxaclassbased_multicaststaticforwarddatatype.h"
#include "moxaclassbased_netconfdatatype.h"
#include "moxaclassbased_perstreamprioritydatatype.h"
#include "moxaclassbased_projectsettingdatatype.h"
#include "moxaclassbased_restfuldatatype.h"
#include "moxaclassbased_snmpdatatype.h"
#include "moxaclassbased_snmptraphostdatatype.h"
#include "moxaclassbased_spanningtreedatatype.h"
#include "moxaclassbased_spanningtreeportdatatype.h"
#include "moxaclassbased_streamcomputedresultdatatype.h"
#include "moxaclassbased_syslogserverdatatype.h"
#include "moxaclassbased_timesyncdatatype.h"
#include "moxaclassbased_trafficconfigurationentrydatatype.h"
#include "moxaclassbased_tsninterfaceconfigurationlistenerdatatype.h"
#include "moxaclassbased_tsninterfaceconfigurationtalkerdatatype.h"
#include "moxaclassbased_tsnstreamapplicationdatatype.h"
#include "moxaclassbased_tsnstreamdatatype.h"
#include "moxaclassbased_tsntrafficspecificationdatatype.h"
#include "moxaclassbased_unicaststaticforwarddatatype.h"
#include "moxaclassbased_vlandatatype.h"
#include "moxaclassbased_vlanportdatatype.h"
// moxaclassbased folder type
#include "moxaclassbased_deviceaccountfoldertype.h"
#include "moxaclassbased_deviceconfigfoldertype.h"
#include "moxaclassbased_devicefoldertype.h"
#include "moxaclassbased_deviceprofilefoldertype.h"
#include "moxaclassbased_linkfoldertype.h"
#include "moxaclassbased_multicaststaticforwardfoldertype.h"
#include "moxaclassbased_operationfoldertype.h"
#include "moxaclassbased_perstreampriorityfoldertype.h"
#include "moxaclassbased_projectfoldertype.h"
#include "moxaclassbased_trafficdesignfoldertype.h"
#include "moxaclassbased_tsnstreamapplicationfoldertype.h"
#include "moxaclassbased_tsnstreamfoldertype.h"
#include "moxaclassbased_unicaststaticforwardfoldertype.h"
#include "moxaclassbased_vlantablefoldertype.h"
// moxaclassbased object type
#include "moxaclassbased_bridgetype.h"
#include "moxaclassbased_broadcastsearchandipsettingstatemachinetype.h"
#include "moxaclassbased_computestatemachinetype.h"
#include "moxaclassbased_connectionaccounttype.h"
#include "moxaclassbased_deploystatemachinetype.h"
#include "moxaclassbased_deviceaccounttype.h"
#include "moxaclassbased_deviceinformationtype.h"
#include "moxaclassbased_deviceprofiletype.h"
#include "moxaclassbased_devicetype.h"
#include "moxaclassbased_discoveredbridgetype.h"
#include "moxaclassbased_endstationtype.h"
#include "moxaclassbased_ethernetinterfacetype.h"
#include "moxaclassbased_exportdeviceconfigstatemachinetype.h"
#include "moxaclassbased_exporteventlogstatemachinetype.h"
#include "moxaclassbased_exportsyslogstatemachinetype.h"
#include "moxaclassbased_exporttrapeventstatemachinetype.h"
#include "moxaclassbased_factorydefaultstatemachinetype.h"
#include "moxaclassbased_firmwareupgradestatemachinetype.h"
#include "moxaclassbased_gatecontroltype.h"
#include "moxaclassbased_ieeedot1as2011porttype.h"
#include "moxaclassbased_ieeedot1as2011type.h"
#include "moxaclassbased_importdeviceconfigstatemachinetype.h"
#include "moxaclassbased_ipsettingtype.h"
#include "moxaclassbased_linktype.h"
#include "moxaclassbased_managementinterfacetype.h"
#include "moxaclassbased_multicaststaticforwardtype.h"
#include "moxaclassbased_netconftype.h"
#include "moxaclassbased_perstreamprioritytype.h"
#include "moxaclassbased_programstatemachinetype.h"
#include "moxaclassbased_projectsettingtype.h"
#include "moxaclassbased_projecttype.h"
#include "moxaclassbased_rebootstatemachinetype.h"
#include "moxaclassbased_restfultype.h"
#include "moxaclassbased_scantopologystatemachinetype.h"
#include "moxaclassbased_snmptraphosttype.h"
#include "moxaclassbased_snmptrapservertype.h"
#include "moxaclassbased_snmptype.h"
#include "moxaclassbased_spanningtreeporttype.h"
#include "moxaclassbased_spanningtreetype.h"
#include "moxaclassbased_syncdeviceconfigstatemachinetype.h"
#include "moxaclassbased_syslogservertype.h"
#include "moxaclassbased_timeawareshapertype.h"
#include "moxaclassbased_timesynctype.h"
#include "moxaclassbased_trafficconfigurationtabletype.h"
#include "moxaclassbased_tsninterfaceconfigurationlistenertype.h"
#include "moxaclassbased_tsninterfaceconfigurationtalkertype.h"
#include "moxaclassbased_tsnlistenerstreamtype.h"
#include "moxaclassbased_tsnstreamapplicationtype.h"
#include "moxaclassbased_tsnstreamtype.h"
#include "moxaclassbased_tsntalkerstreamtype.h"
#include "moxaclassbased_tsntrafficspecificationtype.h"
#include "moxaclassbased_unicaststaticforwardtype.h"
#include "moxaclassbased_vlanporttype.h"
#include "moxaclassbased_vlansettingtype.h"
#include "moxaclassbased_vlantype.h"
// opcuaclassbnm data type
#include "opcuaclassbnm_ietfbasenetworkinterfacedatatype.h"
// opcuaclassbnm object type
#include "opcuaclassbnm_bnmrequesttype.h"
#include "opcuaclassbnm_ibaseethernetcapabilitiestype.h"
#include "opcuaclassbnm_ieeebasetsnstatusstreamtype.h"
#include "opcuaclassbnm_ieeebasetsnstreamtype.h"
#include "opcuaclassbnm_ieeebasetsntrafficspecificationtype.h"
#include "opcuaclassbnm_ieeetsninterfaceconfigurationlistenertype.h"
#include "opcuaclassbnm_ieeetsninterfaceconfigurationtalkertype.h"
#include "opcuaclassbnm_ieeetsninterfaceconfigurationtype.h"
#include "opcuaclassbnm_ieeetsnmacaddresstype.h"
#include "opcuaclassbnm_ieeetsnvlantagtype.h"
#include "opcuaclassbnm_ietfbasenetworkinterfacetype.h"
#include "opcuaclassbnm_iieeeautonegotiationstatustype.h"
#include "opcuaclassbnm_iieeebaseethernetporttype.h"
#include "opcuaclassbnm_iieeebasetsnstatusstreamtype.h"
#include "opcuaclassbnm_iieeebasetsnstreamtype.h"
#include "opcuaclassbnm_iieeebasetsntrafficspecificationtype.h"
#include "opcuaclassbnm_iieeetsninterfaceconfigurationlistenertype.h"
#include "opcuaclassbnm_iieeetsninterfaceconfigurationtalkertype.h"
#include "opcuaclassbnm_iieeetsninterfaceconfigurationtype.h"
#include "opcuaclassbnm_iieeetsnmacaddresstype.h"
#include "opcuaclassbnm_iieeetsnvlantagtype.h"
#include "opcuaclassbnm_iietfbasenetworkinterfacetype.h"
#include "opcuaclassbnm_iprioritymappingentrytype.h"
#include "opcuaclassbnm_isrclasstype.h"
#include "opcuaclassbnm_ivlanidtype.h"
// Act module
#include "act_core.hpp"
#include "act_db.hpp"
#include "act_ws_client.hpp"
#include "nodeaccessinfobase.h"
#include "nodemanageruanode.h"

namespace ClassBased {

class WsBridge;
class MoxaNodeManager : public NodeManagerBase {
  UA_DISABLE_COPY(MoxaNodeManager);

 public:
  MoxaNodeManager();
  virtual ~MoxaNodeManager();
  // NodeManagerUaNode implementation
  virtual UaStatus afterStartUp();
  virtual UaStatus beforeShutDown();
  virtual void getNodeReference(UaNodeId startingNode, OpcUa_Boolean isInverse, UaReferenceDescriptions& references);

  virtual MoxaClassBased::SNMPVersion getSnmpVersion(const ActSnmpVersionEnum& actSnmpVersion);
  virtual const ActSnmpVersionEnum setSnmpVersion(const MoxaClassBased::SNMPVersion& tsnSnmpVersion);
  virtual MoxaClassBased::LinkCableType getCableType(const ActCableTypeEnum& actCableType);
  virtual const ActCableTypeEnum setCableType(const MoxaClassBased::LinkCableType linkCableType);

  virtual bool checkMacAddress(const QString macAddress);
  virtual bool checkIpAddress(const QString ipAddress, const uint8_t type);

  qint64 getProjectId(const UaNodeId& nodeId);
  qint64 getDeviceId(const UaNodeId& nodeId);
  qint64 getInterfaceId(const UaNodeId& nodeId);
  qint64 getLinkId(const UaNodeId& nodeId);
  qint64 getTsnStreamApplicationId(const UaNodeId& nodeId);
  qint64 getTsnStreamId(const UaNodeId& nodeId);

  UaNodeId getDeviceProfileNodeId(const qint64& device_profile_id);
  UaNodeId getProjectNodeId(const qint64& project_id);
  UaNodeId getDeviceNodeId(const qint64& project_id, const qint64& device_id);
  UaNodeId getInterfaceNodeId(const qint64& project_id, const qint64& device_id, const qint64& interface_id);
  UaNodeId getLinkNodeId(const qint64& project_id, const qint64& link_id);
  UaNodeId getTsnStreamFolderNodeId(const qint64& project_id);
  UaNodeId getTsnStreamNodeId(const qint64& project_id, const qint64& stream_id);
  UaNodeId getTsnStreamTalkerNodeId(const qint64& project_id, const qint64& stream_id, const QString& talker_interface);
  UaNodeId getTsnStreamListenerNodeId(const qint64& project_id, const qint64& stream_id,
                                      const QString& listener_interface);
  UaNodeId getTsnStreamApplicationFolderNodeId(const qint64& project_id);
  UaNodeId getTsnStreamApplicationNodeId(const qint64& project_id, const qint64& application_id);

  bool isDeviceProfileExist(const UaString& deviceProfileName);
  void setDeviceProfileName(const UaString& deviceProfileName, const UaNodeId& nodeId, const qint64& device_model_id,
                            const ActDeviceTypeEnum& device_type);
  void removeDeviceProfileName(const UaString& deviceProfileName);
  qint64 getDeviceProfileId(const UaString& deviceProfileName);
  ActDeviceTypeEnum getDeviceProfileType(const UaString& deviceProfileName);

  void setWsClient(WsClient* ws_client) { ws_ = ws_client; }
  WsClient* getWsClient() { return ws_; }
  void setWsBridge(WsBridge* bridge) { bridge_ = bridge; }
  WsBridge* getWsBridge() { return bridge_; }
  void sendWsFromUaThread(QString payload);

 private:
  WsClient* ws_ = nullptr;
  WsBridge* bridge_ = nullptr;
  QMap<UaString, QPair<UaNodeId, QPair<qint64, ActDeviceTypeEnum>>> deviceProfileIdMap;
};

class WsBridge : public QObject {
  Q_OBJECT
 public:
  explicit WsBridge(MoxaNodeManager* nm, QObject* parent = nullptr) : QObject(parent), m_nm(nm) {
    QObject::connect(nm->getWsClient(), &WsClient::receivedMessage, this, &WsBridge::onWsReceived,
                     Qt::QueuedConnection);
    QObject::connect(nm->getWsClient(), &WsClient::errorOccurred, this, &WsBridge::onWsError, Qt::QueuedConnection);
  }

 signals:
  void handleWsMessage(QString payload);
  void handleWsError(QString message);

 private slots:
  void onWsReceived(QString payload) { emit handleWsMessage(payload); }
  void onWsError(QString message) { emit handleWsError(message); }

 private:
  MoxaNodeManager* m_nm;  // 指向 UA NodeManager
};

extern MoxaNodeManager* pMoxaNodeManager;
extern MoxaClassBased::NodeManagerMoxaClassBasedNS* pClassBased;
extern OpcUaClassBnm::NodeManagerOpcUaClassBnmNS* pOpcUaClassBnm;

}  // namespace ClassBased
#endif  // __CLASSBASED_NAMEPSACE_H__
