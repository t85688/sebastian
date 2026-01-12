/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <QDir>
#include <QFile>
#include <QLoggingCategory>

#include "act_json.hpp"
#include "act_status.hpp"

#define ACT_MAJOR_VERSION (0)       ///< The major version of the current ACT
#define ACT_MINOR_VERSION (6)       ///< The minor version of the current ACT
#define ACT_CUSTOMIZED_VERSION (0)  ///< The customized version of the current ACT

#define ACT_LOG_SIZE (1024 * 500)  ///< The log file size in bytes, current is 500KB
#define ACT_LOG_FOLDER "logs"      ///< The folder of log files
#define ACT_LOG_MIN_FREE_GB (5)    // Default minimum free GB for logs
#define ACT_LOG_KEEP_DAYS (30)     // Default log retention days

#define ACT_DATABASE_FOLDER_TMP "db"

#define ACT_VERSION_FILE_NAME "version.txt"

// [feat:2491] MELCO - CLI process control
#define ACT_CLI_FAILURE_FILE_NAME "act_cli_failure.txt"  ///< The failure status of the cli command
// #define ACT_CLI_SUCCESS_FILE_NAME "act_cli_success.txt"  ///< The success status of the cli command
// #define ACT_CLI_CLOSE_FILE_NAME "act_cli_close.txt"      ///< The close status of the cli command

#define MAF_EXPORT_CONFIG_FILE_NAME "device_config.zip"

#define ACT_ENCRYPTED_LICENSE_FILE_NAME "License.txt"
#define ACT_LICENSE_SECRET_KEY (0x89191230)  ///< The secret ket of the license

#define ACT_EVALUATION_DEVICE_QTY (100)          ///< The device quantity of the evaluation version
#define ACT_EVALUATION_EXPIRE_DATE "2099/12/31"  ///< The expire date of the evaluation version
#define ACT_EVALUATION_PROJECT_SIZE (100)        ///< The number of supported projects of the evaluation version
#define ACT_EVALUATION_USER_SIZE (32)            ///< The number of supported users of the evaluation version

#define ACT_DEVICE_PROFILE_FOLDER "configuration/device_profile"
#define ACT_DEVICE_ICON_FOLDER "configuration/device_icon"
#define ACT_FEATURE_PROFILE_FOLDER "configuration/feature_profile"
#define ACT_FIRMWARE_FEATURE_PROFILE_FOLDER "configuration/firmware_feature_profile"
#define ACT_DEFAULT_DEVICE_PROFILE_FOLDER "configuration/default_device_profile"
#define ACT_GENERAL_PROFILE_FOLDER "configuration/general_profile"
#define ACT_FIRMWARE_FILE_FOLDER "firmware"
#define ACT_DEVICE_CONFIG_FILE_FOLDER "tmp/device_config"
#define ACT_GENERATE_OFFLINE_CONFIG_FOLDER "tmp/offline_config"
#define ACT_ETHERNET_MODULE_FOLDER "configuration/ethernet_module"
#define ACT_SFP_MODULE_FOLDER "configuration/sfp_module"
#define ACT_POWER_MODULE_FOLDER "configuration/power_module"

#define ACT_BUILTIN_LINE_MODULE_SLOT (1)
#define ACT_BUILTIN_POWER_MODULE_SLOT (1)

#define ACT_API_PATH_PREFIX "/api/v1"  ///< The API path prefix

#define ACT_STRING_LENGTH_MIN (1)   ///< The minimum length of the input string
#define ACT_STRING_LENGTH_MAX (64)  ///< The maximum length of the input string

#define ACT_USERNAME_LENGTH_MIN (4)
#define ACT_USERNAME_LENGTH_MAX (32)

#define ACT_PASSWORD_LENGTH_MIN (4)
#define ACT_PASSWORD_LENGTH_MAX (63)

#define ACT_NETWORK_BASELINE_NAME_LENGTH_MIN (1)
#define ACT_NETWORK_BASELINE_NAME_LENGTH_MAX (64)
#define ACT_NETWORK_BASELINE_SIZE (1000)
#define ACT_NETWORK_BASELINE_CURRENT_NAME "CURRENT"
#define ACT_NETWORK_BASELINE_INITIAL_NAME "INITIAL"
#define ACT_NETWORK_BASELINE_INITIAL_CREATED_USER ""

#define ACT_MEDIA_OVERHEAD_MIN (1)
#define ACT_MEDIA_OVERHEAD_MAX (64)

#define ACT_TIME_SYNC_DELAY_MIN (0)
#define ACT_TIME_SYNC_DELAY_MAX (1000)

#define ACT_BEST_EFFORT_BANDWIDTH_MIN (35)
#define ACT_TIME_SYNC_BANDWIDTH_MIN (35)

#define ACT_COMPUTE_TIMEOUT_MIN (0)
#define ACT_COMPUTE_TIMEOUT_MAX (9999)

#define ACT_VLAN_MIN (2)
#define ACT_VLAN_MAX (4094)
#define ACT_VLAN_INIT_PVID (1)

#define ACT_INIT_DEFAULT_PCP (0)

#define ACT_PERIOD_MIN (1)
#define ACT_PERIOD_MAX (999999999)

#define ACT_INTERVAL_MIN (30000)
#define ACT_INTERVAL_MAX (999999999)

#define ACT_FRAME_PER_INTERVAL_MIN (1)
#define ACT_FRAME_PER_INTERVAL_MAX (65535)

#define ACT_FRAME_SIZE_MIN (46)
#define ACT_FRAME_SIZE_MAX (1500)

#define ACT_JITTER_MIN (0)
#define ACT_JITTER_MAX (999999999)

#define ACT_TIME_AWARE_MIN (0)
#define ACT_TIME_AWARE_MAX (999999999)

#define ACT_LATENCY_MIN (1000)
#define ACT_LATENCY_MAX (999999999)

#define ACT_RECEIVE_OFFSET_MIN (1)
#define ACT_RECEIVE_OFFSET_MAX (999999.999)

#define ACT_DEFAULT_PROJECT_START_IP "192.168.127.1"

#define ACT_LOCATOR_DURATION_MIN (30)
#define ACT_LOCATOR_DURATION_MAX (300)

#define ACT_SNMP_COMMUNITY_MIN (1)
#define ACT_SNMP_COMMUNITY_MAX (32)

#define ACT_SNMP_PASSWORD_LENGTH_MIN (8)
#define ACT_SNMP_PASSWORD_LENGTH_MAX (64)

#define ACT_TOKEN_SECRET "moxa-act-secret"
#define ACT_IDLE_TIMEOUT (15)
#define ACT_HARD_TIMEOUT (60)
#define ACT_TOKEN_MAX_SIZE (10)
#define ACT_WS_SOCKET_MAX_SIZE (ACT_TOKEN_MAX_SIZE * 3)

// Account
#define ACT_DEFAULT_DEVICE_ACCOUNT_USERNAME "admin"
#define ACT_DEFAULT_DEVICE_ACCOUNT_PASSWORD "moxa"
#define ACT_DEFAULT_DEVICE_ACCOUNT_EMAIL "admin@sample.com"

// SSH
#define ACT_DEFAULT_SSH_PORT (22)

// NETCONF
#define ACT_DEFAULT_NETCONF_TLS_PORT (6513)
#define ACT_DEFAULT_NETCONF_SSH_PORT (830)
#define ACT_NETCONF_SSH_PORT_MIN (1024)
#define ACT_NETCONF_SSH_PORT_MAX (65535)

// RESTful
#define ACT_DEFAULT_RESTFUL_PROTOCOL (ActRestfulProtocolEnum::kHTTPS)
#define ACT_DEFAULT_RESTFUL_PORT (443)
#define ACT_DEFAULT_RESTFUL_HTTP_PORT (80)
#define ACT_DEFAULT_RESTFUL_HTTPS_PORT (443)
#define ACT_RESTFUL_PORT_MIN (1024)
#define ACT_RESTFUL_PORT_MAX (65535)

// SNMP
#define ACT_DEFAULT_SNMP_VERSION (ActSnmpVersionEnum::kV2c)
#define ACT_DEFAULT_SNMP_TRAP_VERSION (ActSnmpVersionEnum::kV2c)
#define ACT_DEFAULT_SNMP_PORT (161)
#define ACT_DEFAULT_SNMP_TRAP_PORT (162)
#define ACT_DEFAULT_SNMP_READ_COMMUNITY "public"
#define ACT_DEFAULT_SNMP_TRAP_COMMUNITY "public"
#define ACT_DEFAULT_SNMP_WRITE_COMMUNITY "private"
#define ACT_DEFAULT_SNMP_USERNAME "moxa"
#define ACT_SNMP_PORT_MIN (1024)
#define ACT_SNMP_PORT_MAX (65535)
#define ACT_SNMP_READ_TIMEOUT (2000000)       ///< The timeout(us, 2 second) of the session
#define ACT_SNMP_READ_BULK_TIMEOUT (6000000)  ///< The timeout(us, 6 second) of the session
#define ACT_SNMP_WRITE_TIMEOUT (10000000)     ///< The timeout(us, 10 second) of the session
#define ACT_SNMP_RETRY_TIMES (2)

// NewMOXACommand
#define ACT_NEW_MOXA_COMMAND_TIMEOUT (5)  ///< The timeout of the newMoxaCommand connection
#define ACT_NEW_MOXA_COMMAND_BROADCAST_PORT (40404)
#define ACT_NEW_MOXA_COMMAND_HTTP_CONNECTION_TYPE "https"
#define ACT_NEW_MOXA_COMMAND_HTTP_CONNECTION_PORT (443)
#define ACT_NEW_MOXA_COMMAND_FAKE_MAC "AA:AA:AA:AA:AA:AA"
#define ACT_NEW_MOXA_COMMAND_FIRMWARE_UPGRADE_TIMEOUT (240)

#define ACT_DEPLOY_TSN_SWITCH_RETRY_TIMES (3)

// #define ACT_DEFAULT_DEVICE_PROFILE_ID (0)
#define ACT_SWITCH_PROFILE_ID (1)
#define ACT_END_STATION_DEVICE_PROFILE_ID (2)
#define ACT_BRIDGE_END_STATION_PROFILE_ID (3)
#define ACT_UNKNOW_DEVICE_PROFILE_ID (4)
#define ACT_ICMP_DEVICE_PROFILE_ID (5)
#define ACT_MOXA_DEVICE_PROFILE_ID (6)

#define ACT_DEFAULT_DEVICE_ICON_NAME "default.png"
#define ACT_DEFAULT_ICMP_ICON_NAME "default-icmp.png"
#define ACT_DEFAULT_SWITCH_ICON_NAME "default-switch.png"
#define ACT_DEFAULT_END_STATION_ICON_NAME "default-end-station.png"
#define ACT_DEFAULT_BRIDGED_END_STATION_ICON_NAME "default-bridged-end-station.png"
#define ACT_MOXA_DEVICE_ICON_NAME "Moxa.png"
#define ACT_DEFAULT_INTELLIGENT_ENDPOINT "http://10.123.31.11:8443"
#define ACT_DEFAULT_INTELLIGENT_LOCAL_ENDPOINT "http://127.0.0.1:59000"
#define ACT_SERVICE_PLATFORM_ENDPOINT "https://c2connect.moxa.com:443"

#define ACT_FAKE_MAC_ADDRESS "00-00-00-00-00-00"
#define ACT_BROADCAST_MAC_ADDRESS "FF-FF-FF-FF-FF-FF"

#define ACT_MIN_NUM_WORKER_THREAD (4)

#define ACT_SERVER_PROJECT_EXPIRED_TIMEOUT (86400)  ///< The timeout of the server project expired in seconds

// DATA VERSION
#define ACT_SYSTEM_DATA_VERSION "2"
#define ACT_USER_DATA_VERSION "2"
#define ACT_PROJECT_DATA_VERSION "2"
#define ACT_DEVICE_PROFILE_DATA_VERSION "2"
#define ACT_FIRMWARE_FEATURE_PROFILE_DATA_VERSION "2"  // for firmware profile
#define ACT_POWER_DEVICE_PROFILE_DATA_VERSION "2"
#define ACT_SOFTWARE_LICENSE_PROFILE_DATA_VERSION "2"
#define ACT_ETHERNET_MODULE_DATA_VERSION "2"
#define ACT_POWER_MODULE_DATA_VERSION "2"
#define ACT_SFP_MODULE_DATA_VERSION "2"
#define ACT_FEATURE_PROFILE_DATA_VERSION "2"
#define ACT_SKU_DATA_VERSION "2"  // general profile
#define ACT_BASELINE_DATA_VERSION "2"
#define ACT_FIRMWARE_DATA_VERSION "2"
#define ACT_TOPOLOGY_DATA_VERSION "2"

// #define ACT_DEFAULT_LOG_SEVERITY
Q_DECLARE_LOGGING_CATEGORY(logger)  ///< The logger module

enum class ActProcessStatus { Init, Start, Running, Exit };
extern ActProcessStatus g_act_process_status;  ///< The stop flag of the infinite loop of the main thread

enum class ActSystemStatusEnum { kIdle, kAutoProbing, kMonitoring };
static const QMap<QString, ActSystemStatusEnum> kActSystemStatusEnumMap = {
    {"Idle", ActSystemStatusEnum::kIdle},
    {"AutoProbing", ActSystemStatusEnum::kAutoProbing},
    {"Monitoring", ActSystemStatusEnum::kMonitoring}};

enum class ActServiceProfileForLicenseEnum { kSelfPlanning, kGeneral, kFoxboro };
static const QMap<QString, ActServiceProfileForLicenseEnum> kActServiceProfileForLicenseEnumMap = {
    {"SelfPlanning", ActServiceProfileForLicenseEnum::kSelfPlanning},
    {"General", ActServiceProfileForLicenseEnum::kGeneral},
    {"Foxboro", ActServiceProfileForLicenseEnum::kFoxboro}};

enum class ActServiceProfileForDeviceProfileEnum {
  kSelfPlanning,
  kVerifiedGeneral,
  kUnVerifiedGeneral,
  kUnVerifiedFoxboro,
  kVerifiedFoxboro
};
static const QMap<QString, ActServiceProfileForDeviceProfileEnum> kActServiceProfileForDeviceProfileEnumMap = {
    {"SelfPlanning", ActServiceProfileForDeviceProfileEnum::kSelfPlanning},
    {"VerifiedGeneral", ActServiceProfileForDeviceProfileEnum::kVerifiedGeneral},
    {"UnVerifiedGeneral", ActServiceProfileForDeviceProfileEnum::kUnVerifiedGeneral},
    {"UnVerifiedFoxboro", ActServiceProfileForDeviceProfileEnum::kUnVerifiedFoxboro},
    {"VerifiedFoxboro", ActServiceProfileForDeviceProfileEnum::kVerifiedFoxboro}};

enum class ActDeploymentTypeEnum { kLocal, kServer };
static const QMap<QString, ActDeploymentTypeEnum> kActDeploymentTypeEnumMap = {
    {"Local", ActDeploymentTypeEnum::kLocal}, {"Server", ActDeploymentTypeEnum::kServer}};

static QString GetIntelligentEndpointFromEnviron() {
  // ACT_DEFAULT_INTELLIGENT_ENDPOINT
  QString endpoint =
      qEnvironmentVariable("CHAMBERLAIN_AI_ASSISTANT_SERVICE_ENDPOINT", ACT_DEFAULT_INTELLIGENT_ENDPOINT);
  return endpoint;
}

static QString GetIntelligentLocalEndpointFromEnviron() {
  // ACT_DEFAULT_INTELLIGENT_LOCAL_ENDPOINT
  QString apiAddr = qEnvironmentVariable("MAF_API_ADDR", "localhost");
  QString apiPort = qEnvironmentVariable("MAF_API_PORT", "59000");
  return "http://" + apiAddr + ":" + apiPort + "/";
}

static QString GetHttpProxyEndpointFromEnviron() {
  QString httpProxy = qEnvironmentVariable("CHAMBERLAIN_HTTP_PROXY_ENDPOINT", "");
  if (httpProxy.isEmpty()) {
    httpProxy = qEnvironmentVariable("CHAMBERLAIN_HTTPS_PROXY_ENDPOINT", "");
  }
  return httpProxy;
}

static QString GetServicePlatformEndpointFromEnviron() {
  // ACT_SERVICE_PLATFORM_ENDPOINT
  QString endpoint = qEnvironmentVariable("CHAMBERLAIN_SERVICE_PLATFORM_ENDPOINT", ACT_SERVICE_PLATFORM_ENDPOINT);
  if (!endpoint.endsWith("/")) {
    endpoint += "/";
  }
  return endpoint;
}

/**
 * @brief The ACT system class
 *
 */
class ActConfigProfile : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, intelligent_endpoint, IntelligentEndpoint);  ///< The endpoint of the AI server
  ACT_JSON_FIELD(QString, intelligent_local_endpoint,
                 IntelligentLocalEndpoint);  ///< The local endpoint of the AI server

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new Act License object
   *
   */
  ActConfigProfile() {
    this->intelligent_endpoint_ = GetIntelligentEndpointFromEnviron();
    this->intelligent_local_endpoint_ = GetIntelligentLocalEndpointFromEnviron();
    this->key_order_.append(QList<QString>({QString("IntelligentEndpoint"), QString("IntelligentLocalEndpoint")}));
  }
};

/**
 * @brief The ACT system class
 *
 */
class ActSystem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(QString, act_version, ActVersion);       ///< act version item
  ACT_JSON_FIELD(QString, data_version, DataVersion);     ///< The version of the class
  ACT_JSON_FIELD(bool, auto_save, AutoSave);              ///< Indicate the auto save feature
  ACT_JSON_FIELD(quint64, idle_timeout, IdleTimeout);     ///< Indicate the idle timeout in minute
  ACT_JSON_FIELD(quint64, hard_timeout, HardTimeout);     ///< Indicate the idle timeout in minute
  ACT_JSON_FIELD(quint16, max_token_size, MaxTokenSize);  ///< The maximum size of tokens
  ACT_JSON_FIELD(QString, serial_number, SerialNumber);
  ACT_JSON_FIELD(QString, intelligent_endpoint, IntelligentEndpoint);  ///< The endpoint of the AI server
  ACT_JSON_FIELD(QString, intelligent_local_endpoint,
                 IntelligentLocalEndpoint);                ///< The local endpoint of the AI server
  ACT_JSON_FIELD(quint32, log_min_free_gb, LogMinFreeGb);  // Minimum free GB for logs
  ACT_JSON_FIELD(quint32, log_keep_days, LogKeepDays);     // Log retention days

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act License object
   *
   */
  ActSystem() {
    this->data_version_ = ACT_SYSTEM_DATA_VERSION;
    this->auto_save_ = true;
    this->idle_timeout_ = ACT_IDLE_TIMEOUT;
    this->hard_timeout_ = ACT_HARD_TIMEOUT;
    this->max_token_size_ = ACT_TOKEN_MAX_SIZE;
    this->intelligent_endpoint_ = "";
    this->intelligent_local_endpoint_ = "";
    this->log_min_free_gb_ = ACT_LOG_MIN_FREE_GB;  // Default minimum free GB for logs
    this->log_keep_days_ = ACT_LOG_KEEP_DAYS;      // Default log retention days
    this->key_order_.append(
        QList<QString>({QString("ActVersion"), QString("DataVersion"), QString("AutoSave"), QString("IdleTimeout"),
                        QString("HardTimeout"), QString("MaxTokenSize"), QString("SerialNumber")}));
    this->key_order_.append(QList<QString>({QString("LogMinFreeGb"), QString("LogKeepDays")}));
  }

  /**
   * @brief Construct a new Act System object via QString JSON
   *
   * @param json
   */
  ActSystem(QString json) : ActSystem() { this->FromString(json); }

  /**
   * @brief Get the Act Version from version.txt
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetActVersionFromFile() {
    ACT_STATUS_INIT();

    // Open the version.txt file
    QFile file(ACT_VERSION_FILE_NAME);
    if (!file.open(QIODevice::ReadOnly)) {
      qCritical() << "Open version.txt file failed";
      return std::make_shared<ActStatusInternalError>("System");
    }

    // Read all contents at one time
    QString content = file.readAll();

    // Close the version.txt file
    file.close();

    this->SetActVersion(content);

    return act_status;
  }

  /**
   * @brief Get the Act Version directly
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetActVersionDirectly(QString version, QString revision, QString buildTime) {
    ACT_STATUS_INIT();

    QString actVersion = QString("V%1_Build_%2 (Rev. %3)").arg(version).arg(buildTime).arg(revision);
    this->SetActVersion(actVersion);

    return act_status;
  }

  /**
   * @brief Get the Act ConfigProfile directly
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetActConfigProfileDirectly() {
    ACT_STATUS_INIT();

    ActConfigProfile config_profile;

    this->SetIntelligentEndpoint(config_profile.GetIntelligentEndpoint());
    this->SetIntelligentLocalEndpoint(config_profile.GetIntelligentLocalEndpoint());

    return act_status;
  }
};

static QString GetActAppName() {
  // ACT_APP_NAME
  QString appName = qEnvironmentVariable("CHAMBERLAIN_APPLICATION_NAME", "Chamberlain");
  return appName;
}

static QString GetAfdFilePath() {
  QString defaultPath = ".";
  QString filePath =
      qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_AFD_FILE", defaultPath + QDir::separator() + "afd.exe");
  return filePath;
}

static QString GetDeviceConfigFilePath() {
  // ACT_DEVICE_CONFIG_FILE_FOLDER
  QString tempPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_TMP_FOLDER", "tmp");
  return tempPath + QDir::separator() + "device_config";
}

static QString GetDefaultDeviceProfilePath() {
  // ACT_DEFAULT_DEVICE_PROFILE_FOLDER
  QString confPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return confPath + QDir::separator() + "configuration" + QDir::separator() + "default_device_profile";
}

static QString GetDeviceIconPath() {
  // ACT_DEVICE_ICON_FOLDER
  QString confPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return confPath + QDir::separator() + "configuration" + QDir::separator() + "device_icon";
}

static QString GetDeviceProfilePath() {
  // ACT_DEVICE_PROFILE_FOLDER
  QString confPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return confPath + QDir::separator() + "configuration" + QDir::separator() + "device_profile";
}

static QString GetEthernetModulePath() {
  // ACT_ETHERNET_MODULE_FOLDER
  QString confPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return confPath + QDir::separator() + "configuration" + QDir::separator() + "ethernet_module";
}

static QString GetFeatureProfilePath() {
  // ACT_FEATURE_PROFILE_FOLDER
  QString confPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return confPath + QDir::separator() + "configuration" + QDir::separator() + "feature_profile";
}

static QString GetFirmwareFeatureProfile() {
  // ACT_FIRMWARE_FEATURE_PROFILE_FOLDER
  QString confPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return confPath + QDir::separator() + "configuration" + QDir::separator() + "firmware_feature_profile";
}

static QString GetGeneralProfilePath() {
  // ACT_GENERAL_PROFILE_FOLDER
  QString confPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return confPath + QDir::separator() + "configuration" + QDir::separator() + "general_profile";
}

static QString GetIntelligentFilePath() {
  // ACT_INTELLIGENT_FILE_FOLDER
  QString confPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return confPath + QDir::separator() + "db" + QDir::separator() + "intelligent";
}

static QString GetGenerateOfflineConfigPath() {
  // ACT_GENERATE_OFFLINE_CONFIG_FOLDER
  QString tempPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_TMP_FOLDER", "tmp");
  return tempPath + QDir::separator() + "offline_config";
}

static QString GetEncryptedLicenseFilePath() {
  // ACT_ENCRYPTED_LICENSE_FILE_NAME
  QString confPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return confPath + QDir::separator() + ACT_ENCRYPTED_LICENSE_FILE_NAME;
}

static QString GetLogPath() { return qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_LOG_FOLDER", ACT_LOG_FOLDER); }

static QString GetMafInternalApiUrl() {
  QString apiAddr = qEnvironmentVariable("MAF_API_ADDR", "localhost");
  QString apiPort = qEnvironmentVariable("MAF_API_PORT", "59000");
  return "http://" + apiAddr + ":" + apiPort + "/";
}

static QString GetMainHttpsUrl() {
  // https://localhost:8443
  QString httpsPort = qEnvironmentVariable("CHAMBERLAIN_MAIN_HTTP_PORT", "8443");
  return "https://localhost:" + httpsPort;
}

static QString GetMainWssUrl() {
  // wss://localhost:8443
  QString httpsPort = qEnvironmentVariable("CHAMBERLAIN_MAIN_HTTP_PORT", "8443");
  return "wss://localhost:" + httpsPort;
}

static QString GetSebastianProgram() {
  // SEBASTIAN_PROGRAM
  QString appName = qEnvironmentVariable("CHAMBERLAIN_APPLICATION_NAME", "Chamberlain");
  return appName + ".Sebastian.exe";
}

static QString GetSfpModulePath() {
  // ACT_SFP_MODULE_FOLDER
  QString confPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return confPath + QDir::separator() + "configuration" + QDir::separator() + "sfp_module";
}

static QString GetPowerModulePath() {
  // ACT_POWER_MODULE_FOLDER
  QString confPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return confPath + QDir::separator() + "configuration" + QDir::separator() + "power_module";
}

static QString GetPowerDeviceProfilePath() {
  // ACT_POWER_DEVICE_PROFILE_FOLDER
  QString confPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return confPath + QDir::separator() + "configuration" + QDir::separator() + "power_device_profile";
}

static QString GetSoftwareLicenseProfilePath() {
  // ACT_SOFTWARE_LICENSE_PROFILE_FOLDER
  QString confPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", ".");
  return confPath + QDir::separator() + "configuration" + QDir::separator() + "software_license_profile";
}