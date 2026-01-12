/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include <QString>

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/web/client/ApiClient.hpp"

class ActMafClient : public oatpp::web::client::ApiClient {
#include OATPP_CODEGEN_BEGIN(ApiClient)

  API_CLIENT_INIT(ActMafClient)

  // event log
  API_CALL("GET", "events", DoGetEvents, QUERY(Int32, limit), QUERY(Int32, offset), QUERY(String, order),
           QUERY(String, sort), QUERY(String, categories), QUERY(String, names), QUERY(String, severities),
           QUERY(String, sources), QUERY(String, download))
  API_CALL("GET", "syslogs", DoGetSyslogs, QUERY(Int32, limit), QUERY(Int32, offset), QUERY(String, download),
           QUERY(String, starttime), QUERY(String, endtime), QUERY(String, ipaddress), QUERY(String, facilities),
           QUERY(String, severities))
  API_CALL("DELETE", "syslogs", DoDeleteSyslogs)
  API_CALL("GET", "syslogs/configuration", DoGetSyslogConfiguration)
  API_CALL("PUT", "syslogs/configuration", DoPutSyslogConfiguration, BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  // offline config
  API_CALL("POST", "dm-api/v1/devices/offline-configuration", DoPostOfflineConfig,
           BODY_DTO(oatpp::Fields<oatpp::Any>, body))

  // storage
  API_CALL("POST", "storage/files/export", DoPostExportFiles, BODY_STRING(oatpp::String, body))
  API_CALL("DELETE", "storage/files", DoDeleteFiles, BODY_STRING(oatpp::String, body))
#include OATPP_CODEGEN_END(ApiClient)
};
