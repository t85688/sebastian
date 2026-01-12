/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <QString>
#include <memory>

#include "act_service_platform_client.hpp"
#include "act_service_platform_request.hpp"
#include "act_status.hpp"
#include "oatpp-curl/ProxyRequestExecutor.hpp"
#include "oatpp/core/data/resource/InMemoryData.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/client/HttpRequestExecutor.hpp"
#include "oatpp/web/mime/multipart/PartList.hpp"

#define ACT_RESTFUL_CLIENT_REQUEST_SUCCESS_200 (200)
#define ACT_RESTFUL_CLIENT_REQUEST_SUCCESS_201 (201)

class ActServicePlatformClientAgent {
 public:
  QString token_;

 private:
  QString base_url_;
  QString proxy_;
  std::shared_ptr<ActServicePlatformClient> client_;  // Fully qualified

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
      client_ = ActServicePlatformClient::createShared(requestExecutor, objectMapper);
    } catch (std::exception &e) {
      qCritical() << __func__ << "Create the service platform client failed. Error:" << e.what();
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

    // Check success
    auto response_status_code = response->getStatusCode();
    if (response_status_code != ACT_RESTFUL_CLIENT_REQUEST_SUCCESS_200 &&
        response_status_code != ACT_RESTFUL_CLIENT_REQUEST_SUCCESS_201) {
      // Print request
      auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

      qDebug() << called_func << "Response Status(" << response_status_code
               << "):" << response->getStatusDescription()->c_str();

      QString response_body = response->readBodyToString()->c_str();
      qDebug() << called_func
               << QString("service platform server(%1) reply failed. response_body: %2")
                      .arg(base_url_)
                      .arg(response_body)
                      .toStdString()
                      .c_str();

      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

 public:
  ActServicePlatformClientAgent(const QString &baseUrl, const QString &proxy) {
    base_url_ = baseUrl;
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

  // login
  ACT_STATUS Login(const ActServicePlatformLoginRequest &login_req, ActServicePlatformLoginResponse &login_res) {
    ACT_STATUS_INIT();

    try {
      // Create DTO request_body
      auto request_dto = ActServicePlatformLoginRequestDto::createShared();
      request_dto->username = oatpp::String(login_req.GetUsername().toStdString().c_str());
      request_dto->password = oatpp::String(login_req.GetPassword().toStdString().c_str());

      // Send request
      auto response = client_->DoLogin(request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      // Deserialize response body
      QString response_body = response->readBodyToString()->c_str();
      login_res.FromString(response_body);

      token_ = login_res.Gettoken();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  // register
  ACT_STATUS RegisterNewProject(const ActServicePlatformRegisterRequest &register_req,
                                ActServicePlatformRegisterResponse &register_res) {
    ACT_STATUS_INIT();

    try {
      // Create DTO request_body
      auto request_dto = ActServicePlatformRegisterRequestDto::createShared();
      request_dto->project_name = oatpp::String(register_req.GetProjectName().toStdString().c_str());

      // Create baseline_list
      auto baseline_list = oatpp::List<oatpp::Object<ActServicePlatformRegisterRequestBaselineDto>>::createShared();
      for (const auto &baseline : register_req.GetBaselineList()) {
        auto baseline_dto = ActServicePlatformRegisterRequestBaselineDto::createShared();
        baseline_dto->id = baseline.GetId();
        baseline_dto->name = oatpp::String(baseline.GetName().toStdString().c_str());
        baseline_dto->created_time = baseline.GetCreatedTime();
        baseline_dto->total_price = oatpp::String(baseline.GetTotalPrice().toStdString().c_str());

        // Create model_list
        auto model_list = oatpp::List<oatpp::Object<ActServicePlatformRegisterRequestBaselineModelDto>>::createShared();
        for (const auto &model : baseline.GetModelList()) {
          auto model_dto = ActServicePlatformRegisterRequestBaselineModelDto::createShared();
          model_dto->model_name = oatpp::String(model.GetModelName().toStdString().c_str());
          model_dto->qty = model.GetQty();
          model_dto->price = oatpp::String(model.GetPrice().toStdString().c_str());
          model_dto->total_price = oatpp::String(model.GetTotalPrice().toStdString().c_str());
          model_list->push_back(model_dto);
        }
        baseline_dto->model_list = model_list;
        baseline_list->push_back(baseline_dto);
      }
      request_dto->baseline_list = baseline_list;

      auto response = client_->DoPostRegister(token_.toStdString().c_str(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      // Deserialize response body
      QString response_body = response->readBodyToString()->c_str();

      register_res.FromString(response_body);
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  ACT_STATUS UpdateProject(const qint64 &platform_project_id,
                           const ActServicePlatformUpdateProjectRequest &register_req,
                           ActServicePlatformRegisterResponse &register_res) {
    ACT_STATUS_INIT();

    try {
      // Create DTO request_body
      auto request_dto = ActServicePlatformUpdateProjectRequestDto::createShared();
      request_dto->project_name = oatpp::String(register_req.GetProjectName().toStdString().c_str());
      request_dto->status = oatpp::String(register_req.GetStatus().toStdString().c_str());

      // Create baseline_list
      auto baseline_list = oatpp::List<oatpp::Object<ActServicePlatformRegisterRequestBaselineDto>>::createShared();
      for (const auto &baseline : register_req.GetBaselineList()) {
        auto baseline_dto = ActServicePlatformRegisterRequestBaselineDto::createShared();
        baseline_dto->id = baseline.GetId();
        baseline_dto->name = oatpp::String(baseline.GetName().toStdString().c_str());
        baseline_dto->created_time = baseline.GetCreatedTime();
        baseline_dto->total_price = oatpp::String(baseline.GetTotalPrice().toStdString().c_str());

        // Create model_list
        auto model_list = oatpp::List<oatpp::Object<ActServicePlatformRegisterRequestBaselineModelDto>>::createShared();
        for (const auto &model : baseline.GetModelList()) {
          auto model_dto = ActServicePlatformRegisterRequestBaselineModelDto::createShared();
          model_dto->model_name = oatpp::String(model.GetModelName().toStdString().c_str());
          model_dto->qty = model.GetQty();
          model_dto->price = oatpp::String(model.GetPrice().toStdString().c_str());
          model_dto->total_price = oatpp::String(model.GetTotalPrice().toStdString().c_str());
          model_list->push_back(model_dto);
        }
        baseline_dto->model_list = model_list;
        baseline_list->push_back(baseline_dto);
      }
      request_dto->baseline_list = baseline_list;

      auto response = client_->DoPutRegister(platform_project_id, token_.toStdString().c_str(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      // Deserialize response body
      QString response_body = response->readBodyToString()->c_str();

      register_res.FromString(response_body);
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }

  // get price
  ACT_STATUS GetPrice(const ActServicePlatformGetPriceRequest &get_price_req,
                      ActServicePlatformGetPriceResponse &get_price_res) {
    ACT_STATUS_INIT();

    try {
      // Create DTO request_body
      auto request_dto = ActServicePlatformGetPriceRequestDto::createShared();
      auto model_list = oatpp::List<oatpp::String>::createShared();
      for (const auto &model : get_price_req.GetModelList()) {
        model_list->push_back(oatpp::String(model.toStdString().c_str()));
      }
      request_dto->model_list = model_list;

      auto response = client_->DoGetPrice(token_.toStdString().c_str(), request_dto);

      // Check success
      act_status = CheckResponseStatus(__func__, response);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      // Deserialize response body
      QString response_body = response->readBodyToString()->c_str();
      get_price_res.FromString(response_body);

      qDebug() << __func__ << "get_price_res::" << get_price_res.ToString().toStdString().c_str();

    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }

    return act_status;
  }
};
