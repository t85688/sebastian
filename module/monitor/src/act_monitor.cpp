#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QDebug>
#include <QRandomGenerator>
#include <QString>
#include <QtGlobal>

#include "act_auto_scan.hpp"
#include "act_monitor.hpp"
#include "act_monitor_data.hpp"

ACT_STATUS ActMonitor::GetConnectControlByFeature(const ActDevice &device,
                                                  ActDeviceConnectStatusControl &connect_control) {
  ACT_STATUS_INIT();

  if (this->GetFakeMode()) {
    connect_control = ActDeviceConnectStatusControl(true, true, true, true);
    return ACT_STATUS_SUCCESS;
  }

  connect_control = ActDeviceConnectStatusControl(false, false, false, false);
  ActDeviceConnectStatusControl monitor_control(false, false, false, false);
  ActDeviceConnectStatusControl scan_control(false, false, false, false);

  // Get Monitor feature
  ActFeatureProfile monitor_feature_profile;
  auto monitor_status = GetDeviceFeature(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kMonitor, monitor_feature_profile);
  if (!IsActStatusSuccess(monitor_status)) {
    qCritical() << __func__
                << QString("Device(%1) GetDeviceFeature(Monitor) failed.")
                       .arg(device.GetIpv4().GetIpAddress())
                       .toStdString()
                       .c_str();
  } else {
    // Find Feature used connect protocols
    GetFeaturesUsedConnectProtocol(monitor_feature_profile, monitor_control);
  }

  // Get AutoScan feature
  ActFeatureProfile auto_feature_profile;
  auto auto_status = GetDeviceFeature(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                      ActFeatureEnum::kAutoScan, auto_feature_profile);
  if (!IsActStatusSuccess(auto_status)) {
    qCritical() << __func__
                << QString("Device(%1) GetDeviceFeature(AutoScan) failed.")
                       .arg(device.GetIpv4().GetIpAddress())
                       .toStdString()
                       .c_str();
  } else {
    // Find Feature used connect protocols
    GetFeaturesUsedConnectProtocol(auto_feature_profile, scan_control);
  }

  // If not found would return
  if ((!IsActStatusSuccess(monitor_status)) && (!IsActStatusSuccess(auto_status))) {
    QString not_found_elem =
        QString("Device(%1) Monitor & AutoScan &  featureProfile").arg(device.GetIpv4().GetIpAddress());
    // qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
    return std::make_shared<ActStatusNotFound>(not_found_elem);
  }

  // Append the AutoScan control result to monitor control
  if (scan_control.GetRESTful() || monitor_control.GetRESTful()) {
    connect_control.SetRESTful(true);
  }
  if (scan_control.GetSNMP() || monitor_control.GetSNMP()) {
    connect_control.SetSNMP(true);
  }
  if (scan_control.GetNETCONF() || monitor_control.GetNETCONF()) {
    connect_control.SetNETCONF(true);
  }

  // Configuration
  ActDeviceConnectStatusControl configuration_control(false, false, false, false);
  ActFeatureProfile configuration_feature_profile;
  auto configuration_status =
      GetDeviceFeature(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                       ActFeatureEnum::kConfiguration, configuration_feature_profile);
  if (IsActStatusSuccess(configuration_status)) {
    // Find Feature used connect protocols
    GetFeaturesUsedConnectProtocol(configuration_feature_profile, configuration_control);
    if (configuration_control.GetRESTful()) {
      connect_control.SetRESTful(true);
    }
    if (configuration_control.GetSNMP()) {
      connect_control.SetSNMP(true);
    }
    if (configuration_control.GetNETCONF()) {
      connect_control.SetNETCONF(true);
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActMonitor::UpdateDeviceConnectByMonitorFeature(ActDevice &device) {
  ACT_STATUS_INIT();

  if (this->GetFakeMode()) {
    // Fake mode
    // SLEEP_MS(500);

    auto dev_status = device.GetDeviceStatus();
    dev_status.SetAllConnectStatus(true);
    device.SetDeviceStatus(dev_status);

    return ACT_STATUS_SUCCESS;
  }

  ActDeviceConnectStatusControl connect_control(false, false, false, false);
  act_status = GetConnectControlByFeature(device, connect_control);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__
                << QString("Device(%1) GetConnectControlByFeature failed.")
                       .arg(device.GetIpv4().GetIpAddress())
                       .toStdString()
                       .c_str();

    return act_status;
  }

  // Update Device connect
  act_status = southbound_.FeatureAssignDeviceStatus(true, device, connect_control);
  if (!IsActStatusSuccess(act_status)) {
    qCritical()
        << __func__
        << QString("Device(%1) AssignDeviceStatus failed.").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

    return act_status;
  }

  // qDebug() << __func__
  //          << QString("Device(%1) Connect status: %2")
  //                 .arg(device.GetIpv4().GetIpAddress())
  //                 .arg(device.GetDeviceStatus().ToString())
  //                 .toStdString()
  //                 .c_str();

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActMonitor::AssignDeviceModuleAndInterfaces(ActDevice &device) {
  ACT_STATUS_INIT();

  if (this->GetFakeMode()) {
    // Fake mode
    // SLEEP_MS(500);
    return ACT_STATUS_SUCCESS;
  }

  // For new device
  act_status = southbound_.AssignDeviceModularConfiguration(device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "AssignDeviceModularConfiguration() failed.";
    return act_status;
  }

  act_status = southbound_.AssignInterfacesAndBuiltinPowerByModular(device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "AssignInterfacesAndBuiltinPowerByModular() failed.";
    return act_status;
  }

  // Assign device interface
  act_status = southbound_.AssignDeviceInterfacesInfo(device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "AssignDeviceInterfacesInfo() failed.";
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActMonitor::IdentifyNewDevice(ActDevice &device) {
  ACT_STATUS_INIT();

  if (this->GetFakeMode()) {
    // Fake mode
    // SLEEP_MS(500);
    return ACT_STATUS_SUCCESS;
  }

  // For new device
  // Identify Device
  act_status = southbound_.IdentifyDevice(device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "IdentifyDevice() failed.";
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActMonitor::AssignDeviceScanLinkData(ActDevice &device) {
  ACT_STATUS_INIT();

  if (this->GetFakeMode()) {
    return ACT_STATUS_SUCCESS;
  }

  // LLDP Data
  act_status = southbound_.AssignDeviceLldpData(device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "AssignDeviceLldpData() failed.";
    return act_status;
  }

  // MAC Table
  act_status = southbound_.AssignDeviceMacTable(device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "AssignDeviceMacTable() failed.";
    return act_status;
  }

  // Get IP-MAC table(arp_table & adapter_table)
  QMap<QString, QString> ip_mac_table;
  act_status = southbound_.GetIpMacTable(ip_mac_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetIpMacTable() failed.";
    return act_status;
  }

  // Update device's MAC by IP-MAC table
  QSet<ActDevice> new_project_devices;

  // Set MAC address & MAC int
  QString mac_addr = ip_mac_table[device.GetIpv4().GetIpAddress()];  //  by ACT's ip_mac_table
  qint64 mac_int = 0;
  MacAddressToQInt64(mac_addr, mac_int);
  // If MAC is "00-00-00-00-00-00" would try to replace by chassis_id
  if (mac_int == 0) {
    // Try to replace the MAC by lldp loc_chassis_id
    QString new_mac_addr = "";
    auto transfer_status = TransferChassisIdToMacFormat(device.lldp_data_.GetLocChassisId(), new_mac_addr);
    if (IsActStatusSuccess(transfer_status)) {
      // Update mac_addr & mac_int
      mac_addr = new_mac_addr;
      MacAddressToQInt64(new_mac_addr, mac_int);
    }
  }
  device.SetMacAddress(mac_addr);
  device.mac_address_int = mac_int;

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActMonitor::GenerateDeviceLinks(const ActDevice &device, const QSet<ActDevice> &project_devices,
                                           ActScanLinksResult &scan_link_result) {
  ACT_STATUS_INIT();

  if (this->GetFakeMode()) {
    return ACT_STATUS_SUCCESS;
  }

  // Get IP-MAC table(arp_table & adapter_table)
  QMap<QString, QString> ip_mac_table;
  act_status = southbound_.GetIpMacTable(ip_mac_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetIpMacTable() failed.";
    return act_status;
  }

  act_status = southbound_.CreateDeviceLink(ip_mac_table, device, project_devices, scan_link_result);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << __func__ << "CreateDeviceLink() failed. Device:" << device.GetIpv4().GetIpAddress();
    return act_status;
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActMonitor::KeepRestfulConnection(const ActDevice &device) {
  ACT_STATUS_INIT();

  if (this->GetFakeMode()) {
    return ACT_STATUS_SUCCESS;
  }

  // Check the device is using RESTful
  ActDeviceConnectStatusControl connect_control(false, false, false, false);
  act_status = GetConnectControlByFeature(device, connect_control);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__
                << QString("Device(%1) GetConnectControlByFeature failed.")
                       .arg(device.GetIpv4().GetIpAddress())
                       .toStdString()
                       .c_str();

    return act_status;
  }

  if (!connect_control.GetRESTful()) {
    return ACT_STATUS_SUCCESS;
  }

  ActFeatureSubItem sub_item;
  // Device's FeatureCapability
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kBase, "CheckConnection", "RESTful", sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // qDebug() << __func__ << "Post the Heartbeat. Device:" << device.GetIpv4().GetIpAddress();

  // Start Check connect
  auto connect_act_status = southbound_.ActionCheckRestfulConnect(device, sub_item);
  if (!IsActStatusSuccess(connect_act_status)) {
    return act_status;
  }

  return act_status;
}

void ActMonitor::MultiKeepRestfulConnectionTask(const ActDevice &device) {
  ACT_STATUS_INIT();

  if (this->GetFakeMode()) {
    return;
  }

  // Check the device is using RESTful
  ActDeviceConnectStatusControl connect_control(false, false, false, false);
  act_status = GetConnectControlByFeature(device, connect_control);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__
                << QString("Device(%1) GetConnectControlByFeature failed.")
                       .arg(device.GetIpv4().GetIpAddress())
                       .toStdString()
                       .c_str();

    return;
  }

  if (!connect_control.GetRESTful()) {
    return;
  }

  ActFeatureSubItem sub_item;
  // Device's FeatureCapability
  act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                       ActFeatureEnum::kBase, "CheckConnection", "RESTful", sub_item);
  if (!IsActStatusSuccess(act_status)) {
    return;
  }

  // qDebug() << __func__ << "Post the Heartbeat. Device:" << device.GetIpv4().GetIpAddress();

  // Start Check connect
  auto connect_act_status = southbound_.ActionCheckRestfulConnect(device, sub_item);
  if (!IsActStatusSuccess(connect_act_status)) {
    return;
  }

  return;
}

ACT_STATUS ActMonitor::FindSourceDevice(const QSet<ActDevice> &project_devices, ActSourceDevice &result_src_device) {
  ACT_STATUS_INIT();

  if (this->GetFakeMode()) {
    return ACT_STATUS_SUCCESS;
  }

  // Find the source device by device's LLDP data & MacTable
  act_status = southbound_.FindSourceDeviceByLLDPAndMacTable(project_devices.values(), result_src_device);
  if (IsActStatusSuccess(act_status)) {
    return ACT_STATUS_SUCCESS;
  }

  // Not found
  return std::make_shared<ActStatusNotFound>("Management Endpoint");
}

ACT_STATUS ActMonitor::CheckDeviceAsManagementEndpoint(const ActDevice &device, ActSourceDevice &result_src_device) {
  ACT_STATUS_INIT();

  if (this->GetFakeMode()) {
    return ACT_STATUS_SUCCESS;
  }

  // Find the source device by device's LLDP data & MacTable
  act_status = southbound_.FindSourceDeviceByLLDPAndMacTable({device}, result_src_device);
  if (IsActStatusSuccess(act_status)) {
    return ACT_STATUS_SUCCESS;
  }

  // Not found
  return std::make_shared<ActStatusNotFound>("Management Endpoint");
}

ACT_STATUS ActMonitor::GetBasicStatus(const ActDevice &device, ActMonitorBasicStatus &result_basic_status) {
  ACT_STATUS_INIT();

  ActFeatureSubItem feature_sub_item;

  result_basic_status.SetDeviceId(device.GetId());
  result_basic_status.SetDeviceIp(device.GetIpv4().GetIpAddress());

  if (this->GetFakeMode()) {
    // Fake mode
    // SLEEP_MS(500);

    // System Uptime
    qint64 uptime =
        QDateTime::currentMSecsSinceEpoch() - QDateTime(QDate::currentDate().startOfDay()).toMSecsSinceEpoch();
    qint64 days = uptime / (1000 * 60 * 60 * 24);
    qint64 hours = (uptime % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60);
    qint64 minutes = (uptime % (1000 * 60 * 60)) / (1000 * 60);
    qint64 seconds = (uptime % (1000 * 60)) / 1000;
    result_basic_status.SetSystemUptime(QString("%1d%2h%3m%4s").arg(days).arg(hours).arg(minutes).arg(seconds));

    // System Utilization
    ActMonitorSystemUtilization system_utilization;
    qreal cpuUsage =
        static_cast<qreal>(QRandomGenerator::global()->bounded(100)) + QRandomGenerator::global()->bounded(100) / 100.0;
    QString formattedCPUUsage = QString::number(cpuUsage, 'f', 2);
    system_utilization.SetCPUUsage(formattedCPUUsage.toDouble());

    qreal memoryUsage =
        static_cast<qreal>(QRandomGenerator::global()->bounded(100)) + QRandomGenerator::global()->bounded(100) / 100.0;
    QString formattedMemoryUsage = QString::number(memoryUsage, 'f', 2);
    system_utilization.SetMemoryUsage(formattedMemoryUsage.toDouble());

    result_basic_status.SetSystemUtilization(system_utilization);

    // Port Status
    QMap<qint64, ActMonitorPortStatusEntry> port_status_map;

    // According the input port number to generate the random port status
    quint16 intf_num = device.GetInterfaces().size();

    // ActLinkStatusTypeEnum port_status_val =
    //     QRandomGenerator::global()->bounded(2) ? ActLinkStatusTypeEnum::kUp : ActLinkStatusTypeEnum::kDown;
    for (qint64 port_id = 1; port_id <= intf_num; port_id++) {
      ActMonitorPortStatusEntry port_status;
      // port_status.SetLinkStatus(port_status_val);
      port_status.SetLinkStatus(ActLinkStatusTypeEnum::kUp);
      port_status_map[port_id] = port_status;
    }
    result_basic_status.SetPortStatus(port_status_map);

    // Fiber Check
    QMap<qint64, ActMonitorFiberCheckEntry> port_fiber_map;
    // According the input port number to generate the random port status
    for (qint64 port_id = 1; port_id <= intf_num; port_id++) {
      ActMonitorFiberCheckEntry fiber_check;
      fiber_check.SetDeviceId(device.GetId());
      fiber_check.SetDeviceIp(device.GetIpv4().GetIpAddress());
      fiber_check.SetInterfaceId(port_id);
      fiber_check.SetInterfaceName(device.GetInterfaces()[port_id - 1].GetInterfaceName());
      fiber_check.SetModelName("Test-SFP-module");
      fiber_check.SetSerialNumber("SFP-1GSXLC");
      fiber_check.SetWavelength("Test-Wavelength");

      double randomTempC = QRandomGenerator::global()->bounded(100.0) - 30.0;
      fiber_check.SetTemperatureC(QString::number(randomTempC, 'f', 2));

      double randomTempF = randomTempC * 9.0 / 5.0 + 32.0;
      fiber_check.SetTemperatureF(QString::number(randomTempF, 'f', 2));

      double randomVoltage = QRandomGenerator::global()->bounded(100.0);
      fiber_check.SetVoltage(QString::number(randomVoltage, 'f', 2));

      fiber_check.SetTxPower("Test-TxPower");
      fiber_check.SetRxPower("Test-RxPower");
      fiber_check.SetTemperatureLimitC("100");
      fiber_check.SetTemperatureLimitF("212");

      QList<QString> tx_power_limit;
      tx_power_limit.append("Test-TxPowerLimit-A");
      tx_power_limit.append("Test-TxPowerLimit-B");
      fiber_check.SetTxPowerLimit(tx_power_limit);

      QList<QString> rx_power_limit;
      rx_power_limit.append("Test-RxPowerLimit-A");
      rx_power_limit.append("Test-RxPowerLimit-B");
      fiber_check.SetRxPowerLimit(rx_power_limit);

      port_fiber_map[port_id] = fiber_check;
    }

    result_basic_status.SetFiberCheck(port_fiber_map);

    result_basic_status.SetDeviceName("Test-Device-Name");
    result_basic_status.SetSerialNumber("Test-Serial-Number");
    result_basic_status.SetFirmwareVersion("Test-Firmware-Version");
    result_basic_status.SetMacAddress("00-00-00-00-00-00");

    return ACT_STATUS_SUCCESS;
  }

  // System Utilization
  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetBasicStatus().GetSystemUtilization()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kMonitor, "BasicStatus", "SystemUtilization", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActMonitorSystemUtilization system_utilization;
      act_status = southbound_.ActionGetSystemUtilization(device, feature_sub_item, system_utilization);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetSystemUtilization() failed.";
        return act_status;
      } else {  // update result
        result_basic_status.SetSystemUtilization(system_utilization);
        // qDebug() << "System Utilization:" << system_utilization.ToString().toStdString().c_str();
      }
    }
  }

  // Port Status
  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetBasicStatus().GetPortStatus()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kMonitor, "BasicStatus", "PortStatus", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QMap<qint64, ActMonitorPortStatusEntry> port_status_map;
      act_status = southbound_.ActionGetPortStatus(device, feature_sub_item, port_status_map);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetPortStatus() failed.";
        return act_status;
      } else {  // update result
        result_basic_status.SetPortStatus(port_status_map);
        // qDebug() << "Port Status Size:" << port_status_map.size();
        // for (qint64 port_id : port_status_map.keys()) {
        //   qDebug() << "Port Status:" << port_id << port_status_map[port_id].ToString().toStdString().c_str();
        // }
      }
    }
  }

  // Fiber Check
  // qDebug() << __func__ << "GetFiberCheck():"
  //          << device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetBasicStatus().GetFiberCheck();

  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetBasicStatus().GetFiberCheck()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kMonitor, "BasicStatus", "FiberCheck", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QMap<qint64, ActMonitorFiberCheckEntry> port_fiber_map;
      act_status = southbound_.ActionGetFiberCheck(device, feature_sub_item, port_fiber_map);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetFiberCheck() failed.";
        return act_status;
      } else {  // update result
        result_basic_status.SetFiberCheck(port_fiber_map);
        // qDebug() << "Fiber Check Size:" << port_fiber_map.size();
        // for (qint64 port_id : port_fiber_map.keys()) {
        //   qDebug() << "Fiber Check:" << port_id << port_fiber_map[port_id].ToString().toStdString().c_str();
        // }
      }
    }
  }

  // MAC Address
  QMap<QString, QString> ip_mac_table;
  act_status = southbound_.GetIpMacTable(ip_mac_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetIpMacTable() failed.";
    return act_status;
  }
  if (ip_mac_table.contains(device.GetIpv4().GetIpAddress())) {
    result_basic_status.SetMacAddress(ip_mac_table[device.GetIpv4().GetIpAddress()]);
  }

  // FirmwareVersion
  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetIdentify().GetFirmwareVersion()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "Identify", "FirmwareVersion", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QString firmware_version;
      act_status = southbound_.ActionGetFirmwareVersion(device, feature_sub_item, firmware_version);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetFirmwareVersion() failed.";
        return act_status;
      } else {  // update result
        result_basic_status.SetFirmwareVersion(firmware_version);
      }
    }
  }

  // System Uptime
  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetSystemUptime()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kAutoScan, "DeviceInformation", "SystemUptime", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QString uptime;
      act_status = southbound_.ActionGetSystemUptime(device, feature_sub_item, uptime);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetSystemUptime() failed.";
        return act_status;
      } else {  // update result
        result_basic_status.SetSystemUptime(uptime);
      }
    }
  }

  // Product Revision
  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetProductRevision()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kAutoScan, "DeviceInformation", "ProductRevision", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QString revision;
      act_status = southbound_.ActionGetProductRevision(device, feature_sub_item, revision);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetProductRevision() failed.";
        return act_status;
      } else {  // update result
        result_basic_status.SetProductRevision(revision);
      }
    }
  }

  // Redundant Protocol
  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetRedundantProtocol()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kAutoScan, "DeviceInformation", "RedundantProtocol", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QString protocol;
      act_status = southbound_.ActionGetRedundantProtocol(device, feature_sub_item, protocol);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetRedundantProtocol() failed.";
        return act_status;
      } else {  // update result
        result_basic_status.SetRedundantProtocol(protocol);
      }
    }
  }

  // Serial Number
  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetSerialNumber()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kAutoScan, "DeviceInformation", "SerialNumber", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QString serial_number;
      act_status = southbound_.ActionGetSerialNumber(device, feature_sub_item, serial_number);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetSerialNumber() failed.";
        return act_status;
      } else {  // update result
        result_basic_status.SetSerialNumber(serial_number);
      }
    }
  }

  // Modular Info
  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetModularInfo()) {
    if ((!device.modular_info_.GetPower().isEmpty()) || (!device.modular_info_.GetEthernet().isEmpty())) {
      result_basic_status.SetModularInfo(device.modular_info_);
    } else {
      act_status =
          GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                  ActFeatureEnum::kAutoScan, "DeviceInformation", "ModularInfo", feature_sub_item);
      if (IsActStatusSuccess(act_status)) {
        ActDeviceModularInfo modular_info;

        act_status = southbound_.ActionGetModularInfo(device, feature_sub_item, modular_info);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "ActionGetModularInfo() failed.";
          return act_status;
        } else {  // update result
          // Filter no module Slot & builtin Ethernet Slot
          for (auto slot_id : modular_info.GetPower().keys()) {
            if (!modular_info.GetPower()[slot_id].GetExist()) {
              modular_info.GetPower().remove(slot_id);
            }
          }
          for (auto slot_id : modular_info.GetEthernet().keys()) {
            if (slot_id == ACT_BUILTIN_LINE_MODULE_SLOT) {
              modular_info.GetEthernet().remove(slot_id);
            }

            if (!modular_info.GetEthernet()[slot_id].GetExist()) {
              modular_info.GetEthernet().remove(slot_id);
            }
          }

          result_basic_status.SetModularInfo(modular_info);
        }
      }
    }
  }

  // Device Information Setting (DeviceName, Location, Description, ContactInformation)
  ActInformationSettingTable info_setting_table(device.GetId());
  act_status = southbound_.ScanInformationSettingTable(device, info_setting_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "ScanInformationSettingTable() failed.";
    return act_status;
  } else {  // update result
    result_basic_status.SetDeviceName(info_setting_table.GetDeviceName());
    result_basic_status.SetLocation(info_setting_table.GetLocation());
    result_basic_status.SetDescription(info_setting_table.GetDescription());
    result_basic_status.SetContactInformation(info_setting_table.GetContactInformation());
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActMonitor::GetDeviceIPConfiguration(const ActDevice &device,
                                                ActNetworkSettingTable &result_network_setting) {
  ACT_STATUS_INIT();

  ActFeatureSubItem feature_sub_item;
  result_network_setting.SetDeviceId(device.GetId());

  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetIPConfiguration()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kAutoScan, "DeviceInformation", "IPConfiguration", feature_sub_item);
    ActIpv4 ipv4;
    if (IsActStatusSuccess(act_status)) {
      act_status = southbound_.ActionGetIPConfiguration(device, feature_sub_item, ipv4);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetIPConfiguration() failed.";
        return act_status;
      }
    }
    result_network_setting.SetIpAddress(ipv4.GetIpAddress());
    result_network_setting.SetSubnetMask(ipv4.GetSubnetMask());
    result_network_setting.SetGateway(ipv4.GetGateway());
    result_network_setting.SetDNS1(ipv4.GetDNS1());
    result_network_setting.SetDNS2(ipv4.GetDNS2());
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActMonitor::GetDeviceUserAccount(const ActDevice &device, ActUserAccountTable &result_user_account) {
  ACT_STATUS_INIT();

  return southbound_.ScanUserAccountTable(device, result_user_account);
}

ACT_STATUS ActMonitor::GetDeviceLoginPolicy(const ActDevice &device, ActLoginPolicyTable &result_login_policy) {
  ACT_STATUS_INIT();

  return southbound_.ScanLoginPolicyTable(device, result_login_policy);
}

ACT_STATUS ActMonitor::GetDeviceSnmpTrapSetting(const ActDevice &device,
                                                ActSnmpTrapSettingTable &result_snmp_trap_setting) {
  ACT_STATUS_INIT();

  return southbound_.ScanSnmpTrapSettingTable(device, result_snmp_trap_setting);
}

ACT_STATUS ActMonitor::GetDeviceSyslogSetting(const ActDevice &device, ActSyslogSettingTable &result_syslog_setting) {
  ACT_STATUS_INIT();

  return southbound_.ScanSyslogSettingTable(device, result_syslog_setting);
}

ACT_STATUS ActMonitor::GetDeviceTimeSetting(const ActDevice &device, ActTimeSettingTable &result_time_setting) {
  ACT_STATUS_INIT();

  return southbound_.ScanTimeSettingTable(device, result_time_setting);
}

ACT_STATUS ActMonitor::GetDevicePortSetting(const ActDevice &device, ActPortSettingTable &result_port_setting) {
  ACT_STATUS_INIT();

  return southbound_.ScanPortSettingTable(device, result_port_setting);
}

ACT_STATUS ActMonitor::GetDeviceLoopProtection(const ActDevice &device,
                                               ActLoopProtectionTable &result_loop_protection) {
  ACT_STATUS_INIT();

  return southbound_.ScanLoopProtectionTable(device, result_loop_protection);
}

ACT_STATUS ActMonitor::GetDeviceInformationSetting(const ActDevice &device,
                                                   ActInformationSettingTable &result_info_setting) {
  ACT_STATUS_INIT();

  return southbound_.ScanInformationSettingTable(device, result_info_setting);
}

ACT_STATUS ActMonitor::GetDeviceManagementInterface(const ActDevice &device,
                                                    ActManagementInterfaceTable &result_mgmt_interface) {
  ACT_STATUS_INIT();

  return southbound_.ScanManagementInterfaceTable(device, result_mgmt_interface);
}

ACT_STATUS ActMonitor::GetDeviceRstpSetting(const ActDevice &device, ActRstpTable &result_rstp) {
  ACT_STATUS_INIT();

  return southbound_.ScanRstpTable(device, result_rstp);
}

ACT_STATUS ActMonitor::GetVlan(const ActDevice &device, ActVlanTable &result_vlan_table) {
  ACT_STATUS_INIT();

  return southbound_.ScanVlanTable(device, result_vlan_table);
}

ACT_STATUS ActMonitor::GetPortDefaultPCP(const ActDevice &device, ActDefaultPriorityTable &result_pcp_table) {
  ACT_STATUS_INIT();

  return southbound_.ScanPortDefaultPCPTable(device, result_pcp_table);
}

ACT_STATUS ActMonitor::GetStreamPriorityIngress(const ActDevice &device, ActStadPortTable &result_stad_port_table) {
  ACT_STATUS_INIT();

  return southbound_.ScanStreamPriorityIngressTable(device, result_stad_port_table);
}

ACT_STATUS ActMonitor::GetStreamPriorityEgress(const ActDevice &device, ActStadConfigTable &result_stad_config_table) {
  ACT_STATUS_INIT();

  return southbound_.ScanStreamPriorityEgressTable(device, result_stad_config_table);
}

ACT_STATUS ActMonitor::GetDeviceUnicastStatic(const ActDevice &device, ActStaticForwardTable &static_forward_table) {
  ACT_STATUS_INIT();

  return southbound_.ScanUnicastStaticTable(device, static_forward_table);
}

ACT_STATUS ActMonitor::GetDeviceMulticastStatic(const ActDevice &device, ActStaticForwardTable &static_forward_table) {
  ACT_STATUS_INIT();

  return southbound_.ScanMulticastStaticTable(device, static_forward_table);
}

ACT_STATUS ActMonitor::GetTimeSyncSetting(const ActDevice &device, ActTimeSyncTable &result_time_sync_table) {
  ACT_STATUS_INIT();

  return southbound_.ScanTimeSyncTable(device, result_time_sync_table);
}

ACT_STATUS ActMonitor::GetDeviceTimeAwareShaper(const ActDevice &device, ActGclTable &result_gcl_table) {
  ACT_STATUS_INIT();

  return southbound_.ScanTimeAwareShaperTable(device, result_gcl_table);
}

ACT_STATUS ActMonitor::GetRSTPStatus(const ActDevice &device, ActMonitorRstpStatus &result_rstp_status) {
  ACT_STATUS_INIT();

  result_rstp_status.SetDeviceId(device.GetId());
  result_rstp_status.SetDeviceIp(device.GetIpv4().GetIpAddress());

  ActFeatureSubItem feature_sub_item;
  // RSTP status
  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetRedundancy().GetRSTP()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kMonitor, "Redundancy", "RSTP", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      act_status = southbound_.ActionGetRstpStatus(device, feature_sub_item, result_rstp_status);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetRstpStatus() failed.";
        return act_status;
      }
    }
  }
  // qDebug() << "result_rstp_status(" << device.GetIpv4().GetIpAddress() << "):" <<
  // result_rstp_status.ToString().toStdString().c_str();
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActMonitor::GetTraffic(const ActDevice &device, ActDeviceMonitorTraffic &traffic) {
  ACT_STATUS_INIT();

  traffic.SetDeviceId(device.GetId());
  traffic.SetDeviceIp(device.GetIpv4().GetIpAddress());
  // The timestamp is recorded here because this is the closest point to the actual data retrieval time.
  // Because we cannot guarantee the time when the data is actually retrieved in the southbound.
  traffic.SetTimestamp(QDateTime::currentMSecsSinceEpoch());

  QMap<qint64, ActDeviceMonitorTrafficEntry> &result_traffic_view_map = traffic.GetTrafficMap();

  if (this->GetFakeMode()) {
    // Cannot extern g_monitor_device_traffic here
    // So move the fake logic to core worker module
    // Fake mode
    return ACT_STATUS_SUCCESS;
  }

  ActFeatureSubItem feature_sub_item;

  // Tx Total Octets
  // auto start = QDateTime::currentMSecsSinceEpoch();

  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetTraffic().GetTxTotalOctets()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kMonitor, "Traffic", "TxTotalOctets", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QMap<qint64, quint64> port_tx_total_octets_map;
      act_status = southbound_.ActionGetTxTotalOctets(device, feature_sub_item, port_tx_total_octets_map);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetTxTotalOctets() failed.";
        return act_status;
      } else {  // update result
        for (auto port_id : port_tx_total_octets_map.keys()) {
          result_traffic_view_map[port_id].SetTxTotalOctets(port_tx_total_octets_map[port_id]);
          result_traffic_view_map[port_id].SetPortId(port_id);
          result_traffic_view_map[port_id].SetDeviceId(device.GetId());
        }
      }
    }
  }

  // auto end = QDateTime::currentMSecsSinceEpoch();

  // qDebug() << "Execution Tx Total Octets duration:" << end - start << "ms";

  // Tx Total Packets
  // start = QDateTime::currentMSecsSinceEpoch();
  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetTraffic().GetTxTotalPackets()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kMonitor, "Traffic", "TxTotalPackets", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QMap<qint64, quint64> port_tx_total_packets_map;
      act_status = southbound_.ActionGetTxTotalPackets(device, feature_sub_item, port_tx_total_packets_map);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetTxTotalPackets() failed.";
        return act_status;
      } else {  // update result
        for (auto port_id : port_tx_total_packets_map.keys()) {
          result_traffic_view_map[port_id].SetTxTotalPackets(port_tx_total_packets_map[port_id]);
          result_traffic_view_map[port_id].SetPortId(port_id);
          result_traffic_view_map[port_id].SetDeviceId(device.GetId());
        }
      }
    }
  }
  // end = QDateTime::currentMSecsSinceEpoch();
  // qDebug() << "Execution Tx Total Packets duration:" << end - start << "ms";

  // Skip
  // // Traffic Utilization
  // if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetTraffic().GetTrafficUtilization()) {
  //   act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(),
  //   profiles_.GetDeviceProfiles(),
  //                                        ActFeatureEnum::kMonitor, "Traffic", "TrafficUtilization",
  //                                        feature_sub_item);
  //   if (IsActStatusSuccess(act_status)) {
  //     QMap<qint64, qreal> port_traffic_utilization_map;
  //     act_status = southbound_.ActionGetTrafficUtilization(device, feature_sub_item, port_traffic_utilization_map);
  //     if (!IsActStatusSuccess(act_status)) {
  //       qCritical() << __func__ << "ActionGetTrafficUtilization() failed.";
  //       return act_status;
  //     } else {  // update result
  //       for (auto port_id : port_traffic_utilization_map.keys()) {
  //         result_traffic_view_map[port_id].SetTrafficUtilization(port_traffic_utilization_map[port_id]);
  //         result_traffic_view_map[port_id].SetPortId(port_id);
  //         result_traffic_view_map[port_id].SetDeviceId(device.GetId());
  //       }
  //     }
  //   }
  // }

  // Port Speed
  if (device.GetDeviceProperty().GetFeatureGroup().GetAutoScan().GetDeviceInformation().GetPortSpeed()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kAutoScan, "DeviceInformation", "PortSpeed", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      QMap<qint64, qint64> port_speed_map;
      act_status = southbound_.ActionGetPortSpeed(device, feature_sub_item, port_speed_map);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetPortSpeed() failed.";
        return act_status;
      } else {  // update result
        for (auto port_id : port_speed_map.keys()) {
          result_traffic_view_map[port_id].SetPortSpeed(port_speed_map[port_id]);
          result_traffic_view_map[port_id].SetPortId(port_id);
          result_traffic_view_map[port_id].SetDeviceId(device.GetId());
        }
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActMonitor::GetTimeSynchronization(const ActDevice &device,
                                              ActMonitorTimeStatus &result_time_synchronization) {
  ACT_STATUS_INIT();

  result_time_synchronization.SetDeviceId(device.GetId());
  result_time_synchronization.SetDeviceIp(device.GetIpv4().GetIpAddress());

  if (this->GetFakeMode()) {
    return ACT_STATUS_SUCCESS;
  }

  ActFeatureSubItem feature_sub_item;

  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetTimeSynchronization().GetIEEE1588_2008()) {
    act_status =
        GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                ActFeatureEnum::kMonitor, "TimeSynchronization", "IEEE1588_2008", feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActMonitorTimeSyncStatus time_sync_status;
      act_status = southbound_.ActionGet1588TimeSyncStatus(device, feature_sub_item, time_sync_status);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGet1588TimeSyncStatus() failed.";
        return act_status;
      } else {  // update result
        result_time_synchronization.SetIEEE1588_2008(time_sync_status);
      }
    }
  }

  if (device.GetDeviceProperty().GetFeatureGroup().GetMonitor().GetTimeSynchronization().GetIEEE802Dot1AS_2011()) {
    act_status = GetDeviceFeatureSubItem(device, profiles_.GetFirmwareFeatureProfiles(), profiles_.GetDeviceProfiles(),
                                         ActFeatureEnum::kMonitor, "TimeSynchronization", "IEEE802Dot1AS_2011",
                                         feature_sub_item);
    if (IsActStatusSuccess(act_status)) {
      ActMonitorTimeSyncStatus time_sync_status;
      act_status = southbound_.ActionGetDot1ASTimeSyncStatus(device, feature_sub_item, time_sync_status);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "ActionGetDot1ASTimeSyncStatus() failed.";
        return act_status;
      } else {  // update result
        result_time_synchronization.SetIEEE802Dot1AS_2011(time_sync_status);
      }
    }
  }

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActMonitor::PingDevice(const ActPingJob &ping_job, ActPingDevice &ping_device) {
  ACT_STATUS_INIT();

  if (this->GetFakeMode()) {
    // Fake mode
    SLEEP_MS(500);
    // 5% return false
    QRandomGenerator random_generator;
    int random_value = random_generator.bounded(100);
    if (random_value < 5) {
      ping_device.SetStatus(false);
    } else {
      ping_device.SetStatus(true);
    }

    return ACT_STATUS_SUCCESS;
  }

  quint8 retry_times = 1;
  act_status = southbound_.PingIpAddress(ping_job.GetIp(), retry_times);
  if (IsActStatusSuccess(act_status)) {  // alive
    ping_device.SetStatus(true);
  } else {
    ping_device.SetStatus(false);
  }

  return ACT_STATUS_SUCCESS;
}

void ActMonitor::MutiPingDeviceTask(const ActPingJob &ping_job, ActPingDevice &ping_device) {
  ACT_STATUS_INIT();

  if (this->GetFakeMode()) {
    // Fake mode
    SLEEP_MS(500);
    // 5% return false
    QRandomGenerator random_generator;
    int random_value = random_generator.bounded(100);
    if (random_value < 5) {
      ping_device.SetStatus(false);
    } else {
      ping_device.SetStatus(true);
    }

    return;
  }

  quint8 retry_times = 1;
  act_status = southbound_.PingIpAddress(ping_job.GetIp(), retry_times);
  if (IsActStatusSuccess(act_status)) {  // alive
    ping_device.SetStatus(true);
  } else {
    ping_device.SetStatus(false);
  }

  return;
}
