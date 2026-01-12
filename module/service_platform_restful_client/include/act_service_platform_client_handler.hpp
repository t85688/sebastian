/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_network_baseline.hpp"
#include "act_service_platform_client.hpp"
#include "act_service_platform_client_agent.hpp"
#include "act_service_platform_request.hpp"
#include "act_status.hpp"

namespace act {
namespace servicePlatformClient {

class ActServicePlatformClient {
 public:
  ActServicePlatformClient();

  ~ActServicePlatformClient();

  ACT_STATUS Login(const QString &baseUrl, const QString &proxy, const qint64 &user_id,
                   const ActServicePlatformLoginRequest &login_req, ActServicePlatformLoginResponse &login_res);

  ACT_STATUS Register(const QString &baseUrl, const QString &proxy, const qint64 &user_id,
                      const qint64 &platform_project_id, const QString &project_name,
                      const QSet<ActNetworkBaseline> &baseline_set, ActServicePlatformRegisterResponse &register_res);

  ACT_STATUS GetPrice(const QString &baseUrl, const QString &proxy, const qint64 &user_id,
                      const ActServicePlatformGetPriceRequest &get_price_req,
                      ActServicePlatformGetPriceResponse &get_price_res);
};

}  // namespace servicePlatformClient
}  // namespace act
