#include <QHostInfo>
#include <QNetworkInterface>
#include <QSysInfo>
#include <QUrl>

#include "act_core.hpp"
#include "act_maf_client_handler.hpp"
#include "act_system.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::GetEventLogs(qint32 limit, qint32 offset, ActEventLogsResponse &response) {
  ACT_STATUS_INIT();

  QString internal_api_endpoint = GetMafInternalApiUrl();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::mafClient::ActMafClient maf_client;
  act_status = maf_client.GetEvents(limit, offset, internal_api_endpoint, http_proxy_endpoint, response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "GetEvents() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::SaveEventsAsCsv(const QString &file_path) {
  ACT_STATUS_INIT();

  QString internal_api_endpoint = GetMafInternalApiUrl();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::mafClient::ActMafClient maf_client;
  act_status = maf_client.SaveEventsAsCsv(file_path, internal_api_endpoint, http_proxy_endpoint);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SaveEventsAsCsv() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::GetSyslogs(ActSyslogQueryData &query_data, ActSyslogsResponse &response) {
  ACT_STATUS_INIT();

  QString internal_api_endpoint = GetMafInternalApiUrl();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::mafClient::ActMafClient maf_client;
  act_status = maf_client.GetSyslogs(query_data, internal_api_endpoint, http_proxy_endpoint, response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "GetSyslogs() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::SaveSyslogsAsCsv(ActSyslogQueryData &query_data, const QString &file_path) {
  ACT_STATUS_INIT();

  QString internal_api_endpoint = GetMafInternalApiUrl();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::mafClient::ActMafClient maf_client;
  act_status = maf_client.SaveSyslogsAsCsv(query_data, file_path, internal_api_endpoint, http_proxy_endpoint);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SaveSyslogsAsCsv() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::SaveSyslogsAsCsv(const QString &file_path) {
  ACT_STATUS_INIT();

  QString internal_api_endpoint = GetMafInternalApiUrl();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::mafClient::ActMafClient maf_client;
  ActSyslogQueryData query_data = ActSyslogQueryData(0, 0);
  act_status = maf_client.SaveSyslogsAsCsv(query_data, file_path, internal_api_endpoint, http_proxy_endpoint);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SaveSyslogsAsCsv() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::DeleteSyslogs(ActDeleteSyslogsResponse &response) {
  ACT_STATUS_INIT();

  QString internal_api_endpoint = GetMafInternalApiUrl();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::mafClient::ActMafClient maf_client;
  act_status = maf_client.DeleteSyslogs(internal_api_endpoint, http_proxy_endpoint, response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "DeleteSyslogs() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::GetSyslogConfiguration(ActSyslogGetConfiguration &response) {
  ACT_STATUS_INIT();

  QString internal_api_endpoint = GetMafInternalApiUrl();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::mafClient::ActMafClient maf_client;
  act_status = maf_client.GetSyslogConfiguration(internal_api_endpoint, http_proxy_endpoint, response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "DeleteSyslogs() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::SetSyslogConfiguration(ActSyslogPutConfiguration &request, ActSyslogPutConfiguration &response) {
  ACT_STATUS_INIT();

  QString internal_api_endpoint = GetMafInternalApiUrl();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::mafClient::ActMafClient maf_client;
  act_status = maf_client.PutSyslogConfiguration(request, internal_api_endpoint, http_proxy_endpoint, response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "DeleteSyslogs() failed.";
    return act_status;
  }

  return act_status;
}

}  // namespace core
}  // namespace act