/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#ifndef ACT_MOXA_IEI_CLIENT_HPP
#define ACT_MOXA_IEI_CLIENT_HPP

#include <QString>

#include "act_json.hpp"
#include "dto/act_moxa_iei_dto.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/web/client/ApiClient.hpp"
#include "oatpp/web/protocol/http/outgoing/MultipartBody.hpp"
// #include "oatpp/web/protocol/http/outgoing/StreamingBody.hpp"

/**
 * @brief The restful client class for MOXA tsn switch
 *
 */
class ActMoxaIEIClient : public oatpp::web::client::ApiClient {
#include OATPP_CODEGEN_BEGIN(ApiClient)

  API_CLIENT_INIT(ActMoxaIEIClient)

  API_CALL("GET", "static/modelName", DoGetModelName)

  API_CALL("GET", "static/loginMessage", DoGetLoginMessage)

  API_CALL("GET", "api/v1/status/systemInformation/serialNumber", DoGetSerialNumber,
           HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/systemInformation/uptime", DoGetUptime, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/systemInformation/productRevision", DoGetProductRevision,
           HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/systemInformation/deviceName", DoGetDeviceName,
           HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/systemInformation/deviceLocation", DoGetDeviceLocation,
           HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/systemInformation/deviceDescription", DoGetDeviceDescription,
           HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/systemInformation/contactInformation", DoGetContactInformation,
           HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/systemInformation/modules", DoGetModules, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/managementIp/ipv4/netmask", DoGetNetmask, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/managementIp/ipv4/gateway", DoGetGateway, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/managementIp/ipv4", DoGetIpv4, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/l3RouterId/ipv4", DoGetL3RouterId,
           HEADER(String, token, "Authorization"))  // L3 switch

  API_CALL("GET", "api/v1/setting/data/networkDns/ipv4", DoGetL3NetworkDns,
           HEADER(String, token, "Authorization"))  // L3 switch；

  API_CALL("GET", "api/v1/setting/data/systemInformation", DoGetSystemInformationSetting,
           HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/uiServiceManagement", DoGetServiceManagement,
           HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/userAccount", DoGetUserAccount, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/loginPolicy", DoGetLoginPolicy, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/snmpTrap", DoGetSnmpTrap, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/syslogServer", DoGetSyslogServer, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/time", DoGetTime, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/time", DoGetTimeStatus, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/mxlp", DoGetMxLp, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/SyncInvalid", DoGetConfigurationSyncStatus, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/systemInformation", DoGetSystemInformation, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/systemUtilization", DoGetSystemUtilization, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/ifmib", DoGetPortSettingAdminStatus, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/portStatus", DoGetPortStatus, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/portInfo", DoGetPortInfo, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/fiberCheckStatus/monitor", DoGetFiberCheckStatus,
           HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/trafficStatistics", DoGetTrafficStatistics, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/1588DefaultInfo", DoGet1588DefaultInfo, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/dot1asInfo", DoGetDot1asInfo, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/logEntry", DoGetLogEntry, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/mxL2Redundancy", DoGetMxL2Redundancy, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/std1w1ap", DoGetRstp, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/std1w1ap/portTable", DoGetRstpPortTable, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/std1d1ap", DoGetStp, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/std1d1ap/portTable", DoGetStpPortTable, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/mxptp", DoGetMxPtp, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/stdot1as", DoGetStDot1as, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/mx1588Default", DoGetMx1588Default, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/mx1588Iec61850", DoGetMx1588Iec61850, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/mx1588C37238", DoGetMx1588C37238, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/mxrstp", DoGetMxRstp, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/mxrstp/portTable", DoGetMxRstpPortTable, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/status/rstpStatus", DoGetRstpStatus, HEADER(String, token, "Authorization"))

  API_CALL("GET", "api/v1/setting/data/streamadapter", DoGetStreamAdapter, HEADER(String, token, "Authorization"))

  API_CALL("POST", "api/v1/auth/login", DoPostLogin, BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("POST", "api/v1/auth/logout", DoPostLogout, HEADER(String, token, "Authorization"))

  API_CALL("POST", "api/v1/auth/heartbeat", DoPostHeartbeat, HEADER(String, token, "Authorization"))

  API_CALL("POST", "api/v1/command/reboot", DoPostReboot, HEADER(String, token, "Authorization"))

  API_CALL("POST", "api/v1/command/factoryDefault", DoPostFactoryDefault, HEADER(String, token, "Authorization"))

  API_CALL("POST", "api/v1/command/time", DoPostTimeCommand, HEADER(String, token, "Authorization"),
           BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("POST", "api/v1/file/import/prestart", DoPostPreStartImport, HEADER(String, token, "Authorization"))

  API_CALL("POST", "api/v1/command/locator", DoPostLocator, HEADER(String, token, "Authorization"),
           BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("POST", "api/v1/file/import/http/cli/cli.conf", DoPostImportConfig, HEADER(String, token, "Authorization"),
           BODY(std::shared_ptr<oatpp::web::protocol::http::outgoing::MultipartBody>, multiple_part_body))

  API_CALL("POST", "api/v1/file/export/http/cli/cli.conf", DoPostExportConfig, HEADER(String, token, "Authorization"),
           BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/userAccount?save", DoPatchUserAccount, HEADER(String, token, "Authorization"),
           BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("DELETE", "api/v1/setting/data/userAccount/{account}", DoDeleteUserAccount, PATH(String, account),
           QUERY(String, save), HEADER(String, token, "Authorization"))

  API_CALL("PATCH", "api/v1/setting/data/uiServiceManagement?save", DoPatchServiceManagement,
           HEADER(String, token, "Authorization"), BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/uiServiceManagement/snmpService?save", DoPatchSnmpService,
           HEADER(String, token, "Authorization"), BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/loginPolicy?save", DoPatchLoginPolicy, HEADER(String, token, "Authorization"),
           BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/snmpTrap?save", DoPatchSnmpTrap, HEADER(String, token, "Authorization"),
           BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/syslogServer?save", DoPatchSyslogServer,
           HEADER(String, token, "Authorization"), BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/time?save", DoPatchTime, HEADER(String, token, "Authorization"),
           BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/mxlp?save", DoPatchMxLp, HEADER(String, token, "Authorization"),
           BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/systemInformation?save", DoPatchSystemInformationSetting,
           HEADER(String, token, "Authorization"), BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/managementIp/ipv4?save", DoPatchManagementIpIpv4,
           HEADER(String, token, "Authorization"), BODY_DTO(Object<ActManagementIpIpv4Dto>, body))

  API_CALL("PATCH", "api/v1/setting/data/ifmib?save", DoPatchPortSettingAdminStatus,
           HEADER(String, token, "Authorization"), BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/mxL2Redundancy?save", DoPatchMxL2Redundancy,
           HEADER(String, token, "Authorization"), BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/std1w1ap?save", DoPatchRstp, HEADER(String, token, "Authorization"),
           BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/std1d1ap?save", DoPatchStp, HEADER(String, token, "Authorization"),
           BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/mxrstp?save", DoPatchMxRstp, HEADER(String, token, "Authorization"),
           BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/streamadapter?save", DoPatchStreamAdapter,
           HEADER(String, token, "Authorization"), BODY_DTO(oatpp::Fields<oatpp::Any>, body))
  // ManagementVLAN
  API_CALL("PATCH", "api/v1/setting/data/mxvlan/mgmtVlan?save", DoPatchMgmtVlan, HEADER(String, token, "Authorization"),
           BODY_DTO(oatpp::List<oatpp::Any>, body))
  API_CALL("GET", "api/v1/setting/data/mxvlan/mgmtVlan", DoGetMgmtVlan, HEADER(String, token, "Authorization"))

  // Default PCP
  API_CALL("GET", "api/v1/setting/data/mxqos", DoGetMxQos, HEADER(String, token, "Authorization"))

  API_CALL("PATCH", "api/v1/setting/data/mxqos/defaultPriorityTable?save", DoPatchDefaultPriorityTable,
           HEADER(String, token, "Authorization"), BODY_DTO(oatpp::List<oatpp::Any>, body))

  // TEMSTID
  API_CALL("PATCH", "api/v1/setting/data/temstid/temstidTable?save", DoPatchTEMSTIDTable,
           HEADER(String, token, "Authorization"), BODY_DTO(oatpp::List<oatpp::Any>, body))
  API_CALL("GET", "api/v1/setting/data/temstid/temstidTable", DoGetTEMSTIDTable, HEADER(String, token, "Authorization"))

  // VLAN
  API_CALL("GET", "api/v1/setting/data/stdvlan", DoGetStdVlan, HEADER(String, token, "Authorization"))

  API_CALL("POST", "api/v1/setting/agent/vlan/vlanIds?save", DoPostAddVlan, HEADER(String, token, "Authorization"),
           BODY_DTO(oatpp::List<oatpp::Any>, body))

  API_CALL("PATCH", "api/v1/setting/data/stdvlan/vlanTable?save", DoPatchSetVlan,
           HEADER(String, token, "Authorization"), BODY_DTO(oatpp::List<oatpp::Any>, body))

  API_CALL("DELETE", "api/v1/setting/agent/vlan/vlanIds", DoDeleteVlan, QUERY(String, vids), QUERY(String, save),
           HEADER(String, token, "Authorization"))

  // MX VLAN
  API_CALL("GET", "api/v1/setting/data/mxvlan", DoGetMxVlan, HEADER(String, token, "Authorization"))

  // Set VLAN Port type & PVID by agent
  API_CALL("POST", "api/v1/setting/agent/vlan/portConfigs?", DoPostVlanPortConfig, QUERY(String, ifindices),
           QUERY(String, save), HEADER(String, token, "Authorization"), BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  // Set VLAN Port type
  // API_CALL("PATCH", "api/v1/setting/data/mxvlan/portTable?save", DoPatchMxVlanPortTable,
  //          HEADER(String, token, "Authorization"), BODY_DTO(oatpp::List<oatpp::Any>, body))

  // Set VLAN PVID
  // API_CALL("PATCH", "api/v1/setting/data/stdvlan/portTable?save", DoPatchStdVlanPortTable,
  //          HEADER(String, token, "Authorization"), BODY_DTO(oatpp::List<oatpp::Any>, body))

  // API_CALL("POST", "api/v1/file/import/http/system/firmware.rom", DoPostFirmwareUpgrade,
  //          HEADER(String, token, "Authorization"),
  //          BODY(std::shared_ptr<oatpp::web::protocol::http::outgoing::MultipartBody>, file))

  // API_CALL_HEADERS(DoPatchManagementIpIpv4Async) { headers.put("Host", "192.168.127.253"); }
  // API_CALL_ASYNC("PATCH", "api/v1/setting/data/managementIp/ipv4?save", DoPatchManagementIpIpv4Async,
  //                HEADER(String, token, "Authorization"),
  //                BODY_DTO(Object<ActManagementIpIpv4Dto>, body))

  // ?save -> save to startup-config

#include OATPP_CODEGEN_END(ApiClient)
};

/**
 * @brief The restful's login request body class
 *
 */
class ActClientLoginRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, username, username);
  ACT_JSON_FIELD(QString, password, password);

 public:
  /**
   * @brief Construct a new Act Client Login Request object
   *
   */
  ActClientLoginRequest() : username_("admin"), password_("moxa") {};

  /**
   * @brief Construct a new Act Client Login Request object
   *
   * @param username
   * @param password
   */
  ActClientLoginRequest(const QString &username, const QString &password) : username_(username), password_(password) {};
};

/**
 * @brief The restful's login response body class
 *
 */
class ActClientLoginResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, access_token, access_token);
  ACT_JSON_FIELD(QString, result, result);
};

/**
 * @brief The restful's SnmpService body class
 *
 */
class ActClientSnmpService : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, mode, mode);
  // 1(Enabled), 2(Disabled), 3(ReadOnly)

  ACT_JSON_FIELD(quint16, port, port);
  ACT_JSON_FIELD(quint16, transport_layer_protocol, transportLayerProtocol);

 public:
  /**
   * @brief Construct a new Act Client Snmp Service object
   *
   */
  ActClientSnmpService() : mode_(1), port_(161), transport_layer_protocol_(1) {};
};

/**
 * @brief The restful's ManagementIp Ipv4 body class
 *
 */
class ActClientManagementIpIpv4 : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, network_setting_mode, networkSettingMode);
  ACT_JSON_FIELD(QString, ip_address, ipAddress);
  ACT_JSON_FIELD(QString, netmask, netmask);
  ACT_JSON_FIELD(QString, gateway, gateway);

 public:
  /**
   * @brief Construct a new Act Client Snmp Service object
   *
   */
  ActClientManagementIpIpv4() : network_setting_mode_("Manual"), ip_address_(""), netmask_(""), gateway_("") {};
};

class ActClientSystemInformation : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, device_name, deviceName);
  ACT_JSON_FIELD(QString, device_location, deviceLocation);
  ACT_JSON_FIELD(QString, device_description, deviceDescription);
  ACT_JSON_FIELD(QString, contact_information, contactInformation);

 public:
  /**
   * @brief Construct a new Act Client System Information object
   *
   */
  ActClientSystemInformation() {
    this->device_name_ = "";
    this->device_location_ = "";
    this->device_description_ = "";
    this->contact_information_ = "";
  };
};

class ActClientEthernetModule : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, module_name, moduleName);
  ACT_JSON_FIELD(QString, serial_number, serialNumber);
  ACT_JSON_FIELD(QString, product_revision, productRevision);
  ACT_JSON_FIELD(QString, status, status);
  ACT_JSON_FIELD(qint64, module_id, moduleId);

 public:
  /**
   * @brief Construct a new Act Client Ethernet Module object
   *
   */
  ActClientEthernetModule() {
    this->module_name_ = "";
    this->serial_number_ = "";
    this->product_revision_ = "";
    this->status_ = "";
    this->module_id_ = -1;
  };
};

class ActClientPowerModule : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, module_name, moduleName);
  ACT_JSON_FIELD(QString, serial_number, serialNumber);
  ACT_JSON_FIELD(QString, product_revision, productRevision);
  ACT_JSON_FIELD(QString, status, status);

 public:
  /**
   * @brief Construct a new Act Client Ethernet Module object
   *
   */
  ActClientPowerModule() {
    this->module_name_ = "";
    this->serial_number_ = "";
    this->product_revision_ = "";
    this->status_ = "";
  };
};

class ActClientModules : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientEthernetModule, ethernet, ethernet);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientPowerModule, power, power);

 public:
  /**
   * @brief Construct a new Act Client Spanning Tree Port Table object
   *
   */
  ActClientModules() {};
};

class ActClientRstpPortStatusEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, bpdu_inconsistency, bpduInconsistency);
  ACT_JSON_FIELD(qint64, designated_cost, designatedCost);
  ACT_JSON_FIELD(bool, edge_port, edgePort);
  ACT_JSON_FIELD(bool, loop_inconsistency, loopInconsistency);
  ACT_JSON_FIELD(bool, oper_bridge_link_type, operBridgeLinkType);
  ACT_JSON_FIELD(qint64, path_cost, pathCost);
  ACT_JSON_FIELD(qint64, port_state, portState);
  ACT_JSON_FIELD(bool, root_inconsistency, rootInconsistency);
  ACT_JSON_FIELD(qint64, rstp_port_role, rstpPortRole);

 public:
  /**
   * @brief Construct a new Act Client Snmp Service object
   *
   */
  ActClientRstpPortStatusEntry() {
    this->bpdu_inconsistency_ = false;
    this->designated_cost_ = 0;
    this->edge_port_ = false;
    this->loop_inconsistency_ = false;
    this->oper_bridge_link_type_ = false;
    this->path_cost_ = 0;
    this->port_state_ = 2;
    this->root_inconsistency_ = false;
    this->rstp_port_role_ = 0;
  };
};

class ActClientRstpStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, designated_root, designatedRoot);
  ACT_JSON_FIELD(qint64, forward_delay,
                 forwardDelay);  ///< The forwardDelay(sec) of the device
  ACT_JSON_FIELD(qint64, hello_time,
                 helloTime);  ///< The hello time(sec) of the device
  ACT_JSON_FIELD(qint64, max_age,
                 maxAge);  ///< The maxAge(sec) of the device
  ACT_JSON_FIELD(qint64, root_cost, rootCost);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientRstpPortStatusEntry, port_table, portTable);

 public:
  /**
   * @brief Construct a new Act Client Spanning Tree Port Table object
   *
   */
  ActClientRstpStatus() {
    this->designated_root_ = "";
    this->forward_delay_ = 0;
    this->hello_time_ = 0;
    this->max_age_ = 0;
    this->root_cost_ = 0;
  };
};

class ActClientRstpPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, force_edge, forceEdge);
  ACT_JSON_FIELD(qint64, path_cost, pathCost);
  ACT_JSON_FIELD(qint64, port_priority, portPriority);
  ACT_JSON_FIELD(bool, rstp_enable, rstpEnable);

 public:
  /**
   * @brief Construct a new Act Client Rstp Port Entry object
   *
   */
  ActClientRstpPortEntry() {
    this->force_edge_ = false;
    this->path_cost_ = 0;
    this->port_priority_ = 128;
    this->rstp_enable_ = true;
  };
};

class ActClientMxL2Redundancy : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, stp_rstp, stprstp);
  ACT_JSON_FIELD(bool, turboring_v2, turboringv2);
  ACT_JSON_FIELD(bool, turbo_chain, turbochain);
  ACT_JSON_FIELD(bool, dual_homing, dualhoming);
  ACT_JSON_FIELD(bool, mstp, mstp);
  ACT_JSON_FIELD(bool, iec62439_2, iec62439_2);

 public:
  /**
   * @brief Construct a new Act Client MxL2Redundancy object
   *
   */
  ActClientMxL2Redundancy() {
    this->stp_rstp_ = false;
    this->turboring_v2_ = false;
    this->turbo_chain_ = false;
    this->dual_homing_ = false;
    this->mstp_ = false;
    this->iec62439_2_ = false;
  };
};

class ActClientRstp : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, spanning_tree_version, spanningTreeVersion);
  ACT_JSON_FIELD(qint64, priority, priority);
  ACT_JSON_FIELD(qint64, max_age, maxAge);
  ACT_JSON_FIELD(qint64, hello_time, helloTime);
  ACT_JSON_FIELD(qint64, forward_delay, forwardDelay);
  ACT_JSON_FIELD(qint64, rstp_tx_hold_count, rstpTxHoldCount);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientRstpPortEntry, port_table, portTable);

 public:
  /**
   * @brief Construct a new Act Client Rstp object
   *
   */
  ActClientRstp() {
    this->spanning_tree_version_ = 2;
    this->priority_ = 32768;
    this->max_age_ = 2000;        //  ActRstpTable.max_age = 2000 / 100
    this->hello_time_ = 200;      //  ActRstpTable.hello_time = 200 / 100
    this->forward_delay_ = 1500;  //  ActRstpTable.forward_delay = 1500 / 100
    this->rstp_tx_hold_count_ = 6;
  };
};

class ActClientStpPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, bridge_link_type, bridgeLinkType);

 public:
  /**
   * @brief Construct a new Act Client Stp Port Entry object
   *
   */
  ActClientStpPortEntry() { this->bridge_link_type_ = 3; };
};

class ActClientStp : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientStpPortEntry, port_table, portTable);

 public:
  /**
   * @brief Construct a new Act Client Stp object
   *
   */
  ActClientStp() {};
};

class ActClientMxRstpPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, auto_edge, autoEdge);
  ACT_JSON_FIELD(bool, bpdu_guard, bpduGuard);
  ACT_JSON_FIELD(bool, root_guard, rootGuard);
  ACT_JSON_FIELD(bool, loop_guard, loopGuard);
  ACT_JSON_FIELD(bool, bpdu_filter, bpduFilter);

 public:
  /**
   * @brief Construct a new Act Client Mx Rstp Port Entry object
   *
   */
  ActClientMxRstpPortEntry() {
    this->auto_edge_ = true;
    this->bpdu_guard_ = false;
    this->root_guard_ = false;
    this->loop_guard_ = false;
    this->bpdu_filter_ = false;
  };
};

class ActClientMxRstp : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, rstp_error_recovery_time,
                 rstpErrorRecoveryTime);  //  ActRstpTable.rstp_error_recovery_time = 30000 / 100

  // swift
  ACT_JSON_FIELD(bool, rstp_config_swift, rstpConfigSwift);
  ACT_JSON_FIELD(bool, rstp_config_revert, rstpConfigRevert);

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientMxRstpPortEntry, port_table, portTable);

 public:
  /**
   * @brief Construct a new Act Client Mx Rstp object
   *
   */
  ActClientMxRstp() {
    this->rstp_error_recovery_time_ = 30000;
    this->rstp_config_swift_ = false;
    this->rstp_config_revert_ = false;
  };
};

class ActClientSystemInformationSetting : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, device_name, deviceName);
  ACT_JSON_FIELD(QString, device_location, deviceLocation);
  ACT_JSON_FIELD(QString, device_description, deviceDescription);
  ACT_JSON_FIELD(QString, contact_information, contactInformation);

 public:
  /**
   * @brief Construct a new Act Client SystemInformation Setting object
   *
   */
  ActClientSystemInformationSetting() {
    this->device_name_ = "";
    this->device_location_ = "";
    this->device_description_ = "";
    this->contact_information_ = "";
  };
};

class ActClientMgmtEncryptedMoxaService : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);

 public:
  /**
   * @brief Construct a new Act Client Encrypted Moxa Service object
   *
   */
  ActClientMgmtEncryptedMoxaService() { this->enable_ = true; }
};

class ActClientMgmtHttpService : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_FIELD(quint16, port, port);

 public:
  /**
   * @brief Construct a new Act Client Management Http Service object
   *
   */
  ActClientMgmtHttpService() {
    this->enable_ = true;
    this->port_ = 80;
  }
};

class ActClientMgmtHttpsService : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_FIELD(quint16, port, port);

 public:
  /**
   * @brief Construct a new Act Client Management Http Service object
   *
   */
  ActClientMgmtHttpsService() {
    this->enable_ = true;
    this->port_ = 443;
  }
};

class ActClientMgmtSnmpService : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, mode, mode);
  ACT_JSON_FIELD(quint16, port, port);
  ACT_JSON_FIELD(quint16, transport_layer_protocol, transportLayerProtocol);

 public:
  /**
   * @brief Construct a new Act Client Management Snmp Service object
   *
   */
  ActClientMgmtSnmpService() {
    this->mode_ = 1;
    this->port_ = 161;
    this->transport_layer_protocol_ = 1;
  }
};

class ActClientMgmtSshService : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_FIELD(quint16, port, port);

 public:
  /**
   * @brief Construct a new Act Client Management Ssh Service object
   *
   */
  ActClientMgmtSshService() {
    this->enable_ = true;
    this->port_ = 22;
  }
};

class ActClientMgmtTelnetService : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_FIELD(quint16, port, port);

 public:
  /**
   * @brief Construct a new Act Client Management Telnet Service object
   *
   */
  ActClientMgmtTelnetService() {
    this->enable_ = true;
    this->port_ = 23;
  }
};

class ActClientServiceManagement : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActClientMgmtEncryptedMoxaService, encrypted_moxa_service, encryptedMoxaService);
  ACT_JSON_OBJECT(ActClientMgmtHttpService, http_service, httpService);
  ACT_JSON_OBJECT(ActClientMgmtHttpsService, https_service, httpsService);
  ACT_JSON_OBJECT(ActClientMgmtSnmpService, snmp_service, snmpService);
  ACT_JSON_OBJECT(ActClientMgmtSshService, ssh_service, sshService);
  ACT_JSON_OBJECT(ActClientMgmtTelnetService, telnet_service, telnetService);

  ACT_JSON_FIELD(quint16, http_max_login_sessions, httpMaxLoginSessions);
  ACT_JSON_FIELD(quint16, terminal_max_login_sessions, terminalMaxLoginSessions);

 public:
  /**
   * @brief Construct a new Act Client ServiceManagement object
   *
   */
  ActClientServiceManagement() {
    this->http_max_login_sessions_ = 5;
    this->terminal_max_login_sessions_ = 1;
  };
};

class ActClientUserAccount : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, active, active);
  ACT_JSON_FIELD(QString, user_name, userName);
  ACT_JSON_FIELD(QString, password, password);
  ACT_JSON_FIELD(QString, role, role);
  ACT_JSON_FIELD(QString, email, email);

 public:
  ActClientUserAccount() {
    this->active_ = true;
    this->user_name_ = "";
    this->password_ = "";
    this->role_ = "admin";
    this->email_ = "";
  };
};

class ActClientLoginPolicy : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, web_login_message, webLoginMessage);
  ACT_JSON_FIELD(QString, login_failure_message, loginFailureMessage);
  ACT_JSON_FIELD(bool, enable_failure_lockout, enableFailureLockout);
  ACT_JSON_FIELD(qint32, retry_failure_threshold, retryFailureThreshold);
  ACT_JSON_FIELD(qint32, failure_lockout_time, failureLockoutTime);
  ACT_JSON_FIELD(qint32, auto_logout, autoLogout);

 public:
  ActClientLoginPolicy() {
    this->web_login_message_ = "";
    this->login_failure_message_ = "";
    this->enable_failure_lockout_ = false;
    this->retry_failure_threshold_ = 5;
    this->failure_lockout_time_ = 5;
    this->auto_logout_ = 5;
  };
};

class ActClientStreamAdapterEntryRule : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // base (L2)(V1)
  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_FIELD(quint32, ether_type, ethertype);
  ACT_JSON_FIELD(bool, frame_type, frametype);
  ACT_JSON_FIELD(quint32, frame_type_value, frametypevalue);
  ACT_JSON_FIELD(quint32, pcp, pcp);
  ACT_JSON_FIELD(quint32, vid, vid);

  // L3 (V2): type, tcpPort, udpPort
  ACT_JSON_COLLECTION(QList, QString, type, type);
  ACT_JSON_FIELD(quint32, tcp_port, tcpPort);
  ACT_JSON_FIELD(quint32, udp_port, udpPort);

 public:
  ActClientStreamAdapterEntryRule() {
    this->enable_ = false;
    this->ether_type_ = 0;
    this->frame_type_ = false;
    this->frame_type_value_ = 0;
    this->pcp_ = 0;
    this->vid_ = 1;

    // L3 (V2): type, tcpPort, udpPort
    this->tcp_port_ = 1;
    this->udp_port_ = 1;
  };
};

class ActClientStreamAdapterEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, egress_untag, egressuntag);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientStreamAdapterEntryRule, rule_index, ruleindex);

 public:
  ActClientStreamAdapterEntry() { this->egress_untag_ = false; };
};

class ActClientStreamAdapter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientStreamAdapterEntry, port_table, portTable);
};

class ActClientMxPtpPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, profile, profile);
  // 1: IEEE 802.1AS-2011; 3: IEEE 1588 Default-2008; 4: IEC 61850-9-3-2016; 5: IEEE C37.238-2017
 public:
  ActClientMxPtpPortEntry() { this->profile_ = 3; };
};

class ActClientMxPtp : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enabled, enable);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientMxPtpPortEntry, port_table, portTable);

 public:
  ActClientMxPtp() { this->enabled_ = true; };
};

class ActClientTimeSyncDot1asPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_FIELD(qint32, announce_interval, announceInterval);
  ACT_JSON_FIELD(qint32, announce_receipt_timeout, announceReceiptTimeout);
  ACT_JSON_FIELD(qint32, sync_interval, syncInterval);
  ACT_JSON_FIELD(qint32, sync_receipt_timeout, syncReceiptTimeout);
  ACT_JSON_FIELD(qint32, pdelay_req_interval, pdelayReqInterval);
  ACT_JSON_FIELD(qint32, neighbor_prop_delay_thresh, neighborPropDelayThresh);

 public:
  ActClientTimeSyncDot1asPortEntry() {
    this->enable_ = true;
    this->announce_interval_ = 0;
    this->announce_receipt_timeout_ = 3;
    this->sync_interval_ = -3;
    this->sync_receipt_timeout_ = 3;
    this->pdelay_req_interval_ = 0;
    this->neighbor_prop_delay_thresh_ = 800;
  };
};

class ActClientTimeSyncDot1as : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint32, priority1, priority1);
  ACT_JSON_FIELD(quint32, priority2, priority2);
  ACT_JSON_FIELD(quint32, clock_class, clockClass);
  ACT_JSON_FIELD(quint32, clock_accuracy, clockAccuracy);
  ACT_JSON_FIELD(quint32, accuracy_alert, accuracyAlert);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientTimeSyncDot1asPortEntry, port_ds, portDS);

 public:
  ActClientTimeSyncDot1as() {
    this->priority1_ = 246;
    this->priority2_ = 248;
    this->clock_class_ = 248;
    this->clock_accuracy_ = 254;
    this->accuracy_alert_ = 500;
  };
};

class ActClientTimeSync1588PortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_FIELD(qint32, announce_interval, announceInterval);
  ACT_JSON_FIELD(qint32, announce_receipt_timeout, announceReceiptTimeout);
  ACT_JSON_FIELD(qint32, sync_interval, syncInterval);
  ACT_JSON_FIELD(qint32, delay_req_interval, delayReqInterval);
  ACT_JSON_FIELD(qint32, pdelay_req_interval, pdelayReqInterval);

 public:
  ActClientTimeSync1588PortEntry() {
    this->enable_ = true;
    this->announce_interval_ = 1;
    this->announce_receipt_timeout_ = 3;
    this->sync_interval_ = 0;
    this->delay_req_interval_ = 0;
    this->pdelay_req_interval_ = 0;
  };
};

class ActClientTimeSync1588 : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint32, clock_type, clockType);
  ACT_JSON_FIELD(quint32, delay_mechanism, delayMechanism);
  ACT_JSON_FIELD(quint32, transport_type, transportType);
  ACT_JSON_FIELD(quint32, priority1, priority1);
  ACT_JSON_FIELD(quint32, priority2, priority2);
  ACT_JSON_FIELD(quint32, domain_number, domainNumber);
  ACT_JSON_FIELD(bool, two_step_flag, twoStepFlag);  // ClockMode: one-step(1), two-step(2)
  ACT_JSON_FIELD(quint32, accuracy_alert, accuracyAlert);
  ACT_JSON_FIELD(quint32, maximum_steps_removed, maximumStepsRemoved);  // TSN no
  ACT_JSON_FIELD(quint32, clock_class, clockClass);
  ACT_JSON_FIELD(quint32, clock_accuracy, clockAccuracy);  //  UI no field

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientTimeSync1588PortEntry, port_ds, portDS);

 public:
  ActClientTimeSync1588() {
    this->clock_type_ = 2;
    this->delay_mechanism_ = 1;
    this->transport_type_ = 3;

    this->priority1_ = 128;
    this->priority2_ = 128;
    this->domain_number_ = 0;
    this->two_step_flag_ = true;
    this->accuracy_alert_ = 1000;

    this->maximum_steps_removed_ = 255;
    this->clock_class_ = 248;
    this->clock_accuracy_ = 254;
  };
};

class ActClientTimeSyncDefaultPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_FIELD(qint32, announce_interval, announceInterval);
  ACT_JSON_FIELD(qint32, announce_receipt_timeout, announceReceiptTimeout);
  ACT_JSON_FIELD(qint32, sync_interval, syncInterval);
  ACT_JSON_FIELD(qint32, delay_req_interval, delayReqInterval);
  ACT_JSON_FIELD(qint32, pdelay_req_interval, pdelayReqInterval);

 public:
  ActClientTimeSyncDefaultPortEntry() {
    this->enable_ = true;
    this->announce_interval_ = 0;
    this->announce_receipt_timeout_ = 3;
    this->sync_interval_ = 0;
    this->delay_req_interval_ = 0;
    this->pdelay_req_interval_ = 0;
  };
};

class ActClientTimeSyncIec61850 : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint32, clock_type, clockType);
  ACT_JSON_FIELD(quint32, delay_mechanism, delayMechanism);
  ACT_JSON_FIELD(quint32, transport_type, transportType);
  ACT_JSON_FIELD(quint32, priority1, priority1);
  ACT_JSON_FIELD(quint32, priority2, priority2);
  ACT_JSON_FIELD(quint32, domain_number, domainNumber);
  ACT_JSON_FIELD(bool, two_step_flag, twoStepFlag);  // ClockMode: one-step(1), two-step(2)
  ACT_JSON_FIELD(quint32, accuracy_alert, accuracyAlert);
  ACT_JSON_FIELD(quint32, maximum_steps_removed, maximumStepsRemoved);
  ACT_JSON_FIELD(quint32, clock_class, clockClass);
  ACT_JSON_FIELD(quint32, clock_accuracy, clockAccuracy);  //  UI no field

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientTimeSyncDefaultPortEntry, port_ds, portDS);

 public:
  ActClientTimeSyncIec61850() {
    this->clock_type_ = 2;
    this->delay_mechanism_ = 2;
    this->transport_type_ = 3;

    this->priority1_ = 128;
    this->priority2_ = 128;
    this->domain_number_ = 0;
    this->two_step_flag_ = true;
    this->accuracy_alert_ = 1000;

    this->maximum_steps_removed_ = 255;
    this->clock_class_ = 248;
    this->clock_accuracy_ = 254;
  };
};

class ActClientTimeSyncC37238 : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint32, clock_type, clockType);
  ACT_JSON_FIELD(quint32, delay_mechanism, delayMechanism);
  ACT_JSON_FIELD(quint32, transport_type, transportType);
  ACT_JSON_FIELD(quint32, priority1, priority1);
  ACT_JSON_FIELD(quint32, priority2, priority2);
  ACT_JSON_FIELD(quint32, domain_number, domainNumber);
  ACT_JSON_FIELD(bool, two_step_flag, twoStepFlag);  // ClockMode: one-step(1), two-step(2)
  ACT_JSON_FIELD(quint32, accuracy_alert, accuracyAlert);
  ACT_JSON_FIELD(quint32, grandmaster_id, grandmasterId);

  ACT_JSON_FIELD(quint32, clock_class, clockClass);                     // // UI no field
  ACT_JSON_FIELD(quint32, clock_accuracy, clockAccuracy);               //  UI no field
  ACT_JSON_FIELD(quint32, maximum_steps_removed, maximumStepsRemoved);  // UI no field

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientTimeSyncDefaultPortEntry, port_ds, portDS);

 public:
  ActClientTimeSyncC37238() {
    this->clock_type_ = 2;
    this->delay_mechanism_ = 2;
    this->transport_type_ = 3;

    this->priority1_ = 128;
    this->priority2_ = 128;
    this->domain_number_ = 254;
    this->two_step_flag_ = true;
    this->accuracy_alert_ = 1000;
    this->grandmaster_id_ = 255;

    this->clock_class_ = 248;
    this->clock_accuracy_ = 254;
    this->maximum_steps_removed_ = 255;
  };
};

class ActClientSnmpTrapHostEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, valid, valid);
  ACT_JSON_FIELD(QString, host_name, hostName);
  ACT_JSON_FIELD(qint32, mode, mode);
  ACT_JSON_FIELD(QString, v1v2c_community, v1v2cCommunity);

 public:
  ActClientSnmpTrapHostEntry() {
    this->valid_ = true;
    this->host_name_ = "";
    this->v1v2c_community_ = "";
  };
};

class ActClientSnmpTrap : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientSnmpTrapHostEntry, host, host);

 public:
  ActClientSnmpTrap() {};
};

class ActClientEventLogEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, severity, severity);
  ACT_JSON_FIELD(QString, timestamp, timestamp);
  ACT_JSON_FIELD(QString, hostname, hostname);
  ACT_JSON_FIELD(QString, progname, progname);
  ACT_JSON_FIELD(QString, boot, boot);
  ACT_JSON_FIELD(QString, uptime, uptime);
  ACT_JSON_FIELD(QString, message, message);

 public:
  ActClientEventLogEntry() {
    this->severity_ = "";
    this->timestamp_ = "";
    this->hostname_ = "";
    this->progname_ = "";
    this->boot_ = "";
    this->uptime_ = "";
    this->message_ = "";
  };
};

class ActClientEventLog : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint32, totalnum, totalnum);
  ACT_JSON_FIELD(quint32, total_debug_num, totalDebugNum);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientEventLogEntry, entries, entries);

 public:
  ActClientEventLog() {
    this->totalnum_ = 0;
    this->total_debug_num_ = 0;
  };
};

class ActClientSyslogServerEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_FIELD(QString, server_address, serverAddress);
  ACT_JSON_FIELD(quint16, server_port, serverPort);
  ACT_JSON_FIELD(quint16, authentication, authentication);

 public:
  ActClientSyslogServerEntry() {
    this->enable_ = false;
    this->server_address_ = "";
    this->server_port_ = 514;
    this->authentication_ = 1;  // false
  };
};

class ActClientSyslogServer : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, logging_enable, loggingEnable);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientSyslogServerEntry, syslog_fwd_table, syslogFwdTable);

 public:
  ActClientSyslogServer() { this->logging_enable_ = false; };
};

class ActClientTimeAuthenticationKey : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, valid, valid);
  ACT_JSON_FIELD(quint16, key_id, keyId);
  ACT_JSON_FIELD(QString, key_type, keyType);
  ACT_JSON_FIELD(QString, key_string, keyString);

 public:
  ActClientTimeAuthenticationKey() {
    this->valid_ = true;
    this->key_id_ = 1;
    this->key_type_ = "";
    this->key_string_ = "";
  };
};

class ActClientTimeNtpClient : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, key_id, keyId);
  ACT_JSON_FIELD(QString, server_address, serverAddress);
  ACT_JSON_FIELD(bool, authentication, authentication);

 public:
  ActClientTimeNtpClient() {
    this->key_id_ = 1;
    this->server_address_ = "";
    this->authentication_ = false;
  };
};

class ActClientTimeNtpServer : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_FIELD(bool, authentication, authentication);

 public:
  ActClientTimeNtpServer() {
    this->enable_ = false;
    this->authentication_ = false;
  };
};

class ActClientTimeNtp : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientTimeAuthenticationKey, authentication_key, authenticationKey);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientTimeNtpClient, ntp_client, ntpClient);
  // ACT_JSON_OBJECT(ActClientTimeNtpServer, ntp_server, ntpServer);

 public:
  ActClientTimeNtp() {};
};

class ActClientTimeSntpClient : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, server_address, serverAddress);

 public:
  ActClientTimeSntpClient() { this->server_address_ = ""; };
};

class ActClientTimeSntp : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientTimeSntpClient, sntp_client, sntpClient);

 public:
  ActClientTimeSntp() {};
};

class ActClientTimeDate : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, year, year);
  ACT_JSON_FIELD(quint16, month, month);
  ACT_JSON_FIELD(quint16, date, date);
  ACT_JSON_FIELD(quint16, hour, hour);
  ACT_JSON_FIELD(quint16, minute, minute);

 public:
  ActClientTimeDate() {
    this->year_ = 2000;
    this->month_ = 1;
    this->date_ = 1;
    this->hour_ = 0;
    this->minute_ = 0;
  };
};

class ActClientTimeDay : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, month, month);
  ACT_JSON_FIELD(quint16, week, week);
  ACT_JSON_FIELD(quint16, day, day);
  ACT_JSON_FIELD(quint16, hour, hour);
  ACT_JSON_FIELD(quint16, minute, minute);

 public:
  ActClientTimeDay() {
    this->month_ = 1;
    this->week_ = 1;
    this->day_ = 1;
    this->hour_ = 1;
    this->minute_ = 0;
  };
};

class ActClientTimeDurationDaylightSaving : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_OBJECT(ActClientTimeDate, start_date, startDate);
  ACT_JSON_OBJECT(ActClientTimeDate, end_date, endDate);
  ACT_JSON_FIELD(quint16, offset, offset);

 public:
  ActClientTimeDurationDaylightSaving() {
    this->enable_ = false;
    this->offset_ = 0;
  };
};

class ActClientTimeDaylightSaving : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_OBJECT(ActClientTimeDay, start, start);
  ACT_JSON_OBJECT(ActClientTimeDay, end, end);
  ACT_JSON_FIELD(quint16, offset_min, offsetMin);

 public:
  ActClientTimeDaylightSaving() {
    this->enable_ = false;
    this->offset_min_ = 0;
  };
};

class ActClientTsnDeviceTime : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, clock_source, clockSource);
  ACT_JSON_FIELD(QString, time_zone, timeZone);
  ACT_JSON_OBJECT(ActClientTimeDurationDaylightSaving, daylight_saving, daylightSaving);

  ACT_JSON_OBJECT(ActClientTimeNtp, ntp, ntp);
  ACT_JSON_OBJECT(ActClientTimeSntp, sntp, sntp);

 public:
  ActClientTsnDeviceTime() {
    this->clock_source_ = "PTP";
    this->time_zone_ = "UTC+00:00";
  };
};

class ActClientNosDeviceTime : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, clock_source, clockSource);
  ACT_JSON_FIELD(QString, time_zone, timeZone);
  ACT_JSON_OBJECT(ActClientTimeDaylightSaving, daylight_saving, daylightSaving);

  ACT_JSON_OBJECT(ActClientTimeNtp, ntp, ntp);
  ACT_JSON_OBJECT(ActClientTimeSntp, sntp, sntp);

 public:
  ActClientNosDeviceTime() {
    this->clock_source_ = "PTP";
    this->time_zone_ = "UTC+00:00";
  };
};

class ActClientMxLp : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, loop_protect_enable, loopProtectEnable);
  ACT_JSON_FIELD(qint32, detect_interval, detectInterval);

 public:
  ActClientMxLp() {
    this->loop_protect_enable_ = false;
    this->detect_interval_ = 10;
  };
};

class ActClientNetconfStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // {"vlan": false, "streamid": false, "frer": false, "qbv": false}

  ACT_JSON_FIELD(bool, vlan, vlan);
  ACT_JSON_FIELD(bool, streamid, streamid);
  ACT_JSON_FIELD(bool, frer, frer);
  ACT_JSON_FIELD(bool, qbv, qbv);

 public:
  ActClientNetconfStatus() : vlan_(false), streamid_(false), frer_(false), qbv_(false) {};
};

class ActClientSystemUtilization : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qreal, cpu_utilization, cpuUtilization);
  ACT_JSON_FIELD(quint64, memory_size, memorySize);
  ACT_JSON_FIELD(qreal, memory_utilization, memoryUtilization);
  ACT_JSON_FIELD(quint64, power_consumption, powerConsumption);

 public:
  ActClientSystemUtilization() : cpu_utilization_(0), memory_size_(0), memory_utilization_(0), power_consumption_(0) {};
};

class ActClientIfMibPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);

 public:
  ActClientIfMibPortEntry() { this->enable_ = true; };
};

class ActClientIfMib : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientIfMibPortEntry, port_table, portTable);

 public:
  ActClientIfMib() {};
};

class ActClientPortInfoEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, name, name);
  ACT_JSON_FIELD(QString, type, type);
  ACT_JSON_FIELD(QString, speed, speed);
  ACT_JSON_FIELD(qint64, module_slot, moduleSlot);
  ACT_JSON_FIELD(qint64, module_port, modulePort);
  ACT_JSON_FIELD(bool, exist, exist);
  ACT_JSON_FIELD(bool, sfp_inserted, sfpInserted);
  ACT_JSON_FIELD(QString, physical_mac_addr, physicalMacAddr);
  ACT_JSON_COLLECTION(QList, QString, function, function);
  ACT_JSON_COLLECTION(QList, QString, medium, medium);

 public:
  ActClientPortInfoEntry() {
    this->type_ = "";
    this->speed_ = "";
    this->name_ = "";
    this->module_slot_ = -1;
    this->module_port_ = -1;
    this->exist_ = false;
    this->sfp_inserted_ = false;
    this->physical_mac_addr_ = "";
  };
};

class ActClientPortInfo : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientPortInfoEntry, port_table, portTable);

 public:
  ActClientPortInfo() {};
};

class ActClientPortStatusEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, media_type, mediaType);
  ACT_JSON_FIELD(quint8, port_state, portState);
  ACT_JSON_FIELD(quint8, link_status, linkStatus);  ///< 1(up), 2(down)
  ACT_JSON_FIELD(QString, oper_duplex, operDuplex);
  ACT_JSON_FIELD(QString, oper_speed, operSpeed);
  ACT_JSON_FIELD(QString, mdi_or_mdix_cap, mdiOrMdixCap);

 public:
  ActClientPortStatusEntry() {
    this->media_type_ = "";
    this->port_state_ = 0;
    this->link_status_ = 0;
    this->oper_duplex_ = "";
    this->oper_speed_ = "";
    this->mdi_or_mdix_cap_ = "";
  };
};

class ActClientPortStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientPortStatusEntry, port_table, portTable);

 public:
  ActClientPortStatus() {};
};

class ActClientTrafficUtilization : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, QString, timestamp, timestamp);
  ACT_JSON_COLLECTION(QList, qreal, data, data);

 public:
  ActClientTrafficUtilization() {};
};

class ActClientTrafficStatisticsEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint64, tx_total_octets, txTotalOctets);
  ACT_JSON_FIELD(quint64, tx_total_packets, txTotalPackets);
  ACT_JSON_FIELD(quint64, tx_unicast_packets, txUnicastPackets);
  ACT_JSON_FIELD(quint64, tx_multicast_packets, txMulticastPackets);
  ACT_JSON_FIELD(quint64, tx_broadcast_packets, txBroadcastPackets);

  ACT_JSON_FIELD(quint64, rx_total_octets, rxTotalOctets);
  ACT_JSON_FIELD(quint64, rx_total_packets, rxTotalPackets);
  ACT_JSON_FIELD(quint64, rx_unicast_packets, rxUnicastPackets);
  ACT_JSON_FIELD(quint64, rx_multicast_packets, rxMulticastPackets);
  ACT_JSON_FIELD(quint64, rx_broadcast_packets, rxBroadcastPackets);

  ACT_JSON_FIELD(quint64, crc_align_error_packets, crcAlignErrorPackets);
  ACT_JSON_FIELD(quint64, drop_packets, dropPackets);
  ACT_JSON_FIELD(quint64, undersize_packets, undersizePackets);
  ACT_JSON_FIELD(quint64, oversize_packets, oversizePackets);

  ACT_JSON_OBJECT(ActClientTrafficUtilization, traffic_utilization, trafficUtilization);

 public:
  ActClientTrafficStatisticsEntry() {
    this->tx_total_octets_ = 0;
    this->tx_total_packets_ = 0;
    this->tx_unicast_packets_ = 0;
    this->tx_multicast_packets_ = 0;
    this->tx_broadcast_packets_ = 0;

    this->rx_total_octets_ = 0;
    this->rx_total_packets_ = 0;
    this->rx_unicast_packets_ = 0;
    this->rx_multicast_packets_ = 0;
    this->rx_broadcast_packets_ = 0;

    this->crc_align_error_packets_ = 0;
    this->drop_packets_ = 0;
    this->undersize_packets_ = 0;
    this->oversize_packets_ = 0;
  };
};

class ActClientTrafficStatistics : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientTrafficStatisticsEntry, port_table, portTable);

 public:
  ActClientTrafficStatistics() {};
};

class ActClientFiberCheckStatusEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, port_index, portIndex);
  ACT_JSON_FIELD(QString, model_name, modelName);
  ACT_JSON_FIELD(QString, serial_number, serialNumber);
  ACT_JSON_FIELD(QString, wavelength, wavelength);
  ACT_JSON_FIELD(QString, temperature_c, temperatureC);
  ACT_JSON_FIELD(QString, temperature_f, temperatureF);
  ACT_JSON_FIELD(QString, voltage, voltage);
  ACT_JSON_FIELD(QString, tx_power, txPower);
  ACT_JSON_FIELD(QString, rx_power, rxPower);
  ACT_JSON_FIELD(QString, tx_bias_current, txBiasCurrent);
  ACT_JSON_FIELD(QString, temperatureLimit_c, temperatureLimitC);
  ACT_JSON_FIELD(QString, temperatureLimit_f, temperatureLimitF);

  ACT_JSON_COLLECTION(QList, QString, tx_power_limit, txPowerLimit);
  ACT_JSON_COLLECTION(QList, QString, rx_power_limit, rxPowerLimit);

 public:
  ActClientFiberCheckStatusEntry() {
    this->port_index_ = 0;
    this->model_name_ = "";
    this->serial_number_ = "";
    this->wavelength_ = "";
    this->temperature_c_ = "";
    this->temperature_f_ = "";
    this->voltage_ = "";
    this->tx_power_ = "";
    this->rx_power_ = "";
    this->tx_bias_current_ = "";
    this->temperatureLimit_c_ = "";
    this->temperatureLimit_f_ = "";
  };
};

class ActClientFiberCheckStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientFiberCheckStatusEntry, port_table, portTable);

 public:
  ActClientFiberCheckStatus() {};
};

class ActClientPtpClockTime : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, year, year);
  ACT_JSON_FIELD(quint16, month, month);
  ACT_JSON_FIELD(quint16, day, day);
  ACT_JSON_FIELD(quint16, hour, hour);
  ACT_JSON_FIELD(quint16, minute, minute);
  ACT_JSON_FIELD(quint16, second, second);

 public:
  ActClientPtpClockTime() {
    this->year_ = 0;
    this->month_ = 0;
    this->day_ = 0;
    this->hour_ = 0;
    this->minute_ = 0;
    this->second_ = 0;
  };
};

class ActClient1588ParentDS : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, parent_clock_identity, parentClockIdentity);
  ACT_JSON_FIELD(quint64, parent_port_number, parentPortNumber);
  ACT_JSON_FIELD(QString, grandmaster_identity, grandmasterIdentity);
  ACT_JSON_FIELD(quint64, grandmaster_clockClass, grandmasterClockClass);
  ACT_JSON_FIELD(quint64, grandmaster_clockAccuracy, grandmasterClockAccuracy);
  ACT_JSON_FIELD(quint64, grandmaster_priority1, grandmasterPriority1);
  ACT_JSON_FIELD(quint64, grandmaster_priority2, grandmasterPriority2);

 public:
  ActClient1588ParentDS() {
    this->parent_clock_identity_ = "";
    this->parent_port_number_ = 0;
    this->grandmaster_identity_ = "";
    this->grandmaster_clockClass_ = 0;
    this->grandmaster_clockAccuracy_ = 0;
    this->grandmaster_priority1_ = 0;
    this->grandmaster_priority2_ = 0;
  };
};

class ActClient1588PortDSEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint64, neighbor_prop_delay, neighborPropDelay);
  ACT_JSON_FIELD(quint8, port_state, portState);
  ACT_JSON_FIELD(QString, port_identity, portIdentity);

 public:
  ActClient1588PortDSEntry() {
    this->neighbor_prop_delay_ = 0;
    this->port_state_ = 0;
    this->port_identity_ = "";
  };
};

class ActClient1588DefaultInfo : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint64, steps_removed, stepsRemoved);
  ACT_JSON_FIELD(QString, offset_from_master, offsetFromMaster);
  ACT_JSON_OBJECT(ActClientPtpClockTime, ptp_clock_time, ptpClockTime);
  ACT_JSON_FIELD(bool, sync_locked, syncLocked);
  ACT_JSON_FIELD(QString, clock_identity, clockIdentity);
  ACT_JSON_FIELD(QString, slave_port, slavePort);
  ACT_JSON_FIELD(QString, mean_path_delay, meanPathDelay);

  ACT_JSON_OBJECT(ActClient1588ParentDS, parent_ds, parentDS);

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClient1588PortDSEntry, port_ds, portDS);

 public:
  ActClient1588DefaultInfo() {
    this->steps_removed_ = 0;
    this->offset_from_master_ = "";
    this->sync_locked_ = false;
    this->clock_identity_ = "";
    this->slave_port_ = "";
    this->mean_path_delay_ = "";
  };
};

class ActClientDot1asParentDS : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, parent_clock_identity, parentClockIdentity);
  ACT_JSON_FIELD(quint64, parent_port_number, parentPortNumber);
  ACT_JSON_FIELD(qreal, cumulative_rate_ratio, cumulativeRateRatio);
  ACT_JSON_FIELD(QString, grandmaster_identity, grandmasterIdentity);
  ACT_JSON_FIELD(quint64, grandmaster_clockClass, grandmasterClockClass);
  ACT_JSON_FIELD(quint64, grandmaster_clockAccuracy, grandmasterClockAccuracy);
  ACT_JSON_FIELD(quint64, grandmaster_priority1, grandmasterPriority1);
  ACT_JSON_FIELD(quint64, grandmaster_priority2, grandmasterPriority2);

 public:
  ActClientDot1asParentDS() {
    this->parent_clock_identity_ = "";
    this->parent_port_number_ = 0;
    this->cumulative_rate_ratio_ = 0;
    this->grandmaster_identity_ = "";
    this->grandmaster_clockClass_ = 0;
    this->grandmaster_clockAccuracy_ = 0;
    this->grandmaster_priority1_ = 0;
    this->grandmaster_priority2_ = 0;
  };
};

class ActClientDot1asPortDSEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint64, neighbor_prop_delay, neighborPropDelay);
  ACT_JSON_FIELD(quint8, port_role, portRole);
  ACT_JSON_FIELD(bool, as_capable, asCapable);
  ACT_JSON_FIELD(qreal, neighbor_rate_ratio, neighborRateRatio);
  ACT_JSON_FIELD(QString, port_identity, portIdentity);

 public:
  ActClientDot1asPortDSEntry() {
    this->neighbor_prop_delay_ = 0;
    this->port_role_ = 0;
    this->as_capable_ = false;
    this->neighbor_rate_ratio_ = 0;
    this->port_identity_ = "";
  };
};

class ActClientDot1asInfo : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint64, steps_removed, stepsRemoved);
  ACT_JSON_FIELD(QString, offset_from_master, offsetFromMaster);
  ACT_JSON_OBJECT(ActClientPtpClockTime, ptp_clock_time, ptpClockTime);
  ACT_JSON_FIELD(bool, sync_locked, syncLocked);
  ACT_JSON_FIELD(QString, clock_identity, clockIdentity);
  ACT_JSON_FIELD(QString, slave_port, slavePort);
  ACT_JSON_FIELD(QString, mean_path_delay, meanPathDelay);
  ACT_JSON_OBJECT(ActClientDot1asParentDS, parent_ds, parentDS);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientDot1asPortDSEntry, port_ds, portDS);

 public:
  ActClientDot1asInfo() {
    this->steps_removed_ = 0;
    this->offset_from_master_ = "";
    this->sync_locked_ = false;
    this->clock_identity_ = "";
    this->slave_port_ = "";
    this->mean_path_delay_ = "";
  };
};

class ActClientStdVlanEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, vid, vid);
  ACT_JSON_FIELD(QString, vlan_name, vlanName);
  ACT_JSON_FIELD(bool, valid, valid);
  ACT_JSON_COLLECTION(QList, bool, egress_ports_pbmp, egressPortsPbmp);
  ACT_JSON_COLLECTION(QList, bool, forbidden_egress_ports_pbmp, forbiddenEgressPortsPbmp);
  ACT_JSON_COLLECTION(QList, bool, untagged_ports_pbmp, untaggedPortsPbmp);

 public:
  QList<QString> no_forbidden_key_order_;

  ActClientStdVlanEntry() {
    this->no_forbidden_key_order_.append(QList<QString>({QString("vid"), QString("vlanName"), QString("valid"),
                                                         QString("egressPortsPbmp"), QString("untaggedPortsPbmp")}));

    this->vid_ = 0;
    this->vlan_name_ = "";
    this->valid_ = true;
  };
};

class ActClientStdPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, pvid, pvid);
  ACT_JSON_FIELD(quint16, acceptable_frame_types, acceptableFrameTypes);
  ACT_JSON_FIELD(bool, ingress_filtering, ingressFiltering);
  ACT_JSON_FIELD(bool, port_gvrp_enable, portGvrpEnable);
  ACT_JSON_FIELD(bool, restricted_vlan_registration, restrictedVlanRegistration);

 public:
  ActClientStdPortEntry() {
    // DUT default
    this->pvid_ = 1;
    this->acceptable_frame_types_ = 3;  ///< Access(3), Trunk & Hybrid(1)
    this->ingress_filtering_ = true;
    this->port_gvrp_enable_ = false;
    this->restricted_vlan_registration_ = false;
  };
};

class ActClientStdVlan : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientStdVlanEntry, vlan_table, vlanTable);

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientStdPortEntry, port_table, portTable);

 public:
  ActClientStdVlan() {};
};

class ActClientMxVlanPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  //  kAccess = 1, kTrunk = 2, kHybrid = 3, kNone = 4
  ACT_JSON_FIELD(quint16, vlan_port_type, vlanPortType);
  ACT_JSON_FIELD(quint32, filtering_utility_criteria, filteringUtilityCriteria);

 public:
  ActClientMxVlanPortEntry() {
    // DUT default
    this->vlan_port_type_ = 1;
    this->filtering_utility_criteria_ = 1;
  };
};

class ActClientMxVlan : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientMxVlanPortEntry, port_table, portTable);

 public:
  ActClientMxVlan() {};
};

class ActClientMxQosDefaultPriorityEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, default_priority_value, defaultPriorityValue);

 public:
  ActClientMxQosDefaultPriorityEntry() {
    // DUT default
    this->default_priority_value_ = 0;
  };
};

class ActClientMxQos : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActClientMxQosDefaultPriorityEntry, default_priority_table, defaultPriorityTable);

 public:
  ActClientMxQos() {};
};

class ActClientIpv4 : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, network_setting_mode, networkSettingMode);
  ACT_JSON_FIELD(QString, ip_address, ipAddress);
  ACT_JSON_FIELD(QString, netmask, netmask);
  ACT_JSON_FIELD(QString, gateway, gateway);
  ACT_JSON_COLLECTION(QList, QString, dns_server, dnsServer);

 public:
  ActClientIpv4() {
    this->network_setting_mode_ = "Manual";
    this->ip_address_ = "";
    this->netmask_ = "";
    this->gateway_ = "";
  };
};

class ActClientL3RouterIdIpv4 : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, ip_address, ipAddress);
  ACT_JSON_FIELD(QString, netmask, netmask);

 public:
  ActClientL3RouterIdIpv4() {
    this->ip_address_ = "";
    this->netmask_ = "";
  };
};

class ActClientL3NetworkDnsIpv4 : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, QString, dns_server, dnsServer);

 public:
  ActClientL3NetworkDnsIpv4() {};
};

class ActClientBadRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, description, description);
  ACT_JSON_FIELD(QString, message, message);
  ACT_JSON_FIELD(QString, path, path);
  ACT_JSON_FIELD(QString, value, value);

 public:
  ActClientBadRequest() {
    this->description_ = "";
    this->message_ = "";
    this->path_ = "";
    this->value_ = "";
  };
};

class ActClientImportConfigRequestFileParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, verify_config, verify_config);

 public:
  ActClientImportConfigRequestFileParameter() { this->verify_config_ = false; };
};

class ActClientImportConfigRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActClientImportConfigRequestFileParameter, file_parameter, file_parameter);
  ACT_JSON_FIELD(bool, save_config, save_config);

 public:
  ActClientImportConfigRequest() { this->save_config_ = true; };
};

class ActClientExportConfigRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, include_default_config, include_default_config);
  ACT_JSON_FIELD(bool, is_running_config, is_running_config);

 public:
  ActClientExportConfigRequest() {
    this->include_default_config_ = false;
    this->is_running_config_ = false;
  };
};

#endif /* ACT_MOXA_IEI_CLIENT_HPP */
