#include "act_offline_config.hpp"

#include "act_maf_client_handler.hpp"

ACT_STATUS act::offline_config::GenerateOfflineConfig(const ActProject &project, const qint64 &device_id,
                                                      QString &result_file_id) {
  ACT_STATUS_INIT();

  act_status = RemoveJsonFile();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "RemoveJsonFile() failed.";
    return act_status;
  }

  QJsonArray feature_list;
  act_status = ConvertToJson(project, device_id, feature_list);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ConvertToJson() failed.";
    return act_status;
  }

  // get informations (model name and generated filename) that tool needed
  ActDevice device;
  project.GetDeviceById(device, device_id);

  MafGenOfflineConfigRequest api_request;
  QJsonObject secret_setting;
  act_status = GenerateApiRequest(device, api_request, secret_setting);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GenerateApiRequest() failed.";
    return act_status;
  }

  act_status = GenerateConfigFileByAPI(api_request, secret_setting, feature_list, result_file_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GenerateConfigFileByAPI() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::offline_config::RemoveJsonFile() {
  ACT_STATUS_INIT();

  QString filename = GetInputJsonFilePath();
  QFile file(filename);

  if (!QFile::exists(filename)) {
    //  not find the file, skip
    return act_status;
  }

  if (file.remove()) {  // remove file successfully
    return act_status;
  } else {
    return std::make_shared<ActStatusInternalError>("Remove offline_config_input.json fail. reason: " +
                                                    file.errorString());
  }
}

ACT_STATUS act::offline_config::ConvertToJson(const ActProject &project, const qint64 &device_id,
                                              QJsonArray &feature_list) {
  ACT_STATUS_INIT();

  QJsonArray root_array;
  const ActDeviceConfig device_config_ = project.GetDeviceConfig();

  QSet<ActDevice> device_set = project.GetDevices();
  ActDevice device;
  act_status = ActGetItemById<ActDevice>(device_set, device_id, device);
  QString model_name = device.GetDeviceProperty().GetModelName();

  auto featureGroup = device.GetDeviceProperty().GetFeatureGroup();

  QMap<qint64, QString> interface_names = GetInterfaceNameMap(device);

  // features
  bool supportIpSetting = featureGroup.GetConfiguration().GetNetworkSetting();
  if (supportIpSetting) {
    act_status = GenL2NetworkSetting(device_config_, device_id, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenL2NetworkSetting() failed.";
      return act_status;
    }
  }

  bool supportLoginPolicy = featureGroup.GetConfiguration().GetLoginPolicy();
  if (supportLoginPolicy) {
    act_status = GenLoginPolicy(device_config_, device_id, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenLoginPolicy() failed.";
      return act_status;
    }
  }

  bool supportLoopProtection = featureGroup.GetConfiguration().GetLoopProtection();
  if (supportLoopProtection) {
    act_status = GenLoopProtection(device_config_, device_id, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenLoopProtection() failed.";
      return act_status;
    }
  }

  bool supportSNMPTrapSetting = featureGroup.GetConfiguration().GetSNMPTrapSetting();
  if (supportSNMPTrapSetting) {
    act_status = GenSNMPTrapSetting(device_config_, device_id, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenSNMPTrapSetting() failed.";
      return act_status;
    }
  }

  bool supportSyslogSetting = featureGroup.GetConfiguration().GetSyslogSetting();
  if (supportSyslogSetting) {
    act_status = GenSyslogSetting(device_config_, device_id, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenSyslogSetting() failed.";
      return act_status;
    }
  }

  bool supportTimeSetting = featureGroup.GetConfiguration().GetTimeSetting().GetSystemTime();
  if (supportTimeSetting) {
    if (model_name.contains("TSN")) {
      act_status = GenTsnTimeSetting(device_config_, device_id, &root_array);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "GenTsnTimeSetting() failed.";

        QString error_msg =
            QString("device - %1, %2").arg(device.GetIpv4().GetIpAddress()).arg(act_status->GetErrorMessage());
        return std::make_shared<ActStatusInternalError>(error_msg);
      }
    } else {
      act_status = GenNosTimeSetting(device_config_, device_id, &root_array);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "GenNosTimeSetting() failed.";
        return act_status;
      }
    }
  }

  bool supportInformationSetting = featureGroup.GetConfiguration().GetInformationSetting();
  if (supportInformationSetting) {
    act_status = GenInformationSetting(device_config_, device_id, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenInformationSetting() failed.";
      return act_status;
    }
  }

  bool supportPortSetting = featureGroup.GetConfiguration().GetPortSetting().GetAdminStatus();
  if (supportPortSetting) {
    act_status = GenPortSetting(device_config_, device_id, interface_names, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenPortSetting() failed.";
      return act_status;
    }
  }

  // VLAN - TSN is different with NOS
  if (model_name.contains("TSN")) {
    // support hybrid = support acceptable-frame-type command
    const bool supportHybrid = featureGroup.GetConfiguration().GetVLANSetting().GetHybridMode();

    act_status = GenTsnVlanSetting(supportHybrid, device_config_, device_id, interface_names, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenTsnVlanSetting() failed.";
      return act_status;
    }
  } else {
    act_status = GenNosVlanSetting(device_config_, device_id, interface_names, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenNosVlanSetting() failed.";
      return act_status;
    }
  }

  bool supportStaticMultiForwardSetting = featureGroup.GetConfiguration().GetStaticForwardSetting().GetMulticast();
  bool supportStaticUniForwardSetting = featureGroup.GetConfiguration().GetStaticForwardSetting().GetUnicast();
  if (supportStaticMultiForwardSetting || supportStaticUniForwardSetting) {
    act_status = GenStaticForwardSetting(device_config_, device_id, interface_names, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenStaticForwardSetting() failed.";
      return act_status;
    }
  }

  act_status = GenPortDefaultPriority(device_config_, device_id, interface_names, &root_array);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GenPortDefaultPriority() failed.";
    return act_status;
  }

  // TSN functions
  bool supportStreamAdapterV1 = featureGroup.GetConfiguration().GetVLANSetting().GetPerStreamPriority();
  if (supportStreamAdapterV1) {
    act_status = GenStreamAdapterV1(device_config_, device_id, interface_names, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenStreamAdapterV1() failed.";
      return act_status;
    }
  }

  bool supportStreamAdapterV2 = featureGroup.GetConfiguration().GetVLANSetting().GetPerStreamPriorityV2();
  if (supportStreamAdapterV2) {
    act_status = GenStreamAdapterV2(device_config_, device_id, interface_names, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenStreamAdapterV2() failed.";
      return act_status;
    }
  }

  bool supportQbv = featureGroup.GetConfiguration().GetTSN().GetIEEE802Dot1Qbv();
  if (supportQbv) {
    act_status = GenGCL(device_config_, device_id, interface_names, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenGCL() failed.";
      return act_status;
    }
  }

  bool supportCB = featureGroup.GetConfiguration().GetTSN().GetIEEE802Dot1CB();
  if (supportCB) {
    QMap<QString, QString> interface_names_from_string = GetInterfaceNameMapFromString(device);

    act_status = GenCB(device_config_, device_id, interface_names_from_string, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenCB() failed.";
      return act_status;
    }
  }

  // RSTP - TSN is different with NOS
  bool supportRSTP = featureGroup.GetConfiguration().GetSTPRSTP().GetRSTP();
  if (supportRSTP) {
    if (model_name.contains("TSN")) {
      act_status = GenTsnRstp(device_config_, device_id, interface_names, &root_array);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "GenTsnRstp() failed.";
        return act_status;
      }
    } else {
      act_status = GenNosRstp(device_config_, device_id, interface_names, &root_array);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "GenNosRstp() failed.";
        return act_status;
      }
    }
  }

  // Time Sync
  bool supportTimeSync = featureGroup.GetConfiguration().GetTimeSyncSetting().CheckSupportAnyOne();
  if (supportTimeSync) {
    ActTimeSyncSettingItem feature_group = featureGroup.GetConfiguration().GetTimeSyncSetting();

    act_status = GenTimeSync(feature_group, device_config_, device_id, interface_names, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenTimeSync() failed.";
      return act_status;
    }
  }

  bool supportManagementInterface = featureGroup.GetConfiguration().GetManagementInterface();
  if (supportManagementInterface) {
    act_status = GenMgmtInterface(device_config_, device_id, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenMgmtInterface() failed.";
      return act_status;
    }
  }

  bool supportUserAccount = featureGroup.GetConfiguration().GetUserAccount();
  if (supportUserAccount) {
    act_status = GenUserAccount(device_config_, device_id, &root_array);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "GenUserAccount() failed.";
      return act_status;
    }
  }

  act_status = SaveJsonToFile(QJsonDocument(root_array));
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "SaveJsonToFile() failed.";
    return std::make_shared<ActStatusInternalError>("Convert To Json");
  }

  feature_list = root_array;

  return act_status;
}

QMap<qint64, QString> act::offline_config::GetInterfaceNameMap(ActDevice &device) {
  // key = interface id; value = interface name
  QMap<qint64, QString> interfaces_name_map;
  device.GetInterfacesIdNameMap(interfaces_name_map);

  QString first_interface_name = interfaces_name_map[1];

  if (first_interface_name.contains("Eth")) {
    // Eth 1/1 -> 1/1
    for (auto it = interfaces_name_map.begin(); it != interfaces_name_map.end(); it++) {
      QString config_interface;
      QString interface_name = it.value();
      config_interface = interface_name.remove("Eth");
      it.value() = config_interface;
    }
  } else if (first_interface_name.contains("G")) {
    // G1 -> 1/1; QG1->1/1, get interface id as config interface name
    for (auto it = interfaces_name_map.begin(); it != interfaces_name_map.end(); it++) {
      QString config_interface;
      QString interface_id = QString::number(it.key());
      config_interface = "1/" + interface_id;
      it.value() = config_interface;
    }
  } else {
    // 1 -> 1/1
    for (auto it = interfaces_name_map.begin(); it != interfaces_name_map.end(); it++) {
      QString config_interface;
      QString interface_name = it.value();
      config_interface = "1/" + interface_name;
      it.value() = config_interface;
    }
  }
  return interfaces_name_map;
}

// CB using only
QMap<QString, QString> act::offline_config::GetInterfaceNameMapFromString(ActDevice &device) {
  // key = interface id; value = interface name
  QMap<qint64, QString> interfaces_name_map;
  device.GetInterfacesIdNameMap(interfaces_name_map);

  // key = interface name; value = interface name (used in config line)
  QMap<QString, QString> config_interface_name_map;

  // traverse the map
  for (auto it = interfaces_name_map.begin(); it != interfaces_name_map.end(); it++) {
    QString interface_name = it.value();
    QString config_interface;

    // Eth 1/1 -> 1/1
    if (interface_name.contains("Eth")) {
      config_interface = interface_name.remove("Eth");
    } else {
      // 1 -> 1/1
      config_interface = "1/" + interface_name;
    }

    config_interface_name_map[interface_name] = config_interface;
  }
  return config_interface_name_map;
}

void act::offline_config::AppendElementData(QJsonArray &array, const QString &port_id, const QString &value) {
  QJsonObject element_data;
  QJsonArray values = {value};

  element_data["value"] = values;
  element_data["portID"] = port_id;

  array.append(element_data);
}

void act::offline_config::AddElementToList(QList<QJsonObject> &elements, const QString &element_name,
                                           const QJsonArray &element_data_list) {
  QJsonObject element;
  element["elementName"] = element_name;
  element["elementData"] = element_data_list;

  elements.push_back(element);
}

QJsonObject act::offline_config::CreateFeature(const QString &feature_name, QList<QJsonObject> &element_data_list) {
  QJsonObject feature_object;
  feature_object["featureName"] = feature_name;

  // QList<QJsonObject> -> QJsonArray
  QJsonArray element_data_array;
  for (const QJsonObject &element : element_data_list) {
    element_data_array.append(element);
  }

  feature_object["elementList"] = element_data_array;

  return feature_object;
}

ACT_STATUS act::offline_config::SaveJsonToFile(const QJsonDocument &doc) {
  ACT_STATUS_INIT();

  QFile file(GetInputJsonFilePath());

  if (file.open(QIODevice::WriteOnly)) {
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return act_status;
  } else {
    qDebug() << "Failed to open file for writing";
    return std::make_shared<ActStatusInternalError>("generate offline config");
  }
}

ACT_STATUS act::offline_config::GenerateApiRequest(const ActDevice &device, MafGenOfflineConfigRequest &api_request,
                                                   QJsonObject &secret_setting) {
  ACT_STATUS_INIT();

  QString device_model_name = device.GetDeviceProperty().GetModelName();
  QString physical_model_name = device.GetDeviceProperty().GetPhysicalModelName();
  QString device_ip = device.GetIpv4().GetIpAddress();
  QDateTime current_date_time = QDateTime::currentDateTime();
  // format: IP_model-name_time
  QString generated_filename =
      QString("%1_%2_%3").arg(device_ip).arg(device_model_name).arg(current_date_time.toString("yyyyMMddhhmm"));

  api_request.SetfileName(generated_filename);
  api_request.SetmodelName(physical_model_name);
  api_request.SetexportType("zip");

  MafOfflineConfigProperties request_properties;
  QString str_device_id = QString::number(device.GetId());
  request_properties.SetdeviceID(str_device_id);
  api_request.Setproperties(request_properties);

  api_request.SettargetIP(device_ip);

  // secret
  QString username = device.GetAccount().GetUsername();
  QString password = device.GetAccount().GetPassword();

  // HTTPS
  QJsonObject https;
  https["username"] = username;
  https["password"] = password;
  https["port"] = device.GetRestfulConfiguration().GetPort();

  secret_setting["https"] = https;

  // HTTP
  QJsonObject http;
  http["username"] = username;
  http["password"] = password;
  http["port"] = 80;  // http default port

  secret_setting["http"] = http;

  // SSH
  QJsonObject ssh;
  ssh["username"] = username;
  ssh["password"] = password;
  ssh["port"] = device.GetSSHPort();

  secret_setting["ssh"] = ssh;

  // telnet
  QJsonObject telnet;
  telnet["username"] = username;
  telnet["password"] = password;
  telnet["port"] = 23;  // telnet default port

  secret_setting["telnet"] = telnet;

  // NETCONF
  QJsonObject netconf;
  netconf["username"] = username;
  netconf["password"] = password;
  netconf["sshPort"] = device.GetNetconfConfiguration().GetNetconfOverSSH().GetSSHPort();

  secret_setting["netconf"] = netconf;

  // MOXA command
  QJsonObject moxa_cmd;
  moxa_cmd["username"] = username;
  moxa_cmd["password"] = password;
  moxa_cmd["port"] = device.GetRestfulConfiguration().GetPort();

  secret_setting["moxa_cmd"] = moxa_cmd;

  // SNMP
  ActSnmpConfiguration device_snmp_setting = device.GetSnmpConfiguration();
  QString version = kActSnmpVersionEnumMap.key(device_snmp_setting.GetVersion());

  QJsonObject snmp;
  snmp["version"] = version;
  snmp["port"] = device_snmp_setting.GetPort();

  if (version == "v3") {
    // v3 only
    snmp["username"] = device_snmp_setting.GetUsername();

    QString auth_type = kActSnmpAuthenticationTypeEnumMap.key(device_snmp_setting.GetAuthenticationType());
    snmp["authType"] = auth_type;

    snmp["authPassword"] = device_snmp_setting.GetAuthenticationPassword();
    snmp["dataEncryptKey"] = device_snmp_setting.GetDataEncryptionKey();

    QString encrypt_type = kActSnmpDataEncryptionTypeEnumMap.key(device_snmp_setting.GetDataEncryptionType());
    snmp["dataEncryptType"] = encrypt_type;

    snmp["transportProtocol"] = "udp";
  } else {
    // for v1, v2
    snmp["readCommunity"] = device_snmp_setting.GetReadCommunity();
    snmp["writeCommunity"] = device_snmp_setting.GetWriteCommunity();
  }

  secret_setting["snmp"] = snmp;

  return act_status;
}

ACT_STATUS act::offline_config::GenerateConfigFileByAPI(const MafGenOfflineConfigRequest &api_request,
                                                        QJsonObject &secret_setting, const QJsonArray &feature_list,
                                                        QString &file_id) {
  ACT_STATUS_INIT();

  QString internal_api_endpoint = GetMafInternalApiUrl();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::mafClient::ActMafClient maf_client;
  MafGenOfflineConfigResponse api_response;
  act_status = maf_client.PostOfflineConfig(feature_list, secret_setting, api_request, internal_api_endpoint,
                                            http_proxy_endpoint, api_response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "PostOfflineConfig() failed.";
    return act_status;
  }

  file_id = api_response.Getdata().GetfileId();

  return act_status;
}

ACT_STATUS act::offline_config::SaveOfflineConfigToTmpFolder(const QList<qint64> &dev_id_list) {
  ACT_STATUS_INIT();

  if (dev_id_list.isEmpty()) {
    QString error_msg = QString("The Device list is empty");
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  QString internal_api_endpoint = GetMafInternalApiUrl();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::mafClient::ActMafClient maf_client;

  // QList<qint64> -> QList<QString>
  QList<QString> str_device_list;
  for (qint64 device_id : dev_id_list) {
    QString str_device_id = QString::number(device_id);
    str_device_list.append(str_device_id);
  }

  QList<QString> feature_list;
  feature_list.append("offlineConf");

  MafExportFilesProperties properties;
  properties.SetdeviceID(str_device_list);
  properties.Setorigin(feature_list);

  MafExportFilesRequest api_request;
  api_request.Setproperties(properties);

  // call MAF API
  act_status = maf_client.PostExportOfflineConfigs(api_request, internal_api_endpoint, http_proxy_endpoint);

  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "PostExportOfflineConfigs() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::offline_config::ClearMafFileDb() {
  ACT_STATUS_INIT();

  QString internal_api_endpoint = GetMafInternalApiUrl();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  act::mafClient::ActMafClient maf_client;

  QList<QString> feature_list;
  feature_list.append("offlineConf");

  MafDeleteFilesProperties properties;
  properties.Setorigin(feature_list);

  MafDeleteFilesRequest api_request;
  api_request.Setproperties(properties);

  // call MAF API - delete offline config files in file db
  act_status = maf_client.ClearOfflineConfigFiles(api_request, internal_api_endpoint, http_proxy_endpoint);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ClearOfflineConfigFiles() failed.";
    return act_status;
  }

  return act_status;
}