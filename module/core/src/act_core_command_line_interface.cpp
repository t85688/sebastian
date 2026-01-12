#include "act_core.hpp"

namespace act {
namespace core {

void ActCore::ParseVlanRequest(qint64 &project_id, const qint64 &ws_listener_id, QString &cli_request) {
  ACT_STATUS_INIT();

  ActIntelligentResponse sys_response;
  sys_response.Getresponse().Setrole("Assistant");
  sys_response.Getstatus().Setcode(200);
  sys_response.Getstatus().Setmessage("OK");
  QString &response = sys_response.Getresponse().Getreply().Getreply();

  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    response = act_status->GetErrorMessage();
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, sys_response),
                                ws_listener_id);
    return;
  }

  QRegularExpression re(
      "Hi Chamberlain, (?<action>(add|delete)) VLAN (?<vlan_id>\\d+) (from|to) (?<ports>((port \\d+)|(all ports))) on "
      "(?<devices>((((25[0-5]|2[0-4][0-9]|[01]?[0-9]{1,2}\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9]{1,2})(,\\s*){0,}){"
      "1,})|(all switches)))( with (?<type>(Access|Trunk|Hybrid)) type){0,1}");
  QRegularExpressionMatch match = re.match(cli_request);
  if (!match.hasMatch()) {
    response = QString("We are not able to find an appropriate action corresponding with your request.");
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, sys_response),
                                ws_listener_id);
    return;
  }

  QString action = match.captured("action");
  qint32 vlan_id = static_cast<qint32>(match.captured("vlan_id").toInt());
  QString ports = match.captured("ports");
  QString devices = match.captured("devices");
  ActVlanPortTypeEnum type = (match.captured("type") == "Hybrid")  ? ActVlanPortTypeEnum::kHybrid
                             : (match.captured("type") == "Trunk") ? ActVlanPortTypeEnum::kTrunk
                                                                   : ActVlanPortTypeEnum::kAccess;

  QList<QString> ip_list;
  if (!devices.compare("all switches")) {
    for (ActDevice device : project.GetDevices()) {
      if ((type == ActVlanPortTypeEnum::kAccess || type == ActVlanPortTypeEnum::kTrunk || !action.compare("delete")) &&
          !device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetAccessTrunkMode()) {
        continue;
      } else if ((type == ActVlanPortTypeEnum::kHybrid || !action.compare("delete")) &&
                 !device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) {
        continue;
      }
      ip_list.append(device.GetIpv4().GetIpAddress());
    }
  } else {
    QRegularExpression re(
        "(?<ipList>((25[0-5]|2[0-4][0-9]|[01]?[0-9]{1,2}\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9]{1,2})(,\\s*){0,}){"
        "1,})");
    QRegularExpressionMatch match = re.match(devices);
    if (match.hasMatch()) {
      QString ipList = match.captured("ipList");
      QStringList list = ipList.split(QRegularExpression(",\\s*"));
      for (int i = 0; i < list.size(); i++) {
        ip_list.append(list[i]);
      }
    }
  }

  for (QString device_ip : ip_list) {
    ActVlanConfig vlan_config;
    vlan_config.SetVlanId(vlan_id);

    ActDevice device;
    act_status = project.GetDeviceByIp(device_ip, device);
    if (!IsActStatusSuccess(act_status)) {
      response = QString("Cannot find device %1 in project.").arg(device_ip);
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                  ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                   ActStatusType::kRunning, sys_response),
                                  ws_listener_id);
      continue;
    }

    vlan_config.GetDevices().insert(device.GetId());

    if (!ports.compare("all ports")) {
      for (ActInterface interface : device.GetInterfaces()) {
        vlan_config.GetPorts().insert(static_cast<qint32>(interface.GetInterfaceId()));
      }
    } else {
      QRegularExpression re("port (?<port>\\d+)");
      QRegularExpressionMatch match = re.match(ports);
      if (match.hasMatch()) {
        vlan_config.GetPorts().insert(static_cast<qint32>(match.captured("port").toInt()));
      }
    }

    if (!action.compare("add")) {
      vlan_config.SetPortType(type);
      act_status = this->UpdateVlanConfig(project, vlan_config);
      if (!IsActStatusSuccess(act_status)) {
        response = act_status->GetErrorMessage();
      } else {
        response = QString("Add VLAN %1 to device %2 success.").arg(vlan_id).arg(device_ip);
      }
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                  ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                   ActStatusType::kRunning, sys_response),
                                  ws_listener_id);
    } else {
      act_status = this->DeleteVlanConfig(project, vlan_config);
      if (!IsActStatusSuccess(act_status)) {
        response = act_status->GetErrorMessage();
      } else {
        response = QString("Delete VLAN %1 from device %2 success.").arg(vlan_id).arg(device_ip);
      }
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                  ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                   ActStatusType::kRunning, sys_response),
                                  ws_listener_id);
    }
  }

  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    response = act_status->GetErrorMessage();
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, sys_response),
                                ws_listener_id);
  }

  return;
}

void ActCore::ParseBackupRequest(qint64 &project_id, const qint64 &ws_listener_id, QString &cli_request,
                                 bool &caller_skip_finish) {
  // Create can not handle request
  ActIntelligentResponse cannot_handle_response;
  cannot_handle_response.Getresponse().Setrole("Assistant");
  cannot_handle_response.Getresponse().Getreply().Setreply(
      "We are not able to find an appropriate action corresponding with your request.");
  cannot_handle_response.Getresponse().Getreply().Settoolbar(false);
  cannot_handle_response.Getstatus().Setcode(200);
  cannot_handle_response.Getstatus().Setmessage("OK");

  // [feat:3344] Intelligent - Import/Export device config
  // CLI: "Hi Chamberlain, backup {IP or IP List} device configuration to {location}"
  QRegularExpression re(
      "Hi Chamberlain, (?<action>\\w+) (?<devices>[\\w\\d,\\.\\s]+) device configuration to (?<location>[\\S]+)");
  QRegularExpressionMatch match = re.match(cli_request);
  if (!match.hasMatch()) {
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, cannot_handle_response),
                                ws_listener_id);
    return;
  }
  QString action = match.captured("action");
  QString devices = match.captured("devices");
  QString location = match.captured("location");

  // Check "backup" action
  if (action != "backup") {
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, cannot_handle_response),
                                ws_listener_id);
    return;
  }

  // Get IP list
  QList<QString> ip_list;
  QList<QString> device_list = devices.split(",", Qt::SkipEmptyParts);

  // Check first device is "all" string
  if (device_list.first() == "all") {  // Export All devices
    qDebug() << "Export All devices";
  } else {
    // Append & Check each device IP address
    for (auto dev_ip : device_list) {
      dev_ip = dev_ip.trimmed();  // trim whitespace

      // Check IP address format
      QHostAddress ip_addr(dev_ip);
      if (ip_addr.isNull()) {  // not valid
        // Send failed reply to client
        ActIntelligentResponse sys_response;
        sys_response.Getresponse().Setrole("Assistant");
        sys_response.Getresponse().Getreply().Setreply(QString("Couldn't recognize the IP address (%1).").arg(dev_ip));
        sys_response.Getresponse().Getreply().Settoolbar(false);
        sys_response.Getstatus().Setcode(400);
        sys_response.Getstatus().Setmessage("OK");
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                    ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                     ActStatusType::kRunning, sys_response),
                                    ws_listener_id);
        return;
      }
      // Check not duplicate
      if (!ip_list.contains(dev_ip)) {
        ip_list.append(dev_ip);
      }
    }

    // Check ip list not empty
    if (ip_list.isEmpty()) {
      ActIntelligentResponse sys_response;
      sys_response.Getresponse().Setrole("Assistant");
      sys_response.Getresponse().Getreply().Setreply("The IP address is missing.");
      sys_response.Getresponse().Getreply().Settoolbar(false);
      sys_response.Getstatus().Setcode(400);
      sys_response.Getstatus().Setmessage("OK");
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                  ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                   ActStatusType::kRunning, sys_response),
                                  ws_listener_id);
      return;
    }

    for (auto ip : ip_list) {
      qDebug() << "Export IP:" << ip;
    }
  }

  qDebug() << "Export Location:" << location;

  // Start Export
  caller_skip_finish = true;  // The intelligent finished reply control
  auto act_status = act::core::g_core.StartIntelligentConfigExport(project_id, ws_listener_id, ip_list, location);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Start IntelligentConfigExport failed";
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActBaseResponse(ActWSCommandEnum::kStartIntelligentRequest, ActStatusType::kFinished),
                                ws_listener_id);
    return;
  }

  ActIntelligentResponse sys_response;
  sys_response.Getresponse().Setrole("Assistant");
  sys_response.Getresponse().Getreply().Setreply("Alright, please wait a moment.");
  sys_response.Getresponse().Getreply().Settoolbar(false);
  sys_response.Getresponse().Getreply().Getbuttons().append(
      ActIntelligentRecognizeButtonResponse("Abort the task", "Hi Chamberlain, abort the task."));
  sys_response.Getstatus().Setcode(200);
  sys_response.Getstatus().Setmessage("OK");

  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                              ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                               ActStatusType::kRunning, sys_response),
                              ws_listener_id);

  return;
}

void ActCore::ParseRestoreRequest(qint64 &project_id, const qint64 &ws_listener_id, QString &cli_request,
                                  bool &caller_skip_finish) {
  // Create can not handle request
  ActIntelligentResponse cannot_handle_response;
  cannot_handle_response.Getresponse().Setrole("Assistant");
  cannot_handle_response.Getresponse().Getreply().Setreply(
      "We are not able to find an appropriate action corresponding with your request.");
  cannot_handle_response.Getresponse().Getreply().Settoolbar(false);
  cannot_handle_response.Getstatus().Setcode(200);
  cannot_handle_response.Getstatus().Setmessage("OK");

  // [feat:3344] Intelligent - Import/Export device config
  // CLI: "Hi Chamberlain, restore configuration from {location} to {IP}"
  QRegularExpression re(
      "Hi Chamberlain, (?<action>\\w+) configuration from (?<location>[\\S]+) to (?<device>[\\w\\d\\.\\-_]+)");
  QRegularExpressionMatch match = re.match(cli_request);
  if (!match.hasMatch()) {
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, cannot_handle_response),
                                ws_listener_id);
    return;
  }
  QString action = match.captured("action");
  QString location = match.captured("location");
  QString dev_ip = match.captured("device");

  // Check "restore" action
  if (action != "restore") {
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, cannot_handle_response),
                                ws_listener_id);
    return;
  }

  // Get & Check IP address format
  QHostAddress ip_addr(dev_ip);
  if (ip_addr.isNull()) {  // not valid
    // Send failed reply to client
    ActIntelligentResponse sys_response;
    sys_response.Getresponse().Setrole("Assistant");
    sys_response.Getresponse().Getreply().Setreply(QString("Couldn't recognize the IP address (%1).").arg(dev_ip));
    sys_response.Getresponse().Getreply().Settoolbar(false);
    sys_response.Getstatus().Setcode(400);
    sys_response.Getstatus().Setmessage("OK");
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, sys_response),
                                ws_listener_id);
    return;
  }

  qDebug() << "Import IP:" << dev_ip;
  qDebug() << "Import Location:" << location;

  // Start Import
  QList<QString> ip_list;
  ip_list.append(dev_ip);

  caller_skip_finish = true;
  auto act_status = act::core::g_core.StartIntelligentConfigImport(project_id, ws_listener_id, ip_list, location);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Start IntelligentConfigImport failed";
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActBaseResponse(ActWSCommandEnum::kStartIntelligentRequest, ActStatusType::kFinished),
                                ws_listener_id);
    return;
  }

  ActIntelligentResponse sys_response;
  sys_response.Getresponse().Setrole("Assistant");
  sys_response.Getresponse().Getreply().Setreply("Alright, please wait a moment.");
  sys_response.Getresponse().Getreply().Settoolbar(false);
  sys_response.Getstatus().Setcode(200);
  sys_response.Getstatus().Setmessage("OK");

  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                              ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                               ActStatusType::kRunning, sys_response),
                              ws_listener_id);

  return;
}

void ActCore::ParseAbortRequest(qint64 &project_id, const qint64 &ws_listener_id, QString &cli_request) {
  // [feat:3344] Intelligent - Import/Export device config
  // CLI: "Hi Chamberlain, abort the task."

  QRegularExpression re("Hi Chamberlain, (?<action>\\w+) the task.");
  QRegularExpressionMatch match = re.match(cli_request);
  if (!match.hasMatch()) {
    return;
  }
  QString action = match.captured("action");

  // Check "abort" action
  if (action != "abort") {
    return;
  }
  auto act_status = act::core::g_core.StopWSJob(project_id, ws_listener_id, ActWSCommandEnum::kStopIntelligentRequest);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Stop WSJob failed";
    return;
  }

  return;
}

ACT_STATUS ActCore::StartCommandLineInterfaceRequest(qint64 &project_id, const qint64 &ws_listener_id,
                                                     ActIntelligentRecognizeRequest &request) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QString cli_request = request.GetText();

  if (!cli_request.startsWith(QString("Hi Chamberlain, "))) {
    return std::make_shared<ActBadRequest>("CLI request failed");
  }

  // Reply the input
  ActIntelligentResponse user_response;
  user_response.Getresponse().Setrole("User");
  user_response.Getresponse().Setinput(cli_request);
  user_response.Getstatus().Setcode(200);
  user_response.Getstatus().Setmessage("OK");
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                              ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                               ActStatusType::kRunning, user_response),
                              ws_listener_id);

  // Create can not handle request
  ActIntelligentResponse cannot_handle_response;
  cannot_handle_response.Getresponse().Setrole("Assistant");
  cannot_handle_response.Getresponse().Getreply().Setreply(
      "We are not able to find an appropriate action corresponding with your request.");
  cannot_handle_response.Getresponse().Getreply().Settoolbar(false);
  cannot_handle_response.Getstatus().Setcode(200);
  cannot_handle_response.Getstatus().Setmessage("OK");

  bool skip_reply_finish = false;
  // Get the action
  QRegularExpression re("Hi Chamberlain, (?<action>\\w+)");
  QRegularExpressionMatch match = re.match(cli_request);
  if (!match.hasMatch()) {
    // Reply can not handle
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                 ActStatusType::kRunning, cannot_handle_response),
                                ws_listener_id);
  } else {
    // Handle action
    QString action = match.captured("action");

    if (action == "add" || action == "delete") {
      this->ParseVlanRequest(project_id, ws_listener_id, cli_request);
    } else if (action == "backup") {
      this->ParseBackupRequest(project_id, ws_listener_id, cli_request, skip_reply_finish);
    } else if (action == "restore") {
      this->ParseRestoreRequest(project_id, ws_listener_id, cli_request, skip_reply_finish);
    } else if (action == "abort") {
      this->ParseAbortRequest(project_id, ws_listener_id, cli_request);
    } else {
      // Reply can not handle
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                  ActIntelligentResponseWSResponse(ActWSCommandEnum::kStartIntelligentRequest,
                                                                   ActStatusType::kRunning, cannot_handle_response),
                                  ws_listener_id);
    }
  }

  if (!skip_reply_finish) {
    // Reply finished
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false,
                                ActBaseResponse(ActWSCommandEnum::kStartIntelligentRequest, ActStatusType::kFinished),
                                ws_listener_id);
  }

  return act_status;
}
}  // namespace core
}  // namespace act