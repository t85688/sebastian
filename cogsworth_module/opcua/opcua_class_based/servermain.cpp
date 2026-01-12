// #include <stdint.h>
// #include <inttypes.h>

// #include "loginCallback.h"
#include <QDebug>

#include "classbased_nodemanagermoxans.h"
#include "moxaclassbased_nodemanagermoxaclassbasedns.h"
#include "opcserver.h"
#include "opcua_class_based_server.h"
#include "opcua_class_based_servercallback.h"
#include "opcuaclassbnm_nodemanageropcuaclassbnmns.h"
#include "shutdown.h"
#include "srvtrace.h"
#include "uaplatformlayer.h"
#include "uathread.h"
#include "xmldocument.h"
// kene+
#include <opcua_trace.h>
#include <serverconfigbase.h>
// kene-

namespace MoxaOpcUaClassBased {

OpcServer* g_pServer;
ServerCallback g_serverCallback;

void opcua_class_based_server_start(WsClient* ws_client) {
  int ret = 0;
  OpcServer* pServer = NULL;
  RegisterSignalHandler();
  // Extract application path
  char* pszAppPath = getAppPath();
  // kene+
  char* pszBindingUri = getBindingUri();
  char* pszConfPath = getConfPath();
  char* pszTracePath = getTracePath();
  // kene-

  //- Initialize the environment --------------
  // Initialize the XML Parser
  UaXmlDocument::initParser();

  // LoginCallback serverCallback;

  // Initialize the UA Stack platform layer
  ret = UaPlatformLayer::init();
  if (ret == 0) {
    // Create configuration file name
    // kene+
    /*
    UaString sConfigFileName(pszAppPath);
    sConfigFileName += "/ServerConfig.xml";
    */
    // kene-

    //- Start up OPC server ---------------------
    // Create and initialize server object
    pServer = new OpcServer;
    // kene+
    /*
    pServer->setServerConfig(sConfigFileName, pszAppPath);
    */
    // kene-

    // Set the callback for user authentication
    pServer->setCallback(&g_serverCallback);

    // kene+
    //  Create ServerConfig object without config file access
    ServerConfigBase* pServerConfig = new ServerConfigBase(pszAppPath, pszConfPath,
                                                           pszTracePath,                  //<UaServerConfig>
                                                           "urn:Moxa:OpcUaServer",        //    <ProductUri />
                                                           "Moxa",                        //    <ManufacturerName />
                                                           "Moxa OPC UA Server",          //    <ProductName />
                                                           SERVERCONFIG_SOFTWAREVERSION,  //    <SoftwareVersion />
                                                           SERVERCONFIG_BUILDNUMBER,      //    <BuildNumber />
                                                           "urn:Moxa:OpcUaServer",        //    <ServerUri />
                                                           "MoxaOpcUaServer@localhost",   //    <ServerName />
                                                           pServer,                       //
                                                           &g_serverCallback              //
    );                                                                                    //
    pServerConfig->setStackTraceSettings(                                                 //    <Trace>
        OpcUa_False,                                       //        <UaStackTraceEnabled />
        OPCUA_TRACE_OUTPUT_LEVEL_ALL                       //        <UaStackTraceLevel />
    );                                                     //
    pServerConfig->setServerTraceSettings(                 //
        OpcUa_True,                                        //        <UaAppTraceEnabled />
        UaTrace::Data,                                     //        <UaAppTraceLevel />
        100000,                                            //        <UaAppTraceMaxEntries />
        5,                                                 //        <UaAppTraceMaxBackup />
        UaString("%1/UaServerCPP.log").arg(pszTracePath),  //        <UaAppTraceFile />
        OpcUa_False                                        //        <UaAppTraceDisableFlush />
    );                                                     //    </Trace>
                                                           //</UaServerConfig>

    // Create endpoint configuration
    UaEndpointBase* pEndpoint = new UaEndpointBase();
    char szHostName[256] = "0.0.0.0";
    UA_GetHostname(szHostName, 256);                        //<UaServerConfig>
    pEndpoint->setEndpointUrl(                              //    <UaEndpoint>
        UaString(pszBindingUri),                            //        <Url />
        OpcUa_True                                          //        <StackUrl />
    );                                                      //
    pEndpoint->setAutomaticallyTrustAllClientCertificates(  //
        OpcUa_True                                          //        <AutomaticallyTrustAllClientCertificates />
    );                                                      //
                                                            //
    // add Endpoint with SecurityPolicy None                //
    UaEndpointSecuritySetting securitySetting;       //
    securitySetting.setSecurityPolicy(               //        <SecuritySetting>
        OpcUa_SecurityPolicy_None                    //            <SecurityPolicy />
    );                                               //
    securitySetting.addMessageSecurityMode(          //
        OPCUA_ENDPOINT_MESSAGESECURITYMODE_NONE      //            <MessageSecurityMode />
    );                                               //
    pEndpoint->addSecuritySetting(securitySetting);  //        </SecuritySetting>
                                                     //
    // add Endpoint with SecurityPolicy Basic256Sha256      //
    securitySetting.setSecurityPolicy(                     //        <SecuritySetting>
        OpcUa_SecurityPolicy_Basic256Sha256                //            <SecurityPolicy />
    );                                                     //
    securitySetting.clearMessageSecurityModes();           //
    securitySetting.addMessageSecurityMode(                //
        OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGN            //            <MessageSecurityMode />
    );                                                     //
    securitySetting.addMessageSecurityMode(                //
        OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGNANDENCRYPT  //            <MessageSecurityMode />
    );                                                     //
    pEndpoint->addSecuritySetting(securitySetting);        //        </SecuritySetting>
                                                           //    </UaEndpoint>
    // configure rejected directory                         //
    pServerConfig->setRejectedCertificatesDirectory(        //
        UaString("%1/pkiserver/rejected").arg(pszConfPath)  //    <RejectedCertificatesDirectory />
    );                                                      //
                                                            //
    // configure available user tokens                      //
    pServerConfig->setEnableAnonymous(                    //    <UserIdentityTokens>
        OpcUa_False                                       //        <EnableAnonymous />
    );                                                    //
    pServerConfig->setEnableUserPw(                       //
        OpcUa_True                                        //        <EnableUserPw />
    );                                                    //
    pServerConfig->setEnableCertificate(                  //
        OpcUa_True                                        //        <EnableCertificate />
    );                                                    //
    pServerConfig->setUserIdentityTokenSecurityPolicy(    //
        OpcUa_SecurityPolicy_Basic256Sha256               //        <SecurityPolicy />
    );                                                    //
    pServerConfig->setRejectedUserCertificatesCount(      //
        100                                               //        <RejectedUserCertificatesCount />
    );                                                    //
    pServerConfig->setRejectedUserCertificatesDirectory(  //
        UaString("%1/pkiuser/rejected").arg(pszConfPath)  //        <RejectedUserCertificatesDirectory />
    );                                                    //    </UserIdentityTokens>
                                                          //</UaServerConfig>

    // configure certificate store for user certificates
    CertificateConfiguration* pCertConfig = NULL;
    CertificateStoreConfiguration* pCertStoreConfig = NULL;
    UaString sRejected;
    OpcUa_UInt32 nNumRejected = 0;
    OpcUa_Boolean bCertTokenconfigured = OpcUa_False;

    pServerConfig->getDefaultUserCertificateStore(&pCertStoreConfig, sRejected,
                                                  nNumRejected,         //<UaServerConfig>
                                                  bCertTokenconfigured  //    <DefaultApplicationCertificateStore>
    );                                                                  //        <OpenSSLStore>
    pCertStoreConfig->m_sCertificateTrustListLocation                   //            <CertificateTrustListLocation />
        = UaString("%1/pkiuser/trusted/certs").arg(pszConfPath);        //
    pCertStoreConfig->m_sCertificateRevocationListLocation        //            <CertificateRevocationListLocation />
        = UaString("%1/pkiuser/trusted/crl").arg(pszConfPath);    //
    pCertStoreConfig->m_sIssuersCertificatesLocation              //            <IssuersCertificatesLocation />
        = UaString("%1/pkiuser/issuers/certs").arg(pszConfPath);  //
    pCertStoreConfig->m_sIssuersRevocationListLocation            //            <IssuersRevocationListLocation />
        = UaString("%1/pkiuser/issuers/crl").arg(pszConfPath);    //        </OpenSSLStore>
                                                                  //    </DefaultApplicationCertificateStore>
                                                                  //</UaServerConfig>

    // add Endpoint
    pServerConfig->addEndpoint(pEndpoint);
    // Set the configuration object for the server
    pServer->setServerConfig(pServerConfig);
    // kene-

    // ++++++ Additional code begin +++++++++++++++++++++++++

    // Add NodeManager for the server specific nodes
    ClassBased::pOpcUaClassBnm = new OpcUaClassBnm::NodeManagerOpcUaClassBnmNS(false);
    pServer->addNodeManager(ClassBased::pOpcUaClassBnm);
    ClassBased::pClassBased = new MoxaClassBased::NodeManagerMoxaClassBasedNS(false);
    pServer->addNodeManager(ClassBased::pClassBased);
    ClassBased::pMoxaNodeManager = new ClassBased::MoxaNodeManager();
    ClassBased::pMoxaNodeManager->setWsClient(ws_client);
    ClassBased::pMoxaNodeManager->setWsBridge(new ClassBased::WsBridge(ClassBased::pMoxaNodeManager));
    pServer->addNodeManager(ClassBased::pMoxaNodeManager);

    // ++++++ Additional code end +++++++++++++++++++++++++

    qInfo() << "opcua class based server is running";

    // Start server object
    ret = pServer->start();
    if (ret == 0) {
      g_serverCallback.setServerManager(pServer->getServerManager());
    } else {
      // Print errors
      std::list<UaString> errorList = SrvT::getPreFileTraces();
      if (errorList.size() > 0) {
        std::list<UaString>::iterator it;
        for (it = errorList.begin(); it != errorList.end(); it++) {
          printf("%s", it->toUtf8());
        }
      } else {
        printf(" Check log file for details\n");
      }
      delete pServer;
      pServer = NULL;
    }
    // printf("***************************************************\n");
    // printf(" Press %s to shut down server\n", SHUTDOWN_SEQUENCE);
    // printf("***************************************************\n");
  }

  if (pszAppPath) delete[] pszAppPath;
  g_pServer = pServer;
}

void opcua_class_based_server_stop() {
  // qDebug() << "Stop opcua class based server!!";
  // OpcServer* pServer = (OpcServer*)pServer_handler;

  // printf("***************************************************\n");
  // printf(" Shutting down server\n");
  // printf("***************************************************\n");
  //- Stop OPC server -------------------------
  // Stop the server and wait three seconds if clients are connected
  // to allow them to disconnect after they received the shutdown signal
  int ret = g_pServer->stop(3, UaLocalizedText("", "User shutdown"));
  if (ret != 0) {
    qDebug() << "OPCUA Server stop failed\n";
  }
  delete g_pServer;
  g_pServer = NULL;

  //- Clean up the environment --------------
  // Clean up the UA Stack platform layer
  UaPlatformLayer::cleanup();
  // Clean up the XML Parser
  UaXmlDocument::cleanupParser();
  //-------------------------------------------

  // qDebug() << "opcua class based server is done";
}
}  // namespace MoxaOpcUaClassBased

/*
int main(int, char* [])
{
    int ret = 0;
    RegisterSignalHandler();
    // Extract application path
    char* pszAppPath = getAppPath();
    //-------------------------------------------
    // Call the OPC server main method
    ret = OpcServerMain(pszAppPath);
    //-------------------------------------------
    if (pszAppPath) delete[] pszAppPath;
    return ret;
}
*/
