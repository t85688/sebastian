#ifndef ACT_MOXA_TSN_CLIENT_AGENT_HPP
#define ACT_MOXIEISN_CLIENT_AGENTIEIP

#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QTime>

#include "act_device.hpp"
#include "act_json.hpp"
#include "act_status.hpp"
#include "client/act_moxa_iei_client.hpp"
#include "deploy_entry/act_deploy_table.hpp"
#include "oatpp-curl/RequestExecutor.hpp"
#include "oatpp-openssl/Config.hpp"
#include "oatpp-openssl/client/ConnectionProvider.hpp"
#include "oatpp/core/data/resource/InMemoryData.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/client/HttpRequestExecutor.hpp"
#include "oatpp/web/mime/multipart/PartList.hpp"

#define ACT_RESTFUL_CLIENT_REQUEST_SUCCESS (200)              /// < The restful request status success  code
#define ACT_RESTFUL_CLIENT_REQUEST_UNAUTHORIZED (401)         /// < The restful request status UNAUTHORIZED  code
#define ACT_RESTFUL_CLIENT_REQUEST_SERVICE_UNAVAILABLE (503)  /// < The restful request status SERVICE UNAVAILABLE  code

/**
 * @brief The Moxa get string request type enum class
 *
 */
enum class ActMoxaRequestTypeEnum {
  kGetLogEntry,
  kGetSerialNumber,
  kGetDeviceName,
  kGetUptime,
  kGetProductRevision,
  kGetMxL2Redundancy,
  kGetDeviceLocation,
  kGetDeviceDescription,
  kGetContactInformation,
  kGetNetmask,
  kGetGateway,
  kGetIpv4,
  kGetL3RouterId,
  kGetL3NetworkDns,
  kGetModules,
  kGetConfigurationSyncStatus,
  kGetSystemInformation,
  kGetSystemUtilization,
  kGetPortSettingAdminStatus,
  kGetPortStatus,
  kGetPortInfo,
  kGetFiberCheckStatus,
  kGetTrafficStatistics,
  kGet1588DefaultInfo,
  kGetDot1asInfo,
  kGetSystemInformationSetting,
  kGetServiceManagement,
  kGetUserAccount,
  kGetStreamAdapter,
  kGetLoginPolicy,
  kGetSnmpTrap,
  kGetSyslogServer,
  kGetTime,
  kGetTimeStatus,
  kGetMxLp,
  kGetMxQos,
  kGetStdVlan,
  kGetMgmtVlan,
  kGetMxVlan,
  kGetTEMSTID,
  kGetMxPtp,
  kGetStDot1as,
  kGetMx1588Default,
  kGetMx1588Iec61850,
  kGetGetMx1588C37238,
  kGetRstp,
  kGetStp,
  kGetMxRstp,
  kGetRstpStatus,
  kPostHeartbeat,
  kPostReboot,
  kPostFactoryDefault,
  kPostPreStartImport,
};

/**
 * @brief The QMap Moxa get string request type enum mapping
 *
 */
static const QMap<QString, ActMoxaRequestTypeEnum> kActMoxaRequestTypeEnumMap = {
    {"GetLogEntry", ActMoxaRequestTypeEnum::kGetLogEntry},
    {"GetSerialNumber", ActMoxaRequestTypeEnum::kGetSerialNumber},
    {"GetDeviceName", ActMoxaRequestTypeEnum::kGetDeviceName},
    {"GetUptime", ActMoxaRequestTypeEnum::kGetUptime},
    {"GetProductRevision", ActMoxaRequestTypeEnum::kGetProductRevision},
    {"GetMxL2Redundancy", ActMoxaRequestTypeEnum::kGetMxL2Redundancy},
    {"GetDeviceLocation", ActMoxaRequestTypeEnum::kGetDeviceLocation},
    {"GetDeviceDescription", ActMoxaRequestTypeEnum::kGetDeviceDescription},
    {"GetContactInformation", ActMoxaRequestTypeEnum::kGetContactInformation},
    {"GetNetmask", ActMoxaRequestTypeEnum::kGetNetmask},
    {"GetGateway", ActMoxaRequestTypeEnum::kGetGateway},
    {"GetIpv4", ActMoxaRequestTypeEnum::kGetIpv4},
    {"GetL3RouterId", ActMoxaRequestTypeEnum::kGetL3RouterId},
    {"GetL3NetworkDns", ActMoxaRequestTypeEnum::kGetL3NetworkDns},
    {"GetModules", ActMoxaRequestTypeEnum::kGetModules},
    {"GetConfigurationSyncStatus", ActMoxaRequestTypeEnum::kGetConfigurationSyncStatus},
    {"GetSystemInformation", ActMoxaRequestTypeEnum::kGetSystemInformation},
    {"GetSystemUtilization", ActMoxaRequestTypeEnum::kGetSystemUtilization},
    {"GetPortSettingAdminStatus", ActMoxaRequestTypeEnum::kGetPortSettingAdminStatus},
    {"GetPortStatus", ActMoxaRequestTypeEnum::kGetPortStatus},
    {"GetPortInfo", ActMoxaRequestTypeEnum::kGetPortInfo},
    {"GetFiberCheckStatus", ActMoxaRequestTypeEnum::kGetFiberCheckStatus},
    {"GetTrafficStatistics", ActMoxaRequestTypeEnum::kGetTrafficStatistics},
    {"Get1588DefaultInfo", ActMoxaRequestTypeEnum::kGet1588DefaultInfo},
    {"GetDot1asInfo", ActMoxaRequestTypeEnum::kGetDot1asInfo},
    {"GetSystemInformationSetting", ActMoxaRequestTypeEnum::kGetSystemInformationSetting},
    {"GetServiceManagement", ActMoxaRequestTypeEnum::kGetServiceManagement},
    {"GetUserAccount", ActMoxaRequestTypeEnum::kGetUserAccount},
    {"GetLoginPolicy", ActMoxaRequestTypeEnum::kGetLoginPolicy},
    {"GetStreamAdapter", ActMoxaRequestTypeEnum::kGetStreamAdapter},
    {"GetSnmpTrap", ActMoxaRequestTypeEnum::kGetSnmpTrap},
    {"GetSyslogServer", ActMoxaRequestTypeEnum::kGetSyslogServer},
    {"GetTime", ActMoxaRequestTypeEnum::kGetTime},
    {"GetTimeStatus", ActMoxaRequestTypeEnum::kGetTimeStatus},
    {"GetMxLp", ActMoxaRequestTypeEnum::kGetMxLp},
    {"GetMxQos", ActMoxaRequestTypeEnum::kGetMxQos},
    {"GetStdVlan", ActMoxaRequestTypeEnum::kGetStdVlan},
    {"GetMgmtVlan", ActMoxaRequestTypeEnum::kGetMgmtVlan},
    {"GetMxVlan", ActMoxaRequestTypeEnum::kGetMxVlan},
    {"GetTEMSTID", ActMoxaRequestTypeEnum::kGetTEMSTID},
    {"GetRstp", ActMoxaRequestTypeEnum::kGetRstp},
    {"GetStp", ActMoxaRequestTypeEnum::kGetStp},
    {"GetMxRstp", ActMoxaRequestTypeEnum::kGetMxRstp},
    {"GetRstpStatus", ActMoxaRequestTypeEnum::kGetRstpStatus},
    {"PostReboot", ActMoxaRequestTypeEnum::kPostReboot},
    {"PostFactoryDefault", ActMoxaRequestTypeEnum::kPostFactoryDefault},
    {"PostPreStartImport", ActMoxaRequestTypeEnum::kPostPreStartImport}};

/**
 * @brief The Moxa Layer2Redundancy request key enum class
 *
 */
enum class ActMoxaL2RedundancyKeyEnum {
  kStprstp,  // SpanningTree
  kTurboringv2,
  kTurbochain,
  kDualhoming,
  kMstp,       // SpanningTree
  kIec62439_2  // Media Redundancy Protocol
};

static const QMap<QString, ActMoxaL2RedundancyKeyEnum> kActMoxaL2RedundancyKeyEnumMap = {
    {"Stprstp", ActMoxaL2RedundancyKeyEnum::kStprstp},
    {"Turboringv2", ActMoxaL2RedundancyKeyEnum::kTurboringv2},
    {"Turbochain", ActMoxaL2RedundancyKeyEnum::kTurbochain},
    {"Dualhoming", ActMoxaL2RedundancyKeyEnum::kDualhoming},
    {"Mstp", ActMoxaL2RedundancyKeyEnum::kMstp},
    {"Iec62439_2", ActMoxaL2RedundancyKeyEnum::kIec62439_2}};

/**
 * @brief The Moxa SpanningTree patch request key enum class
 *
 */
enum class ActMoxaSTPatchRequestKeyEnum {
  // merge
  kBasicRstp,

  // std1w1ap
  kSpanningTreeVersion,
  kHelloTime,
  kPriority,
  kMaxAge,
  kForwardDelay,
  kRstpEnable,
  kPortPriority,
  kForceEdge,
  kPathCost,
  // std1d1ap
  kBridgeLinkType,
  // mxrstp
  kRstpErrorRecoveryTime,
  kRstpConfigSwift,
  kRstpConfigRevert,
  kAutoEdge,
  kBpduGuard,
  kRootGuard,
  kLoopGuard,
  kBpduFilter
};

static const QMap<QString, ActMoxaSTPatchRequestKeyEnum> kActMoxaSTPatchRequestKeyEnumMap = {
    // merge
    {"BasicRstp", ActMoxaSTPatchRequestKeyEnum::kBasicRstp},

    // std1w1ap
    {"SpanningTreeVersion", ActMoxaSTPatchRequestKeyEnum::kSpanningTreeVersion},
    {"HelloTime", ActMoxaSTPatchRequestKeyEnum::kHelloTime},
    {"Priority", ActMoxaSTPatchRequestKeyEnum::kPriority},
    {"MaxAge", ActMoxaSTPatchRequestKeyEnum::kMaxAge},
    {"ForwardDelay", ActMoxaSTPatchRequestKeyEnum::kForwardDelay},
    {"RstpEnable", ActMoxaSTPatchRequestKeyEnum::kRstpEnable},
    {"PortPriority", ActMoxaSTPatchRequestKeyEnum::kPortPriority},
    {"ForceEdge", ActMoxaSTPatchRequestKeyEnum::kForceEdge},
    {"PathCost", ActMoxaSTPatchRequestKeyEnum::kPathCost},
    // std1d1ap
    {"BridgeLinkType", ActMoxaSTPatchRequestKeyEnum::kBridgeLinkType},
    // mxrstp
    {"RstpErrorRecoveryTime", ActMoxaSTPatchRequestKeyEnum::kRstpErrorRecoveryTime},
    {"RstpConfigSwift", ActMoxaSTPatchRequestKeyEnum::kRstpConfigSwift},
    {"RstpConfigRevert", ActMoxaSTPatchRequestKeyEnum::kRstpConfigRevert},
    {"AutoEdge", ActMoxaSTPatchRequestKeyEnum::kAutoEdge},
    {"BpduGuard", ActMoxaSTPatchRequestKeyEnum::kBpduGuard},
    {"RootGuard", ActMoxaSTPatchRequestKeyEnum::kRootGuard},
    {"LoopGuard", ActMoxaSTPatchRequestKeyEnum::kLoopGuard},
    {"BpduFilter", ActMoxaSTPatchRequestKeyEnum::kBpduFilter}

};

/**
 * @brief The Moxa PortSetting patch request key enum class
 *
 */
enum class ActMoxaPortSettingPatchRequestKeyEnum {
  // ifmib
  kAdminStatus
};

static const QMap<QString, ActMoxaPortSettingPatchRequestKeyEnum> kActMoxaPortSettingPatchRequestKeyEnumMap = {
    {"AdminStatus", ActMoxaPortSettingPatchRequestKeyEnum::kAdminStatus}};

/**
 * @brief The MoxaTsn Client agent class
 *
 */
class ActMoxaIEIClientAgent {
 public:
  QString token_;

 private:
  QString device_ip_;
  ActRestfulProtocolEnum protocol_;
  quint16 port_;
  std::shared_ptr<ActMoxaIEIClient> client_;

  /**
   * @brief  The transfer from ActClassObj to DtoClassObj
   *
   * @tparam T1
   * @tparam T2
   * @param act_obj
   * @param dto_object
   */
  template <class T1, class T2>
  void ActClassObjToDtoClassObj(T1 act_obj, T2 &dto_object) {
    auto act_obj_std_str = act_obj.ToString().toStdString();
    auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    oatpp::String oatpp_str = act_obj_std_str;
    dto_object = jsonObjectMapper->readFromString<T2>(oatpp_str);
  }

  /**
   * @brief Create a Restful Client object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS CreateRestfulClient() {
    ACT_STATUS_INIT();

    try {
      // Create ObjectMapper for serialization of DTOs
      auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      // Create RequestExecutor which will execute ApiClient's requests

      std::shared_ptr<oatpp::web::client::RequestExecutor> requestExecutor;
      if (protocol_ == ActRestfulProtocolEnum::kHTTPS) {  // https
        //  HTTPS
        auto config = oatpp::openssl::Config::createDefaultClientConfigShared();

        auto connectionProvider =
            oatpp::openssl::client::ConnectionProvider::createShared(config, {device_ip_.toStdString(), port_});
        requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(connectionProvider);
        client_ = ActMoxaIEIClient::createShared(requestExecutor, objectMapper);

      } else {  // http
        auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared(
            {device_ip_.toStdString(), port_, oatpp::network::Address::IP_4});

        requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(connectionProvider);
        // Create the oattp ActMoxaTsn client
        client_ = ActMoxaIEIClient::createShared(requestExecutor, objectMapper);

        // // // Test monitor
        // auto monitor = std::make_shared<oatpp::network::monitor::ConnectionMonitor>(connectionProvider);

        // monitor->addMetricsChecker(
        //     std::make_shared<oatpp::network::monitor::ConnectionMaxAgeChecker>(std::chrono::seconds(5)));

        // monitor->addMetricsChecker(std::make_shared<oatpp::network::monitor::ConnectionInactivityChecker>(
        //     std::chrono::seconds(5), std::chrono::seconds(5)));

        // requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(monitor);
        // requestExecutor->executeOnce()->putHeader("Host", device_ip_.toStdString());
        /* create connection pool */
        // auto connectionPool = std::make_shared<oatpp::network::ClientConnectionPool>(
        //     connectionProvider /* connection provider */, 5 /* max connections */,
        //     std::chrono::seconds(10) /* max lifetime of idle connection */, std::chrono::seconds(5));
        // requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(connectionPool);

        // // Retry policy
        // auto retryPolicy = std::make_shared<oatpp::web::client::SimpleRetryPolicy>(1, std::chrono::seconds(3));
        // requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(connectionProvider, retryPolicy);
      }

    } catch (std::exception &e) {
      qCritical() << __func__ << "Create the ActMoxaTsn client failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

  /**
   * @brief Check the server response status
   *
   * @return ACT_STATUS
   */
  ACT_STATUS CheckResponseStatus(const QString &called_func,
                                 const std::shared_ptr<oatpp::web::protocol::http::incoming::Response> &response) {
    ACT_STATUS_INIT();

    if (!response) {
      QString error_msg = QString("Device(%1) RESTful response is a nullptr").arg(device_ip_);
      qDebug() << called_func << error_msg.toStdString().c_str();
      return std::make_shared<ActStatusSouthboundFailed>("Device no response");
    }

    // Check success
    auto response_status_code = response->getStatusCode();
    if (response_status_code != ACT_RESTFUL_CLIENT_REQUEST_SUCCESS) {
      qDebug() << called_func << "Response Status(" << response_status_code
               << "):" << response->getStatusDescription()->c_str();

      // Print request
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      // Check UNAUTHORIZED(401)
      if (response_status_code == ACT_RESTFUL_CLIENT_REQUEST_UNAUTHORIZED) {
        return std::make_shared<ActUnauthorized>();
      }

      // Check ACT_RESTFUL_CLIENT_REQUEST_SERVICE_UNAVAILABLE(503)
      // Resource Lock
      if (response_status_code == ACT_RESTFUL_CLIENT_REQUEST_SERVICE_UNAVAILABLE) {
        return std::make_shared<ActServiceUnavailable>();
      }

      QString response_body = response->readBodyToString()->c_str();
      qDebug() << called_func
               << QString("Device(%1) reply failed. response_body: %2")
                      .arg(device_ip_)
                      .arg(response_body)
                      .toStdString()
                      .c_str();

      // Set error message
      QString error_msg = response_body;
      ActClientBadRequest bad_request_reply;
      bad_request_reply.FromString(response_body);
      if (!bad_request_reply.Getdescription().isEmpty()) {
        // Replace error message
        error_msg = bad_request_reply.Getdescription();
      }

      return std::make_shared<ActStatusSouthboundFailed>(error_msg);
    }

    return act_status;
  }

 public:
  /**
   * @brief Construct a new ActMoxaIEIClientAgent object
   *
   * @param device_ip
   * @param port
   */
  ActMoxaIEIClientAgent(const QString &device_ip, const ActRestfulProtocolEnum &protocol, const quint16 &port) {
    protocol_ = protocol;
    device_ip_ = device_ip;
    port_ = port;
  }

  /**
   * @brief Init ActMoxaIEIClientAgent object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Init() {
    ACT_STATUS_INIT();
    act_status = CreateRestfulClient();
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
    return act_status;
  }

  /**
   * @brief Do post Login by restful
   *
   * @param request_act
   * @return ACT_STATUS
   */
  ACT_STATUS Login(const ActClientLoginRequest &request_act) {
    ACT_STATUS_INIT();
    try {
      // Create dto request_body
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      request_dto =
          object_mapper->readFromString<oatpp::Fields<oatpp::Any>>(request_act.ToString().toStdString().c_str());

      // Send request
      ActClientLoginResponse act_login_response;

      auto response = client_->DoPostLogin(request_dto);
      auto response_status_code = response->getStatusCode();

      // Check success
      if (response_status_code != ACT_RESTFUL_CLIENT_REQUEST_SUCCESS) {
        QString error_msg = QString("%1(%2)").arg(response->getStatusDescription()->c_str()).arg(response_status_code);

        qCritical() << __func__ << "Response:" << error_msg;

        return std::make_shared<ActStatusSouthboundFailed>(error_msg);
      }

      // Transfer response body & assign token_
      QString response_body = response->readBodyToString()->c_str();
      act_login_response.FromString(response_body);  // use the act_class to transfer the response
      token_ = act_login_response.Getaccess_token();

      // qDebug() << __func__ << "Response body:" << response_body.toStdString().c_str();
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  /**
   * @brief Do post Logout by restful
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Logout() {
    ACT_STATUS_INIT();
    try {
      // Send request
      auto response = client_->DoPostLogout(token_.toStdString());

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

  /**
   * @brief Do patch SnmpService by restful
   *
   * @param snmp_service_request
   * @return ACT_STATUS
   */
  ACT_STATUS PatchSnmpService(const ActClientSnmpService &snmp_service_request) {
    ACT_STATUS_INIT();
    try {
      // qDebug() << __func__ << "snmp_service_request:" << snmp_service_request.ToString("mode").toStdString().c_str();

      // Create dto request_body
      // auto snmp_service_dto = ActSnmpServiceDto::createShared();
      // ActClassObjToDtoClassObj(snmp_service_request, snmp_service_dto);
      // auto service_management_dto = ActServiceManagementDto::createShared();
      // service_management_dto->snmp_service = snmp_service_dto;

      // Only enable SNMP service
      // [bugfix: 3299] Chamberlain device connection problem
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      request_dto["mode"] = oatpp::UInt8(snmp_service_request.Getmode());
      // Send request
      auto response = client_->DoPatchSnmpService(token_.toStdString(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS PatchUserAccount(const ActUserAccountTable &user_account_table) {
    ACT_STATUS_INIT();

    try {
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();

      for (auto account : user_account_table.GetAccounts().keys()) {
        auto account_dto = oatpp::Fields<oatpp::Any>::createShared();
        account_dto["active"] = oatpp::Boolean(user_account_table.GetAccounts()[account].GetActive());

        account_dto["userName"] =
            oatpp::String(user_account_table.GetAccounts()[account].GetUsername().toStdString().c_str());

        account_dto["email"] =
            oatpp::String(user_account_table.GetAccounts()[account].GetEmail().toStdString().c_str());

        QString role_str = kActUserAccountRoleEnumMap.key(user_account_table.GetAccounts()[account].GetRole());
        account_dto["role"] = oatpp::String(role_str.toStdString().c_str());

        // When the Password as empty would skip modify it.
        if (!user_account_table.GetAccounts()[account].GetPassword().isEmpty()) {
          account_dto["password"] =
              oatpp::String(user_account_table.GetAccounts()[account].GetPassword().toStdString().c_str());
        }

        request_dto[account.toStdString().c_str()] = account_dto;
      }

      // Send request
      auto response = client_->DoPatchUserAccount(token_.toStdString(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS DeleteUserAccount(const QString &account) {
    ACT_STATUS_INIT();

    try {
      // Send request
      auto response =
          client_->DoDeleteUserAccount(account.toStdString().c_str(), "true", token_.toStdString());  // send request

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS PatchManagementInterface(const ActManagementInterfaceTable &mgmt_interface_table) {
    ACT_STATUS_INIT();

    try {
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

      ActClientServiceManagement client_setting;

      // Encrypted MOXA service
      ActClientMgmtEncryptedMoxaService moxa_service;
      moxa_service.Setenable(mgmt_interface_table.GetEncryptedMoxaService().GetEnable());
      client_setting.SetencryptedMoxaService(moxa_service);

      // Http
      ActClientMgmtHttpService http_service;
      http_service.Setenable(mgmt_interface_table.GetHttpService().GetEnable());
      http_service.Setport(mgmt_interface_table.GetHttpService().GetPort());
      client_setting.SethttpService(http_service);

      // Https
      ActClientMgmtHttpsService https_service;
      https_service.Setenable(mgmt_interface_table.GetHttpsService().GetEnable());
      https_service.Setport(mgmt_interface_table.GetHttpsService().GetPort());
      client_setting.SethttpsService(https_service);

      // SNMP
      ActClientMgmtSnmpService snmp_service;
      snmp_service.Setmode(static_cast<quint16>(mgmt_interface_table.GetSnmpService().GetMode()));
      snmp_service.Setport(mgmt_interface_table.GetSnmpService().GetPort());
      snmp_service.SettransportLayerProtocol(
          static_cast<quint16>(mgmt_interface_table.GetSnmpService().GetTransportLayerProtocol()));
      client_setting.SetsnmpService(snmp_service);

      // SSH
      ActClientMgmtSshService ssh_service;
      ssh_service.Setenable(mgmt_interface_table.GetSSHService().GetEnable());
      ssh_service.Setport(mgmt_interface_table.GetSSHService().GetPort());
      client_setting.SetsshService(ssh_service);

      // Telnet
      ActClientMgmtTelnetService telnet_service;
      telnet_service.Setenable(mgmt_interface_table.GetTelnetService().GetEnable());
      telnet_service.Setport(mgmt_interface_table.GetTelnetService().GetPort());
      client_setting.SettelnetService(telnet_service);

      // Others
      client_setting.SethttpMaxLoginSessions(mgmt_interface_table.GetHttpMaxLoginSessions());
      client_setting.SetterminalMaxLoginSessions(mgmt_interface_table.GetTerminalMaxLoginSessions());

      request_dto =
          object_mapper->readFromString<oatpp::Fields<oatpp::Any>>(client_setting.ToString().toStdString().c_str());

      // Send request
      auto response = client_->DoPatchServiceManagement(token_.toStdString(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS PatchLoginPolicy(const ActLoginPolicyTable &login_policy_table) {
    ACT_STATUS_INIT();

    try {
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      request_dto["autoLogout"] = oatpp::Int32(login_policy_table.GetAutoLogoutAfter());
      request_dto["enableFailureLockout"] = oatpp::Boolean(login_policy_table.GetLoginFailureLockout());
      request_dto["failureLockoutTime"] = oatpp::Int32(login_policy_table.GetLockoutDuration());
      request_dto["loginFailureMessage"] =
          oatpp::String(login_policy_table.GetLoginAuthenticationFailureMessage().toStdString().c_str());
      request_dto["retryFailureThreshold"] = oatpp::Int32(login_policy_table.GetRetryFailureThreshold());
      request_dto["webLoginMessage"] = oatpp::String(login_policy_table.GetLoginMessage().toStdString().c_str());

      // Send request
      auto response = client_->DoPatchLoginPolicy(token_.toStdString(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS PatchSnmpTrap(const ActSnmpTrapSettingTable &snmp_trap_table) {
    ACT_STATUS_INIT();

    try {
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto host_list_dto = oatpp::List<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

      QJsonArray json_host_array;
      for (auto host : snmp_trap_table.GetHostList()) {
        QJsonDocument json_doc_host;
        ActClientSnmpTrapHostEntry client_host;
        client_host.Setvalid(true);
        client_host.SethostName(host.GetHostName());
        client_host.Setmode(static_cast<qint32>(host.GetMode()));
        client_host.Setv1v2cCommunity(host.GetTrapCommunity());
        json_doc_host = QJsonDocument::fromJson(client_host.ToString().toUtf8());

        if (!json_doc_host.isNull() && json_doc_host.isObject()) {
          json_host_array.append(json_doc_host.object());
        }
      }

      // Transfer to JSON string > QString > DTO
      QString host_list_json_str = QJsonDocument(json_host_array).toJson();

      // Create reuest DTO
      host_list_dto = object_mapper->readFromString<oatpp::List<oatpp::Any>>(host_list_json_str.toStdString().c_str());
      request_dto["host"] = host_list_dto;

      // Send request
      auto response = client_->DoPatchSnmpTrap(token_.toStdString(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS PatchSyslogServer(const ActSyslogSettingTable &syslog_table) {
    ACT_STATUS_INIT();

    try {
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto host_list_dto = oatpp::List<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

      QJsonArray json_host_array;
      ActClientSyslogServerEntry client_host;

      // Server 1
      QJsonDocument json_doc_host_1;
      client_host.Setenable(syslog_table.GetSyslogServer1());
      client_host.SetserverAddress(syslog_table.GetAddress1());
      client_host.SetserverPort(syslog_table.GetPort1());
      json_doc_host_1 = QJsonDocument::fromJson(client_host.ToString().toUtf8());
      if (!json_doc_host_1.isNull() && json_doc_host_1.isObject()) {
        json_host_array.append(json_doc_host_1.object());
      }

      // Server 2
      QJsonDocument json_doc_host_2;
      client_host.Setenable(syslog_table.GetSyslogServer2());
      client_host.SetserverAddress(syslog_table.GetAddress2());
      client_host.SetserverPort(syslog_table.GetPort2());
      json_doc_host_2 = QJsonDocument::fromJson(client_host.ToString().toUtf8());
      if (!json_doc_host_2.isNull() && json_doc_host_2.isObject()) {
        json_host_array.append(json_doc_host_2.object());
      }

      // Server 3
      QJsonDocument json_doc_host_3;
      client_host.Setenable(syslog_table.GetSyslogServer3());
      client_host.SetserverAddress(syslog_table.GetAddress3());
      client_host.SetserverPort(syslog_table.GetPort3());
      json_doc_host_3 = QJsonDocument::fromJson(client_host.ToString().toUtf8());
      if (!json_doc_host_3.isNull() && json_doc_host_3.isObject()) {
        json_host_array.append(json_doc_host_3.object());
      }

      // Transfer to JSON string > QString > DTO
      QString host_list_json_str = QJsonDocument(json_host_array).toJson();

      // Create reuest DTO
      host_list_dto = object_mapper->readFromString<oatpp::List<oatpp::Any>>(host_list_json_str.toStdString().c_str());
      request_dto["syslogFwdTable"] = host_list_dto;
      request_dto["loggingEnable"] = oatpp::Boolean(syslog_table.GetEnabled());

      // Send request
      auto response = client_->DoPatchSyslogServer(token_.toStdString(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  void ConvertActTimeDayToQDateTime(const quint16 year, const ActTimeDay &time_day, QDateTime &data_time) {
    QDate date(year, time_day.GetMonth(), 1);
    while (date.dayOfWeek() != time_day.GetDay()) {
      qDebug() << "date.dayOfWeek()" << date.dayOfWeek();
      qDebug() << "time_day.GetDay()" << time_day.GetDay();

      date = date.addDays(1);
      qDebug() << "date" << date;
    }

    date = date.addDays((time_day.GetWeek() - 1) * 7);
    qDebug() << "date (after add days)" << date;

    while (date.month() != time_day.GetMonth()) {
      date = date.addDays(-7);
      qDebug() << "date (over week)" << date;
    }
    data_time = QDateTime(date, QTime(time_day.GetHour(), time_day.GetMinute()));
  }

  void ConvertActIntervalDayToClientDurationDate(const ActTimeDay &act_start_day, const ActTimeDay &act_end_day,
                                                 ActClientTimeDate &client_start_date,
                                                 ActClientTimeDate &client_end_date) {
    QDate current_date = QDate::currentDate();

    quint16 start_year = current_date.year();
    quint16 end_year = current_date.year();

    // Start
    QDateTime start_date_time;
    ConvertActTimeDayToQDateTime(start_year, act_start_day, start_date_time);

    // End
    QDateTime end_date_time;
    ConvertActTimeDayToQDateTime(end_year, act_end_day, end_date_time);

    // Handle over the year. Find the date again for the next year.
    if (start_date_time > end_date_time) {
      ConvertActTimeDayToQDateTime(end_year + 1, act_end_day, end_date_time);
    }

    client_start_date.Setyear(start_date_time.date().year());
    client_start_date.Setmonth(start_date_time.date().month());
    client_start_date.Setdate(start_date_time.date().day());
    client_start_date.Sethour(start_date_time.time().hour());
    client_start_date.Setminute(start_date_time.time().minute());

    client_end_date.Setyear(end_date_time.date().year());
    client_end_date.Setmonth(end_date_time.date().month());
    client_end_date.Setdate(end_date_time.date().day());
    client_end_date.Sethour(end_date_time.time().hour());
    client_end_date.Setminute(end_date_time.time().minute());
  }

  ACT_STATUS PatchTimeForTsnDevice(const ActTimeSettingTable &time_table) {
    ACT_STATUS_INIT();

    try {
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

      ActClientTsnDeviceTime client_time;
      client_time.SetclockSource(kActClockSourceEnumMap.key(time_table.GetClockSource()));
      client_time.SettimeZone(kActTimeZoneEnumMap.key(time_table.GetTimeZone()));

      // DaylightSaving
      client_time.GetdaylightSaving().Setenable(time_table.GetDaylightSavingTime());

      quint16 offset_min = 0;
      act_status = ConvertOffsetToOffsetMin(time_table.GetOffset(), offset_min);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      client_time.GetdaylightSaving().Setoffset(offset_min);

      ActClientTimeDate client_start_date;
      ActClientTimeDate client_end_date;
      ConvertActIntervalDayToClientDurationDate(time_table.GetStart(), time_table.GetEnd(), client_start_date,
                                                client_end_date);
      client_time.GetdaylightSaving().SetstartDate(client_start_date);
      client_time.GetdaylightSaving().SetendDate(client_end_date);

      // NTP
      ActClientTimeNtpClient ntp_client_1, ntp_client_2;
      ntp_client_1.Setauthentication(false);
      ntp_client_1.SetserverAddress(time_table.GetNTPTimeServer1());
      ntp_client_2.Setauthentication(false);
      ntp_client_2.SetserverAddress(time_table.GetNTPTimeServer2());
      client_time.Getntp().GetntpClient().append(ntp_client_1);
      client_time.Getntp().GetntpClient().append(ntp_client_2);

      // SNTP
      ActClientTimeSntpClient sntp_client_1, sntp_client_2;
      sntp_client_1.SetserverAddress(time_table.GetSNTPTimeServer1());
      sntp_client_2.SetserverAddress(time_table.GetSNTPTimeServer2());
      client_time.Getsntp().GetsntpClient().append(sntp_client_1);
      client_time.Getsntp().GetsntpClient().append(sntp_client_2);

      request_dto =
          object_mapper->readFromString<oatpp::Fields<oatpp::Any>>(client_time.ToString().toStdString().c_str());

      // Send request
      auto response = client_->DoPatchTime(token_.toStdString(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS ConvertOffsetToOffsetMin(const QString &offset, quint16 &offset_min) {
    ACT_STATUS_INIT();
    QTime time = QTime::fromString(offset, "hh:mm");
    if (!time.isValid()) {
      QString error_msg = QString("The Offset %1 is invalid").arg(offset);
      qCritical() << __func__ << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
    offset_min = static_cast<quint16>(time.hour() * 60 + time.minute());
    return act_status;
  }

  ACT_STATUS PatchTimeForNosDevice(const ActTimeSettingTable &time_table) {
    ACT_STATUS_INIT();

    try {
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

      ActClientNosDeviceTime client_time;
      client_time.SetclockSource(kActClockSourceEnumMap.key(time_table.GetClockSource()));
      client_time.SettimeZone(kActTimeZoneEnumMap.key(time_table.GetTimeZone()));

      // DaylightSaving
      client_time.GetdaylightSaving().Setenable(time_table.GetDaylightSavingTime());

      quint16 offset_min = 0;
      act_status = ConvertOffsetToOffsetMin(time_table.GetOffset(), offset_min);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      client_time.GetdaylightSaving().SetoffsetMin(offset_min);
      client_time.GetdaylightSaving().Getstart().Setmonth(time_table.GetStart().GetMonth());
      client_time.GetdaylightSaving().Getstart().Setweek(time_table.GetStart().GetWeek());
      client_time.GetdaylightSaving().Getstart().Setday(time_table.GetStart().GetDay());
      client_time.GetdaylightSaving().Getstart().Sethour(time_table.GetStart().GetHour());
      client_time.GetdaylightSaving().Getstart().Setminute(time_table.GetStart().GetMinute());

      client_time.GetdaylightSaving().Getend().Setmonth(time_table.GetEnd().GetMonth());
      client_time.GetdaylightSaving().Getend().Setweek(time_table.GetEnd().GetWeek());
      client_time.GetdaylightSaving().Getend().Setday(time_table.GetEnd().GetDay());
      client_time.GetdaylightSaving().Getend().Sethour(time_table.GetEnd().GetHour());
      client_time.GetdaylightSaving().Getend().Setminute(time_table.GetEnd().GetMinute());

      // NTP
      ActClientTimeNtpClient ntp_client_1, ntp_client_2;
      ntp_client_1.Setauthentication(false);
      ntp_client_1.SetserverAddress(time_table.GetNTPTimeServer1());
      ntp_client_2.Setauthentication(false);
      ntp_client_2.SetserverAddress(time_table.GetNTPTimeServer2());
      client_time.Getntp().GetntpClient().append(ntp_client_1);
      client_time.Getntp().GetntpClient().append(ntp_client_2);

      // SNTP
      ActClientTimeSntpClient sntp_client_1, sntp_client_2;
      sntp_client_1.SetserverAddress(time_table.GetSNTPTimeServer1());
      sntp_client_2.SetserverAddress(time_table.GetSNTPTimeServer2());
      client_time.Getsntp().GetsntpClient().append(sntp_client_1);
      client_time.Getsntp().GetsntpClient().append(sntp_client_2);

      request_dto =
          object_mapper->readFromString<oatpp::Fields<oatpp::Any>>(client_time.ToString().toStdString().c_str());

      // Send request
      auto response = client_->DoPatchTime(token_.toStdString(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS PostTimeCommand(const ActTimeDate &time_date) {
    ACT_STATUS_INIT();

    try {
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      request_dto["year"] = oatpp::UInt16(time_date.GetYear());
      request_dto["month"] = oatpp::UInt16(time_date.GetMonth());
      request_dto["date"] = oatpp::UInt16(time_date.GetDate());
      request_dto["hour"] = oatpp::UInt16(time_date.GetHour());
      request_dto["minute"] = oatpp::UInt16(time_date.GetMinute());
      quint16 second = 0;
      request_dto["second"] = oatpp::UInt16(second);

      // Send request
      auto response = client_->DoPostTimeCommand(token_.toStdString(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS PatchMxLp(const ActLoopProtectionTable &lp_table) {
    ACT_STATUS_INIT();

    try {
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      request_dto["loopProtectEnable"] = oatpp::Boolean(lp_table.GetNetworkLoopProtection());
      request_dto["detectInterval"] = oatpp::Int32(lp_table.GetDetectInterval());

      // Send request
      auto response = client_->DoPatchMxLp(token_.toStdString(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  /**
   * @brief Do patch InformationSetting by restful
   *
   * @param info_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS PatchInformationSetting(const ActInformationSettingTable &info_setting_table) {
    ACT_STATUS_INIT();
    try {
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      request_dto["deviceName"] = oatpp::String(info_setting_table.GetDeviceName().toStdString().c_str());
      request_dto["deviceLocation"] = oatpp::String(info_setting_table.GetLocation().toStdString().c_str());
      request_dto["deviceDescription"] = oatpp::String(info_setting_table.GetDescription().toStdString().c_str());
      request_dto["contactInformation"] =
          oatpp::String(info_setting_table.GetContactInformation().toStdString().c_str());

      // Send request
      auto response = client_->DoPatchSystemInformationSetting(token_.toStdString(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS SetMxL2Redundancy(const bool &active, const ActMoxaL2RedundancyKeyEnum &protocol_key) {
    ACT_STATUS_INIT();
    try {
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

      // Get Current Device Setting(MxL2Redundancy)
      auto get_response = client_->DoGetMxL2Redundancy(token_.toStdString());

      // Check success
      act_status = CheckResponseStatus(__func__, get_response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      // Transfer response to JSON object
      QString response_body = get_response->readBodyToString()->c_str();
      response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
      QJsonDocument json_doc = QJsonDocument::fromJson(QString("{%1}").arg(response_body).toUtf8());
      QJsonObject json_obj = json_doc.object();

      // Pre set all value as false
      for (const QString &key : json_obj.keys()) {
        json_obj.insert(key, false);
      }

      // Set target protocol by active
      switch (protocol_key) {
        case ActMoxaL2RedundancyKeyEnum::kStprstp:
          json_obj.insert("stprstp", active);
          break;
        case ActMoxaL2RedundancyKeyEnum::kTurboringv2:
          json_obj.insert("turboringv2", active);
          break;
        case ActMoxaL2RedundancyKeyEnum::kTurbochain:
          json_obj.insert("turbochain", active);
          break;
        case ActMoxaL2RedundancyKeyEnum::kDualhoming:
          json_obj.insert("dualhoming", active);
          break;
        case ActMoxaL2RedundancyKeyEnum::kMstp:
          json_obj.insert("mstp", active);
          break;
        case ActMoxaL2RedundancyKeyEnum::kIec62439_2:
          json_obj.insert("iec62439_2", active);
          break;
        default:
          qCritical() << __func__
                      << QString("Not has the %1 request key case")
                             .arg(kActMoxaL2RedundancyKeyEnumMap.key(protocol_key))
                             .toStdString()
                             .c_str();
          return std::make_shared<ActStatusInternalError>("RESTful");
      }

      // Transfer to JSON string > QString > DTO
      QJsonDocument modified_doc(json_obj);
      QString modified_json_str = modified_doc.toJson();
      request_dto = object_mapper->readFromString<oatpp::Fields<oatpp::Any>>(modified_json_str.toStdString().c_str());

      auto response = client_->DoPatchMxL2Redundancy(token_.toStdString(), request_dto);  // send request

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__
                 << QString("Device(%1) reply failed. Request body: %2")
                        .arg(device_ip_)
                        .arg(object_mapper->writeToString(request_dto)->c_str())
                        .toStdString()
                        .c_str();

        return act_status;
      }

      // qDebug() << __func__
      //          << QString("Set MxL2Redundancy(%1) Response body: %2")
      //                 .arg(kActMoxaL2RedundancyKeyEnumMap.key(protocol_key))
      //                 .arg(response->readBodyToString()->c_str())
      //                 .toStdString()
      //                 .c_str();
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  /**
   * @brief Do patch PortSetting by restful
   *
   * @param port_setting_table
   * @param patch_key
   * @return ACT_STATUS
   */
  ACT_STATUS PatchPortSetting(const ActPortSettingTable &port_setting_table,
                              const ActMoxaPortSettingPatchRequestKeyEnum &patch_key) {
    ACT_STATUS_INIT();

    try {
      // Create dto request_body
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      // auto port_list_dto = oatpp::List<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;

      switch (patch_key) {
        case ActMoxaPortSettingPatchRequestKeyEnum::kAdminStatus: {
          // Get Current Device Setting(DoGetPortSettingAdminStatus) // ifmib
          auto get_response = client_->DoGetPortSettingAdminStatus(token_.toStdString());
          // Check success
          act_status = CheckResponseStatus(__func__, get_response);
          if (!IsActStatusSuccess(act_status)) {
            qDebug() << __func__
                     << QString("Device(%1) reply failed. Request body: %2")
                            .arg(device_ip_)
                            .arg(object_mapper->writeToString(request_dto)->c_str())
                            .toStdString()
                            .c_str();
            return act_status;
          }

          // Transfer response to JSON object
          QString response_body = get_response->readBodyToString()->c_str();
          response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
          ActClientIfMib client_if_mib;
          client_if_mib.FromString(QString("{%1}").arg(response_body));

          // Update request value
          for (auto port_entry : port_setting_table.GetPortSettingEntries()) {
            // Check PortId in the reply table
            auto port_index = port_entry.GetPortId() - 1;
            if (port_index < client_if_mib.GetportTable().size()) {
              client_if_mib.GetportTable()[port_index].Getenable() = port_entry.GetAdminStatus();
            }
          }

          request_dto =
              object_mapper->readFromString<oatpp::Fields<oatpp::Any>>(client_if_mib.ToString().toStdString().c_str());
          response = client_->DoPatchPortSettingAdminStatus(token_.toStdString(), request_dto);  // send request
        } break;

        default:
          qCritical() << __func__
                      << QString("Not has the %1 request key case")
                             .arg(kActMoxaPortSettingPatchRequestKeyEnumMap.key(patch_key))
                             .toStdString()
                             .c_str();
          return std::make_shared<ActStatusInternalError>("RESTful");
      }
      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__
                 << QString("Device(%1) reply failed. Request body: %2")
                        .arg(device_ip_)
                        .arg(object_mapper->writeToString(request_dto)->c_str())
                        .toStdString()
                        .c_str();
        return act_status;
      }

      // qDebug() << __func__
      //          << QString("Patch RSTP(%1) Response body: %2")
      //                 .arg(kActMoxaPortSettingPatchRequestKeyEnumMap.key(patch_key))
      //                 .arg(response->readBodyToString()->c_str())
      //                 .toStdString()
      //                 .c_str();
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  /**
   * @brief Do patch SpanningTree by restful
   *
   * @param rstp_table
   * @param patch_key
   * @return ACT_STATUS
   */
  ACT_STATUS PatchSpanningTree(const ActRstpTable &rstp_table, const ActMoxaSTPatchRequestKeyEnum &patch_key) {
    ACT_STATUS_INIT();

    try {
      // Create dto request_body
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto port_list_dto = oatpp::List<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;

      switch (patch_key) {
        case ActMoxaSTPatchRequestKeyEnum::kSpanningTreeVersion:
          request_dto["spanningTreeVersion"] = oatpp::Int64(static_cast<int>(rstp_table.GetSpanningTreeVersion()));
          response = client_->DoPatchRstp(token_.toStdString(), request_dto);  // send request
          break;
        case ActMoxaSTPatchRequestKeyEnum::kHelloTime:
          request_dto["helloTime"] = oatpp::Int64(rstp_table.GetHelloTime() * 100);
          response = client_->DoPatchRstp(token_.toStdString(), request_dto);  // send request
          break;
        case ActMoxaSTPatchRequestKeyEnum::kPriority:
          request_dto["priority"] = oatpp::Int64(rstp_table.GetPriority());
          response = client_->DoPatchRstp(token_.toStdString(), request_dto);  // send request
          break;
        case ActMoxaSTPatchRequestKeyEnum::kMaxAge:
          request_dto["maxAge"] = oatpp::Int64(rstp_table.GetMaxAge() * 100);
          response = client_->DoPatchRstp(token_.toStdString(), request_dto);  // send request
          break;
        case ActMoxaSTPatchRequestKeyEnum::kForwardDelay:
          request_dto["forwardDelay"] = oatpp::Int64(rstp_table.GetForwardDelay() * 100);
          response = client_->DoPatchRstp(token_.toStdString(), request_dto);  // send request
          break;
        case ActMoxaSTPatchRequestKeyEnum::kRstpErrorRecoveryTime:
          request_dto["rstpErrorRecoveryTime"] = oatpp::Int64(rstp_table.GetRstpErrorRecoveryTime() * 100);
          response = client_->DoPatchMxRstp(token_.toStdString(), request_dto);  // send request
          break;
        case ActMoxaSTPatchRequestKeyEnum::kRstpConfigSwift:
          request_dto["rstpConfigSwift"] = oatpp::Boolean(rstp_table.GetRstpConfigSwift());
          response = client_->DoPatchMxRstp(token_.toStdString(), request_dto);  // send request
          break;
        case ActMoxaSTPatchRequestKeyEnum::kRstpConfigRevert:
          request_dto["rstpConfigRevert"] = oatpp::Boolean(rstp_table.GetRstpConfigRevert());
          response = client_->DoPatchMxRstp(token_.toStdString(), request_dto);  // send request
          break;

        case ActMoxaSTPatchRequestKeyEnum::kRstpEnable:
        case ActMoxaSTPatchRequestKeyEnum::kPortPriority:
        case ActMoxaSTPatchRequestKeyEnum::kForceEdge:
        case ActMoxaSTPatchRequestKeyEnum::kPathCost: {
          // Get Current Device Setting(DoGetRstpPortTable)
          auto get_response = client_->DoGetRstpPortTable(token_.toStdString());
          // Check success
          act_status = CheckResponseStatus(__func__, get_response);
          if (!IsActStatusSuccess(act_status)) {
            qDebug() << __func__
                     << QString("Device(%1) reply failed. Request body: %2")
                            .arg(device_ip_)
                            .arg(object_mapper->writeToString(request_dto)->c_str())
                            .toStdString()
                            .c_str();
            return act_status;
          }

          // Transfer response to JSON object
          QString response_body = get_response->readBodyToString()->c_str();
          response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
          QJsonDocument json_doc = QJsonDocument::fromJson(QString("[%1]").arg(response_body).toUtf8());
          // Modify Port entry object
          if (!json_doc.isNull() && json_doc.isArray()) {
            QJsonArray json_array = json_doc.array();
            for (int idx = 0; idx < json_array.size(); idx++) {  // for each port_entry object
              // Get RSTP_table's port_entry
              ActRstpPortEntry rstp_port_entry(idx + 1);
              auto iter = rstp_table.GetRstpPortEntries().find(rstp_port_entry);
              if (iter == rstp_table.GetRstpPortEntries().end()) {  // not found, keep setting
                continue;
              }
              rstp_port_entry = *iter;
              QJsonObject obj = json_array[idx].toObject();
              if (patch_key == ActMoxaSTPatchRequestKeyEnum::kRstpEnable) {
                obj["rstpEnable"] = rstp_port_entry.GetRstpEnable();
              } else if (patch_key == ActMoxaSTPatchRequestKeyEnum::kPortPriority) {
                obj["portPriority"] = rstp_port_entry.GetPortPriority();
              } else if (patch_key == ActMoxaSTPatchRequestKeyEnum::kForceEdge) {
                obj["forceEdge"] = rstp_port_entry.GetEdge() == ActRstpEdgeEnum::kYes ? true : false;
              } else if (patch_key == ActMoxaSTPatchRequestKeyEnum::kPathCost) {
                obj["pathCost"] = rstp_port_entry.GetPathCost();
              }
              json_array[idx] = obj;  // update to array
            }
            // Transfer to JSON string > QString > DTO
            QJsonDocument modified_doc(json_array);
            QString modified_json_str = modified_doc.toJson();
            port_list_dto =
                object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());

          } else {
            QString error_msg = QString("Device(%1) RESTful response PortTable not an array").arg(device_ip_);
            qCritical() << __func__ << error_msg.toStdString().c_str();
            return std::make_shared<ActBadRequest>(error_msg);
          }
          request_dto["portTable"] = port_list_dto;
          response = client_->DoPatchRstp(token_.toStdString(), request_dto);  // send request
        } break;
        case ActMoxaSTPatchRequestKeyEnum::kBridgeLinkType: {
          // Get Current Device Setting(DoGetStpPortTable)
          auto get_response = client_->DoGetStpPortTable(token_.toStdString());
          // Check success
          act_status = CheckResponseStatus(__func__, get_response);
          if (!IsActStatusSuccess(act_status)) {
            return act_status;
          }

          // Transfer response to JSON object
          QString response_body = get_response->readBodyToString()->c_str();
          response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
          QJsonDocument json_doc = QJsonDocument::fromJson(QString("[%1]").arg(response_body).toUtf8());
          // Modify Port entry object
          if (!json_doc.isNull() && json_doc.isArray()) {
            QJsonArray json_array = json_doc.array();
            for (int idx = 0; idx < json_array.size(); idx++) {  // for each port_entry object
              // Get RSTP_table's port_entry
              ActRstpPortEntry rstp_port_entry(idx + 1);
              auto iter = rstp_table.GetRstpPortEntries().find(rstp_port_entry);
              if (iter == rstp_table.GetRstpPortEntries().end()) {  // not found, keep setting
                continue;
              }
              rstp_port_entry = *iter;
              QJsonObject obj = json_array[idx].toObject();
              if (patch_key == ActMoxaSTPatchRequestKeyEnum::kBridgeLinkType) {
                obj["bridgeLinkType"] = static_cast<int>(rstp_port_entry.GetLinkType());
              }
              json_array[idx] = obj;  // update to array
            }
            // Transfer to JSON string > QString > DTO
            QJsonDocument modified_doc(json_array);
            QString modified_json_str = modified_doc.toJson();
            port_list_dto =
                object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());
          } else {
            QString error_msg = QString("Device(%1) RESTful response PortTable not an array").arg(device_ip_);
            qCritical() << __func__ << error_msg.toStdString().c_str();
            return std::make_shared<ActBadRequest>(error_msg);
          }
          request_dto["portTable"] = port_list_dto;
          response = client_->DoPatchStp(token_.toStdString(), request_dto);  // send request
        } break;

        case ActMoxaSTPatchRequestKeyEnum::kAutoEdge:
        case ActMoxaSTPatchRequestKeyEnum::kBpduGuard:
        case ActMoxaSTPatchRequestKeyEnum::kRootGuard:
        case ActMoxaSTPatchRequestKeyEnum::kLoopGuard:
        case ActMoxaSTPatchRequestKeyEnum::kBpduFilter: {
          // Get Current Device Setting(DoGetMxRstpPortTable)
          auto get_response = client_->DoGetMxRstpPortTable(token_.toStdString());
          // Check success
          act_status = CheckResponseStatus(__func__, get_response);
          if (!IsActStatusSuccess(act_status)) {
            return act_status;
          }

          // Transfer response to JSON object
          QString response_body = get_response->readBodyToString()->c_str();
          response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
          QJsonDocument json_doc = QJsonDocument::fromJson(QString("[%1]").arg(response_body).toUtf8());
          // Modify Port entry object
          if (!json_doc.isNull() && json_doc.isArray()) {
            QJsonArray json_array = json_doc.array();
            for (int idx = 0; idx < json_array.size(); idx++) {  // for each port_entry object
              // Get RSTP_table's port_entry
              ActRstpPortEntry rstp_port_entry(idx + 1);
              auto iter = rstp_table.GetRstpPortEntries().find(rstp_port_entry);
              if (iter == rstp_table.GetRstpPortEntries().end()) {  // not found, keep setting
                continue;
              }
              rstp_port_entry = *iter;
              QJsonObject obj = json_array[idx].toObject();
              if (patch_key == ActMoxaSTPatchRequestKeyEnum::kAutoEdge) {
                obj["autoEdge"] = rstp_port_entry.GetEdge() == ActRstpEdgeEnum::kAuto ? true : false;
              } else if (patch_key == ActMoxaSTPatchRequestKeyEnum::kBpduGuard) {
                obj["bpduGuard"] = rstp_port_entry.GetBpduGuard();
              } else if (patch_key == ActMoxaSTPatchRequestKeyEnum::kRootGuard) {
                obj["rootGuard"] = rstp_port_entry.GetRootGuard();
              } else if (patch_key == ActMoxaSTPatchRequestKeyEnum::kLoopGuard) {
                obj["loopGuard"] = rstp_port_entry.GetLoopGuard();
              } else if (patch_key == ActMoxaSTPatchRequestKeyEnum::kBpduFilter) {
                obj["bpduFilter"] = rstp_port_entry.GetBpduFilter();
              }
              json_array[idx] = obj;  // update to array
            }
            // Transfer to JSON string > QString > DTO
            QJsonDocument modified_doc(json_array);
            QString modified_json_str = modified_doc.toJson();
            port_list_dto =
                object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());
          } else {
            QString error_msg = QString("Device(%1) RESTful response PortTable not an array").arg(device_ip_);
            qCritical() << __func__ << error_msg.toStdString().c_str();
            return std::make_shared<ActBadRequest>(error_msg);
          }
          request_dto["portTable"] = port_list_dto;
          response = client_->DoPatchMxRstp(token_.toStdString(), request_dto);  // send request
        } break;
        case ActMoxaSTPatchRequestKeyEnum::kBasicRstp: {
          // Get Current Device Setting(DoGetRstpPortTable)
          auto get_response = client_->DoGetRstpPortTable(token_.toStdString());
          // Check success
          act_status = CheckResponseStatus(__func__, get_response);
          if (!IsActStatusSuccess(act_status)) {
            qDebug() << __func__
                     << QString("Device(%1) reply failed. Request body: %2")
                            .arg(device_ip_)
                            .arg(object_mapper->writeToString(request_dto)->c_str())
                            .toStdString()
                            .c_str();
            return act_status;
          }
          // Transfer response to JSON object
          QString response_body = get_response->readBodyToString()->c_str();
          response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
          QJsonDocument json_doc = QJsonDocument::fromJson(QString("[%1]").arg(response_body).toUtf8());
          // Modify Port entry object
          if (!json_doc.isNull() && json_doc.isArray()) {
            QJsonArray json_array = json_doc.array();
            for (int idx = 0; idx < json_array.size(); idx++) {  // for each port_entry object
              // Get RSTP_table's port_entry
              ActRstpPortEntry rstp_port_entry(idx + 1);
              auto iter = rstp_table.GetRstpPortEntries().find(rstp_port_entry);
              if (iter == rstp_table.GetRstpPortEntries().end()) {  // not found, keep setting
                continue;
              }
              rstp_port_entry = *iter;
              QJsonObject obj = json_array[idx].toObject();
              obj["portPriority"] = rstp_port_entry.GetPortPriority();
              obj["forceEdge"] = rstp_port_entry.GetEdge() == ActRstpEdgeEnum::kYes ? true : false;
              obj["pathCost"] = rstp_port_entry.GetPathCost();

              json_array[idx] = obj;  // update to array
            }
            // Transfer to JSON string > QString > DTO
            QJsonDocument modified_doc(json_array);
            QString modified_json_str = modified_doc.toJson();
            port_list_dto =
                object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());

          } else {
            QString error_msg = QString("Device(%1) RESTful response PortTable not an array").arg(device_ip_);
            qCritical() << __func__ << error_msg.toStdString().c_str();
            return std::make_shared<ActBadRequest>(error_msg);
          }

          request_dto["helloTime"] = oatpp::Int64(rstp_table.GetHelloTime() * 100);
          request_dto["priority"] = oatpp::Int64(rstp_table.GetPriority());
          request_dto["maxAge"] = oatpp::Int64(rstp_table.GetMaxAge() * 100);
          request_dto["forwardDelay"] = oatpp::Int64(rstp_table.GetForwardDelay() * 100);
          request_dto["portTable"] = port_list_dto;

          response = client_->DoPatchRstp(token_.toStdString(), request_dto);  // send request
        } break;
        default:
          qCritical() << __func__
                      << QString("Not has the %1 request key case")
                             .arg(kActMoxaSTPatchRequestKeyEnumMap.key(patch_key))
                             .toStdString()
                             .c_str();
          return std::make_shared<ActStatusInternalError>("RESTful");
      }
      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__
                 << QString("Device(%1) reply failed. Request body: %2")
                        .arg(device_ip_)
                        .arg(object_mapper->writeToString(request_dto)->c_str())
                        .toStdString()
                        .c_str();
        return act_status;
      }

      // qDebug() << __func__
      //          << QString("Patch RSTP(%1) Response body: %2")
      //                 .arg(kActMoxaSTPatchRequestKeyEnumMap.key(patch_key))
      //                 .arg(response->readBodyToString()->c_str())
      //                 .toStdString()
      //                 .c_str();
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS UpdateStreamAdapterIngressIndexByDevice(ActStadPortTable &stad_port_table, const QJsonObject &json_obj,
                                                     const quint16 &ingress_index_max) {
    ACT_STATUS_INIT();

    QMap<qint64, QSet<qint32>> port_used_index_map;  // QMap<PortID, QSet<UsedIndex(1~10)>>

    // Generate the port_used_index_map from  json_obj
    if (!json_obj.contains("portTable")) {  // check has the key
      qCritical() << __func__ << "StreamAdapter response data not has the portTable key";
      return act_status;
    } else {
      QJsonArray port_table_array = json_obj["portTable"].toArray();

      for (int port_idx = 0; port_idx < port_table_array.size(); port_idx++) {  // for each port_entry object
        qint64 port_id = port_idx + 1;
        QJsonObject port_obj = port_table_array[port_idx].toObject();

        // Get ruleindex array
        QJsonArray rule_index_array = port_obj["ruleindex"].toArray();
        for (int rule_idx = 0; rule_idx < rule_index_array.size(); rule_idx++) {
          QJsonObject rule_index_obj = rule_index_array[rule_idx].toObject();

          // Skip modify the PVID(1)
          // Set it as UsedIndex
          if ((rule_index_obj["vid"].toInt() == ACT_VLAN_INIT_PVID) && (rule_index_obj["enable"].toBool())) {
            port_used_index_map[port_id].insert(rule_idx);
          }
        }
      }
    }

    // Update the stad_port_table's IngressIndex
    QSet<ActInterfaceStadPortEntry> new_if_stad_entries;
    for (auto if_stad_port_entry : stad_port_table.GetInterfaceStadPortEntries()) {
      QSet<ActStadPortEntry> new_stad_entries;
      auto port_id = if_stad_port_entry.GetInterfaceId();
      qint64 start_index = 0;
      for (auto stad_port_entry : if_stad_port_entry.GetStadPortEntries()) {
        // Find idle_index
        for (int idle_index = start_index; idle_index < ingress_index_max; idle_index++) {
          // Check the idle_index not be used
          if (port_used_index_map[port_id].contains(idle_index)) {
            if (idle_index == ingress_index_max) {  // all indexes have been used.
              qCritical() << __func__ << "All StreamPriority's IngressIndexes have been used. Interface:" << port_id;
              return std::make_shared<ActStatusInternalError>("Deploy");
            }
            // Continue to find the next idle index
            continue;
          }

          // Update the idle index to stad_port_entry
          stad_port_entry.SetIngressIndex(idle_index);
          port_used_index_map[port_id].insert(idle_index);
          start_index = idle_index + 1;
          break;
        }
        new_stad_entries.insert(stad_port_entry);
      }
      if_stad_port_entry.SetStadPortEntries(new_stad_entries);
      new_if_stad_entries.insert(if_stad_port_entry);
    }

    stad_port_table.SetInterfaceStadPortEntries(new_if_stad_entries);

    return act_status;
  }

  /**
   * @brief Do patch StreamAdapter Ingress by restful
   *
   * @param const_stad_port_table
   * @return ACT_STATUS
   */
  ACT_STATUS PatchStreamAdapterIngress(const ActStadPortTable &const_stad_port_table,
                                       const quint16 &ingress_index_max) {
    ACT_STATUS_INIT();
    auto stad_port_table = const_stad_port_table;
    // qDebug() << __func__ << "ActStadPortTable:" << stad_port_table.ToString().toStdString().c_str();

    try {
      // Create dto request_body
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto port_list_dto = oatpp::List<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;

      // Get Current Device Setting(DoGetStreamAdapter)
      auto get_response = client_->DoGetStreamAdapter(token_.toStdString());
      // Check success
      act_status = CheckResponseStatus(__func__, get_response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__
                 << QString("Device(%1) reply failed. Request body: %2")
                        .arg(device_ip_)
                        .arg(object_mapper->writeToString(request_dto)->c_str())
                        .toStdString()
                        .c_str();
        return act_status;
      }

      // Transfer response to JSON object
      QString response_body = get_response->readBodyToString()->c_str();
      response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
      QJsonDocument json_doc = QJsonDocument::fromJson(QString("{%1}").arg(response_body).toUtf8());
      QJsonObject json_obj = json_doc.object();

      // Update stad_port_entry's IngressIndex(Skip the PVID(1) && enable(true))
      act_status = UpdateStreamAdapterIngressIndexByDevice(stad_port_table, json_obj, ingress_index_max);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "UpdateStreamAdapterIngressIndexByDevice() failed";
        return act_status;
      }

      // Modify Port entry object
      if (json_obj.contains("portTable")) {
        QJsonArray port_table_array = json_obj["portTable"].toArray();

        for (int port_idx = 0; port_idx < port_table_array.size(); port_idx++) {  // for each port_entry object
          // Get stad_port_table's port_entry
          qint64 port_id = port_idx + 1;
          ActInterfaceStadPortEntry if_stad_port_entry(port_id);
          auto if_stad_port_iter = stad_port_table.GetInterfaceStadPortEntries().find(if_stad_port_entry);
          if (if_stad_port_iter != stad_port_table.GetInterfaceStadPortEntries().end()) {  // found
            if_stad_port_entry = *if_stad_port_iter;
          }

          QJsonObject port_obj = port_table_array[port_idx].toObject();
          // Get ruleindex array
          QJsonArray rule_index_array = port_obj["ruleindex"].toArray();
          for (int rule_idx = 0; rule_idx < rule_index_array.size(); rule_idx++) {
            // Get if_stad_port_entry's index_entry(stad_port_entry)
            qint32 ingress_index = rule_idx + 1;
            ActStadPortEntry stad_port_entry(port_id, ingress_index);
            auto stad_port_iter = if_stad_port_entry.GetStadPortEntries().find(stad_port_entry);
            if (stad_port_iter != if_stad_port_entry.GetStadPortEntries().end()) {  // found
              stad_port_entry = *stad_port_iter;
            } else {
              // Not found would disable it(default)
              stad_port_entry.SetIndexEnable(2);
            }

            QJsonObject rule_index_obj = rule_index_array[rule_idx].toObject();
            // Skip modify the PVID(1) && enable(true)
            if ((rule_index_obj["vid"].toInt() == ACT_VLAN_INIT_PVID) && (rule_index_obj["enable"].toBool())) {
              continue;
            }

            // Modify rule_index obj
            rule_index_obj["enable"] = stad_port_entry.GetIndexEnable() == 1 ? true : false;  // true(1), false(2)
            rule_index_obj["ethertype"] = stad_port_entry.GetEthertypeValue();
            rule_index_obj["frametype"] = stad_port_entry.GetSubtypeEnable() == 1 ? true : false;  //  true(1), false(2)
            rule_index_obj["frametypevalue"] = stad_port_entry.GetSubtypeValue();
            rule_index_obj["vid"] = stad_port_entry.GetVlanId();
            rule_index_obj["pcp"] = stad_port_entry.GetVlanPcp();

            // Check V2(L3) keys exists & modify
            if (rule_index_obj.contains("tcpPort")) {
              rule_index_obj["tcpPort"] = stad_port_entry.GetTcpPort();
            }
            if (rule_index_obj.contains("udpPort")) {
              rule_index_obj["udpPort"] = stad_port_entry.GetUdpPort();
            }
            if (rule_index_obj.contains("type")) {
              QJsonArray new_type_array;
              for (auto type_enum : stad_port_entry.GetType()) {
                QString type_str = kActStreamPriorityTypeEnumMap.key(type_enum);
                new_type_array.append(type_str);
              }
              rule_index_obj["type"] = new_type_array;
            }

            rule_index_array[rule_idx] = rule_index_obj;  // update to rule_index_array
          }
          port_obj["ruleindex"] = rule_index_array;
          port_table_array[port_idx] = port_obj;  // update to array
        }
        // Transfer to JSON string > QString > DTO
        QJsonDocument modified_doc(port_table_array);
        QString modified_json_str = modified_doc.toJson();
        port_list_dto = object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());

      } else {
        QString error_msg = QString("Device(%1) RESTful response PortTable not an array").arg(device_ip_);
        qCritical() << __func__ << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
      request_dto["portTable"] = port_list_dto;
      response = client_->DoPatchStreamAdapter(token_.toStdString(), request_dto);  // send request

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__
                 << QString("Device(%1) reply failed. Request body: %2")
                        .arg(device_ip_)
                        .arg(object_mapper->writeToString(request_dto)->c_str())
                        .toStdString()
                        .c_str();

        return act_status;
      }

      // qDebug() << __func__
      //          << QString("Patch StreamAdapter(portTable) Response body: %2")
      //                 .arg(response->readBodyToString()->c_str())
      //                 .toStdString()
      //                 .c_str();
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  /**
   * @brief Do patch StreamAdapter Egress by restful
   *
   * @param stad_config_table
   * @return ACT_STATUS
   */
  ACT_STATUS PatchStreamAdapterEgress(const ActStadConfigTable &stad_config_table) {
    ACT_STATUS_INIT();
    // qDebug() << __func__ << "ActStadConfigTable:" << stad_config_table.ToString().toStdString().c_str();

    try {
      // Create dto request_body
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto port_list_dto = oatpp::List<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;

      // Get Current Device Setting(DoGetStreamAdapter)
      auto get_response = client_->DoGetStreamAdapter(token_.toStdString());
      // Check success
      act_status = CheckResponseStatus(__func__, get_response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__
                 << QString("Device(%1) reply failed. Request body: %2")
                        .arg(device_ip_)
                        .arg(object_mapper->writeToString(request_dto)->c_str())
                        .toStdString()
                        .c_str();
        return act_status;
      }

      // Transfer response to JSON object
      QString response_body = get_response->readBodyToString()->c_str();
      response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
      QJsonDocument json_doc = QJsonDocument::fromJson(QString("{%1}").arg(response_body).toUtf8());
      QJsonObject json_obj = json_doc.object();

      // Modify Port entry object
      if (json_obj.contains("portTable")) {
        QJsonArray port_table_array = json_obj["portTable"].toArray();

        for (int port_idx = 0; port_idx < port_table_array.size(); port_idx++) {  // for each port_entry object
          // Get stad_config_table's config_entry
          qint64 port_id = port_idx + 1;
          ActStadConfigEntry stad_config_entry(port_id);
          auto stad_config_iter = stad_config_table.GetStadConfigEntries().find(stad_config_entry);
          if (stad_config_iter != stad_config_table.GetStadConfigEntries().end()) {  // found
            stad_config_entry = *stad_config_iter;
          } else {
            // Not found would disable it(default)
            stad_config_entry.SetEgressUntag(2);  // false
          }

          QJsonObject port_obj = port_table_array[port_idx].toObject();
          port_obj["egressuntag"] = stad_config_entry.GetEgressUntag() == 1 ? true : false;  // true(1), false(2)

          port_table_array[port_idx] = port_obj;  // update to array
        }
        // Transfer to JSON string > QString > DTO
        QJsonDocument modified_doc(port_table_array);
        QString modified_json_str = modified_doc.toJson();
        port_list_dto = object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());

      } else {
        QString error_msg = QString("Device(%1) RESTful response PortTable not an array").arg(device_ip_);
        qCritical() << __func__ << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
      request_dto["portTable"] = port_list_dto;
      response = client_->DoPatchStreamAdapter(token_.toStdString(), request_dto);  // send request

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__
                 << QString("Device(%1) reply failed. Request body: %2")
                        .arg(device_ip_)
                        .arg(object_mapper->writeToString(request_dto)->c_str())
                        .toStdString()
                        .c_str();
        return act_status;
      }

      // qDebug() << __func__
      //          << QString("Patch StreamAdapter(portTable) Response body: %2")
      //                 .arg(response->readBodyToString()->c_str())
      //                 .toStdString()
      //                 .c_str();
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  /**
   * @brief Do patch ManagementIp Ipv4 by restful
   *
   * @param ipv4_request
   * @return ACT_STATUS
   */
  ACT_STATUS PatchManagementIpIpv4(const ActClientManagementIpIpv4 &ipv4_request) {
    ACT_STATUS_INIT();
    try {
      qDebug() << __func__ << "ipv4_request:" << ipv4_request.ToString().toStdString().c_str();

      // Create dto request_body
      auto ipv4_dto = ActManagementIpIpv4Dto::createShared();
      ActClassObjToDtoClassObj(ipv4_request, ipv4_dto);

      // Send request
      auto response = client_->DoPatchManagementIpIpv4(token_.toStdString(), ipv4_dto);
      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS PostLocator(const quint16 &duration) {
    ACT_STATUS_INIT();
    try {
      // Create dto request_body
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      request_dto["duration"] = oatpp::UInt16(duration);

      // Send request
      auto response = client_->DoPostLocator(token_.toStdString(), request_dto);
      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  /**
   * @brief Do Get ModelName by restful
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetModelName() {
    ACT_STATUS_INIT();
    try {
      // Send request
      auto response = client_->DoGetModelName();
      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      qDebug() << __func__ << "Response body:" << response->readBodyToString()->c_str();
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  /**
   * @brief Do Get Request by restful
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetStringRequestUseToken(const ActMoxaRequestTypeEnum &type, QString &result) {
    ACT_STATUS_INIT();
    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;
      switch (type) {
        case ActMoxaRequestTypeEnum::kGetSerialNumber:
          response = client_->DoGetSerialNumber(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetDeviceName:
          response = client_->DoGetDeviceName(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetUptime:
          response = client_->DoGetUptime(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetProductRevision:
          response = client_->DoGetProductRevision(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetMxL2Redundancy:
          response = client_->DoGetMxL2Redundancy(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetDeviceLocation:
          response = client_->DoGetDeviceLocation(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetDeviceDescription:
          response = client_->DoGetDeviceDescription(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetContactInformation:
          response = client_->DoGetContactInformation(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetNetmask:
          response = client_->DoGetNetmask(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetGateway:
          response = client_->DoGetGateway(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetIpv4:
          response = client_->DoGetIpv4(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetL3RouterId:
          response = client_->DoGetL3RouterId(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetL3NetworkDns:
          response = client_->DoGetL3NetworkDns(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetModules:
          response = client_->DoGetModules(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetConfigurationSyncStatus:
          response = client_->DoGetConfigurationSyncStatus(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetSystemUtilization:
          response = client_->DoGetSystemUtilization(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetSystemInformation:
          response = client_->DoGetSystemInformation(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetPortSettingAdminStatus:
          response = client_->DoGetPortSettingAdminStatus(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetPortStatus:
          response = client_->DoGetPortStatus(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetPortInfo:
          response = client_->DoGetPortInfo(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetFiberCheckStatus:
          response = client_->DoGetFiberCheckStatus(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetTrafficStatistics:
          response = client_->DoGetTrafficStatistics(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGet1588DefaultInfo:
          response = client_->DoGet1588DefaultInfo(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetDot1asInfo:
          response = client_->DoGetDot1asInfo(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetSystemInformationSetting:
          response = client_->DoGetSystemInformationSetting(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetServiceManagement:
          response = client_->DoGetServiceManagement(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetUserAccount:
          response = client_->DoGetUserAccount(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetLoginPolicy:
          response = client_->DoGetLoginPolicy(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetStreamAdapter:
          response = client_->DoGetStreamAdapter(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetSnmpTrap:
          response = client_->DoGetSnmpTrap(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetSyslogServer:
          response = client_->DoGetSyslogServer(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetTime:
          response = client_->DoGetTime(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetTimeStatus:
          response = client_->DoGetTimeStatus(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetMxLp:
          response = client_->DoGetMxLp(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetMxQos:
          response = client_->DoGetMxQos(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetStdVlan:
          response = client_->DoGetStdVlan(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetMxVlan:
          response = client_->DoGetMxVlan(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetMgmtVlan:
          response = client_->DoGetMgmtVlan(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetMxPtp:
          response = client_->DoGetMxPtp(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetStDot1as:
          response = client_->DoGetStDot1as(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetMx1588Default:
          response = client_->DoGetMx1588Default(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetMx1588Iec61850:
          response = client_->DoGetMx1588Iec61850(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetGetMx1588C37238:
          response = client_->DoGetMx1588C37238(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetTEMSTID:
          response = client_->DoGetTEMSTIDTable(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetRstp:
          response = client_->DoGetRstp(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetStp:
          response = client_->DoGetStp(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetMxRstp:
          response = client_->DoGetMxRstp(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetRstpStatus:
          response = client_->DoGetRstpStatus(token_.toStdString());
          break;
        case ActMoxaRequestTypeEnum::kGetLogEntry:
          response = client_->DoGetLogEntry(token_.toStdString());
          break;

        default:
          qCritical()
              << __func__
              << QString("Not has the %1 request case").arg(kActMoxaRequestTypeEnumMap.key(type)).toStdString().c_str();
          return std::make_shared<ActStatusInternalError>("RESTful");
      }
      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      QString response_body = response->readBodyToString()->c_str();
      response_body = response_body.mid(1, response_body.length() - 2);  // "abc" -> abc

      // qDebug() << __func__
      //          << QString("Response body(%1)(%2):%3")
      //                 .arg(device_ip_)
      //                 .arg(kActMoxaRequestTypeEnumMap.key(type))
      //                 .arg(response_body)
      //                 .toStdString()
      //                 .c_str();

      result = response_body;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

  /**
   * @brief Do Post Request by restful
   *
   * @return ACT_STATUS
   */
  ACT_STATUS PostRequestUseToken(const ActMoxaRequestTypeEnum &type) {
    ACT_STATUS_INIT();

    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;
      switch (type) {
        case ActMoxaRequestTypeEnum::kPostHeartbeat:
          try {
            response = client_->DoPostHeartbeat(token_.toStdString());
          } catch (std::exception &e) {
            // Oattp bug. Error after server sends "Connection: closed" header.
            // https://github.com/oatpp/oatpp/issues/443
            QString err_msg(e.what());
            if (err_msg.contains("Failed to read response")) {
              return ACT_STATUS_SUCCESS;
            }
          }
          break;
        case ActMoxaRequestTypeEnum::kPostReboot:
          try {
            response = client_->DoPostReboot(token_.toStdString());
          } catch (std::exception &e) {
            // Oattp bug. Error after server sends "Connection: closed" header.
            // https://github.com/oatpp/oatpp/issues/443
            QString err_msg(e.what());
            if (err_msg.contains("Failed to read response")) {
              return ACT_STATUS_SUCCESS;
            }
          }
          break;
        case ActMoxaRequestTypeEnum::kPostFactoryDefault:
          try {
            response = client_->DoPostFactoryDefault(token_.toStdString());
          } catch (std::exception &e) {
            // Oattp bug. Error after server sends "Connection: closed" header.
            // https://github.com/oatpp/oatpp/issues/443
            QString err_msg(e.what());
            if (err_msg.contains("Failed to read response")) {
              return ACT_STATUS_SUCCESS;
            }
          }
          break;
        case ActMoxaRequestTypeEnum::kPostPreStartImport:
          response = client_->DoPostPreStartImport(token_.toStdString());
          break;
        default:
          qCritical()
              << __func__
              << QString("Not has the %1 request case").arg(kActMoxaRequestTypeEnumMap.key(type)).toStdString().c_str();
          return std::make_shared<ActStatusInternalError>("RESTful");
      }

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  /**
   * @brief Do Delete VLAN  by restful
   *
   * @param delete_vlan_list
   * @param support_temstid
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteStdVlanMember(const QList<qint32> &delete_vlan_list, const bool &support_temstid) {
    ACT_STATUS_INIT();
    try {
      // Create dto request_body
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;

      if (delete_vlan_list.isEmpty()) {
        return act_status;
      }

      // Delete the TEMSTID
      if (support_temstid) {
        act_status = DeleteTEMSTID(delete_vlan_list);
        if (!IsActStatusSuccess(act_status)) {
          return act_status;
        }
      }

      // Convert QList<quint64> to QStringList
      QStringList string_list;
      for (auto vid : delete_vlan_list) {
        string_list << QString::number(vid);
      }

      // Join the QStringList with "," to create a single QString
      QString vids_str = string_list.join(",");

      // Send delete request to DUT
      response = client_->DoDeleteVlan(vids_str.toStdString().c_str(), "true", token_.toStdString());  // send request

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__
                 << QString("Device(%1) reply failed. Request body: %2")
                        .arg(device_ip_)
                        .arg(object_mapper->writeToString(request_dto)->c_str())
                        .toStdString()
                        .c_str();
        return act_status;
      }

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

  /**
   * @brief Do Add VLAN  by restful
   *
   * @param add_vlan_list
   * @return ACT_STATUS
   */
  ACT_STATUS AddStdVlanMember(const QList<qint32> &add_vlan_list) {
    ACT_STATUS_INIT();
    try {
      // Create dto request_body
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto vlan_list_dto = oatpp::List<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;
      QSet<qint32> dut_exists_vlans;

      // Get GetStdVlan to get the number of ports
      auto get_response = client_->DoGetStdVlan(token_.toStdString());
      // Check success
      act_status = CheckResponseStatus(__func__, get_response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__ << QString("Device(%1) reply failed.").arg(device_ip_).toStdString().c_str();
        return act_status;
      }

      // Transfer response to JSON object
      QString response_body = get_response->readBodyToString()->c_str();
      response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
      // Handle data
      ActClientStdVlan client_std_vlan;
      client_std_vlan.FromString(QString("{%1}").arg(response_body));
      auto std_vlan_entry = client_std_vlan.GetvlanTable().first();
      qint64 number_of_ports = std_vlan_entry.GetegressPortsPbmp().size();
      bool dut_support_forbidden_egress_ports = true;
      if (std_vlan_entry.GetforbiddenEgressPortsPbmp().isEmpty()) {
        dut_support_forbidden_egress_ports = false;
      }

      ActClientStdVlan add_client_std_vlan;
      // Get DUT current exists VLAN & keep client_std_vlan_entry
      for (auto std_vlan : client_std_vlan.GetvlanTable()) {
        dut_exists_vlans.insert(std_vlan.Getvid());
      }

      // Transfer ActVlanStaticTable to ActClientStdVlan
      for (auto new_vid : add_vlan_list) {
        ActClientStdVlanEntry client_std_vlan_entry;
        client_std_vlan_entry.Setvid(new_vid);
        client_std_vlan_entry.Setvalid(true);

        ActClientStdVlanEntry new_client_std_vlan_entry = client_std_vlan_entry;

        // Set VLAN set Ports
        for (qint64 index = 0; index < number_of_ports; index++) {
          qint64 port_id = index + 1;

          // egressPortsPbmp
          new_client_std_vlan_entry.GetegressPortsPbmp().append(false);

          // untaggedPortsPbmp
          new_client_std_vlan_entry.GetuntaggedPortsPbmp().append(false);

          // forbiddenEgressPortsPbmp
          if (dut_support_forbidden_egress_ports) {
            new_client_std_vlan_entry.GetforbiddenEgressPortsPbmp().append(false);
          }
        }

        // Add New VLAN group
        if (!dut_exists_vlans.contains(new_vid)) {
          add_client_std_vlan.GetvlanTable().append(new_client_std_vlan_entry);
        }
      }

      // Add VLAN
      if (!add_client_std_vlan.GetvlanTable().isEmpty()) {
        QJsonArray json_std_vlan_array;
        for (auto client_std_vlan : add_client_std_vlan.GetvlanTable()) {
          QJsonDocument json_doc_std_vlan;
          if (dut_support_forbidden_egress_ports) {
            json_doc_std_vlan = QJsonDocument::fromJson(client_std_vlan.ToString().toUtf8());
          } else {
            json_doc_std_vlan =
                QJsonDocument::fromJson(client_std_vlan.ToString(client_std_vlan.no_forbidden_key_order_).toUtf8());
          }

          if (!json_doc_std_vlan.isNull() && json_doc_std_vlan.isObject()) {
            json_std_vlan_array.append(json_doc_std_vlan.object());
          }
        }

        // Transfer to JSON string > QString > DTO
        QString modified_json_str = QJsonDocument(json_std_vlan_array).toJson();

        // Create reuest DTO
        vlan_list_dto = object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());

        // Send post request to DUT
        response = client_->DoPostAddVlan(token_.toStdString(), vlan_list_dto);  // send request
                                                                                 // Check success
        act_status = CheckResponseStatus(__func__, response);
        if (!IsActStatusSuccess(act_status)) {
          qDebug() << __func__
                   << QString("Device(%1) reply failed. Request body: %2")
                          .arg(device_ip_)
                          .arg(object_mapper->writeToString(vlan_list_dto)->c_str())
                          .toStdString()
                          .c_str();
          return act_status;
        }
      }

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

  /**
   * @brief Set the StdVlanTable object (exists member)
   *
   * @param vlan_static_table
   * @return ACT_STATUS
   */
  ACT_STATUS SetStdVlanTable(const ActVlanStaticTable &vlan_static_table) {
    ACT_STATUS_INIT();
    try {
      // Create dto request_body
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto vlan_list_dto = oatpp::List<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;
      QSet<qint32> dut_exists_vlans;

      // Get StdVlan to get vlan port group
      auto get_response = client_->DoGetStdVlan(token_.toStdString());
      // Check success
      act_status = CheckResponseStatus(__func__, get_response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__ << QString("Device(%1) reply failed.").arg(device_ip_).toStdString().c_str();
        return act_status;
      }

      // Transfer response to JSON object
      QString response_body = get_response->readBodyToString()->c_str();
      response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
      // Handle data (DUT Std VLAN)
      ActClientStdVlan client_std_vlan;
      client_std_vlan.FromString(QString("{%1}").arg(response_body));
      auto std_vlan_entry = client_std_vlan.GetvlanTable().first();
      qint64 number_of_ports = std_vlan_entry.GetegressPortsPbmp().size();
      bool dut_support_forbidden_egress_ports = true;
      if (std_vlan_entry.GetforbiddenEgressPortsPbmp().isEmpty()) {
        dut_support_forbidden_egress_ports = false;
      }

      ActClientStdVlan set_client_std_vlan;
      // Get DUT current exists VLAN & keep client_std_vlan_entry
      for (auto std_vlan : client_std_vlan.GetvlanTable()) {
        dut_exists_vlans.insert(std_vlan.Getvid());
        ActVlanStaticEntry target_vlan_static_entry(std_vlan.Getvid());
        if (!vlan_static_table.GetVlanStaticEntries().contains(target_vlan_static_entry)) {
          set_client_std_vlan.GetvlanTable().append(std_vlan);
        }
      }

      // Transfer ActVlanStaticTable to ActClientStdVlan
      QList<qint64> delete_vids;
      for (auto vlan_static_entry : vlan_static_table.GetVlanStaticEntries()) {
        ActClientStdVlanEntry client_std_vlan_entry;
        client_std_vlan_entry.Setvid(vlan_static_entry.GetVlanId());
        client_std_vlan_entry.SetvlanName(vlan_static_entry.GetName());
        client_std_vlan_entry.Setvalid(true);

        // Set VLAN set Ports
        for (qint64 index = 0; index < number_of_ports; index++) {
          qint64 port_id = index + 1;

          // egressPortsPbmp
          if (vlan_static_entry.GetEgressPorts().contains(port_id)) {
            client_std_vlan_entry.GetegressPortsPbmp().append(true);
          } else {
            client_std_vlan_entry.GetegressPortsPbmp().append(false);
          }

          // untaggedPortsPbmp
          if (vlan_static_entry.GetUntaggedPorts().contains(port_id)) {
            client_std_vlan_entry.GetuntaggedPortsPbmp().append(true);
          } else {
            client_std_vlan_entry.GetuntaggedPortsPbmp().append(false);
          }

          // forbiddenEgressPortsPbmp
          if (dut_support_forbidden_egress_ports) {
            if (vlan_static_entry.GetForbiddenEgressPorts().contains(port_id)) {
              client_std_vlan_entry.GetforbiddenEgressPortsPbmp().append(true);
            } else {
              client_std_vlan_entry.GetforbiddenEgressPortsPbmp().append(false);
            }
          }
        }

        set_client_std_vlan.GetvlanTable().append(client_std_vlan_entry);
      }

      // Set VLAN
      if (!set_client_std_vlan.GetvlanTable().isEmpty()) {
        QJsonArray json_std_vlan_array;
        for (auto client_std_vlan : set_client_std_vlan.GetvlanTable()) {
          QJsonDocument json_doc_std_vlan = QJsonDocument::fromJson(client_std_vlan.ToString().toUtf8());

          if (!json_doc_std_vlan.isNull() && json_doc_std_vlan.isObject()) {
            json_std_vlan_array.append(json_doc_std_vlan.object());
          }
        }

        // Transfer to JSON string > QString > DTO
        QString modified_json_str = QJsonDocument(json_std_vlan_array).toJson();

        // Create reuest DTO
        vlan_list_dto = object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());
        // Send post request to DUT
        response = client_->DoPatchSetVlan(token_.toStdString(), vlan_list_dto);  // send request
        // Check success
        act_status = CheckResponseStatus(__func__, response);
        if (!IsActStatusSuccess(act_status)) {
          qDebug() << __func__
                   << QString("Device(%1) reply failed. Request body: %2")
                          .arg(device_ip_)
                          .arg(object_mapper->writeToString(vlan_list_dto)->c_str())
                          .toStdString()
                          .c_str();

          return act_status;
        }
      }

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

  // ACT_STATUS SetStdVlanTable(const ActVlanStaticTable &vlan_static_table) {
  //     ACT_STATUS_INIT();
  //     try {
  //       // Create dto request_body
  //       auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
  //       auto vlan_list_dto = oatpp::List<oatpp::Any>::createShared();
  //       auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
  //       std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;
  //       QSet<qint32> dut_exists_vlans;

  //       // Get GetStdVlan to get the number of ports
  //       auto get_response = client_->DoGetStdVlan(token_.toStdString());
  //       // Check success
  //       act_status = CheckResponseStatus(__func__, get_response);
  //       if (!IsActStatusSuccess(act_status)) {
  //         qDebug() << __func__ << QString("Device(%1) reply failed.").arg(device_ip_).toStdString().c_str();
  //         return act_status;
  //       }

  //       // Transfer response to JSON object
  //       QString response_body = get_response->readBodyToString()->c_str();
  //       response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
  //       // Handle data
  //       ActClientStdVlan client_std_vlan;
  //       client_std_vlan.FromString(QString("{%1}").arg(response_body));
  //       auto std_vlan_entry = client_std_vlan.GetvlanTable().first();
  //       qint64 number_of_ports = std_vlan_entry.GetegressPortsPbmp().size();
  //       bool dut_support_forbidden_egress_ports = true;
  //       if (std_vlan_entry.GetforbiddenEgressPortsPbmp().isEmpty()) {
  //         dut_support_forbidden_egress_ports = false;
  //       }

  //       ActClientStdVlan add_client_std_vlan, set_client_std_vlan;

  //       // Get DUT current exists VLAN & keep client_std_vlan_entry
  //       for (auto std_vlan : client_std_vlan.GetvlanTable()) {
  //         dut_exists_vlans.insert(std_vlan.Getvid());
  //         ActVlanStaticEntry target_vlan_static_entry(std_vlan.Getvid());
  //         if (!vlan_static_table.GetVlanStaticEntries().contains(target_vlan_static_entry)) {
  //           set_client_std_vlan.GetvlanTable().append(std_vlan);
  //         }
  //       }

  //       // Transfer ActVlanStaticTable to ActClientStdVlan
  //       // And separated into Add, Set and Delete
  //       QList<qint64> delete_vids;
  //       for (auto vlan_static_entry : vlan_static_table.GetVlanStaticEntries()) {
  //         if (vlan_static_entry.GetRowStatus() == 6) {  // Destroy
  //           // Delete
  //           delete_vids.append(vlan_static_entry.GetVlanId());
  //         } else {
  //           // Add & Set
  //           ActClientStdVlanEntry client_std_vlan_entry;
  //           client_std_vlan_entry.Setvid(vlan_static_entry.GetVlanId());
  //           client_std_vlan_entry.SetvlanName(vlan_static_entry.GetName());
  //           client_std_vlan_entry.Setvalid(true);

  //           ActClientStdVlanEntry new_client_std_vlan_entry = client_std_vlan_entry;

  //           // Set VLAN set Ports
  //           for (qint64 index = 0; index < number_of_ports; index++) {
  //             qint64 port_id = index + 1;

  //             // egressPortsPbmp
  //             // new create vlan
  //             new_client_std_vlan_entry.GetegressPortsPbmp().append(false);
  //             // set vlan
  //             if (vlan_static_entry.GetEgressPorts().contains(port_id)) {
  //               client_std_vlan_entry.GetegressPortsPbmp().append(true);
  //             } else {
  //               client_std_vlan_entry.GetegressPortsPbmp().append(false);
  //             }

  //             // untaggedPortsPbmp
  //             // new create vlan
  //             new_client_std_vlan_entry.GetuntaggedPortsPbmp().append(false);
  //             // set vlan
  //             if (vlan_static_entry.GetUntaggedPorts().contains(port_id)) {
  //               client_std_vlan_entry.GetuntaggedPortsPbmp().append(true);
  //             } else {
  //               client_std_vlan_entry.GetuntaggedPortsPbmp().append(false);
  //             }

  //             // forbiddenEgressPortsPbmp
  //             if (dut_support_forbidden_egress_ports) {
  //               // new create vlan
  //               new_client_std_vlan_entry.GetforbiddenEgressPortsPbmp().append(false);
  //               // set vlan
  //               if (vlan_static_entry.GetForbiddenEgressPorts().contains(port_id)) {
  //                 client_std_vlan_entry.GetforbiddenEgressPortsPbmp().append(true);
  //               } else {
  //                 client_std_vlan_entry.GetforbiddenEgressPortsPbmp().append(false);
  //               }
  //             }
  //           }

  //           // Add New VLAN group
  //           if (vlan_static_entry.GetRowStatus() != 0) {
  //             if (!dut_exists_vlans.contains(vlan_static_entry.GetVlanId())) {
  //               add_client_std_vlan.GetvlanTable().append(new_client_std_vlan_entry);
  //             }
  //           }

  //           set_client_std_vlan.GetvlanTable().append(client_std_vlan_entry);
  //         }
  //       }

  //       // Delete VLAN
  //       if (!delete_vids.isEmpty() && vlan_static_table.skip_config_ports_) {
  //         if (support_temstid) {
  //           // Delete the TEMSTID
  //           act_status = DeleteTEMSTID(delete_vids);
  //           if (!IsActStatusSuccess(act_status)) {
  //             return act_status;
  //           }
  //         }

  //         // Sort
  //         std::sort(delete_vids.begin(), delete_vids.end());

  //         // Convert QList<quint64> to QStringList
  //         QStringList string_list;
  //         for (auto vid : delete_vids) {
  //           string_list << QString::number(vid);
  //         }

  //         // Join the QStringList with "," to create a single QString
  //         QString vids_str = string_list.join(",");

  //         // Send delete request to DUT
  //         response = client_->DoDeleteVlan(vids_str.toStdString().c_str(), "true", token_.toStdString());  // send
  //         request

  //         // Check success
  //         act_status = CheckResponseStatus(__func__, response);
  //         if (!IsActStatusSuccess(act_status)) {
  //           qDebug() << __func__
  //                    << QString("Device(%1) reply failed. Request body: %2")
  //                           .arg(device_ip_)
  //                           .arg(object_mapper->writeToString(request_dto)->c_str())
  //                           .toStdString()
  //                           .c_str();
  //           return act_status;
  //         }
  //       }

  //       // Add VLAN
  //       if (!add_client_std_vlan.GetvlanTable().isEmpty() && vlan_static_table.skip_config_ports_) {
  //         QJsonArray json_std_vlan_array;
  //         for (auto client_std_vlan : add_client_std_vlan.GetvlanTable()) {
  //           QJsonDocument json_doc_std_vlan;
  //           if (dut_support_forbidden_egress_ports) {
  //             json_doc_std_vlan = QJsonDocument::fromJson(client_std_vlan.ToString().toUtf8());
  //           } else {
  //             json_doc_std_vlan =
  //                 QJsonDocument::fromJson(client_std_vlan.ToString(client_std_vlan.no_forbidden_key_order_).toUtf8());
  //           }

  //           if (!json_doc_std_vlan.isNull() && json_doc_std_vlan.isObject()) {
  //             json_std_vlan_array.append(json_doc_std_vlan.object());
  //           }
  //         }

  //         // Transfer to JSON string > QString > DTO
  //         QString modified_json_str = QJsonDocument(json_std_vlan_array).toJson();

  //         // Create reuest DTO
  //         vlan_list_dto =
  //         object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());

  //         // Send post request to DUT
  //         response = client_->DoPostAddVlan(token_.toStdString(), vlan_list_dto);  // send request
  //                                                                                  // Check success
  //         act_status = CheckResponseStatus(__func__, response);
  //         if (!IsActStatusSuccess(act_status)) {
  //           qDebug() << __func__
  //                    << QString("Device(%1) reply failed. Request body: %2")
  //                           .arg(device_ip_)
  //                           .arg(object_mapper->writeToString(vlan_list_dto)->c_str())
  //                           .toStdString()
  //                           .c_str();
  //           return act_status;
  //         }
  //       }

  //       // Set VLAN
  //       if (!set_client_std_vlan.GetvlanTable().isEmpty() && (!vlan_static_table.skip_config_ports_)) {
  //         QJsonArray json_std_vlan_array;
  //         for (auto client_std_vlan : set_client_std_vlan.GetvlanTable()) {
  //           QJsonDocument json_doc_std_vlan = QJsonDocument::fromJson(client_std_vlan.ToString().toUtf8());

  //           if (!json_doc_std_vlan.isNull() && json_doc_std_vlan.isObject()) {
  //             json_std_vlan_array.append(json_doc_std_vlan.object());
  //           }
  //         }

  //         // Transfer to JSON string > QString > DTO
  //         QString modified_json_str = QJsonDocument(json_std_vlan_array).toJson();

  //         // Create reuest DTO
  //         vlan_list_dto =
  //         object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());
  //         // Send post request to DUT
  //         response = client_->DoPatchSetVlan(token_.toStdString(), vlan_list_dto);  // send request
  //         // Check success
  //         act_status = CheckResponseStatus(__func__, response);
  //         if (!IsActStatusSuccess(act_status)) {
  //           qDebug() << __func__
  //                    << QString("Device(%1) reply failed. Request body: %2")
  //                           .arg(device_ip_)
  //                           .arg(object_mapper->writeToString(vlan_list_dto)->c_str())
  //                           .toStdString()
  //                           .c_str();

  //           return act_status;
  //         }
  //       }

  //     } catch (std::exception &e) {
  //       qCritical() << __func__ << "failed. Error:" << e.what();
  //       return std::make_shared<ActStatusInternalError>("RESTful");
  //     }
  //     return act_status;
  //   }

  /**
   * @brief Do patch ManagementIp Ipv4 by restful
   *
   * @param ipv4_request
   * @return ACT_STATUS
   */
  ACT_STATUS SetTEMSTID(const ActVlanStaticTable &vlan_static_table) {
    ACT_STATUS_INIT();
    try {
      // Create dto request_body
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto vlan_list_dto = oatpp::List<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;
      QList<qint32> enable_temstid_vlans;

      // Get Dut TEMSTIDTable
      auto get_response = client_->DoGetTEMSTIDTable(token_.toStdString());
      // Check success
      act_status = CheckResponseStatus(__func__, get_response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__ << QString("Device(%1) reply failed.").arg(device_ip_).toStdString().c_str();
        return act_status;
      }

      // Transfer response to JSON object
      QList<qint32> dut_enable_temstid_vlans;

      QString response_body = get_response->readBodyToString()->c_str();
      response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
      QJsonDocument json_doc = QJsonDocument::fromJson(QString("[%1]").arg(response_body).toUtf8());
      // Modify Port entry object
      if (!json_doc.isNull() && json_doc.isArray()) {
        QJsonArray json_array = json_doc.array();
        for (const QJsonValue &value : json_array) {
          dut_enable_temstid_vlans.append(static_cast<qint32>(value.toInt()));
        }
      }

      // Handle data
      enable_temstid_vlans = dut_enable_temstid_vlans;
      for (auto vlan_static_entry : vlan_static_table.GetVlanStaticEntries()) {
        auto vlan_id = vlan_static_entry.GetVlanId();
        // Check not delete entry
        if (vlan_static_entry.GetRowStatus() == 6) {  // Destroy
          continue;
        }
        if (vlan_static_entry.GetTeMstid()) {  // add
          if (!enable_temstid_vlans.contains(vlan_id)) {
            enable_temstid_vlans.append(vlan_id);
          }
        } else {  // remove
          if (enable_temstid_vlans.contains(vlan_id)) {
            enable_temstid_vlans.removeAll(vlan_id);
          }
        }
      }

      // Sort
      std::sort(enable_temstid_vlans.begin(), enable_temstid_vlans.end());

      QJsonArray json_vlan_array;
      for (auto vlan_id : enable_temstid_vlans) {
        json_vlan_array.append(vlan_id);
      }
      // Transfer to JSON string > QString > DTO
      QString modified_json_str = QJsonDocument(json_vlan_array).toJson();

      // Create reuest DTO
      vlan_list_dto = object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());

      // Send post request to DUT
      response = client_->DoPatchTEMSTIDTable(token_.toStdString(), vlan_list_dto);  // send request
                                                                                     // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__
                 << QString("Device(%1) reply failed. Request body: %2")
                        .arg(device_ip_)
                        .arg(object_mapper->writeToString(vlan_list_dto)->c_str())
                        .toStdString()
                        .c_str();
        return act_status;
      }

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

  /**
   * @brief Do patch ManagementIp Ipv4 by restful
   *
   * @param ipv4_request
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteTEMSTID(const QList<qint32> &delete_vlan_ids) {
    ACT_STATUS_INIT();
    try {
      // Create dto request_body
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto vlan_list_dto = oatpp::List<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;
      QList<qint32> enable_temstid_vlans;

      // Get Dut TEMSTIDTable
      auto get_response = client_->DoGetTEMSTIDTable(token_.toStdString());
      // Check success
      act_status = CheckResponseStatus(__func__, get_response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__ << QString("Device(%1) reply failed.").arg(device_ip_).toStdString().c_str();
        return act_status;
      }

      // Transfer response to JSON object
      QList<qint32> dut_enable_temstid_vlans;

      QString response_body = get_response->readBodyToString()->c_str();
      response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
      QJsonDocument json_doc = QJsonDocument::fromJson(QString("[%1]").arg(response_body).toUtf8());
      // Modify Port entry object
      if (!json_doc.isNull() && json_doc.isArray()) {
        QJsonArray json_array = json_doc.array();
        for (const QJsonValue &value : json_array) {
          dut_enable_temstid_vlans.append(static_cast<qint32>(value.toInt()));
        }
      }

      // Handle data
      enable_temstid_vlans = dut_enable_temstid_vlans;
      for (auto delete_vlan_id : delete_vlan_ids) {
        if (enable_temstid_vlans.contains(delete_vlan_id)) {
          enable_temstid_vlans.removeAll(delete_vlan_id);
        }
      }

      // Sort
      std::sort(enable_temstid_vlans.begin(), enable_temstid_vlans.end());

      QJsonArray json_vlan_array;
      for (auto vlan_id : enable_temstid_vlans) {
        json_vlan_array.append(vlan_id);
      }
      // Transfer to JSON string > QString > DTO
      QString modified_json_str = QJsonDocument(json_vlan_array).toJson();

      // Create reuest DTO
      vlan_list_dto = object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());

      // Send post request to DUT
      response = client_->DoPatchTEMSTIDTable(token_.toStdString(), vlan_list_dto);  // send request
                                                                                     // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__
                 << QString("Device(%1) reply failed. Request body: %2")
                        .arg(device_ip_)
                        .arg(object_mapper->writeToString(vlan_list_dto)->c_str())
                        .toStdString()
                        .c_str();
        return act_status;
      }

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

  // ACT_STATUS SetStdVlanPVID(const ActPortVlanTable &port_vlan_table) {
  //   ACT_STATUS_INIT();

  //   if (port_vlan_table.GetPortVlanEntries().isEmpty()) {
  //     qDebug() << __func__ << QString("Device(%1) port_vlan_table is empty").arg(device_ip_).toStdString().c_str();
  //     return act_status;
  //   }

  //   try {
  //     // Create dto request_body
  //     auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
  //     auto port_list_dto = oatpp::List<oatpp::Any>::createShared();
  //     auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
  //     std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;

  //     // Get GetStdVlan to get the number of ports
  //     auto get_response = client_->DoGetStdVlan(token_.toStdString());
  //     // Check success
  //     act_status = CheckResponseStatus(__func__, get_response);
  //     if (!IsActStatusSuccess(act_status)) {
  //       qDebug() << __func__
  //                << QString("Device(%1) reply failed. Request body: %2")
  //                       .arg(device_ip_)
  //                       .arg(object_mapper->writeToString(request_dto)->c_str())
  //                       .toStdString()
  //                       .c_str();
  //       return act_status;
  //     }

  //     // Transfer response to JSON object
  //     QString response_body = get_response->readBodyToString()->c_str();
  //     response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
  //     // Handle data

  //     QJsonArray json_std_port_array;

  //     ActClientStdVlan client_std_vlan;
  //     client_std_vlan.FromString(QString("{%1}").arg(response_body));
  //     for (qint64 index = 0; index < client_std_vlan.GetportTable().size(); index++) {
  //       qint64 port_id = index + 1;
  //       ActPortVlanEntry target_port_vlan_entry;
  //       target_port_vlan_entry.SetPortId(port_id);
  //       auto entry_iter = port_vlan_table.GetPortVlanEntries().find(target_port_vlan_entry);
  //       if (entry_iter != port_vlan_table.GetPortVlanEntries().end()) {
  //         client_std_vlan.GetportTable()[index].Setpvid(entry_iter->GetPVID());
  //       }

  //       // Append to JSON array
  //       QJsonDocument json_doc_std_port =
  //           QJsonDocument::fromJson(client_std_vlan.GetportTable()[index].ToString().toUtf8());
  //       if (!json_doc_std_port.isNull() && json_doc_std_port.isObject()) {
  //         json_std_port_array.append(json_doc_std_port.object());
  //       }
  //     }

  //     // Not empty would set PVID
  //     if (!json_std_port_array.isEmpty()) {
  //       // Transfer to JSON string > QString > DTO
  //       QString modified_json_str = QJsonDocument(json_std_port_array).toJson();

  //       // Create reuest DTO
  //       port_list_dto =
  //       object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());

  //       // Send post request to DUT
  //       response = client_->DoPatchStdVlanPortTable(token_.toStdString(), port_list_dto);  // send request
  //                                                                                          // Check success
  //       act_status = CheckResponseStatus(__func__, response);
  //       if (!IsActStatusSuccess(act_status)) {
  //         qDebug() << __func__
  //                  << QString("Device(%1) reply failed. Request body: %2")
  //                         .arg(device_ip_)
  //                         .arg(object_mapper->writeToString(port_list_dto)->c_str())
  //                         .toStdString()
  //                         .c_str();
  //         return act_status;
  //       }

  //       // qDebug() << __func__
  //       //          << QString("modified_json_str:
  //       //          %1").arg(modified_json_str.toStdString().c_str()).toStdString().c_str();
  //     }

  //   } catch (std::exception &e) {
  //     qCritical() << __func__ << "failed. Error:" << e.what();
  //     return std::make_shared<ActStatusInternalError>("RESTful");
  //   }
  //   return act_status;
  // }

  ACT_STATUS SetStdVlanPVID(const ActPortVlanTable &port_vlan_table) {
    ACT_STATUS_INIT();

    if (port_vlan_table.GetPortVlanEntries().isEmpty()) {
      qDebug() << __func__ << QString("Device(%1) port_vlan_table is empty").arg(device_ip_).toStdString().c_str();
      return act_status;
    }

    try {
      // Create dto request_body
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;

      QMap<quint16, QList<qint64>> pvid_ports_map;  // <PVID, QList<Port>>

      // Aggregate the port PVID
      for (auto port_vlan_entry : port_vlan_table.GetPortVlanEntries()) {
        if (pvid_ports_map.contains(port_vlan_entry.GetPVID())) {
          pvid_ports_map[port_vlan_entry.GetPVID()].append(port_vlan_entry.GetPortId());
        } else {
          pvid_ports_map[port_vlan_entry.GetPVID()] = {port_vlan_entry.GetPortId()};
        }
      }

      // Send PVID request
      for (auto pvid : pvid_ports_map.keys()) {
        auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
        request_dto["pvid"] = oatpp::UInt16(pvid);
        QList<qint64> ports = pvid_ports_map[pvid];

        // Sort
        std::sort(ports.begin(), ports.end());

        // Convert QList<quint64> to QStringList
        QStringList string_list;
        for (auto port : ports) {
          auto port_index = port - 1;
          string_list << QString::number(port_index);
        }
        // Join the QStringList with "," to create a single QString
        QString ports_index_str = string_list.join(",");

        // Send delete request to DUT
        response = client_->DoPostVlanPortConfig(ports_index_str.toStdString().c_str(), "true", token_.toStdString(),
                                                 request_dto);  // send request
        act_status = CheckResponseStatus(__func__, response);
        if (!IsActStatusSuccess(act_status)) {
          qDebug() << __func__
                   << QString("Device(%1) reply failed. Request body: %2")
                          .arg(device_ip_)
                          .arg(object_mapper->writeToString(request_dto)->c_str())
                          .toStdString()
                          .c_str();
          return act_status;
        }
      }

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

  //  ACT_STATUS SetStdVlanPVID(const ActPortVlanTable &port_vlan_table) {
  //     ACT_STATUS_INIT();

  //     if (port_vlan_table.GetPortVlanEntries().isEmpty()) {
  //       qDebug() << __func__ << QString("Device(%1) port_vlan_table is empty").arg(device_ip_).toStdString().c_str();
  //       return act_status;
  //     }

  //     try {
  //       // Create dto request_body
  //       auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
  //       std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;

  //       QMap<quint16, QMap<quint16, QList<qint64>>> pvid_type_ports_map;  // <PVID, QMap<PortType , QList<Port>>>

  //       // Get GetMxVlan to get port type
  //       auto get_response = client_->DoGetMxVlan(token_.toStdString());
  //       // Check success
  //       act_status = CheckResponseStatus(__func__, get_response);
  //       if (!IsActStatusSuccess(act_status)) {
  //         qDebug() << __func__ << QString("Device(%1) reply failed.").arg(device_ip_).toStdString().c_str();
  //         return act_status;
  //       }

  //       // Transfer response to JSON object
  //       QString response_body = get_response->readBodyToString()->c_str();
  //       response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
  //       // Handle data
  //       ActClientMxVlan client_mx_vlan;
  //       client_mx_vlan.FromString(QString("{%1}").arg(response_body));
  //       // Aggregate the port PVID & PortType. Agent need these data(pvid & vlanPortType)
  //       for (auto port_vlan_entry : port_vlan_table.GetPortVlanEntries()) {
  //         auto pvid = port_vlan_entry.GetPVID();
  //         auto port_id = port_vlan_entry.GetPortId();
  //         qint64 port_index = port_id - 1;
  //         auto port_type = client_mx_vlan.GetportTable()[port_index].GetvlanPortType();

  //         if (pvid_type_ports_map.contains(pvid)) {
  //           if (pvid_type_ports_map[pvid].contains(port_type)) {
  //             pvid_type_ports_map[pvid][port_type].append(port_id);
  //           } else {
  //             pvid_type_ports_map[pvid][port_type] = {port_id};
  //           }

  //         } else {
  //           QMap<quint16, QList<qint64>> type_ports_map;
  //           type_ports_map[port_type] = {port_id};
  //           pvid_type_ports_map[pvid] = type_ports_map;
  //         }
  //       }

  //       // Send Agent request (pvid & vlanPortType)
  //       for (auto pvid : pvid_type_ports_map.keys()) {
  //         for (auto port_type : pvid_type_ports_map[pvid].keys()) {
  //           auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
  //           request_dto["pvid"] = oatpp::UInt16(pvid);
  //           request_dto["vlanPortType"] = oatpp::UInt8(port_type);

  //           QList<qint64> ports = pvid_type_ports_map[pvid][port_type];

  //           // Sort
  //           std::sort(ports.begin(), ports.end());

  //           // Convert QList<quint64> to QStringList
  //           QStringList string_list;
  //           for (auto port : ports) {
  //             auto port_index = port - 1;
  //             string_list << QString::number(port_index);
  //           }
  //           // Join the QStringList with "," to create a single QString
  //           QString ports_index_str = string_list.join(",");

  //           // Send delete request to DUT
  //           response = client_->DoPostVlanPortConfig(ports_index_str.toStdString().c_str(), "true",
  //           token_.toStdString(),
  //                                                    request_dto);  // send request
  //           qDebug() << __func__
  //                    << QString("Device(%1) VLAN PVID by agent. Ports_index: %2, Request body: %3")
  //                           .arg(device_ip_)
  //                           .arg(ports_index_str)
  //                           .arg(object_mapper->writeToString(request_dto)->c_str())
  //                           .toStdString()
  //                           .c_str();
  //           act_status = CheckResponseStatus(__func__, response);
  //           if (!IsActStatusSuccess(act_status)) {
  //             qDebug() << __func__
  //                      << QString("Device(%1) reply failed. Request body: %2")
  //                             .arg(device_ip_)
  //                             .arg(object_mapper->writeToString(request_dto)->c_str())
  //                             .toStdString()
  //                             .c_str();
  //             return act_status;
  //           }
  //         }
  //       }

  //     } catch (std::exception &e) {
  //       qCritical() << __func__ << "failed. Error:" << e.what();
  //       return std::make_shared<ActStatusInternalError>("RESTful");
  //     }
  //     return act_status;
  //   }

  ACT_STATUS SetMxVlanPortType(const ActVlanPortTypeTable &vlan_port_type_table) {
    ACT_STATUS_INIT();

    if (vlan_port_type_table.GetVlanPortTypeEntries().isEmpty()) {
      qDebug() << __func__ << QString("Device(%1) vlan_port_type_table is empty").arg(device_ip_).toStdString().c_str();
      return act_status;
    }

    try {
      // Create dto request_body
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;

      // Aggregate the port type
      QList<qint64> access_ports;
      QList<qint64> trunk_ports;
      QList<qint64> hybrid_ports;
      for (auto vlan_port_type_entry : vlan_port_type_table.GetVlanPortTypeEntries()) {
        if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kAccess) {
          access_ports.append(vlan_port_type_entry.GetPortId());
        } else if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kTrunk) {
          trunk_ports.append(vlan_port_type_entry.GetPortId());
        } else if (vlan_port_type_entry.GetVlanPortType() == ActVlanPortTypeEnum::kHybrid) {
          hybrid_ports.append(vlan_port_type_entry.GetPortId());
        }
      }

      // Send Access type request
      if (!access_ports.isEmpty()) {
        auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
        request_dto["vlanPortType"] = oatpp::UInt8(1);  // Access(1)
        // Sort
        std::sort(access_ports.begin(), access_ports.end());

        // Convert QList<quint64> to QStringList
        QStringList string_list;
        for (auto port : access_ports) {
          auto port_index = port - 1;
          string_list << QString::number(port_index);
        }
        // Join the QStringList with "," to create a single QString
        QString ports_index_str = string_list.join(",");
        // Send delete request to DUT
        response = client_->DoPostVlanPortConfig(ports_index_str.toStdString().c_str(), "true", token_.toStdString(),
                                                 request_dto);  // send request
        act_status = CheckResponseStatus(__func__, response);
        if (!IsActStatusSuccess(act_status)) {
          qDebug() << __func__
                   << QString("Device(%1) reply failed. Request body: %2")
                          .arg(device_ip_)
                          .arg(object_mapper->writeToString(request_dto)->c_str())
                          .toStdString()
                          .c_str();
          return act_status;
        }
      }

      // Send Trunk type request
      if (!trunk_ports.isEmpty()) {
        auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
        request_dto["vlanPortType"] = oatpp::UInt8(2);  // Trunk(2)
        // Sort
        std::sort(trunk_ports.begin(), trunk_ports.end());

        // Convert QList<quint64> to QStringList
        QStringList string_list;
        for (auto port : trunk_ports) {
          auto port_index = port - 1;
          string_list << QString::number(port_index);
        }
        // Join the QStringList with "," to create a single QString
        QString ports_index_str = string_list.join(",");
        // Send delete request to DUT
        response = client_->DoPostVlanPortConfig(ports_index_str.toStdString().c_str(), "true", token_.toStdString(),
                                                 request_dto);  // send request

        qDebug() << __func__
                 << QString("Device(%1) VLAN port Type(Trunk(2))). Ports_index: %2, Request body: %3")
                        .arg(device_ip_)
                        .arg(ports_index_str)
                        .arg(object_mapper->writeToString(request_dto)->c_str())
                        .toStdString()
                        .c_str();

        act_status = CheckResponseStatus(__func__, response);
        if (!IsActStatusSuccess(act_status)) {
          qDebug() << __func__
                   << QString("Device(%1) reply failed. Request body: %2")
                          .arg(device_ip_)
                          .arg(object_mapper->writeToString(request_dto)->c_str())
                          .toStdString()
                          .c_str();
          return act_status;
        }
      }

      // Send Hybrid type request
      if (!hybrid_ports.isEmpty()) {
        auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
        request_dto["vlanPortType"] = oatpp::UInt8(3);  // Hybrid(3)
        // Sort
        std::sort(hybrid_ports.begin(), hybrid_ports.end());

        // Convert QList<quint64> to QStringList
        QStringList string_list;
        for (auto port : hybrid_ports) {
          auto port_index = port - 1;
          string_list << QString::number(port_index);
        }
        // Join the QStringList with "," to create a single QString
        QString ports_index_str = string_list.join(",");
        // Send delete request to DUT
        response = client_->DoPostVlanPortConfig(ports_index_str.toStdString().c_str(), "true", token_.toStdString(),
                                                 request_dto);  // send request
        act_status = CheckResponseStatus(__func__, response);

        qDebug() << __func__
                 << QString("Device(%1) VLAN port Type(Hybrid(3)). Ports_index: %2, Request body: %3")
                        .arg(device_ip_)
                        .arg(ports_index_str)
                        .arg(object_mapper->writeToString(request_dto)->c_str())
                        .toStdString()
                        .c_str();

        if (!IsActStatusSuccess(act_status)) {
          qDebug() << __func__
                   << QString("Device(%1) reply failed. Request body: %2")
                          .arg(device_ip_)
                          .arg(object_mapper->writeToString(request_dto)->c_str())
                          .toStdString()
                          .c_str();
          return act_status;
        }
      }

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

  ACT_STATUS SetDefaultPriority(const ActDefaultPriorityTable &default_priority_table) {
    ACT_STATUS_INIT();

    if (default_priority_table.GetDefaultPriorityEntries().isEmpty()) {
      qDebug() << __func__
               << QString("Device(%1) default_priority_table is empty").arg(device_ip_).toStdString().c_str();
      return act_status;
    }

    try {
      // Create dto request_body
      auto port_list_dto = oatpp::List<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;

      // Get Dut defaultPriorityTable to get number of ports
      auto get_response = client_->DoGetMxQos(token_.toStdString());
      // Check success
      act_status = CheckResponseStatus(__func__, get_response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__ << QString("Device(%1) reply failed.").arg(device_ip_).toStdString().c_str();
        return act_status;
      }

      // Transfer response to JSON object
      QString response_body = get_response->readBodyToString()->c_str();
      response_body = response_body.mid(1, response_body.length() - 2);  // "{"abc": true}" -> {"abc": true}
      // Handle data
      QJsonArray json_pcp_array;
      ActClientMxQos mx_qos;
      mx_qos.FromString(QString("{%1}").arg(response_body));
      for (qint64 index = 0; index < mx_qos.GetdefaultPriorityTable().size(); index++) {
        qint64 port_id = index + 1;
        ActDefaultPriorityEntry default_priority_entry;
        default_priority_entry.SetPortId(port_id);
        auto iter = default_priority_table.GetDefaultPriorityEntries().find(default_priority_entry);
        if (iter != default_priority_table.GetDefaultPriorityEntries().end()) {  // found, modify value
          mx_qos.GetdefaultPriorityTable()[index].SetdefaultPriorityValue(iter->GetDefaultPCP());
        }

        // Append to JSON array
        QJsonDocument json_doc_pcp_port =
            QJsonDocument::fromJson(mx_qos.GetdefaultPriorityTable()[index].ToString().toUtf8());
        if (!json_doc_pcp_port.isNull() && json_doc_pcp_port.isObject()) {
          json_pcp_array.append(json_doc_pcp_port.object());
        }
      }

      // Not empty would set Default PCP
      if (!json_pcp_array.isEmpty()) {
        // Transfer to JSON string > QString > DTO
        QString modified_json_str = QJsonDocument(json_pcp_array).toJson();

        // Create reuest DTO
        port_list_dto = object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());

        // Send post request to DUT
        response = client_->DoPatchDefaultPriorityTable(token_.toStdString(), port_list_dto);  // send request
                                                                                               // Check success
        act_status = CheckResponseStatus(__func__, response);
        if (!IsActStatusSuccess(act_status)) {
          qDebug() << __func__
                   << QString("Device(%1) reply failed. Request body: %2")
                          .arg(device_ip_)
                          .arg(object_mapper->writeToString(port_list_dto)->c_str())
                          .toStdString()
                          .c_str();
          return act_status;
        }
      }
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

  ACT_STATUS SetMgmtVlan(const qint32 &mgmt_vlan) {
    ACT_STATUS_INIT();

    try {
      // Create dto request_body
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;
      auto vlan_list_dto = oatpp::List<oatpp::Any>::createShared();

      QJsonArray json_vlan_array;
      json_vlan_array.append(mgmt_vlan);

      // Transfer to JSON string > QString > DTO
      QString modified_json_str = QJsonDocument(json_vlan_array).toJson();

      // Create reuest DTO
      vlan_list_dto = object_mapper->readFromString<oatpp::List<oatpp::Any>>(modified_json_str.toStdString().c_str());

      // Send post request to DUT
      response = client_->DoPatchMgmtVlan(token_.toStdString(), vlan_list_dto);  // send request
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__
                 << QString("Device(%1) reply failed. Request body: %2")
                        .arg(device_ip_)
                        .arg(object_mapper->writeToString(vlan_list_dto)->c_str())
                        .toStdString()
                        .c_str();
        return act_status;
      }
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

  ACT_STATUS PostImportConfig(const QString &file_path) {
    ACT_STATUS_INIT();
    try {
      // file_path: xx/xx/192.168.127.2_TSN-G5004_202411221513.ini
      QFile file(file_path);
      if (!file.open(QIODevice::ReadOnly)) {
        QString error_msg = QString("Open Config file(ini) %1 failed").arg(file.fileName());
        qCritical() << __func__ << error_msg.toStdString().c_str();
        return std::make_shared<ActStatusNotFound>(error_msg);
      }
      QString file_str = QString::fromUtf8(file.readAll());
      file.close();

      ActClientImportConfigRequest client_import_request;

      auto multipart = oatpp::web::mime::multipart::PartList::createSharedWithRandomBoundary();

      // file
      QFileInfo file_info(file.fileName());
      QString file_name = file_info.fileName();
      oatpp::web::mime::multipart::Headers partHeaders;
      auto part1 = std::make_shared<oatpp::web::mime::multipart::Part>(partHeaders);
      multipart->writeNextPartSimple(part1);
      part1->putHeader("Content-Disposition",
                       oatpp::String(QString("form-data; name=\"file\";filename=\"%1\"").arg(file_name).toStdString()));
      part1->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(oatpp::String(file_str.toStdString())));

      // request
      auto part2 = std::make_shared<oatpp::web::mime::multipart::Part>(partHeaders);
      multipart->writeNextPartSimple(part2);
      part2->putHeader("Content-Disposition", "form-data; name=\"request\"");
      part2->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(client_import_request.ToString().toStdString())));

      auto multiple_part_body = std::make_shared<oatpp::web::protocol::http::outgoing::MultipartBody>(multipart);

      auto response = client_->DoPostImportConfig(token_.toStdString(), multiple_part_body);
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        qDebug() << __func__
                 << QString("Device(%1) reply failed. Error:%2")
                        .arg(device_ip_)
                        .arg(act_status->GetErrorMessage())
                        .toStdString()
                        .c_str();
        return act_status;
      }

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS PostExportConfig(const QString &file_path) {
    ACT_STATUS_INIT();

    try {
      // Create dto request_body
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      ActClientExportConfigRequest client_export_request;
      request_dto = object_mapper->readFromString<oatpp::Fields<oatpp::Any>>(
          client_export_request.ToString().toStdString().c_str());

      // Send post request to DUT
      auto response = client_->DoPostExportConfig(token_.toStdString(), request_dto);  // send request
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      // Save file
      QFile file(file_path);
      if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "Open device config file failed:" << file.fileName();
        return std::make_shared<ActStatusInternalError>("RESTful(Save File)");
      }
      file.write(response->readBodyToString()->c_str());
      file.close();

      // qDebug() << __func__ << "response_body:" << response_body.toStdString().c_str();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

  // ACT_STATUS PostFirmwareUpgrade(QString &result) {
  //   ACT_STATUS_INIT();
  //   try {
  //     // Create dto request_body
  //     auto fw_upgrade_dto = ActFirmwareUpgradeDto::createShared();
  //     oatpp::data::resource::File file("FWR_TSN-G5000_v2.2_2022_0720_1430.rom");
  //     // auto body_file =
  //     std::make_shared<oatpp::web::protocol::http::outgoing::StreamingBody>(file.openInputStream());

  //     auto multipart = oatpp::web::mime::multipart::PartList::createSharedWithRandomBoundary();
  //     // oatpp::web::mime::multipart::Headers partHeaders;
  //     // auto part = std::make_shared<oatpp::web::mime::multipart::Part>(partHeaders);
  //     // // multipart->writeNextPartSimple(part);
  //     // // part->putHeader("Content-Type", "multipart/form-data;");
  //     // // part->setPayload(std::make_shared<oatpp::data::resource::File>(file));

  //     // oatpp::web::mime::multipart::FileProvider file_provider("FWR_TSN-G5000_v2.2_2022_0720_1430.rom");
  //     // part->setPayload(std::make_shared<oatpp::data::resource::Resource>(file_provider.getResource()));
  //     // sample
  //     // std::unordered_map<oatpp::String, oatpp::> map;
  //     // map["value1"] = "Hello";
  //     // map["value2"] = "World";
  //     // // "{"file_parameter": {}}"
  //     // for (auto &pair : map) {
  //     //   oatpp::web::mime::multipart::Headers partHeaders;
  //     //   auto part = std::make_shared<oatpp::web::mime::multipart::Part>(partHeaders);
  //     //   multipart->writeNextPartSimple(part);
  //     //   part->putHeader("Content-Disposition", "form-data; name=\"" + pair.first + "\"");
  //     //   part->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(pair.second));
  //     // }

  //     // file
  //     oatpp::web::mime::multipart::Headers partHeaders;
  //     auto part1 = std::make_shared<oatpp::web::mime::multipart::Part>(partHeaders);
  //     multipart->writeNextPartSimple(part1);
  //     part1->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("file") + "\"");
  //     part1->setPayload(std::make_shared<oatpp::data::resource::File>(file));

  //     // request
  //     auto part2 = std::make_shared<oatpp::web::mime::multipart::Part>(partHeaders);
  //     multipart->writeNextPartSimple(part2);
  //     part2->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("request") + "\"");
  //     part2->setPayload(
  //         std::make_shared<oatpp::data::resource::InMemoryData>(oatpp::String("{\"file_parameter\": {}}")));

  //     auto body_file = std::make_shared<oatpp::web::protocol::http::outgoing::MultipartBody>(multipart);
  //     // oatpp/web/protocol/http/outgoing/MultipartBod
  //     // auto multipart = oatpp::web::mime::multipart::PartList::createSharedWithRandomBoundary();
  //     // std::shared_ptr<MultipartBody>
  //     std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;
  //     // response = client_->DoPostFirmwareUpgrade(token_.toStdString(), body_file, fw_upgrade_dto);
  //     response = client_->DoPostFirmwareUpgrade(token_.toStdString(), body_file);
  //     // Check success
  //     auto response_status_code = response->getStatusCode();
  //     if (response_status_code != ACT_RESTFUL_CLIENT_REQUEST_SUCCESS) {
  //       qCritical() << __func__ << "Response Status(" << response_status_code
  //                   << "):" << response->getStatusDescription()->c_str();
  //       return std::make_shared<ActStatusInternalError>("RESTful");
  //     }

  //     QString response_body = response->readBodyToString()->c_str();
  //     qDebug() << __func__ << "Response body:" << response_body.toStdString().c_str();

  //   } catch (std::exception &e) {
  //     qCritical() << __func__ << "failed. Error:" << e.what();
  //     return std::make_shared<ActStatusInternalError>("RESTful");
  //   }

  //   return act_status;
  // }
};

#endif /* ACT_MOXA_IEI_CLIENT_AGENT_HPP */