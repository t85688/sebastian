/******************************************************************************
** Copyright (c) 2006-2023 Unified Automation GmbH. All rights reserved.
**
** Software License Agreement ("SLA") Version 2.7
**
** Unless explicitly acquired and licensed from Licensor under another
** license, the contents of this file are subject to the Software License
** Agreement ("SLA") Version 2.7, or subsequent versions
** as allowed by the SLA, and You may not copy or use this file in either
** source code or executable form, except in compliance with the terms and
** conditions of the SLA.
**
** All software distributed under the SLA is provided strictly on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
** AND LICENSOR HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT
** LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
** PURPOSE, QUIET ENJOYMENT, OR NON-INFRINGEMENT. See the SLA for specific
** language governing rights and limitations under the SLA.
**
** The complete license agreement can be found here:
** http://unifiedautomation.com/License/SLA/2.7/
**
** Project: C++ OPC Server SDK sample code
**
******************************************************************************/
#ifndef __OPCUA_CLASS_BASED_SERVERCALLBACK_H__
#define __OPCUA_CLASS_BASED_SERVERCALLBACK_H__

// #include "mydurablesubscriptioncallback.h"
#include "opcserver.h"
#include "serverconfig.h"
// #include "mydurablesubscriptioncallback.h"

#if SUPPORT_PUBSUB
#include "pubsubserverapplicationcallback.h"
#endif  // SUPPORT_PUBSUB

namespace MoxaOpcUaClassBased {

class ServerCallback : public UaServerApplicationCallback {
  UA_DISABLE_COPY(ServerCallback);

 public:
  /** Construction */
  ServerCallback();

  /** Destruction */
  virtual ~ServerCallback();

  void setServerManager(ServerManager* pServerManager);

  // Interface UaServerApplicationCallback
  virtual Session* createSession(OpcUa_Int32 sessionID, const UaNodeId& authenticationToken);
  virtual UaStatus logonSessionUser(Session* pSession, UaUserIdentityToken* pUserIdentityToken,
                                    ServerConfig* pServerConfig);
  virtual void afterLoadConfiguration(ServerConfig* pServerConfig);
  // virtual UaDurableSubscriptionCallback* getDurableSubscriptionCallback();
  virtual void afterCoreModuleStarted();
  virtual void afterNodeManagersStarted();
  // virtual bool nodeManagerStartUpError(NodeManager* pNodeManager);
  // virtual bool serverApplicationModuleStartUpError(UaServerApplicationModule* pModule);
  virtual UaStatus requestServerStateChange(Session* pSession, OpcUa_ServerState state,
                                            const UaDateTime& estimatedReturnTime, OpcUa_UInt32 secondsTillShutdown,
                                            const UaLocalizedText& shutdownReason, OpcUa_Boolean restart);
  // Interface UaServerApplicationCallback

  bool shutDownRequested() const;
  bool restart() const;
  OpcUa_Int32 secondsTillShutdown() const;
  UaLocalizedText shutdownReason() const;
  void resetShutdown();

 private:
  // UaStatus verifyUserWithOS(const UaString& sUserName, const UaString& sPassword);
#if OPCUA_SUPPORT_PKI
  UaStatus validateCertificate(const UaByteString& certificate);
#endif  // OPCUA_SUPPORT_PKI

 private:
  mutable UaMutex m_mutex;
  ServerManager* m_pServerManager;
#if OPCUA_SUPPORT_PKI
  UaString m_sCertificateTrustListLocation;
  UaString m_sCertificateRevocationListLocation;
  UaString m_sIssuersCertificatesLocation;
  UaString m_sIssuersRevocationListLocation;
  UaString m_sRejectedFolder;
  OpcUa_UInt32 m_nRejectedCertificatesCount;
  OpcUa_Boolean m_bCertificateTokenConfigured;
#endif  // OPCUA_SUPPORT_PKI

  bool m_shutDownRequested;
  bool m_restart;
  OpcUa_Int32 m_secondsTillShutdown;
  UaLocalizedText m_shutdownReason;
  OpcUa_ServerState m_targetState;

  // MyDurableSubscriptionCallback m_DurableSubscriptionManager;
};

#if SUPPORT_PUBSUB
class MyDataSetWriterCallback;
class MyDataSetReaderCallback;

class MyPubSubCallback : public PubSubServerApplicationCallback {
  UA_DISABLE_COPY(MyPubSubCallback);

 public:
  /** Construction */
  MyPubSubCallback();

  /** Destruction */
  virtual ~MyPubSubCallback();

  // Interface PubSubServerApplicationCallback
  virtual void startUpPubSub();
  virtual void beforeShutDownPubSub();
  virtual void shutDownPubSub();
  virtual void newConnection(PubSubBase::PubSubConnection* pConnection);
  virtual void newWriterGroup(PubSubBase::WriterGroup* pWriterGroup, bool& groupHandledByApplication);
  virtual void newDataSetWriter(PubSubBase::DataSetWriter* pDataSetWriter, bool& messageEncodedByApplication);
  virtual void newDataSetReader(PubSubBase::DataSetReader* pDataSetReader, bool& messageDecodedByApplication);
  virtual void pubSubObjectStateChange(PubSubBase::PubSubObject* pPubSubObject,
                                       PubSubBase::PubSubObject::PubSubObjectType pubSubObjectType,
                                       OpcUa_PubSubState newState);
  virtual void pubSubObjectRemoved(PubSubBase::PubSubObject* pPubSubObject,
                                   PubSubBase::PubSubObject::PubSubObjectType pubSubObjectType);
  // Interface PubSubServerApplicationCallback

 private:
  std::list<MyDataSetWriterCallback*> m_handledWriters;
  std::list<MyDataSetReaderCallback*> m_handledReaders;
};

#endif  // SUPPORT_PUBSUB

}  // namespace MoxaOpcUaClassBased
#endif  // MYSERVERCALLBACK_H