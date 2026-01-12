#include "act_restful_client_handler.h"

#include <QDate>
#include <QDateTime>
#include <QTime>

#include "act_core.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

// #include "oatpp-curl/RequestExecutor.hpp"
// #include "oatpp/network/tcp/client/ConnectionProvider.hpp"
// #include "oatpp/parser/json/mapping/ObjectMapper.hpp"
// #include "oatpp/web/client/HttpRequestExecutor.hpp"

ActRestfulClientHandler::ActRestfulClientHandler() {
  auto objects_created =
      oatpp::base::Environment::getObjectsCreated();  // get count of objects created for a whole system lifetime
  // qDebug() << __func__ << "ObjectsCreated:" << objects_created;

  if (objects_created == 0) {
    qDebug() << __func__ << "oatpp::base::Environment::init()";
    oatpp::base::Environment::init();
  }
}

ActRestfulClientHandler::~ActRestfulClientHandler() {
  // oatpp::base::Environment::destroy();
}

ACT_STATUS ActRestfulClientHandler::LoginAndUpdateCoreToken(ActMoxaIEIClientAgent &client_agent,
                                                            const ActDevice &device) {
  ACT_STATUS_INIT();
  QString update_token = "";

  ActClientLoginRequest login_request(device.GetAccount().GetUsername(), device.GetAccount().GetPassword());
  act_status = client_agent.Login(login_request);
  if (IsActStatusUnauthorized(act_status)) {
    SLEEP_MS(500);  // wait 0.5 second
    qWarning() << __func__ << "Login response as UNAUTHORIZED(401) would try again";
    act_status = client_agent.Login(login_request);
  }

  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Login() failed.";

    // qDebug() << __func__
    //          << QString("!!!Device(%1) loging request: %2")
    //                 .arg(device.GetIpv4().GetIpAddress())
    //                 .arg(login_request.ToString())
    //                 .toStdString()
    //                 .c_str();
  }
  // Update token
  // Success would update new token, else update empty token.

  update_token = client_agent.token_;

  QMutexLocker lock(&act::core::g_core.mutex_);

  act::core::g_core.UpdateDeviceRESTfulTokenMap(device.GetId(), update_token);

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetStringRequest(const ActDevice &device,
                                                     const ActFeatureMethodProtocol &protocol_elem,
                                                     QMap<QString, QString> &result_action_map) {
  ACT_STATUS_INIT();

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  result_action_map.clear();
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // ActMoxaRequestTypeEnum type;
  for (auto action_key : protocol_elem.GetActions().keys()) {
    // Get StringRequest
    QString str_result;
    auto type_it = kActMoxaRequestTypeEnumMap.find(action_key);
    if (type_it == kActMoxaRequestTypeEnumMap.end()) {
      qCritical() << __func__ << QString("Not support the %1 request item").arg(action_key).toStdString().c_str();
      return std::make_shared<ActStatusNotFound>(QString("RESTful request item(%1)").arg(action_key));
    }

    // Send request by type
    bool login_retry = true;
    bool unavailable_retry = true;
    while (true) {
      act_status = client_agent.GetStringRequestUseToken(type_it.value(), str_result);
      if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
        unavailable_retry = false;
        SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
        continue;
      }
      if (login_retry && IsActStatusUnauthorized(act_status)) {
        login_retry = false;
        // Try to login & retry access
        act_status = LoginAndUpdateCoreToken(client_agent, device);
        if (!IsActStatusSuccess(act_status)) {
          return act_status;
        }
        continue;
      }

      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "GetStringRequestUseToken() failed.";
        return act_status;
      }

      // Success would break loop
      break;
    }

    // Insert to result map
    result_action_map.insert(action_key, str_result);
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetSerialNumber(const ActDevice &device, const QString &action_key,
                                                    const ActFeatureMethodProtocol &protocol_elem,
                                                    QString &result_serial_number) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetSerialNumber, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(SerialNumber) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  result_serial_number = str_result;

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetSystemUptime(const ActDevice &device, const QString &action_key,
                                                    const ActFeatureMethodProtocol &protocol_elem,
                                                    QString &result_uptime) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetUptime, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(Uptime) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  result_uptime = str_result;

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetProductRevision(const ActDevice &device, const QString &action_key,
                                                       const ActFeatureMethodProtocol &protocol_elem,
                                                       QString &result_product_revision) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetProductRevision, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(ProductRevision) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  result_product_revision = str_result;

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetRedundantProtocol(const ActDevice &device, const QString &action_key,
                                                         const ActFeatureMethodProtocol &protocol_elem,
                                                         QString &result_redundant_protocol) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetMxL2Redundancy, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(ProductRevision) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  QJsonDocument json_doc = QJsonDocument::fromJson(QString("{%1}").arg(str_result).toUtf8());
  QJsonObject json_obj = json_doc.object();

  // Get active protocol
  result_redundant_protocol = "";
  for (const QString &key : json_obj.keys()) {
    if (json_obj[key].toBool() == true) {
      if (RedundantProtocolMappingMap.contains(key)) {
        result_redundant_protocol = RedundantProtocolMappingMap[key];
      } else {
        result_redundant_protocol = key;
      }
      break;
    }
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetDeviceName(const ActDevice &device, const QString &action_key,
                                                  const ActFeatureMethodProtocol &protocol_elem,
                                                  QString &result_device_name) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetDeviceName, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(DeviceName) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  result_device_name = str_result;

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetDeviceLocation(const ActDevice &device, const QString &action_key,
                                                      const ActFeatureMethodProtocol &protocol_elem,
                                                      QString &result_device_location) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetDeviceLocation, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(DeviceLocation) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  result_device_location = str_result;

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetDeviceDescription(const ActDevice &device, const QString &action_key,
                                                         const ActFeatureMethodProtocol &protocol_elem,
                                                         QString &result_device_description) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetDeviceDescription, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(DeviceDescription) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  result_device_description = str_result;

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetContactInformation(const ActDevice &device, const QString &action_key,
                                                          const ActFeatureMethodProtocol &protocol_elem,
                                                          QString &result_contact_info) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetContactInformation, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(ContactInformation) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  result_contact_info = str_result;

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetIPConfiguration(const ActDevice &device, const QString &action_key,
                                                       const ActFeatureMethodProtocol &protocol_elem,
                                                       ActIpv4 &result_ipv4) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetIpv4, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(Ipv4) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientIpv4 client_ipv4;
  client_ipv4.FromString(QString("{%1}").arg(str_result));
  result_ipv4.SetIpAddress(client_ipv4.GetipAddress());
  result_ipv4.SetSubnetMask(client_ipv4.Getnetmask());
  result_ipv4.SetGateway(client_ipv4.Getgateway());
  if (client_ipv4.GetdnsServer().size() > 1) {
    result_ipv4.SetDNS1(client_ipv4.GetdnsServer()[0]);
    result_ipv4.SetDNS2(client_ipv4.GetdnsServer()[1]);
  } else if (client_ipv4.GetdnsServer().size() == 1) {
    result_ipv4.SetDNS1(client_ipv4.GetdnsServer()[0]);
    result_ipv4.SetDNS2("");
  } else {
    result_ipv4.SetDNS1("");
    result_ipv4.SetDNS2("");
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetL3IPConfiguration(const ActDevice &device, const QSet<QString> &action_keys,
                                                         const ActFeatureMethodProtocol &protocol_elem,
                                                         ActIpv4 &result_ipv4) {
  ACT_STATUS_INIT();

  // Check Action element
  for (auto action_key : action_keys) {
    if (!protocol_elem.GetActions().contains(action_key)) {
      qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
      return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
    }
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString router_str;
  QString dns_str;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetL3RouterId, router_str);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(L3RouterId) failed.";
      return act_status;
    }

    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetL3NetworkDns, dns_str);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(L3NetworkDns) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientL3RouterIdIpv4 client_router;

  // qDebug() << "router_str(" << device.GetIpv4().GetIpAddress() << "):" << router_str;
  client_router.FromString(QString("{%1}").arg(router_str));

  // qDebug() << "client_router(" << device.GetIpv4().GetIpAddress()
  //          << "):" << client_router.ToString().toStdString().c_str();

  ActClientL3NetworkDnsIpv4 client_dns;
  client_dns.FromString(QString("{%1}").arg(dns_str));

  result_ipv4.SetIpAddress(client_router.GetipAddress());
  result_ipv4.SetSubnetMask(client_router.Getnetmask());
  if (client_dns.GetdnsServer().size() > 1) {
    result_ipv4.SetDNS1(client_dns.GetdnsServer()[0]);
    result_ipv4.SetDNS2(client_dns.GetdnsServer()[1]);
  } else if (client_dns.GetdnsServer().size() == 1) {
    result_ipv4.SetDNS1(client_dns.GetdnsServer()[0]);
    result_ipv4.SetDNS2("");
  } else {
    result_ipv4.SetDNS1("");
    result_ipv4.SetDNS2("");
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetModularInfo(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem,
                                                   ActDeviceModularInfo &result_modular_info) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetModules, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(ModularInfo) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientModules client_modules;
  client_modules.FromString(QString("{%1}").arg(str_result));

  result_modular_info.GetEthernet().clear();

  // Ethernet
  qint64 slot_index = 0;
  for (auto slot_entry : client_modules.Getethernet()) {
    ActDeviceEthernetModule ethernet_module;

    if (!slot_entry.GetmoduleName().isEmpty()) {
      ethernet_module.SetExist(true);
    }
    ethernet_module.SetModuleName(slot_entry.GetmoduleName());
    ethernet_module.SetSerialNumber(slot_entry.GetserialNumber());
    ethernet_module.SetProductRevision(slot_entry.GetproductRevision());
    ethernet_module.SetStatus(slot_entry.Getstatus());
    ethernet_module.SetModuleId(slot_entry.GetmoduleId());

    result_modular_info.GetEthernet()[slot_index + 1] = ethernet_module;
    slot_index += 1;
  }

  // Power
  slot_index = 0;
  for (auto slot_entry : client_modules.Getpower()) {
    ActDevicePowerModule power_module;

    if (!slot_entry.GetmoduleName().isEmpty()) {
      power_module.SetExist(true);
    }

    power_module.SetModuleName(slot_entry.GetmoduleName());
    power_module.SetSerialNumber(slot_entry.GetserialNumber());
    power_module.SetProductRevision(slot_entry.GetproductRevision());
    power_module.SetStatus(slot_entry.Getstatus());

    result_modular_info.GetPower()[slot_index + 1] = power_module;
    slot_index += 1;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetDeviceConfigurationSyncStatus(const ActDevice &device, bool &check_result) {
  ACT_STATUS_INIT();
  check_result = false;

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get StringRequest
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetConfigurationSyncStatus, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  ActClientNetconfStatus netconf_sync_status;
  netconf_sync_status.FromString(QString("{%1}").arg(str_result));
  if (netconf_sync_status.Getvlan() || netconf_sync_status.Getfrer() || netconf_sync_status.Getstreamid() ||
      netconf_sync_status.Getqbv()) {
    check_result = true;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::CheckConnect(const ActDevice &device, const QString &action_key,
                                                 const ActFeatureMethodProtocol &protocol_elem) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Heartbeat
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PostRequestUseToken(ActMoxaRequestTypeEnum::kPostHeartbeat);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetSnmpService(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem,
                                                   const ActClientSnmpService &snmp_service_config) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set SNMP service
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PatchSnmpService(snmp_service_config);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchSnmpService() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetInformationSetting(const ActDevice &device, const QString &action_key,
                                                          const ActFeatureMethodProtocol &protocol_elem,
                                                          const ActInformationSettingTable &info_setting_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set InformationSetting
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PatchInformationSetting(info_setting_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchInformationSetting() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetInformationSetting(const ActDevice &device, const QString &action_key,
                                                          const ActFeatureMethodProtocol &protocol_elem,
                                                          ActInformationSettingTable &result_info_setting_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status =
        client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetSystemInformationSetting, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(InformationSetting) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientSystemInformationSetting client_setting;
  client_setting.FromString(QString("{%1}").arg(str_result));

  result_info_setting_table.SetDeviceName(client_setting.GetdeviceName());
  result_info_setting_table.SetLocation(client_setting.GetdeviceLocation());
  result_info_setting_table.SetDescription(client_setting.GetdeviceDescription());
  result_info_setting_table.SetContactInformation(client_setting.GetcontactInformation());

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetManagementInterface(const ActDevice &device, const QString &action_key,
                                                           const ActFeatureMethodProtocol &protocol_elem,
                                                           const ActManagementInterfaceTable &mgmt_interface_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set ManagementInterface
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PatchManagementInterface(mgmt_interface_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchManagementInterface() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetManagementInterface(const ActDevice &device, const QString &action_key,
                                                           const ActFeatureMethodProtocol &protocol_elem,
                                                           ActManagementInterfaceTable &result_mgmt_interface_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetServiceManagement, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(ServiceManagement) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientServiceManagement client_setting;
  client_setting.FromString(QString("{%1}").arg(str_result));

  auto client_moxa_service = client_setting.GetencryptedMoxaService();
  ActMgmtEncryptedMoxaService moxa_service(client_moxa_service.Getenable());
  result_mgmt_interface_table.SetEncryptedMoxaService(moxa_service);

  auto client_http_service = client_setting.GethttpService();
  ActMgmtHttpService http_service(client_http_service.Getenable(), client_http_service.Getport());
  result_mgmt_interface_table.SetHttpService(http_service);

  auto client_https_service = client_setting.GethttpsService();
  ActMgmtHttpsService https_service(client_https_service.Getenable(), client_https_service.Getport());
  result_mgmt_interface_table.SetHttpsService(https_service);

  auto client_snmp_service = client_setting.GetsnmpService();
  ActMgmtSnmpServiceModeEnum snmp_mode = static_cast<ActMgmtSnmpServiceModeEnum>(client_snmp_service.Getmode());
  ActMgmtSnmpServiceTransLayerProtoEnum transport_layer_protocol =
      static_cast<ActMgmtSnmpServiceTransLayerProtoEnum>(client_snmp_service.GettransportLayerProtocol());
  ActMgmtSnmpService snmp_service(snmp_mode, client_snmp_service.Getport(), transport_layer_protocol);
  result_mgmt_interface_table.SetSnmpService(snmp_service);

  auto client_ssh_service = client_setting.GetsshService();
  ActMgmtSshService ssh_service(client_ssh_service.Getenable(), client_ssh_service.Getport());
  result_mgmt_interface_table.SetSSHService(ssh_service);

  auto client_telnet_service = client_setting.GettelnetService();
  ActMgmtTelnetService telnet_service(client_telnet_service.Getenable(), client_telnet_service.Getport());
  result_mgmt_interface_table.SetTelnetService(telnet_service);

  result_mgmt_interface_table.SetHttpMaxLoginSessions(client_setting.GethttpMaxLoginSessions());
  result_mgmt_interface_table.SetTerminalMaxLoginSessions(client_setting.GetterminalMaxLoginSessions());

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetUserAccount(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem,
                                                   const ActUserAccountTable &user_account_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Find New Admin Account
  bool has_new_device = false;
  ActDevice tmp_new_device = device;
  for (auto account : user_account_table.GetAccounts().keys()) {
    if (user_account_table.GetAccounts()[account].GetRole() == ActUserAccountRoleEnum::kAdmin) {
      // Check password is empty or not
      QString new_username = user_account_table.GetAccounts()[account].GetUsername();
      QString new_password = user_account_table.GetAccounts()[account].GetPassword();

      // Check the password is empty or not
      if (new_password.isEmpty()) {
        if (new_username == device.GetAccount().GetUsername()) {
          tmp_new_device.GetAccount().SetUsername(new_username);
          tmp_new_device.GetAccount().SetPassword(device.GetAccount().GetPassword());
        } else {
          // Find next account
          continue;
        }
      } else {
        tmp_new_device.GetAccount().SetUsername(new_username);
        tmp_new_device.GetAccount().SetPassword(new_password);
      }
      has_new_device = true;
      break;
    }
  }

  if (has_new_device == false) {
    qCritical() << __func__ << QString("UserAccounts not found the Admin role account and with the password");
    return std::make_shared<ActStatusNotFound>(QString("Admin role account"));
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  QSet<QString> dut_old_accounts;

  // Get UserAccount to check exists User
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetUserAccount, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }

    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "LoginAndUpdateCoreToken() failed.";

        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Get exists account
  QJsonDocument json_doc = QJsonDocument::fromJson(QString("{%1}").arg(str_result).toUtf8());
  QJsonObject json_obj = json_doc.object();
  // Delete not use UserAccount
  for (const QString dut_account : json_obj.keys()) {
    dut_old_accounts.insert(dut_account);
  }

  // Check New create account's password not empty
  for (auto edit_account : user_account_table.GetAccounts().keys()) {
    if (!dut_old_accounts.contains(edit_account)) {  // new account
      if (user_account_table.GetAccounts()[edit_account].GetPassword().isEmpty()) {
        QString error_msg = QString("UserAccount \"%1\" has an empty password").arg(edit_account);
        qCritical() << __func__ << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }
  }

  // Patch UserAccount
  login_retry = true;
  unavailable_retry = true;
  while (true) {
    act_status = client_agent.PatchUserAccount(user_account_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }

    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchUserAccount() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Use the New Admin Account login
  auto retry_times = 3;
  while (retry_times > 0) {
    retry_times = retry_times - 1;
    SLEEP_MS(1000);  // wait dut update db
    act_status = LoginAndUpdateCoreToken(client_agent, tmp_new_device);
    if (IsActStatusSuccess(act_status)) {
      break;
    }
  }

  // Delete not using UserAccount
  for (auto dut_account_key : dut_old_accounts) {
    if (!user_account_table.GetAccounts().contains(dut_account_key)) {
      // Delete UserAccount
      login_retry = true;
      unavailable_retry = true;
      while (true) {
        act_status = client_agent.DeleteUserAccount(dut_account_key);
        if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
          unavailable_retry = false;
          SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
          continue;
        }

        if (login_retry && IsActStatusUnauthorized(act_status)) {
          login_retry = false;
          // Try to login & retry access
          act_status = LoginAndUpdateCoreToken(client_agent, tmp_new_device);
          if (!IsActStatusSuccess(act_status)) {
            qCritical() << __func__ << "LoginAndUpdateCoreToken() failed.";

            return act_status;
          }
          continue;
        }

        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "DeleteUserAccount() failed. UserAccount:" << dut_account_key;
          return act_status;
        }

        // Success would break loop
        break;
      }
    }
  }

  // // Get UserAccount
  // QString str_result;
  // login_retry = true;
  // while (true) {
  //   act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetUserAccount, str_result);
  //   if (login_retry && IsActStatusUnauthorized(act_status)) {
  //     login_retry = false;
  //     // Try to login & retry access
  //     act_status = LoginAndUpdateCoreToken(client_agent, tmp_new_device);
  //     if (!IsActStatusSuccess(act_status)) {
  //       qCritical() << __func__ << "LoginAndUpdateCoreToken() failed.";

  //       return act_status;
  //     }
  //     continue;
  //   }

  //   if (!IsActStatusSuccess(act_status)) {
  //     qCritical() << __func__ << "GetStringRequestUseToken() failed.";
  //     return act_status;
  //   }

  //   // Success would break loop
  //   break;
  // }

  // QJsonDocument json_doc = QJsonDocument::fromJson(QString("{%1}").arg(str_result).toUtf8());
  // QJsonObject json_obj = json_doc.object();
  // // Delete not use UserAccount
  // for (const QString dut_account_key : json_obj.keys()) {
  //   if (!user_account_table.GetAccounts().contains(dut_account_key)) {
  //     // Delete UserAccount
  //     login_retry = true;
  //     while (true) {
  //       act_status = client_agent.DeleteUserAccount(dut_account_key);
  //       if (login_retry && IsActStatusUnauthorized(act_status)) {
  //         login_retry = false;
  //         // Try to login & retry access
  //         act_status = LoginAndUpdateCoreToken(client_agent, tmp_new_device);
  //         if (!IsActStatusSuccess(act_status)) {
  //           qCritical() << __func__ << "LoginAndUpdateCoreToken() failed.";

  //           return act_status;
  //         }
  //         continue;
  //       }

  //       if (!IsActStatusSuccess(act_status)) {
  //         qCritical() << __func__ << "DeleteUserAccount() failed. UserAccount:" << dut_account_key;
  //         return act_status;
  //       }

  //       // Success would break loop
  //       break;
  //     }
  //   }
  // }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetUserAccount(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem,
                                                   ActUserAccountTable &result_user_account_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetUserAccount, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(UserAccount) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  result_user_account_table.GetAccounts().clear();
  QJsonDocument json_doc = QJsonDocument::fromJson(QString("{%1}").arg(str_result).toUtf8());
  QJsonObject json_obj = json_doc.object();

  // Get account by key
  for (const QString &key : json_obj.keys()) {
    ActClientUserAccount client_user_account;
    QJsonObject account_obj = json_obj[key].toObject();
    client_user_account.FromString(QString(QJsonDocument(account_obj).toJson()));

    ActUserAccount user_account;
    user_account.SetActive(client_user_account.Getactive());
    user_account.SetUsername(client_user_account.GetuserName());
    user_account.SetPassword(client_user_account.Getpassword());
    user_account.SetRole(kActUserAccountRoleEnumMap[client_user_account.Getrole()]);
    user_account.SetEmail(client_user_account.Getemail());
    result_user_account_table.GetAccounts()[key] = user_account;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetLoginPolicy(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem,
                                                   const ActLoginPolicyTable &login_policy_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set LoginPolicy
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PatchLoginPolicy(login_policy_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchLoginPolicy() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetLoginPolicy(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem,
                                                   ActLoginPolicyTable &result_login_policy_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetLoginPolicy, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(LoginPolicy) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientLoginPolicy client_login_policy;
  client_login_policy.FromString(QString("{%1}").arg(str_result));
  result_login_policy_table.SetLoginMessage(client_login_policy.GetwebLoginMessage());
  result_login_policy_table.SetLoginAuthenticationFailureMessage(client_login_policy.GetloginFailureMessage());

  result_login_policy_table.SetLoginFailureLockout(client_login_policy.GetenableFailureLockout());
  result_login_policy_table.SetRetryFailureThreshold(client_login_policy.GetretryFailureThreshold());
  result_login_policy_table.SetLockoutDuration(client_login_policy.GetfailureLockoutTime());
  result_login_policy_table.SetAutoLogoutAfter(client_login_policy.GetautoLogout());

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetSnmpTrapSetting(const ActDevice &device, const QString &action_key,
                                                       const ActFeatureMethodProtocol &protocol_elem,
                                                       const ActSnmpTrapSettingTable &snmp_trap_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set SnmpTrap
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PatchSnmpTrap(snmp_trap_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchSnmpTrap() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetSnmpTrapSetting(const ActDevice &device, const QString &action_key,
                                                       const ActFeatureMethodProtocol &protocol_elem,
                                                       ActSnmpTrapSettingTable &result_snmp_trap_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetSnmpTrap, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(SnmpTrap) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientSnmpTrap client_snmp_trap;
  client_snmp_trap.FromString(QString("{%1}").arg(str_result));
  QList<ActSnmpTrapHostEntry> trap_host_list;

  for (auto client_host : client_snmp_trap.Gethost()) {
    ActSnmpTrapHostEntry trap_host;
    trap_host.SetHostName(client_host.GethostName());
    trap_host.SetMode(static_cast<ActSnmpTrapModeEnum>((client_host.Getmode() > 3) ? 4 : client_host.Getmode()));
    trap_host.SetTrapCommunity(client_host.Getv1v2cCommunity());
    trap_host_list.append(trap_host);
  }

  result_snmp_trap_table.SetHostList(trap_host_list);

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetSyslogSetting(const ActDevice &device, const QString &action_key,
                                                     const ActFeatureMethodProtocol &protocol_elem,
                                                     const ActSyslogSettingTable &syslog_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set SnmpTrap
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PatchSyslogServer(syslog_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchSyslogServer() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetSyslogSetting(const ActDevice &device, const QString &action_key,
                                                     const ActFeatureMethodProtocol &protocol_elem,
                                                     ActSyslogSettingTable &result_syslog_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetSyslogServer, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(SyslogServer) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientSyslogServer client_syslog;
  client_syslog.FromString(QString("{%1}").arg(str_result));
  result_syslog_table.SetEnabled(client_syslog.GetloggingEnable());

  // Server 1
  if (1 <= client_syslog.GetsyslogFwdTable().size()) {
    auto client_entry = client_syslog.GetsyslogFwdTable()[0];
    result_syslog_table.SetSyslogServer1(client_entry.Getenable());
    result_syslog_table.SetAddress1(client_entry.GetserverAddress());
    result_syslog_table.SetPort1(client_entry.GetserverPort());
  }

  // Server 2
  if (2 <= client_syslog.GetsyslogFwdTable().size()) {
    auto client_entry = client_syslog.GetsyslogFwdTable()[1];
    result_syslog_table.SetSyslogServer2(client_entry.Getenable());
    result_syslog_table.SetAddress2(client_entry.GetserverAddress());
    result_syslog_table.SetPort2(client_entry.GetserverPort());
  }

  // Server 3
  if (3 <= client_syslog.GetsyslogFwdTable().size()) {
    auto client_entry = client_syslog.GetsyslogFwdTable()[2];
    result_syslog_table.SetSyslogServer3(client_entry.Getenable());
    result_syslog_table.SetAddress3(client_entry.GetserverAddress());
    result_syslog_table.SetPort3(client_entry.GetserverPort());
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetTsnDeviceTimeSetting(const ActDevice &device, const QString &action_key,
                                                            const ActFeatureMethodProtocol &protocol_elem,
                                                            const ActTimeSettingTable &time_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set Time
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PatchTimeForTsnDevice(time_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchTime() failed.";
      return act_status;
    }
    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetNosDeviceTimeSetting(const ActDevice &device, const QString &action_key,
                                                            const ActFeatureMethodProtocol &protocol_elem,
                                                            const ActTimeSettingTable &time_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set Time
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PatchTimeForNosDevice(time_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchTime() failed.";
      return act_status;
    }
    // Success would break loop
    break;
  }

  return act_status;
}

void ActRestfulClientHandler::ConvertClientTimeDateToActTimeDay(const ActClientTimeDate &client_time_date,
                                                                ActTimeDay &act_time_day) {
  QDate date(client_time_date.Getyear(), client_time_date.Getmonth(), client_time_date.Getdate());

  int week = (date.day() + 6) / 7;

  act_time_day.SetMonth(client_time_date.Getmonth());
  act_time_day.SetWeek(week);
  act_time_day.SetDay(date.dayOfWeek());
  act_time_day.SetHour(client_time_date.Gethour());
  act_time_day.SetMinute(client_time_date.Getminute());
}

ACT_STATUS ActRestfulClientHandler::GetTsnDeviceTimeSetting(const ActDevice &device, const QString &action_key,
                                                            const ActFeatureMethodProtocol &protocol_elem,
                                                            ActTimeSettingTable &result_time_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetTime, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(Time) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientTsnDeviceTime client_time;
  client_time.FromString(QString("{%1}").arg(str_result));
  result_time_table.SetClockSource(kActClockSourceEnumMap[client_time.GetclockSource()]);
  result_time_table.SetNTPTimeServer1(client_time.Getntp().GetntpClient()[0].GetserverAddress());
  result_time_table.SetNTPTimeServer2(client_time.Getntp().GetntpClient()[1].GetserverAddress());
  result_time_table.SetSNTPTimeServer1(client_time.Getsntp().GetsntpClient()[0].GetserverAddress());
  result_time_table.SetSNTPTimeServer2(client_time.Getsntp().GetsntpClient()[1].GetserverAddress());

  result_time_table.SetTimeZone(kActTimeZoneEnumMap[client_time.GettimeZone()]);
  result_time_table.SetDaylightSavingTime(client_time.GetdaylightSaving().Getenable());

  // // Transfer the date to the day
  ConvertClientTimeDateToActTimeDay(client_time.GetdaylightSaving().GetstartDate(), result_time_table.GetStart());
  ConvertClientTimeDateToActTimeDay(client_time.GetdaylightSaving().GetendDate(), result_time_table.GetEnd());

  // Offset
  auto offset_min = client_time.GetdaylightSaving().Getoffset();
  int hours = offset_min / 60;
  int minutes = offset_min % 60;
  QString offset = QString("%1:%2").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0'));
  result_time_table.SetOffset(offset);

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetNosDeviceTimeSetting(const ActDevice &device, const QString &action_key,
                                                            const ActFeatureMethodProtocol &protocol_elem,
                                                            ActTimeSettingTable &result_time_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetTime, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(Time) failed.";
      return act_status;
    }
    // Success would break loop
    break;
  }

  // Handle data
  ActClientNosDeviceTime client_time;
  client_time.FromString(QString("{%1}").arg(str_result));
  result_time_table.SetClockSource(kActClockSourceEnumMap[client_time.GetclockSource()]);
  result_time_table.SetNTPTimeServer1(client_time.Getntp().GetntpClient()[0].GetserverAddress());
  result_time_table.SetNTPTimeServer2(client_time.Getntp().GetntpClient()[1].GetserverAddress());
  result_time_table.SetSNTPTimeServer1(client_time.Getsntp().GetsntpClient()[0].GetserverAddress());
  result_time_table.SetSNTPTimeServer2(client_time.Getsntp().GetsntpClient()[1].GetserverAddress());

  result_time_table.SetTimeZone(kActTimeZoneEnumMap[client_time.GettimeZone()]);
  result_time_table.SetDaylightSavingTime(client_time.GetdaylightSaving().Getenable());

  result_time_table.GetStart().SetMonth(client_time.GetdaylightSaving().Getstart().Getmonth());
  result_time_table.GetStart().SetWeek(client_time.GetdaylightSaving().Getstart().Getweek());
  result_time_table.GetStart().SetDay(client_time.GetdaylightSaving().Getstart().Getday());
  result_time_table.GetStart().SetHour(client_time.GetdaylightSaving().Getstart().Gethour());
  result_time_table.GetStart().SetMinute(client_time.GetdaylightSaving().Getstart().Getminute());

  result_time_table.GetEnd().SetMonth(client_time.GetdaylightSaving().Getend().Getmonth());
  result_time_table.GetEnd().SetWeek(client_time.GetdaylightSaving().Getend().Getweek());
  result_time_table.GetEnd().SetDay(client_time.GetdaylightSaving().Getend().Getday());
  result_time_table.GetEnd().SetHour(client_time.GetdaylightSaving().Getend().Gethour());
  result_time_table.GetEnd().SetMinute(client_time.GetdaylightSaving().Getend().Getminute());

  // Offset
  auto offset_min = client_time.GetdaylightSaving().GetoffsetMin();
  int hours = offset_min / 60;
  int minutes = offset_min % 60;
  QString offset = QString("%1:%2").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0'));
  result_time_table.SetOffset(offset);

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetTimeStatus(const ActDevice &device, const QString &action_key,
                                                  const ActFeatureMethodProtocol &protocol_elem,
                                                  ActTimeDate &result_time_data) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetTimeStatus, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(Time status) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientTimeDate client_time_status;
  client_time_status.FromString(QString("{%1}").arg(str_result));

  result_time_data.SetYear(client_time_status.Getyear());
  result_time_data.SetMonth(client_time_status.Getmonth());
  result_time_data.SetDate(client_time_status.Getdate());
  result_time_data.SetHour(client_time_status.Gethour());
  result_time_data.SetMinute(client_time_status.Getminute());

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetLoopProtection(const ActDevice &device, const QString &action_key,
                                                      const ActFeatureMethodProtocol &protocol_elem,
                                                      const ActLoopProtectionTable &lp_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set LoopProtection
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PatchMxLp(lp_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchLoopProtection() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetLoopProtection(const ActDevice &device, const QString &action_key,
                                                      const ActFeatureMethodProtocol &protocol_elem,
                                                      ActLoopProtectionTable &result_lp_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetMxLp, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(LoopProtection) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientMxLp client_mx_lp;
  client_mx_lp.FromString(QString("{%1}").arg(str_result));
  result_lp_table.SetNetworkLoopProtection(client_mx_lp.GetloopProtectEnable());
  result_lp_table.SetDetectInterval(client_mx_lp.GetdetectInterval());

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetLayer2Redundancy(const ActDevice &device, const QString &action_key,
                                                        const ActFeatureMethodProtocol &protocol_elem,
                                                        const bool &active) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set MxL2Redundancy
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    auto type_it = kActMoxaL2RedundancyKeyEnumMap.find(action_key);
    if (type_it == kActMoxaL2RedundancyKeyEnumMap.end()) {
      qCritical() << __func__ << QString("Not support the %1 request item").arg(action_key).toStdString().c_str();
      return std::make_shared<ActStatusNotFound>(QString("RESTful request item(%1)").arg(action_key));
    }

    act_status = client_agent.SetMxL2Redundancy(active, type_it.value());
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "SetMxL2Redundancy() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetRstpStatus(const ActDevice &device, const QString &action_key,
                                                  const ActFeatureMethodProtocol &protocol_elem,
                                                  ActMonitorRstpStatus &result_rstp_status) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get SpanningTree
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetRstpStatus, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(RstpStatus) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientRstpStatus client_rstp_status;
  client_rstp_status.FromString(QString("{%1}").arg(str_result));

  result_rstp_status.SetDesignatedRoot(client_rstp_status.GetdesignatedRoot());
  result_rstp_status.SetForwardDelay(client_rstp_status.GetforwardDelay() / 100);
  result_rstp_status.SetHelloTime(client_rstp_status.GethelloTime() / 100);
  result_rstp_status.SetMaxAge(client_rstp_status.GetmaxAge() / 100);
  result_rstp_status.SetRootCost(client_rstp_status.GetrootCost());

  // Port status map
  result_rstp_status.GetPortStatus().clear();
  qint64 port_index = 0;
  for (auto port_entry : client_rstp_status.GetportTable()) {
    ActMonitorRstpPortStatusEntry entry;
    entry.SetEdge(port_entry.GetedgePort());
    entry.SetPortRole(static_cast<ActRstpPortRoleEnum>(port_entry.GetrstpPortRole()));
    entry.SetPortState(static_cast<ActRstpPortStateEnum>(port_entry.GetportState()));
    entry.SetPathCost(port_entry.GetpathCost());
    entry.SetRootPathCost(port_entry.GetdesignatedCost());

    result_rstp_status.GetPortStatus()[port_index + 1] = entry;
    port_index += 1;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetSpanningTree(const ActDevice &device, const QString &action_key,
                                                    const ActFeatureMethodProtocol &protocol_elem,
                                                    ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();

  // // Check Action element
  // if (!protocol_elem.GetActions().contains(action_key)) {
  //   qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
  //   return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  // }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get SpanningTree
  QString l2_redundancy_str;
  QString rstp_str;
  QString stp_str;
  QString mxrstp_str;

  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetMxL2Redundancy, l2_redundancy_str);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(L2Redundancy) failed.";
      return act_status;
    }

    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetRstp, rstp_str);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(Rstp) failed.";
      return act_status;
    }

    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetStp, stp_str);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(Stp) failed.";
      return act_status;
    }

    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetMxRstp, mxrstp_str);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(MxRstp) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientMxL2Redundancy client_l2_redundancy;
  client_l2_redundancy.FromString(QString("{%1}").arg(l2_redundancy_str));

  ActClientRstp client_rstp;
  client_rstp.FromString(QString("{%1}").arg(rstp_str));

  ActClientStp client_stp;
  client_stp.FromString(QString("{%1}").arg(stp_str));

  ActClientMxRstp client_mxrstp;
  client_mxrstp.FromString(QString("{%1}").arg(mxrstp_str));

  // MxL2Redundancy
  rstp_table.SetActive(client_l2_redundancy.Getstprstp());

  // std1w1ap (rstp)
  rstp_table.SetSpanningTreeVersion(static_cast<ActSpanningTreeVersionEnum>(client_rstp.GetspanningTreeVersion()));
  rstp_table.SetPriority(client_rstp.Getpriority());
  rstp_table.SetForwardDelay(client_rstp.GetforwardDelay() / 100);
  rstp_table.SetHelloTime(client_rstp.GethelloTime() / 100);
  rstp_table.SetMaxAge(client_rstp.GetmaxAge() / 100);

  // mxrstp
  rstp_table.SetRstpErrorRecoveryTime(client_mxrstp.GetrstpErrorRecoveryTime() / 100);
  rstp_table.SetRstpConfigSwift(client_mxrstp.GetrstpConfigSwift());
  rstp_table.SetRstpConfigRevert(client_mxrstp.GetrstpConfigRevert());

  // Port entry
  rstp_table.GetRstpPortEntries().clear();

  for (qint32 port_index = 0; port_index < client_rstp.GetportTable().size(); port_index++) {
    auto rstp_port = client_rstp.GetportTable()[port_index];
    auto stp_port = client_stp.GetportTable()[port_index];
    auto mxrstp_port = client_mxrstp.GetportTable()[port_index];

    qint32 port_id = port_index + 1;
    ActRstpPortEntry entry(port_id);

    // std1w1ap (rstp)
    entry.SetRstpEnable(rstp_port.GetrstpEnable());
    entry.SetPortPriority(rstp_port.GetportPriority());
    entry.SetPathCost(rstp_port.GetpathCost());

    // std1d1ap (stp)
    entry.SetLinkType(static_cast<ActRstpLinkTypeEnum>(stp_port.GetbridgeLinkType()));

    // mx_rstp
    entry.SetBpduGuard(mxrstp_port.GetbpduGuard());
    entry.SetRootGuard(mxrstp_port.GetrootGuard());
    entry.SetLoopGuard(mxrstp_port.GetloopGuard());
    entry.SetBpduFilter(mxrstp_port.GetbpduFilter());

    // Edge: std1w1ap (rstp) + mx_rstp
    //  - Auto: AutoEdge: true, ForceEdge: false.
    //  - Yes:  AutoEdge: false, ForceEdge: true.
    //  - No :  AutoEdge: false, ForceEdge: false.
    mxrstp_port.GetautoEdge()  ? entry.SetEdge(ActRstpEdgeEnum::kAuto)
    : rstp_port.GetforceEdge() ? entry.SetEdge(ActRstpEdgeEnum::kYes)
                               : entry.SetEdge(ActRstpEdgeEnum::kNo);

    rstp_table.GetRstpPortEntries().insert(entry);
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetSpanningTree(const ActDevice &device, const QString &action_key,
                                                    const ActFeatureMethodProtocol &protocol_elem,
                                                    const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set SpanningTree
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    auto type_it = kActMoxaSTPatchRequestKeyEnumMap.find(action_key);
    if (type_it == kActMoxaSTPatchRequestKeyEnumMap.end()) {
      qCritical() << __func__ << QString("Not support the %1 request item").arg(action_key).toStdString().c_str();
      return std::make_shared<ActStatusNotFound>(QString("RESTful request item(%1)").arg(action_key));
    }

    act_status = client_agent.PatchSpanningTree(rstp_table, type_it.value());
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchSpanningTree() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetStreamAdapterIngress(const ActDevice &device, const QString &action_key,
                                                            const ActFeatureMethodProtocol &protocol_elem,
                                                            const ActStadPortTable &stad_port_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set StreamAdapter (Ingress)
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PatchStreamAdapterIngress(
        stad_port_table, device.GetDeviceProperty().GetStreamPriorityConfigIngressIndexMax());
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchStreamAdapterIngress() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetStreamAdapterEgress(const ActDevice &device, const QString &action_key,
                                                           const ActFeatureMethodProtocol &protocol_elem,
                                                           const ActStadConfigTable &stad_config_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set StreamAdapter (Ingress)
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PatchStreamAdapterEgress(stad_config_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchStreamAdapterEgress() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetStreamAdapterIngress(const ActDevice &device, const QString &action_key,
                                                            const ActFeatureMethodProtocol &protocol_elem,
                                                            ActStadPortTable &stad_port_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetStreamAdapter, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(StreamAdapter) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  stad_port_table.GetInterfaceStadPortEntries().clear();
  stad_port_table.SetDeviceId(device.GetId());
  ActClientStreamAdapter client_stream_adapter;
  client_stream_adapter.FromString(QString("{%1}").arg(str_result));
  qint64 port_index = 0;
  for (auto client_port : client_stream_adapter.GetportTable()) {  // each port
    qint32 port_id = port_index + 1;
    ActInterfaceStadPortEntry ifs_stad_port_entry(port_id);

    qint64 rule_index = 0;
    for (auto client_rule : client_port.Getruleindex()) {  // each rule
      // Skip disenable config
      if (!client_rule.Getenable()) {
        continue;
      }

      ActStadPortEntry stad_port_entry;
      stad_port_entry.SetPortId(port_id);
      stad_port_entry.SetIngressIndex(rule_index);
      stad_port_entry.SetIndexEnable(client_rule.Getenable());
      stad_port_entry.SetVlanId(client_rule.Getvid());
      stad_port_entry.SetVlanPcp(client_rule.Getpcp());
      stad_port_entry.SetEthertypeValue(client_rule.Getethertype());
      stad_port_entry.SetSubtypeEnable(client_rule.Getframetype());
      stad_port_entry.SetSubtypeValue(client_rule.Getframetypevalue());

      // V2
      if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetPerStreamPriorityV2()) {
        stad_port_entry.GetType().clear();
        for (auto type_str : client_rule.Gettype()) {
          stad_port_entry.GetType().insert(kActStreamPriorityTypeEnumMap[type_str]);
        }
        stad_port_entry.SetUdpPort(client_rule.GetudpPort());
        stad_port_entry.SetTcpPort(client_rule.GettcpPort());
      }

      ifs_stad_port_entry.GetStadPortEntries().insert(stad_port_entry);
      rule_index = rule_index + 1;
    }

    stad_port_table.GetInterfaceStadPortEntries().insert(ifs_stad_port_entry);
    port_index = port_index + 1;
  }

  return act_status;
}
ACT_STATUS ActRestfulClientHandler::GetStreamAdapterEgress(const ActDevice &device, const QString &action_key,
                                                           const ActFeatureMethodProtocol &protocol_elem,
                                                           ActStadConfigTable &stad_config_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetStreamAdapter, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(StreamAdapter) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  stad_config_table.GetStadConfigEntries().clear();
  stad_config_table.SetDeviceId(device.GetId());
  ActClientStreamAdapter client_stream_adapter;
  client_stream_adapter.FromString(QString("{%1}").arg(str_result));
  qint64 port_index = 0;
  for (auto client_port : client_stream_adapter.GetportTable()) {  // each port
    qint32 port_id = port_index + 1;
    ActStadConfigEntry ifs_stad_config_entry(port_id, client_port.Getegressuntag() ? 1 : 2);  // (true(1), false(2)

    stad_config_table.GetStadConfigEntries().insert(ifs_stad_config_entry);
    port_index = port_index + 1;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetTimeSyncBaseConfig(const ActDevice &device, const QString &action_key,
                                                          const ActFeatureMethodProtocol &protocol_elem,
                                                          ActTimeSyncBaseConfig &time_sync_base_config) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetMxPtp, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(MxPtp) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientMxPtp client_mx_ptp;
  client_mx_ptp.FromString(QString("{%1}").arg(str_result));

  // Enable
  time_sync_base_config.SetEnabled(client_mx_ptp.Getenable());

  // Profile
  auto client_profile = client_mx_ptp.GetportTable().first().Getprofile();
  time_sync_base_config.SetProfile(static_cast<ActTimeSyncProfileEnum>(client_profile));

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetTimeSync802Dot1ASConfig(const ActDevice &device, const QString &action_key,
                                                               const ActFeatureMethodProtocol &protocol_elem,
                                                               ActTimeSync802Dot1ASConfig &time_sync_config) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetStDot1as, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(StDot1as) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientTimeSyncDot1as client_dot1as;
  client_dot1as.FromString(QString("{%1}").arg(str_result));

  time_sync_config.SetPriority1(client_dot1as.Getpriority1());
  time_sync_config.SetPriority2(client_dot1as.Getpriority2());
  time_sync_config.SetClockClass(client_dot1as.GetclockClass());
  time_sync_config.SetClockAccuracy(client_dot1as.GetclockAccuracy());
  time_sync_config.SetAccuracyAlert(client_dot1as.GetaccuracyAlert());

  time_sync_config.GetPortEntries().clear();
  qint64 port_index = 0;
  for (auto client_port : client_dot1as.GetportDS()) {  // each port
    qint32 port_id = port_index + 1;
    ActTimeSync802Dot1ASPortEntry time_sync_port_entry(port_id);
    time_sync_port_entry.SetEnable(client_port.Getenable());
    time_sync_port_entry.SetAnnounceInterval(client_port.GetannounceInterval());
    time_sync_port_entry.SetAnnounceReceiptTimeout(client_port.GetannounceReceiptTimeout());
    time_sync_port_entry.SetSyncInterval(client_port.GetsyncInterval());
    time_sync_port_entry.SetSyncReceiptTimeout(client_port.GetsyncReceiptTimeout());
    time_sync_port_entry.SetPdelayReqInterval(client_port.GetpdelayReqInterval());
    time_sync_port_entry.SetNeighborPropDelayThresh(client_port.GetneighborPropDelayThresh());

    time_sync_config.GetPortEntries().insert(time_sync_port_entry);
    port_index = port_index + 1;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetTimeSync1588Config(const ActDevice &device, const QString &action_key,
                                                          const ActFeatureMethodProtocol &protocol_elem,
                                                          ActTimeSync1588Config &time_sync_config) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetMx1588Default, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(Mx1588Default) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientTimeSync1588 client_1588;
  client_1588.FromString(QString("{%1}").arg(str_result));

  time_sync_config.SetClockType(static_cast<Act1588ClockTypeEnum>(client_1588.GetclockType()));
  time_sync_config.SetDelayMechanism(static_cast<Act1588DelayMechanismEnum>(client_1588.GetdelayMechanism()));
  time_sync_config.SetTransportType(static_cast<Act1588TransportTypeEnum>(client_1588.GettransportType()));
  time_sync_config.SetPriority1(client_1588.Getpriority1());
  time_sync_config.SetPriority2(client_1588.Getpriority2());
  time_sync_config.SetDomainNumber(client_1588.GetdomainNumber());
  time_sync_config.SetClockMode(client_1588.GettwoStepFlag() ? Act1588ClockModeEnum::kTwoStep
                                                             : Act1588ClockModeEnum::kOneStep);
  time_sync_config.SetAccuracyAlert(client_1588.GetaccuracyAlert());
  time_sync_config.SetMaximumStepsRemoved(client_1588.GetmaximumStepsRemoved());
  time_sync_config.SetClockClass(client_1588.GetclockClass());
  time_sync_config.SetClockAccuracy(client_1588.GetclockAccuracy());

  time_sync_config.GetPortEntries().clear();
  qint64 port_index = 0;
  for (auto client_port : client_1588.GetportDS()) {  // each port
    qint32 port_id = port_index + 1;
    ActTimeSync1588PortEntry time_sync_port_entry(port_id);
    time_sync_port_entry.SetEnable(client_port.Getenable());
    time_sync_port_entry.SetAnnounceInterval(client_port.GetannounceInterval());
    time_sync_port_entry.SetAnnounceReceiptTimeout(client_port.GetannounceReceiptTimeout());
    time_sync_port_entry.SetSyncInterval(client_port.GetsyncInterval());
    time_sync_port_entry.SetDelayReqInterval(client_port.GetdelayReqInterval());
    time_sync_port_entry.SetPdelayReqInterval(client_port.GetpdelayReqInterval());

    time_sync_config.GetPortEntries().insert(time_sync_port_entry);
    port_index = port_index + 1;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetTimeSyncIec61850Config(const ActDevice &device, const QString &action_key,
                                                              const ActFeatureMethodProtocol &protocol_elem,
                                                              ActTimeSyncIec61850Config &time_sync_config) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetMx1588Iec61850, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(Mx1588Iec61850) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientTimeSyncIec61850 client_iec_61850;
  client_iec_61850.FromString(QString("{%1}").arg(str_result));

  time_sync_config.SetClockType(static_cast<Act1588ClockTypeEnum>(client_iec_61850.GetclockType()));
  time_sync_config.SetDelayMechanism(static_cast<Act1588DelayMechanismEnum>(client_iec_61850.GetdelayMechanism()));
  time_sync_config.SetTransportType(static_cast<Act1588TransportTypeEnum>(client_iec_61850.GettransportType()));
  time_sync_config.SetPriority1(client_iec_61850.Getpriority1());
  time_sync_config.SetPriority2(client_iec_61850.Getpriority2());
  time_sync_config.SetDomainNumber(client_iec_61850.GetdomainNumber());
  time_sync_config.SetClockMode(client_iec_61850.GettwoStepFlag() ? Act1588ClockModeEnum::kTwoStep
                                                                  : Act1588ClockModeEnum::kOneStep);
  time_sync_config.SetAccuracyAlert(client_iec_61850.GetaccuracyAlert());
  time_sync_config.SetMaximumStepsRemoved(client_iec_61850.GetmaximumStepsRemoved());
  time_sync_config.SetClockClass(client_iec_61850.GetclockClass());
  time_sync_config.SetClockAccuracy(client_iec_61850.GetclockAccuracy());

  time_sync_config.GetPortEntries().clear();
  qint64 port_index = 0;
  for (auto client_port : client_iec_61850.GetportDS()) {  // each port
    qint32 port_id = port_index + 1;
    ActTimeSyncDefaultPortEntry time_sync_port_entry(port_id);
    time_sync_port_entry.SetEnable(client_port.Getenable());
    time_sync_port_entry.SetAnnounceInterval(client_port.GetannounceInterval());
    time_sync_port_entry.SetAnnounceReceiptTimeout(client_port.GetannounceReceiptTimeout());
    time_sync_port_entry.SetSyncInterval(client_port.GetsyncInterval());
    time_sync_port_entry.SetDelayReqInterval(client_port.GetdelayReqInterval());
    time_sync_port_entry.SetPdelayReqInterval(client_port.GetpdelayReqInterval());

    time_sync_config.GetPortEntries().insert(time_sync_port_entry);
    port_index = port_index + 1;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetTimeSyncC37Dot238Config(const ActDevice &device, const QString &action_key,
                                                               const ActFeatureMethodProtocol &protocol_elem,
                                                               ActTimeSyncC37Dot238Config &time_sync_config) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetGetMx1588C37238, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(Mx1588C37238) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientTimeSyncC37238 client_c37238;
  client_c37238.FromString(QString("{%1}").arg(str_result));

  time_sync_config.SetClockType(static_cast<Act1588ClockTypeEnum>(client_c37238.GetclockType()));
  time_sync_config.SetDelayMechanism(static_cast<Act1588DelayMechanismEnum>(client_c37238.GetdelayMechanism()));
  time_sync_config.SetTransportType(static_cast<Act1588TransportTypeEnum>(client_c37238.GettransportType()));
  time_sync_config.SetPriority1(client_c37238.Getpriority1());
  time_sync_config.SetPriority2(client_c37238.Getpriority2());
  time_sync_config.SetDomainNumber(client_c37238.GetdomainNumber());
  time_sync_config.SetClockMode(client_c37238.GettwoStepFlag() ? Act1588ClockModeEnum::kTwoStep
                                                               : Act1588ClockModeEnum::kOneStep);
  time_sync_config.SetAccuracyAlert(client_c37238.GetaccuracyAlert());
  time_sync_config.SetGrandmasterId(client_c37238.GetgrandmasterId());

  time_sync_config.SetClockClass(client_c37238.GetclockClass());
  time_sync_config.SetClockAccuracy(client_c37238.GetclockAccuracy());
  time_sync_config.SetMaximumStepsRemoved(client_c37238.GetmaximumStepsRemoved());

  time_sync_config.GetPortEntries().clear();
  qint64 port_index = 0;
  for (auto client_port : client_c37238.GetportDS()) {  // each port
    qint32 port_id = port_index + 1;
    ActTimeSyncDefaultPortEntry time_sync_port_entry(port_id);
    time_sync_port_entry.SetEnable(client_port.Getenable());
    time_sync_port_entry.SetAnnounceInterval(client_port.GetannounceInterval());
    time_sync_port_entry.SetAnnounceReceiptTimeout(client_port.GetannounceReceiptTimeout());
    time_sync_port_entry.SetSyncInterval(client_port.GetsyncInterval());
    time_sync_port_entry.SetDelayReqInterval(client_port.GetdelayReqInterval());
    time_sync_port_entry.SetPdelayReqInterval(client_port.GetpdelayReqInterval());

    time_sync_config.GetPortEntries().insert(time_sync_port_entry);
    port_index = port_index + 1;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::Reboot(const ActDevice &device, const QString &action_key,
                                           const ActFeatureMethodProtocol &protocol_elem) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Post Request
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PostRequestUseToken(ActMoxaRequestTypeEnum::kPostReboot);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PostRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::FactoryDefault(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Post Request
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PostRequestUseToken(ActMoxaRequestTypeEnum::kPostFactoryDefault);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PostRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::Locator(const ActDevice &device, const QString &action_key,
                                            const ActFeatureMethodProtocol &protocol_elem, const quint16 &duration) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Post Request
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    QString result;
    act_status = client_agent.PostLocator(duration);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PostLocator() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetEventLog(const ActDevice &device, const QString &action_key,
                                                const ActFeatureMethodProtocol &protocol_elem,
                                                ActDeviceEventLog &result_event_log) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool can_retry_again = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetLogEntry, str_result);
    if (can_retry_again && IsActStatusUnauthorized(act_status)) {
      can_retry_again = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(LogEntry) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientEventLog client_event_log;
  client_event_log.FromString(QString("{%1}").arg(str_result));
  QList<ActDeviceEventLogEntry> event_log_entries;

  for (auto client_entry : client_event_log.Getentries()) {
    ActDeviceEventLogEntry entry;
    entry.SetBootupNumber(client_entry.Getboot());
    entry.SetSeverity(static_cast<ActSeverityLevelEnum>(client_entry.Getseverity().toUInt()));
    entry.SetTimestamp(client_entry.Gettimestamp());
    entry.SetUptime(client_entry.Getuptime());
    entry.SetMessage(client_entry.Getmessage());
    event_log_entries.append(entry);
  }

  result_event_log.SetDeviceId(device.GetId());
  result_event_log.SetTotalNum(client_event_log.Gettotalnum());
  result_event_log.SetEntries(event_log_entries);

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::PreStartImport(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Post Request
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PostRequestUseToken(ActMoxaRequestTypeEnum::kPostPreStartImport);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PostRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

// ACT_STATUS ActRestfulClientHandler::GetSystemInformation(const ActDevice &device, const QString &action_key,
//                                                          const ActFeatureMethodProtocol &protocol_elem,
//                                                          ActMonitorSystemInformation &result_system_information) {
//   ACT_STATUS_INIT();

//   // Check Action element
//   if (!protocol_elem.GetActions().contains(action_key)) {
//     qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
//     return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
//   }

//   ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
//                                      device.GetRestfulConfiguration().GetPort());

//   // Init
//   client_agent.Init();
//   if (!IsActStatusSuccess(act_status)) {
//     qCritical() << __func__ << "Init() failed.";
//     return act_status;
//   }

//   // Get Token from core
//   act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
//   if (client_agent.token_.isEmpty()) {  // token empty would login and update token
//     act_status = LoginAndUpdateCoreToken(client_agent, device);
//     if (!IsActStatusSuccess(act_status)) {
//       return act_status;
//     }
//   }

//   // Get StringRequest
//   QString str_result;
//   bool login_retry = true;
bool unavailable_retry = true;
//   while (true) {
//     act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetSystemInformation, str_result);
//     if (login_retry && IsActStatusUnauthorized(act_status)) {
//       login_retry = false;
//       // Try to login & retry access
//       act_status = LoginAndUpdateCoreToken(client_agent, device);
//       if (!IsActStatusSuccess(act_status)) {
//         return act_status;
//       }
//       continue;
//     }

//     if (!IsActStatusSuccess(act_status)) {
//       qCritical() << __func__ << "GetStringRequestUseToken() failed.";
//       return act_status;
//     }

//     // Success would break loop
//     break;
//   }

//   // Handle data
//   QJsonParseError parse_error;
//   QJsonDocument json_doc = QJsonDocument::fromJson(QString("{%1}").arg(str_result).toUtf8(), &parse_error);
//   // Check parser error
//   if (parse_error.error != QJsonParseError::NoError) {
//     QString error_msg = QString("Device(%1) parser RESTful response failed. Error: %2")
//                             .arg(device.GetIpv4().GetIpAddress())
//                             .arg(parse_error.errorString());
//     qCritical() << __func__ << error_msg.toStdString().c_str();
//     return std::make_shared<ActBadRequest>(error_msg);
//   }

//   // Set result
//   QJsonObject json_obj = json_doc.object();
//   if (json_obj.contains("uptime")) {
//     result_system_information.SetSystemUptime(json_obj["uptime"].toString());
//   }

//   return act_status;
// }

ACT_STATUS ActRestfulClientHandler::GetSystemUtilization(const ActDevice &device, const QString &action_key,
                                                         const ActFeatureMethodProtocol &protocol_elem,
                                                         ActMonitorSystemUtilization &result_system_utilization) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get StringRequest
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetSystemUtilization, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientSystemUtilization client_sys_util;
  client_sys_util.FromString(QString("{%1}").arg(str_result));
  result_system_utilization.SetCPUUsage(client_sys_util.GetcpuUtilization());
  result_system_utilization.SetMemoryUsage(client_sys_util.GetmemoryUtilization());

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetPortSettingAdminStatus(const ActDevice &device, const QString &action_key,
                                                              const ActFeatureMethodProtocol &protocol_elem,
                                                              QMap<qint64, bool> &result_port_admin_status_map) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get StringRequest
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetPortSettingAdminStatus, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  result_port_admin_status_map.clear();
  ActClientIfMib client_if_mib;
  client_if_mib.FromString(QString("{%1}").arg(str_result));
  qint64 port_index = 0;
  for (auto port_entry : client_if_mib.GetportTable()) {
    result_port_admin_status_map[port_index + 1] = port_entry.Getenable();
    port_index += 1;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetPortSetting(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem,
                                                   const ActPortSettingTable &port_setting_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());
  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set PortSetting
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    auto type_it = kActMoxaPortSettingPatchRequestKeyEnumMap.find(action_key);
    if (type_it == kActMoxaPortSettingPatchRequestKeyEnumMap.end()) {
      qCritical() << __func__ << QString("Not support the %1 request item").arg(action_key).toStdString().c_str();
      return std::make_shared<ActStatusNotFound>(QString("RESTful request item(%1)").arg(action_key));
    }

    act_status = client_agent.PatchPortSetting(port_setting_table, type_it.value());
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PatchPortSetting() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetPortInfo(const ActDevice &device, const QString &action_key,
                                                const ActFeatureMethodProtocol &protocol_elem,
                                                QMap<qint64, ActDevicePortInfoEntry> &result_port_info_map) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get StringRequest
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetPortInfo, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  result_port_info_map.clear();
  ActClientPortInfo client_port_info;
  client_port_info.FromString(QString("{%1}").arg(str_result));
  qint64 port_index = 0;
  for (auto client_port : client_port_info.GetportTable()) {
    qint32 port_id = port_index + 1;

    ActDevicePortInfoEntry monitor_port_info(port_id);

    monitor_port_info.SetModuleSlot(client_port.GetmoduleSlot());
    monitor_port_info.SetModulePort(client_port.GetmodulePort());
    monitor_port_info.SetExist(client_port.Getexist());
    monitor_port_info.SetSFPInserted(client_port.GetsfpInserted());

    result_port_info_map[port_id] = monitor_port_info;
    port_index += 1;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetPortStatus(const ActDevice &device, const QString &action_key,
                                                  const ActFeatureMethodProtocol &protocol_elem,
                                                  QMap<qint64, ActMonitorPortStatusEntry> &result_port_status_map) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get StringRequest
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetPortStatus, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(kGetPortStatus) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  result_port_status_map.clear();
  ActClientPortStatus client_port_status;
  client_port_status.FromString(QString("{%1}").arg(str_result));
  qint64 port_index = 0;
  for (auto port_status_entry : client_port_status.GetportTable()) {
    ActMonitorPortStatusEntry monitor_port_status;

    // Link Status
    monitor_port_status.SetLinkStatus(static_cast<ActLinkStatusTypeEnum>(port_status_entry.GetlinkStatus()));
    if (!kActLinkStatusTypeEnumMap.values().contains(monitor_port_status.GetLinkStatus())) {
      monitor_port_status.SetLinkStatus(ActLinkStatusTypeEnum::kDown);
    }

    result_port_status_map[port_index + 1] = monitor_port_status;
    port_index += 1;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetFiberCheckStatus(
    const ActDevice &device, const QString &action_key, const ActFeatureMethodProtocol &protocol_elem,
    QMap<qint64, ActMonitorFiberCheckEntry> &result_port_fiber_map) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get StringRequest
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetFiberCheckStatus, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  QList<ActInterface> interface_list = device.GetInterfaces();
  result_port_fiber_map.clear();
  ActClientFiberCheckStatus client_fiber_status;
  client_fiber_status.FromString(QString("{%1}").arg(str_result));
  for (auto fiber_entry : client_fiber_status.GetportTable()) {
    ActMonitorFiberCheckEntry monitor_fiber_status;

    qint64 port_id = fiber_entry.GetportIndex();

    // Set device information
    monitor_fiber_status.SetDeviceId(device.GetId());
    monitor_fiber_status.SetDeviceIp(device.GetIpv4().GetIpAddress());
    monitor_fiber_status.SetInterfaceId(port_id);

    // Set interface name
    ActInterface device_interface(port_id);
    auto interface_index = interface_list.indexOf(device_interface);
    if (interface_index != -1) {
      device_interface = interface_list.at(interface_index);
      monitor_fiber_status.SetInterfaceName(device_interface.GetInterfaceName());
    } else {
      monitor_fiber_status.SetInterfaceName(QString::number(port_id));
    }

    // Set exist
    if (fiber_entry.GetmodelName().isEmpty() || fiber_entry.GetmodelName() == "N/A") {
      monitor_fiber_status.SetExist(false);
    } else {
      monitor_fiber_status.SetExist(true);
    }

    // Set Fiber status data
    monitor_fiber_status.SetModelName(fiber_entry.GetmodelName());
    monitor_fiber_status.SetSerialNumber(fiber_entry.GetserialNumber());
    monitor_fiber_status.SetWavelength(fiber_entry.Getwavelength());
    monitor_fiber_status.SetTemperatureC(fiber_entry.GettemperatureC());
    monitor_fiber_status.SetTemperatureF(fiber_entry.GettemperatureF());
    monitor_fiber_status.SetVoltage(fiber_entry.Getvoltage());
    monitor_fiber_status.SetTxPower(fiber_entry.GettxPower());
    monitor_fiber_status.SetRxPower(fiber_entry.GetrxPower());
    monitor_fiber_status.SetTemperatureLimitC(fiber_entry.GettemperatureLimitC());
    monitor_fiber_status.SetTemperatureLimitF(fiber_entry.GettemperatureLimitF());

    monitor_fiber_status.SetTxPowerLimit(fiber_entry.GettxPowerLimit());
    monitor_fiber_status.SetRxPowerLimit(fiber_entry.GetrxPowerLimit());

    result_port_fiber_map[port_id] = monitor_fiber_status;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetTrafficStatistics(
    const ActDevice &device, const QString &action_key, const ActFeatureMethodProtocol &protocol_elem,
    QMap<qint64, ActMonitorTrafficStatisticsEntry> &result_port_traffic_statistics_map) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get StringRequest
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetTrafficStatistics, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  result_port_traffic_statistics_map.clear();
  ActClientTrafficStatistics client_traffic_statistics;
  client_traffic_statistics.FromString(QString("{%1}").arg(str_result));
  qint64 port_index = 0;
  for (auto port_entry : client_traffic_statistics.GetportTable()) {
    ActMonitorTrafficStatisticsEntry monitor_traffic_statistic;

    // TxTotalOctets & TxTotalPackets & TrafficUtilization
    monitor_traffic_statistic.SetTxTotalOctets(port_entry.GettxTotalOctets());
    monitor_traffic_statistic.SetTxTotalPackets(port_entry.GettxTotalPackets());
    monitor_traffic_statistic.SetTrafficUtilization(port_entry.GettrafficUtilization().Getdata().last());

    result_port_traffic_statistics_map[port_index + 1] = monitor_traffic_statistic;
    port_index += 1;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::Get1588DefaultInfo(const ActDevice &device, const QString &action_key,
                                                       const ActFeatureMethodProtocol &protocol_elem,
                                                       ActMonitorTimeSyncStatus &result_time_sync_status) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get StringRequest
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGet1588DefaultInfo, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClient1588DefaultInfo client_1588_default_info;
  client_1588_default_info.FromString(QString("{%1}").arg(str_result));

  result_time_sync_status.SetGrandmasterIdentity(client_1588_default_info.GetparentDS().GetgrandmasterIdentity());
  result_time_sync_status.SetParentIdentity(client_1588_default_info.GetparentDS().GetparentClockIdentity());
  result_time_sync_status.SetOffsetFromMaster(client_1588_default_info.GetoffsetFromMaster());
  result_time_sync_status.SetStepsRemoved(client_1588_default_info.GetstepsRemoved());

  // Port State map
  qint64 port_index = 0;
  result_time_sync_status.GetPortState().clear();
  for (auto port_entry : client_1588_default_info.GetportDS()) {
    result_time_sync_status.GetPortState()[port_index + 1] = port_entry.GetportState();
    port_index += 1;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetDot1asInfo(const ActDevice &device, const QString &action_key,
                                                  const ActFeatureMethodProtocol &protocol_elem,
                                                  ActMonitorTimeSyncStatus &result_time_sync_status) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get StringRequest
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetDot1asInfo, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientDot1asInfo client_dot1as_info;
  client_dot1as_info.FromString(QString("{%1}").arg(str_result));

  result_time_sync_status.SetGrandmasterIdentity(client_dot1as_info.GetparentDS().GetgrandmasterIdentity());
  result_time_sync_status.SetParentIdentity(client_dot1as_info.GetparentDS().GetparentClockIdentity());
  result_time_sync_status.SetOffsetFromMaster(client_dot1as_info.GetoffsetFromMaster());
  result_time_sync_status.SetStepsRemoved(client_dot1as_info.GetstepsRemoved());

  // Port State map
  qint64 port_index = 0;
  result_time_sync_status.GetPortState().clear();
  for (auto port_entry : client_dot1as_info.GetportDS()) {
    result_time_sync_status.GetPortState()[port_index + 1] = port_entry.GetportRole();
    port_index += 1;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetStdVlanTable(const ActDevice &device, const QString &action_key,
                                                    const ActFeatureMethodProtocol &protocol_elem,
                                                    ActVlanStaticTable &result_vlan_static_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get StringRequest VLAN
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetStdVlan, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Set TE-MSTID
  // Device support the TEMSTID
  QList<qint32> dut_enable_temstid_vlans;
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetTEMSTID()) {
    QString temstid_str_result;
    bool login_retry = true;
    bool unavailable_retry = true;
    while (true) {
      act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetTEMSTID, temstid_str_result);
      if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
        unavailable_retry = false;
        SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
        continue;
      }
      if (login_retry && IsActStatusUnauthorized(act_status)) {
        login_retry = false;
        // Try to login & retry access
        act_status = LoginAndUpdateCoreToken(client_agent, device);
        if (!IsActStatusSuccess(act_status)) {
          return act_status;
        }
        continue;
      }

      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "GetStringRequestUseToken() failed.";
        return act_status;
      }

      // Success would break loop
      break;
    }
    // Handle  TE-MSTID data
    QJsonDocument json_doc = QJsonDocument::fromJson(QString("[%1]").arg(temstid_str_result).toUtf8());
    // Modify Port entry object
    if (!json_doc.isNull() && json_doc.isArray()) {
      QJsonArray json_array = json_doc.array();
      for (const QJsonValue &value : json_array) {
        dut_enable_temstid_vlans.append(static_cast<qint32>(value.toInt()));
      }
    }
  }

  // Handle VLAN data
  ActVlanStaticTable vlan_static_table(device.GetId());
  ActClientStdVlan client_std_vlan;
  client_std_vlan.FromString(QString("{%1}").arg(str_result));
  for (auto std_vlan_entry : client_std_vlan.GetvlanTable()) {
    ActVlanStaticEntry vlan_static_entry;
    vlan_static_entry.SetVlanId(std_vlan_entry.Getvid());
    vlan_static_entry.SetName(std_vlan_entry.GetvlanName());
    vlan_static_entry.SetRowStatus(1);
    // For each port
    for (qint64 index = 0; index < client_std_vlan.GetportTable().size(); index++) {
      qint64 port_id = index + 1;
      // egressPortsPbmp
      if (std_vlan_entry.GetegressPortsPbmp().size() > index) {
        if (std_vlan_entry.GetegressPortsPbmp()[index]) {
          vlan_static_entry.GetEgressPorts().insert(port_id);
        }
      }

      // untaggedPortsPbmp
      if (std_vlan_entry.GetuntaggedPortsPbmp().size() > index) {
        if (std_vlan_entry.GetuntaggedPortsPbmp()[index]) {
          vlan_static_entry.GetUntaggedPorts().insert(port_id);
        }
      }

      // forbiddenEgressPortsPbmp
      if (std_vlan_entry.GetforbiddenEgressPortsPbmp().size() > index) {
        if (std_vlan_entry.GetforbiddenEgressPortsPbmp()[index]) {
          vlan_static_entry.GetForbiddenEgressPorts().insert(port_id);
        }
      }
    }

    // Set TEMSTID
    if (dut_enable_temstid_vlans.contains(vlan_static_entry.GetVlanId())) {
      vlan_static_entry.SetTeMstid(true);
    } else {
      vlan_static_entry.SetTeMstid(false);
    }

    vlan_static_table.GetVlanStaticEntries().insert(vlan_static_entry);
  }

  result_vlan_static_table = vlan_static_table;
  return act_status;
}

ACT_STATUS ActRestfulClientHandler::DeleteStdVlanMember(const ActDevice &device, const QString &action_key,
                                                        const ActFeatureMethodProtocol &protocol_elem,
                                                        const QList<qint32> &delete_vlan_list) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  bool support_temstid = device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetTEMSTID();

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set VLAN table
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.DeleteStdVlanMember(delete_vlan_list, support_temstid);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "SetStdVlanTable() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::AddStdVlanMember(const ActDevice &device, const QString &action_key,
                                                     const ActFeatureMethodProtocol &protocol_elem,
                                                     const QList<qint32> &add_vlan_list) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set VLAN table
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.AddStdVlanMember(add_vlan_list);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "AddStdVlanMember() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetStdVlanTable(const ActDevice &device, const QString &action_key,
                                                    const ActFeatureMethodProtocol &protocol_elem,
                                                    const ActVlanStaticTable &vlan_static_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set VLAN table(Name, Ports)
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.SetStdVlanTable(vlan_static_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "SetStdVlanTable() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Set TE-MSTID
  if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetTEMSTID()) {
    // Check vlan_static_table not empty

    bool login_retry = true;
    bool unavailable_retry = true;
    while (true) {
      act_status = client_agent.SetTEMSTID(vlan_static_table);
      if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
        unavailable_retry = false;
        SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
        continue;
      }
      if (login_retry && IsActStatusUnauthorized(act_status)) {
        login_retry = false;
        // Try to login & retry access
        act_status = LoginAndUpdateCoreToken(client_agent, device);
        if (!IsActStatusSuccess(act_status)) {
          return act_status;
        }
        continue;
      }

      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "SetStdVlanTable() failed.";
        return act_status;
      }

      // Success would break loop
      break;
    }
  }

  return act_status;
}
ACT_STATUS ActRestfulClientHandler::GetStdVlanPVID(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem,
                                                   ActPortVlanTable &result_port_vlan_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get StringRequest
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetStdVlan, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActPortVlanTable port_vlan_table(device.GetId());
  ActClientStdVlan client_std_vlan;
  client_std_vlan.FromString(QString("{%1}").arg(str_result));

  qint64 number_of_ports = client_std_vlan.GetportTable().size();
  for (qint64 index = 0; index < number_of_ports; index++) {
    qint64 port_id = index + 1;
    ActPortVlanEntry port_vlan_entry;
    port_vlan_entry.SetPortId(port_id);
    port_vlan_entry.SetPVID(client_std_vlan.GetportTable()[index].Getpvid());

    port_vlan_table.GetPortVlanEntries().insert(port_vlan_entry);
  }

  result_port_vlan_table = port_vlan_table;
  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetVlanPortType(const ActDevice &device, const QString &action_key,
                                                    const ActFeatureMethodProtocol &protocol_elem,
                                                    ActVlanPortTypeTable &result_vlan_port_type_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get StringRequest
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetMxVlan, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActVlanPortTypeTable port_type_table(device.GetId());
  ActClientMxVlan client_mx_vlan;
  client_mx_vlan.FromString(QString("{%1}").arg(str_result));

  qint64 number_of_ports = client_mx_vlan.GetportTable().size();
  for (qint64 index = 0; index < number_of_ports; index++) {
    qint64 port_id = index + 1;
    ActVlanPortTypeEntry port_type_entry;
    port_type_entry.SetPortId(port_id);
    port_type_entry.SetVlanPortType(
        static_cast<ActVlanPortTypeEnum>(client_mx_vlan.GetportTable()[index].GetvlanPortType()));
    // kActVlanPortTypeEnumMap
    port_type_table.GetVlanPortTypeEntries().insert(port_type_entry);
  }

  result_vlan_port_type_table = port_type_table;
  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetVlanPortType(const ActDevice &device, const QString &action_key,
                                                    const ActFeatureMethodProtocol &protocol_elem,
                                                    const ActVlanPortTypeTable &vlan_port_type_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set VLAN table
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.SetMxVlanPortType(vlan_port_type_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "SetMxVlanPortType() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetStdVlanPVID(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem,
                                                   const ActPortVlanTable &port_vlan_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set VLAN table
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.SetStdVlanPVID(port_vlan_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "SetStdVlanPVID() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetQosDefaultPriority(const ActDevice &device, const QString &action_key,
                                                          const ActFeatureMethodProtocol &protocol_elem,
                                                          ActDefaultPriorityTable &result_pcp_table) {
  ACT_STATUS_INIT();
  result_pcp_table.GetDefaultPriorityEntries().clear();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  //

  // Get Data
  QString qos_str;
  QString port_status_str;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetMxQos, qos_str);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(QosDefaultPriority) failed.";
      return act_status;
    }

    // Use for getting the port number
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetPortStatus, port_status_str);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(kGetPortStatus) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  ActClientMxQos mx_qos;
  mx_qos.FromString(QString("{%1}").arg(qos_str));
  ActClientPortStatus client_port_status;
  client_port_status.FromString(QString("{%1}").arg(port_status_str));

  qint64 number_of_ports = client_port_status.GetportTable().size();
  for (qint64 index = 0; index < number_of_ports; index++) {
    qint64 port_id = index + 1;
    auto pcp = mx_qos.GetdefaultPriorityTable()[index].GetdefaultPriorityValue();
    ActDefaultPriorityEntry entry(port_id, pcp);
    result_pcp_table.GetDefaultPriorityEntries().insert(entry);
  }
  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetQosDefaultPriority(const ActDevice &device, const QString &action_key,
                                                          const ActFeatureMethodProtocol &protocol_elem,
                                                          const ActDefaultPriorityTable &default_priority_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set VLAN table
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.SetDefaultPriority(default_priority_table);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "SetDefaultPriority() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::GetManagementVlan(const ActDevice &device, const QString &action_key,
                                                      const ActFeatureMethodProtocol &protocol_elem,
                                                      qint32 &result_mgmt_vlan) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Get Data
  QString str_result;
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.GetStringRequestUseToken(ActMoxaRequestTypeEnum::kGetMgmtVlan, str_result);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GetStringRequestUseToken(MgmtVlan) failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  // Handle data
  result_mgmt_vlan = static_cast<qint32>(str_result.toInt());

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::SetManagementVlan(const ActDevice &device, const QString &action_key,
                                                      const ActFeatureMethodProtocol &protocol_elem,
                                                      const qint32 &mgmt_vlan) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Set VLAN table
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.SetMgmtVlan(mgmt_vlan);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "SetMgmtVlan() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::ImportConfig(const ActDevice &device, const QString &action_key,
                                                 const ActFeatureMethodProtocol &protocol_elem,
                                                 const QString &file_path) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Import Config
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PostImportConfig(file_path);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PostImportConfig() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}

ACT_STATUS ActRestfulClientHandler::ExportConfig(const ActDevice &device, const QString &action_key,
                                                 const ActFeatureMethodProtocol &protocol_elem,
                                                 const QString &file_path) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  ActMoxaIEIClientAgent client_agent(device.GetIpv4().GetIpAddress(), device.GetRestfulConfiguration().GetProtocol(),
                                     device.GetRestfulConfiguration().GetPort());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Token from core
  act::core::g_core.GetDeviceRESTfulTokenMap(device.GetId(), client_agent.token_);
  if (client_agent.token_.isEmpty()) {  // token empty would login and update token
    act_status = LoginAndUpdateCoreToken(client_agent, device);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Import Config
  bool login_retry = true;
  bool unavailable_retry = true;
  while (true) {
    act_status = client_agent.PostExportConfig(file_path);
    if (unavailable_retry && IsActStatusServiceUnavailable(act_status)) {
      unavailable_retry = false;
      SLEEP_MS(ACT_SERVICE_UNAVAILABLE_SLEEP_TIME);
      continue;
    }
    if (login_retry && IsActStatusUnauthorized(act_status)) {
      login_retry = false;
      // Try to login & retry access
      act_status = LoginAndUpdateCoreToken(client_agent, device);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }
      continue;
    }

    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "PostExportConfig() failed.";
      return act_status;
    }

    // Success would break loop
    break;
  }

  return act_status;
}
