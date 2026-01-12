/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include <QTextStream>

#include "act_event_log.hpp"
#include "act_maf_client.hpp"
#include "act_maf_client_agent.hpp"
#include "act_status.hpp"
/**
 * @brief The restful client class for moxa application framework
 *
 */
namespace act {
namespace mafClient {
class ActMafClient {
 private:
  const QString ACT_SNMP_TRAP_EVENT_LOG_CSV_NAME = "snmp-trap-events";
  const QString ACT_SYSLOG_CSV_NAME = "syslogs";

 public:
  ActMafClient();

  ~ActMafClient();

  // event log
  ACT_STATUS GetEvents(const qint32 limit, const qint32 offset, const QString &base_url, const QString &proxy,
                       ActEventLogsResponse &response);
  ACT_STATUS SaveEventsAsCsv(const QString &file_path, const QString &base_url, const QString &proxy);
  ACT_STATUS GetSyslogs(const ActSyslogQueryData &query_data, const QString &base_url, const QString &proxy,
                        ActSyslogsResponse &response);
  ACT_STATUS SaveSyslogsAsCsv(const ActSyslogQueryData &query_data, const QString &file_path, const QString &base_url,
                              const QString &proxy);
  ACT_STATUS DeleteSyslogs(const QString &base_url, const QString &proxy, ActDeleteSyslogsResponse &response);
  ACT_STATUS GetSyslogConfiguration(const QString &base_url, const QString &proxy, ActSyslogGetConfiguration &response);
  ACT_STATUS PutSyslogConfiguration(const ActSyslogPutConfiguration &request, const QString &base_url,
                                    const QString &proxy, ActSyslogPutConfiguration &response);

  // offline config
  ACT_STATUS PostOfflineConfig(const QJsonArray &feature_list, QJsonObject &secret_setting,
                               const MafGenOfflineConfigRequest &request, const QString &base_url, const QString &proxy,
                               MafGenOfflineConfigResponse &response);

  // storage
  ACT_STATUS PostExportOfflineConfigs(const MafExportFilesRequest &request, const QString &base_url,
                                      const QString &proxy);
  ACT_STATUS ClearOfflineConfigFiles(const MafDeleteFilesRequest &request, const QString &base_url,
                                     const QString &proxy);
};
}  // namespace mafClient
}  // namespace act