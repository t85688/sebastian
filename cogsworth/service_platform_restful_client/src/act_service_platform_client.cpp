#include "act_core.hpp"
#include "act_service_platform_client_handler.hpp"

act::servicePlatformClient::ActServicePlatformClient::ActServicePlatformClient() {
  auto objects_created = oatpp::base::Environment::getObjectsCreated();

  if (objects_created == 0) {
    qDebug() << __func__ << "oatpp::base::Environment::init()";
    oatpp::base::Environment::init();
  }
}

act::servicePlatformClient::ActServicePlatformClient::~ActServicePlatformClient() {}

ACT_STATUS act::servicePlatformClient::ActServicePlatformClient::Login(const QString &baseUrl, const QString &proxy,
                                                                       const qint64 &user_id,
                                                                       const ActServicePlatformLoginRequest &login_req,
                                                                       ActServicePlatformLoginResponse &login_res) {
  ACT_STATUS_INIT();

  ActServicePlatformClientAgent client_agent(baseUrl, proxy);

  // Init
  act_status = client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  act_status = client_agent.Login(login_req, login_res);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Login() failed.";
    return act_status;
  }

  // Set the token to the core
  act::core::g_core.UpdateServicePlatformToken(user_id, client_agent.token_);

  return act_status;
}

ACT_STATUS act::servicePlatformClient::ActServicePlatformClient::Register(
    const QString &baseUrl, const QString &proxy, const qint64 &user_id, const qint64 &platform_project_id,
    const QString &project_name, const QSet<ActNetworkBaseline> &baseline_set,
    ActServicePlatformRegisterResponse &register_res) {
  ACT_STATUS_INIT();

  // Function summary:
  // Check the Project is registered before (w/ platform id != -1)
  // If not registered, call post method
  // If registered, call put method, if receive 404, call post method

  ActServicePlatformClientAgent client_agent(baseUrl, proxy);

  // Init
  act_status = client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetServicePlatformToken(user_id, client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    // return failed and ask login request
    return std::make_shared<ActUnauthorized>();
    // act_status = LoginAndUpdateCoreToken(client_agent, device);
    // if (!IsActStatusSuccess(act_status)) {
    //   return act_status;
    // }
  }

  QList<ActServicePlatformRegisterRequestBaseline> baseline_list;
  for (ActNetworkBaseline baseline : baseline_set) {
    ActBaselineBOMDetail baseline_bom_detail(baseline);

    ActServicePlatformRegisterRequestBaseline req_baseline;
    req_baseline.SetId(baseline_bom_detail.GetId());
    req_baseline.SetName(baseline_bom_detail.GetName());
    req_baseline.SetCreatedTime(baseline_bom_detail.GetDate());
    req_baseline.SetTotalPrice(baseline_bom_detail.GetTotalPrice());

    QList<ActServicePlatformRegisterRequestBaselineModel> model_list;
    for (auto sku_name : baseline_bom_detail.GetSkuQuantitiesMap().keys()) {
      ActSkuQuantity sku_quantity = baseline_bom_detail.GetSkuQuantitiesMap()[sku_name];

      ActServicePlatformRegisterRequestBaselineModel model;
      model.SetModelName(sku_name);
      model.SetQty(sku_quantity.GetQuantity());
      model.SetPrice(sku_quantity.GetPrice());
      model.SetTotalPrice(sku_quantity.GetTotalPrice());
      model_list.append(model);
    }

    req_baseline.SetModelList(model_list);
    baseline_list.append(req_baseline);
  }

  // Check the Project is registered before (w/ platform id != -1)
  if (platform_project_id == -1) {
    ActServicePlatformRegisterRequest register_req;
    register_req.SetProjectName(project_name);
    register_req.SetBaselineList(baseline_list);

    act_status = client_agent.RegisterNewProject(register_req, register_res);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "Register() failed. Project name:" << project_name;
      return act_status;
    }
  } else {
    // Update the project
    ActServicePlatformUpdateProjectRequest register_req;
    register_req.SetProjectName(project_name);
    register_req.SetStatus("not_reviewed");  // TODO: status should be Enum
    register_req.SetBaselineList(baseline_list);

    act_status = client_agent.UpdateProject(platform_project_id, register_req, register_res);
    if (!IsActStatusSuccess(act_status)) {
      // Dump the project detail
      qWarning() << __func__ << "UpdateProject() failed. Project name:" << project_name;
      if (act_status->GetStatus() == ActStatusType::kNotFound) {
        // Call post method
        ActServicePlatformRegisterRequest register_req;
        register_req.SetProjectName(project_name);
        register_req.SetBaselineList(baseline_list);

        act_status = client_agent.RegisterNewProject(register_req, register_res);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "Register() failed. Project name:" << project_name;
          return act_status;
        }
      }
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS act::servicePlatformClient::ActServicePlatformClient::GetPrice(
    const QString &baseUrl, const QString &proxy, const qint64 &user_id,
    const ActServicePlatformGetPriceRequest &get_price_req, ActServicePlatformGetPriceResponse &get_price_res) {
  ACT_STATUS_INIT();

  ActServicePlatformClientAgent client_agent(baseUrl, proxy);

  // Init
  act_status = client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetServicePlatformToken(user_id, client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    // return failed and ask login request
    return std::make_shared<ActUnauthorized>();
    // act_status = LoginAndUpdateCoreToken(client_agent, device);
    // if (!IsActStatusSuccess(act_status)) {
    //   return act_status;
    // }
  }

  act_status = client_agent.GetPrice(get_price_req, get_price_res);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetPrice() failed.";
    return act_status;
  }

  return act_status;
}
