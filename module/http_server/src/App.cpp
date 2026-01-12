#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QDebug>
#include <csignal>
#include <iostream>

#include "act_system.hpp"
#include "controller/act_controller.hpp"
#include "http_server.h"
// Must be the first header

#include "./AppComponent.hpp"
#include "./controller/MyController.hpp"
#include "app_component.hpp"
#include "controller/act_class_based_controller.hpp"
#include "controller/act_compute_controller.hpp"
#include "controller/act_deploy_controller.hpp"
#include "controller/act_device_controller.hpp"
#include "controller/act_device_module_controller.hpp"
#include "controller/act_device_profile_controller.hpp"
#include "controller/act_device_setting_controller.hpp"
#include "controller/act_event_log_controller.hpp"
#include "controller/act_firmware_controller.hpp"
#include "controller/act_group_controller.hpp"
#include "controller/act_host_controller.hpp"
#include "controller/act_intelligent_controller.hpp"
#include "controller/act_license_controller.hpp"
#include "controller/act_link_controller.hpp"
#include "controller/act_login_controller.hpp"
#include "controller/act_management_interface_controller.hpp"
#include "controller/act_manufacture_controller.hpp"
#include "controller/act_monitor_controller.hpp"
#include "controller/act_network_baseline_controller.hpp"
#include "controller/act_project_controller.hpp"
#include "controller/act_project_setting_controller.hpp"
#include "controller/act_redundant_swift_controller.hpp"
#include "controller/act_service_platform_controller.hpp"
#include "controller/act_service_profile_controller.hpp"
#include "controller/act_static_forward_config_controller.hpp"
#include "controller/act_stream_controller.hpp"
#include "controller/act_swagger_controller.hpp"
#include "controller/act_system_controller.hpp"
#include "controller/act_topology_controller.hpp"
#include "controller/act_traffic_controller.hpp"
#include "controller/act_user_controller.hpp"
#include "controller/act_vlan_config_controller.hpp"
#include "controller/act_vlan_view_controller.hpp"
#include "controller/act_ws_controller.hpp"
#include "oatpp-swagger/Controller.hpp"
#include "oatpp/core/data/stream/BufferStream.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

// #include "controller/act_test_controller.hpp"
// #include "oatpp-openssl/Callbacks.hpp"
std::shared_ptr<std::thread> http_server_thread_;

/**
 *  1) set Environment components.
 *  2) add ApiController's endpoints to router
 *  3) run server
 */
void RunHttpServer() {
  oatpp::base::Environment::init();

  /* Register Components in scope of run() method */
  AppComponent components;
  /* Get router component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
  // Create Swagger Controller
  // list of endpoints to document with swagger-ui
  oatpp::web::server::api::Endpoints docEndpoints;

  docEndpoints.append(router->addController(ActLoginController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActWSController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActProjectController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActManagementInterfacesController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActProjectSettingController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActSystemController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActLicenseController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActUserController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActDeviceController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActDeviceProfileController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActDeviceSettingController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActHostController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActDeployController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActManufactureController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActDeviceModuleController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActLinkController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActStreamController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActClassBasedController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActGroupController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActComputeController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActFirmwareController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActStaticForwardConfigController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActVlanConfigController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActServiceProfileController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActVlanViewController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActIntelligentController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActTopologyController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActRedundantSwiftController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActTrafficController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActServicePlatformController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActMonitorController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActNetworkBaselineController::createShared())->getEndpoints());
  docEndpoints.append(router->addController(ActEventLogController::createShared())->getEndpoints());
  //   docEndpoints.append(router->addController(ActTestController::createShared())->getEndpoints());

  // Register https://localhost:8443/swagger/ui
  router->addController(oatpp::swagger::Controller::createShared(docEndpoints));

  // Register https://localhost:8443/
  router->addController(ActController::createShared());

  /* Get connection handler component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler, "https");

  /* Get connection provider component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

  /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
  oatpp::network::Server server(connectionProvider, connectionHandler);

  std::thread oatppThread([&server] {
    /* Run server */
    server.run();
  });

  // Print info about server port
  // OATPP_LOGI("MyApp", "Server running on port %s", connectionProvider->getProperty("port").getData());
  qInfo() << "***************************************";
  qInfo() << " Http(s) Server running on port:" << (char *)connectionProvider->getProperty("port").getData();
  qInfo() << "***************************************";

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  // while (g_act_process_status == ActProcessStatus::Running) {
  while (1) {
    std::this_thread::yield();
    SLEEP_MS(500);
  }

  /* First, stop the ServerConnectionProvider so we don't accept any new connections */
  qDebug() << "First, stop the ServerConnectionProvider so we don't accept any new connections";
  connectionProvider->stop();

  /* Now, check if server is still running and stop it if needed */
  qDebug() << "Now, check if server is still running and stop it if needed";
  if (server.getStatus() == oatpp::network::Server::STATUS_RUNNING) {
    server.stop();
  }

  /* Finally, stop the ConnectionHandler and wait until all running connections are closed */
  // qDebug() << "Finally, stop the ConnectionHandler and wait until all running connections are closed";
  // connectionHandler->stop();

  /* Before returning, check if the server-thread has already stopped or if we need to wait for the server to stop */
  qDebug() << "Before returning, check if the server-thread has already stopped or if we need to wait for the server "
              "to stop";
  if (oatppThread.joinable()) {
    /* We need to wait until the thread is done */
    qDebug() << "We need to wait until the oatpp server thread is done";
    oatppThread.join();
  }

  /* Print how much objects were created during app running, and what have left-probably leaked */
  /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better
  performance
   */
  qDebug() << "Oatpp Environment:";
  qDebug() << "objectsCount =" << oatpp::base::Environment::getObjectsCount();
  qDebug() << "objectsCreated =" << oatpp::base::Environment::getObjectsCreated();

  oatpp::base::Environment::destroy();

  return;
}

// void RunTestHttpServer() {
//   oatpp::base::Environment::init();

//   /* Register Components in scope of run() method */
//   MyAppComponent components;

//   /* Get router component */
//   OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

//   /* Create MyController and add all of its endpoints to router */
//   router->addController(std::make_shared<MyController>());

//   /* Get connection handler component */
//   OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);

//   /* Get connection provider component */
//   OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

//   /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
//   oatpp::network::Server server(connectionProvider, connectionHandler);

//   /* Run server */
//   server.run();

//   /* Print info about server port */
//   qInfo() << "***************************************";
//   qInfo() << " Http(s) Server running on port:" << (char*)connectionProvider->getProperty("port").getData();
//   qInfo() << "***************************************";

//   // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
//   while (g_act_process_status == ActProcessStatus::Running) {
//     std::this_thread::yield();
//     SLEEP_MS(500);
//   }

//   /* First, stop the ServerConnectionProvider so we don't accept any new connections */
//   connectionProvider->stop();

//   /* Now, check if server is still running and stop it if needed */
//   if (server.getStatus() == oatpp::network::Server::STATUS_RUNNING) {
//     server.stop();
//   }

//   /* Finally, stop the ConnectionHandler and wait until all running connections are closed */
//   connectionHandler->stop();

//   qDebug() << "Oatpp Environment:";
//   qDebug() << "objectsCount =" << oatpp::base::Environment::getObjectsCount();
//   qDebug() << "objectsCreated =" << oatpp::base::Environment::getObjectsCreated();
//   oatpp::base::Environment::destroy();
// }

// void StartTestHttpServer() { http_server_thread_ = std::make_shared<std::thread>(&RunTestHttpServer); }
void StartHttpServer() {
  http_server_thread_ = std::make_shared<std::thread>(&RunHttpServer);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"HttpServer";
  HRESULT hr = SetThreadDescription(http_server_thread_->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif
}

void StopHttpServer() {
  if (http_server_thread_ != nullptr && http_server_thread_->joinable()) {
    qDebug() << "We need to wait until the http server thread is done";
    http_server_thread_->join();
  }
}
