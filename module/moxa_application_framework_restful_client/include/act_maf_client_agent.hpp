#pragma once

#include <QDebug>
#include <QQueue>
#include <QString>
#include <string>

#include "act_device.hpp"
#include "act_event_log.hpp"
#include "act_maf_client.hpp"
#include "act_status.hpp"
#include "oatpp-curl/ProxyRequestExecutor.hpp"
#include "oatpp/core/data/resource/InMemoryData.hpp"
#include "oatpp/core/data/stream/BufferStream.hpp"
#include "oatpp/core/data/stream/FileStream.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/client/HttpRequestExecutor.hpp"
#include "oatpp/web/mime/multipart/PartList.hpp"
#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/web/protocol/http/outgoing/MultipartBody.hpp"

#define ACT_RESTFUL_CLIENT_REQUEST_SUCCESS (200)  /// < The restful request status success  code
#define ACT_RESTFUL_CLIENT_REQUEST_NO_CONTENT (204)

class MafSyslogRotateConfig : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, desiredDiskCacheSizeMB, desiredDiskCacheSizeMB);
  ACT_JSON_FIELD(qint32, alertThreshold, alertThreshold);

 public:
  MafSyslogRotateConfig() {
    this->desiredDiskCacheSizeMB_ = 512;
    this->alertThreshold_ = 80;
  }
};

class MafSyslogConfigurationData : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_FIELD(QString, host, host);
  ACT_JSON_FIELD(qint32, port, port);
  ACT_JSON_OBJECT(MafSyslogRotateConfig, rotate, rotate);

 public:
  MafSyslogConfigurationData() {
    this->enable_ = true;
    this->host_ = "0.0.0.0";
    this->port_ = 514;
    MafSyslogRotateConfig rotate;
    this->rotate_ = rotate;
  }
  MafSyslogConfigurationData(bool enable) {
    this->enable_ = enable;
    this->host_ = "0.0.0.0";
    this->port_ = 514;
    MafSyslogRotateConfig rotate;
    this->rotate_ = rotate;
  }
};

class MafSyslogConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(MafSyslogConfigurationData, data, data);

 public:
  MafSyslogConfiguration() {
    MafSyslogConfigurationData data;
    this->data_ = data;
  }
  MafSyslogConfiguration(bool enable) {
    MafSyslogConfigurationData data = MafSyslogConfigurationData(enable);
    this->data_ = data;
  }
};

class MafOfflineConfigProperties : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, deviceID, deviceID);
};
class MafGenOfflineConfigRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, fileName, fileName);
  ACT_JSON_FIELD(QString, modelName, modelName);
  ACT_JSON_FIELD(QString, exportType, exportType);
  ACT_JSON_OBJECT(MafOfflineConfigProperties, properties, properties);
  // feature list is passed via function parameters; ACT JSON is not defined here.

  ACT_JSON_FIELD(QString, targetIP, targetIP);
};

class MafOfflineConfigRespData : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, code, code);
  ACT_JSON_FIELD(QString, result, result);
  ACT_JSON_FIELD(QString, fileId, fileId);
};

class MafGenOfflineConfigResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(MafOfflineConfigRespData, data, data);
};
class MafExportFilesProperties : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, QString, deviceID, deviceID);
  ACT_JSON_COLLECTION(QList, QString, origin, origin);
};

class MafExportFilesRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(MafExportFilesProperties, properties, properties);
  ACT_JSON_FIELD(QString, exportType, exportType);

 public:
  MafExportFilesRequest() {
    const QString EXPORT_TYPE_ZIP = "zip";
    exportType_ = EXPORT_TYPE_ZIP;
  }
};

class MafDeleteFilesProperties : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, QString, origin, origin);
};

class MafDeleteFilesRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(MafDeleteFilesProperties, properties, properties);
};

class ActMafClientAgent {
 private:
  QString token_;
  QString base_url_;
  QString proxy_;
  std::shared_ptr<ActMafClient> client_;

  // offline config filepath
  QString offlineConfigFilePath = GetDeviceConfigFilePath();
  QString offlineConfigFileName = (QString(MAF_EXPORT_CONFIG_FILE_NAME));
  QString offlineConfigPath = (QString("%1/%2").arg(offlineConfigFilePath).arg(offlineConfigFileName));

  std::string offlineConfigFullPath = offlineConfigPath.toStdString();

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
      std::string baseUrl = base_url_.toStdString();
      std::string proxyUrl = proxy_.toStdString();
      std::shared_ptr<oatpp::web::client::RequestExecutor> requestExecutor =
          oatpp::curl::ProxyRequestExecutor::createShared(baseUrl, proxyUrl);
      client_ = ActMafClient::createShared(requestExecutor, objectMapper);
    } catch (std::exception &e) {
      qCritical() << __func__ << "Create the Act MAF client failed. Error:" << e.what();
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

    // Check request is success or no content
    auto response_status_code = response->getStatusCode();
    if ((response_status_code == ACT_RESTFUL_CLIENT_REQUEST_SUCCESS) ||
        (response_status_code == ACT_RESTFUL_CLIENT_REQUEST_NO_CONTENT)) {
      // Success response, nothing to do
    } else {
      // Print request
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

      qDebug() << called_func << "Response Status(" << response_status_code
               << "):" << response->getStatusDescription()->c_str();

      QString response_body = response->readBodyToString()->c_str();
      qDebug() << called_func
               << QString("MAF server(%1) reply failed. response_body: %2")
                      .arg(base_url_)
                      .arg(response_body)
                      .toStdString()
                      .c_str();

      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

 public:
  ActMafClientAgent(const QString &base_url, const QString &proxy) {
    base_url_ = base_url;
    proxy_ = proxy;
  }

  ACT_STATUS Init() {
    ACT_STATUS_INIT();
    act_status = CreateRestfulClient();
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
    return act_status;
  }

  // event log

  ACT_STATUS GetEvents(qint32 limit, qint32 offset, ActEventLogsResponse &response) {
    ACT_STATUS_INIT();

    const std::string ORDER = "time";
    const std::string SORT = "desc";
    // filter conditions
    const std::string CATEGORIES = "";                         // all events
    const std::string NAMES = "";                              // all events
    const std::string SEVERITIES = "inform,warning,critical";  // all events
    const std::string SOURCES = "SNMP%20Trap";                 // only SNMP trap events
    // download csv file (get csv file content)
    const std::string DOWNLOAD = "false";

    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> temp_response;

      temp_response =
          client_->DoGetEvents(limit, offset, ORDER, SORT, CATEGORIES, NAMES, SEVERITIES, SOURCES, DOWNLOAD);

      // Check success
      act_status = CheckResponseStatus(__func__, temp_response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      auto response_status_code = temp_response->getStatusCode();

      QString response_body = temp_response->readBodyToString()->c_str();

      response.FromString(response_body);

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  ACT_STATUS GetEventCsvContent(QString &csv_content) {
    ACT_STATUS_INIT();

    const qint32 LIMIT = 0;   // all events
    const qint32 OFFSET = 0;  // all events

    const std::string ORDER = "time";
    const std::string SORT = "desc";
    // filter conditions
    const std::string CATEGORIES = "";                         // all events
    const std::string NAMES = "";                              // all events
    const std::string SEVERITIES = "inform,warning,critical";  // all events
    const std::string SOURCES = "SNMP%20Trap";                 // only SNMP trap events
    // download csv file (get csv file content)
    const std::string DOWNLOAD = "true";

    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> temp_response;

      temp_response =
          client_->DoGetEvents(LIMIT, OFFSET, ORDER, SORT, CATEGORIES, NAMES, SEVERITIES, SOURCES, DOWNLOAD);

      // Check success
      act_status = CheckResponseStatus(__func__, temp_response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      auto response_status_code = temp_response->getStatusCode();

      csv_content = temp_response->readBodyToString()->c_str();

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  ACT_STATUS GetSyslogs(const ActSyslogQueryData &query_data, ActSyslogsResponse &response) {
    ACT_STATUS_INIT();

    qint32 limit = query_data.GetLimit();
    qint32 offset = query_data.GetOffset();

    // download csv file (get csv file content)
    const std::string DOWNLOAD = "false";

    oatpp::String starttime = oatpp::String(query_data.GetStarttime().toUtf8().constData());
    oatpp::String endtime = oatpp::String(query_data.GetEndtime().toUtf8().constData());
    oatpp::String ip_address = oatpp::String(query_data.GetIpaddress().toUtf8().constData());
    oatpp::String facilities = oatpp::String(query_data.GetFacilities().toUtf8().constData());
    oatpp::String severities = oatpp::String(query_data.GetSeverities().toUtf8().constData());

    if (starttime == "-1") {
      starttime = nullptr;
    }
    if (endtime == "-1") {
      endtime = nullptr;
    }
    if (ip_address == "-1") {
      ip_address = nullptr;
    }
    if (facilities == "-1") {
      facilities = nullptr;
    }
    if (severities == "-1") {
      severities = nullptr;
    }

    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> temp_response;

      temp_response =
          client_->DoGetSyslogs(limit, offset, DOWNLOAD, starttime, endtime, ip_address, facilities, severities);

      // Check success
      act_status = CheckResponseStatus(__func__, temp_response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      auto response_status_code = temp_response->getStatusCode();

      QString response_body = temp_response->readBodyToString()->c_str();

      response.FromString(response_body);

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  ACT_STATUS GetSyslogCsvContent(const ActSyslogQueryData &query_data, QString &csv_content) {
    ACT_STATUS_INIT();

    qint32 limit = query_data.GetLimit();
    qint32 offset = query_data.GetOffset();

    oatpp::String starttime = oatpp::String(query_data.GetStarttime().toUtf8().constData());
    oatpp::String endtime = oatpp::String(query_data.GetEndtime().toUtf8().constData());
    oatpp::String ip_address = oatpp::String(query_data.GetIpaddress().toUtf8().constData());
    oatpp::String facilities = oatpp::String(query_data.GetFacilities().toUtf8().constData());
    oatpp::String severities = oatpp::String(query_data.GetSeverities().toUtf8().constData());

    if (starttime == "-1") {
      starttime = nullptr;
    }
    if (endtime == "-1") {
      endtime = nullptr;
    }
    if (ip_address == "-1") {
      ip_address = nullptr;
    }
    if (facilities == "-1") {
      facilities = nullptr;
    }
    if (severities == "-1") {
      severities = nullptr;
    }

    // download csv file (get csv file content)
    const std::string DOWNLOAD = "true";

    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> temp_response;

      temp_response =
          client_->DoGetSyslogs(limit, offset, DOWNLOAD, starttime, endtime, ip_address, facilities, severities);

      // Check success
      act_status = CheckResponseStatus(__func__, temp_response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      auto response_status_code = temp_response->getStatusCode();

      csv_content = temp_response->readBodyToString()->c_str();

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  ACT_STATUS DeleteSyslogs(ActDeleteSyslogsResponse &response) {
    ACT_STATUS_INIT();
    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> temp_response;

      temp_response = client_->DoDeleteSyslogs();

      // Check success
      act_status = CheckResponseStatus(__func__, temp_response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      auto response_status_code = temp_response->getStatusCode();

      QString response_body = temp_response->readBodyToString()->c_str();

      response.FromString(response_body);

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  ACT_STATUS GetSyslogConfiguration(MafSyslogConfiguration &response) {
    ACT_STATUS_INIT();
    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> temp_response;

      temp_response = client_->DoGetSyslogConfiguration();

      // Check success
      act_status = CheckResponseStatus(__func__, temp_response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      auto response_status_code = temp_response->getStatusCode();

      QString response_body = temp_response->readBodyToString()->c_str();

      response.FromString(response_body);

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  ACT_STATUS PutSyslogConfiguration(const MafSyslogConfiguration &request, MafSyslogConfiguration &response) {
    ACT_STATUS_INIT();
    try {
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();

      // could change enabled only
      request_dto["enable"] = oatpp::Boolean(request.Getdata().Getenable());
      request_dto["host"] = oatpp::String("0.0.0.0");
      request_dto["port"] = oatpp::Int32(514);

      auto rotate_dto = oatpp::Fields<oatpp::Any>::createShared();
      rotate_dto["desiredDiskCacheSizeMB"] = oatpp::Int32(512);
      rotate_dto["alertThreshold"] = oatpp::Int32(80);

      request_dto["rotate"] = rotate_dto;

      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> temp_response;
      temp_response = client_->DoPutSyslogConfiguration(request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, temp_response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      auto response_status_code = temp_response->getStatusCode();

      QString response_body = temp_response->readBodyToString()->c_str();

      response.FromString(response_body);

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  void clearSensitiveKeys(QJsonObject &obj, const QStringList &sensitiveKeys) {
    for (auto it = obj.begin(); it != obj.end(); ++it) {
      if (it->isObject()) {
        // Recursively process nested objects (e.g., snmp, https, etc.)
        QJsonObject nested = it->toObject();
        clearSensitiveKeys(nested, sensitiveKeys);
        it.value() = nested;
      } else {
        // Clear the value if the key is in the sensitive list
        if (sensitiveKeys.contains(it.key())) {
          it.value() = "";
        }
      }
    }
  }

  // offline config
  ACT_STATUS PostOfflineConfig(const QJsonArray &feature_list, QJsonObject &secret_setting,
                               const MafGenOfflineConfigRequest &request, MafGenOfflineConfigResponse &response) {
    ACT_STATUS_INIT();
    try {
      auto request_dto = oatpp::Fields<oatpp::Any>::createShared();

      request_dto["fileName"] = oatpp::String(request.GetfileName().toStdString().c_str());
      request_dto["modelName"] = oatpp::String(request.GetmodelName().toStdString().c_str());
      request_dto["exportType"] = oatpp::String(request.GetexportType().toStdString().c_str());

      auto prop_dto = oatpp::Fields<oatpp::Any>::createShared();
      prop_dto["deviceID"] = oatpp::String(request.Getproperties().GetdeviceID().toStdString().c_str());

      request_dto["properties"] = prop_dto;

      // Transfer to JSON string > QString > DTO
      QString feature_list_str = QJsonDocument(feature_list).toJson();

      auto feature_list_dto = oatpp::List<oatpp::Any>::createShared();
      auto feature_list_object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      feature_list_dto =
          feature_list_object_mapper->readFromString<oatpp::List<oatpp::Any>>(feature_list_str.toStdString().c_str());
      request_dto["featureList"] = feature_list_dto;

      // target IP
      request_dto["targetIP"] = oatpp::String(request.GettargetIP().toStdString().c_str());

      // secret
      // Transfer to JSON string > QString > DTO
      QString secret_str = QJsonDocument(secret_setting).toJson();

      auto secret_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto secret_object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      secret_dto = secret_object_mapper->readFromString<oatpp::Fields<oatpp::Any>>(secret_str.toStdString().c_str());
      request_dto["secret"] = secret_dto;

      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> temp_response;
      temp_response = client_->DoPostOfflineConfig(request_dto);

      // DEBUG MSG: print request body to MAF
      // hide password & key
      QStringList sensitiveKeys = {"password", "authPassword", "dataEncryptKey", "readCommunity", "writeCommunity"};
      clearSensitiveKeys(secret_setting, sensitiveKeys);

      QString log_secret_str = QJsonDocument(secret_setting).toJson();

      auto log_secret_dto = oatpp::Fields<oatpp::Any>::createShared();
      auto log_secret_object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      log_secret_dto =
          log_secret_object_mapper->readFromString<oatpp::Fields<oatpp::Any>>(log_secret_str.toStdString().c_str());
      request_dto["secret"] = log_secret_dto;

      auto log_object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      auto log_json = log_object_mapper->writeToString(request_dto);
      qDebug() << "request body: " << log_json->c_str();

      // Check success
      act_status = CheckResponseStatus(__func__, temp_response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      auto response_status_code = temp_response->getStatusCode();
      QString response_body = temp_response->readBodyToString()->c_str();

      response.FromString(response_body);

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  // storage
  ACT_STATUS PostExportOfflineConfigs(const MafExportFilesRequest &request) {
    ACT_STATUS_INIT();
    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> temp_response;
      oatpp::String request_str = request.ToString().toStdString();
      temp_response = client_->DoPostExportFiles(request_str);

      // DEBUG MSG
      qDebug() << "export offline configs request body" << request.ToString().toStdString().c_str();

      // save file
      oatpp::data::stream::FileOutputStream fileOut(offlineConfigFullPath.c_str());
      temp_response->transferBodyToStream(&fileOut);

      // Check success
      act_status = CheckResponseStatus(__func__, temp_response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      // DEBUG MSG
      qDebug() << "PostExportOfflineConfigs Success";

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  ACT_STATUS ClearOfflineConfigFiles(const MafDeleteFilesRequest &request) {
    ACT_STATUS_INIT();
    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> temp_response;
      oatpp::String request_str = request.ToString().toStdString();
      temp_response = client_->DoDeleteFiles(request_str);

      // DEBUG MSG
      qDebug() << "clear offline configs request body" << request.ToString().toStdString().c_str();

      // Check success
      act_status = CheckResponseStatus(__func__, temp_response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      // DEBUG MSG
      qDebug() << "ClearOfflineConfigFiles Success";

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }
};
