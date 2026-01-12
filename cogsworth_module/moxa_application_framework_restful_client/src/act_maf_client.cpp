#include "act_maf_client_handler.hpp"

act::mafClient::ActMafClient::ActMafClient() {
  auto objects_created =
      oatpp::base::Environment::getObjectsCreated();  // get count of objects created for a whole system lifetime

  if (objects_created == 0) {
    qDebug() << __func__ << "oatpp::base::Environment::init()";
    oatpp::base::Environment::init();
  }
}

act::mafClient::ActMafClient::~ActMafClient() {}

ACT_STATUS act::mafClient::ActMafClient::GetEvents(const qint32 limit, const qint32 offset, const QString &base_url,
                                                   const QString &proxy, ActEventLogsResponse &response) {
  ACT_STATUS_INIT();

  ActMafClientAgent client_agent(base_url, proxy);

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  act_status = client_agent.GetEvents(limit, offset, response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetEvents() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::mafClient::ActMafClient::SaveEventsAsCsv(const QString &file_path, const QString &base_url,
                                                         const QString &proxy) {
  ACT_STATUS_INIT();

  ActMafClientAgent client_agent(base_url, proxy);

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  QString content;
  act_status = client_agent.GetEventCsvContent(content);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetEventCsvContent() failed.";
    return act_status;
  }

  // save to csv file (specified file path/snmp-trap-events_yyyyMMdd_HHmmss.csv)
  QString file_name(file_path);
  file_name.append("/");
  file_name.append(ACT_SNMP_TRAP_EVENT_LOG_CSV_NAME);
  file_name.append("_");
  // Get the current date and time
  QString dateTime = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
  file_name.append(dateTime);
  file_name.append(".csv");

  QFile file(file_name);

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qCritical() << __func__ << "Failed to open file for writing.";
    return std::make_shared<ActStatusInternalError>("Open csv file fail");
  }

  QTextStream out(&file);
  out << content;
  file.close();

  return act_status;
}

ACT_STATUS act::mafClient::ActMafClient::GetSyslogs(const ActSyslogQueryData &query_data, const QString &base_url,
                                                    const QString &proxy, ActSyslogsResponse &response) {
  ACT_STATUS_INIT();

  ActMafClientAgent client_agent(base_url, proxy);

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  act_status = client_agent.GetSyslogs(query_data, response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetSyslogs() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::mafClient::ActMafClient::SaveSyslogsAsCsv(const ActSyslogQueryData &query_data,
                                                          const QString &file_path, const QString &base_url,
                                                          const QString &proxy) {
  ACT_STATUS_INIT();

  ActMafClientAgent client_agent(base_url, proxy);

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  QString content;
  act_status = client_agent.GetSyslogCsvContent(query_data, content);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetEventCsvContent() failed.";
    return act_status;
  }

  // check file path is exist
  QDir dir(file_path);
  if (!dir.exists()) {
    qCritical() << __func__ << "Directory does not exist:" << file_path;
    return std::make_shared<ActStatusInternalError>("Directory does not exist");
  }
  // file name: specified file path/syslogs_yyyyMMdd_HHmmss.csv
  QString file_name(file_path);
  file_name.append("/");
  file_name.append(ACT_SYSLOG_CSV_NAME);
  file_name.append("_");
  // Get the current date and time
  QString dateTime = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
  file_name.append(dateTime);
  file_name.append(".csv");

  QFile file(file_name);

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qCritical() << __func__ << "Failed to open file for writing.";
    return std::make_shared<ActStatusInternalError>("Open csv file fail");
  }

  QTextStream out(&file);
  out << content;
  file.close();

  return act_status;
}

ACT_STATUS act::mafClient::ActMafClient::DeleteSyslogs(const QString &base_url, const QString &proxy,
                                                       ActDeleteSyslogsResponse &response) {
  ACT_STATUS_INIT();

  ActMafClientAgent client_agent(base_url, proxy);

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  act_status = client_agent.DeleteSyslogs(response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "DeleteSyslogs() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::mafClient::ActMafClient::GetSyslogConfiguration(const QString &base_url, const QString &proxy,
                                                                ActSyslogGetConfiguration &response) {
  ACT_STATUS_INIT();

  ActMafClientAgent client_agent(base_url, proxy);

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  MafSyslogConfiguration maf_response;
  act_status = client_agent.GetSyslogConfiguration(maf_response);

  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetSyslogConfiguration() failed.";
    return act_status;
  }

  response.Setenable(maf_response.Getdata().Getenable());
  response.Setport(maf_response.Getdata().Getport());

  return act_status;
}

ACT_STATUS act::mafClient::ActMafClient::PutSyslogConfiguration(const ActSyslogPutConfiguration &request,
                                                                const QString &base_url, const QString &proxy,
                                                                ActSyslogPutConfiguration &response) {
  ACT_STATUS_INIT();

  ActMafClientAgent client_agent(base_url, proxy);

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  MafSyslogConfiguration maf_request = MafSyslogConfiguration(request.Getenable());
  MafSyslogConfiguration maf_response;
  act_status = client_agent.PutSyslogConfiguration(maf_request, maf_response);

  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "PutSyslogConfiguration() failed.";
    return act_status;
  }

  response.Setenable(maf_response.Getdata().Getenable());

  return act_status;
}

ACT_STATUS act::mafClient::ActMafClient::PostOfflineConfig(const QJsonArray &feature_list, QJsonObject &secret_setting,
                                                           const MafGenOfflineConfigRequest &request,
                                                           const QString &base_url, const QString &proxy,
                                                           MafGenOfflineConfigResponse &response) {
  ACT_STATUS_INIT();

  ActMafClientAgent client_agent(base_url, proxy);

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  act_status = client_agent.PostOfflineConfig(feature_list, secret_setting, request, response);

  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "PostOfflineConfig() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::mafClient::ActMafClient::PostExportOfflineConfigs(const MafExportFilesRequest &request,
                                                                  const QString &base_url, const QString &proxy) {
  ACT_STATUS_INIT();

  ActMafClientAgent client_agent(base_url, proxy);

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  act_status = client_agent.PostExportOfflineConfigs(request);

  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "PostExportFiles() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::mafClient::ActMafClient::ClearOfflineConfigFiles(const MafDeleteFilesRequest &request,
                                                                 const QString &base_url, const QString &proxy) {
  ACT_STATUS_INIT();

  ActMafClientAgent client_agent(base_url, proxy);

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  act_status = client_agent.ClearOfflineConfigFiles(request);

  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ClearOfflineConfigFiles() failed.";
    return act_status;
  }

  return act_status;
}