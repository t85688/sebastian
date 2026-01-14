/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <QString>

#include "dto/act_service_platform_get_price_request_dto.hpp"  // Include new DTO
#include "dto/act_service_platform_login_request_dto.hpp"
#include "dto/act_service_platform_register_request_dto.hpp"  // Include new DTO
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/web/client/ApiClient.hpp"
#include "oatpp/web/protocol/http/outgoing/MultipartBody.hpp"

class ActServicePlatformClient : public oatpp::web::client::ApiClient {
#include OATPP_CODEGEN_BEGIN(ApiClient)

  API_CLIENT_INIT(ActServicePlatformClient)

  API_CALL("POST", "api/v1/login", DoLogin, BODY_DTO(Object<ActServicePlatformLoginRequestDto>, body))

  // API_CALL("GET", "api/v1/projects", DoGetRegister, AUTHORIZATION(String, token, "Bearer"),
  //          BODY_DTO(Object<ActServicePlatformRegisterRequestDto>, body))

  API_CALL("POST", "api/v1/projects", DoPostRegister, AUTHORIZATION(String, token, "Bearer"),
           BODY_DTO(Object<ActServicePlatformRegisterRequestDto>, body))

  API_CALL("PUT", "api/v1/projects/{projectId}", DoPutRegister, PATH(UInt64, projectId),
           AUTHORIZATION(String, token, "Bearer"), BODY_DTO(Object<ActServicePlatformUpdateProjectRequestDto>, body))

  API_CALL("POST", "api/v1/prices", DoGetPrice, AUTHORIZATION(String, token, "Bearer"),
           BODY_DTO(Object<ActServicePlatformGetPriceRequestDto>, body))

#include OATPP_CODEGEN_END(ApiClient)
};
