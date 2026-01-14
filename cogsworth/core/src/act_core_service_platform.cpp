#include <QHostInfo>
#include <QNetworkInterface>
#include <QSysInfo>
#include <QUrl>

#include "act_core.hpp"
#include "act_service_platform_client_handler.hpp"
#include "act_service_platform_request.hpp"
#include "act_status.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::GetServicePlatformTokenMap(QMap<qint64, QString> &service_platform_token_map) {
  ACT_STATUS_INIT();
  service_platform_token_map = this->service_platform_token_map_;
  service_platform_token_map.detach();
  return act_status;
}

ACT_STATUS ActCore::SetServicePlatformTokenMap(const QMap<qint64, QString> &service_platform_token_map) {
  ACT_STATUS_INIT();

  this->service_platform_token_map_ = service_platform_token_map;
  this->service_platform_token_map_.detach();

  return act_status;
}

ACT_STATUS ActCore::GetServicePlatformToken(const qint64 &user_id, QString &token) {
  ACT_STATUS_INIT();
  if (this->service_platform_token_map_.contains(user_id)) {
    token = this->service_platform_token_map_[user_id];
  } else {
    token = "";
  }

  service_platform_token_map_.detach();
  return act_status;
}

ACT_STATUS ActCore::UpdateServicePlatformToken(const qint64 &user_id, const QString &token) {
  ACT_STATUS_INIT();

  this->service_platform_token_map_[user_id] = token;
  this->service_platform_token_map_.detach();

  return act_status;
}

ACT_STATUS ActCore::DeleteServicePlatformToken(const qint64 &user_id) {
  ACT_STATUS_INIT();

  this->service_platform_token_map_.remove(user_id);
  this->service_platform_token_map_.detach();

  return act_status;
}

ACT_STATUS ActCore::ServicePlatformLogin(const qint64 &user_id, const ActServicePlatformLoginRequest &login_req,
                                         ActServicePlatformLoginResponse &login_res) {
  ACT_STATUS_INIT();

  qDebug() << __func__;

  QString service_platform_endpoint = GetServicePlatformEndpointFromEnviron();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::servicePlatformClient::ActServicePlatformClient service_platform_client;
  act_status =
      service_platform_client.Login(service_platform_endpoint, http_proxy_endpoint, user_id, login_req, login_res);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Login() failed.";
    return act_status;
  }

  // After login success, get price for all general profiles
  ActServicePlatformGetPriceRequest get_price_req;
  get_price_req.SetModelList(this->GetGeneralProfileMap().keys());

  ActServicePlatformGetPriceResponse get_price_res;
  act_status = service_platform_client.GetPrice(service_platform_endpoint, http_proxy_endpoint, user_id, get_price_req,
                                                get_price_res);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Login() failed.";
    return act_status;
  }

  // Update the price to general profiles
  QMap<QString, ActSkuWithPrice> general_profiles = this->GetGeneralProfileMap();
  for (auto it = general_profiles.begin(); it != general_profiles.end(); ++it) {
    const QString &model_name = it.key();
    if (get_price_res.Getdata().contains(model_name)) {
      it.value().SetPrice(get_price_res.Getdata()[model_name]);
    }
  }
  this->SetGeneralProfileMap(general_profiles);

  // update price of all SKU quantities map in the projects
  act_status = UpdateSkuQuantityPriceInProjects();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "UpdateSkuQuantityPriceInProjects() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::CheckServicePlatformToken(const qint64 &user_id, ActServicePlatformLoginCheck &status) {
  ACT_STATUS_INIT();

  qDebug() << __func__;

  // Get token from the map
  QString token;
  act_status = GetServicePlatformToken(user_id, token);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetServicePlatformToken() failed.";
    return act_status;
  }

  token.isEmpty() ? status.SetStatus(false) : status.SetStatus(true);

  return act_status;
}

ACT_STATUS ActCore::ServicePlatformRegister(const qint64 &user_id, const qint64 &project_id) {
  ACT_STATUS_INIT();

  qDebug() << __func__;

  // Get project
  ActProject project;
  act_status = GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetProject() failed.";
    return act_status;
  }

  ActServicePlatformRegisterRequest register_req;
  register_req.SetProjectName(project.GetProjectName());
  QSet<ActNetworkBaseline> baseline_set;

  // foreach baseline in the core, check if the project id is matched
  for (ActNetworkBaseline baseline : this->GetDesignBaselineSet()) {
    if (baseline.GetProjectId() == project_id) {
      baseline_set.insert(baseline);
    }
  }

  QString service_platform_endpoint = GetServicePlatformEndpointFromEnviron();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::servicePlatformClient::ActServicePlatformClient service_platform_client;
  ActServicePlatformRegisterResponse register_res;
  act_status = service_platform_client.Register(service_platform_endpoint, http_proxy_endpoint, user_id,
                                                project.GetPlatformProjectId(), project.GetProjectName(), baseline_set,
                                                register_res);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Register() failed.";
    // If the status is 401, which means the token has expired or not exist, we need to notify the user re-login
    if (act_status->GetStatus() == ActStatusType::kUnauthorized) {
      act_status->SetStatus(ActStatusType::kServicePlatformUnauthorized);
      act_status->SetStatusCode(static_cast<qint64>(ActStatusType::kServicePlatformUnauthorized));
      act_status->SetSeverity(ActSeverity::kCritical);
      act_status->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kServicePlatformUnauthorized));

      this->DeleteServicePlatformToken(user_id);
    }
    return act_status;
  }

  project.SetPlatformProjectId(register_res.GetId());

  act_status = UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "UpdateProject() failed.";
    return act_status;
  }

  return act_status;
}

}  // namespace core
}  // namespace act