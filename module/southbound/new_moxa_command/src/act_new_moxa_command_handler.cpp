#include "act_new_moxa_command_handler.h"

#include <QDebug>
#include <QNetworkInterface>
#include <QtEndian>

#include "act_utilities.hpp"

ACT_STATUS ActNewMoxaCommandHandler::GetNewMoxaCmdErrMsg(const int &err_num, QString &err_msg) {
  ACT_STATUS_INIT();

  switch (err_num) {
    case MOXA_COMMAND_OK:
      printf("OK");
      break;
    case MOXA_COMMAND_GENERAL_ERR:
      err_msg = "General Error";
      break;
    case MOXA_COMMAND_AGAINST_PASSWORD_POLICY:
      err_msg = "The password is against the device's password policy";
      break;
    case MOXA_COMMAND_USER_PRIVILEGE:
      err_msg = "This account has only user privilege.";
      break;
    case MOXA_COMMAND_HEADER_MISMATCH:
      err_msg = "The received response's tag is different from the command you just sent.";
      break;
    case MOXA_COMMAND_HEADER_PASSWORD_TOO_LONG:
      err_msg = "The length of password is over 16 bytes";
      break;
    case MOXA_COMMAND_HEADER_INVALID_RANDOM_ID:
      err_msg = "The random ID is different from which you just sent.";
      break;
    case MOXA_COMMAND_RESPONSE_OP_CODE_MISMATCH:
      err_msg = "The received response's tag is different from the command you just sent.";
      break;
    case MOXA_COMMAND_RESPONSE_LENGTH_MISMATCH:
      err_msg = "The response random ID's length is not 16 bytes";
      break;
    case MOXA_COMMAND_EMPTY_RESPONSE:
      err_msg = "Empty response";
      break;
    case MOXA_COMMAND_FAILED_TO_OPEN_FILE:
      err_msg = "Failed to open file";
      break;
    case MOXA_COMMAND_FILE_TOO_LONG:
      err_msg = "File too large.";
      break;
    case MOXA_COMMAND_TIMEOUT:
      err_msg = "Timeout";
      break;
    case MOXA_COMMAND_WEB_SESSION_FULL:
      err_msg = "The device's web session is full.";
      break;
    case MOXA_COMMAND_INVALID_WEB_SESSION_ID:
      err_msg = "Invalid web session id";
      break;
    case MOXA_COMMAND_FIELD_TOO_LONG:
      err_msg = "The Username or password you just input is too long";
      break;
    case MOXA_COMMAND_INVALID_PRIVILEGE:
      err_msg = "This command requires higher privilege";
      break;
    case MOXA_COMMAND_INCORRECT_USERNAME_PASSWORD:
      err_msg = "Incorrect username or password.";
      break;
    case MOXA_COMMAND_DUPLICATED_SESSION_ID:
      err_msg = "Duplicated web session id. (It legal!)";
      break;
    case MOXA_COMMAND_EXPORT_ENCRYPTED_CONFIG_FILE_IS_DISABLED:
      err_msg = "The device's configuration file password is disabled.";
      break;
    default:
      if (err_num < -CONFIG_IMPORT_ERROR_CODE_OFFSET && err_num >= -CONFIG_IMPORT_ERROR_CODE_OFFSET - 0x3A) {
        err_msg = "Error on Config Import:" + err_num;
      } else {
        err_msg = "Unknown error: " + err_num;
      }
  }

  return act_status;
}

// typedef int (*SCAN_CALLBACK)(MOXA_IEI_DEVICE_T *device);
typedef int (*DLL_SEARCH_DEVICE_BY_NETWORK_INTERFACES)(MOXA_IEI_DEVICE_T *callback, unsigned int *ip_list, int count,
                                                       int port, int timeout);

QList<ActDevice> gl_act_dev_list;
QMap<QString, QString> gl_mac_host_map;
int BcastSearchCallback(MOXA_IEI_DEVICE_T *iei_dev) {
  // Get IP address
  QString ip_addr_str;
  quint32 original_addr_number = iei_dev->ip;
  ActIpv4::AddressNumberToStr(qToBigEndian(original_addr_number), ip_addr_str);

  // Get MAC address
  QString mac_str = "";
  for (int i = 0; i < 6; i++) {
    char str2[3] = {0};
    sprintf_s(str2, sizeof(str2), "%02x", (unsigned char)(iei_dev->mac[i]));
    mac_str += QString(str2).toUpper();
    if (i != 5) {
      mac_str += "-";
    }
  }

  // qDebug() << __func__
  //          << QString("Received a bcast search response device(MAC: %1, IP: %2)")
  //                 .arg(mac_str)
  //                 .arg(ip_addr_str)
  //                 .toStdString()
  //                 .c_str();

  // [bugfix:1736] Configuration wizard searching TSN switch would appear duplicate devices.
  // Check duplicated and skip it
  if (gl_mac_host_map.contains(mac_str)) {
    // qDebug()
    //     << __func__
    //     << QString("Received a duplicated device(MAC: %1, IP:
    //     %2)").arg(mac_str).arg(ip_addr_str).toStdString().c_str();
    return 0;
  }

  // Create device
  ActDevice dev(gl_act_dev_list.size() + 1);
  dev.SetDeviceName(iei_dev->name);
  dev.SetMacAddress(mac_str);

  // Set MAC int
  qint64 mac_int = 0;
  MacAddressToQInt64(dev.GetMacAddress(), mac_int);
  dev.mac_address_int = mac_int;

  // Set Ipv4
  ActIpv4 ipv4(ip_addr_str);
  dev.SetIpv4(ipv4);

  // Set ModelName
  ActDeviceProperty dev_property;
  dev_property.SetModelName(iei_dev->modelname);
  dev.SetDeviceProperty(dev_property);

  // Set DeviceInfo(Location)
  ActDeviceInfo dev_info(iei_dev->location);
  dev.SetDeviceInfo(dev_info);

  // init DeviceStatus(default all false)
  ActDeviceStatus dev_status;
  dev.SetDeviceStatus(dev_status);

  gl_act_dev_list.append(dev);

  if (mac_str != "") {  // for update mac_host_map_
    QString host_ip_addr_str;
    quint32 host_original_addr_number = iei_dev->host;
    ActIpv4::AddressNumberToStr(qToBigEndian(host_original_addr_number), host_ip_addr_str);

    gl_mac_host_map[mac_str] = host_ip_addr_str;
  }
  return 0;
}

ACT_STATUS ActNewMoxaCommandHandler::SearchDevices(const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem,
                                                   QList<ActDevice> &dev_list, QMap<QString, QString> &mac_host_map) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  int result = -1;
  dev_list.clear();
  gl_act_dev_list.clear();
  gl_mac_host_map.clear();

  unsigned int nic_list[32];
  int nic_cnt = 0;
  QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
  for (const QNetworkInterface &interface : interfaces) {
    // Check if it is an Ethernet interface
    if (interface.type() != QNetworkInterface::Ethernet) {
      continue;
    }

    // Check the interface is up and not loopback and is Ethernet
    if (interface.flags().testFlag(QNetworkInterface::IsRunning) &&
        !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
      QList<QNetworkAddressEntry> entries = interface.addressEntries();
      for (const QNetworkAddressEntry &entry : entries) {
        QHostAddress address = entry.ip();
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost) {
          // Convert to unsigned int
          quint32 ipv4Address = address.toIPv4Address();

          // Convert to big endian
          ipv4Address = qToBigEndian(ipv4Address);

          // Ensure there's space in the array
          if (nic_cnt < 32) {
            // Cast and store in the array
            nic_list[nic_cnt++] = static_cast<unsigned int>(ipv4Address);
          }
        }
      }
    }
  }

  // No device found
  if (!nic_cnt) {
    return act_status;
  }

  try {
    // Initialize the DLL
    dll_init();

    // Call the broadcast search function
    result = dll_search_device_by_network_interfaces(BcastSearchCallback, nic_list, nic_cnt,
                                                     ACT_NEW_MOXA_COMMAND_BROADCAST_PORT, ACT_NEW_MOXA_COMMAND_TIMEOUT);

  } catch (std::exception &e) {
    qCritical() << __func__ << "NewMoxaCommand Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("NewMoxaCommand");
  }

  if (result != MOXA_COMMAND_OK) {  // failed
    QString err_msg;
    GetNewMoxaCmdErrMsg(result, err_msg);
    qCritical() << __func__ << "NewMoxaCommandMessage:" << err_msg;
    return std::make_shared<ActStatusSouthboundFailed>(err_msg);
  }

  dev_list = gl_act_dev_list;

  // update mac_host_map
  foreach (auto mac, gl_mac_host_map.keys()) {  // mac: 38-F3-AB-E2-69-67
    mac_host_map[mac] = gl_mac_host_map[mac];
  }

  // clear global variable
  gl_act_dev_list.clear();
  gl_mac_host_map.clear();

  return act_status;
}

ACT_STATUS ActNewMoxaCommandHandler::SetArpTable(const ActDevice &device, const QString &act_host_ip) {
  ACT_STATUS_INIT();

  int result = -1;

  try {
    // Initialize the DLL
    dll_init();

    result = dll_set_arp_table((char *)device.GetIpv4().GetIpAddress().toStdString().c_str(),
                               (char *)device.GetMacAddress().toStdString().c_str(),
                               (char *)act_host_ip.toStdString().c_str());
  } catch (std::exception &e) {
    qCritical() << __func__ << "NewMoxaCommand Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("NewMoxaCommand");
  }

  if (result != MOXA_COMMAND_OK) {  // failed
    QString err_msg;
    GetNewMoxaCmdErrMsg(result, err_msg);
    qCritical() << __func__ << "set_arp_table() fail. NewMoxaCommandMessage:" << err_msg;
    return std::make_shared<ActStatusSouthboundFailed>(err_msg);
  }

  return act_status;
}

ACT_STATUS ActNewMoxaCommandHandler::DeleteArpEntry(const ActDevice &device) {
  ACT_STATUS_INIT();

  int result = -1;

  try {
    // Initialize the DLL
    dll_init();

    result = dll_delete_arp_table((char *)device.GetIpv4().GetIpAddress().toStdString().c_str());
    if (result != MOXA_COMMAND_OK) {  // failed
      QString err_msg;
      GetNewMoxaCmdErrMsg(result, err_msg);
      qCritical() << __func__ << "delete_arp_entry_string() fail. NewMoxaCommandMessage:" << err_msg;
      return std::make_shared<ActStatusSouthboundFailed>(err_msg);
    }
  } catch (std::exception &e) {
    qCritical() << __func__ << "NewMoxaCommand Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("NewMoxaCommand");
  }

  return act_status;
}

ACT_STATUS ActNewMoxaCommandHandler::ChangeNetworkSettings(const ActDevice &device, const QString &action_key,
                                                           const ActFeatureMethodProtocol &protocol_elem,
                                                           const ActNetworkSettingTable &network_setting_table) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  int result = -1;

  MOXA_DEVICE_NETWORK_CONFIG_T network_settings;
  memset(&network_settings, 0, sizeof(MOXA_DEVICE_NETWORK_CONFIG_T));

  if (network_setting_table.GetNetworkSettingMode() == ActNetworkSettingModeEnum::kStatic) {
    network_settings.ipconfig_flag = IP_FLAG_STATIC;  // static ip setting
  } else if (network_setting_table.GetNetworkSettingMode() == ActNetworkSettingModeEnum::kDHCP) {
    network_settings.ipconfig_flag = IP_FLAG_DHCP;  // dhcp ip setting
  } else {
    network_settings.ipconfig_flag = IP_FLAG_BOOTP;  // bootp ip setting
  }

  strcpy_s(network_settings.ip, sizeof(network_settings.ip) / sizeof(network_settings.ip[0]),
           network_setting_table.GetIpAddress().toStdString().c_str());
  strcpy_s(network_settings.netmask, sizeof(network_settings.netmask) / sizeof(network_settings.netmask[0]),
           network_setting_table.GetSubnetMask().toStdString().c_str());
  strcpy_s(network_settings.gateway, sizeof(network_settings.gateway) / sizeof(network_settings.gateway[0]),
           network_setting_table.GetGateway().toStdString().c_str());

  strcpy_s(network_settings.dns1, sizeof(network_settings.dns1) / sizeof(network_settings.dns1[0]),
           network_setting_table.GetDNS1().toStdString().c_str());
  strcpy_s(network_settings.dns2, sizeof(network_settings.dns2) / sizeof(network_settings.dns2[0]),
           network_setting_table.GetDNS2().toStdString().c_str());

  try {
    // Initialize the DLL
    dll_init();

    // [bugfix: 3299] Chamberlain device connection problem
    result = network_config_set_ex(
        (char *)device.GetIpv4().GetIpAddress().toStdString().c_str(),
        (char *)device.GetMacAddress().toStdString().c_str(), device.GetRestfulConfiguration().GetPort(),
        (char *)device.GetAccount().GetUsername().toStdString().c_str(),
        (char *)device.GetAccount().GetPassword().toStdString().c_str(), &network_settings, 60);

  } catch (std::exception &e) {
    qCritical() << __func__ << "NewMoxaCommand Error:" << e.what();
    qCritical() << __func__ << "network_setting_table:" << network_setting_table.ToString().toStdString().c_str();
    return std::make_shared<ActStatusInternalError>("NewMoxaCommand");
  }

  if (result != MOXA_COMMAND_OK) {  // failed
    QString err_msg;
    GetNewMoxaCmdErrMsg(result, err_msg);
    qCritical() << __func__ << "NewMoxaCommandMessage:" << err_msg;
    qCritical() << __func__ << "network_setting_table:" << network_setting_table.ToString().toStdString().c_str();
    return std::make_shared<ActStatusSouthboundFailed>(err_msg);
  }

  return act_status;
}

char *UpgradeFirmwareCallback(char *intermediate_data, int intermediate_data_length) {
  int i = 0;

  QString intermediate_response = "";
  for (i = 0; i < intermediate_data_length; i++) {
    // printf("%02x", intermediate_data[i]);
    intermediate_response.append(intermediate_data[i]);
  }

  qDebug() << __func__ << QString("Intermediate response: %1").arg(intermediate_response).toStdString().c_str();

  return "";
}

ACT_STATUS ActNewMoxaCommandHandler::UpgradeFirmware(const ActDevice &device, const QString &action_key,
                                                     const ActFeatureMethodProtocol &protocol_elem,
                                                     const QString &file_path) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  int result = -1;

  try {
    // Initialize the DLL
    dll_init();

    // [bugfix: 3299] Chamberlain device connection problem
    // The target MAC can be ignored
    result = fwUpgrade_ex(
        (char *)device.GetIpv4().GetIpAddress().toStdString().c_str(), NULL, device.GetRestfulConfiguration().GetPort(),
        (char *)device.GetAccount().GetUsername().toStdString().c_str(),
        (char *)device.GetAccount().GetPassword().toStdString().c_str(), (char *)file_path.toStdString().c_str(),
        (char *)UpgradeFirmwareCallback, ACT_NEW_MOXA_COMMAND_FIRMWARE_UPGRADE_TIMEOUT);

  } catch (std::exception &e) {
    qCritical() << __func__ << "NewMoxaCommand Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("NewMoxaCommand");
  }

  if (result != MOXA_COMMAND_OK) {  // failed
    QString err_msg;
    GetNewMoxaCmdErrMsg(result, err_msg);
    qCritical() << __func__ << QString("NewMoxaCommandMessage(%1): %2").arg(result).arg(err_msg).toStdString().c_str();
    return std::make_shared<ActStatusSouthboundFailed>(err_msg);
  }

  return act_status;
}

ACT_STATUS ActNewMoxaCommandHandler::VerifyAccount(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  int result = -1;

  // Get the host ip to connect to the device
  QString host_ip = "";
  if (gl_mac_host_map.contains(device.GetMacAddress())) {
    host_ip = gl_mac_host_map[device.GetMacAddress()];
  }

  try {
    // Initialize the DLL
    dll_init();

    // [bugfix: 3299] Chamberlain device connection problem
    // The target MAC & host IP can be ignored
    result = dll_verify_password(
        (char *)device.GetIpv4().GetIpAddress().toStdString().c_str(), device.GetRestfulConfiguration().GetPort(), NULL,
        "0.0.0.0", (char *)device.GetAccount().GetUsername().toStdString().c_str(),
        (char *)device.GetAccount().GetPassword().toStdString().c_str(), ACT_NEW_MOXA_COMMAND_TIMEOUT);

  } catch (std::exception &e) {
    qCritical() << __func__ << "NewMoxaCommand Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("NewMoxaCommand");
  }

  if (result < 0) {  // failed
    QString err_msg;
    GetNewMoxaCmdErrMsg(result, err_msg);
    qCritical() << __func__ << "NewMoxaCommandMessage:" << err_msg;
    return std::make_shared<ActStatusInternalError>("NewMoxaCommand");
  }

  return act_status;
}

ACT_STATUS ActNewMoxaCommandHandler::ExportConfig(const ActDevice &device, const QString &action_key,
                                                  const ActFeatureMethodProtocol &protocol_elem,
                                                  const QString &file_path) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  int result = -1;

  try {
    // Initialize the DLL
    dll_init();

    // The target MAC can be ignored
    result = cfgExport_ex((char *)device.GetIpv4().GetIpAddress().toStdString().c_str(), NULL,
                          device.GetRestfulConfiguration().GetPort(),
                          (char *)device.GetAccount().GetUsername().toStdString().c_str(),
                          (char *)device.GetAccount().GetPassword().toStdString().c_str(),
                          (char *)file_path.toStdString().c_str(), ACT_NEW_MOXA_COMMAND_FIRMWARE_UPGRADE_TIMEOUT);

  } catch (std::exception &e) {
    qCritical() << __func__ << "NewMoxaCommand Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("NewMoxaCommand");
  }

  if (result != MOXA_COMMAND_OK) {  // failed
    QString err_msg;
    GetNewMoxaCmdErrMsg(result, err_msg);
    qCritical() << __func__ << QString("NewMoxaCommandMessage(%1): %2").arg(result).arg(err_msg).toStdString().c_str();
    return std::make_shared<ActStatusSouthboundFailed>(err_msg);
  }

  return act_status;
}

ACT_STATUS ActNewMoxaCommandHandler::ImportConfig(const ActDevice &device, const QString &action_key,
                                                  const ActFeatureMethodProtocol &protocol_elem,
                                                  const QString &file_path) {
  ACT_STATUS_INIT();

  // Check Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }

  int result = -1;

  try {
    // Initialize the DLL
    dll_init();

    // The target MAC can be ignored
    result = cfgImport_ex((char *)device.GetIpv4().GetIpAddress().toStdString().c_str(), NULL,
                          device.GetRestfulConfiguration().GetPort(),
                          (char *)device.GetAccount().GetUsername().toStdString().c_str(),
                          (char *)device.GetAccount().GetPassword().toStdString().c_str(),
                          (char *)device.GetDeviceProperty().GetModelName().toStdString().c_str(),
                          (char *)device.GetIpv4().GetIpAddress().toStdString().c_str(),
                          (char *)file_path.toStdString().c_str(), ACT_NEW_MOXA_COMMAND_FIRMWARE_UPGRADE_TIMEOUT);

  } catch (std::exception &e) {
    qCritical() << __func__ << "NewMoxaCommand Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("NewMoxaCommand");
  }

  if (result != MOXA_COMMAND_OK) {  // failed
    QString err_msg;
    GetNewMoxaCmdErrMsg(result, err_msg);
    qCritical() << __func__ << QString("NewMoxaCommandMessage(%1): %2").arg(result).arg(err_msg).toStdString().c_str();
    return std::make_shared<ActStatusSouthboundFailed>(err_msg);
  }

  return act_status;
}
