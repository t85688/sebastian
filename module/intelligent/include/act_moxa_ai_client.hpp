/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include <QString>

#include "act_json.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/web/client/ApiClient.hpp"
#include "oatpp/web/protocol/http/outgoing/MultipartBody.hpp"

class ActMoxaAIClient : public oatpp::web::client::ApiClient {
#include OATPP_CODEGEN_BEGIN(ApiClient)

  API_CLIENT_INIT(ActMoxaAIClient)

  API_CALL("POST", "ai-assistant/v2/send", DoPostTheRecognizeRequest,
           BODY(std::shared_ptr<oatpp::web::protocol::http::outgoing::MultipartBody>, multiple_part_body))

  API_CALL("POST", "ai-assistant/v3/send", DoPostTheRecognizeRequestByStreamAPI,
           BODY(std::shared_ptr<oatpp::web::protocol::http::outgoing::MultipartBody>, multiple_part_body))
  API_CALL("POST", "ai-assistant/v3/upload", DoPostQuestionnaireUpload,
           BODY(std::shared_ptr<oatpp::web::protocol::http::outgoing::MultipartBody>, multiple_part_body))

  API_CALL("POST", "ai-assistant/v3/verify", DoPostTheQuestionnaireVerify,
           BODY(std::shared_ptr<oatpp::web::protocol::http::outgoing::MultipartBody>, multiple_part_body))

  API_CALL("POST", "ai-assistant/v3/report", DoPostTheRecognizeReport,
           BODY(std::shared_ptr<oatpp::web::protocol::http::outgoing::MultipartBody>, multiple_part_body))

  // [feat:2709] AI Dialog - Load history message
  // User POST to avoid character parsing error
  API_CALL("POST", "ai-assistant/v3/history", DoPostTheHistoryRequest,
           BODY(std::shared_ptr<oatpp::web::protocol::http::outgoing::MultipartBody>, multiple_part_body))

  API_CALL("GET", "ai-assistant/v3/download", DoGetQuestionnaireTemplate, QUERY(String, intelligentEndpoint),
           QUERY(String, sessionId), QUERY(String, projectId), QUERY(String, sysVersion), QUERY(String, projectMode),
           QUERY(String, questionnaireMode), QUERY(String, profile))

#include OATPP_CODEGEN_END(ApiClient)
};
