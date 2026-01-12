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
#include "opcua_class_based_servercallback.h"

#include "act_core.hpp"
#include "nodeaccessinfo.h"
#include "nodemanagerroot.h"
#include "opcua_aliasnamecategorytype.h"
#include "opcua_dictionaryfoldertype.h"
#include "roletypeoperations.h"
#include "serverconfigdata.h"
#include "sessionusercontextbase.h"
#include "uaobjectsserver.h"
#include "uasession.h"

#if OPCUA_SUPPORT_PKI
#include "opcua_core.h"
#include "uadir.h"
#include "uapkicertificate.h"
#endif  // OPCUA_SUPPORT_PKI

#if SUPPORT_PUBSUB
#include <vector>

#include "datasetreadercallback.h"
#include "datasetwritercallback.h"
#include "publisheddataset.h"
#include "pubsubconnection.h"
UA_BEGIN_EXTERN_C
#include <uaencoder/double.h>
#include <uaencoder/encoder.h>
#include <uaencoder/float.h>
#include <uaencoder/int16.h>
#include <uaencoder/int32.h>
#include <uaencoder/int64.h>
#include <uaencoder/int8.h>
#include <uaencoder/uint16.h>
#include <uaencoder/uint32.h>
#include <uaencoder/uint64.h>
#include <uaencoder/uint8.h>
UA_END_EXTERN_C
#endif  // SUPPORT_PUBSUB

namespace MoxaOpcUaClassBased {

ServerCallback::ServerCallback() {
  m_pServerManager = NULL;

#if OPCUA_SUPPORT_PKI
  m_nRejectedCertificatesCount = 100;
  m_bCertificateTokenConfigured = OpcUa_False;
#endif  // OPCUA_SUPPORT_PKI

  m_shutDownRequested = false;
  m_restart = false;
  m_secondsTillShutdown = 0;
  m_targetState = OpcUa_ServerState_Running;
}

ServerCallback::~ServerCallback() {}

void ServerCallback::setServerManager(ServerManager* pServerManager) {
  m_pServerManager = pServerManager;
  if (m_pServerManager) {
    if (m_pServerManager->getNodeManagerRoot()->pServerObject()->serverState() != m_targetState) {
      m_pServerManager->getNodeManagerRoot()->pServerObject()->changeServerState(m_targetState);
    }
    // We create the EstimatedReturnTime variable since we support the RequestServerStateChange method in this sample
    // code
    m_pServerManager->getNodeManagerRoot()->pServerObject()->setEstimatedReturnTime(UaDateTime());
  }
}

Session* ServerCallback::createSession(OpcUa_Int32 sessionID, const UaNodeId& authenticationToken) {
  // Create the application specific session where we can store our own information during user authentication
  // (logonSessionUser) We will used this information later for user authorization like in browse, read and write
  UaSession* pSession = new UaSession(sessionID, authenticationToken);
  return pSession;
}

UaStatus ServerCallback::logonSessionUser(Session* pSession, UaUserIdentityToken* pUserIdentityToken,
                                          ServerConfig* pServerConfig) {
  if (pServerConfig == NULL) {
    return OpcUa_Bad;
  }
  OpcUa_Boolean bEnableAnonymous;
  OpcUa_Boolean bEnableUserPw;
  OpcUa_Boolean bEnableCertificate;
  OpcUa_Boolean bEnableKerberosTicket;

  pServerConfig->getUserIdentityTokenConfig(bEnableAnonymous, bEnableUserPw, bEnableCertificate, bEnableKerberosTicket);

  if (pUserIdentityToken->getTokenType() == OpcUa_UserTokenType_Anonymous) {
    if (bEnableAnonymous == OpcUa_False) {
      // Return error if Anonymous is not enabled
      return OpcUa_BadIdentityTokenRejected;
    } else {
      SessionUserContext* pUserContext = new SessionUserContextBase();
      pUserContext->setIdentity(pUserIdentityToken);
      pSession->setUserContext(pUserContext);
      UaStatus ret = NodeManagerRoot::CreateRootNodeManager()->pServerManager()->setRoleIds(pSession);
      UA_ASSERT(ret.isGood());
      pUserContext->releaseReference();

      return OpcUa_Good;
    }
  }
  //! [Username/Password Authentication]
  else if (pUserIdentityToken->getTokenType() == OpcUa_UserTokenType_UserName) {
    if (bEnableUserPw == OpcUa_False) {
      // Return error if User/Password is not enabled
      return OpcUa_BadIdentityTokenRejected;
    } else {
      // Check user and password and set user related information on MySession
      UaUserIdentityTokenUserPassword* pUserPwToken = (UaUserIdentityTokenUserPassword*)pUserIdentityToken;

      for (ActUser actUser : act::core::g_core.GetUserSet()) {
        // actUser.DecryptPassword();
        if (actUser.GetUsername() == QString(pUserPwToken->sUserName.toUtf8()) &&
            actUser.GetPassword() == QString(pUserPwToken->sPassword.toUtf8())) {
          SessionUserContextBase* pUserContext = new SessionUserContextBase();
          pUserContext->setIdentity(pUserIdentityToken);
          pSession->setUserContext(pUserContext);
          // Set the roles for the session based on the user
          // This is done based on the user to role assignment configured in afterNodeManagersStarted()
          UaStatus ret = NodeManagerRoot::CreateRootNodeManager()->pServerManager()->setRoleIds(pSession);
          UA_ASSERT(ret.isGood());
          // For user root we override the mapping rules and grant full access to everything
          // if (pUserPwToken->sUserName == "root") {
          //   pUserContext->setIsRoot(true);
          // }
          pUserContext->releaseReference();
          return OpcUa_Good;
        }
      }
      return OpcUa_BadUserAccessDenied;
      /*
      // ++ Simplified sample code +++++++++++++++++++++++++++++++++++++++
      // Implement user authentication here
      // This is just a trivial example with 5 different users
      if ((pUserPwToken->sUserName == "root" && pUserPwToken->sPassword == "secret") ||
          (pUserPwToken->sUserName == "joe" && pUserPwToken->sPassword == "god") ||
          (pUserPwToken->sUserName == "john" && pUserPwToken->sPassword == "master") ||
          (pUserPwToken->sUserName == "sue" && pUserPwToken->sPassword == "curly") ||
          (pUserPwToken->sUserName == "sam" && pUserPwToken->sPassword == "serious")) {
        // We know that this is a known user with a valid password
        SessionUserContextBase* pUserContext = new SessionUserContextBase();
        pUserContext->setIdentity(pUserIdentityToken);
        pSession->setUserContext(pUserContext);
        // Set the roles for the session based on the user
        // This is done based on the user to role assignment configured in afterNodeManagersStarted()
        UaStatus ret = NodeManagerRoot::CreateRootNodeManager()->pServerManager()->setRoleIds(pSession);
        UA_ASSERT(ret.isGood());
        // For user root we override the mapping rules and grant full access to everything
        if (pUserPwToken->sUserName == "root") {
          pUserContext->setIsRoot(true);
        }
        pUserContext->releaseReference();
        return OpcUa_Good;
        // } else if (verifyUserWithOS(pUserPwToken->sUserName, pUserPwToken->sPassword).isGood()) {
        //   // This is a user known by the operating system
        //   SessionUserContext* pUserContext = new SessionUserContextBase();
        //   pUserContext->setIdentity(pUserIdentityToken);
        //   pSession->setUserContext(pUserContext);
        //   UaStatus ret = NodeManagerRoot::CreateRootNodeManager()->pServerManager()->setRoleIds(pSession);
        //   UA_ASSERT(ret.isGood());
        //   pUserContext->releaseReference();
        //   return OpcUa_Good;
      } else {
        return OpcUa_BadUserAccessDenied;
      }
      // ++ Simplified sample code +++++++++++++++++++++++++++++++++++++++
      */
    }
  }
  //! [Username/Password Authentication]
  //! [User Certificate Authentication]
  else if (pUserIdentityToken->getTokenType() == OpcUa_UserTokenType_Certificate) {
#if OPCUA_SUPPORT_PKI
    if (bEnableCertificate == OpcUa_False || m_bCertificateTokenConfigured == OpcUa_False) {
      // Return error if CertificateToken is not enabled
      return OpcUa_BadIdentityTokenRejected;
    } else {
      // Just check if the certificate it trusted - we don't map this to a known user
      UaUserIdentityTokenCertificate* pUserCertToken = (UaUserIdentityTokenCertificate*)pUserIdentityToken;

      // create certificate object to get name and thumbprint
      UaPkiCertificate pkiCertificate = UaPkiCertificate::fromDER(pUserCertToken->userCertificateData);

      // validate the user certificate against the user certificate store
      UaStatus ret = validateCertificate(pUserCertToken->userCertificateData);

      // the certificate is trusted so we accept the user
      if (ret.isGood()) {
        SessionUserContextBase* pUserContext = new SessionUserContextBase();
        pUserContext->setIdentity(pUserIdentityToken);
        pSession->setUserContext(pUserContext);
        // Set the roles for the session based on the thumbprint
        ret = NodeManagerRoot::CreateRootNodeManager()->pServerManager()->setRoleIds(pSession);
        UA_ASSERT(ret.isGood());
        pUserContext->releaseReference();
      }
      // Copy the certificate into the rejected folder
      else {
        // Get count of files in the directory
        UaDir dirHelper("");
        OpcUa_UInt16 rejectedCertCount = dirHelper.recursiveFileCount(m_sRejectedFolder.toUtf16());

        // Check number of rejected certificates to respect the m_nRejectedCertificatesCount
        if (rejectedCertCount <= m_nRejectedCertificatesCount) {
          UaString fileName = UaString("%1/%2.der").arg(m_sRejectedFolder).arg(pkiCertificate.thumbPrint().toHex());

          // save certificate
          pkiCertificate.toDERFile(UaByteString(UaDir::fromNativeSeparators(fileName.toUtf8()).toLocal8Bit()));
        }
        ret = OpcUa_BadIdentityTokenRejected;
      }
      return ret;
    }
#else   // OPCUA_SUPPORT_PKI
        //  Return error if PKI is not enabled
    return OpcUa_BadIdentityTokenRejected;
#endif  // OPCUA_SUPPORT_PKI
  }
  //! [User Certificate Authentication]
  return OpcUa_BadIdentityTokenInvalid;
}

void ServerCallback::afterLoadConfiguration(ServerConfig* pServerConfig) {
  ServerConfigData* pServerConfigData = (ServerConfigData*)pServerConfig;

  // Make sure the demo server product information can not be overwritten by the config file
  pServerConfigData->setProductUri("urn:UnifiedAutomation:UaServerCpp");
  pServerConfigData->setManufacturerName("Unified Automation GmbH");
  pServerConfigData->setProductName("C++ SDK OPC UA Demo Server");
  pServerConfigData->setSoftwareVersion(SERVERCONFIG_SOFTWAREVERSION);
  pServerConfigData->setBuildNumber(SERVERCONFIG_BUILDNUMBER);

#if OPCUA_SUPPORT_PKI
  CertificateStoreConfiguration* pCertificateStore = NULL;
  UaString sRejectedCertificateDirectory;
  OpcUa_UInt32 nRejectedCertificatesCount;
  OpcUa_Boolean bCertificateTokenConfigured = OpcUa_False;

  pServerConfigData->getDefaultUserCertificateStore(&pCertificateStore, sRejectedCertificateDirectory,
                                                    nRejectedCertificatesCount, bCertificateTokenConfigured);

  m_sCertificateTrustListLocation = pCertificateStore->m_sCertificateTrustListLocation;
  m_sCertificateRevocationListLocation = pCertificateStore->m_sCertificateRevocationListLocation;
  m_sIssuersCertificatesLocation = pCertificateStore->m_sIssuersCertificatesLocation;
  m_sIssuersRevocationListLocation = pCertificateStore->m_sIssuersRevocationListLocation;
  m_sRejectedFolder = sRejectedCertificateDirectory;
  m_nRejectedCertificatesCount = nRejectedCertificatesCount;
  m_bCertificateTokenConfigured = bCertificateTokenConfigured;

  // make sure the paths all exist
  if (bCertificateTokenConfigured) {
    UaDir dirHelper("");
    UaUniString usPath;
    usPath = UaUniString(UaDir::fromNativeSeparators(m_sCertificateTrustListLocation.toUtf16()));
    dirHelper.mkpath(usPath);
    usPath = UaUniString(UaDir::fromNativeSeparators(m_sCertificateRevocationListLocation.toUtf16()));
    dirHelper.mkpath(usPath);
    usPath = UaUniString(UaDir::fromNativeSeparators(m_sIssuersCertificatesLocation.toUtf16()));
    dirHelper.mkpath(usPath);
    usPath = UaUniString(UaDir::fromNativeSeparators(m_sIssuersRevocationListLocation.toUtf16()));
    dirHelper.mkpath(usPath);
    usPath = UaUniString(UaDir::fromNativeSeparators(m_sRejectedFolder.toUtf16()));
    dirHelper.mkpath(usPath);
  }
#endif  // OPCUA_SUPPORT_PKI
}

// UaDurableSubscriptionCallback* ServerCallback::getDurableSubscriptionCallback() {
//   return &m_DurableSubscriptionManager;
// }

void ServerCallback::afterCoreModuleStarted() {
  // Create OPC UA namespace nodes here if not created by SDK
  // This method is called after the SDK managed NodeManager are started but before
  // the application specific NodeManagers are started

  // Get the singleton NodeManagerRoot
  /*
  NodeManagerRoot* pNMRoot = NodeManagerRoot::CreateRootNodeManager();
  */

  // Create a node that is not created by the SDK
  /*
  OpcUa::DictionaryFolderType* pDictrionaries = new OpcUa::DictionaryFolderType(
      OpcUaId_Dictionaries,
      OpcUa_BrowseName_Dictionaries,
      0,
      pNMRoot);
  pNMRoot->addNodeAndReference(OpcUaId_Server, pDictrionaries, OpcUaId_HasComponent);
  */

  // Type nodes can be created with the corresponding implementation classes
  // This is normally done when the first instance is created
  /*
  OpcUa::AliasNameCategoryType::createTypes();
  */
}

void ServerCallback::afterNodeManagersStarted() {
  // add identity mapping rules
  // e.g. john has Roles: DemoRole1, Operator
  ServerManager* pServerManager = NodeManagerRoot::CreateRootNodeManager()->pServerManager();

  // Add rules for well known roles
  RoleTypeOperations* pRoleTypeOperations;

  for (ActUser actUser : act::core::g_core.GetUserSet()) {
    if (actUser.GetRole() == ActRoleEnum::kAdmin) {
      UaStatus ret = pServerManager->getRoleByNodeId(OpcUaId_WellKnownRole_ConfigureAdmin, &pRoleTypeOperations);
      if (ret.isGood()) {
        UaIdentityMappingRuleType ruleToAdd;
        ruleToAdd.setCriteriaType(OpcUa_IdentityCriteriaType_UserName);
        ruleToAdd.setCriteria(UaString(actUser.GetUsername().toStdString().c_str()));
        pRoleTypeOperations->addIdentity(ruleToAdd);
      } else {
        UA_ASSERT(false);
      }
    } else if (actUser.GetRole() == ActRoleEnum::kSupervisor) {
      UaStatus ret = pServerManager->getRoleByNodeId(OpcUaId_WellKnownRole_Supervisor, &pRoleTypeOperations);
      if (ret.isGood()) {
        UaIdentityMappingRuleType ruleToAdd;
        ruleToAdd.setCriteriaType(OpcUa_IdentityCriteriaType_UserName);
        ruleToAdd.setCriteria(UaString(actUser.GetUsername().toStdString().c_str()));
        pRoleTypeOperations->addIdentity(ruleToAdd);
      } else {
        UA_ASSERT(false);
      }
    } else if (actUser.GetRole() == ActRoleEnum::kUser) {
      UaStatus ret = pServerManager->getRoleByNodeId(OpcUaId_WellKnownRole_Observer, &pRoleTypeOperations);
      if (ret.isGood()) {
        UaIdentityMappingRuleType ruleToAdd;
        ruleToAdd.setCriteriaType(OpcUa_IdentityCriteriaType_UserName);
        ruleToAdd.setCriteria(UaString(actUser.GetUsername().toStdString().c_str()));
        pRoleTypeOperations->addIdentity(ruleToAdd);
      } else {
        UA_ASSERT(false);
      }
    }
  }
  /*
  // Observer
  UaStatus ret = pServerManager->getRoleByNodeId(OpcUaId_WellKnownRole_Observer, &pRoleTypeOperations);
  if (ret.isGood()) {
    // add john and joe
    UaIdentityMappingRuleType ruleToAdd;
    ruleToAdd.setCriteriaType(OpcUa_IdentityCriteriaType_UserName);
    ruleToAdd.setCriteria("john");
    pRoleTypeOperations->addIdentity(ruleToAdd);
    ruleToAdd.setCriteria("joe");
    pRoleTypeOperations->addIdentity(ruleToAdd);
  } else {
    UA_ASSERT(false);
  }

  // Operator
  ret = pServerManager->getRoleByNodeId(OpcUaId_WellKnownRole_Operator, &pRoleTypeOperations);
  if (ret.isGood()) {
    // add john and sue
    UaIdentityMappingRuleType ruleToAdd;
    ruleToAdd.setCriteriaType(OpcUa_IdentityCriteriaType_UserName);
    ruleToAdd.setCriteria("john");
    pRoleTypeOperations->addIdentity(ruleToAdd);
    ruleToAdd.setCriteriaType(OpcUa_IdentityCriteriaType_UserName);
    ruleToAdd.setCriteria("sue");
    pRoleTypeOperations->addIdentity(ruleToAdd);
  } else {
    UA_ASSERT(false);
  }

  // ConfigureAdmin
  ret = pServerManager->getRoleByNodeId(OpcUaId_WellKnownRole_ConfigureAdmin, &pRoleTypeOperations);
  if (ret.isGood()) {
    // add root john and sam
    UaIdentityMappingRuleType ruleToAdd;
    ruleToAdd.setCriteriaType(OpcUa_IdentityCriteriaType_UserName);
    ruleToAdd.setCriteria("john");
    pRoleTypeOperations->addIdentity(ruleToAdd);
    ruleToAdd.setCriteria("sam");
    pRoleTypeOperations->addIdentity(ruleToAdd);
    // adding root here isn't really necessary because we set the root flag in logonSessionUser
    // we still add a mapping rule to make it visible in the address space
    ruleToAdd.setCriteria("root");
    pRoleTypeOperations->addIdentity(ruleToAdd);
  } else {
    UA_ASSERT(false);
  }

  // SecurityAdmin
  ret = pServerManager->getRoleByNodeId(OpcUaId_WellKnownRole_SecurityAdmin, &pRoleTypeOperations);
  if (ret.isGood()) {
    // add root and sam
    UaIdentityMappingRuleType ruleToAdd;
    ruleToAdd.setCriteriaType(OpcUa_IdentityCriteriaType_UserName);
    ruleToAdd.setCriteria("sam");
    pRoleTypeOperations->addIdentity(ruleToAdd);
    // adding root here isn't really necessary because we set the root flag in logonSessionUser
    // we still add a mapping rule to make it visible in the address space
    ruleToAdd.setCriteria("root");
    pRoleTypeOperations->addIdentity(ruleToAdd);
  } else {
    UA_ASSERT(false);
  }

  // DemoRole1
  ret = pServerManager->getRoleByName("MyDemoRole1", "", &pRoleTypeOperations);
  if (ret.isGood()) {
    // add joe and sue
    UaIdentityMappingRuleType ruleToAdd;
    ruleToAdd.setCriteriaType(OpcUa_IdentityCriteriaType_UserName);
    ruleToAdd.setCriteria("joe");
    pRoleTypeOperations->addIdentity(ruleToAdd);
    ruleToAdd.setCriteria("sue");
    pRoleTypeOperations->addIdentity(ruleToAdd);
  } else {
    UA_ASSERT(false);
  }

  // DemoRole2
  ret = pServerManager->getRoleByName("MyDemoRole2", "http://www.unifiedautomation.com/DemoServer/",
                                      &pRoleTypeOperations);
  if (ret.isGood()) {
    // add Sue
    UaIdentityMappingRuleType ruleToAdd;
    ruleToAdd.setCriteriaType(OpcUa_IdentityCriteriaType_UserName);
    ruleToAdd.setCriteria("sue");
    pRoleTypeOperations->addIdentity(ruleToAdd);
  } else {
    UA_ASSERT(false);
  }
  */
}
/*
bool ServerCallback::nodeManagerStartUpError(NodeManager* pNodeManager) {
  // override default to stop server startup in case of an error
  OpcUa_ReferenceParameter(pNodeManager);
  printf("Nodemanager failed to start. Stopping server startUp\n");
  return false;
}

bool ServerCallback::serverApplicationModuleStartUpError(UaServerApplicationModule* pModule) {
  // override default to stop server startup in case of an error
  OpcUa_ReferenceParameter(pModule);
  printf("UaServerApplicationModule failed to start. Stopping server startUp\n");
  return false;
}
*/

UaStatus ServerCallback::requestServerStateChange(Session* pSession, OpcUa_ServerState state,
                                                  const UaDateTime& estimatedReturnTime,
                                                  OpcUa_UInt32 secondsTillShutdown,
                                                  const UaLocalizedText& shutdownReason, OpcUa_Boolean restart) {
  OpcUa_ReferenceParameter(pSession);

  UaMutexLocker lock(&m_mutex);

  if (restart == OpcUa_False) {
    m_restart = false;
    if (state == OpcUa_ServerState_Shutdown) {
      m_shutDownRequested = true;
      m_secondsTillShutdown = secondsTillShutdown;
      m_shutdownReason = shutdownReason;
    }
    if (m_pServerManager) {
      m_targetState = state;
      m_pServerManager->getNodeManagerRoot()->pServerObject()->changeServerState(m_targetState);
      m_pServerManager->getNodeManagerRoot()->pServerObject()->setEstimatedReturnTime(estimatedReturnTime);
    } else {
      return OpcUa_BadInvalidState;
    }
  } else {
    if (state == OpcUa_ServerState_Shutdown) {
      // Target state Shutdown and Restart makes no sense
      return OpcUa_BadInvalidState;
    }
    m_shutDownRequested = true;
    m_secondsTillShutdown = secondsTillShutdown;
    m_shutdownReason = shutdownReason;
    m_targetState = state;
    m_restart = true;
    if (m_pServerManager) {
      m_pServerManager->getNodeManagerRoot()->pServerObject()->setEstimatedReturnTime(estimatedReturnTime);
    }
  }

  return OpcUa_Good;
}

bool ServerCallback::shutDownRequested() const {
  UaMutexLocker lock(&m_mutex);
  bool ret = m_shutDownRequested;
  return ret;
}
bool ServerCallback::restart() const {
  UaMutexLocker lock(&m_mutex);
  bool ret = m_restart;
  return ret;
}
OpcUa_Int32 ServerCallback::secondsTillShutdown() const {
  UaMutexLocker lock(&m_mutex);
  OpcUa_Int32 ret = m_secondsTillShutdown;
  return ret;
}
UaLocalizedText ServerCallback::shutdownReason() const {
  UaMutexLocker lock(&m_mutex);
  UaLocalizedText ret = m_shutdownReason;
  return ret;
}

void ServerCallback::resetShutdown() {
  UaMutexLocker lock(&m_mutex);
  m_shutDownRequested = false;
  m_restart = false;
  m_secondsTillShutdown = 0;
  m_shutdownReason.clear();
}

// UaStatus ServerCallback::verifyUserWithOS(const UaString& sUserName, const UaString& sPassword) {
//   OpcUa_ReferenceParameter(sUserName);
//   OpcUa_ReferenceParameter(sPassword);
//   UaStatus ret = OpcUa_BadUserAccessDenied;

//   // Only Windows sample code available at the moment
// #if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(WIN_IOT)

//   HANDLE userContextHandle;
//   UaString tempUserName;
//   UaString uaDomain;
//   int i;
//   int iBSPos = sUserName.find("\\");
//   int iAtPos = sUserName.find("@");
//   if (iBSPos > 0 && iAtPos == -1) {
//     for (i = 0; i < iBSPos; i++) {
//       uaDomain += sUserName.at(i).data();
//     }
//     for (i = iBSPos + 1; i < sUserName.length(); i++) {
//       tempUserName += sUserName.at(i).data();
//     }
//   } else if (iAtPos > 0 && iBSPos == -1) {
//     for (i = 0; i < iAtPos; i++) {
//       tempUserName += sUserName.at(i).data();
//     }
//     for (i = iAtPos + 1; i < sUserName.length(); i++) {
//       uaDomain += sUserName.at(i).data();
//     }
//   } else {
//     tempUserName = sUserName;
//   }

// #ifdef UNICODE
//   UaByteArray wsUserName = tempUserName.toUtf16();
//   LPTSTR lpszUsername = (wchar_t*)(const UaUShort*)wsUserName;
//   UaByteArray wsDomain = uaDomain.toUtf16();
//   LPTSTR lpszDomain = (wsDomain.size() > 0) ? ((wchar_t*)(const UaUShort*)wsDomain) : NULL;
//   UaByteArray wsPassword = sPassword.toUtf16();
//   LPTSTR lpszPassword = (wchar_t*)(const UaUShort*)wsPassword;

// #else  /* UNICODE */
//   char* lpszUsername = (char*)uaUserName.toUtf8();
//   char* lpszDomain = (uaDomain.length() > 0) ? ((char*)uaDomain.toUtf8()) : NULL;
//   char* lpszPassword = (char*)pUserPwToken->sPassword.toUtf8();
// #endif /* UNICODE */

//   // http://msdn2.microsoft.com/en-us/library/Aa378184.aspx
//   // requires include Windows.h
//   // requires lib Advapi32.lib
//   BOOL logOnResult = LogonUser(lpszUsername, lpszDomain, lpszPassword,
//                                LOGON32_LOGON_INTERACTIVE,  // DWORD dwLogonType,
//                                LOGON32_PROVIDER_DEFAULT,   // DWORD dwLogonProvider,
//                                &userContextHandle);

//   if (logOnResult == FALSE) {
//     ret = OpcUa_BadUserAccessDenied;
//   } else {
//     CloseHandle(userContextHandle);
//     ret = OpcUa_Good;
//   }
// #endif /* defined (_WIN32) && !defined(_WIN32_WCE) && !defined(WIN_IOT) */

//   return ret;
// }

#if OPCUA_SUPPORT_PKI
UaStatus ServerCallback::validateCertificate(const UaByteString& certificate) {
  UaStatus ret;

  // create configuration for PkiProvider
  OpcUa_CertificateStoreConfiguration pkiConfig;
  OpcUa_CertificateStoreConfiguration_Initialize(&pkiConfig);
  OpcUa_UInt32 length;
  UaByteArray baTmp;

  // PkiType
  length = OpcUa_StrLenA(OPCUA_P_PKI_TYPE_OPENSSL);
  if (length > 0) {
    pkiConfig.strPkiType = (OpcUa_StringA)OpcUa_Alloc(length + 1);
    OpcUa_StrnCpyA(pkiConfig.strPkiType, OPCUA_P_PKI_TYPE_OPENSSL, length + 1);
  }

  // CertificateRevocationListLocation
  if (m_sCertificateRevocationListLocation.length() > 0) {
    baTmp = m_sCertificateRevocationListLocation.toLocal8Bit();
    pkiConfig.strRevokedCertificateListLocation = (OpcUa_StringA)OpcUa_Alloc(baTmp.size());
    OpcUa_MemCpy(pkiConfig.strRevokedCertificateListLocation, baTmp.size(), baTmp.data(), baTmp.size());
  }

  // TrustedCertificateListLocation
  if (m_sCertificateTrustListLocation.length() > 0) {
    baTmp = m_sCertificateTrustListLocation.toLocal8Bit();
    pkiConfig.strTrustedCertificateListLocation = (OpcUa_StringA)OpcUa_Alloc(baTmp.size());
    OpcUa_MemCpy(pkiConfig.strTrustedCertificateListLocation, baTmp.size(), baTmp.data(), baTmp.size());
  }

  // RevokedIssuerCertificateListLocation and IssuersCertificatesLocation
  if (m_sIssuersRevocationListLocation.length() > 0 && m_sIssuersCertificatesLocation.length() > 0) {
    baTmp = m_sIssuersRevocationListLocation.toLocal8Bit();
    pkiConfig.strRevokedIssuerCertificateListLocation = (OpcUa_StringA)OpcUa_Alloc(baTmp.size());
    OpcUa_MemCpy(pkiConfig.strRevokedIssuerCertificateListLocation, baTmp.size(), baTmp.data(), baTmp.size());

    baTmp = m_sIssuersCertificatesLocation.toLocal8Bit();
    pkiConfig.strIssuerCertificateStoreLocation = (OpcUa_StringA)OpcUa_Alloc(baTmp.size());
    OpcUa_MemCpy(pkiConfig.strIssuerCertificateStoreLocation, baTmp.size(), baTmp.data(), baTmp.size());
  }

  pkiConfig.uFlags = OPCUA_P_PKI_OPENSSL_CHECK_REVOCATION_STATUS;
  pkiConfig.pvOverride = NULL;
  pkiConfig.pfVerifyCallback = NULL;
  pkiConfig.pvVerifyCallbackUserData = NULL;

  // create Pki Provider with the configuration setup above
  OpcUa_PKIProvider pkiProvider;
  OpcUa_PKIProvider_Create(&pkiConfig, &pkiProvider);

  // open the certificate store
  OpcUa_Handle hCertificateStore;
  ret = pkiProvider.OpenCertificateStore(&pkiProvider, &hCertificateStore);

  // validate the certificate
  if (ret.isGood()) {
    OpcUa_Int validationCode;
    ret = pkiProvider.ValidateCertificate(&pkiProvider, (const OpcUa_ByteString*)certificate, hCertificateStore,
                                          &validationCode);

    // close the certificate store
    pkiProvider.CloseCertificateStore(&pkiProvider, &hCertificateStore);
  }

  OpcUa_CertificateStoreConfiguration_Clear(&pkiConfig);
  return ret;
}
#endif  // OPCUA_SUPPORT_PKI

#if SUPPORT_PUBSUB
class MyDataSetWriterCallback : public PubSubBase::DataSetWriterCallback {
  UA_DISABLE_COPY(MyDataSetWriterCallback);

 public:
  MyDataSetWriterCallback() : m_simulationValue(0), m_simulationValueSigned(0) {}
  virtual ~MyDataSetWriterCallback() {}

  virtual OpcUa_StatusCode startUpWriter(PubSubBase::DataSetWriter*);
  virtual OpcUa_StatusCode shutDownWriter(PubSubBase::DataSetWriter*) { return OpcUa_Good; }
  virtual OpcUa_StatusCode writeDataSetMessageFields(PubSubBase::DataSetWriter* pDataSetWriter,
                                                     struct ua_encoder_context* pEncoder,
                                                     struct pubsub_datasetmsg_ctx* pDataSetMessageContext);

 private:
  uint8_t m_simulationValue;
  int8_t m_simulationValueSigned;
  std::vector<OpcUa_BuiltInType> m_fieldDataTypes;
};

class MyDataSetReaderCallback : public PubSubBase::DataSetReaderCallback {
  UA_DISABLE_COPY(MyDataSetReaderCallback);

 public:
  MyDataSetReaderCallback();
  virtual ~MyDataSetReaderCallback();

  virtual OpcUa_StatusCode startUpReader(PubSubBase::DataSetReader*);
  virtual OpcUa_StatusCode shutDownReader(PubSubBase::DataSetReader*);
  virtual OpcUa_StatusCode newMessageReceived(PubSubBase::DataSetReader* pDataSetReader,
                                              struct ua_decoder_context* pDecoder,
                                              struct pubsub_datasetmsg_ctx* pDataSetMessageContext);

 private:
  UaString m_sReaderName;
  std::vector<OpcUa_BuiltInType> m_fieldDataTypes;
};

MyPubSubCallback::MyPubSubCallback() {}

MyPubSubCallback::~MyPubSubCallback() {
  // Clean up resources allocated for reader and writer management objects
  std::list<MyDataSetWriterCallback*>::iterator itWriters;
  for (itWriters = m_handledWriters.begin(); itWriters != m_handledWriters.end(); itWriters++) {
    delete (*itWriters);
    (*itWriters) = NULL;
  }
  m_handledWriters.clear();

  std::list<MyDataSetReaderCallback*>::iterator itReaders;
  for (itReaders = m_handledReaders.begin(); itReaders != m_handledReaders.end(); itReaders++) {
    delete (*itReaders);
    (*itReaders) = NULL;
  }
  m_handledReaders.clear();
}

/** Callback informing the application about the start up of the PubSub module in the server SDK.
 */
void MyPubSubCallback::startUpPubSub() {}

/** Callback informing the application about the start of the shut down of the PubSub module in the server SDK.
 */
void MyPubSubCallback::beforeShutDownPubSub() {}

/** Callback informing the application about the shut down of the PubSub module in the server SDK.
 *  The application is not allowed to call any methods on the PubSubModule if this method call is returned.
 */
void MyPubSubCallback::shutDownPubSub() {}

/** Callback to inform application about creation of new PubSubConnection
 *
 *  The application can provide an own network backend by setting the PubSubNetworkBackendInterface
 *  on the PubSubConnection object.
 */
void MyPubSubCallback::newConnection(
    PubSubBase::PubSubConnection* pConnection  //!< [in] The connection management object.
) {
  OpcUa_ReferenceParameter(pConnection);

  // Here the application can set a custom network backend for the connection
  // by calling pConnection->setPubSubNetworkBackendInterface();

  // Set own network backend on connection
  // if (pConnection->transportFacet() == PubSubBase::PubSubObject::TransportFacet_PubSub_ETH_UADP)
  // {
  //    // We use our sample user network backend that creates a loop back
  //    pConnection->setPubSubNetworkBackendInterface(pNetworkBackendUser);
  // }
}

/** Callback to inform application about creation of new writer group
 *  The application can handle the processing of the group by setting the
 *  groupHandledByApplication to true.
 */
void MyPubSubCallback::newWriterGroup(
    PubSubBase::WriterGroup* pWriterGroup,  //!< [in] The writer group management object.
    bool& groupHandledByApplication /**< [out] Flag used to indicate that the application is responsible for group
                                    processing (true) or handling should be done by SDK (false).
                                    The PubSubWriterGroup provides methods for sampling (creating the NetworkMessage)
                                    and publishing that can be called by the application for the timing.*/
) {
  OpcUa_ReferenceParameter(pWriterGroup);

  // Here the application can take responsibility for timing for
  // (1) sampling at WriterGroup::MessageSettings::SamplingOffset to create a NetworkMessage
  //     by calling pWriterGroup->sample();
  // (2) publishing at WriterGroup::MessageSettings::PublishingOffset to send the NetworkMessage
  //     by calling pWriterGroup->publish();

  groupHandledByApplication = false;
}

/** Callback to inform application about creation of new DataSetWriter
 *
 *  The application can handle the processing of the DataSetMessage encoding by setting the messageHandledByApplication
 * to true. In this case the application must set the DataSetWriterCallback to receive the message encoding callbacks.
 */
void MyPubSubCallback::newDataSetWriter(
    PubSubBase::DataSetWriter* pDataSetWriter,  //!< [in] The DataSetWriter management object.
    bool& messageEncodedByApplication /**< [out] Flag used to indicate that the application is responsible for encoding
                                      the the fields of the DataSetMessage (true) or encoding should be done by SDK
                                      (false). In the case of true, the encoding is started with
                                      writeDataSetMessageFields. */
) {
  // Do custom handling in sample code if PublishingInterval of parent WriterGroup is set to 99
  // and if the DataSetMessage contains fields with raw encoding and only fields with number data types
  if (pDataSetWriter->pParent()->publishingInterval() == 99) {
    bool isValidConfiguration = false;
    if (pDataSetWriter->dataSetFieldContentMask() & OpcUa_DataSetFieldContentMask_RawData) {
      isValidConfiguration = true;
      OpcUa_Int32 i = 0;
      UaPublishedVariableDataTypes publishedVariables;
      pDataSetWriter->pPublishedDataSet()->publishedVariables(publishedVariables);
      if (publishedVariables.length() ==
          (OpcUa_UInt32)pDataSetWriter->pPublishedDataSet()->pDataSetMetaData()->NoOfFields) {
        for (i = 0; i < pDataSetWriter->pPublishedDataSet()->pDataSetMetaData()->NoOfFields; i++) {
          if (pDataSetWriter->pPublishedDataSet()->pDataSetMetaData()->Fields[i].BuiltInType < OpcUaType_SByte ||
              pDataSetWriter->pPublishedDataSet()->pDataSetMetaData()->Fields[i].BuiltInType > OpcUaType_Double) {
            // The sample code is only able to handle number data types
            isValidConfiguration = false;
            break;
          } else {
            UaNodeId fieldVariableNodeId(publishedVariables[i].PublishedVariable);
            // The simple sample code sends simulated data independent of the selected source variables
            // A real implementation must verify the NodeId and use the NodeId to know what
            // data is to be published. An optimized information for fast access to the data source
            // is then stored for each field for efficient cyclic processing in writeDataSetMessageFields()
            //
            // This information is then typically stored in the management object for the DataSetWriter
            // This would be the MyDataSetWriterCallback class in this example
          }
        }
      } else {
        isValidConfiguration = false;
      }
    }

    if (isValidConfiguration) {
      messageEncodedByApplication = true;

      // Create and store writer handler
      MyDataSetWriterCallback* pDataSetWriterHandler = new MyDataSetWriterCallback;
      m_handledWriters.push_back(pDataSetWriterHandler);

      // Set callback on DataSetWriter
      pDataSetWriter->setDataSetWriterCallback(pDataSetWriterHandler);
    } else {
      // The message must be handled by the SDK
      messageEncodedByApplication = false;
    }
  } else {
    messageEncodedByApplication = false;
  }
}

/** Callback to inform application about creation of new DataSetReader
 *
 *  The application can handle the processing of the DataSetMessage decoding by setting the messageHandledByApplication
 * to true. In this case the application must set the DataSetReaderCallback to receive the message encoding callbacks.
 */
void MyPubSubCallback::newDataSetReader(
    PubSubBase::DataSetReader* pDataSetReader,  //!< [in] The DataSetReader management object.
    bool& messageDecodedByApplication /**< [out] Flag used to indicate that the application is responsible for decoding
                                      the the fields of the DataSetMessage (true) or decoding should be done by SDK
                                      (false). In the case of true, the decoding is started with newMessageReceived. */
) {
  // Do custom handling in sample code if PublishingInterval is set to 999
  // and if the DataSetMessage contains fields with raw encoding and only fields number data types
  UaUadpDataSetReaderMessageDataType uadpMessageSettings;
  if (pDataSetReader->messageSettingsUadp(uadpMessageSettings).isGood() &&
      uadpMessageSettings.getPublishingInterval() == 999) {
    bool isValidConfiguration = false;
    if (pDataSetReader->dataSetFieldContentMask() & OpcUa_DataSetFieldContentMask_RawData) {
      isValidConfiguration = true;
      OpcUa_Int32 i = 0;
      for (i = 0; i < pDataSetReader->pDataSetMetaData()->NoOfFields; i++) {
        if (pDataSetReader->pDataSetMetaData()->Fields[i].BuiltInType < OpcUaType_SByte ||
            pDataSetReader->pDataSetMetaData()->Fields[i].BuiltInType > OpcUaType_Double) {
          // The sample code is only able to handle number data types
          isValidConfiguration = false;
        }
      }
    }

    if (isValidConfiguration) {
      // We have a configuration that can be handled by the sample code
      messageDecodedByApplication = true;

      // Create and store reader handler
      MyDataSetReaderCallback* pDataSetReaderHandler = new MyDataSetReaderCallback;
      m_handledReaders.push_back(pDataSetReaderHandler);

      // Set callback on DataSetReader
      pDataSetReader->setDataSetReaderCallback(pDataSetReaderHandler);
    } else {
      // The message must be handled by the SDK
      messageDecodedByApplication = false;
    }
  } else {
    // The message must be handled by the SDK
    messageDecodedByApplication = false;
  }
}

/* Callback to inform application about a state change of a PubSub object.
 */
void MyPubSubCallback::pubSubObjectStateChange(
    PubSubBase::PubSubObject* pPubSubObject,                      // [in] The affected PubSub object
    PubSubBase::PubSubObject::PubSubObjectType pubSubObjectType,  // [in] The type of the affected PubSub object
    OpcUa_PubSubState newState)                                   // [in] The new state of the affected PubSub object
{
  OpcUa_ReferenceParameter(pPubSubObject);
  OpcUa_ReferenceParameter(pubSubObjectType);
  OpcUa_ReferenceParameter(newState);
}

/* Callback to inform application about the deletion of a PubSub object.
 */
void MyPubSubCallback::pubSubObjectRemoved(
    PubSubBase::PubSubObject* pPubSubObject,                      // [in] The affected PubSub object
    PubSubBase::PubSubObject::PubSubObjectType pubSubObjectType)  // [in] The type of the affected PubSub object
{
  OpcUa_ReferenceParameter(pPubSubObject);
  OpcUa_ReferenceParameter(pubSubObjectType);
}

/** Callback informing the application about the start up of the DataSetWriter in the server SDK.
 */
OpcUa_StatusCode MyDataSetWriterCallback::startUpWriter(
    PubSubBase::DataSetWriter* pDataSetWriter)  //!< [in] The affected DataSetWriter
{
  if (pDataSetWriter->dataSetFieldContentMask() & OpcUa_DataSetFieldContentMask_RawData) {
    OpcUa_Int32 i = 0;
    m_fieldDataTypes.reserve(pDataSetWriter->pPublishedDataSet()->pDataSetMetaData()->NoOfFields);
    for (i = 0; i < pDataSetWriter->pPublishedDataSet()->pDataSetMetaData()->NoOfFields; i++) {
      if (pDataSetWriter->pPublishedDataSet()->pDataSetMetaData()->Fields[i].BuiltInType >= OpcUaType_SByte &&
          pDataSetWriter->pPublishedDataSet()->pDataSetMetaData()->Fields[i].BuiltInType <= OpcUaType_Double) {
        // The sample code is only able to handle number data types
        m_fieldDataTypes.push_back(
            (OpcUa_BuiltInType)pDataSetWriter->pPublishedDataSet()->pDataSetMetaData()->Fields[i].BuiltInType);
      } else {
        // We return an error if the DataSet contains fields that are not numbers
        return OpcUa_BadConfigurationError;
      }
    }

    return OpcUa_Good;
  } else {
    return OpcUa_BadConfigurationError;
  }
}

/** Callback for encoding the fields of a new DataSetMessage before sending
 *
 *  The application is responsible for writing the field count and the field data.
 *  The DataSetMessage header is already filled by the SDK.
 *
 *  The out parameter pDataSetMessageContext allows to influence the header to set the
 *  timestamp, handle error cases or to change the message type between messages.
 */
OpcUa_StatusCode MyDataSetWriterCallback::writeDataSetMessageFields(
    PubSubBase::DataSetWriter* pDataSetWriter,  //!< [in] The affected DataSetWriter
    struct ua_encoder_context*
        pEncoder,  //!< [in] The encoder object used to write the DataSetMessage fields into the payload.
    struct pubsub_datasetmsg_ctx*
        pDataSetMessageContext /**< [in,out] Context and header fields for the DataSetMessage.<br>
The fields minor_version, major_version, sequence_number, type and encoding are set by the caller.<br>
The fields ts, status and valid must be set by the method implementation.<br>
- ts: Timestamp of the DataSetMessage.
- status: The overall status of the DataSetMessage as severity and sub code. ua_statuscode_get_code_to_uint16 can
convert a StatusCode to a UInt16 value that contains only severity and sub code
- valid: Flag indicating if the DataSetMessage is valid
- type: Type of the written DataSetMessage. The type is set my the caller but may be modified by the implementation in
case of Event or Data Delta Frames. The type is a bit mask indicating the type of the written DataSetMessage.<br> The
bit range 0-3 indicates the DataSetMessage type listed in the following table<br> Bit Values |  Description
-----------|-------------------
0000       | Data Key Frame
0001       | Data Delta Frame
0010       | Event
0011       | Keep Alive
Bit 4 indicates if more events are available if the type is Event.*/
) {
  OpcUa_ReferenceParameter(pDataSetWriter);

  int ret = 0;
  size_t i = 0;

  pDataSetMessageContext->valid = true;
  pDataSetMessageContext->status = 0;
  ua_datetime_now(&pDataSetMessageContext->ts);

  // A very simple simulation of the sent data
  m_simulationValue++;
  m_simulationValueSigned++;

  // We have raw encoding - this was checked before
  // Encode simulated value
  for (i = 0; i < m_fieldDataTypes.size(); i++) {
    switch (m_fieldDataTypes[i]) {
      case OpcUaType_SByte: {
        int8_t tempVal = m_simulationValueSigned;
        ret = ua_encode_uint8_value(pEncoder, (uint8_t)tempVal);
        if (ret != 0) {
          // Fatal error
          return OpcUa_BadEncodingError;
        }
        break;
      }
      case OpcUaType_Byte: {
        uint8_t tempVal = m_simulationValue;
        ret = ua_encode_uint8_value(pEncoder, tempVal);
        if (ret != 0) {
          // Fatal error
          return OpcUa_BadEncodingError;
        }
        break;
      }
      case OpcUaType_Int16: {
        int16_t tempVal = m_simulationValueSigned;
        ret = ua_encode_int16(pEncoder, &tempVal);
        if (ret != 0) {
          // Fatal error
          return OpcUa_BadEncodingError;
        }
        break;
      }
      case OpcUaType_UInt16: {
        uint16_t tempVal = m_simulationValue;
        ret = ua_encode_uint16(pEncoder, &tempVal);
        if (ret != 0) {
          // Fatal error
          return OpcUa_BadEncodingError;
        }
        break;
      }
      case OpcUaType_Int32: {
        int32_t tempVal = m_simulationValueSigned;
        ret = ua_encode_int32(pEncoder, &tempVal);
        if (ret != 0) {
          // Fatal error
          return OpcUa_BadEncodingError;
        }
        break;
      }
      case OpcUaType_UInt32: {
        uint32_t tempVal = m_simulationValue;
        ret = ua_encode_uint32(pEncoder, &tempVal);
        if (ret != 0) {
          // Fatal error
          return OpcUa_BadEncodingError;
        }
        break;
      }
      case OpcUaType_Int64: {
        int64_t tempVal = m_simulationValueSigned;
        ret = ua_encode_int64(pEncoder, &tempVal);
        if (ret != 0) {
          // Fatal error
          return OpcUa_BadEncodingError;
        }
        break;
      }
      case OpcUaType_UInt64: {
        uint64_t tempVal = m_simulationValue;
        ret = ua_encode_uint64(pEncoder, &tempVal);
        if (ret != 0) {
          // Fatal error
          return OpcUa_BadEncodingError;
        }
        break;
      }
      case OpcUaType_Float: {
        float tempVal = (float)(m_simulationValueSigned + 0.5);
        ret = ua_encode_float(pEncoder, &tempVal);
        if (ret != 0) {
          // Fatal error
          return OpcUa_BadEncodingError;
        }
        break;
      }
      case OpcUaType_Double: {
        double tempVal = m_simulationValueSigned + 0.5;
        ret = ua_encode_double(pEncoder, &tempVal);
        if (ret != 0) {
          // Fatal error
          return OpcUa_BadEncodingError;
        }
        break;
      }
      default: {
        // No fatal error but we make the message invalid
        // This allows other DataSetMessages in the same Network message to be sent.
        pDataSetMessageContext->valid = false;
        return OpcUa_Good;
      }
    }
  }

  return OpcUa_Good;
}

MyDataSetReaderCallback::MyDataSetReaderCallback() {}

MyDataSetReaderCallback::~MyDataSetReaderCallback() {}

/** Callback informing the application about the start up of the DataSetReader in the server SDK.
 */
OpcUa_StatusCode MyDataSetReaderCallback::startUpReader(
    PubSubBase::DataSetReader* pDataSetReader)  //!< [in] The affected DataSetReader
{
  if (pDataSetReader->dataSetFieldContentMask() & OpcUa_DataSetFieldContentMask_RawData) {
    m_sReaderName = pDataSetReader->name();

    OpcUa_Int32 i = 0;
    m_fieldDataTypes.reserve(pDataSetReader->pDataSetMetaData()->NoOfFields);
    for (i = 0; i < pDataSetReader->pDataSetMetaData()->NoOfFields; i++) {
      if (pDataSetReader->pDataSetMetaData()->Fields[i].BuiltInType >= OpcUaType_SByte &&
          pDataSetReader->pDataSetMetaData()->Fields[i].BuiltInType <= OpcUaType_Double) {
        // The sample code is only able to handle number data types
        m_fieldDataTypes.push_back((OpcUa_BuiltInType)pDataSetReader->pDataSetMetaData()->Fields[i].BuiltInType);
      } else {
        // We return an error if the DataSet contains fields that are not numbers
        return OpcUa_BadConfigurationError;
      }
    }

    return OpcUa_Good;
  } else {
    return OpcUa_BadConfigurationError;
  }
}

/** Callback informing the application about the shut down of the PubSub module in the server SDK.
 *  The application is not allowed to call any methods on the PubSubModule if this method call is returned.
 */
OpcUa_StatusCode MyDataSetReaderCallback::shutDownReader(
    PubSubBase::DataSetReader* pDataSetReader)  //!< [in] The affected DataSetReader
{
  OpcUa_ReferenceParameter(pDataSetReader);
  return OpcUa_Good;
}

/** Callback informing about a new message received for a DataSetReader
 *
 * This callback is used if the application decodes the received message.
 *
 * The method should not block
 */
OpcUa_StatusCode MyDataSetReaderCallback::newMessageReceived(
    PubSubBase::DataSetReader* pDataSetReader,  //!< [in] The affected DataSetReader
    struct ua_decoder_context*
        pDecoder,  //!< [in] The decoder object used to read the DataSetMessage fields from the message payload.
    struct pubsub_datasetmsg_ctx* pDataSetMessageContext /**< [in,out] Context and header fields for the
        DataSetMessage.<br> The following fields are contained in the structure<br>
        - ts: Timestamp of the DataSetMessage.
        - status: The overall status of the DataSetMessage as severity and sub code. ua_statuscode_from_uint16_code can
        convert the UInt16 value to a StatusCode
        - valid: Flag indicating if the DataSetMessage is valid
        - sequence_number: Sequence number of the received DataSetMessage
        - minor_version: Minor version for the received message based on DataSetMetaData of the sender
        - major_version Major version for the received message based on DataSetMetaData of the sender
        - type: Type of the DataSetMessage.
        The type is a bit mask indicating the type of the DataSetMessage.<br>
        The bit range 0-3 indicates the DataSetMessage type listed in the following table<br>
        Bit Values |  Description
        -----------|-------------------
        0000       | Data Key Frame
        0001       | Data Delta Frame
        0010       | Event
        0011       | Keep Alive */
) {
  OpcUa_ReferenceParameter(pDataSetReader);

  printf("New DataSetMessage received for %s\n", m_sReaderName.toUtf8());

  OpcUa_StatusCode messageStatusCode = OpcUa_StatusCode_SetFromUInt16(pDataSetMessageContext->status);
  if (OpcUa_IsBad(messageStatusCode)) {
    printf("   Message status is BAD 0x%" OpcUa_PriXUInt32 " - not able to process fields\n", messageStatusCode);
    return OpcUa_Good;
  } else if (OpcUa_IsUncertain(messageStatusCode)) {
    printf("   Message status is UNCERTAIN 0x%" OpcUa_PriXUInt32 "\n", messageStatusCode);
  }

  int ret = 0;
  size_t i = 0;

  // We have raw encoding - this was checked before
  // Decode fields and print message to console
  for (i = 0; i < m_fieldDataTypes.size(); i++) {
    switch (m_fieldDataTypes[i]) {
      case OpcUaType_SByte: {
        int8_t tempVal;
        ret = ua_decode_int8(pDecoder, &tempVal);
        if (ret == 0) {
          printf("   Field[%" OpcUa_PriSize_t "] %" OpcUa_PriSByte "\n", i, tempVal);
        }
        break;
      }
      case OpcUaType_Byte: {
        uint8_t tempVal;
        ret = ua_decode_uint8(pDecoder, &tempVal);
        if (ret == 0) {
          printf("   Field[%" OpcUa_PriSize_t "] %" OpcUa_PriByte "\n", i, tempVal);
        }
        break;
      }
      case OpcUaType_Int16: {
        int16_t tempVal;
        ret = ua_decode_int16(pDecoder, &tempVal);
        if (ret == 0) {
          printf("   Field[%" OpcUa_PriSize_t "] %" OpcUa_PriInt16 "\n", i, tempVal);
        }
        break;
      }
      case OpcUaType_UInt16: {
        uint16_t tempVal;
        ret = ua_decode_uint16(pDecoder, &tempVal);
        if (ret == 0) {
          printf("   Field[%" OpcUa_PriSize_t "] %" OpcUa_PriUInt16 "\n", i, tempVal);
        }
        break;
      }
      case OpcUaType_Int32: {
        int32_t tempVal;
        ret = ua_decode_int32(pDecoder, &tempVal);
        if (ret == 0) {
          printf("   Field[%" OpcUa_PriSize_t "] %" OpcUa_PriInt32 "\n", i, tempVal);
        }
        break;
      }
      case OpcUaType_UInt32: {
        uint32_t tempVal;
        ret = ua_decode_uint32(pDecoder, &tempVal);
        if (ret == 0) {
          printf("   Field[%" OpcUa_PriSize_t "] %" OpcUa_PriUInt32 "\n", i, tempVal);
        }
        break;
      }
      case OpcUaType_Int64: {
        int64_t tempVal;
        ret = ua_decode_int64(pDecoder, &tempVal);
        if (ret == 0) {
          printf("   Field[%" OpcUa_PriSize_t "] %" OpcUa_PriInt64 "\n", i, tempVal);
        }
        break;
      }
      case OpcUaType_UInt64: {
        uint64_t tempVal;
        ret = ua_decode_uint64(pDecoder, &tempVal);
        if (ret == 0) {
          printf("   Field[%" OpcUa_PriSize_t "] %" OpcUa_PriUInt64 "\n", i, tempVal);
        }
        break;
      }
      case OpcUaType_Float: {
        float tempVal;
        ret = ua_decode_float(pDecoder, &tempVal);
        if (ret == 0) {
          printf("   Field[%" OpcUa_PriSize_t "] %" OpcUa_PriFloat "\n", i, tempVal);
        }
        break;
      }
      case OpcUaType_Double: {
        double tempVal;
        ret = ua_decode_double(pDecoder, &tempVal);
        if (ret == 0) {
          printf("   Field[%" OpcUa_PriSize_t "] %" OpcUa_PriDouble "\n", i, tempVal);
        }
        break;
      }
      default: {
        printf("   Field[%" OpcUa_PriSize_t "] unsupported type\n", i);
        return OpcUa_BadInvalidArgument;
      }
    }
  }

  return OpcUa_Good;
}
#endif  // SUPPORT_PUBSUB
}  // namespace MoxaOpcUaClassBased